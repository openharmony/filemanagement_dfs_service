/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "account_status_listener.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "data_syncer_rdb_store.h"
#include "dfs_error.h"
#include "result_set.h"
#include "setting_data_helper.h"

namespace OHOS::FileManagement::CloudDisk::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using namespace OHOS::FileManagement::CloudFile;
using namespace OHOS::FileManagement::CloudSync;
using namespace OHOS::NativeRdb;
using Want = OHOS::AAFwk::Want;

constexpr int32_t USER_ID = 101;

class AccountStatusListenerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<AccountStatusListener> accountStatusListener_ = nullptr;
    static inline shared_ptr<AccountStatusSubscriber> accountStatusSubscriber_ = nullptr;
    static inline shared_ptr<EventFwk::CommonEventSubscribeInfo> subscribeInfo_ = nullptr;
};

void AccountStatusListenerTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_HWID_LOGIN);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_HWID_LOGOUT);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_DATA_SHARE_READY);
    subscribeInfo_ = make_shared<EventFwk::CommonEventSubscribeInfo>(matchingSkills);
    accountStatusSubscriber_ = make_shared<AccountStatusSubscriber>(*subscribeInfo_);
    accountStatusListener_ = make_shared<AccountStatusListener>();
}

void AccountStatusListenerTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void AccountStatusListenerTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void AccountStatusListenerTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
    DataSyncerRdbStoreMock::proxy_ = nullptr;
    ResultSetMock::proxy_ = nullptr;
}

/**
 * @tc.name: OnReceiveEventTest001
 * @tc.desc: Verify the OnReceiveEvent function
 * @tc.type: FUNC
 * @tc.require: issuesI90JZZ
 */
HWTEST_F(AccountStatusListenerTest, OnReceiveEventTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnReceiveEventTest001 Start";
    try {
        Want want;
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_HWID_LOGIN);
        string data(EventFwk::CommonEventSupport::COMMON_EVENT_HWID_LOGIN);
        EventFwk::CommonEventData eventData(want, USER_ID, data);
        accountStatusSubscriber_->OnReceiveEvent(eventData);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnReceiveEventTest001  ERROR";
    }
    GTEST_LOG_(INFO) << "OnReceiveEventTest001 End";
}

/**
 * @tc.name: OnReceiveEventTest002
 * @tc.desc: Verify the OnReceiveEvent function
 * @tc.type: FUNC
 * @tc.require: issuesI90JZZ
 */
HWTEST_F(AccountStatusListenerTest, OnReceiveEventTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnReceiveEventTest002 Start";
    try {
        Want want;
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_HWID_LOGOUT);
        string data(EventFwk::CommonEventSupport::COMMON_EVENT_HWID_LOGOUT);
        EventFwk::CommonEventData eventData(want, USER_ID, data);
        accountStatusSubscriber_->OnReceiveEvent(eventData);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnReceiveEventTest002  ERROR";
    }
    GTEST_LOG_(INFO) << "OnReceiveEventTest002 End";
}

/**
 * @tc.name: OnReceiveEventTest003
 * @tc.desc: Verify the OnReceiveEvent function
 * @tc.type: FUNC
 * @tc.require: issuesIBMCHL
 */
HWTEST_F(AccountStatusListenerTest, OnReceiveEventTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnReceiveEventTest003 Start";
    try {
        Want want;
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED);
        string data(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED);
        EventFwk::CommonEventData eventData(want, USER_ID, data);
        accountStatusSubscriber_->OnReceiveEvent(eventData);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnReceiveEventTest003  ERROR";
    }
    GTEST_LOG_(INFO) << "OnReceiveEventTest003 End";
}

/**
 * @tc.name: OnReceiveEventTest004
 * @tc.desc: Verify the OnReceiveEvent function
 * @tc.type: FUNC
 */
HWTEST_F(AccountStatusListenerTest, OnReceiveEventTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnReceiveEventTest004 Start";
    try {
        Want want;
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_DATA_SHARE_READY);
        string data(EventFwk::CommonEventSupport::COMMON_EVENT_DATA_SHARE_READY);
        EventFwk::CommonEventData eventData(want, USER_ID, data);
        
        SettingDataHelper::GetInstance().isBundleInited_ = true;
        accountStatusSubscriber_->OnReceiveEvent(eventData);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnReceiveEventTest004  ERROR";
    }
    GTEST_LOG_(INFO) << "OnReceiveEventTest004 End";
}

/**
 * @tc.name: StopTest001
 * @tc.desc: Verify the Stop function
 * @tc.type: FUNC
 * @tc.require: issuesI90JZZ
 */
