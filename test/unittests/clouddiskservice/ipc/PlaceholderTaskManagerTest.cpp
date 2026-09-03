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

#include <fcntl.h>
#include <gtest/gtest.h>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <limits>
#include <mutex>
#include <thread>

#include "assistant.h"
#include "cloud_disk_service_error.h"
#include "iremote_stub.h"
#include "placeholder_callback_manager.h"
#include "placeholder_helper.h"
#include "placeholder_task_manager.h"
#include "placeholder_progress_manager.h"

namespace OHOS::FileManagement::CloudDiskService::Test {
using namespace testing;
using namespace testing::ext;

namespace {
const std::string TEST_SYNC_FOLDER = "/storage/Users/currentUser/sync";
const std::string TEST_BUNDLE_NAME = "com.example.cloud.disk";
constexpr uint32_t TEST_SYNC_FOLDER_INDEX = 100;

UniqueFd OpenTaskFd()
{
    return UniqueFd(dup(STDOUT_FILENO));
}

CallbackExecuteRequest MakeRequest(const std::vector<uint8_t> &reqKey, uint64_t offset = 0,
    const std::vector<uint8_t> &data = {}, uint64_t totalSize = 0, bool isComplete = true)
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

class TaskProgressRecorder final : public IRemoteStub<ICloudDiskProgressCallback> {
public:
    void OnProgress(const HydrateProgress &progress) override
    {
        events.push_back(progress);
    }
    int32_t OnRemoteRequest(uint32_t, MessageParcel &, MessageParcel &, MessageOption &) override { return E_OK; }
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
        called = reqHead.callbackType == CloudDiskCallbackType::FETCH_DATA && reqContext.fetchData != nullptr;
        if (called) {
            priority = reqContext.fetchData->priority;
        }
        cv.notify_one();
    }

    int32_t OnRemoteRequest(uint32_t, MessageParcel &, MessageParcel &, MessageOption &) override
    {
        return E_OK;
    }

    std::mutex mutex;
    std::condition_variable cv;
    bool called = false;
    CloudDiskHydratePriority priority = CLOUD_DISK_HYDRATE_PRIORITY_LOW;
};
} // namespace

class PlaceholderTaskManagerTest : public testing::Test {
public:
    void SetUp() override
    {
        PlaceholderTaskManager::GetInstance().StopWorkerPool();
        mock_ = std::make_shared<AssistantMock>();
        Assistant::ins = mock_;
        Assistant::mockFdApi = false;
        Assistant::mockPwriteApi = false;
    }

    void TearDown() override
    {
        PlaceholderTaskManager::GetInstance().StopWorkerPool();
        PlaceholderProgressManager::GetInstance().Drain();
        PlaceholderProgressManager::GetInstance().Clear();
        PlaceholderCallbackManager::GetInstance().ClearBySyncFolder(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX);
        Assistant::ins = nullptr;
        Assistant::mockPwriteApi = false;
        mock_ = nullptr;
    }

