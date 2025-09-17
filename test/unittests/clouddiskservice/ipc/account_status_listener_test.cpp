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

namespace OHOS::FileManagement::CloudDiskService::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using Want = OHOS::AAFwk::Want;

class AccountStatusSubscriberTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<AccountStatusSubscriber> subscriber_;
    static inline shared_ptr<DfsuAccessTokenMock> dfsuAccessToken_;
};

void AccountStatusSubscriberTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    EventFwk::CommonEventSubscribeInfo subscribeInfo;
    subscriber_ = make_shared<AccountStatusSubscriber>(subscribeInfo);
    dfsuAccessToken_ = make_shared<DfsuAccessTokenMock>();
    DfsuAccessTokenMock::dfsuAccessToken = dfsuAccessToken_;
}

void AccountStatusSubscriberTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
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
 * @tc.name: OnReceiveEventTest001
 * @tc.desc: Verify the OnReceiveEvent function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AccountStatusSubscriberTest, OnReceiveEventTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnReceiveEventTest001 start";
    try {
        Want want;
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_USER_STARTED);
        EventFwk::CommonEventData eventData(want);
        std::vector<FileManagement::SyncFolderExt> syncFolders;
        FileManagement::SyncFolderExt ext1;
        ext1.path_ = "path";
        syncFolders.push_back(ext1);
        OHOS::FileManagement::Test::CloudDiskSyncFolderManagerMock& mockManager =
            OHOS::FileManagement::Test::CloudDiskSyncFolderManagerMock::GetInstance();
        EXPECT_CALL(mockManager, GetAllSyncFoldersForSa(_)).WillOnce(DoAll(SetArgReferee<0>(syncFolders), Return(0)));
        EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(0));
        subscriber_->OnReceiveEvent(eventData);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnReceiveEventTest001 failed";
    }
    GTEST_LOG_(INFO) << "OnReceiveEventTest001 end";
}

/**
 * @tc.name: OnReceiveEventTest002
 * @tc.desc: Verify the OnReceiveEvent function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AccountStatusSubscriberTest, OnReceiveEventTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnReceiveEventTest002 start";
    try {
        Want want;
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_USER_STARTED);
        EventFwk::CommonEventData eventData(want);
        std::vector<FileManagement::SyncFolderExt> syncFolders;
        FileManagement::SyncFolderExt ext1;
        ext1.path_ = "";
        syncFolders.push_back(ext1);
        OHOS::FileManagement::Test::CloudDiskSyncFolderManagerMock& mockManager =
            OHOS::FileManagement::Test::CloudDiskSyncFolderManagerMock::GetInstance();
        EXPECT_CALL(mockManager, GetAllSyncFoldersForSa(_)).WillOnce(DoAll(SetArgReferee<0>(syncFolders), Return(0)));
        EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(1));
        CloudDiskSyncFolder::GetInstance().syncFolderMap.clear();
        subscriber_->OnReceiveEvent(eventData);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnReceiveEventTest002 failed";
    }
    GTEST_LOG_(INFO) << "OnReceiveEventTest002 end";
}

/**
 * @tc.name: OnReceiveEventTest003
 * @tc.desc: Verify the OnReceiveEvent function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AccountStatusSubscriberTest, OnReceiveEventTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnReceiveEventTest003 start";
    try {
        Want want;
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_USER_STARTED);
        EventFwk::CommonEventData eventData(want);
        std::vector<FileManagement::SyncFolderExt> syncFolders;
        FileManagement::SyncFolderExt ext1;
        ext1.path_ = "";
        syncFolders.push_back(ext1);
        OHOS::FileManagement::Test::CloudDiskSyncFolderManagerMock& mockManager =
            OHOS::FileManagement::Test::CloudDiskSyncFolderManagerMock::GetInstance();
        EXPECT_CALL(mockManager, GetAllSyncFoldersForSa(_)).WillOnce(DoAll(SetArgReferee<0>(syncFolders), Return(0)));
        EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(1));
        CloudDiskSyncFolder::GetInstance().syncFolderMap.clear();
        SyncFolderValue value{"bundleName", "path"};
        CloudDiskSyncFolder::GetInstance().syncFolderMap[0] = value;
        subscriber_->OnReceiveEvent(eventData);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnReceiveEventTest003 failed";
    }
    GTEST_LOG_(INFO) << "OnReceiveEventTest003 end";
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
        listener_->commonEventSubscriber_ = nullptr;
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
        EventFwk::CommonEventSubscribeInfo info;
        listener_->commonEventSubscriber_ = std::make_shared<AccountStatusSubscriber>(info);
        listener_->Stop();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StopTest002 failed";
    }
    GTEST_LOG_(INFO) << "StopTest002 end";
}
}