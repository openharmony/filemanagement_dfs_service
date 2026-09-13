/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <fcntl.h>
#include <gtest/gtest.h>
#include <limits>
#include <memory>
#include <mutex>
#include <thread>
#include <unistd.h>

#include "assistant.h"
#include "cloud_disk_service_error.h"
#include "iremote_stub.h"
#include "placeholder_callback_manager.h"
#include "placeholder_helper.h"
#include "placeholder_progress_manager.h"
#include "placeholder_task_manager.h"

namespace OHOS::FileManagement::CloudDiskService::Test {
using namespace testing;
using namespace testing::ext;

namespace {
const std::string TEST_TEMP_ROOT = "/data/test";
const std::string TEST_SYNC_FOLDER = "/storage/Users/currentUser/sync";
const std::string TEST_BUNDLE_NAME = "com.example.cloud.disk";
constexpr uint32_t TEST_SYNC_FOLDER_INDEX = 100;
constexpr uint64_t TEST_DEVICE_ID = 1;
constexpr uint64_t TEST_INODE_ID = 100;
constexpr uint64_t REPLACED_TEST_INODE_ID = 101;
constexpr int32_t NON_EXECUTE_CALLBACK_TYPE = static_cast<int32_t>(CloudDiskCallbackType::DEHYDRATE);
constexpr int32_t INVALID_CALLBACK_TYPE = 3;
constexpr int32_t ADMISSION_AND_ACTIVATION_METADATA_QUERY_COUNT = 2;
static_assert(NON_EXECUTE_CALLBACK_TYPE == 2);

UniqueFd OpenTaskFd()
{
    char pathTemplate[] = "/data/test/cloud_disk_task_XXXXXX";
    UniqueFd fd(mkstemp(pathTemplate));
    if (fd < 0) {
        return UniqueFd(-1);
    }
    if (unlink(pathTemplate) != 0) {
        return UniqueFd(-1);
    }
    return fd;
}

class HydrationTestFile {
public:
    HydrationTestFile()
    {
        char pathTemplate[] = "/data/test/cloud_disk_hydration_XXXXXX";
        fd_ = mkstemp(pathTemplate);
        path_ = pathTemplate;
        if (fd_ < 0) {
            Reset();
        }
    }

    ~HydrationTestFile()
    {
        Reset();
    }

    bool IsValid() const
    {
        return fd_ >= 0;
    }

    UniqueFd DuplicateFd() const
    {
        return UniqueFd(dup(fd_));
    }

    const std::string &GetPath() const
    {
        return path_;
    }

    bool Remove()
    {
        if (fd_ >= 0) {
            close(fd_);
            fd_ = -1;
        }
        return !path_.empty() && unlink(path_.c_str()) == 0;
    }

    bool Replace()
    {
        if (fd_ < 0 || path_.empty()) {
            return false;
        }
        int32_t oldFd = fd_;
        fd_ = -1;
        if (unlink(path_.c_str()) != 0) {
            fd_ = oldFd;
            return false;
        }
        fd_ = open(path_.c_str(), O_CREAT | O_EXCL | O_RDWR | O_CLOEXEC, S_IRUSR | S_IWUSR);
        close(oldFd);
        return fd_ >= 0;
    }

private:
    void Reset()
    {
        if (fd_ >= 0) {
            close(fd_);
            fd_ = -1;
        }
        if (!path_.empty()) {
            unlink(path_.c_str());
            path_.clear();
        }
    }

    int32_t fd_ = -1;
    std::string path_;
};

CallbackExecuteRequest MakeRequest(const std::vector<uint8_t> &reqKey,
                                   uint64_t offset = 0,
                                   const std::vector<uint8_t> &data = {},
                                   uint64_t totalSize = 0,
                                   bool isComplete = true)
{
    CallbackExecuteRequest request;
    request.reqKey = reqKey;
    request.syncFolder = TEST_SYNC_FOLDER;
    request.filePath = "file.txt";
    request.offset = offset;
    request.size = data.size();
    request.data = data;
    request.totalSize = totalSize;
    request.isComplete = isComplete;
    return request;
}

auto ReturnPlaceholderState(uint8_t state)
{
    return [state](int, const char *, void *value, size_t size) {
        *static_cast<uint8_t *>(value) = MakeFileSyncState(state, 0);
        return static_cast<ssize_t>(size);
    };
}

class TaskProgressRecorder final : public IRemoteStub<ICloudDiskProgressCallback> {
public:
    void OnProgress(const HydrateProgress &progress) override
    {
        events.push_back(progress);
    }
    int32_t OnRemoteRequest(uint32_t, MessageParcel &, MessageParcel &, MessageOption &) override
    {
        return E_OK;
    }
    std::vector<HydrateProgress> events;
};

class ExecuteWriteBarrier {
public:
    void BlockFirstWrite()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        firstEntered_ = true;
        cv_.notify_all();
        cv_.wait(lock, [this] { return releaseFirst_; });
    }

    bool WaitForFirstWrite()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        return cv_.wait_for(lock, WAIT_TIMEOUT, [this] { return firstEntered_; });
    }

    void CompleteSecondWrite()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        secondDone_ = true;
        cv_.notify_all();
    }

    bool ReleaseFirstAfterSecondWrite()
    {
        bool independent = false;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            independent = cv_.wait_for(lock, WAIT_TIMEOUT, [this] { return secondDone_; });
            releaseFirst_ = true;
        }
        cv_.notify_all();
        return independent;
    }

private:
    static constexpr auto WAIT_TIMEOUT = std::chrono::seconds(2);
    std::mutex mutex_;
    std::condition_variable cv_;
    bool firstEntered_ = false;
    bool releaseFirst_ = false;
    bool secondDone_ = false;
};

class HydrationCallbackStub final : public IRemoteStub<ICloudDiskServiceCallbackTable> {
public:
    void OnCallback(const CloudDiskCallbackReqHead &reqHead, CloudDiskCallbackContext &reqContext) override
    {
        std::lock_guard<std::mutex> lock(mutex);
        callbackTypes.push_back(reqHead.callbackType);
        bool isFetch = reqHead.callbackType == CloudDiskCallbackType::FETCH_DATA && reqContext.fetchData != nullptr;
        called = called || isFetch;
        if (isFetch) {
            priority = reqContext.fetchData->priority;
        }
        cv.notify_all();
    }

    int32_t OnRemoteRequest(uint32_t, MessageParcel &, MessageParcel &, MessageOption &) override
    {
        return E_OK;
    }

    std::mutex mutex;
    std::condition_variable cv;
    bool called = false;
    CloudDiskHydratePriority priority = CLOUD_DISK_HYDRATE_PRIORITY_LOW;
    std::vector<CloudDiskCallbackType> callbackTypes;
};
} // namespace

class PlaceholderTaskManagerTest : public testing::Test {
public:
    void SetUp() override
    {
        PlaceholderTaskManager::GetInstance().StopScheduler();
        mock_ = std::make_shared<AssistantMock>();
        Assistant::ins = mock_;
        Assistant::mockErrno = 0;
        Assistant::mockFdApi = false;
        Assistant::mockPwriteApi = false;
        Assistant::mockFsyncApi = false;
    }

    void TearDown() override
    {
        PlaceholderTaskManager::GetInstance().StopScheduler();
        PlaceholderProgressManager::GetInstance().Drain();
        PlaceholderProgressManager::GetInstance().Clear();
        PlaceholderCallbackManager::GetInstance().ClearBySyncFolder(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX);
        Assistant::ins = nullptr;
        Assistant::mockErrno = 0;
        Assistant::mockFdApi = false;
        Assistant::mockPwriteApi = false;
        Assistant::mockFsyncApi = false;
        mock_ = nullptr;
    }

    void ExpectStateTransitions(int32_t taskFd, uint8_t &stateByte)
    {
        constexpr int32_t STATE_UPDATE_COUNT = 2;                    // Partially hydrated, then fully hydrated.
        constexpr int32_t STATE_READ_COUNT = STATE_UPDATE_COUNT + 1; // Initial check plus each update.
        EXPECT_CALL(*mock_, fgetxattr(taskFd, StrEq(CLOUD_DISK_FILE_SYNC_STATE_XATTR), _, sizeof(uint8_t)))
            .Times(STATE_READ_COUNT)
            .WillRepeatedly(Invoke([&stateByte](int, const char *, void *value, size_t) {
                *static_cast<uint8_t *>(value) = stateByte;
                return static_cast<ssize_t>(sizeof(uint8_t));
            }));
        EXPECT_CALL(*mock_, fsetxattr(taskFd, StrEq(CLOUD_DISK_FILE_SYNC_STATE_XATTR), _, sizeof(uint8_t), 0))
            .Times(STATE_UPDATE_COUNT)
            .WillRepeatedly(Invoke([&stateByte](int, const char *, const void *value, size_t, int) {
                stateByte = *static_cast<const uint8_t *>(value);
                return 0;
            }));
    }

    static int32_t ActivateTask(PlaceholderTaskManager &manager,
                                const PlaceholderTaskManager::RequestKey &reqKey,
                                UniqueFd outputFd = OpenTaskFd())
    {
        auto task = manager.taskMap_.at(reqKey);
        int32_t taskFd = outputFd.Get();
        task->outputFd = std::move(outputFd);
        task->fetchDispatched = true;
        task->state = PlaceholderTaskState::IN_PROGRESS;
        manager.NotifyProgressLocked(task);
        return taskFd;
    }

    void ExpectAdmissionMetadata(uint64_t inodeId, uint64_t logicalSize = 0)
    {
        EXPECT_CALL(*mock_, fstat(_, _)).WillOnce(Invoke([inodeId, logicalSize](int, struct stat *metadata) {
            metadata->st_dev = static_cast<dev_t>(TEST_DEVICE_ID);
            metadata->st_ino = static_cast<ino_t>(inodeId);
            metadata->st_mode = S_IFREG | S_IRUSR | S_IWUSR;
            metadata->st_size = static_cast<off_t>(logicalSize);
            return 0;
        }));
    }

    void ExpectAdmissionAndActivationMetadata(uint64_t admissionInode,
                                              uint64_t activationInode,
                                              uint64_t admissionSize = 0,
                                              uint64_t activationSize = 0)
    {
        EXPECT_CALL(*mock_, fstat(_, _))
            .Times(ADMISSION_AND_ACTIVATION_METADATA_QUERY_COUNT)
            .WillOnce(Invoke([admissionInode, admissionSize](int, struct stat *metadata) {
                metadata->st_dev = static_cast<dev_t>(TEST_DEVICE_ID);
                metadata->st_ino = static_cast<ino_t>(admissionInode);
                metadata->st_mode = S_IFREG | S_IRUSR | S_IWUSR;
                metadata->st_size = static_cast<off_t>(admissionSize);
                return 0;
            }))
            .WillOnce(Invoke([activationInode, activationSize](int, struct stat *metadata) {
                metadata->st_dev = static_cast<dev_t>(TEST_DEVICE_ID);
                metadata->st_ino = static_cast<ino_t>(activationInode);
                metadata->st_mode = S_IFREG | S_IRUSR | S_IWUSR;
                metadata->st_size = static_cast<off_t>(activationSize);
                return 0;
            }));
        EXPECT_CALL(*mock_, fgetxattr(_, StrEq(CLOUD_DISK_FILE_SYNC_STATE_XATTR), _, sizeof(uint8_t)))
            .WillOnce(Invoke([](int, const char *, void *value, size_t) {
                *static_cast<uint8_t *>(value) = MakeFileSyncState(PLACEHOLDER_STATE_UNHYDRATED, 0);
                return static_cast<ssize_t>(sizeof(uint8_t));
            }));
    }

    static void CheckCompletedProgress(const sptr<TaskProgressRecorder> &observer)
    {
        ASSERT_GE(observer->events.size(), 3u);
        EXPECT_EQ(observer->events.front().state, static_cast<int32_t>(HydrateProgressState::PENDING));
        EXPECT_EQ(observer->events[1].state, static_cast<int32_t>(HydrateProgressState::IN_PROGRESS));
        const auto &completed = observer->events.back();
        EXPECT_EQ(completed.state, static_cast<int32_t>(HydrateProgressState::COMPLETED));
        EXPECT_EQ(completed.filePath, TEST_SYNC_FOLDER + "/file.txt");
        EXPECT_EQ(completed.processedSize, 6u);
        EXPECT_EQ(completed.totalSize, 6u);
    }

    std::shared_ptr<AssistantMock> mock_;
};

/**
 * @tc.name: OpenValidatedHydrationFileErrorMapping_001
 * @tc.desc: Map a missing target and malformed persisted placeholder state to their dedicated errors.
 * @tc.type: RELI
 * @tc.require: NA
 */
