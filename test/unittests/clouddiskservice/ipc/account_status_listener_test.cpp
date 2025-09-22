/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "account_status_listener.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "cloud_disk_sync_folder.h"
#include "cloud_disk_sync_folder_manager_mock.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "common_event_data.h"
#include "dfsu_access_token_helper_mock.h"
#include "if_system_ability_manager_mock.h"
#include "system_ability_manager_client_mock.h"

namespace OHOS::FileManagement::CloudDiskService::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using Want = OHOS::AAFwk::Want;
using namespace AccountSA;

class AccountStatusSubscriberTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<AccountStatusSubscriber> subscriber_;
    static inline shared_ptr<DfsuAccessTokenMock> dfsuAccessToken_;
    static inline shared_ptr<SystemAbilityManagerClientMock> smc_ = nullptr;
    static inline shared_ptr<ISystemAbilityManagerMock> samgr = make_shared<ISystemAbilityManagerMock>();
};

void AccountStatusSubscriberTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    smc_ = make_shared<SystemAbilityManagerClientMock>();
    SystemAbilityManagerClientMock::smc = smc_;
    SystemAbilityManagerClient::GetInstance().systemAbilityManager_ = sptr<ISystemAbilityManager>(samgr.get());
    OsAccountSubscribeInfo subscribeInfo;
    subscriber_ = make_shared<AccountStatusSubscriber>(subscribeInfo);
    dfsuAccessToken_ = make_shared<DfsuAccessTokenMock>();
    DfsuAccessTokenMock::dfsuAccessToken = dfsuAccessToken_;
}

void AccountStatusSubscriberTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    SystemAbilityManagerClientMock::smc = nullptr;
    smc_ = nullptr;
    samgr = nullptr;
    SystemAbilityManagerClient::GetInstance().systemAbilityManager_ = nullptr;
    subscriber_ = nullptr;
    DfsuAccessTokenMock::dfsuAccessToken = nullptr;
    dfsuAccessToken_ = nullptr;
}

void AccountStatusSubscriberTest::SetUp(void)
{
}

void AccountStatusSubscriberTest::TearDown(void)
{
}

