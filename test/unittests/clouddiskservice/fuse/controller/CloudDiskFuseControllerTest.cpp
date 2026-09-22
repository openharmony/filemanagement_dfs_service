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

#include <gtest/gtest.h>

#include <atomic>
#include <cerrno>
#include <chrono>
#include <climits>
#include <condition_variable>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <functional>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <sys/eventfd.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <thread>
#include <unistd.h>
#include <unordered_map>
#include <utility>
#include <vector>

#define private public
#define protected public
#include "cloud_disk_fuse_controller.h"
#undef protected
#undef private

#include "CloudDiskFuseControllerMock.h"
#include "ffrt_inner.h"

using namespace testing::ext;

namespace OHOS::FileManagement::CloudDiskService::Test {
namespace {

constexpr int32_t TEST_USER_ID = 100;
constexpr uint32_t TEST_ROOT_INDEX = 7;
constexpr uint64_t TEST_ROOT_EPOCH = 11;
constexpr fuse_ino_t TEST_NODE_ID = FUSE_ROOT_ID + 1;
constexpr dev_t TEST_DEVICE = 21;
constexpr ino_t TEST_INODE = 34;
constexpr mode_t TEST_LOOKUP_FILE_MODE = 0600;
constexpr int32_t E_OK = 0;

class CloudDiskFuseControllerTest : public testing::Test {
public:
    void SetUp() override
    {
        ffrt::wait();
        CloudDiskFuseControllerMock::Reset();
        controller_ = std::make_unique<CloudDiskFuseController>();
    }

    void TearDown() override
    {
        ffrt::wait();
        if (controller_ != nullptr) {
            controller_->started_ = false;
            controller_->shutdown_ = false;
            controller_->session_ = nullptr;
            controller_->wakeFd_ = -1;
            controller_->pendingWarmupCount_ = 0;
        }
        controller_.reset();
        for (const auto &path : filesToRemove_) {
            (void)unlink(path.c_str());
        }
        for (auto it = directoriesToRemove_.rbegin(); it != directoriesToRemove_.rend(); ++it) {
            (void)rmdir(it->c_str());
        }
        CloudDiskFuseControllerMock::Reset();
    }

protected:
    CloudDiskFuseController::RootContext
        AddRoot(uint32_t index = TEST_ROOT_INDEX,
                CloudDiskFuseController::RootState state = CloudDiskFuseController::RootState::READY,
                uint64_t epoch = TEST_ROOT_EPOCH)
    {
        CloudDiskFuseController::RootContext root;
        root.syncFolderIndex = index;
        root.physicalPath = "/physical/root";
        root.mountPath = "/mnt/mock/root";
        root.warmupEpoch = epoch;
        root.state = state;
        controller_->roots_[index] = root;
        return root;
    }

    CloudDiskFuseController::NodeContext AddNode(fuse_ino_t nodeId = TEST_NODE_ID,
                                                 uint32_t index = TEST_ROOT_INDEX,
                                                 uint64_t epoch = TEST_ROOT_EPOCH,
                                                 dev_t device = TEST_DEVICE,
                                                 ino_t inode = TEST_INODE)
    {
        CloudDiskFuseController::NodeContext node;
        node.nodeId = nodeId;
        node.syncFolderIndex = index;
        node.path = "/mock/file";
        node.device = device;
        node.inode = inode;
        node.rootEpoch = epoch;
        node.nlookup = 1;
        controller_->nodes_[nodeId] = node;
        controller_->identityToNode_[{static_cast<uint64_t>(device), static_cast<uint64_t>(inode)}] = nodeId;
        return node;
    }

    CloudDiskFuseController::LookupResult MakeLookupResult() const
    {
        CloudDiskFuseController::LookupResult result;
        result.syncFolderIndex = TEST_ROOT_INDEX;
        result.rootEpoch = TEST_ROOT_EPOCH;
        result.path = "/mock/file";
        result.attr.st_dev = TEST_DEVICE;
        result.attr.st_ino = TEST_INODE;
        result.attr.st_mode = S_IFREG | TEST_LOOKUP_FILE_MODE;
        return result;
    }

    void SetupWarmupHook(std::mutex &hookMutex, std::condition_variable &hookCondition,
                         uint32_t &lookupCalls, bool &resumeLookup)
    {
        auto &mock = CloudDiskFuseControllerMock::GetState();
        mock.metaFileLookupHook = [&hookMutex, &hookCondition, &lookupCalls, &resumeLookup] {
            std::unique_lock<std::mutex> lock(hookMutex);
            ++lookupCalls;
            hookCondition.notify_all();
            if (lookupCalls == 1) {
                hookCondition.wait(lock, [&resumeLookup] { return resumeLookup; });
            }
        };
    }

    void ConfigureRootBehavior(uint32_t rootIndex)
    {
        auto &behavior = CloudDiskFuseControllerMock::GetState().metaBehaviors[rootIndex];
        behavior.exists = true;
        behavior.lookupResult = E_OK;
        behavior.relativePathResult = E_OK;
        behavior.placeholder = 1;
        behavior.mode = S_IFREG | TEST_LOOKUP_FILE_MODE;
    }

    void ResumeRootWarmup(uint32_t rootIndex, bool firstScanObserved)
    {
        std::lock_guard<std::mutex> lock(controller_->mutex_);
        if (firstScanObserved) {
            controller_->roots_[rootIndex].state = CloudDiskFuseController::RootState::READY;
            controller_->roots_[rootIndex].warmupInFlight = false;
        } else {
            controller_->shutdown_ = true;
            controller_->state_ = CloudDiskFuseController::State::IDLE;
        }
    }

    std::string CreateTempDirectory()
    {
        const std::vector<std::string> patterns = {
            "/data/local/tmp/CloudDiskFuseControllerTestXXXXXX",
            "/data/test/CloudDiskFuseControllerTestXXXXXX",
            "/tmp/CloudDiskFuseControllerTestXXXXXX",
        };
        for (const auto &pattern : patterns) {
            std::vector<char> buffer(pattern.begin(), pattern.end());
            buffer.push_back('\0');
            char *path = mkdtemp(buffer.data());
            if (path != nullptr) {
                directoriesToRemove_.emplace_back(path);
                return path;
            }
        }
        return {};
    }

    std::string CreateTempFile(const std::string &directory, const std::string &name, const std::string &content)
    {
        std::string path = directory + "/" + name;
        int fd = open(path.c_str(), O_CREAT | O_EXCL | O_RDWR | O_CLOEXEC, 0600);
        if (fd < 0) {
            return {};
        }
        ssize_t written = write(fd, content.data(), content.size());
        (void)close(fd);
        if (written != static_cast<ssize_t>(content.size())) {
            (void)unlink(path.c_str());
            return {};
        }
        filesToRemove_.push_back(path);
        return path;
    }