HWTEST_F(PlaceholderTaskManagerTest, OpenValidatedHydrationFileErrorMapping_001, TestSize.Level1)
{
    HydrationTestFile file;
    ASSERT_TRUE(file.IsValid());
    const std::string path = file.GetPath();
    ASSERT_TRUE(file.Remove());
    UniqueFd outputFd;
    HydrationFileMetadata metadata;
    EXPECT_EQ(OpenValidatedHydrationFile(TEST_TEMP_ROOT, path, outputFd, metadata), E_FILE_NOT_EXIST);

    HydrationTestFile malformedFile;
    ASSERT_TRUE(malformedFile.IsValid());
    ExpectAdmissionMetadata(TEST_INODE_ID);
    EXPECT_CALL(*mock_, fgetxattr(_, StrEq(CLOUD_DISK_FILE_SYNC_STATE_XATTR), _, sizeof(uint8_t))).WillOnce(Return(0));
    EXPECT_EQ(OpenValidatedHydrationFile(TEST_TEMP_ROOT, malformedFile.GetPath(), outputFd, metadata),
              E_INVALID_PLACEHOLDER_STATE);

    Mock::VerifyAndClearExpectations(mock_.get());
    ExpectAdmissionMetadata(TEST_INODE_ID);
    EXPECT_CALL(*mock_, fgetxattr(_, StrEq(CLOUD_DISK_FILE_SYNC_STATE_XATTR), _, sizeof(uint8_t)))
        .WillOnce(Invoke([](int, const char *, void *, size_t) {
            errno = ERANGE;
            return -1;
        }));
    EXPECT_EQ(OpenValidatedHydrationFile(TEST_TEMP_ROOT, malformedFile.GetPath(), outputFd, metadata),
              E_INVALID_PLACEHOLDER_STATE);
}

/**
 * @tc.name: CreateHydrateTask_001
 * @tc.desc: Verify priority ordering, same-priority FIFO, and the per-file active-task guard.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(PlaceholderTaskManagerTest, CreateHydrateTask_001, TestSize.Level1)
{
    auto &manager = PlaceholderTaskManager::GetInstance();
    PlaceholderTaskManager::RequestKey lowKey;
    PlaceholderTaskManager::RequestKey highKey;
    PlaceholderTaskManager::RequestKey secondHighKey;
    PlaceholderTaskManager::RequestKey duplicateKey;
    ASSERT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "low.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                        CLOUD_DISK_HYDRATE_PRIORITY_LOW, OpenTaskFd(), lowKey),
              E_OK);
    ASSERT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "high.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                        CLOUD_DISK_HYDRATE_PRIORITY_HIGH, OpenTaskFd(), highKey),
              E_OK);
    ASSERT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "second-high.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                        CLOUD_DISK_HYDRATE_PRIORITY_HIGH, OpenTaskFd(), secondHighKey),
              E_OK);
    EXPECT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "low.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                        CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, OpenTaskFd(), duplicateKey),
              E_HYDRATE_IN_PROGRESS);
    EXPECT_LT(manager.taskMap_.at(lowKey)->outputFd.Get(), 0);
    auto selected = manager.SelectNextTaskLocked();
    ASSERT_NE(selected, nullptr);
    EXPECT_EQ(selected->reqKey, highKey);
    selected->state = PlaceholderTaskState::IN_PROGRESS;
    selected = manager.SelectNextTaskLocked();
    ASSERT_NE(selected, nullptr);
    EXPECT_EQ(selected->reqKey, secondHighKey);
    EXPECT_TRUE(manager.HasOutstandingTask(TEST_SYNC_FOLDER, "low.txt", TEST_SYNC_FOLDER_INDEX));
    EXPECT_FALSE(manager.HasOutstandingTask(TEST_SYNC_FOLDER, "low.txt", TEST_SYNC_FOLDER_INDEX + 1));
}

/**
 * @tc.name: CreateHydrateTask_002
 * @tc.desc: Reject each invalid task field and new work while the scheduler is stopping.
 * @tc.type: SECU
 * @tc.require: NA
 */
HWTEST_F(PlaceholderTaskManagerTest, CreateHydrateTask_002, TestSize.Level2)
{
    auto &manager = PlaceholderTaskManager::GetInstance();
    PlaceholderTaskManager::RequestKey key{1};
    EXPECT_EQ(manager.CreateHydrateTask("", "file.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                        CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, OpenTaskFd(), key),
              E_INVALID_ARG);
    EXPECT_TRUE(key.empty());
    EXPECT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                        CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, OpenTaskFd(), key),
              E_INVALID_ARG);
    EXPECT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "file.txt", "", TEST_SYNC_FOLDER_INDEX,
                                        CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, OpenTaskFd(), key),
              E_INVALID_ARG);
    EXPECT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "file.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                        static_cast<CloudDiskHydratePriority>(CLOUD_DISK_HYDRATE_PRIORITY_HIGH + 1),
                                        OpenTaskFd(), key),
              E_INVALID_ARG);
    EXPECT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "file.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                        CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, UniqueFd(-1), key),
              E_INVALID_ARG);

    EXPECT_CALL(*mock_, fstat(_, _)).WillOnce(Invoke([](int, struct stat *metadata) {
        metadata->st_size = 0;
        return 0;
    }));
    manager.stopping_ = true;
    EXPECT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "file.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                        CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, OpenTaskFd(), key),
              E_TRY_AGAIN);
    manager.stopping_ = false;
    EXPECT_TRUE(manager.taskMap_.empty());
}

/**
 * @tc.name: CreateHydrateTask_003
 * @tc.desc: Verify request-key recovery and create-sequence wrap protection with queued work.
 * @tc.type: RELI
 * @tc.require: NA
 */
HWTEST_F(PlaceholderTaskManagerTest, CreateHydrateTask_003, TestSize.Level2)
{
    auto &manager = PlaceholderTaskManager::GetInstance();
    manager.nextReqKeyValue_ = 0;
    manager.nextCreateSeq_ = std::numeric_limits<uint64_t>::max();
    PlaceholderTaskManager::RequestKey firstKey;
    ASSERT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "first.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                        CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, OpenTaskFd(), firstKey),
              E_OK);
    ASSERT_EQ(firstKey.size(), sizeof(uint64_t));
    EXPECT_EQ(firstKey.front(), 1U);
    EXPECT_EQ(manager.taskMap_.at(firstKey)->createSeq, 1U);

    manager.nextCreateSeq_ = std::numeric_limits<uint64_t>::max();
    PlaceholderTaskManager::RequestKey secondKey;
    EXPECT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "second.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                        CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, OpenTaskFd(), secondKey),
              E_TRY_AGAIN);
    EXPECT_TRUE(secondKey.empty());
    EXPECT_EQ(manager.taskMap_.size(), 1U);
}

