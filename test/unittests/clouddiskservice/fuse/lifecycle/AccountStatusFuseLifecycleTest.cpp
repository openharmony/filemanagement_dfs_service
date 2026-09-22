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

#include "AccountStatusLifecycleMock.h"
#include "CloudDiskFuseLifecycleMock.h"
#include "account_status_listener.h"
#include "cloud_disk_sync_folder.h"
#include "cloud_disk_sync_folder_manager_mock.h"
#include "if_system_ability_manager_mock.h"
#include "system_ability_manager_client_mock.h"

namespace OHOS::AccountSA {

ErrCode OsAccountManager::SubscribeOsAccount(const std::shared_ptr<OsAccountSubscriber> &subscriber)
{
    FileManagement::CloudDiskService::Test::AccountStatusLifecycleMock::RecordSubscribe(subscriber);
    return ERR_OK;
}

ErrCode OsAccountManager::UnsubscribeOsAccount(const std::shared_ptr<OsAccountSubscriber> &subscriber)
{
    FileManagement::CloudDiskService::Test::AccountStatusLifecycleMock::RecordUnsubscribe(subscriber);
    return ERR_OK;
}

} // namespace OHOS::AccountSA

namespace OHOS::FileManagement::CloudDiskService::Test {
using namespace AccountSA;
using namespace testing;
using namespace testing::ext;

namespace {
constexpr int32_t TEST_USER_ID = 100;
constexpr int32_t OTHER_USER_ID = 101;
constexpr int32_t CLOUD_DISK_SERVICE_SA_ID = 5207;
} // namespace

class AccountStatusFuseLifecycleTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        clientMock_ = std::make_shared<SystemAbilityManagerClientMock>();
        ISystemAbilityManagerClient::smc = clientMock_;
    }

    static void TearDownTestCase()
    {
        ISystemAbilityManagerClient::smc = nullptr;
        clientMock_.reset();
    }

    void SetUp() override
    {
        ResetCloudDiskFuseLifecycleTrace();
        AccountStatusLifecycleMock::Reset();
        CloudDiskSyncFolder::GetInstance().ClearMap();
        OsAccountSubscribeInfo subscribeInfo;
        subscriber_ = std::make_shared<AccountStatusSubscriber>(subscribeInfo, TEST_USER_ID);
    }

    void TearDown() override
    {
        CloudDiskSyncFolder::GetInstance().ClearMap();
        subscriber_.reset();
        Mock::VerifyAndClearExpectations(clientMock_.get());
        Mock::VerifyAndClearExpectations(&CloudDiskSyncFolderManagerMock::GetInstance());
        AccountStatusLifecycleMock::Reset();
    }

    static inline std::shared_ptr<SystemAbilityManagerClientMock> clientMock_;
    std::shared_ptr<AccountStatusSubscriber> subscriber_;
};

/*
 * @tc.name: OnStateChanged_001
 * @tc.desc: Verify switching users stops old FUSE and starts the new user's registered roots
 * @tc.type: FUNC
 */
HWTEST_F(AccountStatusFuseLifecycleTest, OnStateChanged_001, TestSize.Level1)
{
    std::vector<SyncFolderExt> folders(1);
    folders[0].path_ = "/storage/Users/currentUser/Download/new-user";
    folders[0].bundleName_ = "com.example.newuser";
    EXPECT_CALL(CloudDiskSyncFolderManagerMock::GetInstance(), GetAllSyncFoldersForSa(_))
        .WillOnce(DoAll(SetArgReferee<0>(folders), Return(0)));
    OsAccountStateData stateData;
    stateData.state = OsAccountState::SWITCHED;
    stateData.toId = OTHER_USER_ID;

    subscriber_->OnStateChanged(stateData);

    const auto &trace = GetCloudDiskFuseLifecycleTrace();
    EXPECT_EQ(trace.stopCalls, 1U);
    EXPECT_EQ(trace.startUserCalls, 1U);
    EXPECT_EQ(trace.userId, OTHER_USER_ID);
    EXPECT_EQ(trace.roots.size(), 1U);
}

/*
 * @tc.name: OnStateChanged_002
 * @tc.desc: Verify a sync-folder query failure starts an empty FUSE session before deferred unload
 * @tc.type: RELI
 */
HWTEST_F(AccountStatusFuseLifecycleTest, OnStateChanged_002, TestSize.Level2)
{
    GetCloudDiskFuseLifecycleTrace().canUnload = false;
    EXPECT_CALL(CloudDiskSyncFolderManagerMock::GetInstance(), GetAllSyncFoldersForSa(_)).WillOnce(Return(EIO));
    OsAccountStateData stateData;
    stateData.state = OsAccountState::SWITCHED;
    stateData.toId = OTHER_USER_ID;

    subscriber_->OnStateChanged(stateData);

    const auto &trace = GetCloudDiskFuseLifecycleTrace();
    EXPECT_EQ(trace.stopCalls, 1U);
    EXPECT_EQ(trace.startUserCalls, 1U);
    EXPECT_TRUE(trace.roots.empty());
    EXPECT_EQ(trace.canUnloadCalls, 1U);
}