    std::unique_ptr<CloudDiskFuseController> controller_;
    std::vector<std::string> filesToRemove_;
    std::vector<std::string> directoriesToRemove_;
};

/*
 * @tc.name: BuildRootContext_001
 * @tc.desc: Verify successful and failed physical-path conversion branches
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseControllerTest, BuildRootContext_001, TestSize.Level1)
{
    auto &mock = CloudDiskFuseControllerMock::GetState();
    CloudDiskFuseController::RootContext root;
    EXPECT_TRUE(controller_->BuildRootContext(TEST_USER_ID, TEST_ROOT_INDEX, "/physical/root", root));
    EXPECT_EQ(root.syncFolderIndex, TEST_ROOT_INDEX);
    EXPECT_EQ(root.mountPath, mock.convertedPath);

    mock.convertResult = false;
    EXPECT_FALSE(controller_->BuildRootContext(TEST_USER_ID, TEST_ROOT_INDEX, "/invalid", root));
    EXPECT_EQ(mock.convertCalls, 2U);
}

/*
 * @tc.name: BeginRootWarmup_001
 * @tc.desc: Verify current and stale warmup requests
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseControllerTest, BeginRootWarmup_001, TestSize.Level1)
{
    controller_->activeUserId_ = TEST_USER_ID;
    auto root = AddRoot(TEST_ROOT_INDEX, CloudDiskFuseController::RootState::WARMING);
    EXPECT_TRUE(controller_->BeginRootWarmup(TEST_USER_ID, root));
    EXPECT_EQ(controller_->roots_[TEST_ROOT_INDEX].warmupTaskCount, 1U);
    EXPECT_EQ(controller_->pendingWarmupCount_, 1U);

    root.warmupEpoch++;
    EXPECT_FALSE(controller_->BeginRootWarmup(TEST_USER_ID, root));
    controller_->shutdown_ = true;
    root.warmupEpoch--;
    EXPECT_FALSE(controller_->BeginRootWarmup(TEST_USER_ID, root));
}

/*
 * @tc.name: FinishRootWarmup_001
 * @tc.desc: Verify warmup success and failure state transitions
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseControllerTest, FinishRootWarmup_001, TestSize.Level1)
{
    controller_->activeUserId_ = TEST_USER_ID;
    auto root = AddRoot(TEST_ROOT_INDEX, CloudDiskFuseController::RootState::WARMING);
    controller_->roots_[TEST_ROOT_INDEX].warmupTaskCount = 1;
    controller_->roots_[TEST_ROOT_INDEX].warmupInFlight = true;
    controller_->pendingWarmupCount_ = 1;
    controller_->FinishRootWarmup(TEST_USER_ID, root, E_OK);
    EXPECT_EQ(controller_->roots_[TEST_ROOT_INDEX].state, CloudDiskFuseController::RootState::READY);
    EXPECT_FALSE(controller_->roots_[TEST_ROOT_INDEX].warmupInFlight);
    EXPECT_EQ(controller_->pendingWarmupCount_, 0U);

    controller_->roots_[TEST_ROOT_INDEX].state = CloudDiskFuseController::RootState::WARMING;
    controller_->roots_[TEST_ROOT_INDEX].warmupTaskCount = 1;
    controller_->pendingWarmupCount_ = 1;
    controller_->FinishRootWarmup(TEST_USER_ID, root, EIO);
    EXPECT_EQ(controller_->roots_[TEST_ROOT_INDEX].state, CloudDiskFuseController::RootState::WARMING);
    EXPECT_EQ(controller_->roots_[TEST_ROOT_INDEX].lastWarmupError, EIO);
}

/*
 * @tc.name: RunRootWarmup_001
 * @tc.desc: Verify a single warmup invocation is committed to the current root
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseControllerTest, RunRootWarmup_001, TestSize.Level1)
{
    controller_->activeUserId_ = TEST_USER_ID;
    auto root = AddRoot(TEST_ROOT_INDEX, CloudDiskFuseController::RootState::WARMING);
    controller_->roots_[TEST_ROOT_INDEX].warmupTaskCount = 1;
    controller_->roots_[TEST_ROOT_INDEX].warmupInFlight = true;
    controller_->pendingWarmupCount_ = 1;
    controller_->RunRootWarmup(TEST_USER_ID, root);
    EXPECT_EQ(CloudDiskFuseControllerMock::GetState().warmupCalls, 1U);
    EXPECT_FALSE(CloudDiskFuseControllerMock::GetState().warmupCancelValue);
    EXPECT_EQ(controller_->roots_[TEST_ROOT_INDEX].state, CloudDiskFuseController::RootState::READY);
}

/*
 * @tc.name: AddRoot_001
 * @tc.desc: Verify inactive and conversion-failure roots use the fallback scheduler
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseControllerTest, AddRoot_001, TestSize.Level2)
{
    controller_->AddRoot(TEST_USER_ID, TEST_ROOT_INDEX, "/physical/root");
    EXPECT_EQ(CloudDiskFuseControllerMock::GetState().scheduleFillCalls, 1U);
    EXPECT_TRUE(controller_->roots_.empty());

    CloudDiskFuseControllerMock::GetState().convertResult = false;
    controller_->AddRoot(TEST_USER_ID, TEST_ROOT_INDEX, "/invalid");
    EXPECT_EQ(CloudDiskFuseControllerMock::GetState().scheduleFillCalls, 2U);
}

/*
 * @tc.name: AddRoot_002
 * @tc.desc: Verify active root insertion and replacement each schedule one warmup
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseControllerTest, AddRoot_002, TestSize.Level1)
{
    auto &mock = CloudDiskFuseControllerMock::GetState();
    controller_->started_ = true;
    controller_->activeUserId_ = TEST_USER_ID;
    controller_->rootWarmupEpoch_ = TEST_ROOT_EPOCH;

    controller_->AddRoot(TEST_USER_ID, TEST_ROOT_INDEX, "/physical/root");
    ffrt::wait();

    ASSERT_EQ(controller_->roots_.size(), 1U);
    EXPECT_EQ(mock.warmupCalls, 1U);
    EXPECT_EQ(controller_->pendingWarmupCount_, 0U);
    EXPECT_EQ(controller_->roots_[TEST_ROOT_INDEX].warmupEpoch, TEST_ROOT_EPOCH + 1);
    EXPECT_EQ(controller_->roots_[TEST_ROOT_INDEX].state, CloudDiskFuseController::RootState::READY);
    EXPECT_EQ(controller_->roots_[TEST_ROOT_INDEX].warmupTaskCount, 0U);
    EXPECT_FALSE(controller_->roots_[TEST_ROOT_INDEX].warmupInFlight);

    const uint64_t previousEpoch = controller_->roots_[TEST_ROOT_INDEX].warmupEpoch;
    controller_->roots_[TEST_ROOT_INDEX].warmupTaskCount = 2;
    mock.warmupCalls = 0;
    controller_->AddRoot(TEST_USER_ID, TEST_ROOT_INDEX, "/physical/replacement");
    ffrt::wait();

    EXPECT_EQ(mock.warmupCalls, 1U);
    EXPECT_EQ(controller_->pendingWarmupCount_, 0U);
    EXPECT_EQ(controller_->roots_[TEST_ROOT_INDEX].warmupEpoch, previousEpoch + 1);
    EXPECT_EQ(controller_->roots_[TEST_ROOT_INDEX].state, CloudDiskFuseController::RootState::READY);
    EXPECT_EQ(controller_->roots_[TEST_ROOT_INDEX].warmupTaskCount, 2U);
    EXPECT_TRUE(controller_->roots_[TEST_ROOT_INDEX].warmupInFlight);
}

/*
 * @tc.name: RemoveRoot_001
 * @tc.desc: Verify root removal guards and retiring transition
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseControllerTest, RemoveRoot_001, TestSize.Level1)
{
    controller_->activeUserId_ = TEST_USER_ID;
    AddRoot();
    controller_->RemoveRoot(TEST_USER_ID + 1, TEST_ROOT_INDEX);
    EXPECT_EQ(controller_->roots_[TEST_ROOT_INDEX].state, CloudDiskFuseController::RootState::READY);
    controller_->RemoveRoot(TEST_USER_ID, TEST_ROOT_INDEX + 1);
    EXPECT_EQ(controller_->roots_.size(), 1U);

    controller_->RemoveRoot(TEST_USER_ID, TEST_ROOT_INDEX);
    EXPECT_EQ(controller_->roots_[TEST_ROOT_INDEX].state, CloudDiskFuseController::RootState::RETIRING);
    uint64_t epoch = controller_->rootWarmupEpoch_;
    controller_->RemoveRoot(TEST_USER_ID, TEST_ROOT_INDEX);
    EXPECT_EQ(controller_->rootWarmupEpoch_, epoch);
}

/*
 * @tc.name: StopLocked_001
 * @tc.desc: Verify pending unmount preservation and service-exit discard modes
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseControllerTest, StopLocked_001, TestSize.Level1)
{
    controller_->pendingUnmounts_.push_back({TEST_USER_ID, "/mnt/pending"});
    controller_->StopLocked(CloudDiskFuseController::StopMode::PRESERVE_PENDING_UNMOUNTS);
    EXPECT_EQ(controller_->pendingUnmounts_.size(), 1U);

    controller_->StopLocked(CloudDiskFuseController::StopMode::DISCARD_PENDING_UNMOUNTS);
    EXPECT_TRUE(controller_->pendingUnmounts_.empty());
    EXPECT_EQ(controller_->state_, CloudDiskFuseController::State::IDLE);
}

/*
 * @tc.name: StateHelpers_001
 * @tc.desc: Verify idle, active-root, current-warmup and worker-stop predicates
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseControllerTest, StateHelpers_001, TestSize.Level1)
{
    controller_->SetIdleCallback([] {});
    EXPECT_TRUE(static_cast<bool>(controller_->idleCallback_));
    EXPECT_TRUE(controller_->CanUnload());
    EXPECT_FALSE(controller_->ShouldStopWorker());
    controller_->shutdown_ = true;
    EXPECT_TRUE(controller_->ShouldStopWorker());
    controller_->shutdown_ = false;
    controller_->activeUserId_ = TEST_USER_ID;
    auto root = AddRoot(TEST_ROOT_INDEX, CloudDiskFuseController::RootState::WARMING);
    EXPECT_TRUE(controller_->HasActiveRootLocked());
    EXPECT_TRUE(controller_->IsWarmupCurrentLocked(TEST_USER_ID, TEST_ROOT_INDEX, root.warmupEpoch));
    EXPECT_FALSE(controller_->IsWarmupCurrentLocked(TEST_USER_ID + 1, TEST_ROOT_INDEX, root.warmupEpoch));
    EXPECT_FALSE(controller_->CanUnload());
    controller_->roots_[TEST_ROOT_INDEX].state = CloudDiskFuseController::RootState::RETIRING;
    EXPECT_FALSE(controller_->HasActiveRootLocked());
    EXPECT_EQ(controller_->GetMountPoint(TEST_USER_ID), "/mnt/data/100/cloud_disk_fuse/clouddiskservice");
}

/*
 * @tc.name: GetRetireDeadlineLocked_001
 * @tc.desc: Verify only quiescent retiring roots contribute a deadline
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseControllerTest, GetRetireDeadlineLocked_001, TestSize.Level1)
{
    EXPECT_EQ(controller_->GetRetireDeadlineLocked(), std::chrono::steady_clock::time_point::max());
    AddRoot(TEST_ROOT_INDEX, CloudDiskFuseController::RootState::RETIRING);
    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
    controller_->roots_[TEST_ROOT_INDEX].retireDeadline = deadline;
    EXPECT_EQ(controller_->GetRetireDeadlineLocked(), deadline);
    controller_->roots_[TEST_ROOT_INDEX].warmupTaskCount = 1;
    EXPECT_EQ(controller_->GetRetireDeadlineLocked(), std::chrono::steady_clock::time_point::max());
}

/*
 * @tc.name: FinalizeRetiredRoots_001
 * @tc.desc: Verify expired roots are finalized into idle or draining state
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseControllerTest, FinalizeRetiredRoots_001, TestSize.Level1)
{
    AddRoot(TEST_ROOT_INDEX, CloudDiskFuseController::RootState::RETIRING);
    controller_->roots_[TEST_ROOT_INDEX].retireDeadline = std::chrono::steady_clock::now();
    controller_->FinalizeRetiredRoots();
    EXPECT_TRUE(controller_->roots_.empty());
    EXPECT_EQ(controller_->state_, CloudDiskFuseController::State::IDLE);

    AddRoot(TEST_ROOT_INDEX, CloudDiskFuseController::RootState::RETIRING);
    controller_->roots_[TEST_ROOT_INDEX].retireDeadline = std::chrono::steady_clock::now();
    controller_->session_ = reinterpret_cast<struct fuse_session *>(1);
    controller_->FinalizeRetiredRoots();
    EXPECT_EQ(controller_->state_, CloudDiskFuseController::State::DRAINING);
    controller_->session_ = nullptr;
}

/*
 * @tc.name: SelectPendingUnmount_001
 * @tc.desc: Verify orphan selection and conflicting lifecycle guards
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseControllerTest, SelectPendingUnmount_001, TestSize.Level1)
{
    CloudDiskFuseController::UnmountTarget target;
    EXPECT_FALSE(controller_->SelectPendingUnmount(target));
    controller_->pendingUnmounts_.push_back({TEST_USER_ID, "/mnt/old"});
    EXPECT_TRUE(controller_->SelectPendingUnmount(target));
    EXPECT_EQ(target.mountPoint, "/mnt/old");
    EXPECT_EQ(controller_->state_, CloudDiskFuseController::State::CLEANUP_RETRY);

    controller_->shutdown_ = true;
    EXPECT_FALSE(controller_->SelectPendingUnmount(target));
    controller_->shutdown_ = false;
    controller_->pendingUnmounts_.clear();
    controller_->activeUserId_ = TEST_USER_ID;
    AddRoot();
    controller_->pendingUnmounts_.push_back({TEST_USER_ID, "/mnt/unrelated"});
    EXPECT_FALSE(controller_->SelectPendingUnmount(target));
    controller_->pendingUnmounts_.push_back({TEST_USER_ID, controller_->GetMountPoint(TEST_USER_ID)});
    EXPECT_TRUE(controller_->SelectPendingUnmount(target));
    EXPECT_EQ(target.mountPoint, controller_->GetMountPoint(TEST_USER_ID));

    controller_->roots_.clear();
    controller_->session_ = reinterpret_cast<struct fuse_session *>(1);
    EXPECT_FALSE(controller_->SelectPendingUnmount(target));
    AddRoot();
    controller_->state_ = CloudDiskFuseController::State::MOUNTING;
    EXPECT_FALSE(controller_->SelectPendingUnmount(target));
    controller_->state_ = CloudDiskFuseController::State::READY;
    EXPECT_TRUE(controller_->SelectPendingUnmount(target));
    controller_->session_ = nullptr;
}

/*
 * @tc.name: PrepareMountAttempt_001
 * @tc.desc: Verify active-root preparation and shutdown cancellation
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseControllerTest, PrepareMountAttempt_001, TestSize.Level1)
{
    controller_->activeUserId_ = TEST_USER_ID;
    AddRoot();
    CloudDiskFuseController::MountAttempt attempt;
    EXPECT_TRUE(controller_->PrepareMountAttempt(attempt));
    EXPECT_EQ(attempt.userId, TEST_USER_ID);
    EXPECT_EQ(attempt.generation, 1U);
    EXPECT_EQ(controller_->state_, CloudDiskFuseController::State::MOUNTING);

    controller_->shutdown_ = true;
    EXPECT_FALSE(controller_->PrepareMountAttempt(attempt));
}

/*
 * @tc.name: MountSession_001
 * @tc.desc: Verify mount errors, invalid descriptors and stale-mount cleanup
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseControllerTest, MountSession_001, TestSize.Level1)
{
    CloudDiskFuseController::MountAttempt attempt{TEST_USER_ID, "/mnt/mock", 1};
    int32_t ret = E_OK;
    auto &mock = CloudDiskFuseControllerMock::GetState();
    mock.mountResult = EACCES;
    EXPECT_TRUE(controller_->MountSession(attempt, ret));
    EXPECT_EQ(ret, EACCES);
    EXPECT_EQ(mock.unmountCalls, 0U);

    mock.mountResult = E_OK;
    mock.mountFd = -1;
    EXPECT_TRUE(controller_->MountSession(attempt, ret));
    EXPECT_EQ(ret, -EBADF);
    EXPECT_EQ(mock.unmountCalls, 1U);

    mock.mountResult = EBUSY;
    EXPECT_TRUE(controller_->MountSession(attempt, ret));
    EXPECT_EQ(mock.unmountCalls, 2U);
    EXPECT_EQ(mock.forceCreateDirectoryCalls, 3U);
}

/*
 * @tc.name: HandlePendingUnmount_001
 * @tc.desc: Verify pending unmount requests are delegated and finalized
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseControllerTest, HandlePendingUnmount_001, TestSize.Level1)
{
    CloudDiskFuseController::UnmountTarget target{TEST_USER_ID, "/mnt/pending"};
    controller_->pendingUnmounts_.push_back(target);
    EXPECT_TRUE(controller_->HandlePendingUnmount(target));
    EXPECT_TRUE(controller_->pendingUnmounts_.empty());
    EXPECT_EQ(CloudDiskFuseControllerMock::GetState().unmountCalls, 1U);
}

/*
 * @tc.name: WaitForMountRetry_001
 * @tc.desc: Verify shutdown, no-root and bounded retry-count branches
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseControllerTest, WaitForMountRetry_001, TestSize.Level1)
{
    uint32_t retryCount = 2;
    EXPECT_TRUE(controller_->WaitForMountRetry(EIO, retryCount));
    EXPECT_EQ(retryCount, 0U);

    AddRoot();
    EXPECT_TRUE(controller_->WaitForMountRetry(EIO, retryCount));
    EXPECT_EQ(retryCount, 1U);

    controller_->shutdown_ = true;
    EXPECT_FALSE(controller_->WaitForMountRetry(EIO, retryCount));
}

/*
 * @tc.name: WaitForMountRetry_002
 * @tc.desc: Verify exactly three mount retries precede the root-change reset wait
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseControllerTest, WaitForMountRetry_002, TestSize.Level1)
{
    static_assert(CLOUD_DISK_FUSE_MOUNT_RETRY_COUNT == 3U, "Mount retry count must remain fixed at three");
    AddRoot();
    uint32_t retryCount = 0;
    for (uint32_t expectedCount = 1; expectedCount <= CLOUD_DISK_FUSE_MOUNT_RETRY_COUNT; ++expectedCount) {
        EXPECT_TRUE(controller_->WaitForMountRetry(EIO, retryCount));
        EXPECT_EQ(retryCount, expectedCount);
    }

    controller_->state_ = CloudDiskFuseController::State::MOUNTING;
    bool retryResult = false;
    std::thread retryThread(
        [this, &retryCount, &retryResult] { retryResult = controller_->WaitForMountRetry(EIO, retryCount); });
    bool enteredResetWait = false;
    {
        std::unique_lock<std::mutex> lock(controller_->mutex_);
        enteredResetWait = controller_->condition_.wait_for(lock, std::chrono::seconds(1), [this] {
            return controller_->state_ == CloudDiskFuseController::State::IDLE;
        });
        if (enteredResetWait) {
            ++controller_->rootWarmupEpoch_;
        } else {
            controller_->shutdown_ = true;
        }
    }
    controller_->condition_.notify_all();
    retryThread.join();

    ASSERT_TRUE(enteredResetWait);
    EXPECT_TRUE(retryResult);
    EXPECT_EQ(retryCount, 0U);
}

/*
 * @tc.name: WaitForRetryReset_001
 * @tc.desc: Verify immediate epoch, no-root and shutdown reset conditions
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseControllerTest, WaitForRetryReset_001, TestSize.Level1)
{
    controller_->rootWarmupEpoch_ = 5;
    EXPECT_TRUE(controller_->WaitForRetryReset(4, false));
    EXPECT_TRUE(controller_->WaitForRetryReset(5, true));
    controller_->shutdown_ = true;
    EXPECT_FALSE(controller_->WaitForRetryReset(5, false));
}

/*
 * @tc.name: RetryUnmountWithLimit_001
 * @tc.desc: Verify fixed retry success, retry exhaustion and shutdown interruption
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseControllerTest, RetryUnmountWithLimit_001, TestSize.Level1)
{
    uint32_t retryCount = 0;
    auto &mock = CloudDiskFuseControllerMock::GetState();
    mock.unmountResults = {EIO, E_OK};
    EXPECT_EQ(controller_->RetryUnmountWithLimit(TEST_USER_ID, "/mnt/mock", EIO, retryCount), E_OK);
    EXPECT_EQ(retryCount, 2U);
    EXPECT_EQ(mock.unmountCalls, 2U);

    retryCount = CLOUD_DISK_FUSE_UNMOUNT_RETRY_COUNT;
    EXPECT_EQ(controller_->RetryUnmountWithLimit(TEST_USER_ID, "/mnt/mock", EBUSY, retryCount), EBUSY);
    controller_->shutdown_ = true;
    retryCount = 0;
    EXPECT_EQ(controller_->RetryUnmountWithLimit(TEST_USER_ID, "/mnt/mock", EIO, retryCount), EIO);
    EXPECT_EQ(retryCount, 0U);
}

/*
 * @tc.name: RetryUnmountWithLimit_002
 * @tc.desc: Verify an unmount failure performs exactly three retries before returning the error
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseControllerTest, RetryUnmountWithLimit_002, TestSize.Level1)
{
    static_assert(CLOUD_DISK_FUSE_UNMOUNT_RETRY_COUNT == 3U, "Unmount retry count must remain fixed at three");
    auto &mock = CloudDiskFuseControllerMock::GetState();
    mock.unmountResults = {EIO, EIO, EIO};
    uint32_t retryCount = 0;

    EXPECT_EQ(controller_->RetryUnmountWithLimit(TEST_USER_ID, "/mnt/mock", EIO, retryCount), EIO);
    EXPECT_EQ(retryCount, 3U);
    EXPECT_EQ(mock.unmountCalls, 3U);
    EXPECT_TRUE(mock.unmountResults.empty());
}

/*
 * @tc.name: RetryUnmount_001
 * @tc.desc: Verify pending cleanup on success and preservation on shutdown failure
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseControllerTest, RetryUnmount_001, TestSize.Level1)
{
    controller_->pendingUnmounts_.push_back({TEST_USER_ID, "/mnt/mock"});
    EXPECT_TRUE(controller_->RetryUnmount(TEST_USER_ID, "/mnt/mock", E_OK));
    EXPECT_TRUE(controller_->pendingUnmounts_.empty());
    EXPECT_EQ(controller_->state_, CloudDiskFuseController::State::IDLE);

    controller_->shutdown_ = true;
    EXPECT_FALSE(controller_->RetryUnmount(TEST_USER_ID, "/mnt/failed", EIO));
    EXPECT_EQ(controller_->pendingUnmounts_.size(), 1U);
    EXPECT_EQ(controller_->state_, CloudDiskFuseController::State::STOPPING);
}

/*
 * @tc.name: SessionHelpers_001
 * @tc.desc: Verify no-session handling, active-session detection and wake event
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseControllerTest, SessionHelpers_001, TestSize.Level1)
{
    EXPECT_FALSE(controller_->HasActiveSession());
    EXPECT_TRUE(controller_->HandleActiveSession());
    EXPECT_EQ(controller_->StopSession(TEST_USER_ID, "/mnt/mock"), E_OK);
    controller_->session_ = reinterpret_cast<struct fuse_session *>(1);
    EXPECT_TRUE(controller_->HasActiveSession());
    controller_->session_ = nullptr;

    controller_->shutdown_ = true;
    std::unique_lock<std::mutex> lock(controller_->mutex_);
    controller_->WaitForSessionStop(lock);
    EXPECT_TRUE(lock.owns_lock());
    lock.unlock();
    controller_->shutdown_ = false;

    int wakeFd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
    ASSERT_GE(wakeFd, 0);
    controller_->wakeFd_ = wakeFd;
    controller_->WakeSession();
    uint64_t value = 0;
    ASSERT_EQ(read(wakeFd, &value, sizeof(value)), static_cast<ssize_t>(sizeof(value)));
    EXPECT_EQ(value, 1U);
    controller_->wakeFd_ = -1;
    (void)close(wakeFd);
}

/*
 * @tc.name: ResolveLookup_001
 * @tc.desc: Verify invalid names and disconnected lookup rejection
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseControllerTest, ResolveLookup_001, TestSize.Level2)
{
    CloudDiskFuseController::LookupResult result;
    EXPECT_EQ(controller_->ResolveLookup(FUSE_ROOT_ID, nullptr, result), EINVAL);
    EXPECT_EQ(controller_->ResolveLookup(FUSE_ROOT_ID, "", result), EINVAL);
    EXPECT_EQ(controller_->ResolveLookup(FUSE_ROOT_ID, ".", result), EINVAL);
    EXPECT_EQ(controller_->ResolveLookup(FUSE_ROOT_ID, "..", result), EINVAL);
    EXPECT_EQ(controller_->ResolveLookup(FUSE_ROOT_ID, "bad/name", result), EINVAL);
    std::string longName(NAME_MAX + 1, 'a');
    EXPECT_EQ(controller_->ResolveLookup(FUSE_ROOT_ID, longName.c_str(), result), EINVAL);
    EXPECT_EQ(controller_->ResolveLookup(FUSE_ROOT_ID, "valid", result), ENOTCONN);
}

/*
 * @tc.name: ResolveLookup_002
 * @tc.desc: Verify no-match, warmup-failure, metadata terminal-error and match results
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseControllerTest, ResolveLookup_002, TestSize.Level1)
{
    controller_->state_ = CloudDiskFuseController::State::READY;
    controller_->activeUserId_ = TEST_USER_ID;
    CloudDiskFuseController::LookupResult result;
    EXPECT_EQ(controller_->ResolveLookup(FUSE_ROOT_ID, "file", result), EAGAIN);

    AddRoot(TEST_ROOT_INDEX, CloudDiskFuseController::RootState::WARMING);
    controller_->roots_[TEST_ROOT_INDEX].lastWarmupError = EIO;
    EXPECT_EQ(controller_->ResolveLookup(FUSE_ROOT_ID, "file", result), EIO);
    controller_->roots_[TEST_ROOT_INDEX].state = CloudDiskFuseController::RootState::READY;
    auto &behavior = CloudDiskFuseControllerMock::GetState().metaBehaviors[TEST_ROOT_INDEX];
    behavior.exists = true;
    behavior.lookupResult = ENOENT;
    EXPECT_EQ(controller_->ResolveLookup(FUSE_ROOT_ID, "file", result), ENOENT);

    std::string rootPath = CreateTempDirectory();
    ASSERT_FALSE(rootPath.empty());
    std::string filePath = CreateTempFile(rootPath, "file", "data");
    ASSERT_FALSE(filePath.empty());
    controller_->roots_[TEST_ROOT_INDEX].mountPath = rootPath;
    behavior.lookupResult = E_OK;
    behavior.relativePathResult = E_OK;
    behavior.placeholder = 1;
    behavior.mode = S_IFREG | 0600;
    EXPECT_EQ(controller_->ResolveLookup(FUSE_ROOT_ID, "file", result), E_OK);
    EXPECT_EQ(result.path, filePath);
}

/*
 * @tc.name: ResolveLookup_003
 * @tc.desc: Verify lookup waits for an in-flight warmup and retries the root scan once
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseControllerTest, ResolveLookup_003, TestSize.Level1)
{
    std::string rootPath = CreateTempDirectory();
    ASSERT_FALSE(rootPath.empty());
    std::string filePath = CreateTempFile(rootPath, "file", "data");
    ASSERT_FALSE(filePath.empty());

    controller_->state_ = CloudDiskFuseController::State::READY;
    controller_->activeUserId_ = TEST_USER_ID;
    controller_->generation_ = 3;
    AddRoot(1, CloudDiskFuseController::RootState::READY, 1);
    controller_->roots_[1].mountPath = rootPath;
    AddRoot(2, CloudDiskFuseController::RootState::WARMING, 2);
    controller_->roots_[2].warmupInFlight = true;

    std::mutex hookMutex;
    std::condition_variable hookCondition;
    uint32_t lookupCalls = 0;
    bool resumeLookup = false;
    SetupWarmupHook(hookMutex, hookCondition, lookupCalls, resumeLookup);

    CloudDiskFuseController::LookupResult result;
    int32_t lookupResult = EIO;
    std::thread lookupThread(
        [this, &result, &lookupResult] { lookupResult = controller_->ResolveLookup(FUSE_ROOT_ID, "file", result); });

    bool firstScanObserved = false;
    {
        std::unique_lock<std::mutex> lock(hookMutex);
        firstScanObserved =
            hookCondition.wait_for(lock, std::chrono::seconds(1), [&lookupCalls] { return lookupCalls >= 1; });
    }
    if (firstScanObserved) {
        ConfigureRootBehavior(1);
    }
    ResumeRootWarmup(2, firstScanObserved);
    {
        std::lock_guard<std::mutex> lock(hookMutex);
        resumeLookup = true;
    }
    hookCondition.notify_all();
    controller_->condition_.notify_all();
    lookupThread.join();
    CloudDiskFuseControllerMock::GetState().metaFileLookupHook = nullptr;

    ASSERT_TRUE(firstScanObserved);
    EXPECT_EQ(lookupResult, E_OK);
    EXPECT_EQ(result.syncFolderIndex, 1U);
    EXPECT_EQ(result.path, filePath);
    std::lock_guard<std::mutex> lock(hookMutex);
    EXPECT_GE(lookupCalls, 2U);
}

/*
 * @tc.name: CaptureLookupSnapshot_001
 * @tc.desc: Verify ready, warming, failed and retiring root classification
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseControllerTest, CaptureLookupSnapshot_001, TestSize.Level1)
{
    CloudDiskFuseController::LookupSnapshot snapshot;
    EXPECT_EQ(controller_->CaptureLookupSnapshot(snapshot), ENOTCONN);
    controller_->state_ = CloudDiskFuseController::State::READY;
    controller_->activeUserId_ = TEST_USER_ID;
    controller_->generation_ = 9;
    AddRoot(1, CloudDiskFuseController::RootState::READY, 1);
    AddRoot(2, CloudDiskFuseController::RootState::WARMING, 2);
    controller_->roots_[2].warmupInFlight = true;
    AddRoot(3, CloudDiskFuseController::RootState::WARMING, 3);
    controller_->roots_[3].lastWarmupError = EIO;
    AddRoot(4, CloudDiskFuseController::RootState::RETIRING, 4);
    EXPECT_EQ(controller_->CaptureLookupSnapshot(snapshot), E_OK);
    EXPECT_EQ(snapshot.userId, TEST_USER_ID);
    EXPECT_EQ(snapshot.generation, 9U);
    EXPECT_EQ(snapshot.roots.size(), 1U);
    EXPECT_TRUE(snapshot.rootWarming);
    EXPECT_EQ(snapshot.warmupError, EIO);
}

/*
 * @tc.name: BuildLookupCandidate_001
 * @tc.desc: Verify path joining and overflow guards
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseControllerTest, BuildLookupCandidate_001, TestSize.Level1)
{
    auto root = AddRoot();
    std::string candidate;
    EXPECT_EQ(controller_->BuildLookupCandidate(root, "relative", "file", candidate), E_OK);
    EXPECT_EQ(candidate, "/mnt/mock/rootrelative/file");
    EXPECT_EQ(controller_->BuildLookupCandidate(root, "relative/", "file", candidate), E_OK);
    EXPECT_EQ(candidate, "/mnt/mock/rootrelative/file");
    root.mountPath.assign(PATH_MAX + 1, 'x');
    EXPECT_EQ(controller_->BuildLookupCandidate(root, "", "file", candidate), ENAMETOOLONG);
}

/*
 * @tc.name: ValidateLookupCandidate_001
 * @tc.desc: Verify missing, non-regular, escaping and valid backing paths
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseControllerTest, ValidateLookupCandidate_001, TestSize.Level1)
{
    std::string rootPath = CreateTempDirectory();
    std::string outsidePath = CreateTempDirectory();
    ASSERT_FALSE(rootPath.empty());
    ASSERT_FALSE(outsidePath.empty());
    std::string filePath = CreateTempFile(rootPath, "file", "data");
    std::string escapedPath = CreateTempFile(outsidePath, "escaped", "data");
    ASSERT_FALSE(filePath.empty());
    ASSERT_FALSE(escapedPath.empty());
    CloudDiskFuseController::RootContext root;
    root.mountPath = rootPath;
    std::string canonicalPath;
    struct stat attr {};
    EXPECT_EQ(controller_->ValidateLookupCandidate(root, rootPath + "/missing", canonicalPath, attr), ENOENT);
    EXPECT_EQ(controller_->ValidateLookupCandidate(root, rootPath, canonicalPath, attr), ESTALE);
    EXPECT_EQ(controller_->ValidateLookupCandidate(root, escapedPath, canonicalPath, attr), ESTALE);
    EXPECT_EQ(controller_->ValidateLookupCandidate(root, filePath, canonicalPath, attr), E_OK);
    EXPECT_EQ(canonicalPath, filePath);
}

/*
 * @tc.name: ResolveLookupInRoot_001
 * @tc.desc: Verify metadata absence, lookup errors and placeholder validation
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseControllerTest, ResolveLookupInRoot_001, TestSize.Level1)
{
    auto root = AddRoot();
    CloudDiskFuseController::LookupResult result;
    auto outcome = controller_->ResolveLookupInRoot(TEST_USER_ID, FUSE_ROOT_ID, "file", root, result);
    EXPECT_EQ(outcome.state, CloudDiskFuseController::RootLookupState::NO_PARENT);

    auto &behavior = CloudDiskFuseControllerMock::GetState().metaBehaviors[TEST_ROOT_INDEX];
    behavior.exists = true;
    behavior.lookupResult = ENOENT;
    outcome = controller_->ResolveLookupInRoot(TEST_USER_ID, FUSE_ROOT_ID, "file", root, result);
    EXPECT_EQ(outcome.state, CloudDiskFuseController::RootLookupState::TERMINAL_ERROR);
    EXPECT_EQ(outcome.error, ENOENT);
    behavior.lookupResult = EACCES;
    outcome = controller_->ResolveLookupInRoot(TEST_USER_ID, FUSE_ROOT_ID, "file", root, result);
    EXPECT_EQ(outcome.state, CloudDiskFuseController::RootLookupState::CONTINUE);
    EXPECT_EQ(outcome.error, EIO);
    behavior.lookupResult = E_OK;
    behavior.placeholder = 0;
    outcome = controller_->ResolveLookupInRoot(TEST_USER_ID, FUSE_ROOT_ID, "file", root, result);
    EXPECT_EQ(outcome.error, ESTALE);
    behavior.placeholder = 1;
    behavior.mode = S_IFDIR | 0700;
    outcome = controller_->ResolveLookupInRoot(TEST_USER_ID, FUSE_ROOT_ID, "file", root, result);
    EXPECT_EQ(outcome.error, ESTALE);
}

/*
 * @tc.name: ResolveLookupInRoot_002
 * @tc.desc: Verify relative-path, candidate validation and successful lookup branches
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseControllerTest, ResolveLookupInRoot_002, TestSize.Level1)
{
    std::string rootPath = CreateTempDirectory();
    ASSERT_FALSE(rootPath.empty());
    std::string filePath = CreateTempFile(rootPath, "file", "data");
    ASSERT_FALSE(filePath.empty());
    auto root = AddRoot();
    root.mountPath = rootPath;
    auto &behavior = CloudDiskFuseControllerMock::GetState().metaBehaviors[TEST_ROOT_INDEX];
    behavior.exists = true;
    behavior.placeholder = 1;
    behavior.mode = S_IFREG | 0600;
    behavior.relativePathResult = EIO;
    CloudDiskFuseController::LookupResult result;
    auto outcome = controller_->ResolveLookupInRoot(TEST_USER_ID, FUSE_ROOT_ID, "file", root, result);
    EXPECT_EQ(outcome.error, EIO);
    behavior.relativePathResult = E_OK;
    behavior.relativePath.clear();
    outcome = controller_->ResolveLookupInRoot(TEST_USER_ID, FUSE_ROOT_ID, "missing", root, result);
    EXPECT_EQ(outcome.state, CloudDiskFuseController::RootLookupState::CONTINUE);
    EXPECT_EQ(outcome.error, ENOENT);
    outcome = controller_->ResolveLookupInRoot(TEST_USER_ID, FUSE_ROOT_ID, "file", root, result);
    EXPECT_EQ(outcome.state, CloudDiskFuseController::RootLookupState::MATCH);
    EXPECT_EQ(result.path, filePath);
    EXPECT_EQ(result.syncFolderIndex, TEST_ROOT_INDEX);
}

/*
 * @tc.name: ScanLookupRoots_001
 * @tc.desc: Verify no-parent, terminal-error and continued-error aggregation
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseControllerTest, ScanLookupRoots_001, TestSize.Level1)
{
    CloudDiskFuseController::LookupSnapshot snapshot;
    snapshot.userId = TEST_USER_ID;
    snapshot.roots.push_back(AddRoot(1));
    snapshot.roots.push_back(AddRoot(2));
    CloudDiskFuseController::LookupResult result;
    auto outcome = controller_->ScanLookupRoots(snapshot, FUSE_ROOT_ID, "file", result);
    EXPECT_EQ(outcome.state, CloudDiskFuseController::RootLookupState::NO_PARENT);

    auto &first = CloudDiskFuseControllerMock::GetState().metaBehaviors[1];
    first.exists = true;
    first.lookupResult = EIO;
    outcome = controller_->ScanLookupRoots(snapshot, FUSE_ROOT_ID, "file", result);
    EXPECT_EQ(outcome.state, CloudDiskFuseController::RootLookupState::CONTINUE);
    EXPECT_EQ(outcome.error, EIO);
    auto &second = CloudDiskFuseControllerMock::GetState().metaBehaviors[2];
    second.exists = true;
    second.lookupResult = ENOENT;
    outcome = controller_->ScanLookupRoots(snapshot, FUSE_ROOT_ID, "file", result);
    EXPECT_EQ(outcome.state, CloudDiskFuseController::RootLookupState::TERMINAL_ERROR);
    EXPECT_EQ(outcome.error, ENOENT);
}

/*
 * @tc.name: WaitForLookupWarmup_001
 * @tc.desc: Verify completed and invalidated lookup snapshots
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseControllerTest, WaitForLookupWarmup_001, TestSize.Level1)
{
    controller_->state_ = CloudDiskFuseController::State::READY;
    controller_->activeUserId_ = TEST_USER_ID;
    controller_->generation_ = 3;
    CloudDiskFuseController::LookupSnapshot snapshot;
    snapshot.userId = TEST_USER_ID;
    snapshot.generation = 3;
    EXPECT_EQ(controller_->WaitForLookupWarmup(snapshot), E_OK);
    snapshot.generation = 2;
    EXPECT_EQ(controller_->WaitForLookupWarmup(snapshot), ENOTCONN);
}

/*
 * @tc.name: WaitForLookupWarmup_002
 * @tc.desc: Verify an unchanged in-flight warmup reaches the bounded wait timeout
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseControllerTest, WaitForLookupWarmup_002, TestSize.Level2)
{
    controller_->state_ = CloudDiskFuseController::State::READY;
    controller_->activeUserId_ = TEST_USER_ID;
    controller_->generation_ = 3;
    AddRoot(TEST_ROOT_INDEX, CloudDiskFuseController::RootState::WARMING);
    controller_->roots_[TEST_ROOT_INDEX].warmupInFlight = true;
    CloudDiskFuseController::LookupSnapshot snapshot;
    snapshot.userId = TEST_USER_ID;
    snapshot.generation = 3;

    EXPECT_EQ(controller_->WaitForLookupWarmup(snapshot), EAGAIN);
}

/*
 * @tc.name: PurgeContexts_001
 * @tc.desc: Verify root-specific and complete node/open-context purging
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseControllerTest, PurgeContexts_001, TestSize.Level1)
{
    int fds[2] = {-1, -1};
    ASSERT_EQ(pipe(fds), 0);
    AddNode(TEST_NODE_ID, TEST_ROOT_INDEX);
    AddNode(TEST_NODE_ID + 1, TEST_ROOT_INDEX + 1, TEST_ROOT_EPOCH, TEST_DEVICE + 1, TEST_INODE + 1);
    auto context = std::make_unique<CloudDiskFuseController::OpenContext>();
    context->syncFolderIndex = TEST_ROOT_INDEX;
    context->fd = UniqueFd(fds[0]);
    controller_->openContexts_[1] = std::move(context);
    std::vector<int> released;
    controller_->PurgeRootLocked(TEST_ROOT_INDEX, released);
    ASSERT_EQ(released.size(), 1U);
    EXPECT_EQ(released[0], fds[0]);
    EXPECT_EQ(controller_->nodes_.size(), 1U);
    ASSERT_EQ(controller_->identityToNode_.size(), 1U);
    auto survivorIdentity = controller_->identityToNode_.find(
        {static_cast<uint64_t>(TEST_DEVICE + 1), static_cast<uint64_t>(TEST_INODE + 1)});
    ASSERT_NE(survivorIdentity, controller_->identityToNode_.end());
    EXPECT_EQ(survivorIdentity->second, TEST_NODE_ID + 1);
    (void)close(released[0]);
    (void)close(fds[1]);

    controller_->pendingOpenCount_ = 2;
    controller_->nextNodeId_ = 99;
    controller_->nextHandle_ = 88;
    released.clear();
    controller_->PurgeAllLocked(released);
    EXPECT_TRUE(controller_->nodes_.empty());
    EXPECT_TRUE(controller_->identityToNode_.empty());
    EXPECT_EQ(controller_->pendingOpenCount_, 0U);
    EXPECT_EQ(controller_->nextNodeId_, FUSE_ROOT_ID + 1);
    EXPECT_EQ(controller_->nextHandle_, 1U);
}

/*
 * @tc.name: RollbackLookup_001
 * @tc.desc: Verify lookup rollback decrements and erases eligible nodes
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseControllerTest, RollbackLookup_001, TestSize.Level1)
{
    controller_->RollbackLookup(TEST_NODE_ID);
    AddNode();
    controller_->nodes_[TEST_NODE_ID].nlookup = 2;
    controller_->RollbackLookup(TEST_NODE_ID);
    EXPECT_EQ(controller_->nodes_[TEST_NODE_ID].nlookup, 1U);
    controller_->RollbackLookup(TEST_NODE_ID);
    EXPECT_TRUE(controller_->nodes_.empty());
    EXPECT_TRUE(controller_->identityToNode_.empty());
}

/*
 * @tc.name: RollbackOpen_001
 * @tc.desc: Verify open rollback closes descriptors and removes unused nodes
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseControllerTest, RollbackOpen_001, TestSize.Level1)
{
    controller_->RollbackOpen(99);
    int fds[2] = {-1, -1};
    ASSERT_EQ(pipe(fds), 0);
    AddNode();
    controller_->nodes_[TEST_NODE_ID].nlookup = 0;
    controller_->nodes_[TEST_NODE_ID].openCount = 1;
    auto context = std::make_unique<CloudDiskFuseController::OpenContext>();
    context->nodeId = TEST_NODE_ID;
    context->fd = UniqueFd(fds[0]);
    controller_->openContexts_[99] = std::move(context);
    controller_->RollbackOpen(99);
    EXPECT_TRUE(controller_->openContexts_.empty());
    EXPECT_TRUE(controller_->nodes_.empty());
    EXPECT_EQ(fcntl(fds[0], F_GETFD), -1);
    (void)close(fds[1]);
}

/*
 * @tc.name: Init_001
 * @tc.desc: Verify FUSE init accepts only an active mounting generation
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseControllerTest, Init_001, TestSize.Level1)
{
    CloudDiskFuseController::Init(nullptr, nullptr);
    AddRoot();
    controller_->state_ = CloudDiskFuseController::State::MOUNTING;
    CloudDiskFuseController::Init(controller_.get(), nullptr);
    EXPECT_EQ(controller_->state_, CloudDiskFuseController::State::READY);
    controller_->state_ = CloudDiskFuseController::State::IDLE;
    CloudDiskFuseController::Init(controller_.get(), nullptr);
    EXPECT_EQ(controller_->state_, CloudDiskFuseController::State::IDLE);
    CloudDiskFuseController::Destroy(controller_.get());
}

/*
 * @tc.name: RegisterLookupNode_001
 * @tc.desc: Verify new, repeated, stale and exhausted node registration
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseControllerTest, RegisterLookupNode_001, TestSize.Level1)
{
    auto result = MakeLookupResult();
    fuse_ino_t nodeId = 0;
    EXPECT_EQ(controller_->RegisterLookupNode(result, nodeId), ESTALE);
    AddRoot();
    controller_->state_ = CloudDiskFuseController::State::READY;
    EXPECT_EQ(controller_->RegisterLookupNode(result, nodeId), E_OK);
    EXPECT_EQ(controller_->nodes_[nodeId].nlookup, 1U);
    fuse_ino_t repeatedNodeId = 0;
    EXPECT_EQ(controller_->RegisterLookupNode(result, repeatedNodeId), E_OK);
    EXPECT_EQ(repeatedNodeId, nodeId);
    EXPECT_EQ(controller_->nodes_[nodeId].nlookup, 2U);
    controller_->nodes_[nodeId].nlookup = std::numeric_limits<uint64_t>::max();
    EXPECT_EQ(controller_->RegisterLookupNode(result, repeatedNodeId), EOVERFLOW);

    controller_->identityToNode_[{99, 100}] = 999;
    result.attr.st_dev = 99;
    result.attr.st_ino = 100;
    EXPECT_EQ(controller_->RegisterLookupNode(result, repeatedNodeId), EOVERFLOW);
    controller_->identityToNode_.erase({99, 100});
    controller_->nextNodeId_ = 0;
    EXPECT_EQ(controller_->RegisterLookupNode(result, repeatedNodeId), ENOSPC);
}

/*
 * @tc.name: ForgetNode_001
 * @tc.desc: Verify root, missing, partial and final forget behavior
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseControllerTest, ForgetNode_001, TestSize.Level1)
{
    controller_->ForgetNode(FUSE_ROOT_ID, 1);
    controller_->ForgetNode(TEST_NODE_ID, 1);
    AddNode();
    controller_->nodes_[TEST_NODE_ID].nlookup = 3;
    controller_->ForgetNode(TEST_NODE_ID, 1);
    EXPECT_EQ(controller_->nodes_[TEST_NODE_ID].nlookup, 2U);
    controller_->nodes_[TEST_NODE_ID].openCount = 1;
    controller_->ForgetNode(TEST_NODE_ID, 5);
    EXPECT_EQ(controller_->nodes_[TEST_NODE_ID].nlookup, 0U);
    controller_->nodes_[TEST_NODE_ID].openCount = 0;
    controller_->ForgetNode(TEST_NODE_ID, 1);
    EXPECT_TRUE(controller_->nodes_.empty());
}

/*
 * @tc.name: GetAttrNode_001
 * @tc.desc: Verify disconnected, missing, stale and valid attribute contexts
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseControllerTest, GetAttrNode_001, TestSize.Level1)
{
    CloudDiskFuseController::NodeContext node;
    EXPECT_EQ(controller_->GetAttrNode(TEST_NODE_ID, node), ENOTCONN);
    controller_->state_ = CloudDiskFuseController::State::READY;
    EXPECT_EQ(controller_->GetAttrNode(TEST_NODE_ID, node), ESTALE);
    AddNode();
    EXPECT_EQ(controller_->GetAttrNode(TEST_NODE_ID, node), ESTALE);
    AddRoot();
    EXPECT_EQ(controller_->GetAttrNode(TEST_NODE_ID, node), E_OK);
    EXPECT_EQ(node.nodeId, TEST_NODE_ID);
    controller_->roots_[TEST_ROOT_INDEX].warmupEpoch++;
    EXPECT_EQ(controller_->GetAttrNode(TEST_NODE_ID, node), ESTALE);
}

/*
 * @tc.name: GetOperationIdentity_001
 * @tc.desc: Verify operation identity captures current user and generation
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseControllerTest, GetOperationIdentity_001, TestSize.Level1)
{
    controller_->activeUserId_ = TEST_USER_ID;
    controller_->generation_ = 17;
    auto identity = controller_->GetOperationIdentity();
    EXPECT_EQ(identity.userId, TEST_USER_ID);
    EXPECT_EQ(identity.generation, 17U);
    EXPECT_EQ(identity.rootIndex, 0U);
}

/*
 * @tc.name: ReserveOpen_001
 * @tc.desc: Verify open reservation state, node, root and capacity guards
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseControllerTest, ReserveOpen_001, TestSize.Level1)
{
    CloudDiskFuseController::OpenOperation operation;
    EXPECT_EQ(controller_->ReserveOpen(TEST_NODE_ID, operation), ENOTCONN);
    controller_->state_ = CloudDiskFuseController::State::READY;
    EXPECT_EQ(controller_->ReserveOpen(TEST_NODE_ID, operation), ESTALE);
    AddNode();
    EXPECT_EQ(controller_->ReserveOpen(TEST_NODE_ID, operation), ESTALE);
    AddRoot();
    controller_->pendingOpenCount_ = 1024;
    EXPECT_EQ(controller_->ReserveOpen(TEST_NODE_ID, operation), EMFILE);
    controller_->pendingOpenCount_ = 0;
    controller_->generation_ = 8;
    EXPECT_EQ(controller_->ReserveOpen(TEST_NODE_ID, operation), E_OK);
    EXPECT_EQ(controller_->pendingOpenCount_, 1U);
    EXPECT_EQ(operation.generation, 8U);
    controller_->ReleaseOpenReservation();
    EXPECT_EQ(controller_->pendingOpenCount_, 0U);
    controller_->ReleaseOpenReservation();
    EXPECT_EQ(controller_->pendingOpenCount_, 0U);
}

/*
 * @tc.name: OpenBackingFile_001
 * @tc.desc: Verify missing, identity-mismatch and successful backing-file opens
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseControllerTest, OpenBackingFile_001, TestSize.Level1)
{
    CloudDiskFuseController::OpenOperation operation;
    operation.node.path = "/path/that/does/not/exist";
    EXPECT_EQ(controller_->OpenBackingFile(operation), ENOENT);
    std::string rootPath = CreateTempDirectory();
    ASSERT_FALSE(rootPath.empty());
    std::string filePath = CreateTempFile(rootPath, "file", "payload");
    ASSERT_FALSE(filePath.empty());
    struct stat attr {};
    ASSERT_EQ(lstat(rootPath.c_str(), &attr), 0);
    operation.node.path = rootPath;
    operation.node.device = attr.st_dev;
    operation.node.inode = attr.st_ino;
    EXPECT_EQ(controller_->OpenBackingFile(operation), ESTALE);
    ASSERT_EQ(lstat(filePath.c_str(), &attr), 0);
    operation.node.path = filePath;
    operation.node.device = attr.st_dev;
    operation.node.inode = attr.st_ino + 1;
    EXPECT_EQ(controller_->OpenBackingFile(operation), ESTALE);
    operation.node.inode = attr.st_ino;
    EXPECT_EQ(controller_->OpenBackingFile(operation), E_OK);
    EXPECT_GE(operation.fd.Get(), 0);
}

/*
 * @tc.name: CommitOpen_001
 * @tc.desc: Verify generation, node, root, handle and successful commit branches
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseControllerTest, CommitOpen_001, TestSize.Level1)
{
    CloudDiskFuseController::OpenOperation operation;
    operation.generation = 1;
    controller_->pendingOpenCount_ = 1;
    EXPECT_EQ(controller_->CommitOpen(TEST_NODE_ID, operation), ENOTCONN);
    controller_->state_ = CloudDiskFuseController::State::READY;
    controller_->generation_ = 1;
    EXPECT_EQ(controller_->CommitOpen(TEST_NODE_ID, operation), ESTALE);
    operation.node = AddNode();
    EXPECT_EQ(controller_->CommitOpen(TEST_NODE_ID, operation), ESTALE);
    AddRoot();
    controller_->nextHandle_ = 0;
    EXPECT_EQ(controller_->CommitOpen(TEST_NODE_ID, operation), EMFILE);
    controller_->nextHandle_ = 1;
    int fds[2] = {-1, -1};
    ASSERT_EQ(pipe(fds), 0);
    operation.fd = UniqueFd(fds[0]);
    operation.attr.st_dev = TEST_DEVICE;
    operation.attr.st_ino = TEST_INODE;
    EXPECT_EQ(controller_->CommitOpen(TEST_NODE_ID, operation), E_OK);
    EXPECT_EQ(operation.handle, 1U);
    EXPECT_EQ(controller_->nodes_[TEST_NODE_ID].openCount, 1U);
    EXPECT_EQ(controller_->openContexts_.size(), 1U);
    (void)close(fds[1]);
}

/*
 * @tc.name: TakeOpenContext_001
 * @tc.desc: Verify invalid handles and successful open-context release
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseControllerTest, TakeOpenContext_001, TestSize.Level1)
{
    CloudDiskFuseController::ReleaseOperation operation;
    EXPECT_EQ(controller_->TakeOpenContext(TEST_NODE_ID, 1, operation), EBADF);
    int fds[2] = {-1, -1};
    ASSERT_EQ(pipe(fds), 0);
    AddNode();
    controller_->generation_ = 3;
    controller_->nodes_[TEST_NODE_ID].nlookup = 0;
    controller_->nodes_[TEST_NODE_ID].openCount = 1;
    auto context = std::make_unique<CloudDiskFuseController::OpenContext>();
    context->handle = 1;
    context->nodeId = TEST_NODE_ID;
    context->syncFolderIndex = TEST_ROOT_INDEX;
    context->generation = 3;
    context->fd = UniqueFd(fds[0]);
    controller_->openContexts_[1] = std::move(context);
    EXPECT_EQ(controller_->TakeOpenContext(TEST_NODE_ID + 1, 1, operation), EBADF);
    EXPECT_EQ(controller_->TakeOpenContext(TEST_NODE_ID, 1, operation), E_OK);
    EXPECT_EQ(operation.fd, fds[0]);
    EXPECT_EQ(operation.identity.rootIndex, TEST_ROOT_INDEX);
    EXPECT_TRUE(controller_->openContexts_.empty());
    EXPECT_TRUE(controller_->nodes_.empty());
    (void)close(operation.fd);
    (void)close(fds[1]);
}

/*
 * @tc.name: ReadFuseFd_001
 * @tc.desc: Verify null userdata, normal pipe read and wake interruption
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseControllerTest, ReadFuseFd_001, TestSize.Level1)
{
    char buffer[8] = {};
    errno = 0;
    EXPECT_EQ(CloudDiskFuseController::ReadFuseFd(-1, buffer, sizeof(buffer), nullptr), -1);
    EXPECT_EQ(errno, EINVAL);

    int dataFds[2] = {-1, -1};
    ASSERT_EQ(pipe(dataFds), 0);
    ASSERT_EQ(write(dataFds[1], "abc", 3), 3);
    EXPECT_EQ(CloudDiskFuseController::ReadFuseFd(dataFds[0], buffer, sizeof(buffer), controller_.get()), 3);
    EXPECT_EQ(std::string(buffer, 3), "abc");

    int wakeFd = eventfd(1, EFD_CLOEXEC | EFD_NONBLOCK);
    ASSERT_GE(wakeFd, 0);
    controller_->wakeFd_ = wakeFd;
    errno = 0;
    EXPECT_EQ(CloudDiskFuseController::ReadFuseFd(dataFds[0], buffer, sizeof(buffer), controller_.get()), -1);
    EXPECT_EQ(errno, EINTR);
    controller_->wakeFd_ = -1;
    (void)close(wakeFd);
    (void)close(dataFds[0]);
    (void)close(dataFds[1]);
}

/*
 * @tc.name: WriteFuseFd_001
 * @tc.desc: Verify vectored FUSE output is written to the supplied descriptor
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseControllerTest, WriteFuseFd_001, TestSize.Level1)
{
    int fds[2] = {-1, -1};
    ASSERT_EQ(pipe(fds), 0);
    char first[] = "ab";
    char second[] = "cd";
    struct iovec vectors[2] = {
        {first, 2},
        {second, 2},
    };
    EXPECT_EQ(CloudDiskFuseController::WriteFuseFd(fds[1], vectors, 2, controller_.get()), 4);
    char buffer[4] = {};
    ASSERT_EQ(read(fds[0], buffer, sizeof(buffer)), 4);
    EXPECT_EQ(std::string(buffer, sizeof(buffer)), "abcd");
    (void)close(fds[0]);
    (void)close(fds[1]);
}

} // namespace
} // namespace OHOS::FileManagement::CloudDiskService::Test