/**
 * @tc.name: CreateHydrateTask_004
 * @tc.desc: Enforce the twenty-per-application and forty-global pending hydration task limits.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(PlaceholderTaskManagerTest, CreateHydrateTask_004, TestSize.Level1)
{
    constexpr size_t APP_TASK_LIMIT = CLOUD_DISK_MAX_PENDING_TASKS_PER_APP;
    constexpr size_t GLOBAL_TASK_LIMIT = CLOUD_DISK_MAX_PENDING_TASKS_GLOBAL;
    auto &manager = PlaceholderTaskManager::GetInstance();
    for (size_t index = 0; index < APP_TASK_LIMIT; ++index) {
        PlaceholderTaskManager::RequestKey key;
        ASSERT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "app-" + std::to_string(index), TEST_BUNDLE_NAME,
                                            TEST_SYNC_FOLDER_INDEX, CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, OpenTaskFd(),
                                            key),
                  E_OK);
    }
    PlaceholderTaskManager::RequestKey rejectedKey;
    EXPECT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "app-overflow", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                        CLOUD_DISK_HYDRATE_PRIORITY_HIGH, OpenTaskFd(), rejectedKey),
              E_HYDRATION_TASK_LIMIT_REACHED);

    for (size_t index = APP_TASK_LIMIT; index < GLOBAL_TASK_LIMIT; ++index) {
        PlaceholderTaskManager::RequestKey key;
        ASSERT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "global-" + std::to_string(index),
                                            "com.example.other." + std::to_string(index), TEST_SYNC_FOLDER_INDEX,
                                            CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, OpenTaskFd(), key),
                  E_OK);
    }
    EXPECT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "global-overflow", "com.example.last", TEST_SYNC_FOLDER_INDEX,
                                        CLOUD_DISK_HYDRATE_PRIORITY_HIGH, OpenTaskFd(), rejectedKey),
              E_HYDRATION_TASK_LIMIT_REACHED);
}

/**
 * @tc.name: SchedulerCapacity_001
 * @tc.desc: Skip an application at its active limit and stop dispatching at the global active limit.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(PlaceholderTaskManagerTest, SchedulerCapacity_001, TestSize.Level1)
{
    auto &manager = PlaceholderTaskManager::GetInstance();
    for (size_t index = 0; index < CLOUD_DISK_MAX_ACTIVE_TASKS_PER_APP; ++index) {
        PlaceholderTaskManager::RequestKey key;
        ASSERT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "active-app-" + std::to_string(index), TEST_BUNDLE_NAME,
                                            TEST_SYNC_FOLDER_INDEX, CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, OpenTaskFd(),
                                            key),
                  E_OK);
        ActivateTask(manager, key);
    }

    PlaceholderTaskManager::RequestKey blockedAppKey;
    ASSERT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "blocked-app.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                        CLOUD_DISK_HYDRATE_PRIORITY_HIGH, OpenTaskFd(), blockedAppKey),
              E_OK);
    PlaceholderTaskManager::RequestKey eligibleAppKey;
    ASSERT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "eligible-app.txt", "com.example.eligible",
                                        TEST_SYNC_FOLDER_INDEX, CLOUD_DISK_HYDRATE_PRIORITY_LOW, OpenTaskFd(),
                                        eligibleAppKey),
              E_OK);
    auto selected = manager.SelectNextTaskLocked();
    ASSERT_NE(selected, nullptr);
    EXPECT_EQ(selected->reqKey, eligibleAppKey);
    ActivateTask(manager, eligibleAppKey);

    constexpr size_t ACTIVE_AFTER_ELIGIBLE = CLOUD_DISK_MAX_ACTIVE_TASKS_PER_APP + 1;
    for (size_t index = ACTIVE_AFTER_ELIGIBLE; index < CLOUD_DISK_MAX_ACTIVE_TASKS_GLOBAL; ++index) {
        PlaceholderTaskManager::RequestKey key;
        ASSERT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "active-global-" + std::to_string(index),
                                            "com.example.global." + std::to_string(index), TEST_SYNC_FOLDER_INDEX,
                                            CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, OpenTaskFd(), key),
                  E_OK);
        ActivateTask(manager, key);
    }
    EXPECT_EQ(manager.SelectNextTaskLocked(), nullptr);
}

/**
 * @tc.name: PendingCancelNotification_001
 * @tc.desc: A queued task may deliver CANCEL as the first callback before FETCH_DATA is dispatched.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(PlaceholderTaskManagerTest, PendingCancelNotification_001, TestSize.Level1)
{
    auto callback = sptr(new HydrationCallbackStub());
    ASSERT_EQ(PlaceholderCallbackManager::GetInstance().RegisterCallbackTable(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                                                              callback),
              E_OK);
    auto &manager = PlaceholderTaskManager::GetInstance();
    PlaceholderTaskManager::RequestKey key;
    ASSERT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "file.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                        CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, OpenTaskFd(), key),
              E_OK);
    ASSERT_EQ(manager.CancelTask(TEST_SYNC_FOLDER, "file.txt", TEST_SYNC_FOLDER_INDEX), E_OK);
    std::unique_lock<std::mutex> lock(callback->mutex);
    ASSERT_TRUE(
        callback->cv.wait_for(lock, std::chrono::seconds(2), [&callback] { return !callback->callbackTypes.empty(); }));
    ASSERT_EQ(callback->callbackTypes.size(), 1U);
    EXPECT_EQ(callback->callbackTypes.front(), CloudDiskCallbackType::CANCEL_FETCH_DATA);
}

/**
 * @tc.name: PendingFileRemoved_001
 * @tc.desc: Cancel a queued task when its original file is deleted before activation.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(PlaceholderTaskManagerTest, PendingFileRemoved_001, TestSize.Level1)
{
    auto callback = sptr(new HydrationCallbackStub());
    ASSERT_EQ(PlaceholderCallbackManager::GetInstance().RegisterCallbackTable(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                                                              callback),
              E_OK);
    HydrationTestFile file;
    ASSERT_TRUE(file.IsValid());
    ExpectAdmissionMetadata(TEST_INODE_ID);
    auto &manager = PlaceholderTaskManager::GetInstance();
    PlaceholderTaskManager::RequestKey key;
    ASSERT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "file.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                        CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, file.DuplicateFd(), key,
                                        {-1, "", file.GetPath(), TEST_TEMP_ROOT}),
              E_OK);
    ASSERT_TRUE(file.Remove());
    manager.StartScheduler();
    std::unique_lock<std::mutex> lock(callback->mutex);
    ASSERT_TRUE(
        callback->cv.wait_for(lock, std::chrono::seconds(2), [&callback] { return !callback->callbackTypes.empty(); }));
    ASSERT_EQ(callback->callbackTypes.size(), 1U);
    EXPECT_EQ(callback->callbackTypes.front(), CloudDiskCallbackType::CANCEL_FETCH_DATA);
    lock.unlock();
    PlaceholderTaskState state;
    EXPECT_FALSE(manager.GetTaskState(key, state));
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, MakeRequest(key)), E_CANCELLED);
}

/**
 * @tc.name: PendingFileReplaced_001
 * @tc.desc: Cancel a queued task when the path resolves to a different inode before activation.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(PlaceholderTaskManagerTest, PendingFileReplaced_001, TestSize.Level1)
{
    auto callback = sptr(new HydrationCallbackStub());
    ASSERT_EQ(PlaceholderCallbackManager::GetInstance().RegisterCallbackTable(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                                                              callback),
              E_OK);
    HydrationTestFile file;
    ASSERT_TRUE(file.IsValid());
    ExpectAdmissionAndActivationMetadata(TEST_INODE_ID, REPLACED_TEST_INODE_ID);
    auto &manager = PlaceholderTaskManager::GetInstance();
    PlaceholderTaskManager::RequestKey key;
    ASSERT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "file.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                        CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, file.DuplicateFd(), key,
                                        {-1, "", file.GetPath(), TEST_TEMP_ROOT}),
              E_OK);
    ASSERT_TRUE(file.Replace());
    manager.StartScheduler();
    std::unique_lock<std::mutex> lock(callback->mutex);
    ASSERT_TRUE(
        callback->cv.wait_for(lock, std::chrono::seconds(2), [&callback] { return !callback->callbackTypes.empty(); }));
    ASSERT_EQ(callback->callbackTypes.size(), 1U);
    EXPECT_EQ(callback->callbackTypes.front(), CloudDiskCallbackType::CANCEL_FETCH_DATA);
}

/**
 * @tc.name: PendingFileResize_001
 * @tc.desc: Preserve file identity and use the latest logical size when a queued file changes size.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(PlaceholderTaskManagerTest, PendingFileResize_001, TestSize.Level1)
{
    constexpr uint64_t LATEST_SIZE = 4096;
    auto callback = sptr(new HydrationCallbackStub());
    ASSERT_EQ(PlaceholderCallbackManager::GetInstance().RegisterCallbackTable(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                                                              callback),
              E_OK);
    HydrationTestFile file;
    ASSERT_TRUE(file.IsValid());
    ExpectAdmissionAndActivationMetadata(TEST_INODE_ID, TEST_INODE_ID, 0, LATEST_SIZE);
    auto &manager = PlaceholderTaskManager::GetInstance();
    PlaceholderTaskManager::RequestKey key;
    ASSERT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "file.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                        CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, file.DuplicateFd(), key,
                                        {-1, "", file.GetPath(), TEST_TEMP_ROOT}),
              E_OK);
    manager.StartScheduler();
    {
        std::unique_lock<std::mutex> lock(callback->mutex);
        ASSERT_TRUE(callback->cv.wait_for(lock, std::chrono::seconds(2), [&callback] { return callback->called; }));
    }
    auto task = manager.FindTask(key);
    ASSERT_NE(task, nullptr);
    std::lock_guard<std::mutex> lock(task->mutex);
    EXPECT_EQ(task->state, PlaceholderTaskState::IN_PROGRESS);
    EXPECT_EQ(task->totalSize, LATEST_SIZE);
}

/**
 * @tc.name: CancelCancellationRecord_001
 * @tc.desc: Retain only the latest sixteen cancellation records per application and expire stale entries.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(PlaceholderTaskManagerTest, CancelCancellationRecord_001, TestSize.Level1)
{
    constexpr size_t CANCELLATION_RECORD_COUNT = 17;
    auto &manager = PlaceholderTaskManager::GetInstance();
    std::vector<PlaceholderTaskManager::RequestKey> keys;
    for (size_t index = 0; index < CANCELLATION_RECORD_COUNT; ++index) {
        PlaceholderTaskManager::RequestKey key;
        std::string filePath = "file-" + std::to_string(index);
        ASSERT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, filePath, TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                            CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, OpenTaskFd(), key),
                  E_OK);
        ASSERT_EQ(manager.CancelTask(TEST_SYNC_FOLDER, filePath, TEST_SYNC_FOLDER_INDEX), E_OK);
        keys.push_back(key);
    }
    EXPECT_EQ(manager.cancellationRecordMap_.size(), 16U);
    auto firstRequest = MakeRequest(keys.front());
    firstRequest.filePath = "file-0";
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, firstRequest), E_NO_HYDRATION_IN_PROGRESS);
    auto lastRequest = MakeRequest(keys.back());
    lastRequest.filePath = "file-16";
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, lastRequest), E_CANCELLED);
    manager.cancellationRecordMap_.at(keys.back()).expiresAt = std::chrono::steady_clock::now();
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, lastRequest), E_NO_HYDRATION_IN_PROGRESS);
}

/**
 * @tc.name: CancelTask_001
 * @tc.desc: Verify cancellation removes the active task and Execute observes the cancellation record.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(PlaceholderTaskManagerTest, CancelTask_001, TestSize.Level1)
{
    auto &manager = PlaceholderTaskManager::GetInstance();
    PlaceholderTaskManager::RequestKey reqKey;
    ASSERT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "file.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                        CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, OpenTaskFd(), reqKey),
              E_OK);
    EXPECT_EQ(manager.CancelTask(TEST_SYNC_FOLDER, "file.txt", TEST_SYNC_FOLDER_INDEX), E_OK);
    PlaceholderTaskState state = PlaceholderTaskState::PENDING;
    EXPECT_FALSE(manager.GetTaskState(reqKey, state));
    EXPECT_FALSE(manager.HasOutstandingTask(TEST_SYNC_FOLDER, "file.txt", TEST_SYNC_FOLDER_INDEX));
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, MakeRequest(reqKey)), E_CANCELLED);
    EXPECT_EQ(manager.CancelTask(TEST_SYNC_FOLDER, "missing.txt", TEST_SYNC_FOLDER_INDEX), E_NO_HYDRATION_IN_PROGRESS);
}

/**
 * @tc.name: Execute_001
 * @tc.desc: Verify missing, pending, and provider-identity Execute branches.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(PlaceholderTaskManagerTest, Execute_001, TestSize.Level2)
{
    auto &manager = PlaceholderTaskManager::GetInstance();
    PlaceholderTaskManager::RequestKey missingKey{1, 2, 3};
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, MakeRequest(missingKey)),
              E_NO_HYDRATION_IN_PROGRESS);

    PlaceholderTaskManager::RequestKey reqKey;
    ASSERT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "file.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                        CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, OpenTaskFd(), reqKey),
              E_OK);
    EXPECT_EQ(manager.Execute("other.bundle", TEST_SYNC_FOLDER_INDEX, MakeRequest(reqKey)), E_CALLBACK_NOT_REGISTERED);
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX + 1, MakeRequest(reqKey)),
              E_CALLBACK_NOT_REGISTERED);
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, MakeRequest(reqKey)), E_TRY_AGAIN);
}

/**
 * @tc.name: Execute_002
 * @tc.desc: Verify block writes advance 1 to 2 to 3, preserve sync bits, and close the task fd on completion.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(PlaceholderTaskManagerTest, Execute_002, TestSize.Level1)
{
    auto observer = sptr(new TaskProgressRecorder());
    auto &progressManager = PlaceholderProgressManager::GetInstance();
    ASSERT_EQ(progressManager.Register({1, 10}, 100, observer), E_OK);
    UniqueFd backingFd = OpenTaskFd();
    ASSERT_GE(backingFd.Get(), 0);
    UniqueFd outputFd(dup(backingFd.Get()));
    ASSERT_GE(outputFd.Get(), 0);
    auto &manager = PlaceholderTaskManager::GetInstance();
    PlaceholderTaskManager::RequestKey reqKey;
    ASSERT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "file.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                        CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, std::move(outputFd), reqKey, {100, ""}),
              E_OK);
    int32_t taskFd = ActivateTask(manager, reqKey, UniqueFd(dup(backingFd.Get())));
    ASSERT_GE(taskFd, 0);

    uint8_t stateByte = MakeFileSyncState(PLACEHOLDER_STATE_UNHYDRATED, 1);
    ExpectStateTransitions(taskFd, stateByte);
    std::vector<uint8_t> firstBlock{1, 2, 3};
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, MakeRequest(reqKey, 0, firstBlock, 6, false)),
              E_OK);
    EXPECT_EQ(GetPlaceholderStateFromFileSyncState(stateByte), PLACEHOLDER_STATE_PARTIALLY_HYDRATED);
    EXPECT_EQ(GetSyncStateFromFileSyncState(stateByte), 1);
    std::vector<uint8_t> finalBlock{4, 5, 6};
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, MakeRequest(reqKey, 3, finalBlock, 6, true)),
              E_OK);
    EXPECT_EQ(GetPlaceholderStateFromFileSyncState(stateByte), PLACEHOLDER_STATE_FULLY_HYDRATED);
    EXPECT_EQ(GetSyncStateFromFileSyncState(stateByte), 1);
    PlaceholderTaskState taskState;
    EXPECT_FALSE(manager.GetTaskState(reqKey, taskState));
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, MakeRequest(reqKey, 3, finalBlock, 6, true)),
              E_NO_HYDRATION_IN_PROGRESS);
    EXPECT_EQ(fcntl(taskFd, F_GETFD), -1);
    std::vector<uint8_t> actual(6);
    ASSERT_EQ(pread(backingFd.Get(), actual.data(), actual.size(), 0), static_cast<ssize_t>(actual.size()));
    EXPECT_EQ(actual, (std::vector<uint8_t>{1, 2, 3, 4, 5, 6}));
    progressManager.Drain();
    CheckCompletedProgress(observer);
}

/**
 * @tc.name: Execute_EmptyFile_001
 * @tc.desc: Complete an empty placeholder with the only valid zero-sized Execute shape.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(PlaceholderTaskManagerTest, Execute_EmptyFile_001, TestSize.Level1)
{
    UniqueFd backingFd = OpenTaskFd();
    ASSERT_GE(backingFd.Get(), 0);
    UniqueFd outputFd(dup(backingFd.Get()));
    auto &manager = PlaceholderTaskManager::GetInstance();
    PlaceholderTaskManager::RequestKey key;
    ASSERT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "file.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                        CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, std::move(outputFd), key),
              E_OK);
    int32_t taskFd = ActivateTask(manager, key, UniqueFd(dup(backingFd.Get())));
    ASSERT_GE(taskFd, 0);
    uint8_t stateByte = MakeFileSyncState(PLACEHOLDER_STATE_UNHYDRATED, 1);
    EXPECT_CALL(*mock_, fgetxattr(taskFd, StrEq(CLOUD_DISK_FILE_SYNC_STATE_XATTR), _, sizeof(uint8_t)))
        .WillOnce(Invoke([&stateByte](int, const char *, void *value, size_t) {
            *static_cast<uint8_t *>(value) = stateByte;
            return static_cast<ssize_t>(sizeof(uint8_t));
        }));
    EXPECT_CALL(*mock_, fsetxattr(taskFd, StrEq(CLOUD_DISK_FILE_SYNC_STATE_XATTR), _, sizeof(uint8_t), 0))
        .WillOnce(Invoke([&stateByte](int, const char *, const void *value, size_t, int) {
            stateByte = *static_cast<const uint8_t *>(value);
            return 0;
        }));
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, MakeRequest(key)), E_OK);
    EXPECT_EQ(GetPlaceholderStateFromFileSyncState(stateByte), PLACEHOLDER_STATE_FULLY_HYDRATED);
    PlaceholderTaskState state;
    EXPECT_FALSE(manager.GetTaskState(key, state));
}

/**
 * @tc.name: StartScheduler_001
 * @tc.desc: The scheduler dispatches FETCH_DATA asynchronously and retains the task after callback return.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(PlaceholderTaskManagerTest, StartScheduler_001, TestSize.Level1)
{
    auto callback = sptr(new HydrationCallbackStub());
    ASSERT_EQ(PlaceholderCallbackManager::GetInstance().RegisterCallbackTable(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                                                              callback),
              E_OK);
    auto &manager = PlaceholderTaskManager::GetInstance();
    HydrationTestFile file;
    ASSERT_TRUE(file.IsValid());
    ExpectAdmissionAndActivationMetadata(TEST_INODE_ID, TEST_INODE_ID);
    PlaceholderTaskManager::RequestKey reqKey;
    ASSERT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "file.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                        CLOUD_DISK_HYDRATE_PRIORITY_HIGH, file.DuplicateFd(), reqKey,
                                        {-1, "", file.GetPath(), TEST_TEMP_ROOT}),
              E_OK);
    manager.StartScheduler();
    bool called = false;
    {
        std::unique_lock<std::mutex> lock(callback->mutex);
        called = callback->cv.wait_for(lock, std::chrono::seconds(2), [&callback] { return callback->called; });
    }
    EXPECT_TRUE(called);
    EXPECT_EQ(callback->priority, CLOUD_DISK_HYDRATE_PRIORITY_HIGH);
    auto task = manager.FindTask(reqKey);
    ASSERT_NE(task, nullptr);
    int32_t taskFd = -1;
    {
        std::lock_guard<std::mutex> lock(task->mutex);
        EXPECT_EQ(task->state, PlaceholderTaskState::IN_PROGRESS);
        taskFd = task->outputFd.Get();
    }
    PlaceholderTaskState state;
    ASSERT_TRUE(manager.GetTaskState(reqKey, state));
    EXPECT_EQ(state, PlaceholderTaskState::IN_PROGRESS);
    EXPECT_GE(fcntl(taskFd, F_GETFD), 0);
    manager.StopScheduler();
    EXPECT_FALSE(manager.GetTaskState(reqKey, state));
    EXPECT_EQ(fcntl(taskFd, F_GETFD), -1);
}

/**
 * @tc.name: IdleTimeout_001
 * @tc.desc: Expire an idle dispatched task and send exactly one CANCEL after FETCH.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(PlaceholderTaskManagerTest, IdleTimeout_001, TestSize.Level1)
{
    auto callback = sptr(new HydrationCallbackStub());
    ASSERT_EQ(PlaceholderCallbackManager::GetInstance().RegisterCallbackTable(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                                                              callback),
              E_OK);
    auto &manager = PlaceholderTaskManager::GetInstance();
    HydrationTestFile file;
    ASSERT_TRUE(file.IsValid());
    ExpectAdmissionAndActivationMetadata(TEST_INODE_ID, TEST_INODE_ID);
    PlaceholderTaskManager::RequestKey key;
    ASSERT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "file.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                        CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, file.DuplicateFd(), key,
                                        {-1, "", file.GetPath(), TEST_TEMP_ROOT}),
              E_OK);
    manager.StartScheduler();
    {
        std::unique_lock<std::mutex> lock(callback->mutex);
        ASSERT_TRUE(callback->cv.wait_for(lock, std::chrono::seconds(2),
                                          [&callback] { return !callback->callbackTypes.empty(); }));
    }
    auto task = manager.FindTask(key);
    ASSERT_NE(task, nullptr);
    {
        std::lock_guard<std::mutex> lock(task->mutex);
        ASSERT_EQ(task->state, PlaceholderTaskState::IN_PROGRESS);
    }
    {
        std::lock_guard<std::mutex> lock(manager.mapMutex_);
        manager.deadlineMap_[key] = std::chrono::steady_clock::now();
    }
    manager.deadlineCv_.notify_one();
    {
        std::unique_lock<std::mutex> lock(callback->mutex);
        ASSERT_TRUE(callback->cv.wait_for(lock, std::chrono::seconds(2),
                                          [&callback] { return callback->callbackTypes.size() >= 2; }));
        ASSERT_EQ(callback->callbackTypes.size(), 2U);
        EXPECT_EQ(callback->callbackTypes[0], CloudDiskCallbackType::FETCH_DATA);
        EXPECT_EQ(callback->callbackTypes[1], CloudDiskCallbackType::CANCEL_FETCH_DATA);
    }
    PlaceholderTaskState state;
    EXPECT_FALSE(manager.GetTaskState(key, state));
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, MakeRequest(key)), E_CANCELLED);
}

/**
 * @tc.name: Execute_003
 * @tc.desc: Reject oversized, overflowing, inconsistent and cross-file writes before touching the fd.
 * @tc.type: SECU
 * @tc.require: NA
 */
