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
#include <cstdint>
#include <unistd.h>

#include "network_set_manager.h"
#include "data_ability_observer_stub.h"
#include "accesstoken_kit.h"
#include "cloud_file_kit.h"
#include "datashare_errno.h"
#include "datashare_result_set.h"
#include "dfs_error.h"
#include "ipc_skeleton.h"
#include "parameters.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using namespace CloudFile;

class NetworkSetManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<NetworkSetManager> networkSetManager_ = nullptr;
};

void NetworkSetManagerTest::SetUpTestCase(void)
{
    networkSetManager_ = make_shared<NetworkSetManager>();
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void NetworkSetManagerTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void NetworkSetManagerTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void NetworkSetManagerTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: QueryCellularConnectTest001
 * @tc.desc: Verify the QueryCellularConnect function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetworkSetManagerTest, QueryCellularConnectTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryCellularConnectTest Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.ohos.photos";

        int32_t ret = networkSetManager_->QueryCellularConnect(userId, bundleName);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "QueryCellularConnectTest FAILED";
    }
    GTEST_LOG_(INFO) << "QueryCellularConnectTest End";
}

/**
 * @tc.name: QueryNetConnectTest001
 * @tc.desc: Verify the QueryNetConnect function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetworkSetManagerTest, QueryNetConnectTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryNetConnectTest Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.ohos.photos";
        
        int32_t ret = networkSetManager_->QueryNetConnect(userId, bundleName);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "QueryNetConnectTest FAILED";
    }
    GTEST_LOG_(INFO) << "QueryNetConnectTest End";
}

/**
 * @tc.name: GetNetConnectTest001
 * @tc.desc: Verify the GetNetConnect function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetworkSetManagerTest, GetNetConnectTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetNetConnectTest Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.ohos.photos";

        networkSetManager_->GetNetConnect(bundleName, userId);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetNetConnectTest FAILED";
    }
    GTEST_LOG_(INFO) << "GetNetConnectTest End";
}

/**
 * @tc.name: UnregisterObserverTest001
 * @tc.desc: Verify the UnregisterObserver function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetworkSetManagerTest, UnregisterObserverTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterObserverTest Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.ohos.photos";
        int32_t type = NetworkSetManager::CELLULARCONNECT;

        networkSetManager_->UnregisterObserver(bundleName, userId, type);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UnregisterObserverTest FAILED";
    }
    GTEST_LOG_(INFO) << "UnregisterObserverTest End";
}

/**
 * @tc.name: UnregisterObserverTest002
 * @tc.desc: Verify the UnregisterObserver function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetworkSetManagerTest, UnregisterObserverTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterObserverTest Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.ohos.photos";
        int32_t type = NetworkSetManager::NETCONNECT;

        networkSetManager_->UnregisterObserver(bundleName, userId, type);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UnregisterObserverTest FAILED";
    }
    GTEST_LOG_(INFO) << "UnregisterObserverTest End";
}

/**
 * @tc.name: RegisterObserverTest001
 * @tc.desc: Verify the RegisterObserver function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetworkSetManagerTest, RegisterObserverTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterObserverTest Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.ohos.photos";
        int32_t type = NetworkSetManager::CELLULARCONNECT;

        networkSetManager_->RegisterObserver(bundleName, userId, type);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RegisterObserverTest FAILED";
    }
    GTEST_LOG_(INFO) << "RegisterObserverTest End";
}

/**
 * @tc.name: GetCellularConnectTest
 * @tc.desc: Verify the GetCellularConnect function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetworkSetManagerTest, GetCellularConnectTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCellularConnectTest Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.ohos.photos";

        (networkSetManager_->cellularNetMap_).Insert(std::to_string(userId) + "/" + bundleName, true);
        networkSetManager_->dataSyncManager_ = std::make_shared<CloudFile::DataSyncManager>();
        networkSetManager_->GetCellularConnect(bundleName, userId);
        EXPECT_NE(networkSetManager_->netStatus_, NetworkSetManager::NetConnStatus::WIFI_CONNECT);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetCellularConnectTest FAILED";
    }
    GTEST_LOG_(INFO) << "GetCellularConnectTest End";
}

/**
 * @tc.name: OnChangeTest001
 * @tc.desc: Verify the OnChange function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetworkSetManagerTest, OnChangeTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnChangeTest Start";
    try {
        std::string bundleName;
        int32_t userId = 0;
        int32_t type = 0;
        MobileNetworkObserver mobile(bundleName, userId, type);
        mobile.observerCallback_ = [] () {};
        mobile.type_ = 0;

        mobile.OnChange();
        EXPECT_NE(mobile.observerCallback_, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnChange FAILED";
    }
    GTEST_LOG_(INFO) << "OnChange End";
}

/**
 * @tc.name: OnChangeTest002
 * @tc.desc: Verify the OnChange function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetworkSetManagerTest, OnChangeTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnChangeTest Start";
    try {
        std::string bundleName;
        int32_t userId = 0;
        int32_t type = -1;
        MobileNetworkObserver mobile(bundleName, userId, type);

        mobile.OnChange();
        EXPECT_EQ(mobile.observerCallback_, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnChange FAILED";
    }
    GTEST_LOG_(INFO) << "OnChange End";
}
}