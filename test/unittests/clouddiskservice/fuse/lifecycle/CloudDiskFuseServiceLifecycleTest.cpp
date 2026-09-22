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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cerrno>
#include <new>

#include "CloudDiskFuseLifecycleMock.h"
#include "CloudDiskServiceSyncFolderLifecycleMock.h"
#include "cloud_disk_service.h"
#include "cloud_disk_service_access_token_mock.h"
#include "cloud_disk_service_error.h"
#include "cloud_disk_sync_folder.h"
#include "cloud_disk_sync_folder_manager_mock.h"
#include "cloud_file_utils.h"

namespace OHOS {

bool g_cloudDiskFusePublishResult = false;

bool SystemAbility::Publish(sptr<IRemoteObject> systemAbility)
{
    (void)systemAbility;
    return g_cloudDiskFusePublishResult;
}

} // namespace OHOS

namespace OHOS::FileManagement::CloudDiskService::Test {
using namespace testing;
using namespace testing::ext;

namespace {
constexpr int32_t TEST_USER_ID = 100;
constexpr int32_t CLOUD_DISK_SERVICE_SA_ID = 5207;
constexpr uint32_t ROOT_ONE = 11;
constexpr uint32_t ROOT_TWO = 22;
const std::string TEST_BUNDLE = "com.example.fuse";
const std::string SANDBOX_ROOT = "/storage/Users/currentUser/Download/fuse";
const std::string PHYSICAL_ROOT = "/data/service/el2/100/hmdfs/account/files/Docs/Download/fuse";
} // namespace

class CloudDiskFuseServiceLifecycleTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        service_ = new (std::nothrow) CloudDiskService(CLOUD_DISK_SERVICE_SA_ID, true);
        ASSERT_NE(service_, nullptr);
        accessToken_ = std::make_shared<CloudDiskServiceAccessTokenMock>();
        CloudDiskServiceAccessTokenVirtual::dfsuAccessToken = accessToken_;
    }

    static void TearDownTestCase()
    {
        CloudDiskServiceAccessTokenVirtual::dfsuAccessToken = nullptr;
        accessToken_.reset();
        service_ = nullptr;
    }

    void SetUp() override
    {
        g_cloudDiskFusePublishResult = false;
        ResetCloudDiskFuseLifecycleTrace();
        ResetCloudDiskServiceSyncFolderLifecycleMock();
        CloudDiskSyncFolder::GetInstance().ClearMap();
    }

    void TearDown() override
    {
        CloudDiskSyncFolder::GetInstance().ClearMap();
        Mock::VerifyAndClearExpectations(accessToken_.get());
        Mock::VerifyAndClearExpectations(&CloudDiskSyncFolderManagerMock::GetInstance());
    }

    static inline sptr<CloudDiskService> service_;
    static inline std::shared_ptr<CloudDiskServiceAccessTokenMock> accessToken_;
};