HWTEST_F(PlaceholderTaskManagerTest, Execute_003, TestSize.Level2)
{
    auto &manager = PlaceholderTaskManager::GetInstance();
    PlaceholderTaskManager::RequestKey key;
    ASSERT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "file.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                        CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, OpenTaskFd(), key),
              E_OK);
    auto task = manager.taskMap_.at(key);
    ActivateTask(manager, key);
    Assistant::mockPwriteApi = true;
    EXPECT_CALL(*mock_, Pwrite(_, _, _, _)).Times(0);

    auto request = MakeRequest(key, 0, {1}, 1, false);
    request.size = 2;
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, request), E_INVALID_ARG);
    request = MakeRequest(key, 0, std::vector<uint8_t>(MAX_EXECUTE_DATA_SIZE + 1), MAX_EXECUTE_DATA_SIZE + 1, false);
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, request), E_INVALID_ARG);
    request = MakeRequest(key, std::numeric_limits<uint64_t>::max(), {1}, std::numeric_limits<uint64_t>::max(), false);
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, request), E_INVALID_ARG);
    request = MakeRequest(key, 1, {1}, 1, false);
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, request), E_INVALID_ARG);
    request = MakeRequest(key, 2, {}, 1, true);
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, request), E_INVALID_ARG);
    constexpr uint64_t MAX_OFFSET = static_cast<uint64_t>(std::numeric_limits<off_t>::max());
    request = MakeRequest(key, MAX_OFFSET + 1, {}, MAX_OFFSET + 1, true);
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, request), E_INVALID_ARG);
    request = MakeRequest(key, MAX_OFFSET, {1}, MAX_OFFSET + 1, true);
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, request), E_INVALID_ARG);
    request = MakeRequest(key);
    request.filePath = "other.txt";
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, request), E_INVALID_ARG);
    request = MakeRequest(key);
    request.syncFolder += "-other";
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, request), E_INVALID_ARG);
    request = MakeRequest(key);
    request.callbackType = NON_EXECUTE_CALLBACK_TYPE;
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, request), E_INVALID_ARG);
    request.callbackType = INVALID_CALLBACK_TYPE;
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, request), E_INVALID_ARG);
}

/**
 * @tc.name: Execute_013
 * @tc.desc: Reject inconsistent completion metadata and unavailable output descriptors.
 * @tc.type: SECU
 * @tc.require: NA
 */
HWTEST_F(PlaceholderTaskManagerTest, Execute_013, TestSize.Level2)
{
    auto &manager = PlaceholderTaskManager::GetInstance();
    PlaceholderTaskManager::RequestKey key;
    ASSERT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "file.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                        CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, OpenTaskFd(), key),
              E_OK);
    auto task = manager.taskMap_.at(key);
    ActivateTask(manager, key);
    Assistant::mockPwriteApi = true;
    EXPECT_CALL(*mock_, Pwrite(_, _, _, _)).Times(0);

    auto request = MakeRequest(key, 0, {}, 1, false);
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, request), E_INVALID_ARG);
    request = MakeRequest(key, 0, {}, 1, true);
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, request), E_INVALID_ARG);
    request = MakeRequest(key, 0, {}, 0, false);
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, request), E_INVALID_ARG);
    task->totalSize = 1;
    task->totalSizeInitialized = true;
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, MakeRequest(key)), E_INVALID_ARG);
    task->totalSizeInitialized = false;
    task->cachedSize = std::numeric_limits<uint64_t>::max();
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, MakeRequest(key, 0, {1}, 1, false)),
              E_INVALID_ARG);
    task->cachedSize = 0;
    task->outputFd.Reset(-1);
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, MakeRequest(key)), E_TRY_AGAIN);
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, MakeRequest(key)), E_CANCELLED);
}

/**
 * @tc.name: Execute_128KiB_001
 * @tc.desc: Accept an Execute block exactly at the 128 KiB limit.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(PlaceholderTaskManagerTest, Execute_128KiB_001, TestSize.Level1)
{
    auto &manager = PlaceholderTaskManager::GetInstance();
    PlaceholderTaskManager::RequestKey key;
    ASSERT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "file.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                        CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, OpenTaskFd(), key),
              E_OK);
    auto task = manager.taskMap_.at(key);
    ActivateTask(manager, key);
    task->hasPartialState = true;
    Assistant::mockPwriteApi = true;
    std::vector<uint8_t> data(MAX_EXECUTE_DATA_SIZE, 1);
    EXPECT_CALL(*mock_, Pwrite(task->outputFd.Get(), _, MAX_EXECUTE_DATA_SIZE, 0))
        .WillOnce(Return(static_cast<ssize_t>(MAX_EXECUTE_DATA_SIZE)));
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                              MakeRequest(key, 0, data, MAX_EXECUTE_DATA_SIZE + 1, false)),
              E_OK);
    EXPECT_EQ(task->cachedSize, MAX_EXECUTE_DATA_SIZE);
}

/**
 * @tc.name: Execute_004
 * @tc.desc: A CANCEL Execute closes the task fd and removes the task without changing file data or state.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(PlaceholderTaskManagerTest, Execute_004, TestSize.Level1)
{
    auto &manager = PlaceholderTaskManager::GetInstance();
    UniqueFd fd = OpenTaskFd();
    ASSERT_GE(fd.Get(), 0);
    PlaceholderTaskManager::RequestKey key;
    ASSERT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "file.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                        CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, std::move(fd), key),
              E_OK);
    int32_t taskFd = ActivateTask(manager, key);
    auto request = MakeRequest(key);
    request.callbackType = static_cast<int32_t>(CloudDiskCallbackType::CANCEL_FETCH_DATA);
    EXPECT_CALL(*mock_, fsetxattr(_, _, _, _, _)).Times(0);
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, request), E_OK);
    EXPECT_EQ(fcntl(taskFd, F_GETFD), -1);
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, request), E_CANCELLED);
}

/**
 * @tc.name: Execute_005
 * @tc.desc: Map write failures without erasing the task and retry EINTR and short writes at the correct offset.
 * @tc.type: RELI
 * @tc.require: NA
 */
HWTEST_F(PlaceholderTaskManagerTest, Execute_005, TestSize.Level2)
{
    auto &manager = PlaceholderTaskManager::GetInstance();
    PlaceholderTaskManager::RequestKey key;
    ASSERT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "file.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                        CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, OpenTaskFd(), key),
              E_OK);
    auto task = manager.taskMap_.at(key);
    ActivateTask(manager, key);
    task->hasPartialState = true;
    Assistant::mockPwriteApi = true;
    auto request = MakeRequest(key, 0, {1, 2, 3}, 3, false);
    const std::vector<std::pair<int, int32_t>> errors = {
        {ENOSPC, E_NO_SPACE_LEFT}, {EDQUOT, E_NO_SPACE_LEFT}, {EFBIG, E_FILE_TOO_LARGE},
        {EACCES, E_ACCES},         {EPERM, E_PERM},           {EIO, E_TRY_AGAIN},
    };
    for (const auto &[error, expected] : errors) {
        EXPECT_CALL(*mock_, Pwrite(task->outputFd.Get(), _, 3, 0))
            .WillOnce(Invoke([writeError = error](int, const void *, size_t, off_t) {
                errno = writeError;
                return -1;
            }));
        EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, request), expected);
        EXPECT_EQ(task->state.load(), PlaceholderTaskState::IN_PROGRESS);
        EXPECT_GE(fcntl(task->outputFd.Get(), F_GETFD), 0);
        Mock::VerifyAndClearExpectations(mock_.get());
    }
    EXPECT_CALL(*mock_, Pwrite(task->outputFd.Get(), _, 3, 0)).WillOnce(Return(0));
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, request), E_TRY_AGAIN);
    Mock::VerifyAndClearExpectations(mock_.get());
    {
        InSequence sequence;
        EXPECT_CALL(*mock_, Pwrite(task->outputFd.Get(), _, 3, 0))
            .WillOnce(Invoke([](int, const void *, size_t, off_t) {
                errno = EINTR;
                return -1;
            }));
        EXPECT_CALL(*mock_, Pwrite(task->outputFd.Get(), _, 3, 0)).WillOnce(Return(1));
        EXPECT_CALL(*mock_, Pwrite(task->outputFd.Get(), _, 2, 1)).WillOnce(Return(2));
    }
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, request), E_OK);
    EXPECT_EQ(task->state.load(), PlaceholderTaskState::IN_PROGRESS);
}

/**
 * @tc.name: Execute_006
 * @tc.desc: State persistence failure keeps the task and fd available for a retry.
 * @tc.type: RELI
 * @tc.require: NA
 */
HWTEST_F(PlaceholderTaskManagerTest, Execute_006, TestSize.Level2)
{
    auto &manager = PlaceholderTaskManager::GetInstance();
    UniqueFd fd = OpenTaskFd();
    ASSERT_GE(fd.Get(), 0);
    PlaceholderTaskManager::RequestKey key;
    ASSERT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "file.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                        CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, std::move(fd), key),
              E_OK);
    int32_t taskFd = ActivateTask(manager, key);
    Assistant::mockPwriteApi = true;
    EXPECT_CALL(*mock_, Pwrite(taskFd, _, 1, 0)).WillOnce(Return(1));
    EXPECT_CALL(*mock_, fgetxattr(taskFd, _, _, _)).WillOnce(Invoke([](int, const char *, void *, size_t) {
        errno = EIO;
        return -1;
    }));
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, MakeRequest(key, 0, {1}, 1, false)),
              E_TRY_AGAIN);
    EXPECT_GE(fcntl(taskFd, F_GETFD), 0);
    PlaceholderTaskState state;
    ASSERT_TRUE(manager.GetTaskState(key, state));
    EXPECT_EQ(state, PlaceholderTaskState::IN_PROGRESS);
}

/**
 * @tc.name: Execute_008
 * @tc.desc: Keep tasks retryable when stored, partial or completed placeholder-state persistence fails.
 * @tc.type: RELI
 * @tc.require: NA
 */
HWTEST_F(PlaceholderTaskManagerTest, Execute_008, TestSize.Level2)
{
    auto &manager = PlaceholderTaskManager::GetInstance();
    Assistant::mockPwriteApi = true;
    const auto runFailure = [this, &manager](const std::string &filePath, bool complete, const auto &expectStateCalls) {
        PlaceholderTaskManager::RequestKey key;
        ASSERT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, filePath, TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                            CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, OpenTaskFd(), key),
                  E_OK);
        auto task = manager.taskMap_.at(key);
        ActivateTask(manager, key);
        EXPECT_CALL(*mock_, Pwrite(task->outputFd.Get(), _, 1, 0)).WillOnce(Return(1));
        expectStateCalls(task);
        auto request = MakeRequest(key, 0, {1}, 1, complete);
        request.filePath = filePath;
        EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, request), E_TRY_AGAIN);
        PlaceholderTaskState state;
        ASSERT_TRUE(manager.GetTaskState(key, state));
        EXPECT_EQ(state, PlaceholderTaskState::IN_PROGRESS);
        Mock::VerifyAndClearExpectations(mock_.get());
    };

    runFailure("invalid-state.txt", false, [this](const std::shared_ptr<PlaceholderTaskRecord> &task) {
        EXPECT_CALL(*mock_, fgetxattr(task->outputFd.Get(), _, _, _))
            .WillOnce(Invoke([](int, const char *, void *value, size_t size) {
                *static_cast<uint8_t *>(value) = MakeFileSyncState(PLACEHOLDER_STATE_NONE, 0);
                return static_cast<ssize_t>(size);
            }));
    });
    runFailure("partial-state.txt", false, [this](const std::shared_ptr<PlaceholderTaskRecord> &task) {
        EXPECT_CALL(*mock_, fgetxattr(task->outputFd.Get(), _, _, _))
            .Times(2)
            .WillRepeatedly(Invoke([](int, const char *, void *value, size_t size) {
                *static_cast<uint8_t *>(value) = MakeFileSyncState(PLACEHOLDER_STATE_UNHYDRATED, 0);
                return static_cast<ssize_t>(size);
            }));
        EXPECT_CALL(*mock_, fsetxattr(task->outputFd.Get(), _, _, _, _))
            .WillOnce(Invoke([](int, const char *, const void *, size_t, int) {
                errno = EINVAL;
                return -1;
            }));
    });
}