    void ExpectStateTransitions(int32_t taskFd, uint8_t &stateByte)
    {
        constexpr int32_t STATE_UPDATE_COUNT = 2; // Partially hydrated, then fully hydrated.
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
    ASSERT_FALSE(manager.pendingQueue_.empty());
    EXPECT_EQ(manager.pendingQueue_.top().reqKey, highKey);
    manager.pendingQueue_.pop();
    EXPECT_EQ(manager.pendingQueue_.top().reqKey, secondHighKey);
    EXPECT_TRUE(manager.HasActiveTask(TEST_SYNC_FOLDER, "low.txt", TEST_SYNC_FOLDER_INDEX));
    EXPECT_FALSE(manager.HasActiveTask(TEST_SYNC_FOLDER, "low.txt", TEST_SYNC_FOLDER_INDEX + 1));
}

/**
 * @tc.name: CancelTask_001
 * @tc.desc: Verify cancellation marks the task and Execute observes the cancelled state.
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
    ASSERT_TRUE(manager.GetTaskState(reqKey, state));
    EXPECT_EQ(state, PlaceholderTaskState::CANCELLED);
    EXPECT_FALSE(manager.HasActiveTask(TEST_SYNC_FOLDER, "file.txt", TEST_SYNC_FOLDER_INDEX));
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, MakeRequest(reqKey)), E_CANCELLED);
    EXPECT_EQ(manager.CancelTask(TEST_SYNC_FOLDER, "missing.txt", TEST_SYNC_FOLDER_INDEX),
              E_NO_HYDRATION_IN_PROGRESS);
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
    EXPECT_EQ(manager.Execute("other.bundle", TEST_SYNC_FOLDER_INDEX, MakeRequest(reqKey)),
              E_CALLBACK_NOT_REGISTERED);
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
    std::unique_ptr<FILE, decltype(&fclose)> file(tmpfile(), fclose);
    ASSERT_NE(file.get(), nullptr);
    UniqueFd outputFd(dup(fileno(file.get())));
    ASSERT_GE(outputFd.Get(), 0);
    int32_t taskFd = outputFd.Get();
    auto &manager = PlaceholderTaskManager::GetInstance();
    PlaceholderTaskManager::RequestKey reqKey;
    ASSERT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "file.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                        CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, std::move(outputFd), reqKey, {100, ""}),
              E_OK);
    manager.running_ = true;
    ASSERT_NE(manager.GetNextTask(), nullptr);

    uint8_t stateByte = MakeFileSyncState(PLACEHOLDER_STATE_UNHYDRATED, 1);
    ExpectStateTransitions(taskFd, stateByte);
    std::vector<uint8_t> firstBlock{1, 2, 3};
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
        MakeRequest(reqKey, 0, firstBlock, 6, false)), E_OK);
    EXPECT_EQ(GetPlaceholderStateFromFileSyncState(stateByte), PLACEHOLDER_STATE_PARTIALLY_HYDRATED);
    EXPECT_EQ(GetSyncStateFromFileSyncState(stateByte), 1);
    std::vector<uint8_t> finalBlock{4, 5, 6};
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
        MakeRequest(reqKey, 3, finalBlock, 6, true)), E_OK);
    EXPECT_EQ(GetPlaceholderStateFromFileSyncState(stateByte), PLACEHOLDER_STATE_FULLY_HYDRATED);
    EXPECT_EQ(GetSyncStateFromFileSyncState(stateByte), 1);
    PlaceholderTaskState taskState;
    EXPECT_FALSE(manager.GetTaskState(reqKey, taskState));
    EXPECT_EQ(fcntl(taskFd, F_GETFD), -1);
    std::vector<uint8_t> actual(6);
    ASSERT_EQ(pread(fileno(file.get()), actual.data(), actual.size(), 0), static_cast<ssize_t>(actual.size()));
    EXPECT_EQ(actual, (std::vector<uint8_t>{1, 2, 3, 4, 5, 6}));
    progressManager.Drain();
    CheckCompletedProgress(observer);
}
/**
 * @tc.name: StartWorkerPool_001
 * @tc.desc: Workers dispatch FETCH_DATA asynchronously and release incomplete tasks after callback return.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(PlaceholderTaskManagerTest, StartWorkerPool_001, TestSize.Level1)
{
    auto callback = sptr(new HydrationCallbackStub());
    ASSERT_EQ(PlaceholderCallbackManager::GetInstance().RegisterCallbackTable(
        TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, callback), E_OK);
    auto &manager = PlaceholderTaskManager::GetInstance();
    UniqueFd outputFd = OpenTaskFd();
    ASSERT_GE(outputFd.Get(), 0);
    int32_t taskFd = outputFd.Get();
    PlaceholderTaskManager::RequestKey reqKey;
    ASSERT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "file.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
        CLOUD_DISK_HYDRATE_PRIORITY_HIGH, std::move(outputFd), reqKey), E_OK);
    manager.StartWorkerPool();
    bool called = false;
    {
        std::unique_lock<std::mutex> lock(callback->mutex);
        called = callback->cv.wait_for(lock, std::chrono::seconds(2), [&callback] { return callback->called; });
    }
    manager.StopWorkerPool();
    EXPECT_TRUE(called);
    EXPECT_EQ(callback->priority, CLOUD_DISK_HYDRATE_PRIORITY_HIGH);
    PlaceholderTaskState state;
    EXPECT_FALSE(manager.GetTaskState(reqKey, state));
    EXPECT_EQ(fcntl(taskFd, F_GETFD), -1);
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
        CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, OpenTaskFd(), key), E_OK);
    auto task = manager.taskMap_.at(key);
    task->state = PlaceholderTaskState::IN_PROGRESS;
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
    request = MakeRequest(key);
    request.filePath = "other.txt";
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, request), E_INVALID_ARG);
    request = MakeRequest(key);
    request.syncFolder += "-other";
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, request), E_INVALID_ARG);
    request = MakeRequest(key);
    request.callbackType = static_cast<int32_t>(CloudDiskCallbackType::FETCH_RANGE_DATA);
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, request), E_INVALID_ARG);
    task->totalSize = 1;
    task->totalSizeInitialized = true;
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, MakeRequest(key)), E_INVALID_ARG);
    task->state = PlaceholderTaskState::COMPLETED;
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, MakeRequest(key)), E_ALREADY_HYDRATED);
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
    int taskFd = fd.Get();
    PlaceholderTaskManager::RequestKey key;
    ASSERT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "file.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
        CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, std::move(fd), key), E_OK);
    manager.taskMap_.at(key)->state = PlaceholderTaskState::IN_PROGRESS;
    auto request = MakeRequest(key);
    request.callbackType = static_cast<int32_t>(CloudDiskCallbackType::CANCEL_FETCH_DATA);
    EXPECT_CALL(*mock_, fsetxattr(_, _, _, _, _)).Times(0);
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, request), E_OK);
    EXPECT_EQ(fcntl(taskFd, F_GETFD), -1);
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, request), E_NO_HYDRATION_IN_PROGRESS);
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
        CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, OpenTaskFd(), key), E_OK);
    auto task = manager.taskMap_.at(key);
    task->state = PlaceholderTaskState::IN_PROGRESS;
    task->hasPartialState = true;
    Assistant::mockPwriteApi = true;
    auto request = MakeRequest(key, 0, {1, 2, 3}, 3, false);
    const std::vector<std::pair<int, int32_t>> errors{{ENOSPC, E_NO_SPACE_LEFT}, {EIO, E_TRY_AGAIN}};
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
            .WillOnce(Invoke([](int, const void *, size_t, off_t) { errno = EINTR; return -1; }));
        EXPECT_CALL(*mock_, Pwrite(task->outputFd.Get(), _, 3, 0)).WillOnce(Return(1));
        EXPECT_CALL(*mock_, Pwrite(task->outputFd.Get(), _, 2, 1)).WillOnce(Return(2));
    }
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, request), E_OK);
    EXPECT_EQ(task->state.load(), PlaceholderTaskState::IN_PROGRESS);
}

/**
 * @tc.name: Execute_006
 * @tc.desc: State persistence failure cancels the task and closes its fd after a successful data write.
 * @tc.type: RELI
 * @tc.require: NA
 */
HWTEST_F(PlaceholderTaskManagerTest, Execute_006, TestSize.Level2)
{
    auto &manager = PlaceholderTaskManager::GetInstance();
    UniqueFd fd = OpenTaskFd();
    ASSERT_GE(fd.Get(), 0);
    int taskFd = fd.Get();
    PlaceholderTaskManager::RequestKey key;
    ASSERT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "file.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
        CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, std::move(fd), key), E_OK);
    manager.taskMap_.at(key)->state = PlaceholderTaskState::IN_PROGRESS;
    Assistant::mockPwriteApi = true;
    EXPECT_CALL(*mock_, Pwrite(taskFd, _, 1, 0)).WillOnce(Return(1));
    EXPECT_CALL(*mock_, fgetxattr(taskFd, _, _, _))
        .WillOnce(Invoke([](int, const char *, void *, size_t) { errno = EIO; return -1; }));
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
        MakeRequest(key, 0, {1}, 1, false)), E_TRY_AGAIN);
    EXPECT_EQ(fcntl(taskFd, F_GETFD), -1);
    PlaceholderTaskState state;
    EXPECT_FALSE(manager.GetTaskState(key, state));
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
        CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, OpenTaskFd(), firstKey), E_OK);
    ASSERT_EQ(manager.CreateHydrateTask(TEST_SYNC_FOLDER, "second.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
        CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, OpenTaskFd(), secondKey), E_OK);
    auto firstTask = manager.taskMap_.at(firstKey);
    auto secondTask = manager.taskMap_.at(secondKey);
    firstTask->state = secondTask->state = PlaceholderTaskState::IN_PROGRESS;
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
        CLOUD_DISK_HYDRATE_PRIORITY_HIGH, OpenTaskFd(), key, {100, "/original/file.txt"}), E_OK);
    auto task = manager.taskMap_.at(key);
    EXPECT_EQ(task->absolutePath, "/original/file.txt");
    EXPECT_EQ(task->userId, 100);
    EXPECT_EQ(task->totalSize, 20u);
    EXPECT_EQ(task->cachedSize, 0u);
    manager.running_ = true;
    EXPECT_EQ(manager.GetNextTask(), task);
    task->hasPartialState = true;
    Assistant::mockPwriteApi = true;
    EXPECT_CALL(*mock_, Pwrite(_, _, 2, 0)).WillOnce(Return(2));
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
        MakeRequest(key, 0, {1, 2}, 5, false)), E_OK);
    EXPECT_EQ(task->totalSize, 5u);
    EXPECT_EQ(task->cachedSize, 2u);
    EXPECT_CALL(*mock_, Pwrite(_, _, 3, 2)).WillOnce(Return(0));
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
        MakeRequest(key, 2, {3, 4, 5}, 5, false)), E_TRY_AGAIN);
    EXPECT_EQ(task->cachedSize, 2u);
    EXPECT_CALL(*mock_, Pwrite(_, _, 3, 2)).WillOnce(Return(3));
    EXPECT_EQ(manager.Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
        MakeRequest(key, 2, {3, 4, 5}, 5, false)), E_OK);
    EXPECT_EQ(task->cachedSize, 5u);
    EXPECT_EQ(manager.CancelTask(TEST_SYNC_FOLDER, "file.txt", TEST_SYNC_FOLDER_INDEX), E_OK);
    EXPECT_TRUE(task->terminalProgressSent);
    manager.FinishDispatch(key);
    EXPECT_FALSE(manager.HasActiveTask(TEST_SYNC_FOLDER, "file.txt", TEST_SYNC_FOLDER_INDEX));
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
        CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, std::move(fd), key), E_TRY_AGAIN);
    EXPECT_TRUE(manager.taskMap_.empty());
    EXPECT_TRUE(manager.pendingQueue_.empty());
    EXPECT_EQ(fcntl(rawFd, F_GETFD), -1);
}
} // namespace OHOS::FileManagement::CloudDiskService::Test
