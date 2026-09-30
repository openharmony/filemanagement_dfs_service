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
#include <gtest/gtest.h>

#include "CloudDiskFuseMountAdapterMock.h"
#include "cloud_disk_fuse_mount_adapter.h"

namespace OHOS::FileManagement::CloudDiskService::Test {
using namespace testing;
using namespace testing::ext;

namespace {
constexpr int32_t TEST_USER_ID = 100;
constexpr int32_t TEST_FUSE_FD = 51;
constexpr char TEST_MOUNT_POINT[] = "/mnt/data/100/cloud_disk_fuse";
} // namespace

class CloudDiskFuseMountAdapterTest : public testing::Test {
public:
    void SetUp() override
    {
        ResetCloudDiskFuseMountAdapterMock();
        state_ = &GetCloudDiskFuseMountAdapterMockState();
    }

    void TearDown() override
    {
        ResetCloudDiskFuseMountAdapterMock();
        state_ = nullptr;
    }

protected:
    CloudDiskFuseMountAdapterMockState *state_{nullptr};
};

/*
 * @tc.name: Mount_001
 * @tc.desc: Verify mount returns EIO and resets the fd when the SA manager is unavailable.
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseMountAdapterTest, Mount_001, TestSize.Level2)
{
    state_->systemAbilityManagerAvailable = false;
    int fuseFd = TEST_FUSE_FD;
    EXPECT_EQ(CloudDiskFuseMountAdapter::Mount(TEST_USER_ID, TEST_MOUNT_POINT, fuseFd), -EIO);
    EXPECT_EQ(fuseFd, -1);
    EXPECT_EQ(state_->getSystemAbilityManagerCallCount, 1U);
    EXPECT_EQ(state_->getSystemAbilityCallCount, 0U);
    EXPECT_EQ(state_->errorLogCallCount, 1U);
}

/*
 * @tc.name: Mount_002
 * @tc.desc: Verify mount returns EIO when StorageManager system ability is absent.
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseMountAdapterTest, Mount_002, TestSize.Level2)
{
    state_->remoteKind = StorageManagerRemoteKind::NONE;
    int fuseFd = TEST_FUSE_FD;
    EXPECT_EQ(CloudDiskFuseMountAdapter::Mount(TEST_USER_ID, TEST_MOUNT_POINT, fuseFd), -EIO);
    EXPECT_EQ(fuseFd, -1);
    EXPECT_EQ(state_->requestedSystemAbilityId, STORAGE_MANAGER_MANAGER_ID);
    EXPECT_EQ(state_->mountCallCount, 0U);
    EXPECT_EQ(state_->errorLogCallCount, 1U);
}

/*
 * @tc.name: Mount_003
 * @tc.desc: Verify mount returns EIO when the remote object cannot be cast to StorageManager.
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseMountAdapterTest, Mount_003, TestSize.Level2)
{
    state_->remoteKind = StorageManagerRemoteKind::WRONG_INTERFACE;
    int fuseFd = TEST_FUSE_FD;
    EXPECT_EQ(CloudDiskFuseMountAdapter::Mount(TEST_USER_ID, TEST_MOUNT_POINT, fuseFd), -EIO);
    EXPECT_EQ(fuseFd, -1);
    EXPECT_EQ(state_->mountCallCount, 0U);
    EXPECT_EQ(state_->errorLogCallCount, 1U);
}

/*
 * @tc.name: Mount_004
 * @tc.desc: Verify mount forwards IPC errors when no descriptor was returned.
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseMountAdapterTest, Mount_004, TestSize.Level2)
{
    state_->mountResult = -EPERM;
    state_->mountOutputFd = -1;
    int fuseFd = TEST_FUSE_FD;
    EXPECT_EQ(CloudDiskFuseMountAdapter::Mount(TEST_USER_ID, TEST_MOUNT_POINT, fuseFd), -EPERM);
    EXPECT_EQ(fuseFd, -1);
    EXPECT_EQ(state_->mountCallCount, 1U);
    EXPECT_EQ(state_->mountUserId, TEST_USER_ID);
    EXPECT_EQ(state_->mountPoint, TEST_MOUNT_POINT);
    EXPECT_EQ(state_->closeCallCount, 0U);
    EXPECT_EQ(state_->errorLogCallCount, 1U);
}

/*
 * @tc.name: Mount_005
 * @tc.desc: Verify mount closes and invalidates a descriptor returned together with an IPC error.
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseMountAdapterTest, Mount_005, TestSize.Level2)
{
    state_->mountResult = -EACCES;
    state_->mountOutputFd = TEST_FUSE_FD;
    state_->mockClose = true;
    state_->closeFd = TEST_FUSE_FD;
    int fuseFd = -1;
    EXPECT_EQ(CloudDiskFuseMountAdapter::Mount(TEST_USER_ID, TEST_MOUNT_POINT, fuseFd), -EACCES);
    EXPECT_EQ(fuseFd, -1);
    EXPECT_EQ(state_->closeCallCount, 1U);
    EXPECT_EQ(state_->errorLogCallCount, 1U);
}

/*
 * @tc.name: Mount_006
 * @tc.desc: Verify mount returns EBADF when successful IPC supplies an invalid descriptor.
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseMountAdapterTest, Mount_006, TestSize.Level2)
{
    state_->mountResult = 0;
    state_->mountOutputFd = -1;
    int fuseFd = TEST_FUSE_FD;
    EXPECT_EQ(CloudDiskFuseMountAdapter::Mount(TEST_USER_ID, TEST_MOUNT_POINT, fuseFd), -EBADF);
    EXPECT_EQ(fuseFd, -1);
    EXPECT_EQ(state_->errorLogCallCount, 1U);
}

/*
 * @tc.name: Mount_007
 * @tc.desc: Verify mount returns a valid descriptor and preserves the IPC parameters.
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseMountAdapterTest, Mount_007, TestSize.Level1)
{
    state_->mountOutputFd = TEST_FUSE_FD;
    int fuseFd = -1;
    EXPECT_EQ(CloudDiskFuseMountAdapter::Mount(TEST_USER_ID, TEST_MOUNT_POINT, fuseFd), 0);
    EXPECT_EQ(fuseFd, TEST_FUSE_FD);
    EXPECT_EQ(state_->mountUserId, TEST_USER_ID);
    EXPECT_EQ(state_->mountPoint, TEST_MOUNT_POINT);
    EXPECT_EQ(state_->closeCallCount, 0U);
    EXPECT_EQ(state_->errorLogCallCount, 0U);
}

/*
 * @tc.name: Unmount_001
 * @tc.desc: Verify unmount returns EIO when the StorageManager proxy is unavailable.
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseMountAdapterTest, Unmount_001, TestSize.Level2)
{
    state_->systemAbilityManagerAvailable = false;
    EXPECT_EQ(CloudDiskFuseMountAdapter::Unmount(TEST_USER_ID, TEST_MOUNT_POINT), -EIO);
    EXPECT_EQ(state_->unmountCallCount, 0U);
    EXPECT_EQ(state_->errorLogCallCount, 1U);
}

/*
 * @tc.name: Unmount_002
 * @tc.desc: Verify unmount forwards a successful IPC request and its parameters.
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseMountAdapterTest, Unmount_002, TestSize.Level1)
{
    EXPECT_EQ(CloudDiskFuseMountAdapter::Unmount(TEST_USER_ID, TEST_MOUNT_POINT), 0);
    EXPECT_EQ(state_->unmountCallCount, 1U);
    EXPECT_EQ(state_->unmountUserId, TEST_USER_ID);
    EXPECT_EQ(state_->unmountPoint, TEST_MOUNT_POINT);
    EXPECT_EQ(state_->errorLogCallCount, 0U);
}

/*
 * @tc.name: Unmount_003
 * @tc.desc: Verify unmount returns the StorageManager IPC error unchanged.
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseMountAdapterTest, Unmount_003, TestSize.Level2)
{
    state_->unmountResult = -EBUSY;
    EXPECT_EQ(CloudDiskFuseMountAdapter::Unmount(TEST_USER_ID, TEST_MOUNT_POINT), -EBUSY);
    EXPECT_EQ(state_->unmountCallCount, 1U);
    EXPECT_EQ(state_->unmountUserId, TEST_USER_ID);
    EXPECT_EQ(state_->unmountPoint, TEST_MOUNT_POINT);
    EXPECT_EQ(state_->errorLogCallCount, 1U);
}

} // namespace OHOS::FileManagement::CloudDiskService::Test