/**
 * @tc.name: Execute_014
 * @tc.desc: Keep a completed task retryable when final placeholder-state persistence fails.
 * @tc.type: RELI
 * @tc.require: NA
 */
HWTEST_F(PlaceholderTaskManagerTest, Execute_014, TestSize.Level2)
{
    auto &manager = PlaceholderTaskManager::GetInstance();
    PlaceholderTaskManager::RequestKey key;
    ASSERT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "complete-state.txt", TEST_BUNDLE_NAME,
                                        TEST_SYNC_FOLDER_INDEX, CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, OpenTaskFd(), key),
              E_OK);
    auto task = manager.taskMap_.at(key);
    ActivateTask(manager, key);
    Assistant::mockPwriteApi = true;
    task->hasPartialState = true;
    EXPECT_CALL(*mock_, Pwrite(task->outputFd.Get(), _, 1, 0)).WillOnce(Return(1));
    EXPECT_CALL(*mock_, fgetxattr(task->outputFd.Get(), _, _, _))
        .WillOnce(Invoke(ReturnPlaceholderState(PLACEHOLDER_STATE_PARTIALLY_HYDRATED)));
    EXPECT_CALL(*mock_, fsetxattr(task->outputFd.Get(), _, _, _, _))
        .WillOnce(Invoke([](int, const char *, const void *, size_t, int) {
            errno = ERANGE;
            return -1;
        }));
    auto request = MakeRequest(key, 0, {1}, 1, true);
    request.filePath = "complete-state.txt";
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, request), E_TRY_AGAIN);
    PlaceholderTaskState state;
    ASSERT_TRUE(manager.GetTaskState(key, state));
    EXPECT_EQ(state, PlaceholderTaskState::IN_PROGRESS);
}

/**
 * @tc.name: Execute_009
 * @tc.desc: Preserve an active task while reporting corrupt persisted placeholder state precisely.
 * @tc.type: RELI
 * @tc.require: NA
 */
HWTEST_F(PlaceholderTaskManagerTest, Execute_009, TestSize.Level2)
{
    auto &manager = PlaceholderTaskManager::GetInstance();
    PlaceholderTaskManager::RequestKey key;
    ASSERT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "file.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                        CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, OpenTaskFd(), key),
              E_OK);
    auto task = manager.taskMap_.at(key);
    ActivateTask(manager, key);
    Assistant::mockPwriteApi = true;
    auto request = MakeRequest(key, 0, {1}, 1, false);

    EXPECT_CALL(*mock_, Pwrite(task->outputFd.Get(), _, 1, 0)).WillOnce(Return(1));
    EXPECT_CALL(*mock_, fgetxattr(task->outputFd.Get(), _, _, _))
        .WillOnce(Invoke([](int, const char *, void *value, size_t size) {
            *static_cast<uint8_t *>(value) = MakeFileSyncState(PLACEHOLDER_STATE_FULLY_HYDRATED + 1, 0);
            return static_cast<ssize_t>(size);
        }));
    EXPECT_CALL(*mock_, fsetxattr(_, _, _, _, _)).Times(0);
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, request), E_INVALID_PLACEHOLDER_STATE);
    EXPECT_EQ(task->state.load(), PlaceholderTaskState::IN_PROGRESS);

    Mock::VerifyAndClearExpectations(mock_.get());
    EXPECT_CALL(*mock_, Pwrite(task->outputFd.Get(), _, 1, 0)).WillOnce(Return(1));
    EXPECT_CALL(*mock_, fgetxattr(task->outputFd.Get(), _, _, _)).WillOnce(Return(0));
    EXPECT_CALL(*mock_, fsetxattr(_, _, _, _, _)).Times(0);
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, request), E_INVALID_PLACEHOLDER_STATE);
    EXPECT_EQ(task->state.load(), PlaceholderTaskState::IN_PROGRESS);
}

/**
 * @tc.name: Execute_007
 * @tc.desc: A blocked pwrite holds only its own task mutex and does not block another file's Execute.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(PlaceholderTaskManagerTest, Execute_007, TestSize.Level1)
{
    auto &manager = PlaceholderTaskManager::GetInstance();
    PlaceholderTaskManager::RequestKey firstKey;
    PlaceholderTaskManager::RequestKey secondKey;
    ASSERT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "file.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                        CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, OpenTaskFd(), firstKey),
              E_OK);
    ASSERT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "second.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                        CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, OpenTaskFd(), secondKey),
              E_OK);
    auto firstTask = manager.taskMap_.at(firstKey);
    auto secondTask = manager.taskMap_.at(secondKey);
    ActivateTask(manager, firstKey);
    ActivateTask(manager, secondKey);
    firstTask->hasPartialState = secondTask->hasPartialState = true;
    Assistant::mockPwriteApi = true;
    ExecuteWriteBarrier barrier;
    EXPECT_CALL(*mock_, Pwrite(firstTask->outputFd.Get(), _, 1, 0))
        .WillOnce(Invoke([&barrier](int, const void *, size_t, off_t) {
            barrier.BlockFirstWrite();
            return 1;
        }));
    EXPECT_CALL(*mock_, Pwrite(secondTask->outputFd.Get(), _, 1, 0)).WillOnce(Return(1));
    int32_t firstResult = E_TRY_AGAIN;
    int32_t secondResult = E_TRY_AGAIN;
    auto firstRequest = MakeRequest(firstKey, 0, {1}, 1, false);
    auto secondRequest = MakeRequest(secondKey, 0, {2}, 1, false);
    secondRequest.filePath = "second.txt";
    std::thread first([&manager, &firstResult, &firstRequest] {
        firstResult = manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, firstRequest);
    });
    EXPECT_TRUE(barrier.WaitForFirstWrite());
    {
        std::unique_lock<std::mutex> taskLock(firstTask->mutex, std::try_to_lock);
        EXPECT_FALSE(taskLock.owns_lock());
    }
    std::thread second([&manager, &secondResult, &secondRequest, &barrier] {
        secondResult = manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, secondRequest);
        barrier.CompleteSecondWrite();
    });
    bool independent = barrier.ReleaseFirstAfterSecondWrite();
    first.join();
    second.join();
    EXPECT_TRUE(independent);
    EXPECT_EQ(firstResult, E_OK);
    EXPECT_EQ(secondResult, E_OK);
}
/**
 * @tc.name: ProgressMetadata_001
 * @tc.desc: Capture original path and metadata, then accumulate only successful Execute bytes.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(PlaceholderTaskManagerTest, ProgressMetadata_001, TestSize.Level2)
{
    auto &manager = PlaceholderTaskManager::GetInstance();
    EXPECT_CALL(*mock_, fstat(_, _)).WillOnce(Invoke([](int, struct stat *metadata) {
        metadata->st_size = 20;
        return 0;
    }));
    PlaceholderTaskManager::RequestKey key;
    ASSERT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "file.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                        CLOUD_DISK_HYDRATE_PRIORITY_HIGH, OpenTaskFd(), key,
                                        {100, "/original/file.txt"}),
              E_OK);
    auto task = manager.taskMap_.at(key);
    EXPECT_EQ(task->absolutePath, "/original/file.txt");
    EXPECT_EQ(task->userId, 100);
    EXPECT_EQ(task->totalSize, 20u);
    EXPECT_EQ(task->cachedSize, 0u);
    ActivateTask(manager, key);
    task->hasPartialState = true;
    Assistant::mockPwriteApi = true;
    EXPECT_CALL(*mock_, Pwrite(_, _, 2, 0)).WillOnce(Return(2));
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, MakeRequest(key, 0, {1, 2}, 5, false)), E_OK);
    EXPECT_EQ(task->totalSize, 5u);
    EXPECT_EQ(task->cachedSize, 2u);
    EXPECT_CALL(*mock_, Pwrite(_, _, 3, 2)).WillOnce(Return(0));
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, MakeRequest(key, 2, {3, 4, 5}, 5, false)),
              E_TRY_AGAIN);
    EXPECT_EQ(task->cachedSize, 2u);
    EXPECT_CALL(*mock_, Pwrite(_, _, 3, 2)).WillOnce(Return(3));
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, MakeRequest(key, 2, {3, 4, 5}, 5, false)),
              E_OK);
    EXPECT_EQ(task->cachedSize, 5u);
    EXPECT_EQ(manager.CancelTask(TEST_SYNC_FOLDER, "file.txt", TEST_SYNC_FOLDER_INDEX), E_OK);
    EXPECT_TRUE(task->terminalProgressSent);
    EXPECT_FALSE(manager.HasOutstandingTask(TEST_SYNC_FOLDER, "file.txt", TEST_SYNC_FOLDER_INDEX));
}
/**
 * @tc.name: ProgressMetadata_002
 * @tc.desc: Reject metadata failures without retaining an fd, record or queued task.
 * @tc.type: RELI
 * @tc.require: NA
 */
HWTEST_F(PlaceholderTaskManagerTest, ProgressMetadata_002, TestSize.Level2)
{
    auto &manager = PlaceholderTaskManager::GetInstance();
    EXPECT_CALL(*mock_, fstat(_, _)).WillOnce(Return(-1));
    UniqueFd fd = OpenTaskFd();
    int32_t rawFd = fd.Get();
    PlaceholderTaskManager::RequestKey key;
    EXPECT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "file.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                        CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, std::move(fd), key),
              E_TRY_AGAIN);
    EXPECT_TRUE(manager.taskMap_.empty());
    EXPECT_EQ(fcntl(rawFd, F_GETFD), -1);
}

/**
 * @tc.name: OpenValidatedHydrationFile_001
 * @tc.desc: Reject empty, missing, escaped, and non-child hydration paths before opening the target.
 * @tc.type: SECU
 * @tc.require: NA
 */
HWTEST_F(PlaceholderTaskManagerTest, OpenValidatedHydrationFile_001, TestSize.Level2)
{
    HydrationTestFile file;
    ASSERT_TRUE(file.IsValid());
    UniqueFd outputFd;
    HydrationFileMetadata metadata;
    EXPECT_EQ(OpenValidatedHydrationFile("", file.GetPath(), outputFd, metadata), E_INVALID_ARG);
    EXPECT_EQ(OpenValidatedHydrationFile(TEST_TEMP_ROOT, "", outputFd, metadata), E_INVALID_ARG);
    EXPECT_EQ(OpenValidatedHydrationFile(file.GetPath() + "-missing", file.GetPath(), outputFd, metadata),
              E_SYNC_FOLDER_PATH_NOT_EXIST);
    EXPECT_EQ(OpenValidatedHydrationFile("/dev", file.GetPath(), outputFd, metadata), E_INVALID_ARG);
    EXPECT_EQ(OpenValidatedHydrationFile(TEST_TEMP_ROOT, TEST_TEMP_ROOT, outputFd, metadata), E_INVALID_ARG);
}

/**
 * @tc.name: OpenValidatedHydrationFile_002
 * @tc.desc: Map root, component, metadata, directory, and invalid-size failures precisely.
 * @tc.type: RELI
 * @tc.require: NA
 */
HWTEST_F(PlaceholderTaskManagerTest, OpenValidatedHydrationFile_002, TestSize.Level2)
{
    HydrationTestFile file;
    ASSERT_TRUE(file.IsValid());
    UniqueFd outputFd;
    HydrationFileMetadata metadata;

    UniqueFd rootFd = OpenTaskFd();
    UniqueFd nestedRootFd = OpenTaskFd();
    UniqueFd dataDirFd = OpenTaskFd();
    UniqueFd testDirFd = OpenTaskFd();
    ASSERT_GE(rootFd.Get(), 0);
    ASSERT_GE(nestedRootFd.Get(), 0);
    ASSERT_GE(dataDirFd.Get(), 0);
    ASSERT_GE(testDirFd.Get(), 0);

    Assistant::mockFdApi = true;
    Assistant::mockErrno = EACCES;
    EXPECT_CALL(*mock_, Open(_, _, _)).WillOnce(Return(-1));
    EXPECT_EQ(OpenValidatedHydrationFile(TEST_TEMP_ROOT, file.GetPath(), outputFd, metadata), E_ACCES);
    Mock::VerifyAndClearExpectations(mock_.get());

    int32_t rawRootFd = rootFd.Release();
    Assistant::mockErrno = ELOOP;
    EXPECT_CALL(*mock_, Open(_, _, _)).WillOnce(Return(rawRootFd));
    EXPECT_CALL(*mock_, OpenAt(rawRootFd, _, _, _)).WillOnce(Return(-1));
    EXPECT_EQ(OpenValidatedHydrationFile(TEST_TEMP_ROOT, file.GetPath(), outputFd, metadata), E_INVALID_ARG);
    Mock::VerifyAndClearExpectations(mock_.get());

    int32_t rawNestedRootFd = nestedRootFd.Release();
    int32_t rawDataDirFd = dataDirFd.Release();
    int32_t rawTestDirFd = testDirFd.Release();
    Assistant::mockErrno = EACCES;
    {
        InSequence sequence;
        EXPECT_CALL(*mock_, Open(StrEq("/"), _, _)).WillOnce(Return(rawNestedRootFd));
        EXPECT_CALL(*mock_, OpenAt(rawNestedRootFd, StrEq("data"), _, _)).WillOnce(Return(rawDataDirFd));
        EXPECT_CALL(*mock_, OpenAt(rawDataDirFd, StrEq("test"), _, _)).WillOnce(Return(rawTestDirFd));
        EXPECT_CALL(*mock_, OpenAt(rawTestDirFd, _, _, _)).WillOnce(Return(-1));
    }
    EXPECT_EQ(OpenValidatedHydrationFile("/", file.GetPath(), outputFd, metadata), E_ACCES);
    Mock::VerifyAndClearExpectations(mock_.get());
    Assistant::mockErrno = 0;
    Assistant::mockFdApi = false;
}

