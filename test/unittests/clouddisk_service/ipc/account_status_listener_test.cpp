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

#include "account_status_listener.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "cloud_disk_sync_folder.h"
#include "cloud_disk_sync_folder_manager_mock.h"
#include "common_event_data.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "if_system_ability_manager_mock.h"
#include "securec.h"
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
    static inline shared_ptr<AccountStatusSubscriber> subscribers_;
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
    subscribers_ = make_shared<AccountStatusSubscriber>(subscribeInfo);
}

void AccountStatusSubscriberTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    SystemAbilityManagerClientMock::smc = nullptr;
    smc_ = nullptr;
    samgr = nullptr;
    SystemAbilityManagerClient::GetInstance().systemAbilityManager_ = nullptr;
    subscribers_ = nullptr;
}

void AccountStatusSubscriberTest::SetUp(void)
{
}

void AccountStatusSubscriberTest::TearDown(void)
{
}

/**
 * @tc.name: OnStateChangedTest001
 * @tc.desc: Verify the OnStateChanged function with SWITCHED state and valid sync folder
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AccountStatusSubscriberTest, OnStateChangedTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnStateChangedTest001 start";
    try {
        OsAccountStateData osAccountData;
        osAccountData.state = SWITCHED;
        std::vector<SyncFolderExt> syncFolder;
        SyncFolderExt ext;
        ext.path_ = "path";
        syncFolder.push_back(ext);
        CloudDiskSyncFolderManagerMock &mockFolderManager = CloudDiskSyncFolderManagerMock::GetInstance();
        EXPECT_CALL(mockFolderManager, GetAllSyncFoldersForSa(_))
            .WillOnce(DoAll(SetArgReferee<0>(syncFolder), Return(0)));
        subscribers_->OnStateChanged(osAccountData);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnStateChangedTest001 failed";
    }
    GTEST_LOG_(INFO) << "OnStateChangedTest001 end";
}

/**
 * @tc.name: OnStateChangedTest002
 * @tc.desc: Verify the OnStateChanged function with SWITCHED state and valid path
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AccountStatusSubscriberTest, OnStateChangedTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnStateChangedTest002 start";
    try {
        OsAccountStateData osAccountData;
        osAccountData.state = SWITCHED;
        std::vector<SyncFolderExt> syncFolder;
        SyncFolderExt ext;
        ext.path_ = "/storage/Users/currentUser/Download/test";
        syncFolder.push_back(ext);
        CloudDiskSyncFolderManagerMock &mockFolderManager = CloudDiskSyncFolderManagerMock::GetInstance();
        EXPECT_CALL(mockFolderManager, GetAllSyncFoldersForSa(_))
            .WillOnce(DoAll(SetArgReferee<0>(syncFolder), Return(0)));
        subscribers_->OnStateChanged(osAccountData);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnStateChangedTest002 failed";
    }
    GTEST_LOG_(INFO) << "OnStateChangedTest002 end";
}

/**
 * @tc.name: OnStateChangedTest003
 * @tc.desc: Verify the OnStateChanged function with SWITCHED state and empty sync folders
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AccountStatusSubscriberTest, OnStateChangedTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnStateChangedTest003 start";
    try {
        OsAccountStateData osAccountData;
        osAccountData.state = SWITCHED;
        std::vector<SyncFolderExt> syncFolder;
        CloudDiskSyncFolderManagerMock &mockFolderManager = CloudDiskSyncFolderManagerMock::GetInstance();
        EXPECT_CALL(mockFolderManager, GetAllSyncFoldersForSa(_))
            .WillOnce(DoAll(SetArgReferee<0>(syncFolder), Return(0)));
        subscribers_->OnStateChanged(osAccountData);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnStateChangedTest003 failed";
    }
    GTEST_LOG_(INFO) << "OnStateChangedTest003 end";
}

/**
 * @tc.name: OnStateChangedTest004
 * @tc.desc: Verify the OnStateChanged function with STOPPED state
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AccountStatusSubscriberTest, OnStateChangedTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnStateChangedTest004 start";
    try {
        OsAccountStateData osAccountData;
        osAccountData.state = STOPPED;
        std::vector<SyncFolderExt> syncFolder;
        CloudDiskSyncFolderManagerMock &mockFolderManager = CloudDiskSyncFolderManagerMock::GetInstance();
        EXPECT_CALL(mockFolderManager, GetAllSyncFoldersForSa(_))
            .WillOnce(DoAll(SetArgReferee<0>(syncFolder), Return(0)));
        subscribers_->OnStateChanged(osAccountData);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnStateChangedTest004 failed";
    }
    GTEST_LOG_(INFO) << "OnStateChangedTest004 end";
}

/**
 * @tc.name: OnStateChangedTest005
 * @tc.desc: Verify the OnStateChanged function with SWITCHED state and multiple sync folders
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AccountStatusSubscriberTest, OnStateChangedTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnStateChangedTest005 start";
    try {
        OsAccountStateData osAccountData;
        osAccountData.state = SWITCHED;
        std::vector<SyncFolderExt> syncFolder;
        SyncFolderExt ext;
        ext.path_ = "/storage/Users/currentUser/Download/test1";
        ext.bundleName_ = "com.test.example1";
        syncFolder.push_back(ext);
        SyncFolderExt ext2;
        ext2.path_ = "/storage/Users/currentUser/Download/test2";
        ext2.bundleName_ = "com.test.example2";
        syncFolder.push_back(ext2);
        CloudDiskSyncFolderManagerMock &mockFolderManager = CloudDiskSyncFolderManagerMock::GetInstance();
        EXPECT_CALL(mockFolderManager, GetAllSyncFoldersForSa(_))
            .WillOnce(DoAll(SetArgReferee<0>(syncFolder), Return(0)));
        subscribers_->OnStateChanged(osAccountData);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnStateChangedTest005 failed";
    }
    GTEST_LOG_(INFO) << "OnStateChangedTest005 end";
}

/**
 * @tc.name: OnStateChangedTest006
 * @tc.desc: Verify the OnStateChanged function with SWITCHED state and failed to get sync folders
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AccountStatusSubscriberTest, OnStateChangedTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnStateChangedTest006 start";
    try {
        OsAccountStateData osAccountData;
        osAccountData.state = SWITCHED;
        std::vector<SyncFolderExt> syncFolder;
        CloudDiskSyncFolderManagerMock &mockFolderManager = CloudDiskSyncFolderManagerMock::GetInstance();
        EXPECT_CALL(mockFolderManager, GetAllSyncFoldersForSa(_))
            .WillOnce(DoAll(SetArgReferee<0>(syncFolder), Return(-1)));
        subscribers_->OnStateChanged(osAccountData);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnStateChangedTest006 failed";
    }
    GTEST_LOG_(INFO) << "OnStateChangedTest006 end";
}

/**
 * @tc.name: UnloadSaTest001
 * @tc.desc: Verify the UnloadSa function with null system ability manager
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AccountStatusSubscriberTest, UnloadSaTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnloadSaTest001 start";
    try {
        EXPECT_CALL(*smc_, GetSystemAbilityManager()).WillRepeatedly(Return(nullptr));
        subscribers_->UnloadSa();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UnloadSaTest001 failed";
    }
    GTEST_LOG_(INFO) << "UnloadSaTest001 end";
}

/**
 * @tc.name: UnloadSaTest002
 * @tc.desc: Verify the UnloadSa function with valid system ability manager and success
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AccountStatusSubscriberTest, UnloadSaTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnloadSaTest002 start";
    try {
        auto mockManager = sptr<ISystemAbilityManagerMock>(new ISystemAbilityManagerMock());
        EXPECT_CALL(*smc_, GetSystemAbilityManager()).WillRepeatedly(Return(mockManager));
        int32_t id = 5207;
        EXPECT_CALL(*mockManager, UnloadSystemAbility(id)).WillOnce(Return(ERR_OK));
        subscribers_->UnloadSa();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UnloadSaTest002 failed";
    }
    GTEST_LOG_(INFO) << "UnloadSaTest002 end";
}

/**
 * @tc.name: UnloadSaTest003
 * @tc.desc: Verify the UnloadSa function with valid system ability manager and failure
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AccountStatusSubscriberTest, UnloadSaTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnloadSaTest003 start";
    try {
        auto mockManager = sptr<ISystemAbilityManagerMock>(new ISystemAbilityManagerMock());
        EXPECT_CALL(*smc_, GetSystemAbilityManager()).WillRepeatedly(Return(mockManager));
        int32_t id = 5207;
        EXPECT_CALL(*mockManager, UnloadSystemAbility(id)).WillOnce(Return(-1));
        subscribers_->UnloadSa();
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
    static inline shared_ptr<AccountStatusListener> listeners_;
};

void AccountStatusListenerTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    listeners_ = make_shared<AccountStatusListener>();
}

void AccountStatusListenerTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    listeners_ = nullptr;
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
        listeners_->Start();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StartTest001 failed";
    }
    GTEST_LOG_(INFO) << "StartTest001 end";
}

/**
 * @tc.name: StartTest002
 * @tc.desc: Verify the Start function when called multiple times
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AccountStatusListenerTest, StartTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartTest002 start";
    try {
        listeners_->Start();
        listeners_->Start();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StartTest002 failed";
    }
    GTEST_LOG_(INFO) << "StartTest002 end";
}

/**
 * @tc.name: StopTest001
 * @tc.desc: Verify the Stop function with null subscriber
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AccountStatusListenerTest, StopTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StopTest001 start";
    try {
        listeners_->osAccountSubscriber_ = nullptr;
        listeners_->Stop();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StopTest001 failed";
    }
    GTEST_LOG_(INFO) << "StopTest001 end";
}

/**
 * @tc.name: StopTest002
 * @tc.desc: Verify the Stop function with valid subscriber
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AccountStatusListenerTest, StopTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StopTest002 start";
    try {
        OsAccountSubscribeInfo info;
        listeners_->osAccountSubscriber_ = std::make_shared<AccountStatusSubscriber>(info);
        listeners_->Stop();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StopTest002 failed";
    }
    GTEST_LOG_(INFO) << "StopTest002 end";
}

/**
 * @tc.name: StopTest003
 * @tc.desc: Verify the Stop function when called multiple times
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AccountStatusListenerTest, StopTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StopTest003 start";
    try {
        OsAccountSubscribeInfo info;
        listeners_->osAccountSubscriber_ = std::make_shared<AccountStatusSubscriber>(info);
        listeners_->Stop();
        listeners_->Stop();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StopTest003 failed";
    }
    GTEST_LOG_(INFO) << "StopTest003 end";
}
} // namespace OHOS::FileManagement::CloudDiskService::Test
