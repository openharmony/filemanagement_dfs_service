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

#include <cerrno>
#include <chrono>
#include <fcntl.h>
#include <gtest/gtest.h>
#include <sys/stat.h>

#include "CloudDiskFuseOperationsMock.h"
#include "cloud_disk_fuse_operations.cpp"
#include "cloud_disk_fuse_operations.h"

namespace OHOS::FileManagement::CloudDiskService::Test {
using namespace testing;
using namespace testing::ext;

namespace {
constexpr fuse_ino_t TEST_PARENT_NODE = 11;
constexpr fuse_ino_t TEST_NODE = 12;
constexpr uint64_t TEST_HANDLE = 13;
constexpr uint64_t TEST_LOOKUP_COUNT = 14;
constexpr dev_t TEST_DEVICE = 15;
constexpr ino_t TEST_INODE = 16;
constexpr uint32_t TEST_ROOT_INDEX = 17;
constexpr uint64_t TEST_GENERATION = 18;
constexpr int32_t TEST_USER_ID = 100;
constexpr uint32_t OPERATION_LOG_LIMIT = 16;
constexpr uint32_t TIMED_OPERATION_LOG_LIMIT = 32;
constexpr mode_t TEST_REGULAR_FILE_MODE = 0440;
constexpr char TEST_NAME[] = "entry";
constexpr char TEST_PATH[] = "/mock/root/entry";
} // namespace

class CloudDiskFuseOperationsTest : public testing::Test {
public:
    void SetUp() override
    {
        ResetCloudDiskFuseOperationsMock();
        auto futureWindowStart =
            std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now().time_since_epoch())
                .count() +
            3600;
        g_openLogWindowStart.store(futureWindowStart, std::memory_order_relaxed);
        g_openLogWindowCount.store(0, std::memory_order_relaxed);
        g_releaseLogWindowStart.store(futureWindowStart, std::memory_order_relaxed);
        g_releaseLogWindowCount.store(0, std::memory_order_relaxed);
        state_ = &GetCloudDiskFuseOperationsMockState();
        controller_ = &GetCloudDiskFuseOperationsMockController();
        request_ = {};
        request_.userdata = controller_;
    }

    void TearDown() override
    {
        ResetCloudDiskFuseOperationsMock();
        state_ = nullptr;
        controller_ = nullptr;
    }

protected:
    void SetRegularLstatResult()
    {
        state_->mockLstat = true;
        state_->lstatResult = 0;
        state_->lstatAttr.st_mode = S_IFREG | TEST_REGULAR_FILE_MODE;
        state_->lstatAttr.st_dev = TEST_DEVICE;
        state_->lstatAttr.st_ino = TEST_INODE;
        state_->nodeDevice = TEST_DEVICE;
        state_->nodeInode = TEST_INODE;
        state_->nodePath = TEST_PATH;
    }

    CloudDiskFuseOperationsMockState *state_{nullptr};
    CloudDiskFuseController *controller_{nullptr};
    struct fuse_req request_ {};
};