/**
 * @tc.name: OpenValidatedHydrationFile_004
 * @tc.desc: Map metadata, directory, and invalid-size failures precisely.
 * @tc.type: RELI
 * @tc.require: NA
 */
HWTEST_F(PlaceholderTaskManagerTest, OpenValidatedHydrationFile_004, TestSize.Level2)
{
    HydrationTestFile file;
    ASSERT_TRUE(file.IsValid());
    UniqueFd outputFd;
    HydrationFileMetadata metadata;

    EXPECT_CALL(*mock_, fstat(_, _)).WillOnce(Invoke([](int, struct stat *) {
        errno = EACCES;
        return -1;
    }));
    EXPECT_EQ(OpenValidatedHydrationFile(TEST_TEMP_ROOT, file.GetPath(), outputFd, metadata), E_ACCES);
    Mock::VerifyAndClearExpectations(mock_.get());

    EXPECT_CALL(*mock_, fstat(_, _)).WillOnce(Invoke([](int, struct stat *fileStat) {
        fileStat->st_mode = S_IFDIR;
        fileStat->st_size = 0;
        return 0;
    }));
    EXPECT_EQ(OpenValidatedHydrationFile(TEST_TEMP_ROOT, file.GetPath(), outputFd, metadata), E_INVALID_ARG);
    Mock::VerifyAndClearExpectations(mock_.get());

    EXPECT_CALL(*mock_, fstat(_, _)).WillOnce(Invoke([](int, struct stat *fileStat) {
        fileStat->st_mode = S_IFREG;
        fileStat->st_size = -1;
        return 0;
    }));
    EXPECT_EQ(OpenValidatedHydrationFile(TEST_TEMP_ROOT, file.GetPath(), outputFd, metadata), E_INVALID_ARG);
}

/**
 * @tc.name: OpenValidatedHydrationFile_003
 * @tc.desc: Cover all persisted placeholder-state decisions and return successful metadata.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(PlaceholderTaskManagerTest, OpenValidatedHydrationFile_003, TestSize.Level1)
{
    HydrationTestFile file;
    ASSERT_TRUE(file.IsValid());
    UniqueFd outputFd;
    HydrationFileMetadata metadata;
    const auto runState = [this, &file, &outputFd, &metadata](uint8_t state, int32_t expected) {
        ExpectAdmissionMetadata(TEST_INODE_ID, 7);
        EXPECT_CALL(*mock_, fgetxattr(_, StrEq(CLOUD_DISK_FILE_SYNC_STATE_XATTR), _, sizeof(uint8_t)))
            .WillOnce(Invoke(ReturnPlaceholderState(state)));
        EXPECT_EQ(OpenValidatedHydrationFile(TEST_TEMP_ROOT, file.GetPath(), outputFd, metadata), expected);
        Mock::VerifyAndClearExpectations(mock_.get());
    };

    runState(PLACEHOLDER_STATE_NONE, E_NOT_A_PLACEHOLDER);
    runState(PLACEHOLDER_STATE_FULLY_HYDRATED, E_ALREADY_HYDRATED);
    runState(PLACEHOLDER_STATE_FULLY_HYDRATED + 1, E_INVALID_PLACEHOLDER_STATE);
    ExpectAdmissionMetadata(TEST_INODE_ID, 7);
    EXPECT_CALL(*mock_, fgetxattr(_, StrEq(CLOUD_DISK_FILE_SYNC_STATE_XATTR), _, sizeof(uint8_t)))
        .WillOnce(Invoke([](int, const char *, void *, size_t) {
            errno = ENOENT;
            return -1;
        }));
    EXPECT_EQ(OpenValidatedHydrationFile(TEST_TEMP_ROOT, file.GetPath(), outputFd, metadata), E_FILE_NOT_EXIST);
    Mock::VerifyAndClearExpectations(mock_.get());

    runState(PLACEHOLDER_STATE_PARTIALLY_HYDRATED, E_OK);
    EXPECT_GE(outputFd.Get(), 0);
    EXPECT_EQ(metadata.deviceId, TEST_DEVICE_ID);
    EXPECT_EQ(metadata.inodeId, TEST_INODE_ID);
    EXPECT_EQ(metadata.logicalSize, 7U);
}

/**
 * @tc.name: CreateHydrateTask_005
 * @tc.desc: Reject a priority below the public range and negative file metadata.
 * @tc.type: RELI
 * @tc.require: NA
 */
HWTEST_F(PlaceholderTaskManagerTest, CreateHydrateTask_005, TestSize.Level2)
{
    auto &manager = PlaceholderTaskManager::GetInstance();
    PlaceholderTaskManager::RequestKey key{1};
    EXPECT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "file.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                        static_cast<CloudDiskHydratePriority>(-1), OpenTaskFd(), key),
              E_INVALID_ARG);
    EXPECT_TRUE(key.empty());

    EXPECT_CALL(*mock_, fstat(_, _)).WillOnce(Invoke([](int, struct stat *metadata) {
        metadata->st_size = -1;
        return 0;
    }));
    EXPECT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "file.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                        CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, OpenTaskFd(), key),
              E_TRY_AGAIN);
    EXPECT_TRUE(key.empty());
    EXPECT_TRUE(manager.taskMap_.empty());
}

/**
 * @tc.name: GenerateRequestKeyLocked_001
 * @tc.desc: Wrap the request-key counter and skip keys held by live tasks or cancellation records.
 * @tc.type: RELI
 * @tc.require: NA
 */
HWTEST_F(PlaceholderTaskManagerTest, GenerateRequestKeyLocked_001, TestSize.Level2)
{
    auto &manager = PlaceholderTaskManager::GetInstance();
    manager.nextReqKeyValue_ = std::numeric_limits<uint64_t>::max();
    auto maxKey = manager.GenerateRequestKeyLocked();
    EXPECT_EQ(maxKey.size(), sizeof(uint64_t));
    EXPECT_EQ(manager.nextReqKeyValue_, 1U);

    manager.nextReqKeyValue_ = 20;
    auto taskKey = manager.GenerateRequestKeyLocked();
    auto task = std::make_shared<PlaceholderTaskRecord>();
    task->reqKey = taskKey;
    manager.taskMap_[taskKey] = task;
    manager.nextReqKeyValue_ = 20;
    auto afterTaskKey = manager.GenerateRequestKeyLocked();
    EXPECT_NE(afterTaskKey, taskKey);
    manager.taskMap_.clear();

    manager.nextReqKeyValue_ = 30;
    auto cancellationRecordKey = manager.GenerateRequestKeyLocked();
    PlaceholderTaskManager::CancelledTaskRecord cancellationRecord;
    cancellationRecord.reqKey = cancellationRecordKey;
    cancellationRecord.expiresAt = std::chrono::steady_clock::now() + std::chrono::minutes(1);
    cancellationRecord.createSeq = 1;
    manager.cancellationRecordMap_[cancellationRecordKey] = cancellationRecord;
    manager.nextReqKeyValue_ = 30;
    auto afterCancellationRecordKey = manager.GenerateRequestKeyLocked();
    EXPECT_NE(afterCancellationRecordKey, cancellationRecordKey);
    manager.ClearCancellationRecords();
}

/**
 * @tc.name: StartScheduler_002
 * @tc.desc: Starting an already running scheduler is idempotent and creates no extra monitor task.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(PlaceholderTaskManagerTest, StartScheduler_002, TestSize.Level1)
{
    auto &manager = PlaceholderTaskManager::GetInstance();
    manager.StartScheduler();
    ASSERT_TRUE(manager.running_);
    ASSERT_EQ(manager.monitorHandles_.size(), 1U);
    manager.StartScheduler();
    EXPECT_TRUE(manager.running_);
    EXPECT_EQ(manager.monitorHandles_.size(), 1U);
    manager.StopScheduler();
    EXPECT_FALSE(manager.running_);
    EXPECT_FALSE(manager.stopping_);
    EXPECT_TRUE(manager.monitorHandles_.empty());
}

/**
 * @tc.name: ScheduleDispatch_001
 * @tc.desc: Cover stopped, stopping, already-scheduled, and empty dispatch-loop guards.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(PlaceholderTaskManagerTest, ScheduleDispatch_001, TestSize.Level2)
{
    auto &manager = PlaceholderTaskManager::GetInstance();
    manager.running_ = false;
    manager.stopping_ = false;
    manager.dispatchScheduled_ = false;
    manager.ScheduleDispatch();
    EXPECT_FALSE(manager.dispatchScheduled_);

    manager.running_ = true;
    manager.stopping_ = true;
    manager.ScheduleDispatch();
    EXPECT_FALSE(manager.dispatchScheduled_);

    manager.stopping_ = false;
    manager.dispatchScheduled_ = true;
    manager.ScheduleDispatch();
    EXPECT_TRUE(manager.dispatchScheduled_);
    manager.DispatchLoop();
    EXPECT_FALSE(manager.dispatchScheduled_);

    manager.running_ = true;
    manager.stopping_ = true;
    manager.dispatchScheduled_ = true;
    manager.DispatchLoop();
    EXPECT_FALSE(manager.dispatchScheduled_);
    manager.running_ = false;
    manager.stopping_ = false;
}

/**
 * @tc.name: ActivateAndDispatch_001
 * @tc.desc: Ignore terminal records and remove a validated task when no provider callback is registered.
 * @tc.type: RELI
 * @tc.require: NA
 */
HWTEST_F(PlaceholderTaskManagerTest, ActivateAndDispatch_001, TestSize.Level2)
{
    auto &manager = PlaceholderTaskManager::GetInstance();
    auto terminalTask = std::make_shared<PlaceholderTaskRecord>();
    terminalTask->state = PlaceholderTaskState::COMPLETED;
    manager.ActivateAndDispatch(terminalTask);
    EXPECT_EQ(terminalTask->state, PlaceholderTaskState::COMPLETED);

    HydrationTestFile file;
    ASSERT_TRUE(file.IsValid());
    ExpectAdmissionAndActivationMetadata(TEST_INODE_ID, TEST_INODE_ID);
    PlaceholderTaskManager::RequestKey key;
    ASSERT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "file.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                        CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, file.DuplicateFd(), key,
                                        {-1, "", file.GetPath(), TEST_TEMP_ROOT}),
              E_OK);
    manager.ActivateAndDispatch(manager.taskMap_.at(key));
    PlaceholderTaskState state;
    EXPECT_FALSE(manager.GetTaskState(key, state));
    EXPECT_EQ(manager.cancellationRecordMap_.count(key), 0U);
}

/**
 * @tc.name: ActivateAndDispatch_002
 * @tc.desc: Cancel activation when the reopened file has moved to a different device.
 * @tc.type: RELI
 * @tc.require: NA
 */
HWTEST_F(PlaceholderTaskManagerTest, ActivateAndDispatch_002, TestSize.Level2)
{
    constexpr uint64_t REPLACED_DEVICE_ID = TEST_DEVICE_ID + 1;
    auto &manager = PlaceholderTaskManager::GetInstance();
    HydrationTestFile file;
    ASSERT_TRUE(file.IsValid());
    EXPECT_CALL(*mock_, fstat(_, _))
        .Times(2)
        .WillOnce(Invoke([](int, struct stat *metadata) {
            metadata->st_dev = static_cast<dev_t>(TEST_DEVICE_ID);
            metadata->st_ino = static_cast<ino_t>(TEST_INODE_ID);
            metadata->st_mode = S_IFREG;
            metadata->st_size = 0;
            return 0;
        }))
        .WillOnce(Invoke([](int, struct stat *metadata) {
            metadata->st_dev = static_cast<dev_t>(REPLACED_DEVICE_ID);
            metadata->st_ino = static_cast<ino_t>(TEST_INODE_ID);
            metadata->st_mode = S_IFREG;
            metadata->st_size = 0;
            return 0;
        }));
    EXPECT_CALL(*mock_, fgetxattr(_, _, _, _)).WillOnce(Invoke([](int, const char *, void *value, size_t size) {
        *static_cast<uint8_t *>(value) = MakeFileSyncState(PLACEHOLDER_STATE_UNHYDRATED, 0);
        return static_cast<ssize_t>(size);
    }));
    PlaceholderTaskManager::RequestKey key;
    ASSERT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "file.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                        CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, file.DuplicateFd(), key,
                                        {-1, "", file.GetPath(), TEST_TEMP_ROOT}),
              E_OK);
    manager.ActivateAndDispatch(manager.taskMap_.at(key));
    EXPECT_EQ(manager.taskMap_.count(key), 0U);
    EXPECT_EQ(manager.cancellationRecordMap_.count(key), 1U);
}