/**
 * @tc.name: OnStateChangedTest001
 * @tc.desc: Verify the OnStateChanged function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AccountStatusSubscriberTest, OnStateChangedTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnStateChangedTest001 start";
    try {
        OsAccountStateData osAccountStateData;
        osAccountStateData.state = SWITCHED;
        std::vector<FileManagement::SyncFolderExt> syncFolders;
        FileManagement::SyncFolderExt ext1;
        ext1.path_ = "path";
        syncFolders.push_back(ext1);
        OHOS::FileManagement::Test::CloudDiskSyncFolderManagerMock& mockManager =
            OHOS::FileManagement::Test::CloudDiskSyncFolderManagerMock::GetInstance();
        EXPECT_CALL(mockManager, GetAllSyncFoldersForSa(_)).WillOnce(DoAll(SetArgReferee<0>(syncFolders), Return(0)));
        subscriber_->OnStateChanged(osAccountStateData);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnStateChangedTest001 failed";
    }
    GTEST_LOG_(INFO) << "OnStateChangedTest001 end";
}

/**
 * @tc.name: OnStateChangedTest002
 * @tc.desc: Verify the OnStateChanged function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AccountStatusSubscriberTest, OnStateChangedTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnStateChangedTest002 start";
    try {
        OsAccountStateData osAccountStateData;
        osAccountStateData.state = SWITCHED;
        std::vector<FileManagement::SyncFolderExt> syncFolders;
        FileManagement::SyncFolderExt ext1;
        ext1.path_ = "/storage/Users/currentUser/Download/test";
        syncFolders.push_back(ext1);
        OHOS::FileManagement::Test::CloudDiskSyncFolderManagerMock& mockManager =
            OHOS::FileManagement::Test::CloudDiskSyncFolderManagerMock::GetInstance();
        EXPECT_CALL(mockManager, GetAllSyncFoldersForSa(_)).WillOnce(DoAll(SetArgReferee<0>(syncFolders), Return(0)));
        subscriber_->OnStateChanged(osAccountStateData);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnStateChangedTest002 failed";
    }
    GTEST_LOG_(INFO) << "OnStateChangedTest002 end";
}

/**
 * @tc.name: OnStateChangedTest003
 * @tc.desc: Verify the OnStateChanged function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AccountStatusSubscriberTest, OnStateChangedTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnStateChangedTest003 start";
    try {
        OsAccountStateData osAccountStateData;
        osAccountStateData.state = SWITCHED;
        std::vector<FileManagement::SyncFolderExt> syncFolders;
        OHOS::FileManagement::Test::CloudDiskSyncFolderManagerMock& mockManager =
            OHOS::FileManagement::Test::CloudDiskSyncFolderManagerMock::GetInstance();
        EXPECT_CALL(mockManager, GetAllSyncFoldersForSa(_)).WillOnce(DoAll(SetArgReferee<0>(syncFolders), Return(0)));
        subscriber_->OnStateChanged(osAccountStateData);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnStateChangedTest003 failed";
    }
    GTEST_LOG_(INFO) << "OnStateChangedTest003 end";
}

/**
 * @tc.name: OnStateChangedTest004
 * @tc.desc: Verify the OnStateChanged function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AccountStatusSubscriberTest, OnStateChangedTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnStateChangedTest004 start";
    try {
        OsAccountStateData osAccountStateData;
        osAccountStateData.state = STOPPED;
        std::vector<FileManagement::SyncFolderExt> syncFolders;
        OHOS::FileManagement::Test::CloudDiskSyncFolderManagerMock& mockManager =
            OHOS::FileManagement::Test::CloudDiskSyncFolderManagerMock::GetInstance();
        EXPECT_CALL(mockManager, GetAllSyncFoldersForSa(_)).WillOnce(DoAll(SetArgReferee<0>(syncFolders), Return(0)));
        subscriber_->OnStateChanged(osAccountStateData);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnStateChangedTest004 failed";
    }
    GTEST_LOG_(INFO) << "OnStateChangedTest004 end";
}

/**
 * @tc.name: UnloadSaTest001
 * @tc.desc: Verify the UnloadSa function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AccountStatusSubscriberTest, UnloadSaTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnloadSaTest001 start";
    try {
        EXPECT_CALL(*smc_, GetSystemAbilityManager()).WillOnce(Return(nullptr));
        subscriber_->UnloadSa();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UnloadSaTest001 failed";
    }
    GTEST_LOG_(INFO) << "UnloadSaTest001 end";
}

/**
 * @tc.name: UnloadSaTest002
 * @tc.desc: Verify the UnloadSa function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AccountStatusSubscriberTest, UnloadSaTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnloadSaTest002 start";
    try {
        auto sysAbilityManager = sptr<ISystemAbilityManagerMock>(new ISystemAbilityManagerMock());
        EXPECT_CALL(*smc_, GetSystemAbilityManager()).WillOnce(Return(sysAbilityManager));
        int32_t saId = 5207;
        EXPECT_CALL(*sysAbilityManager, UnloadSystemAbility(saId)).WillOnce(Return(ERR_OK));
        subscriber_->UnloadSa();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UnloadSaTest002 failed";
    }
    GTEST_LOG_(INFO) << "UnloadSaTest002 end";
}

/**
 * @tc.name: UnloadSaTest003
 * @tc.desc: Verify the UnloadSa function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AccountStatusSubscriberTest, UnloadSaTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnloadSaTest003 start";
    try {
        auto sysAbilityManager = sptr<ISystemAbilityManagerMock>(new ISystemAbilityManagerMock());
        EXPECT_CALL(*smc_, GetSystemAbilityManager()).WillOnce(Return(sysAbilityManager));
        int32_t saId = 5207;
        EXPECT_CALL(*sysAbilityManager, UnloadSystemAbility(saId)).WillOnce(Return(-1));
        subscriber_->UnloadSa();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UnloadSaTest003 failed";
    }
    GTEST_LOG_(INFO) << "UnloadSaTest003 end";
}

class AccountStatusListenerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<AccountStatusListener> listener_;
};

void AccountStatusListenerTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    listener_ = make_shared<AccountStatusListener>();
}

void AccountStatusListenerTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    listener_ = nullptr;
}

void AccountStatusListenerTest::SetUp(void)
{
}

void AccountStatusListenerTest::TearDown(void)
{
}

/**
 * @tc.name: StartTest001
 * @tc.desc: Verify the Start function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AccountStatusListenerTest, StartTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartTest001 start";
    try {
        listener_->Start();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StartTest001 failed";
    }
    GTEST_LOG_(INFO) << "StartTest001 end";
}

/**
 * @tc.name: StopTest001
 * @tc.desc: Verify the Stop function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AccountStatusListenerTest, StopTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StopTest001 start";
    try {
        listener_->osAccountSubscriber_ = nullptr;
        listener_->Stop();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StopTest001 failed";
    }
    GTEST_LOG_(INFO) << "StopTest001 end";
}

/**
 * @tc.name: StopTest002
 * @tc.desc: Verify the Stop function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AccountStatusListenerTest, StopTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StopTest002 start";
    try {
        OsAccountSubscribeInfo info;
        listener_->osAccountSubscriber_ = std::make_shared<AccountStatusSubscriber>(info);
        listener_->Stop();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StopTest002 failed";
    }
    GTEST_LOG_(INFO) << "StopTest002 end";
}
}