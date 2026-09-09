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

#include "common_event_manager.h"
#include "common_event_support.h"
#include "ffrt_inner.h"
#include "network_signal_strength_listener.h"
#include "network_status.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using namespace CloudFile;
using Want = OHOS::AAFwk::Want;

class NetworkSignalStrengthListenerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};
void NetworkSignalStrengthListenerTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void NetworkSignalStrengthListenerTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void NetworkSignalStrengthListenerTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void NetworkSignalStrengthListenerTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: StopTest001
 * @tc.desc: Verify the Stop function
 * @tc.type: FUNC
 * @tc.require: IB3SWZ
 */
HWTEST_F(NetworkSignalStrengthListenerTest, StopTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StopTest001 start";
    try {
        auto dataSyncManager = std::make_shared<DataSyncManager>();
        auto networkSignalStrengthListener = std::make_shared<NetworkSignalStrengthListener>(dataSyncManager);
        networkSignalStrengthListener->Start();
        EXPECT_NE(networkSignalStrengthListener->commonEventSubscriber_, nullptr);
        networkSignalStrengthListener->Stop();
        EXPECT_EQ(networkSignalStrengthListener->commonEventSubscriber_, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StopTest001 failed";
    }
    GTEST_LOG_(INFO) << "StopTest001 end";
}

/**
 * @tc.name: OnReceiveEventTest001
 * @tc.desc: Verify the OnReceiveEvent function
 * @tc.type: FUNC
 * @tc.require: IB3SWZ
 */
HWTEST_F(NetworkSignalStrengthListenerTest, OnReceiveEventTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnReceiveEventTest001 start";
    try {
        Want want;
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_SIGNAL_INFO_CHANGED);
        EventFwk::CommonEventData eventData(want);
        auto dataSyncManager = std::make_shared<DataSyncManager>();
        auto networkSignalStrengthListener = std::make_shared<NetworkSignalStrengthListener>(dataSyncManager);
        auto subscriber = std::make_shared<NetworkSignalStrengthSubscriber>(EventFwk::CommonEventSubscribeInfo(),
            networkSignalStrengthListener);
        NetworkStatus::SetNetConnStatus(NetworkStatus::NetConnStatus::ETHERNET_CONNECT);
        subscriber->OnReceiveEvent(eventData);
        ffrt::wait();
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
 * @tc.require: IB3SWZ
 */
HWTEST_F(NetworkSignalStrengthListenerTest, OnReceiveEventTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnReceiveEventTest002 start";
    try {
        Want want;
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_WIFI_RSSI_VALUE);
        EventFwk::CommonEventData eventData(want);
        auto dataSyncManager = std::make_shared<DataSyncManager>();
        auto networkSignalStrengthListener = std::make_shared<NetworkSignalStrengthListener>(dataSyncManager);
        auto subscriber = std::make_shared<NetworkSignalStrengthSubscriber>(EventFwk::CommonEventSubscribeInfo(),
            networkSignalStrengthListener);
        subscriber->OnReceiveEvent(eventData);
        ffrt::wait();
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
 * @tc.require: IB3SWZ
 */
HWTEST_F(NetworkSignalStrengthListenerTest, OnReceiveEventTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnReceiveEventTest003 start";
    try {
        Want want;
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_SIGNAL_INFO_CHANGED);
        EventFwk::CommonEventData eventData(want);
        auto dataSyncManager = std::make_shared<DataSyncManager>();
        auto networkSignalStrengthListener = std::make_shared<NetworkSignalStrengthListener>(dataSyncManager);
        auto subscriber = std::make_shared<NetworkSignalStrengthSubscriber>(EventFwk::CommonEventSubscribeInfo(),
            networkSignalStrengthListener);
        NetworkStatus::SetNetConnStatus(NetworkStatus::NetConnStatus::CELLULAR_CONNECT);
        subscriber->OnReceiveEvent(eventData);
        ffrt::wait();
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
 * @tc.require: IB3SWZ
 */
HWTEST_F(NetworkSignalStrengthListenerTest, OnReceiveEventTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnReceiveEventTest004 start";
    try {
        Want want;
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_WIFI_RSSI_VALUE);
        EventFwk::CommonEventData eventData(want);
        auto dataSyncManager = std::make_shared<DataSyncManager>();
        auto networkSignalStrengthListener = std::make_shared<NetworkSignalStrengthListener>(dataSyncManager);
        auto subscriber = std::make_shared<NetworkSignalStrengthSubscriber>(EventFwk::CommonEventSubscribeInfo(),
            networkSignalStrengthListener);
        NetworkStatus::SetNetConnStatus(NetworkStatus::NetConnStatus::WIFI_CONNECT);
        subscriber->OnReceiveEvent(eventData);
        ffrt::wait();
        EXPECT_NE(subscriber->listener_, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnReceiveEventTest004 failed";
    }
    GTEST_LOG_(INFO) << "OnReceiveEventTest004 end";
}

/**
 * @tc.name: OnReceiveEventTest005
 * @tc.desc: Verify the OnReceiveEvent function
 * @tc.type: FUNC
 * @tc.require: IB3SWZ
 */
HWTEST_F(NetworkSignalStrengthListenerTest, OnReceiveEventTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnReceiveEventTest005 start";
    try {
        Want want;
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_UNLOCKED);
        EventFwk::CommonEventData eventData(want);
        auto dataSyncManager = std::make_shared<DataSyncManager>();
        auto networkSignalStrengthListener = std::make_shared<NetworkSignalStrengthListener>(dataSyncManager);
        auto subscriber = std::make_shared<NetworkSignalStrengthSubscriber>(EventFwk::CommonEventSubscribeInfo(),
            networkSignalStrengthListener);
        NetworkStatus::SetNetConnStatus(NetworkStatus::NetConnStatus::WIFI_CONNECT);
        subscriber->OnReceiveEvent(eventData);
        ffrt::wait();
        EXPECT_NE(subscriber->listener_, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnReceiveEventTest005 failed";
    }
    GTEST_LOG_(INFO) << "OnReceiveEventTest005 end";
}

/**
 * @tc.name: OnReceiveEventTest006
 * @tc.desc: Verify the OnReceiveEvent function
 * @tc.type: FUNC
 * @tc.require: IB3SWZ
 */
HWTEST_F(NetworkSignalStrengthListenerTest, OnReceiveEventTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnReceiveEventTest006 start";
    try {
        Want want;
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_WIFI_RSSI_VALUE);
        want.SetParam("wifiSignalLevel", 5);
        EventFwk::CommonEventData eventData(want);
        auto dataSyncManager = std::make_shared<DataSyncManager>();
        auto networkSignalStrengthListener = std::make_shared<NetworkSignalStrengthListener>(dataSyncManager);
        auto subscriber = std::make_shared<NetworkSignalStrengthSubscriber>(EventFwk::CommonEventSubscribeInfo(),
            networkSignalStrengthListener);
        NetworkStatus::SetNetConnStatus(NetworkStatus::NetConnStatus::WIFI_CONNECT);
        subscriber->OnReceiveEvent(eventData);
        ffrt::wait();
        EXPECT_NE(subscriber->listener_, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnReceiveEventTest006 failed";
    }
    GTEST_LOG_(INFO) << "OnReceiveEventTest006 end";
}

/**
 * @tc.name: OnReceiveEventTest007
 * @tc.desc: Verify the OnReceiveEvent function
 * @tc.type: FUNC
 * @tc.require: IB3SWZ
 */
HWTEST_F(NetworkSignalStrengthListenerTest, OnReceiveEventTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnReceiveEventTest007 start";
    try {
        Want want;
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_SIGNAL_INFO_CHANGED);
        std::vector<std::string> contentStr;
        std::string signalInfo = "signalLevel:5,gsmBer:0";
        contentStr.push_back(signalInfo);
        want.SetParam("signalInfos", contentStr);
        EventFwk::CommonEventData eventData(want);
        auto dataSyncManager = std::make_shared<DataSyncManager>();
        auto networkSignalStrengthListener = std::make_shared<NetworkSignalStrengthListener>(dataSyncManager);
        auto subscriber = std::make_shared<NetworkSignalStrengthSubscriber>(EventFwk::CommonEventSubscribeInfo(),
            networkSignalStrengthListener);
        NetworkStatus::SetNetConnStatus(NetworkStatus::NetConnStatus::CELLULAR_CONNECT);
        subscriber->OnReceiveEvent(eventData);
        ffrt::wait();
        EXPECT_NE(subscriber->listener_, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnReceiveEventTest007 failed";
    }
    GTEST_LOG_(INFO) << "OnReceiveEventTest007 end";
}

/**
 * @tc.name: OnReceiveEventTest008
 * @tc.desc: Verify the OnReceiveEvent function
 * @tc.type: FUNC
 * @tc.require: IB3SWZ
 */
HWTEST_F(NetworkSignalStrengthListenerTest, OnReceiveEventTest008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnReceiveEventTest008 start";
    try {
        Want want;
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_SIGNAL_INFO_CHANGED);
        EventFwk::CommonEventData eventData(want);
        auto dataSyncManager = std::make_shared<DataSyncManager>();
        auto networkSignalStrengthListener = std::make_shared<NetworkSignalStrengthListener>(dataSyncManager);
        auto subscriber = std::make_shared<NetworkSignalStrengthSubscriber>(EventFwk::CommonEventSubscribeInfo(),
            networkSignalStrengthListener);
        NetworkStatus::SetNetConnStatus(NetworkStatus::NetConnStatus::WIFI_CONNECT);
        subscriber->OnReceiveEvent(eventData);
        ffrt::wait();
        EXPECT_NE(subscriber->listener_, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnReceiveEventTest008 failed";
    }
    GTEST_LOG_(INFO) << "OnReceiveEventTest008 end";
}

/**
 * @tc.name: OnReceiveEventTest009
 * @tc.desc: Verify the OnReceiveEvent function
 * @tc.type: FUNC
 * @tc.require: IB3SWZ
 */
HWTEST_F(NetworkSignalStrengthListenerTest, OnReceiveEventTest009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnReceiveEventTest009 start";
    try {
        Want want;
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_WIFI_RSSI_VALUE);
        EventFwk::CommonEventData eventData(want);
        auto dataSyncManager = std::make_shared<DataSyncManager>();
        auto networkSignalStrengthListener = std::make_shared<NetworkSignalStrengthListener>(dataSyncManager);
        auto subscriber = std::make_shared<NetworkSignalStrengthSubscriber>(EventFwk::CommonEventSubscribeInfo(),
            networkSignalStrengthListener);
        NetworkStatus::SetNetConnStatus(NetworkStatus::NetConnStatus::CELLULAR_CONNECT);
        subscriber->OnReceiveEvent(eventData);
        ffrt::wait();
        EXPECT_NE(subscriber->listener_, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnReceiveEventTest009 failed";
    }
    GTEST_LOG_(INFO) << "OnReceiveEventTest009 end";
}

/**
 * @tc.name: OnReceiveEventTest010
 * @tc.desc: Verify the OnReceiveEvent function
 * @tc.type: FUNC
 * @tc.require: IB3SWZ
 */
HWTEST_F(NetworkSignalStrengthListenerTest, OnReceiveEventTest010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnReceiveEventTest010 start";
    try {
        Want want;
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_SIGNAL_INFO_CHANGED);
        std::vector<std::string> contentStr;
        std::string signalInfo = "signalevel:5,gsmBer:0";
        contentStr.push_back(signalInfo);
        want.SetParam("signalInfos", contentStr);
        EventFwk::CommonEventData eventData(want);
        auto dataSyncManager = std::make_shared<DataSyncManager>();
        auto networkSignalStrengthListener = std::make_shared<NetworkSignalStrengthListener>(dataSyncManager);
        auto subscriber = std::make_shared<NetworkSignalStrengthSubscriber>(EventFwk::CommonEventSubscribeInfo(),
            networkSignalStrengthListener);
        NetworkStatus::SetNetConnStatus(NetworkStatus::NetConnStatus::CELLULAR_CONNECT);
        subscriber->OnReceiveEvent(eventData);
        ffrt::wait();
        EXPECT_NE(subscriber->listener_, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnReceiveEventTest010 failed";
    }
    GTEST_LOG_(INFO) << "OnReceiveEventTest010 end";
}

/**
 * @tc.name: OnReceiveEventTest011
 * @tc.desc: Verify the OnReceiveEvent function
 * @tc.type: FUNC
 * @tc.require: IB3SWZ
 */
HWTEST_F(NetworkSignalStrengthListenerTest, OnReceiveEventTest011, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnReceiveEventTest011 start";
    try {
        Want want;
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_SIGNAL_INFO_CHANGED);
        std::vector<std::string> contentStr;
        std::string signalInfo = "signalLevel:a,gsmBer:a";
        contentStr.push_back(signalInfo);
        want.SetParam("signalInfos", contentStr);
        EventFwk::CommonEventData eventData(want);
        auto dataSyncManager = std::make_shared<DataSyncManager>();
        auto networkSignalStrengthListener = std::make_shared<NetworkSignalStrengthListener>(dataSyncManager);
        auto subscriber = std::make_shared<NetworkSignalStrengthSubscriber>(EventFwk::CommonEventSubscribeInfo(),
            networkSignalStrengthListener);
        NetworkStatus::SetNetConnStatus(NetworkStatus::NetConnStatus::CELLULAR_CONNECT);
        subscriber->OnReceiveEvent(eventData);
        ffrt::wait();
        EXPECT_NE(subscriber->listener_, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnReceiveEventTest011 failed";
    }
    GTEST_LOG_(INFO) << "OnReceiveEventTest011 end";
}

/**
 * @tc.name: OnReceiveEventTest012
 * @tc.desc: Verify the OnReceiveEvent function
 * @tc.type: FUNC
 * @tc.require: IB3SWZ
 */
HWTEST_F(NetworkSignalStrengthListenerTest, OnReceiveEventTest012, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnReceiveEventTest012 start";
    try {
        Want want;
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_SIGNAL_INFO_CHANGED);
        std::vector<std::string> contentStr;
        std::string signalInfo = "signalLevel:5";
        contentStr.push_back(signalInfo);
        want.SetParam("signalInfos", contentStr);
        EventFwk::CommonEventData eventData(want);
        auto dataSyncManager = std::make_shared<DataSyncManager>();
        auto networkSignalStrengthListener = std::make_shared<NetworkSignalStrengthListener>(dataSyncManager);
        auto subscriber = std::make_shared<NetworkSignalStrengthSubscriber>(EventFwk::CommonEventSubscribeInfo(),
            networkSignalStrengthListener);
        NetworkStatus::SetNetConnStatus(NetworkStatus::NetConnStatus::CELLULAR_CONNECT);
        subscriber->OnReceiveEvent(eventData);
        ffrt::wait();
        EXPECT_NE(subscriber->listener_, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnReceiveEventTest012 failed";
    }
    GTEST_LOG_(INFO) << "OnReceiveEventTest012 end";
}
}