/**
 * @tc.name: CancelTasksBySyncFolder_001
 * @tc.desc: Cancel only matching queued and active tasks, then cancel all remaining tasks without cancellation records.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(PlaceholderTaskManagerTest, CancelTasksBySyncFolder_001, TestSize.Level1)
{
    auto &manager = PlaceholderTaskManager::GetInstance();
    PlaceholderTaskManager::RequestKey pendingKey;
    PlaceholderTaskManager::RequestKey activeKey;
    PlaceholderTaskManager::RequestKey otherKey;
    ASSERT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "pending.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                        CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, OpenTaskFd(), pendingKey),
              E_OK);
    ASSERT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "active.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                        CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, OpenTaskFd(), activeKey),
              E_OK);
    ASSERT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "other.txt", "com.example.other", TEST_SYNC_FOLDER_INDEX,
                                        CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, OpenTaskFd(), otherKey),
              E_OK);
    ActivateTask(manager, activeKey);

    manager.CancelTasksBySyncFolder(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, PlaceholderTaskCancelReason::UNREGISTER);
    EXPECT_EQ(manager.taskMap_.count(pendingKey), 0U);
    EXPECT_EQ(manager.taskMap_.count(activeKey), 0U);
    EXPECT_EQ(manager.taskMap_.count(otherKey), 1U);
    EXPECT_EQ(manager.cancellationRecordMap_.count(pendingKey), 0U);
    EXPECT_EQ(manager.cancellationRecordMap_.count(activeKey), 1U);

    manager.CancelAllTasks(PlaceholderTaskCancelReason::USER_SWITCH);
    EXPECT_TRUE(manager.taskMap_.empty());
    EXPECT_EQ(manager.cancellationRecordMap_.count(otherKey), 0U);
    manager.ClearCancellationRecords();
    EXPECT_TRUE(manager.cancellationRecordMap_.empty());
    EXPECT_TRUE(manager.cancellationOrder_.empty());
}

/**
 * @tc.name: AddCancellationRecordLocked_001
 * @tc.desc: Wrap cancellation record sequence numbers, enforce the global cap, and purge stale order entries.
 * @tc.type: RELI
 * @tc.require: NA
 */
HWTEST_F(PlaceholderTaskManagerTest, AddCancellationRecordLocked_001, TestSize.Level2)
{
    constexpr size_t GLOBAL_CANCELLATION_RECORD_LIMIT = 32;
    auto &manager = PlaceholderTaskManager::GetInstance();
    manager.nextCancellationSeq_ = std::numeric_limits<uint64_t>::max();
    std::vector<PlaceholderTaskManager::RequestKey> keys;
    for (size_t index = 0; index <= GLOBAL_CANCELLATION_RECORD_LIMIT; ++index) {
        auto task = std::make_shared<PlaceholderTaskRecord>();
        task->reqKey = {static_cast<uint8_t>(index)};
        task->syncFolder = TEST_SYNC_FOLDER;
        task->filePath = "file-" + std::to_string(index);
        task->bundleName = "com.example.cancellation_record." + std::to_string(index);
        task->syncFolderIndex = TEST_SYNC_FOLDER_INDEX;
        manager.AddCancellationRecordLocked(task);
        keys.push_back(task->reqKey);
    }
    EXPECT_EQ(manager.cancellationRecordMap_.size(), GLOBAL_CANCELLATION_RECORD_LIMIT);
    EXPECT_EQ(manager.cancellationRecordMap_.count(keys.front()), 0U);
    EXPECT_EQ(manager.cancellationRecordMap_.count(keys.back()), 1U);
    EXPECT_EQ(manager.nextCancellationSeq_, GLOBAL_CANCELLATION_RECORD_LIMIT + 2U);

    const auto &latest = manager.cancellationRecordMap_.at(keys.back());
    manager.cancellationOrder_.emplace_front(keys.back(), latest.createSeq + 1);
    manager.PurgeExpiredCancellationRecordsLocked();
    EXPECT_EQ(manager.cancellationOrder_.size(), GLOBAL_CANCELLATION_RECORD_LIMIT);
    EXPECT_EQ(manager.cancellationRecordMap_.count(keys.back()), 1U);
    manager.ClearCancellationRecords();
}

/**
 * @tc.name: Execute_010
 * @tc.desc: Cover request-key, terminal-state, and cancellation-record validation branches.
 * @tc.type: RELI
 * @tc.require: NA
 */
HWTEST_F(PlaceholderTaskManagerTest, Execute_010, TestSize.Level2)
{
    auto &manager = PlaceholderTaskManager::GetInstance();
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, MakeRequest({})), E_INVALID_ARG);
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                              MakeRequest(std::vector<uint8_t>(MAX_CALLBACK_REQUEST_KEY_SIZE + 1, 1))),
              E_INVALID_ARG);

    PlaceholderTaskManager::RequestKey key;
    ASSERT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "file.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                        CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, OpenTaskFd(), key),
              E_OK);
    auto task = manager.taskMap_.at(key);
    task->state = PlaceholderTaskState::COMPLETED;
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, MakeRequest(key)), E_NO_HYDRATION_IN_PROGRESS);
    task->state = PlaceholderTaskState::CANCELLED;
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, MakeRequest(key)), E_CANCELLED);
    task->state = PlaceholderTaskState::PENDING;
    ASSERT_EQ(manager.CancelTask(TEST_SYNC_FOLDER, "file.txt", TEST_SYNC_FOLDER_INDEX), E_OK);

    auto request = MakeRequest(key);
    EXPECT_EQ(manager.Execute("com.example.other", TEST_SYNC_FOLDER_INDEX, request), E_CALLBACK_NOT_REGISTERED);
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX + 1, request), E_CALLBACK_NOT_REGISTERED);
    request.syncFolder += "-other";
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, request), E_INVALID_ARG);
    request = MakeRequest(key);
    request.filePath = "other.txt";
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, request), E_INVALID_ARG);
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, MakeRequest(key)), E_CANCELLED);
    manager.ClearCancellationRecords();
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, MakeRequest(key)), E_NO_HYDRATION_IN_PROGRESS);
}

/**
 * @tc.name: EnsurePartialStateLocked_001
 * @tc.desc: Cover cached, persisted, corrupt, missing, stale-old-state, and success branches.
 * @tc.type: RELI
 * @tc.require: NA
 */
HWTEST_F(PlaceholderTaskManagerTest, EnsurePartialStateLocked_001, TestSize.Level2)
{
    auto &manager = PlaceholderTaskManager::GetInstance();
    PlaceholderTaskManager::RequestKey key;
    ASSERT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "file.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                        CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, OpenTaskFd(), key),
              E_OK);
    auto task = manager.taskMap_.at(key);
    ActivateTask(manager, key);
    task->hasPartialState = true;
    EXPECT_CALL(*mock_, fgetxattr(_, _, _, _)).Times(0);
    EXPECT_EQ(manager.EnsurePartialStateLocked(task), E_OK);
    Mock::VerifyAndClearExpectations(mock_.get());

    task->hasPartialState = false;
    EXPECT_CALL(*mock_, fgetxattr(task->outputFd.Get(), _, _, _))
        .WillOnce(Invoke(ReturnPlaceholderState(PLACEHOLDER_STATE_PARTIALLY_HYDRATED)));
    EXPECT_CALL(*mock_, fsetxattr(_, _, _, _, _)).Times(0);
    EXPECT_EQ(manager.EnsurePartialStateLocked(task), E_OK);
    EXPECT_TRUE(task->hasPartialState);
    Mock::VerifyAndClearExpectations(mock_.get());

    task->hasPartialState = false;
    EXPECT_CALL(*mock_, fgetxattr(task->outputFd.Get(), _, _, _))
        .WillOnce(Invoke(ReturnPlaceholderState(PLACEHOLDER_STATE_FULLY_HYDRATED)));
    EXPECT_EQ(manager.EnsurePartialStateLocked(task), E_TRY_AGAIN);
    Mock::VerifyAndClearExpectations(mock_.get());

    EXPECT_CALL(*mock_, fgetxattr(task->outputFd.Get(), _, _, _))
        .WillOnce(Invoke(ReturnPlaceholderState(PLACEHOLDER_STATE_FULLY_HYDRATED + 1)));
    EXPECT_EQ(manager.EnsurePartialStateLocked(task), E_INVALID_PLACEHOLDER_STATE);
    Mock::VerifyAndClearExpectations(mock_.get());

    EXPECT_CALL(*mock_, fgetxattr(task->outputFd.Get(), _, _, _))
        .WillOnce(Invoke([](int, const char *, void *, size_t) {
            errno = ENOENT;
            return -1;
        }));
    EXPECT_EQ(manager.EnsurePartialStateLocked(task), E_FILE_NOT_EXIST);
    Mock::VerifyAndClearExpectations(mock_.get());
}

/**
 * @tc.name: EnsurePartialStateLocked_002
 * @tc.desc: Cover stale old-state validation and successful partial-state persistence.
 * @tc.type: RELI
 * @tc.require: NA
 */
HWTEST_F(PlaceholderTaskManagerTest, EnsurePartialStateLocked_002, TestSize.Level2)
{
    auto &manager = PlaceholderTaskManager::GetInstance();
    PlaceholderTaskManager::RequestKey key;
    ASSERT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "file.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                        CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, OpenTaskFd(), key),
              E_OK);
    auto task = manager.taskMap_.at(key);
    ActivateTask(manager, key);

    EXPECT_CALL(*mock_, fgetxattr(task->outputFd.Get(), _, _, _))
        .Times(2)
        .WillOnce(Invoke(ReturnPlaceholderState(PLACEHOLDER_STATE_UNHYDRATED)))
        .WillOnce(Invoke(ReturnPlaceholderState(PLACEHOLDER_STATE_FULLY_HYDRATED + 1)));
    EXPECT_CALL(*mock_, fsetxattr(task->outputFd.Get(), _, _, _, _)).WillOnce(Return(0));
    EXPECT_EQ(manager.EnsurePartialStateLocked(task), E_INVALID_PLACEHOLDER_STATE);
    Mock::VerifyAndClearExpectations(mock_.get());

    EXPECT_CALL(*mock_, fgetxattr(task->outputFd.Get(), _, _, _))
        .Times(2)
        .WillOnce(Invoke(ReturnPlaceholderState(PLACEHOLDER_STATE_UNHYDRATED)))
        .WillOnce(Invoke(ReturnPlaceholderState(PLACEHOLDER_STATE_NONE)));
    EXPECT_CALL(*mock_, fsetxattr(task->outputFd.Get(), _, _, _, _)).WillOnce(Return(0));
    EXPECT_EQ(manager.EnsurePartialStateLocked(task), E_TRY_AGAIN);
    Mock::VerifyAndClearExpectations(mock_.get());

    EXPECT_CALL(*mock_, fgetxattr(task->outputFd.Get(), _, _, _))
        .Times(2)
        .WillOnce(Invoke(ReturnPlaceholderState(PLACEHOLDER_STATE_UNHYDRATED)))
        .WillOnce(Invoke(ReturnPlaceholderState(PLACEHOLDER_STATE_PARTIALLY_HYDRATED)));
    EXPECT_CALL(*mock_, fsetxattr(task->outputFd.Get(), _, _, _, _)).WillOnce(Return(0));
    EXPECT_EQ(manager.EnsurePartialStateLocked(task), E_OK);
    EXPECT_TRUE(task->hasPartialState);
}

/**
 * @tc.name: SetCompleteStateLocked_001
 * @tc.desc: Reject read failures and invalid current or old states before accepting a valid transition.
 * @tc.type: RELI
 * @tc.require: NA
 */