/*
 * @tc.name: StartFuseForUser_001
 * @tc.desc: Verify service startup forwards every registered root and installs the idle callback
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseServiceLifecycleTest, StartFuseForUser_001, TestSize.Level1)
{
    CloudDiskSyncFolder::GetInstance().AddSyncFolder(ROOT_ONE, {"bundle.one", "/root/one"});
    CloudDiskSyncFolder::GetInstance().AddSyncFolder(ROOT_TWO, {"bundle.two", "/root/two"});

    service_->StartFuseForUser(TEST_USER_ID);

    const auto &trace = GetCloudDiskFuseLifecycleTrace();
    EXPECT_EQ(trace.startUserCalls, 1U);
    EXPECT_EQ(trace.setIdleCallbackCalls, 1U);
    EXPECT_EQ(trace.userId, TEST_USER_ID);
    ASSERT_EQ(trace.roots.size(), 2U);
    EXPECT_EQ(trace.roots.at(ROOT_ONE), "/root/one");
    EXPECT_EQ(trace.roots.at(ROOT_TWO), "/root/two");
    EXPECT_TRUE(static_cast<bool>(trace.idleCallback));
}

/*
 * @tc.name: StopFuseForServiceExit_001
 * @tc.desc: Verify service exit clears the callback before stopping FUSE
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseServiceLifecycleTest, StopFuseForServiceExit_001, TestSize.Level1)
{
    service_->StartFuseForUser(TEST_USER_ID);
    service_->StopFuseForServiceExit();

    const auto &trace = GetCloudDiskFuseLifecycleTrace();
    EXPECT_EQ(trace.setIdleCallbackCalls, 2U);
    EXPECT_EQ(trace.stopForServiceExitCalls, 1U);
    EXPECT_FALSE(static_cast<bool>(trace.idleCallback));
}

/*
 * @tc.name: RegisterSyncFolderInner_001
 * @tc.desc: Verify a successfully registered sync folder is added to FUSE
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseServiceLifecycleTest, RegisterSyncFolderInner_001, TestSize.Level1)
{
    EXPECT_CALL(*accessToken_, CheckCallerPermission(_)).WillOnce(Return(true));

    int32_t result = service_->RegisterSyncFolderInner(TEST_USER_ID, TEST_BUNDLE, SANDBOX_ROOT);

    const auto &trace = GetCloudDiskFuseLifecycleTrace();
    EXPECT_EQ(result, E_OK);
    EXPECT_EQ(GetRegisterSyncFolderCalls(), 1U);
    EXPECT_EQ(trace.addRootCalls, 1U);
    EXPECT_EQ(trace.userId, TEST_USER_ID);
    EXPECT_EQ(trace.path, PHYSICAL_ROOT);
    EXPECT_EQ(trace.syncFolderIndex, CloudDisk::CloudFileUtils::DentryHash(PHYSICAL_ROOT));
}

/*
 * @tc.name: RegisterSyncFolderInner_002
 * @tc.desc: Verify a failed sync-folder registration is not exposed to FUSE
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseServiceLifecycleTest, RegisterSyncFolderInner_002, TestSize.Level2)
{
    constexpr int32_t REGISTER_ERROR = EIO;
    EXPECT_CALL(*accessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
    SetRegisterSyncFolderResult(REGISTER_ERROR);

    int32_t result = service_->RegisterSyncFolderInner(TEST_USER_ID, TEST_BUNDLE, SANDBOX_ROOT);

    EXPECT_EQ(result, REGISTER_ERROR);
    EXPECT_EQ(GetCloudDiskFuseLifecycleTrace().addRootCalls, 0U);
}

/*
 * @tc.name: UnregisterSyncFolderInner_001
 * @tc.desc: Verify FUSE root removal is rolled back when unregistering storage metadata fails
 * @tc.type: RELI
 */