HWTEST_F(AccountStatusListenerTest, StopTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StopTest001 Start";
    try {
        accountStatusListener_->commonEventSubscriber_ = std::make_shared<AccountStatusSubscriber>(*subscribeInfo_);
        accountStatusListener_->Stop();
        EXPECT_TRUE(accountStatusListener_->commonEventSubscriber_ == nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StopTest001  ERROR";
    }
    GTEST_LOG_(INFO) << "StopTest001 End";
}

/**
 * @tc.name: StopTest002
 * @tc.desc: Verify the Stop function
 * @tc.type: FUNC
 * @tc.require: issuesI90JZZ
 */
HWTEST_F(AccountStatusListenerTest, StopTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StopTest002 Start";
    try {
        accountStatusListener_->commonEventSubscriber_ = nullptr;
        accountStatusListener_->Stop();
        EXPECT_TRUE(accountStatusListener_->commonEventSubscriber_ == nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StopTest002  ERROR";
    }
    GTEST_LOG_(INFO) << "StopTest002 End";
}

/**
 * @tc.name: IsCloudSyncEnabledTest001
 * @tc.desc: Verify the IsCloudSyncEnabled function
 * @tc.type: FUNC
 */
HWTEST_F(AccountStatusListenerTest, IsCloudSyncEnabledTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsCloudSyncEnabledTest001 Start";
    try {
        DataSyncerRdbStoreMock::proxy_ = std::make_shared<DataSyncerRdbStoreMock>();
        ResultSetMock::proxy_ = std::make_shared<ResultSetMock>();
        ResultSetMock::count_ = 1;
        int32_t userId = 0;
        std::string bundleName = "";
        EXPECT_CALL(*DataSyncerRdbStoreMock::proxy_, QueryCloudSync(_, _, _)).WillOnce(Return(E_OK));
        EXPECT_CALL(*ResultSetMock::proxy_, GetRowCount(_)).WillOnce(Return(E_OK));
        bool ret = accountStatusSubscriber_->IsCloudSyncEnabled(userId, bundleName);
        EXPECT_EQ(ret, true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsCloudSyncEnabledTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "IsCloudSyncEnabledTest001 End";
}

/**
 * @tc.name: IsCloudSyncEnabledTest002
 * @tc.desc: Verify the IsCloudSyncEnabled function
 * @tc.type: FUNC
 */
HWTEST_F(AccountStatusListenerTest, IsCloudSyncEnabledTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsCloudSyncEnabledTest002 Start";
    try {
        DataSyncerRdbStoreMock::proxy_ = std::make_shared<DataSyncerRdbStoreMock>();
        ResultSetMock::proxy_ = std::make_shared<ResultSetMock>();
        ResultSetMock::count_ = 1;
        int32_t userId = 0;
        std::string bundleName = "";
        EXPECT_CALL(*DataSyncerRdbStoreMock::proxy_, QueryCloudSync(_, _, _)).WillOnce(Return(E_INVAL_ARG));
        EXPECT_CALL(*ResultSetMock::proxy_, GetRowCount(_)).Times(0);
        bool ret = accountStatusSubscriber_->IsCloudSyncEnabled(userId, bundleName);
        EXPECT_EQ(ret, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsCloudSyncEnabledTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "IsCloudSyncEnabledTest002 End";
}

/**
 * @tc.name: IsCloudSyncEnabledTest003
 * @tc.desc: Verify the IsCloudSyncEnabled function
 * @tc.type: FUNC
 */
HWTEST_F(AccountStatusListenerTest, IsCloudSyncEnabledTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsCloudSyncEnabledTest003 Start";
    try {
        DataSyncerRdbStoreMock::proxy_ = std::make_shared<DataSyncerRdbStoreMock>();
        ResultSetMock::proxy_ = std::make_shared<ResultSetMock>();
        ResultSetMock::count_ = 1;
        int32_t userId = -1;
        std::string bundleName = "";
        EXPECT_CALL(*DataSyncerRdbStoreMock::proxy_, QueryCloudSync(_, _, _)).WillOnce(Return(E_OK));
        EXPECT_CALL(*ResultSetMock::proxy_, GetRowCount(_)).Times(0);
        bool ret = accountStatusSubscriber_->IsCloudSyncEnabled(userId, bundleName);
        EXPECT_EQ(ret, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsCloudSyncEnabledTest003 ERROR";
    }
    GTEST_LOG_(INFO) << "IsCloudSyncEnabledTest003 End";
}

/**
 * @tc.name: IsCloudSyncEnabledTest004
 * @tc.desc: Verify the IsCloudSyncEnabled function
 * @tc.type: FUNC
 */
HWTEST_F(AccountStatusListenerTest, IsCloudSyncEnabledTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsCloudSyncEnabledTest004 Start";
    try {
        DataSyncerRdbStoreMock::proxy_ = std::make_shared<DataSyncerRdbStoreMock>();
        ResultSetMock::proxy_ = std::make_shared<ResultSetMock>();
        ResultSetMock::count_ = 1;
        int32_t userId = 0;
        std::string bundleName = "";
        EXPECT_CALL(*DataSyncerRdbStoreMock::proxy_, QueryCloudSync(_, _, _)).WillOnce(Return(E_OK));
        EXPECT_CALL(*ResultSetMock::proxy_, GetRowCount(_)).WillOnce(Return(E_INVAL_ARG));
        bool ret = accountStatusSubscriber_->IsCloudSyncEnabled(userId, bundleName);
        EXPECT_EQ(ret, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsCloudSyncEnabledTest004 ERROR";
    }
    GTEST_LOG_(INFO) << "IsCloudSyncEnabledTest004 End";
}

/**
 * @tc.name: IsCloudSyncEnabledTest005
 * @tc.desc: Verify the IsCloudSyncEnabled function
 * @tc.type: FUNC
 */
HWTEST_F(AccountStatusListenerTest, IsCloudSyncEnabledTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsCloudSyncEnabledTest005 Start";
    try {
        DataSyncerRdbStoreMock::proxy_ = std::make_shared<DataSyncerRdbStoreMock>();
        ResultSetMock::proxy_ = std::make_shared<ResultSetMock>();
        ResultSetMock::count_ = -1;
        int32_t userId = 0;
        std::string bundleName = "";
        EXPECT_CALL(*DataSyncerRdbStoreMock::proxy_, QueryCloudSync(_, _, _)).WillOnce(Return(E_OK));
        EXPECT_CALL(*ResultSetMock::proxy_, GetRowCount(_)).WillOnce(Return(E_OK));
        bool ret = accountStatusSubscriber_->IsCloudSyncEnabled(userId, bundleName);
        EXPECT_EQ(ret, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsCloudSyncEnabledTest005 ERROR";
    }
    GTEST_LOG_(INFO) << "IsCloudSyncEnabledTest005 End";
}

/**
 * @tc.name: RemovedCleanTest001
 * @tc.desc: Verify the RemovedClean function
 * @tc.type: FUNC
 */
HWTEST_F(AccountStatusListenerTest, RemovedCleanTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RemovedCleanTest001 Start";
    try {
        DataSyncerRdbStoreMock::proxy_ = std::make_shared<DataSyncerRdbStoreMock>();
        ResultSetMock::proxy_ = std::make_shared<ResultSetMock>();
        ResultSetMock::count_ = 1;
        EventFwk::CommonEventData eventData;
        AAFwk::Want want;
        want.SetParam("uid", 1);
        eventData.SetWant(want);
        EXPECT_CALL(*DataSyncerRdbStoreMock::proxy_, QueryCloudSync(_, _, _)).WillOnce(Return(E_OK));
        EXPECT_CALL(*ResultSetMock::proxy_, GetRowCount(_)).WillOnce(Return(E_OK));
        accountStatusSubscriber_->RemovedClean(eventData);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RemovedCleanTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "RemovedCleanTest001 End";
}

/**
 * @tc.name: RemovedCleanTest002
 * @tc.desc: Verify the RemovedClean function
 * @tc.type: FUNC
 */
HWTEST_F(AccountStatusListenerTest, RemovedCleanTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RemovedCleanTest002 Start";
    try {
        DataSyncerRdbStoreMock::proxy_ = std::make_shared<DataSyncerRdbStoreMock>();
        ResultSetMock::proxy_ = std::make_shared<ResultSetMock>();
        ResultSetMock::count_ = 1;
        EventFwk::CommonEventData eventData;
        AAFwk::Want want;
        want.SetParam("uid", 1);
        eventData.SetWant(want);
        EXPECT_CALL(*DataSyncerRdbStoreMock::proxy_, QueryCloudSync(_, _, _)).WillOnce(Return(E_INVAL_ARG));
        EXPECT_CALL(*ResultSetMock::proxy_, GetRowCount(_)).Times(0);
        accountStatusSubscriber_->RemovedClean(eventData);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RemovedCleanTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "RemovedCleanTest002 End";
}
} // namespace OHOS::FileManagement::CloudDisk::Test