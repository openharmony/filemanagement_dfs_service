/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "common_event_manager.h"
#include "common_event_support.h"
#include "session_manager.h"
#include "user_status_listener.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using namespace CloudFile;
using Want = OHOS::AAFwk::Want;

class UserStatusListenerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};
void UserStatusListenerTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void UserStatusListenerTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void UserStatusListenerTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void UserStatusListenerTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: StopTest001
 * @tc.desc: Verify the Stop function
 * @tc.type: FUNC
 * @tc.require: IB3T5H
 */
HWTEST_F(UserStatusListenerTest, StopTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StopTest001 start";
    try {
        auto dataSyncManager = std::make_shared<DataSyncManager>();
        auto userStatusListener = std::make_shared<UserStatusListener>(dataSyncManager);
        userStatusListener->Start();
        EXPECT_NE(userStatusListener->commonEventSubscriber_, nullptr);
        userStatusListener->Stop();
        EXPECT_EQ(userStatusListener->commonEventSubscriber_, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StopTest001 failed";
    }
    GTEST_LOG_(INFO) << "StopTest001 end";
}

/**
 * @tc.name: AddObserverTest001
 * @tc.desc: Verify the AddObserver function
 * @tc.type: FUNC
 * @tc.require: IB3T5H
 */
HWTEST_F(UserStatusListenerTest, AddObserverTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AddObserverTest001 start";
    try {
        auto dataSyncManager = std::make_shared<DataSyncManager>();
        auto userStatusListener = std::make_shared<UserStatusListener>(dataSyncManager);
        auto sessionManager = make_shared<SessionManager>();
        sessionManager->Init();
        userStatusListener->AddObserver(sessionManager);
        EXPECT_NE(userStatusListener->observers_.size(), 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "AddObserverTest001 failed";
    }
    GTEST_LOG_(INFO) << "AddObserverTest001 end";
}

/**
 * @tc.name: NotifyUserUnlockedTest001
 * @tc.desc: Verify the NotifyUserUnlocked function
 * @tc.type: FUNC
 * @tc.require: IB3T5H
 */
HWTEST_F(UserStatusListenerTest, NotifyUserUnlockedTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyUserUnlockedTest001 start";
    try {
        auto dataSyncManager = std::make_shared<DataSyncManager>();
        auto userStatusListener = std::make_shared<UserStatusListener>(dataSyncManager);
        userStatusListener->NotifyUserUnlocked();
        EXPECT_EQ(userStatusListener->observers_.size(), 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "NotifyUserUnlockedTest001 failed";
    }
    GTEST_LOG_(INFO) << "NotifyUserUnlockedTest001 end";
}

/**
 * @tc.name: DoCleanVideoCacheTest001
 * @tc.desc: Verify the DoCleanVideoCache function
 * @tc.type: FUNC
 * @tc.require: IB3T5H
 */
HWTEST_F(UserStatusListenerTest, DoCleanVideoCacheTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoCleanVideoCacheTest001 start";
    try {
        auto dataSyncManager = std::make_shared<DataSyncManager>();
        auto userStatusListener = std::make_shared<UserStatusListener>(dataSyncManager);
        userStatusListener->DoCleanVideoCache();
        EXPECT_NE(userStatusListener->dataSyncManager_, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DoCleanVideoCacheTest001 failed";
    }
    GTEST_LOG_(INFO) << "DoCleanVideoCacheTest001 end";
}

/**
 * @tc.name: OnReceiveEventTest001
 * @tc.desc: Verify the OnReceiveEvent function
 * @tc.type: FUNC
 * @tc.require: IB3T5H
 */
HWTEST_F(UserStatusListenerTest, OnReceiveEventTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnReceiveEventTest001 start";
    try {
        Want want;
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_OKAY);
        EventFwk::CommonEventData eventData(want);
        auto dataSyncManager = std::make_shared<DataSyncManager>();
        auto userStatusListener = std::make_shared<UserStatusListener>(dataSyncManager);
        auto subscriber = std::make_shared<UserStatusSubscriber>(EventFwk::CommonEventSubscribeInfo(),
            userStatusListener);
        subscriber->OnReceiveEvent(eventData);
        EXPECT_NE(subscriber->listener_, nullptr);
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
 * @tc.require: IB3T5H
 */
HWTEST_F(UserStatusListenerTest, OnReceiveEventTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnReceiveEventTest002 start";
    try {
        Want want;
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_CHARGING);
        EventFwk::CommonEventData eventData(want);
        auto dataSyncManager = std::make_shared<DataSyncManager>();
        auto userStatusListener = std::make_shared<UserStatusListener>(dataSyncManager);
        auto subscriber = std::make_shared<UserStatusSubscriber>(EventFwk::CommonEventSubscribeInfo(),
            userStatusListener);
        subscriber->OnReceiveEvent(eventData);
        EXPECT_NE(subscriber->listener_, nullptr);
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
 * @tc.require: IB3T5H
 */
HWTEST_F(UserStatusListenerTest, OnReceiveEventTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnReceiveEventTest003 start";
    try {
        Want want;
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_DISCHARGING);
        EventFwk::CommonEventData eventData(want);
        auto dataSyncManager = std::make_shared<DataSyncManager>();
        auto userStatusListener = std::make_shared<UserStatusListener>(dataSyncManager);
        auto subscriber = std::make_shared<UserStatusSubscriber>(EventFwk::CommonEventSubscribeInfo(),
            userStatusListener);
        subscriber->OnReceiveEvent(eventData);
        EXPECT_NE(subscriber->listener_, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnReceiveEventTest003 failed";
    }
    GTEST_LOG_(INFO) << "OnReceiveEventTest003 end";
}

/**
 * @tc.name: OnReceiveEventTest004
 * @tc.desc: Verify the OnReceiveEvent function
 * @tc.type: FUNC
 * @tc.require: IB3T5H
 */
HWTEST_F(UserStatusListenerTest, OnReceiveEventTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnReceiveEventTest004 start";
    try {
        Want want;
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_CONNECTED);
        EventFwk::CommonEventData eventData(want);
        auto dataSyncManager = std::make_shared<DataSyncManager>();
        auto userStatusListener = std::make_shared<UserStatusListener>(dataSyncManager);
        auto subscriber = std::make_shared<UserStatusSubscriber>(EventFwk::CommonEventSubscribeInfo(),
            userStatusListener);
        subscriber->OnReceiveEvent(eventData);
        EXPECT_NE(subscriber->listener_, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnReceiveEventTest004 failed";
    }
    GTEST_LOG_(INFO) << "OnReceiveEventTest004 end";
}
}