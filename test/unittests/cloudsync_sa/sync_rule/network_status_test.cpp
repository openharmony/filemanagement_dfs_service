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
#include <cstdint>
#include <future>
#include <thread>
#include <unistd.h>

#include "dfs_error.h"
#include "net_conn_client.h"
#include "parameter.h"
#include "net_conn_callback_observer.h"
#include "network_status.h"
#include "utils_log.h"
#include "net_conn_client_mock.h"
#include "net_handle.h"
#include "settings_data_manager.h"
#include "network_set_manager.h"

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using namespace CloudFile;

class NetworkStatusTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<NetConnClientMock> dfsNetConnClient_ = nullptr;
};
void NetworkStatusTest::SetUpTestCase(void)
{
    dfsNetConnClient_ = make_shared<NetConnClientMock>();
    NetConnClientMock::dfsNetConnClient = dfsNetConnClient_;
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void NetworkStatusTest::TearDownTestCase(void)
{
    NetConnClientMock::dfsNetConnClient = nullptr;
    dfsNetConnClient_ = nullptr;
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void NetworkStatusTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void NetworkStatusTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: RegisterNetConnCallbackTest
 * @tc.desc: Verify the RegisterNetConnCallback function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetworkStatusTest, RegisterNetConnCallbackTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterNetConnCallbackTest Start";
    try {
        auto dataSyncManager = std::make_shared<DataSyncManager>();
        int32_t ret = NetworkStatus::RegisterNetConnCallback(dataSyncManager);
        EXPECT_EQ(ret, E_GET_NETWORK_MANAGER_FAILED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " RegisterNetConnCallbackTest FAILED";
    }
    GTEST_LOG_(INFO) << "RegisterNetConnCallbackTest End";
}

/**
 * @tc.name: GetDefaultNetTest001
 * @tc.desc: Verify the GetDefaultNet function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetworkStatusTest, GetDefaultNetTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetDefaultNetTest001 Start";
    EXPECT_CALL(*dfsNetConnClient_, GetDefaultNet(_)).WillOnce(Return(E_GET_NETWORK_MANAGER_FAILED));
    NetworkStatus netStatus;
    NetworkStatus::NetConnStatus status = NetworkStatus::WIFI_CONNECT;
    int32_t ret = netStatus.GetDefaultNet(status);
    EXPECT_EQ(ret, E_GET_NETWORK_MANAGER_FAILED);
    EXPECT_EQ(status, NetworkStatus::WIFI_CONNECT);
    GTEST_LOG_(INFO) << "GetDefaultNetTest001 End";
}

/**
 * @tc.name: GetDefaultNetTest002
 * @tc.desc: Verify the GetDefaultNet function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetworkStatusTest, GetDefaultNetTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetDefaultNetTest002 Start";
    EXPECT_CALL(*dfsNetConnClient_, GetDefaultNet(_)).WillOnce(Return(NetManagerStandard::NETMANAGER_SUCCESS));
    NetworkStatus netStatus;
    NetworkStatus::NetConnStatus status = NetworkStatus::WIFI_CONNECT;
    int32_t ret = netStatus.GetDefaultNet(status);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(status, NetworkStatus::NO_NETWORK);
    GTEST_LOG_(INFO) << "GetDefaultNetTest002 End";
}

/**
 * @tc.name: SetNetConnStatusTest001
 * @tc.desc: Verify the SetNetConnStatus function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetworkStatusTest, SetNetConnStatusTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetNetConnStatusTest001 Start";
    try {
        NetManagerStandard::NetAllCapabilities netAllCap;
        NetworkStatus::SetNetConnStatus(netAllCap);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " SetNetConnStatusTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "SetNetConnStatusTest001 End";
}

/**
 * @tc.name: SetNetConnStatusTest002
 * @tc.desc: Verify the SetNetConnStatus function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetworkStatusTest, SetNetConnStatusTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetNetConnStatusTest001 Start";
    try {
        NetManagerStandard::NetAllCapabilities netAllCap;
        netAllCap.netCaps_.insert(NetManagerStandard::NetCap::NET_CAPABILITY_INTERNET);
        NetworkStatus::SetNetConnStatus(netAllCap);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " SetNetConnStatusTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "SetNetConnStatusTest001 End";
}

/**
 * @tc.name: SetNetConnStatusTest003
 * @tc.desc: Verify the SetNetConnStatus function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetworkStatusTest, SetNetConnStatusTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetNetConnStatusTest001 Start";
    try {
        NetManagerStandard::NetAllCapabilities netAllCap;
        netAllCap.netCaps_.insert(NetManagerStandard::NetCap::NET_CAPABILITY_INTERNET);
        netAllCap.bearerTypes_.insert(NetManagerStandard::BEARER_ETHERNET);
        NetworkStatus::SetNetConnStatus(netAllCap);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " SetNetConnStatusTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "SetNetConnStatusTest001 End";
}

/**
 * @tc.name: SetNetConnStatusTest004
 * @tc.desc: Verify the SetNetConnStatus function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetworkStatusTest, SetNetConnStatusTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetNetConnStatusTest001 Start";
    try {
        NetManagerStandard::NetAllCapabilities netAllCap;
        netAllCap.netCaps_.insert(NetManagerStandard::NetCap::NET_CAPABILITY_INTERNET);
        netAllCap.bearerTypes_.insert(NetManagerStandard::BEARER_WIFI);
        NetworkStatus::SetNetConnStatus(netAllCap);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " SetNetConnStatusTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "SetNetConnStatusTest001 End";
}

/**
 * @tc.name: SetNetConnStatusTest005
 * @tc.desc: Verify the SetNetConnStatus function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetworkStatusTest, SetNetConnStatusTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetNetConnStatusTest001 Start";
    try {
        NetManagerStandard::NetAllCapabilities netAllCap;
        netAllCap.netCaps_.insert(NetManagerStandard::NetCap::NET_CAPABILITY_INTERNET);
        netAllCap.bearerTypes_.insert(NetManagerStandard::BEARER_CELLULAR);
        NetworkStatus::SetNetConnStatus(netAllCap);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " SetNetConnStatusTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "SetNetConnStatusTest001 End";
}

/**
 * @tc.name: GetAndRegisterNetworkTest
 * @tc.desc: Verify the GetAndRegisterNetwork function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetworkStatusTest, GetAndRegisterNetworkTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAndRegisterNetworkTest Start";
    try {
        auto dataSyncManager = std::make_shared<DataSyncManager>();
        int32_t ret = NetworkStatus::GetAndRegisterNetwork(dataSyncManager);
        EXPECT_EQ(ret, E_GET_NETWORK_MANAGER_FAILED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetAndRegisterNetworkTest FAILED";
    }
    GTEST_LOG_(INFO) << "GetAndRegisterNetworkTest End";
}

/**
 * @tc.name: InitNetworkTest
 * @tc.desc: Verify the InitNetwork function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetworkStatusTest, InitNetworkTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InitNetworkTest Start";
    try {
        auto dataSyncManager = std::make_shared<DataSyncManager>();
        NetworkStatus::InitNetwork(dataSyncManager);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " InitNetworkTest FAILED";
    }
    GTEST_LOG_(INFO) << "InitNetworkTest End";
}

/**
 * @tc.name: CheckMobileNetworkTest001
 * @tc.desc: Verify the CheckMobileNetwork function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetworkStatusTest, CheckMobileNetworkTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckMobileNetworkTest Start";
    try {
        NetworkStatus networkStatus;
        string bundleName = "";
        int32_t userId = 100;
        bool ret = networkStatus.CheckMobileNetwork(bundleName, userId);
        EXPECT_EQ(ret, true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckMobileNetworkTest FAILED";
    }
    GTEST_LOG_(INFO) << "CheckMobileNetworkTest End";
}

/**
 * @tc.name: CheckMobileNetworkTest002
 * @tc.desc: Verify the CheckMobileNetwork function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetworkStatusTest, CheckMobileNetworkTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckMobileNetworkTest Start";
    try {
        NetworkStatus networkStatus;
        string bundleName = "com.ohos.photos";
        int32_t userId = 1;
        bool ret = networkStatus.CheckMobileNetwork(bundleName, userId);
        EXPECT_EQ(ret, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckMobileNetworkTest FAILED";
    }
    GTEST_LOG_(INFO) << "CheckMobileNetworkTest End";
}

/**
 * @tc.name: CheckMobileNetworkTest003
 * @tc.desc: Verify the CheckMobileNetwork function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetworkStatusTest, CheckMobileNetworkTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckMobileNetworkTest Start";
    try {
        NetworkStatus networkStatus;
        string bundleName = "com.ohos.photos";
        int32_t userId = 100;
        networkStatus.SetNetConnStatus(NetworkStatus::WIFI_CONNECT);
        bool ret = networkStatus.CheckMobileNetwork(bundleName, userId);
        EXPECT_EQ(ret, true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckMobileNetworkTest FAILED";
    }
    GTEST_LOG_(INFO) << "CheckMobileNetworkTest End";
}

/**
 * @tc.name: CheckMobileNetworkTest004
 * @tc.desc: Verify the CheckMobileNetwork function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetworkStatusTest, CheckMobileNetworkTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckMobileNetworkTest Start";
    try {
        NetworkStatus networkStatus;
        string bundleName = "com.ohos.photos";
        int32_t userId = 100;
        networkStatus.SetNetConnStatus(NetworkStatus::NO_NETWORK);
        bool ret = networkStatus.CheckMobileNetwork(bundleName, userId);
        EXPECT_EQ(ret, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckMobileNetworkTest FAILED";
    }
    GTEST_LOG_(INFO) << "CheckMobileNetworkTest End";
}

/**
 * @tc.name: CheckMobileNetworkTest005
 * @tc.desc: Verify the CheckMobileNetwork function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetworkStatusTest, CheckMobileNetworkTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckMobileNetworkTest005 Start";
    try {
        NetworkStatus networkStatus;
        string bundleName = "com.ohos.ailife";
        int32_t userId = 100;
        SettingsDataManager::settingsDataMap_.EnsureInsert("photos_mobile_data_sync", "1");
        bool ret = networkStatus.CheckMobileNetwork(bundleName, userId);
        EXPECT_EQ(ret, true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckMobileNetworkTest005 FAILED";
    }
    GTEST_LOG_(INFO) << "CheckMobileNetworkTest005 End";
}

/**
 * @tc.name: CheckMobileNetworkTest006
 * @tc.desc: Verify the CheckMobileNetwork function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetworkStatusTest, CheckMobileNetworkTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckMobileNetworkTest006 Start";
    try {
        NetworkStatus networkStatus;
        string bundleName = "com.ohos.photos";
        int32_t userId = 100;
        std::string key = std::to_string(userId) + "/" + bundleName;
        NetworkSetManager::cellularNetMap_.EnsureInsert(key, true);
        bool ret = networkStatus.CheckMobileNetwork(bundleName, userId);
        EXPECT_EQ(ret, true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckMobileNetworkTest006 FAILED";
    }
    GTEST_LOG_(INFO) << "CheckMobileNetworkTest006 End";
}

/**
 * @tc.name: CheckMobileNetworkTest007
 * @tc.desc: Verify the CheckMobileNetwork function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetworkStatusTest, CheckMobileNetworkTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckMobileNetworkTest007 Start";
    try {
        NetworkStatus networkStatus;
        string bundleName = "com.ohos.photos";
        int32_t userId = 100;
        std::string key = std::to_string(userId) + "/" + bundleName;
        networkStatus.SetNetConnStatus(NetworkStatus::WIFI_CONNECT);
        NetworkSetManager::cellularNetMap_.EnsureInsert(key, false);
        bool ret = networkStatus.CheckMobileNetwork(bundleName, userId);
        EXPECT_EQ(ret, true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckMobileNetworkTest007 FAILED";
    }
    GTEST_LOG_(INFO) << "CheckMobileNetworkTest007 End";
}

/**
 * @tc.name: CheckMobileNetworkTest008
 * @tc.desc: Verify the CheckMobileNetwork function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetworkStatusTest, CheckMobileNetworkTest008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckMobileNetworkTest008 Start";
    try {
        NetworkStatus networkStatus;
        string bundleName = "com.ohos.photos";
        int32_t userId = 100;
        std::string key = std::to_string(userId) + "/" + bundleName;
        networkStatus.SetNetConnStatus(NetworkStatus::NO_NETWORK);
        NetworkSetManager::cellularNetMap_.EnsureInsert(key, false);
        bool ret = networkStatus.CheckMobileNetwork(bundleName, userId);
        EXPECT_EQ(ret, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckMobileNetworkTest008 FAILED";
    }
    GTEST_LOG_(INFO) << "CheckMobileNetworkTest008 End";
}

/**
 * @tc.name: CheckMobileNetworkTest009
 * @tc.desc: Verify the CheckMobileNetwork function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetworkStatusTest, CheckMobileNetworkTest009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckMobileNetworkTest009 Start";
    try {
        NetworkStatus networkStatus;
        string bundleName = "xxxxxxxxx";
        int32_t userId = 100;
        std::string key = std::to_string(userId) + "/" + bundleName;
        networkStatus.SetNetConnStatus(NetworkStatus::NO_NETWORK);
        NetworkSetManager::cellularNetMap_.EnsureInsert(key, false);
        bool ret = networkStatus.CheckMobileNetwork(bundleName, userId);
        EXPECT_EQ(ret, true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckMobileNetworkTest009 FAILED";
    }
    GTEST_LOG_(INFO) << "CheckMobileNetworkTest009 End";
}

/**
 * @tc.name: CheckMobileNetworkTest010
 * @tc.desc: Verify the CheckMobileNetwork function
 * @tc.type: FUNC
 * @tc.require: ICQFDC
 */
HWTEST_F(NetworkStatusTest, CheckMobileNetworkTest010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckMobileNetworkTest010 Start";
    try {
        NetworkStatus networkStatus;
        string bundleName = "com.ohos.ailife";
        int32_t userId = 100;
        SettingsDataManager::settingsDataMap_.EnsureInsert("photos_mobile_data_sync", "0");
        bool ret = networkStatus.CheckMobileNetwork(bundleName, userId);
        EXPECT_EQ(ret, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckMobileNetworkTest005 FAILED";
    }
    GTEST_LOG_(INFO) << "CheckMobileNetworkTest005 End";
}

/**
 * @tc.name: CheckNetworkTest001
 * @tc.desc: Verify the CheckNetwork function
 * @tc.type: FUNC
 * @tc.require: ICQFDC
 */
HWTEST_F(NetworkStatusTest, CheckNetworkTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckNetworkTest Start";
    try {
        NetworkStatus networkStatus;
        string bundleName = "";
        int32_t userId = 100;
        bool ret = networkStatus.CheckNetwork(bundleName, userId);
        EXPECT_EQ(ret, true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckNetworkTest FAILED";
    }
    GTEST_LOG_(INFO) << "CheckNetworkTest End";
}

/**
 * @tc.name: CheckNetworkTest002
 * @tc.desc: Verify the CheckNetwork function
 * @tc.type: FUNC
 * @tc.require: ICQFDC
 */
HWTEST_F(NetworkStatusTest, CheckNetworkTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckNetworkTest Start";
    try {
        NetworkStatus networkStatus;
        string bundleName = "com.ohos.photos";
        int32_t userId = 1;
        SettingsDataManager::settingsDataMap_.EnsureInsert("photo_network_connection_status", "off");
        bool ret = networkStatus.CheckNetwork(bundleName, userId);
        EXPECT_EQ(ret, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckNetworkTest FAILED";
    }
    GTEST_LOG_(INFO) << "CheckNetworkTest End";
}

/**
 * @tc.name: CheckNetworkTest003
 * @tc.desc: Verify the CheckNetwork function
 * @tc.type: FUNC
 * @tc.require: ICQFDC
 */
HWTEST_F(NetworkStatusTest, CheckNetworkTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckNetworkTest Start";
    try {
        NetworkStatus networkStatus;
        string bundleName = "com.ohos.photos";
        int32_t userId = 100;
        SettingsDataManager::settingsDataMap_.EnsureInsert("photo_network_connection_status", "on");
        bool ret = networkStatus.CheckNetwork(bundleName, userId);
        EXPECT_EQ(ret, true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckNetworkTest FAILED";
    }
    GTEST_LOG_(INFO) << "CheckNetworkTest End";
}

/**
 * @tc.name: CheckNetworkTest004
 * @tc.desc: Verify the CheckNetwork function
 * @tc.type: FUNC
 * @tc.require: ICQFDC
 */
HWTEST_F(NetworkStatusTest, CheckNetworkTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckNetworkTest004 Start";
    try {
        NetworkStatus networkStatus;
        string bundleName = "com.ohos.ailife";
        int32_t userId = 100;
        SettingsDataManager::settingsDataMap_.EnsureInsert("photo_network_connection_status", "on");
        bool ret = networkStatus.CheckNetwork(bundleName, userId);
        EXPECT_EQ(ret, true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckNetworkTest004 FAILED";
    }
    GTEST_LOG_(INFO) << "CheckNetworkTest004 End";
}

/**
 * @tc.name: CheckNetworkTest005
 * @tc.desc: Verify the CheckNetwork function
 * @tc.type: FUNC
 * @tc.require: ICQFDC
 */
HWTEST_F(NetworkStatusTest, CheckNetworkTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckNetworkTest005 Start";
    try {
        NetworkStatus networkStatus;
        string bundleName = "com.ohos.ailife";
        int32_t userId = 100;
        SettingsDataManager::settingsDataMap_.EnsureInsert("photo_network_connection_status", "false");
        bool ret = networkStatus.CheckNetwork(bundleName, userId);
        EXPECT_EQ(ret, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckNetworkTest005 FAILED";
    }
    GTEST_LOG_(INFO) << "CheckNetworkTest005 End";
}

/**
 * @tc.name: OnNetworkAvailTest001
 * @tc.desc: Verify the OnNetworkAvail function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetworkStatusTest, OnNetworkAvailTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnNetworkAvailTest Start";
    try {
        NetworkStatus networkStatus;
        networkStatus.SetNetConnStatus(NetworkStatus::WIFI_CONNECT);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnNetworkAvailTest FAILED";
    }
    GTEST_LOG_(INFO) << "OnNetworkAvailTest End";
}

/**
 * @tc.name: CheckWifiOrEthernetTest001
 * @tc.desc: Verify the CheckNetwork function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetworkStatusTest, CheckWifiOrEthernetTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckWifiOrEthernetTest Start";
    try {
        NetworkStatus networkStatus;
        networkStatus.SetNetConnStatus(NetworkStatus::WIFI_CONNECT);
        bool ret = networkStatus.CheckWifiOrEthernet();
        EXPECT_EQ(ret, true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckWifiOrEthernetTest FAILED";
    }
    GTEST_LOG_(INFO) << "CheckWifiOrEthernetTest End";
}

/**
 * @tc.name: CheckWifiOrEthernetTest002
 * @tc.desc: Verify the CheckNetwork function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetworkStatusTest, CheckWifiOrEthernetTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckWifiOrEthernetTest Start";
    try {
        NetworkStatus networkStatus;
        networkStatus.SetNetConnStatus(NetworkStatus::ETHERNET_CONNECT);
        bool ret = networkStatus.CheckWifiOrEthernet();
        EXPECT_EQ(ret, true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckWifiOrEthernetTest FAILED";
    }
    GTEST_LOG_(INFO) << "CheckWifiOrEthernetTest End";
}

// Reset netStatus_ so each case below exercises DoInitialFetch afresh.
static void ResetNetStatusForTest()
{
    NetworkStatus::netStatus_ = NetworkStatus::NETWORK_NOT_INIT;
}

/**
 * @tc.name: DoInitialFetchFailTest
 * @tc.desc: DoInitialFetch leaves netStatus_ NETWORK_NOT_INIT when GetDefaultNet fails
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetworkStatusTest, DoInitialFetchFailTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoInitialFetchFailTest Start";
    ResetNetStatusForTest();
    EXPECT_EQ(NetworkStatus::netStatus_, NetworkStatus::NETWORK_NOT_INIT);
    EXPECT_CALL(*dfsNetConnClient_, GetDefaultNet(_)).WillOnce(Return(E_GET_NETWORK_MANAGER_FAILED));
    NetworkStatus::DoInitialFetch();
    EXPECT_EQ(NetworkStatus::netStatus_, NetworkStatus::NETWORK_NOT_INIT);    // fetch failed, untouched
    EXPECT_EQ(NetworkStatus::GetNetConnStatus(), NetworkStatus::NO_NETWORK);  // NOT_INIT maps to NO_NETWORK
    GTEST_LOG_(INFO) << "DoInitialFetchFailTest End";
}

/**
 * @tc.name: DoInitialFetchSuccessTest
 * @tc.desc: DoInitialFetch goes through the fetch-success path
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetworkStatusTest, DoInitialFetchSuccessTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoInitialFetchSuccessTest Start";
    ResetNetStatusForTest();
    // Mock returns success but leaves netHandle default (netId 0 < MIN_VALID_NETID),
    // so GetDefaultNet() records NO_NETWORK and returns E_OK -> initial log path.
    EXPECT_CALL(*dfsNetConnClient_, GetDefaultNet(_)).WillOnce(Return(NetManagerStandard::NETMANAGER_SUCCESS));
    NetworkStatus::DoInitialFetch();
    EXPECT_EQ(NetworkStatus::netStatus_, NetworkStatus::NO_NETWORK);
    GTEST_LOG_(INFO) << "DoInitialFetchSuccessTest End";
}

/**
 * @tc.name: DoInitialFetchTimeoutTest
 * @tc.desc: DoInitialFetch leaves netStatus_ NETWORK_NOT_INIT when the bounded wait times out
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetworkStatusTest, DoInitialFetchTimeoutTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoInitialFetchTimeoutTest Start";
    ResetNetStatusForTest();
    EXPECT_EQ(NetworkStatus::netStatus_, NetworkStatus::NETWORK_NOT_INIT);
    auto blocker = std::make_shared<std::promise<void>>();
    std::shared_future<void> gate = blocker->get_future().share();
    EXPECT_CALL(*dfsNetConnClient_, GetDefaultNet(_))
        .WillOnce(InvokeWithoutArgs([gate]() { gate.wait(); return E_GET_NETWORK_MANAGER_FAILED; }));
    NetworkStatus::DoInitialFetch();
    EXPECT_EQ(NetworkStatus::netStatus_, NetworkStatus::NETWORK_NOT_INIT);    // timed out, untouched
    EXPECT_EQ(NetworkStatus::GetNetConnStatus(), NetworkStatus::NO_NETWORK);
    blocker->set_value();
    GTEST_LOG_(INFO) << "DoInitialFetchTimeoutTest End";
}

/**
 * @tc.name: CallbackRecoversAfterFetchFailTest
 * @tc.desc: A later wifi callback recovers the status after the fetch failed (issue scenario)
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetworkStatusTest, CallbackRecoversAfterFetchFailTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CallbackRecoversAfterFetchFailTest Start";
    ResetNetStatusForTest();
    EXPECT_CALL(*dfsNetConnClient_, GetDefaultNet(_)).WillOnce(Return(E_GET_NETWORK_MANAGER_FAILED));
    NetworkStatus::DoInitialFetch();                                       // startup: SA not ready
    NetManagerStandard::NetAllCapabilities netAllCap;
    netAllCap.netCaps_.insert(NetManagerStandard::NetCap::NET_CAPABILITY_INTERNET);
    netAllCap.bearerTypes_.insert(NetManagerStandard::BEARER_WIFI);
    NetworkStatus::SetNetConnStatus(netAllCap);                            // wifi callback arrives
    EXPECT_EQ(NetworkStatus::netStatus_, NetworkStatus::WIFI_CONNECT);     // recovered
    EXPECT_EQ(NetworkStatus::GetNetConnStatus(), NetworkStatus::WIFI_CONNECT);
    GTEST_LOG_(INFO) << "CallbackRecoversAfterFetchFailTest End";
}

/**
 * @tc.name: DoInitialFetchSkippedWhenCallbackFirstTest
 * @tc.desc: DoInitialFetch skips fetching when a callback already set netStatus_
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetworkStatusTest, DoInitialFetchSkippedWhenCallbackFirstTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoInitialFetchSkippedWhenCallbackFirstTest Start";
    ResetNetStatusForTest();
    NetworkStatus::SetNetConnStatus(NetworkStatus::WIFI_CONNECT);  // callback arrived first
    EXPECT_CALL(*dfsNetConnClient_, GetDefaultNet(_)).Times(0);    // must not fetch
    NetworkStatus::DoInitialFetch();
    EXPECT_EQ(NetworkStatus::netStatus_, NetworkStatus::WIFI_CONNECT);  // unchanged
    GTEST_LOG_(INFO) << "DoInitialFetchSkippedWhenCallbackFirstTest End";
}

/**
 * @tc.name: DoInitialFetchDoesNotOverwriteCallbackTest
 * @tc.desc: A fetch result must not overwrite a callback value that arrived during the fetch
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetworkStatusTest, DoInitialFetchDoesNotOverwriteCallbackTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoInitialFetchDoesNotOverwriteCallbackTest Start";
    ResetNetStatusForTest();
    auto blocker = std::make_shared<std::promise<void>>();
    std::shared_future<void> gate = blocker->get_future().share();
    auto entered = std::make_shared<std::promise<void>>();
    auto enteredFut = entered->get_future();
    EXPECT_CALL(*dfsNetConnClient_, GetDefaultNet(_))
        .WillOnce(InvokeWithoutArgs([gate, entered]() {
            entered->set_value();
            gate.wait();
            return NetManagerStandard::NETMANAGER_SUCCESS;
        }));
    auto fetchDone = std::make_shared<std::promise<void>>();
    std::thread([fetchDone]() {
        NetworkStatus::DoInitialFetch();
        fetchDone->set_value();
    }).detach();
    enteredFut.get();  // fetch is in flight: first check passed, callback can now win
    NetworkStatus::SetNetConnStatus(NetworkStatus::WIFI_CONNECT);
    blocker->set_value();
    fetchDone->get_future().get();
    EXPECT_EQ(NetworkStatus::netStatus_, NetworkStatus::WIFI_CONNECT);
    GTEST_LOG_(INFO) << "DoInitialFetchDoesNotOverwriteCallbackTest End";
}
} // namespace OHOS::FileManagement::CloudSync::Test