/*
 * @tc.name: OnStateChanged_003
 * @tc.desc: Verify stopping another user does not affect the active FUSE lifecycle
 * @tc.type: FUNC
 */
HWTEST_F(AccountStatusFuseLifecycleTest, OnStateChanged_003, TestSize.Level2)
{
    OsAccountStateData stateData;
    stateData.state = OsAccountState::STOPPED;
    stateData.toId = OTHER_USER_ID;

    subscriber_->OnStateChanged(stateData);

    EXPECT_EQ(GetCloudDiskFuseLifecycleTrace().stopForServiceExitCalls, 0U);
    EXPECT_EQ(GetCloudDiskFuseLifecycleTrace().canUnloadCalls, 0U);
}

/*
 * @tc.name: OnStateChanged_004
 * @tc.desc: Verify stopping the active user tears down FUSE and defers unload while it is busy
 * @tc.type: RELI
 */
HWTEST_F(AccountStatusFuseLifecycleTest, OnStateChanged_004, TestSize.Level2)
{
    GetCloudDiskFuseLifecycleTrace().canUnload = false;
    OsAccountStateData stateData;
    stateData.state = OsAccountState::STOPPED;
    stateData.toId = TEST_USER_ID;

    subscriber_->OnStateChanged(stateData);

    EXPECT_EQ(GetCloudDiskFuseLifecycleTrace().stopForServiceExitCalls, 1U);
    EXPECT_EQ(GetCloudDiskFuseLifecycleTrace().canUnloadCalls, 1U);
}

/*
 * @tc.name: UnloadSa_001
 * @tc.desc: Verify an idle FUSE lifecycle permits unloading the system ability
 * @tc.type: FUNC
 */
HWTEST_F(AccountStatusFuseLifecycleTest, UnloadSa_001, TestSize.Level1)
{
    auto manager = sptr<ISystemAbilityManagerMock>(new ISystemAbilityManagerMock());
    EXPECT_CALL(*clientMock_, GetSystemAbilityManager()).WillOnce(Return(manager));
    EXPECT_CALL(*manager, UnloadSystemAbility(CLOUD_DISK_SERVICE_SA_ID)).WillOnce(Return(ERR_OK));

    subscriber_->UnloadSa();

    EXPECT_EQ(GetCloudDiskFuseLifecycleTrace().canUnloadCalls, 1U);
}

/*
 * @tc.name: UnloadSa_002
 * @tc.desc: Verify a busy FUSE lifecycle prevents querying the system ability manager
 * @tc.type: RELI
 */
HWTEST_F(AccountStatusFuseLifecycleTest, UnloadSa_002, TestSize.Level2)
{
    GetCloudDiskFuseLifecycleTrace().canUnload = false;
    EXPECT_CALL(*clientMock_, GetSystemAbilityManager()).Times(0);

    subscriber_->UnloadSa();

    EXPECT_EQ(GetCloudDiskFuseLifecycleTrace().canUnloadCalls, 1U);
}

/*
 * @tc.name: ListenerStart_001
 * @tc.desc: Verify restarting the listener unsubscribes the old subscriber before subscribing the replacement
 * @tc.type: FUNC
 */
HWTEST_F(AccountStatusFuseLifecycleTest, ListenerStart_001, TestSize.Level1)
{
    auto listener = std::make_shared<AccountStatusListener>();
    OsAccountSubscribeInfo subscribeInfo;
    auto oldSubscriber = std::make_shared<AccountStatusSubscriber>(subscribeInfo, TEST_USER_ID);
    listener->osAccountSubscriber_ = oldSubscriber;

    listener->Start(OTHER_USER_ID);

    EXPECT_EQ(AccountStatusLifecycleMock::GetUnsubscribeCalls(), 1U);
    EXPECT_EQ(AccountStatusLifecycleMock::GetSubscribeCalls(), 1U);
    EXPECT_EQ(AccountStatusLifecycleMock::GetUnsubscribed(), oldSubscriber);
    ASSERT_NE(AccountStatusLifecycleMock::GetSubscribed(), nullptr);
    EXPECT_NE(AccountStatusLifecycleMock::GetSubscribed(), oldSubscriber);
    const auto &operations = AccountStatusLifecycleMock::GetOperations();
    ASSERT_EQ(operations.size(), 2U);
    EXPECT_EQ(operations[0], AccountStatusLifecycleMock::Operation::UNSUBSCRIBE);
    EXPECT_EQ(operations[1], AccountStatusLifecycleMock::Operation::SUBSCRIBE);
    auto subscriber = std::static_pointer_cast<AccountStatusSubscriber>(listener->osAccountSubscriber_);
    ASSERT_NE(subscriber, nullptr);
    EXPECT_EQ(subscriber->currentUserId_, OTHER_USER_ID);
}

} // namespace OHOS::FileManagement::CloudDiskService::Test