/*
 * @tc.name: Lookup_001
 * @tc.desc: Verify lookup rejects a request without controller userdata.
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseOperationsTest, Lookup_001, TestSize.Level2)
{
    request_.userdata = nullptr;
    CloudDiskFuseOperations::Lookup(&request_, TEST_PARENT_NODE, TEST_NAME);
    EXPECT_EQ(request_.reply.error, EIO);
    EXPECT_EQ(request_.reply.errorReplyCount, 1U);
    EXPECT_EQ(state_->resolveLookupCallCount, 0U);
}

/*
 * @tc.name: Lookup_002
 * @tc.desc: Verify lookup forwards resolve errors without registering a node.
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseOperationsTest, Lookup_002, TestSize.Level2)
{
    state_->resolveLookupResult = ENOENT;
    CloudDiskFuseOperations::Lookup(&request_, TEST_PARENT_NODE, TEST_NAME);
    EXPECT_EQ(request_.reply.error, ENOENT);
    EXPECT_EQ(state_->resolveLookupCallCount, 1U);
    EXPECT_EQ(state_->lookupParent, TEST_PARENT_NODE);
    EXPECT_EQ(state_->lookupName, TEST_NAME);
    EXPECT_EQ(state_->registerLookupCallCount, 0U);
}

/*
 * @tc.name: Lookup_003
 * @tc.desc: Verify lookup forwards node registration errors.
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseOperationsTest, Lookup_003, TestSize.Level2)
{
    state_->registerLookupResult = ENOMEM;
    CloudDiskFuseOperations::Lookup(&request_, TEST_PARENT_NODE, nullptr);
    EXPECT_EQ(request_.reply.error, ENOMEM);
    EXPECT_TRUE(state_->lookupNameIsNull);
    EXPECT_EQ(state_->registerLookupCallCount, 1U);
    EXPECT_EQ(request_.reply.entryReplyCount, 0U);
}

/*
 * @tc.name: Lookup_004
 * @tc.desc: Verify lookup rolls the node back when the FUSE entry reply fails.
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseOperationsTest, Lookup_004, TestSize.Level2)
{
    state_->registerLookupNodeId = TEST_NODE;
    state_->lookupAttr.st_ino = TEST_INODE;
    request_.reply.entryReplyResult = -EIO;
    CloudDiskFuseOperations::Lookup(&request_, TEST_PARENT_NODE, TEST_NAME);
    EXPECT_EQ(request_.reply.entryReplyCount, 1U);
    EXPECT_EQ(request_.reply.entry.ino, TEST_NODE);
    EXPECT_EQ(request_.reply.entry.attr.st_ino, TEST_INODE);
    EXPECT_DOUBLE_EQ(request_.reply.entry.attr_timeout, 1.0);
    EXPECT_DOUBLE_EQ(request_.reply.entry.entry_timeout, 1.0);
    EXPECT_EQ(state_->rollbackLookupCallCount, 1U);
    EXPECT_EQ(state_->rollbackLookupNodeId, TEST_NODE);
}

/*
 * @tc.name: Lookup_005
 * @tc.desc: Verify successful lookup keeps the registered node and enters the success log branch.
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseOperationsTest, Lookup_005, TestSize.Level1)
{
    state_->registerLookupNodeId = TEST_NODE;
    CloudDiskFuseOperations::Lookup(&request_, TEST_PARENT_NODE, TEST_NAME);
    EXPECT_EQ(request_.reply.entryReplyCount, 1U);
    EXPECT_EQ(state_->rollbackLookupCallCount, 0U);
    EXPECT_EQ(state_->infoLogCallCount, 1U);
}

/*
 * @tc.name: Lookup_006
 * @tc.desc: Verify successful lookup suppresses logging after the operation log limit.
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseOperationsTest, Lookup_006, TestSize.Level2)
{
    state_->lookupLogCountSeed = OPERATION_LOG_LIMIT;
    controller_ = &GetCloudDiskFuseOperationsMockController();
    request_.userdata = controller_;
    CloudDiskFuseOperations::Lookup(&request_, TEST_PARENT_NODE, TEST_NAME);
    EXPECT_EQ(request_.reply.entryReplyCount, 1U);
    EXPECT_EQ(state_->rollbackLookupCallCount, 0U);
    EXPECT_EQ(state_->infoLogCallCount, 0U);
}

/*
 * @tc.name: Forget_001
 * @tc.desc: Verify forget replies without dereferencing missing userdata.
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseOperationsTest, Forget_001, TestSize.Level2)
{
    request_.userdata = nullptr;
    CloudDiskFuseOperations::Forget(&request_, TEST_NODE, TEST_LOOKUP_COUNT);
    EXPECT_EQ(request_.reply.noneReplyCount, 1U);
    EXPECT_EQ(state_->forgetNodeCallCount, 0U);
}

/*
 * @tc.name: Forget_002
 * @tc.desc: Verify forget forwards node and lookup count to the controller.
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseOperationsTest, Forget_002, TestSize.Level1)
{
    CloudDiskFuseOperations::Forget(&request_, TEST_NODE, TEST_LOOKUP_COUNT);
    EXPECT_EQ(request_.reply.noneReplyCount, 1U);
    EXPECT_EQ(state_->forgetNodeCallCount, 1U);
    EXPECT_EQ(state_->forgetNodeId, TEST_NODE);
    EXPECT_EQ(state_->forgetNodeLookupCount, TEST_LOOKUP_COUNT);
    EXPECT_EQ(state_->infoLogCallCount, 1U);
}

/*
 * @tc.name: Forget_003
 * @tc.desc: Verify forget suppresses logging after the operation log limit.
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseOperationsTest, Forget_003, TestSize.Level2)
{
    state_->forgetLogCountSeed = OPERATION_LOG_LIMIT;
    controller_ = &GetCloudDiskFuseOperationsMockController();
    request_.userdata = controller_;
    CloudDiskFuseOperations::Forget(&request_, TEST_NODE, TEST_LOOKUP_COUNT);
    EXPECT_EQ(request_.reply.noneReplyCount, 1U);
    EXPECT_EQ(state_->forgetNodeCallCount, 1U);
    EXPECT_EQ(state_->infoLogCallCount, 0U);
}

/*
 * @tc.name: GetAttr_001
 * @tc.desc: Verify getattr rejects a request without controller userdata.
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseOperationsTest, GetAttr_001, TestSize.Level2)
{
    request_.userdata = nullptr;
    CloudDiskFuseOperations::GetAttr(&request_, TEST_NODE, nullptr);
    EXPECT_EQ(request_.reply.error, EIO);
    EXPECT_EQ(state_->getAttrNodeCallCount, 0U);
}

/*
 * @tc.name: GetAttr_002
 * @tc.desc: Verify getattr returns the synthetic root directory attributes.
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseOperationsTest, GetAttr_002, TestSize.Level1)
{
    struct fuse_file_info fileInfo {};
    CloudDiskFuseOperations::GetAttr(&request_, FUSE_ROOT_ID, &fileInfo);
    EXPECT_EQ(request_.reply.attrReplyCount, 1U);
    EXPECT_EQ(request_.reply.attr.st_ino, FUSE_ROOT_ID);
    EXPECT_TRUE(S_ISDIR(request_.reply.attr.st_mode));
    EXPECT_EQ(request_.reply.attr.st_mode & 0777, 0550U);
    EXPECT_EQ(request_.reply.attr.st_nlink, 2U);
    EXPECT_DOUBLE_EQ(request_.reply.attrTimeout, 0.0);
}

/*
 * @tc.name: GetAttr_003
 * @tc.desc: Verify getattr forwards controller lookup errors.
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseOperationsTest, GetAttr_003, TestSize.Level2)
{
    state_->getAttrNodeResult = ESTALE;
    CloudDiskFuseOperations::GetAttr(&request_, TEST_NODE, nullptr);
    EXPECT_EQ(request_.reply.error, ESTALE);
    EXPECT_EQ(state_->getAttrNodeId, TEST_NODE);
    EXPECT_EQ(state_->lstatCallCount, 0U);
}

/*
 * @tc.name: GetAttr_004
 * @tc.desc: Verify getattr returns the lstat error for a missing backing path.
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseOperationsTest, GetAttr_004, TestSize.Level2)
{
    state_->nodePath = TEST_PATH;
    state_->mockLstat = true;
    state_->lstatResult = -1;
    state_->lstatError = ENOENT;
    CloudDiskFuseOperations::GetAttr(&request_, TEST_NODE, nullptr);
    EXPECT_EQ(request_.reply.error, ENOENT);
    EXPECT_EQ(state_->lstatCallCount, 1U);
    EXPECT_EQ(state_->lstatPath, TEST_PATH);
}

/*
 * @tc.name: GetAttr_005
 * @tc.desc: Verify getattr rejects a non-regular non-directory backing object as stale.
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseOperationsTest, GetAttr_005, TestSize.Level2)
{
    SetRegularLstatResult();
    state_->lstatAttr.st_mode = S_IFLNK | 0777;
    CloudDiskFuseOperations::GetAttr(&request_, TEST_NODE, nullptr);
    EXPECT_EQ(request_.reply.error, ESTALE);
}

/*
 * @tc.name: GetAttr_006
 * @tc.desc: Verify getattr rejects a backing object whose device changed.
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseOperationsTest, GetAttr_006, TestSize.Level2)
{
    SetRegularLstatResult();
    state_->nodeDevice = TEST_DEVICE + 1;
    CloudDiskFuseOperations::GetAttr(&request_, TEST_NODE, nullptr);
    EXPECT_EQ(request_.reply.error, ESTALE);
}

/*
 * @tc.name: GetAttr_007
 * @tc.desc: Verify getattr rejects a backing object whose inode changed.
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseOperationsTest, GetAttr_007, TestSize.Level2)
{
    SetRegularLstatResult();
    state_->nodeInode = TEST_INODE + 1;
    CloudDiskFuseOperations::GetAttr(&request_, TEST_NODE, nullptr);
    EXPECT_EQ(request_.reply.error, ESTALE);
}

/*
 * @tc.name: GetAttr_008
 * @tc.desc: Verify getattr replies with current regular-file attributes and logs success.
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseOperationsTest, GetAttr_008, TestSize.Level1)
{
    SetRegularLstatResult();
    CloudDiskFuseOperations::GetAttr(&request_, TEST_NODE, nullptr);
    EXPECT_EQ(request_.reply.attrReplyCount, 1U);
    EXPECT_EQ(request_.reply.attr.st_dev, TEST_DEVICE);
    EXPECT_EQ(request_.reply.attr.st_ino, TEST_INODE);
    EXPECT_DOUBLE_EQ(request_.reply.attrTimeout, 0.0);
    EXPECT_EQ(state_->infoLogCallCount, 1U);
}

/*
 * @tc.name: GetAttr_009
 * @tc.desc: Verify getattr does not enter success logging when the FUSE reply fails.
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseOperationsTest, GetAttr_009, TestSize.Level2)
{
    SetRegularLstatResult();
    request_.reply.attrReplyResult = -EIO;
    CloudDiskFuseOperations::GetAttr(&request_, TEST_NODE, nullptr);
    EXPECT_EQ(request_.reply.attrReplyCount, 1U);
    EXPECT_EQ(state_->infoLogCallCount, 0U);
}

/*
 * @tc.name: GetAttr_010
 * @tc.desc: Verify getattr suppresses success logging after the operation log limit.
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseOperationsTest, GetAttr_010, TestSize.Level2)
{
    SetRegularLstatResult();
    state_->getattrLogCountSeed = OPERATION_LOG_LIMIT;
    controller_ = &GetCloudDiskFuseOperationsMockController();
    request_.userdata = controller_;
    CloudDiskFuseOperations::GetAttr(&request_, TEST_NODE, nullptr);
    EXPECT_EQ(request_.reply.attrReplyCount, 1U);
    EXPECT_EQ(state_->infoLogCallCount, 0U);
}

/*
 * @tc.name: OpenLogLimit_001
 * @tc.desc: Verify timed open logging accepts only the configured number of events per window.
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseOperationsTest, OpenLogLimit_001, TestSize.Level2)
{
    for (uint32_t index = 0; index <= TIMED_OPERATION_LOG_LIMIT; ++index) {
        CloudDiskFuseOperations::Open(&request_, TEST_NODE, nullptr);
    }
    EXPECT_EQ(request_.reply.errorReplyCount, TIMED_OPERATION_LOG_LIMIT + 1);
    EXPECT_EQ(state_->getIdentityCallCount, TIMED_OPERATION_LOG_LIMIT + 1);
    EXPECT_EQ(state_->infoLogCallCount, TIMED_OPERATION_LOG_LIMIT);
}

/*
 * @tc.name: Open_001
 * @tc.desc: Verify open rejects a request without controller userdata.
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseOperationsTest, Open_001, TestSize.Level2)
{
    request_.userdata = nullptr;
    struct fuse_file_info fileInfo {};
    CloudDiskFuseOperations::Open(&request_, TEST_NODE, &fileInfo);
    EXPECT_EQ(request_.reply.error, EINVAL);
    EXPECT_EQ(state_->getIdentityCallCount, 0U);
}

/*
 * @tc.name: Open_002
 * @tc.desc: Verify open rejects a null file information pointer.
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseOperationsTest, Open_002, TestSize.Level2)
{
    CloudDiskFuseOperations::Open(&request_, TEST_NODE, nullptr);
    EXPECT_EQ(request_.reply.error, EINVAL);
    EXPECT_EQ(state_->reserveOpenCallCount, 0U);
}

/*
 * @tc.name: Open_003
 * @tc.desc: Verify open rejects directory requests.
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseOperationsTest, Open_003, TestSize.Level2)
{
    struct fuse_file_info fileInfo {};
    fileInfo.flags = O_RDONLY | O_DIRECTORY;
    CloudDiskFuseOperations::Open(&request_, TEST_NODE, &fileInfo);
    EXPECT_EQ(request_.reply.error, ENOTDIR);
    EXPECT_EQ(state_->reserveOpenCallCount, 0U);
}

/*
 * @tc.name: Open_004
 * @tc.desc: Verify open rejects writable access.
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseOperationsTest, Open_004, TestSize.Level2)
{
    struct fuse_file_info fileInfo {};
    fileInfo.flags = O_WRONLY;
    CloudDiskFuseOperations::Open(&request_, TEST_NODE, &fileInfo);
    EXPECT_EQ(request_.reply.error, EROFS);
}

/*
 * @tc.name: Open_005
 * @tc.desc: Verify open rejects mutating flags even with read-only access.
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseOperationsTest, Open_005, TestSize.Level2)
{
    struct fuse_file_info fileInfo {};
    fileInfo.flags = O_RDONLY | O_TRUNC;
    CloudDiskFuseOperations::Open(&request_, TEST_NODE, &fileInfo);
    EXPECT_EQ(request_.reply.error, EROFS);
}

/*
 * @tc.name: Open_006
 * @tc.desc: Verify open forwards reservation errors.
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseOperationsTest, Open_006, TestSize.Level2)
{
    state_->reserveOpenResult = ESTALE;
    struct fuse_file_info fileInfo {};
    fileInfo.flags = O_RDONLY;
    CloudDiskFuseOperations::Open(&request_, TEST_NODE, &fileInfo);
    EXPECT_EQ(request_.reply.error, ESTALE);
    EXPECT_EQ(state_->openBackingFileCallCount, 0U);
}

/*
 * @tc.name: Open_007
 * @tc.desc: Verify open releases its reservation when opening the backing file fails.
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseOperationsTest, Open_007, TestSize.Level2)
{
    state_->openBackingFileResult = EACCES;
    struct fuse_file_info fileInfo {};
    fileInfo.flags = O_RDONLY;
    CloudDiskFuseOperations::Open(&request_, TEST_NODE, &fileInfo);
    EXPECT_EQ(request_.reply.error, EACCES);
    EXPECT_EQ(state_->releaseOpenReservationCallCount, 1U);
    EXPECT_EQ(state_->commitOpenCallCount, 0U);
}

/*
 * @tc.name: Open_008
 * @tc.desc: Verify open forwards commit errors without replying open.
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseOperationsTest, Open_008, TestSize.Level2)
{
    state_->commitOpenResult = EBUSY;
    struct fuse_file_info fileInfo {};
    fileInfo.flags = O_RDONLY;
    CloudDiskFuseOperations::Open(&request_, TEST_NODE, &fileInfo);
    EXPECT_EQ(request_.reply.error, EBUSY);
    EXPECT_EQ(request_.reply.openReplyCount, 0U);
}

/*
 * @tc.name: Open_009
 * @tc.desc: Verify open rolls a committed handle back when a negative FUSE reply fails.
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseOperationsTest, Open_009, TestSize.Level2)
{
    state_->committedHandle = TEST_HANDLE;
    request_.reply.openReplyResult = -EIO;
    struct fuse_file_info fileInfo {};
    fileInfo.flags = O_RDONLY;
    CloudDiskFuseOperations::Open(&request_, TEST_NODE, &fileInfo);
    EXPECT_EQ(fileInfo.fh, TEST_HANDLE);
    EXPECT_EQ(state_->rollbackOpenCallCount, 1U);
    EXPECT_EQ(state_->rollbackOpenHandle, TEST_HANDLE);
}

/*
 * @tc.name: Open_010
 * @tc.desc: Verify open rolls a committed handle back when a positive FUSE reply fails.
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseOperationsTest, Open_010, TestSize.Level2)
{
    state_->committedHandle = TEST_HANDLE;
    request_.reply.openReplyResult = EIO;
    struct fuse_file_info fileInfo {};
    fileInfo.flags = O_RDONLY;
    CloudDiskFuseOperations::Open(&request_, TEST_NODE, &fileInfo);
    EXPECT_EQ(state_->rollbackOpenCallCount, 1U);
    EXPECT_EQ(state_->rollbackOpenHandle, TEST_HANDLE);
}

/*
 * @tc.name: Open_011
 * @tc.desc: Verify successful open returns the committed handle without rollback.
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseOperationsTest, Open_011, TestSize.Level1)
{
    state_->identityUserId = TEST_USER_ID;
    state_->reserveOpenSyncFolderIndex = TEST_ROOT_INDEX;
    state_->reserveOpenGeneration = TEST_GENERATION;
    state_->committedHandle = TEST_HANDLE;
    struct fuse_file_info fileInfo {};
    fileInfo.flags = O_RDONLY;
    CloudDiskFuseOperations::Open(&request_, TEST_NODE, &fileInfo);
    EXPECT_EQ(request_.reply.openReplyCount, 1U);
    EXPECT_EQ(request_.reply.fileInfo.fh, TEST_HANDLE);
    EXPECT_EQ(state_->reserveOpenNodeId, TEST_NODE);
    EXPECT_EQ(state_->commitOpenNodeId, TEST_NODE);
    EXPECT_EQ(state_->rollbackOpenCallCount, 0U);
}

/*
 * @tc.name: Release_001
 * @tc.desc: Verify release rejects a request without controller userdata.
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseOperationsTest, Release_001, TestSize.Level2)
{
    request_.userdata = nullptr;
    struct fuse_file_info fileInfo {};
    fileInfo.fh = TEST_HANDLE;
    CloudDiskFuseOperations::Release(&request_, TEST_NODE, &fileInfo);
    EXPECT_EQ(request_.reply.error, EBADF);
    EXPECT_EQ(state_->getIdentityCallCount, 0U);
}

/*
 * @tc.name: ReleaseLogLimit_001
 * @tc.desc: Verify timed release logging accepts only the configured number of events per window.
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseOperationsTest, ReleaseLogLimit_001, TestSize.Level2)
{
    for (uint32_t index = 0; index <= TIMED_OPERATION_LOG_LIMIT; ++index) {
        CloudDiskFuseOperations::Release(&request_, TEST_NODE, nullptr);
    }
    EXPECT_EQ(request_.reply.errorReplyCount, TIMED_OPERATION_LOG_LIMIT + 1);
    EXPECT_EQ(state_->getIdentityCallCount, TIMED_OPERATION_LOG_LIMIT + 1);
    EXPECT_EQ(state_->infoLogCallCount, TIMED_OPERATION_LOG_LIMIT);
}

/*
 * @tc.name: Release_002
 * @tc.desc: Verify release rejects a null file information pointer.
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseOperationsTest, Release_002, TestSize.Level2)
{
    CloudDiskFuseOperations::Release(&request_, TEST_NODE, nullptr);
    EXPECT_EQ(request_.reply.error, EBADF);
    EXPECT_EQ(state_->takeOpenContextCallCount, 0U);
}

/*
 * @tc.name: Release_003
 * @tc.desc: Verify release rejects a zero handle.
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseOperationsTest, Release_003, TestSize.Level2)
{
    struct fuse_file_info fileInfo {};
    CloudDiskFuseOperations::Release(&request_, TEST_NODE, &fileInfo);
    EXPECT_EQ(request_.reply.error, EBADF);
    EXPECT_EQ(state_->takeOpenContextCallCount, 0U);
}

/*
 * @tc.name: Release_004
 * @tc.desc: Verify release forwards controller context errors.
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseOperationsTest, Release_004, TestSize.Level2)
{
    state_->takeOpenContextResult = ESTALE;
    struct fuse_file_info fileInfo {};
    fileInfo.fh = TEST_HANDLE;
    CloudDiskFuseOperations::Release(&request_, TEST_NODE, &fileInfo);
    EXPECT_EQ(request_.reply.error, ESTALE);
    EXPECT_EQ(state_->takeOpenNodeId, TEST_NODE);
    EXPECT_EQ(state_->takeOpenHandle, TEST_HANDLE);
    EXPECT_EQ(state_->closeCallCount, 0U);
}

/*
 * @tc.name: Release_005
 * @tc.desc: Verify release succeeds without close when the stored descriptor is invalid.
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseOperationsTest, Release_005, TestSize.Level1)
{
    state_->releaseFd = -1;
    struct fuse_file_info fileInfo {};
    fileInfo.fh = TEST_HANDLE;
    CloudDiskFuseOperations::Release(&request_, TEST_NODE, &fileInfo);
    EXPECT_EQ(request_.reply.error, 0);
    EXPECT_EQ(state_->closeCallCount, 0U);
}

/*
 * @tc.name: Release_006
 * @tc.desc: Verify release closes a valid stored descriptor and replies success.
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseOperationsTest, Release_006, TestSize.Level1)
{
    state_->releaseFd = 51;
    state_->mockClose = true;
    state_->closeFd = 51;
    struct fuse_file_info fileInfo {};
    fileInfo.fh = TEST_HANDLE;
    CloudDiskFuseOperations::Release(&request_, TEST_NODE, &fileInfo);
    EXPECT_EQ(request_.reply.error, 0);
    EXPECT_EQ(state_->closeCallCount, 1U);
}

/*
 * @tc.name: Release_007
 * @tc.desc: Verify release returns errno when closing the stored descriptor fails.
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseOperationsTest, Release_007, TestSize.Level2)
{
    state_->releaseFd = 52;
    state_->mockClose = true;
    state_->closeFd = 52;
    state_->closeResult = -1;
    state_->closeError = EIO;
    struct fuse_file_info fileInfo {};
    fileInfo.fh = TEST_HANDLE;
    CloudDiskFuseOperations::Release(&request_, TEST_NODE, &fileInfo);
    EXPECT_EQ(request_.reply.error, EIO);
    EXPECT_EQ(state_->closeCallCount, 1U);
}

} // namespace OHOS::FileManagement::CloudDiskService::Test