HWTEST_F(PlaceholderTaskManagerTest, SetCompleteStateLocked_001, TestSize.Level2)
{
    auto &manager = PlaceholderTaskManager::GetInstance();
    PlaceholderTaskManager::RequestKey key;
    ASSERT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "file.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                        CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, OpenTaskFd(), key),
              E_OK);
    auto task = manager.taskMap_.at(key);
    ActivateTask(manager, key);
    const auto returnState = [](uint8_t state) {
        return [state](int, const char *, void *value, size_t size) {
            *static_cast<uint8_t *>(value) = MakeFileSyncState(state, 0);
            return static_cast<ssize_t>(size);
        };
    };

    EXPECT_CALL(*mock_, fgetxattr(task->outputFd.Get(), _, _, _))
        .WillOnce(Invoke([](int, const char *, void *, size_t) {
            errno = EINVAL;
            return -1;
        }));
    EXPECT_EQ(manager.SetCompleteStateLocked(task), E_INVALID_PLACEHOLDER_STATE);
    Mock::VerifyAndClearExpectations(mock_.get());

    EXPECT_CALL(*mock_, fgetxattr(task->outputFd.Get(), _, _, _))
        .WillOnce(Invoke(returnState(PLACEHOLDER_STATE_FULLY_HYDRATED + 1)));
    EXPECT_EQ(manager.SetCompleteStateLocked(task), E_INVALID_PLACEHOLDER_STATE);
    Mock::VerifyAndClearExpectations(mock_.get());

    EXPECT_CALL(*mock_, fgetxattr(task->outputFd.Get(), _, _, _))
        .WillOnce(Invoke(returnState(PLACEHOLDER_STATE_FULLY_HYDRATED)));
    EXPECT_EQ(manager.SetCompleteStateLocked(task), E_TRY_AGAIN);
    Mock::VerifyAndClearExpectations(mock_.get());

    EXPECT_CALL(*mock_, fgetxattr(task->outputFd.Get(), _, _, _))
        .Times(2)
        .WillOnce(Invoke(returnState(PLACEHOLDER_STATE_UNHYDRATED)))
        .WillOnce(Invoke(returnState(PLACEHOLDER_STATE_FULLY_HYDRATED + 1)));
    EXPECT_CALL(*mock_, fsetxattr(task->outputFd.Get(), _, _, _, _)).WillOnce(Return(0));
    EXPECT_EQ(manager.SetCompleteStateLocked(task), E_INVALID_PLACEHOLDER_STATE);
    Mock::VerifyAndClearExpectations(mock_.get());

    EXPECT_CALL(*mock_, fgetxattr(task->outputFd.Get(), _, _, _))
        .Times(2)
        .WillOnce(Invoke(returnState(PLACEHOLDER_STATE_UNHYDRATED)))
        .WillOnce(Invoke(returnState(PLACEHOLDER_STATE_NONE)));
    EXPECT_CALL(*mock_, fsetxattr(task->outputFd.Get(), _, _, _, _)).WillOnce(Return(0));
    EXPECT_EQ(manager.SetCompleteStateLocked(task), E_TRY_AGAIN);
    Mock::VerifyAndClearExpectations(mock_.get());

    EXPECT_CALL(*mock_, fgetxattr(task->outputFd.Get(), _, _, _))
        .Times(2)
        .WillOnce(Invoke(returnState(PLACEHOLDER_STATE_PARTIALLY_HYDRATED)))
        .WillOnce(Invoke(returnState(PLACEHOLDER_STATE_PARTIALLY_HYDRATED)));
    EXPECT_CALL(*mock_, fsetxattr(task->outputFd.Get(), _, _, _, _)).WillOnce(Return(0));
    EXPECT_EQ(manager.SetCompleteStateLocked(task), E_OK);
}

/**
 * @tc.name: Execute_011
 * @tc.desc: Keep a task retryable when flushing the completed hydration data fails.
 * @tc.type: RELI
 * @tc.require: NA
 */
HWTEST_F(PlaceholderTaskManagerTest, Execute_011, TestSize.Level2)
{
    auto &manager = PlaceholderTaskManager::GetInstance();
    PlaceholderTaskManager::RequestKey key;
    ASSERT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "file.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                        CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, OpenTaskFd(), key),
              E_OK);
    auto task = manager.taskMap_.at(key);
    int32_t taskFd = ActivateTask(manager, key);
    ASSERT_GE(taskFd, 0);
    task->hasPartialState = true;
    Assistant::mockPwriteApi = true;
    Assistant::mockFsyncApi = true;
    EXPECT_CALL(*mock_, Pwrite(taskFd, _, 1, 0)).WillOnce(Return(1));
    EXPECT_CALL(*mock_, Fsync(taskFd)).WillOnce(Invoke([](int) {
        errno = EIO;
        return -1;
    }));
    EXPECT_CALL(*mock_, fgetxattr(_, _, _, _)).Times(0);
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, MakeRequest(key, 0, {1}, 1, true)),
              E_TRY_AGAIN);
    EXPECT_EQ(task->state, PlaceholderTaskState::IN_PROGRESS);
}

/**
 * @tc.name: CancelTaskRecordLocked_001
 * @tc.desc: Avoid duplicate cancellation callbacks and ignore records that are already terminal.
 * @tc.type: RELI
 * @tc.require: NA
 */
HWTEST_F(PlaceholderTaskManagerTest, CancelTaskRecordLocked_001, TestSize.Level2)
{
    auto callback = sptr(new HydrationCallbackStub());
    ASSERT_EQ(PlaceholderCallbackManager::GetInstance().RegisterCallbackTable(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                                                              callback),
              E_OK);
    auto &manager = PlaceholderTaskManager::GetInstance();
    PlaceholderTaskManager::RequestKey cancelledKey;
    ASSERT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "file.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                        CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, OpenTaskFd(), cancelledKey),
              E_OK);
    auto cancelledTask = manager.taskMap_.at(cancelledKey);
    cancelledTask->cancelCallbackAttempted = true;
    manager.CancelTaskRecordLocked(cancelledTask, PlaceholderTaskCancelReason::USER_REQUEST);
    EXPECT_EQ(manager.taskMap_.count(cancelledKey), 0U);
    EXPECT_EQ(manager.cancellationRecordMap_.count(cancelledKey), 1U);
    EXPECT_TRUE(callback->callbackTypes.empty());

    PlaceholderTaskManager::RequestKey terminalKey;
    ASSERT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "terminal.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                        CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, OpenTaskFd(), terminalKey),
              E_OK);
    auto terminalTask = manager.taskMap_.at(terminalKey);
    terminalTask->state = PlaceholderTaskState::COMPLETED;
    manager.CancelTaskRecordLocked(terminalTask, PlaceholderTaskCancelReason::USER_REQUEST);
    EXPECT_EQ(manager.taskMap_.count(terminalKey), 1U);
    terminalTask->state = PlaceholderTaskState::PENDING;
    manager.CancelTaskRecordLocked(terminalTask, PlaceholderTaskCancelReason::SERVICE_STOP);
    EXPECT_EQ(manager.taskMap_.count(terminalKey), 0U);
    EXPECT_EQ(manager.cancellationRecordMap_.count(terminalKey), 0U);
    manager.ClearCancellationRecords();
}

/**
 * @tc.name: RefreshDeadlineLocked_001
 * @tc.desc: Refresh only live records and erase deadlines without removing a different record instance.
 * @tc.type: RELI
 * @tc.require: NA
 */
HWTEST_F(PlaceholderTaskManagerTest, RefreshDeadlineLocked_001, TestSize.Level2)
{
    auto &manager = PlaceholderTaskManager::GetInstance();
    PlaceholderTaskManager::RequestKey key;
    ASSERT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "file.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                        CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, OpenTaskFd(), key),
              E_OK);
    auto task = manager.taskMap_.at(key);
    manager.RefreshDeadlineLocked(task);
    ASSERT_EQ(manager.deadlineMap_.count(key), 1U);

    auto otherRecord = std::make_shared<PlaceholderTaskRecord>();
    otherRecord->reqKey = key;
    otherRecord->outputFd = OpenTaskFd();
    int32_t otherFd = otherRecord->outputFd.Get();
    ASSERT_GE(otherFd, 0);
    manager.EraseTaskLocked(otherRecord);
    EXPECT_EQ(fcntl(otherFd, F_GETFD), -1);
    EXPECT_EQ(manager.taskMap_.count(key), 1U);
    EXPECT_EQ(manager.deadlineMap_.count(key), 0U);

    manager.RefreshDeadlineLocked(task);
    EXPECT_EQ(manager.deadlineMap_.count(key), 1U);
    manager.EraseTaskLocked(task);
    EXPECT_EQ(manager.taskMap_.count(key), 0U);
    EXPECT_EQ(manager.deadlineMap_.count(key), 0U);
    manager.RefreshDeadlineLocked(task);
    EXPECT_EQ(manager.deadlineMap_.count(key), 0U);
}

/**
 * @tc.name: NotifyProgressLocked_001
 * @tc.desc: Publish one terminal progress event and suppress repeated terminal notifications.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(PlaceholderTaskManagerTest, NotifyProgressLocked_001, TestSize.Level1)
{
    auto observer = sptr(new TaskProgressRecorder());
    auto &progressManager = PlaceholderProgressManager::GetInstance();
    ASSERT_EQ(progressManager.Register({2, 20}, 100, observer), E_OK);
    auto &manager = PlaceholderTaskManager::GetInstance();
    PlaceholderTaskManager::RequestKey key;
    ASSERT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "file.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                        CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, OpenTaskFd(), key,
                                        {100, "/original/file.txt"}),
              E_OK);
    auto task = manager.taskMap_.at(key);
    task->state = PlaceholderTaskState::CANCELLED;
    manager.NotifyProgressLocked(task);
    manager.NotifyProgressLocked(task);
    progressManager.Drain();
    ASSERT_EQ(observer->events.size(), 2U);
    EXPECT_EQ(observer->events.front().state, static_cast<int32_t>(HydrateProgressState::PENDING));
    EXPECT_EQ(observer->events.back().state, static_cast<int32_t>(HydrateProgressState::CANCELLED));
    EXPECT_EQ(observer->events.back().filePath, "/original/file.txt");
    EXPECT_TRUE(task->terminalProgressSent);
    manager.EraseTaskLocked(task);
}

/**
 * @tc.name: DispatchLoop_001
 * @tc.desc: Releasing an application active slot dispatches its next queued task after the cancellation callback.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(PlaceholderTaskManagerTest, DispatchLoop_001, TestSize.Level1)
{
    auto callback = sptr(new HydrationCallbackStub());
    ASSERT_EQ(PlaceholderCallbackManager::GetInstance().RegisterCallbackTable(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                                                              callback),
              E_OK);
    auto &manager = PlaceholderTaskManager::GetInstance();
    std::vector<std::string> activePaths;
    activePaths.reserve(CLOUD_DISK_MAX_ACTIVE_TASKS_PER_APP);
    for (size_t index = 0; index < CLOUD_DISK_MAX_ACTIVE_TASKS_PER_APP; ++index) {
        std::string filePath = "active-" + std::to_string(index) + ".txt";
        PlaceholderTaskManager::RequestKey key;
        ASSERT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, filePath, TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                            CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, OpenTaskFd(), key),
                  E_OK);
        ActivateTask(manager, key);
        activePaths.push_back(std::move(filePath));
    }

    HydrationTestFile pendingFile;
    ASSERT_TRUE(pendingFile.IsValid());
    ExpectAdmissionAndActivationMetadata(TEST_INODE_ID, TEST_INODE_ID);
    PlaceholderTaskManager::RequestKey pendingKey;
    const std::string pendingPath = "pending.txt";
    ASSERT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, pendingPath, TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                        CLOUD_DISK_HYDRATE_PRIORITY_HIGH, pendingFile.DuplicateFd(), pendingKey,
                                        {-1, "", pendingFile.GetPath(), TEST_TEMP_ROOT}),
              E_OK);

    manager.StartScheduler();
    auto initialBarrier = manager.dispatchQueue_.submit_h([] {});
    manager.dispatchQueue_.wait(initialBarrier);
    PlaceholderTaskState state = PlaceholderTaskState::CANCELLED;
    ASSERT_TRUE(manager.GetTaskState(pendingKey, state));
    ASSERT_EQ(state, PlaceholderTaskState::PENDING);

    ASSERT_EQ(manager.CancelTask(TEST_SYNC_FOLDER, activePaths.front(), TEST_SYNC_FOLDER_INDEX), E_OK);
    auto dispatchBarrier = manager.dispatchQueue_.submit_h([] {});
    manager.dispatchQueue_.wait(dispatchBarrier);
    {
        std::lock_guard<std::mutex> lock(callback->mutex);
        ASSERT_GE(callback->callbackTypes.size(), 2U);
        EXPECT_EQ(callback->callbackTypes[0], CloudDiskCallbackType::CANCEL_FETCH_DATA);
        EXPECT_EQ(callback->callbackTypes[1], CloudDiskCallbackType::FETCH_DATA);
    }
    ASSERT_TRUE(manager.GetTaskState(pendingKey, state));
    EXPECT_EQ(state, PlaceholderTaskState::IN_PROGRESS);
}

/**
 * @tc.name: Execute_012
 * @tc.desc: A successful non-final Execute refreshes an already expired sliding deadline.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(PlaceholderTaskManagerTest, Execute_012, TestSize.Level1)
{
    auto &manager = PlaceholderTaskManager::GetInstance();
    PlaceholderTaskManager::RequestKey key;
    ASSERT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "file.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                        CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, OpenTaskFd(), key),
              E_OK);
    auto task = manager.taskMap_.at(key);
    ActivateTask(manager, key);
    task->hasPartialState = true;
    const auto expiredDeadline = std::chrono::steady_clock::now() - std::chrono::seconds(1);
    {
        std::lock_guard<std::mutex> lock(manager.mapMutex_);
        manager.deadlineMap_[key] = expiredDeadline;
    }

    Assistant::mockPwriteApi = true;
    EXPECT_CALL(*mock_, Pwrite(task->outputFd.Get(), _, 1, 0)).WillOnce(Return(1));
    const auto beforeExecute = std::chrono::steady_clock::now();
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, MakeRequest(key, 0, {1}, 2, false)), E_OK);

    std::lock_guard<std::mutex> lock(manager.mapMutex_);
    ASSERT_EQ(manager.deadlineMap_.count(key), 1U);
    EXPECT_GT(manager.deadlineMap_.at(key), expiredDeadline);
    EXPECT_GT(manager.deadlineMap_.at(key), beforeExecute);
    EXPECT_EQ(task->state, PlaceholderTaskState::IN_PROGRESS);
}
} // namespace OHOS::FileManagement::CloudDiskService::Test