HWTEST_F(CloudDiskFuseServiceLifecycleTest, UnregisterSyncFolderInner_001, TestSize.Level2)
{
    constexpr int32_t UNREGISTER_ERROR = EIO;
    EXPECT_CALL(*accessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
    SetUnregisterSyncFolderResult(UNREGISTER_ERROR);

    int32_t result = service_->UnregisterSyncFolderInner(TEST_USER_ID, TEST_BUNDLE, SANDBOX_ROOT);

    const auto &trace = GetCloudDiskFuseLifecycleTrace();
    EXPECT_EQ(result, UNREGISTER_ERROR);
    EXPECT_EQ(GetUnregisterSyncFolderCalls(), 1U);
    EXPECT_EQ(trace.removeRootCalls, 1U);
    EXPECT_EQ(trace.addRootCalls, 1U);
    EXPECT_EQ(trace.path, PHYSICAL_ROOT);
    EXPECT_EQ(trace.syncFolderIndex, CloudDisk::CloudFileUtils::DentryHash(PHYSICAL_ROOT));
}

/*
 * @tc.name: UnregisterSyncFolderInner_002
 * @tc.desc: Verify successful unregistration removes the FUSE root without rolling it back
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseServiceLifecycleTest, UnregisterSyncFolderInner_002, TestSize.Level1)
{
    EXPECT_CALL(*accessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
    auto syncFolderIndex = CloudDisk::CloudFileUtils::DentryHash(PHYSICAL_ROOT);
    CloudDiskSyncFolder::GetInstance().AddSyncFolder(syncFolderIndex, {TEST_BUNDLE, PHYSICAL_ROOT});
    GetCloudDiskFuseLifecycleTrace().canUnload = false;

    int32_t result = service_->UnregisterSyncFolderInner(TEST_USER_ID, TEST_BUNDLE, SANDBOX_ROOT);

    const auto &trace = GetCloudDiskFuseLifecycleTrace();
    EXPECT_EQ(result, E_OK);
    EXPECT_EQ(GetUnregisterSyncFolderCalls(), 1U);
    EXPECT_EQ(trace.removeRootCalls, 1U);
    EXPECT_EQ(trace.addRootCalls, 0U);
    EXPECT_EQ(trace.userId, TEST_USER_ID);
    EXPECT_EQ(trace.syncFolderIndex, syncFolderIndex);
    EXPECT_EQ(trace.canUnloadCalls, 1U);
}

/*
 * @tc.name: UnregisterForSaInner_001
 * @tc.desc: Verify successful SA unregistration removes the matching FUSE root
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseServiceLifecycleTest, UnregisterForSaInner_001, TestSize.Level1)
{
    auto syncFolderIndex = CloudDisk::CloudFileUtils::DentryHash(PHYSICAL_ROOT);
    CloudDiskSyncFolder::GetInstance().AddSyncFolder(syncFolderIndex, {TEST_BUNDLE, PHYSICAL_ROOT});
    GetCloudDiskFuseLifecycleTrace().canUnload = false;
    EXPECT_CALL(*accessToken_, GetUserId()).WillOnce(Return(TEST_USER_ID));
    EXPECT_CALL(CloudDiskSyncFolderManagerMock::GetInstance(), UnregisterForSa(_)).WillOnce(Return(E_OK));

    int32_t result = service_->UnregisterForSaInner(PHYSICAL_ROOT);

    const auto &trace = GetCloudDiskFuseLifecycleTrace();
    EXPECT_EQ(result, E_OK);
    EXPECT_EQ(trace.removeRootCalls, 1U);
    EXPECT_EQ(trace.addRootCalls, 0U);
    EXPECT_EQ(trace.userId, TEST_USER_ID);
    EXPECT_EQ(trace.syncFolderIndex, syncFolderIndex);
    EXPECT_EQ(trace.canUnloadCalls, 1U);
}

/*
 * @tc.name: OnStart_001
 * @tc.desc: Verify publish failure rolls back the FUSE lifecycle started during service startup
 * @tc.type: RELI
 */
HWTEST_F(CloudDiskFuseServiceLifecycleTest, OnStart_001, TestSize.Level2)
{
    constexpr char START_REASON[] = "fuse.lifecycle.test";
    service_->state_ = ServiceRunningState::STATE_NOT_START;
    service_->registerToService_ = false;
    EXPECT_CALL(*accessToken_, GetUserId()).WillOnce(Return(TEST_USER_ID));
    EXPECT_CALL(CloudDiskSyncFolderManagerMock::GetInstance(), GetAllSyncFoldersForSa(_)).WillOnce(Return(E_OK));
    SystemAbilityOnDemandReason reason;
    reason.SetName(START_REASON);

    service_->OnStart(reason);

    const auto &trace = GetCloudDiskFuseLifecycleTrace();
    EXPECT_EQ(trace.startUserCalls, 1U);
    EXPECT_EQ(trace.stopForServiceExitCalls, 1U);
    EXPECT_EQ(trace.setIdleCallbackCalls, 2U);
    EXPECT_FALSE(static_cast<bool>(trace.idleCallback));
    ASSERT_EQ(trace.events.size(), 4U);
    EXPECT_EQ(trace.events[0], "SetIdleCallback");
    EXPECT_EQ(trace.events[1], "StartUser");
    EXPECT_EQ(trace.events[2], "ClearIdleCallback");
    EXPECT_EQ(trace.events[3], "StopForServiceExit");
    EXPECT_EQ(service_->state_, ServiceRunningState::STATE_NOT_START);
    EXPECT_FALSE(service_->registerToService_);
}

/*
 * @tc.name: UnloadSa_001
 * @tc.desc: Verify service unload is deferred while the FUSE lifecycle is busy
 * @tc.type: RELI
 */
HWTEST_F(CloudDiskFuseServiceLifecycleTest, UnloadSa_001, TestSize.Level2)
{
    GetCloudDiskFuseLifecycleTrace().canUnload = false;

    service_->UnloadSa();

    EXPECT_EQ(GetCloudDiskFuseLifecycleTrace().canUnloadCalls, 1U);
}

/*
 * @tc.name: OnStop_001
 * @tc.desc: Verify stopping the service also performs the FUSE service-exit teardown
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskFuseServiceLifecycleTest, OnStop_001, TestSize.Level1)
{
    service_->state_ = ServiceRunningState::STATE_RUNNING;
    service_->registerToService_ = true;

    service_->OnStop();

    EXPECT_EQ(GetCloudDiskFuseLifecycleTrace().stopForServiceExitCalls, 1U);
    EXPECT_EQ(service_->state_, ServiceRunningState::STATE_NOT_START);
    EXPECT_FALSE(service_->registerToService_);
}

} // namespace OHOS::FileManagement::CloudDiskService::Test
