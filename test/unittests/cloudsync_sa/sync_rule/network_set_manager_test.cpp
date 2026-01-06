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
#include "cloud_file_kit_mock.h"
#include "datashare_helper_mock.h"
#include "datashare_result_set_mock.h"
#include "dfs_error.h"
#include "ipc_skeleton.h"
#include "parameters.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using namespace CloudFile;
using namespace OHOS::DataShare;

class NetworkSetManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    shared_ptr<NetworkSetManager> networkSetManager_ = nullptr;
    shared_ptr<DataShareHelperMock> dataShareHelperMock_ = nullptr;
    shared_ptr<DataShareResultSetMock> resultSetMock_ = nullptr;
};

void NetworkSetManagerTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void NetworkSetManagerTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void NetworkSetManagerTest::SetUp(void)
{
    networkSetManager_ = make_shared<NetworkSetManager>();
    dataShareHelperMock_ = std::make_shared<DataShareHelperMock>();
    resultSetMock_ = std::make_shared<DataShareResultSetMock>();
    DataShareHelperMock::proxy_ = dataShareHelperMock_;
    DataShareResultSetMock::proxy_ = resultSetMock_;
    GTEST_LOG_(INFO) << "SetUp";
}

void NetworkSetManagerTest::TearDown(void)
{
    networkSetManager_.reset();
    dataShareHelperMock_ = nullptr;
    DataShareHelperMock::proxy_ = nullptr;
    resultSetMock_ = nullptr;
    DataShareResultSetMock::proxy_ = nullptr;
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
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(nullptr));

        int32_t ret = networkSetManager_->QueryCellularConnect(userId, bundleName);
        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "QueryCellularConnectTest FAILED";
    }
    GTEST_LOG_(INFO) << "QueryCellularConnectTest End";
}

/**
 * @tc.name: QueryCellularConnectTest002
 * @tc.desc: Verify the QueryCellularConnect function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetworkSetManagerTest, QueryCellularConnectTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryCellularConnectTest002 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.ohos.photos";
        std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(dataShareHelper));
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(nullptr));
        
        int32_t ret = networkSetManager_->QueryCellularConnect(userId, bundleName);
        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "QueryCellularConnectTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "QueryCellularConnectTest002 End";
}

/**
 * @tc.name: QueryCellularConnectTest003
 * @tc.desc: Verify the QueryCellularConnect function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetworkSetManagerTest, QueryCellularConnectTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryCellularConnectTest003 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.ohos.photos";
        std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
        std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(dataShareHelper));
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
        EXPECT_CALL(*resultSetMock_, GoToFirstRow()).WillOnce(Return(E_RDB));
        EXPECT_CALL(*resultSetMock_, Close()).WillOnce(Return(E_OK));
        
        int32_t ret = networkSetManager_->QueryCellularConnect(userId, bundleName);
        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "QueryCellularConnectTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "QueryCellularConnectTest003 End";
}

/**
 * @tc.name: QueryCellularConnectTest004
 * @tc.desc: Verify the QueryCellularConnect function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetworkSetManagerTest, QueryCellularConnectTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryCellularConnectTest004 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.ohos.photos";
        std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
        std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(dataShareHelper));
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
        EXPECT_CALL(*resultSetMock_, GoToFirstRow()).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetColumnIndex(_, _)).WillOnce(Return(E_RDB));
        EXPECT_CALL(*resultSetMock_, Close()).WillOnce(Return(E_OK));
        
        int32_t ret = networkSetManager_->QueryCellularConnect(userId, bundleName);
        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "QueryCellularConnectTest004 FAILED";
    }
    GTEST_LOG_(INFO) << "QueryCellularConnectTest004 End";
}

/**
 * @tc.name: QueryCellularConnectTest005
 * @tc.desc: Verify the QueryCellularConnect function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetworkSetManagerTest, QueryCellularConnectTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryCellularConnectTest005 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.ohos.photos";
        std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
        std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(dataShareHelper));
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
        EXPECT_CALL(*resultSetMock_, GoToFirstRow()).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetLong(_, _)).WillOnce(Return(E_RDB));
        EXPECT_CALL(*resultSetMock_, Close()).WillOnce(Return(E_OK));
        
        int32_t ret = networkSetManager_->QueryCellularConnect(userId, bundleName);
        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "QueryCellularConnectTest005 FAILED";
    }
    GTEST_LOG_(INFO) << "QueryCellularConnectTest005 End";
}

/**
 * @tc.name: QueryCellularConnectTest006
 * @tc.desc: Verify the QueryCellularConnect function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetworkSetManagerTest, QueryCellularConnectTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryCellularConnectTest006 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.ohos.photos";
        std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
        std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(dataShareHelper));
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
        EXPECT_CALL(*resultSetMock_, GoToFirstRow()).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetLong(_, _)).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, Close()).WillOnce(Return(E_OK));
        
        int32_t ret = networkSetManager_->QueryCellularConnect(userId, bundleName);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "QueryCellularConnectTest006 FAILED";
    }
    GTEST_LOG_(INFO) << "QueryCellularConnectTest006 End";
}

/**
 * @tc.name: QueryNetConnectTest001
 * @tc.desc: Verify the QueryNetConnect function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetworkSetManagerTest, QueryNetConnectTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryNetConnectTest001 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.ohos.photos";
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(nullptr));
        
        int32_t ret = networkSetManager_->QueryNetConnect(userId, bundleName);
        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "QueryNetConnectTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "QueryNetConnectTest001 End";
}

/**
 * @tc.name: QueryNetConnectTest002
 * @tc.desc: Verify the QueryNetConnect function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetworkSetManagerTest, QueryNetConnectTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryNetConnectTest002 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.ohos.photos";
        std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(dataShareHelper));
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(nullptr));
        
        int32_t ret = networkSetManager_->QueryNetConnect(userId, bundleName);
        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "QueryNetConnectTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "QueryNetConnectTest002 End";
}

/**
 * @tc.name: QueryNetConnectTest003
 * @tc.desc: Verify the QueryNetConnect function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetworkSetManagerTest, QueryNetConnectTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryNetConnectTest003 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.ohos.photos";
        std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
        std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(dataShareHelper));
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
        EXPECT_CALL(*resultSetMock_, GoToFirstRow()).WillOnce(Return(E_RDB));
        EXPECT_CALL(*resultSetMock_, Close()).WillOnce(Return(E_OK));
        
        int32_t ret = networkSetManager_->QueryNetConnect(userId, bundleName);
        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "QueryNetConnectTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "QueryNetConnectTest003 End";
}

/**
 * @tc.name: QueryNetConnectTest004
 * @tc.desc: Verify the QueryNetConnect function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetworkSetManagerTest, QueryNetConnectTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryNetConnectTest004 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.ohos.photos";
        std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
        std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(dataShareHelper));
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
        EXPECT_CALL(*resultSetMock_, GoToFirstRow()).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetColumnIndex(_, _)).WillOnce(Return(E_RDB));
        EXPECT_CALL(*resultSetMock_, Close()).WillOnce(Return(E_OK));
        
        int32_t ret = networkSetManager_->QueryNetConnect(userId, bundleName);
        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "QueryNetConnectTest004 FAILED";
    }
    GTEST_LOG_(INFO) << "QueryNetConnectTest004 End";
}

/**
 * @tc.name: QueryNetConnectTest005
 * @tc.desc: Verify the QueryNetConnect function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetworkSetManagerTest, QueryNetConnectTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryNetConnectTest005 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.ohos.photos";
        std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
        std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(dataShareHelper));
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
        EXPECT_CALL(*resultSetMock_, GoToFirstRow()).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetString(_, _)).WillOnce(Return(E_RDB));
        EXPECT_CALL(*resultSetMock_, Close()).WillOnce(Return(E_OK));
        
        int32_t ret = networkSetManager_->QueryNetConnect(userId, bundleName);
        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "QueryNetConnectTest005 FAILED";
    }
    GTEST_LOG_(INFO) << "QueryNetConnectTest005 End";
}

/**
 * @tc.name: QueryNetConnectTest006
 * @tc.desc: Verify the QueryNetConnect function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetworkSetManagerTest, QueryNetConnectTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryNetConnectTest006 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.ohos.photos";
        std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
        std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(dataShareHelper));
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
        EXPECT_CALL(*resultSetMock_, GoToFirstRow()).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetString(_, _)).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, Close()).WillOnce(Return(E_OK));
        
        int32_t ret = networkSetManager_->QueryNetConnect(userId, bundleName);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "QueryNetConnectTest006 FAILED";
    }
    GTEST_LOG_(INFO) << "QueryNetConnectTest006 End";
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

/**
 * @tc.name: GetConfigParams001
 * @tc.desc: Verify the GetConfigParams function
 * @tc.type: FUNC
 * @tc.require: ICEGLJ
 */
HWTEST_F(NetworkSetManagerTest, GetConfigParams001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetConfigParams001 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.ohos.photos";
        CloudFile::CloudFileKit::instance_ = nullptr;
        bool ret = networkSetManager_->GetConfigParams(bundleName, userId);
        EXPECT_EQ(ret, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetConfigParams001 FAILED";
    }
    GTEST_LOG_(INFO) << "GetConfigParams001 End";
}

/**
 * @tc.name: GetConfigParams002
 * @tc.desc: Verify the GetConfigParams function
 * @tc.require: ICEGLJ
 */
HWTEST_F(NetworkSetManagerTest, GetConfigParams002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetConfigParams002 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.ohos.photos";
        auto driveKit = new (std::nothrow) CloudFilekitImplMock();
        CloudFile::CloudFileKit::instance_ = driveKit;
        EXPECT_CALL(*driveKit, GetAppConfigParams(_, _, _)).WillOnce(Return(E_RDB));
        bool ret = networkSetManager_->GetConfigParams(bundleName, userId);
        EXPECT_EQ(ret, false);
        delete driveKit;
        driveKit = nullptr;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetConfigParams002 FAILED";
    }
    GTEST_LOG_(INFO) << "GetConfigParams002 End";
}

/**
 * @tc.name: GetConfigParams003
 * @tc.desc: Verify the GetConfigParams function
 * @tc.type: FUNC
 * @tc.require: ICEGLJ
 */
HWTEST_F(NetworkSetManagerTest, GetConfigParams003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetConfigParams003 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.ohos.photos";
        auto driveKit = new (std::nothrow) CloudFilekitImplMock();
        CloudFile::CloudFileKit::instance_ = driveKit;
        std::map<std::string, std::string> param;
        param.insert({"test", "test"});
        EXPECT_CALL(*driveKit, GetAppConfigParams(_, _, _)).WillOnce(DoAll(SetArgReferee<2>(param), Return(E_OK)));
        bool ret = networkSetManager_->GetConfigParams(bundleName, userId);
        EXPECT_EQ(ret, false);
        delete driveKit;
        driveKit = nullptr;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetConfigParams003 FAILED";
    }
    GTEST_LOG_(INFO) << "GetConfigParams003 End";
}

/**
 * @tc.name: GetConfigParams004
 * @tc.desc: Verify the GetConfigParams function
 * @tc.require: ICEGLJ
 */
HWTEST_F(NetworkSetManagerTest, GetConfigParams004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetConfigParams004 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.ohos.photos";
        auto driveKit = new (std::nothrow) CloudFilekitImplMock();
        CloudFile::CloudFileKit::instance_ = driveKit;
        std::map<std::string, std::string> param;
        param.insert({"useMobileNetworkData", "0"});
        EXPECT_CALL(*driveKit, GetAppConfigParams(_, _, _)).WillOnce(DoAll(SetArgReferee<2>(param), Return(E_OK)));
        bool ret = networkSetManager_->GetConfigParams(bundleName, userId);
        EXPECT_EQ(ret, false);
        delete driveKit;
        driveKit = nullptr;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetConfigParams004 FAILED";
    }
    GTEST_LOG_(INFO) << "GetConfigParams004 End";
}

/**
 * @tc.name: GetConfigParams005
 * @tc.desc: Verify the GetConfigParams function
 * @tc.require: ICEGLJ
 */
HWTEST_F(NetworkSetManagerTest, GetConfigParams005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetConfigParams005 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.ohos.photos";
        auto driveKit = new (std::nothrow) CloudFilekitImplMock();
        CloudFile::CloudFileKit::instance_ = driveKit;
        std::map<std::string, std::string> param;
        param.insert({"useMobileNetworkData", "1"});
        EXPECT_CALL(*driveKit, GetAppConfigParams(_, _, _)).WillOnce(DoAll(SetArgReferee<2>(param), Return(E_OK)));
        bool ret = networkSetManager_->GetConfigParams(bundleName, userId);
        EXPECT_EQ(ret, true);
        delete driveKit;
        driveKit = nullptr;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetConfigParams005 FAILED";
    }
    GTEST_LOG_(INFO) << "GetConfigParams005 End";
}

/**
 * @tc.name: IsAllowNetConnect001
 * @tc.desc: Verify the IsAllowNetConnect func
 * @tc.type: FUNC
 * @tc.require: ICEGLJ
 */
HWTEST_F(NetworkSetManagerTest, IsAllowNetConnect001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsAllowNetConnect001 Start";
    try {
        std::string bundleName = "test";
        int32_t userId = 100;
        networkSetManager_->netMap_.Clear();
        networkSetManager_->netMap_.EnsureInsert(std::to_string(userId) + "/" + bundleName, false);
        bool ret = networkSetManager_->IsAllowNetConnect(bundleName, userId);
        EXPECT_EQ(ret, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsAllowNetConnect001 FAILED";
    }
    GTEST_LOG_(INFO) << "IsAllowNetConnect001 End";
}

/**
 * @tc.name: IsAllowNetConnect002
 * @tc.desc: Verify the IsAllowNetConnect func
 * @tc.type: FUNC
 * @tc.require: ICEGLJ
 */
HWTEST_F(NetworkSetManagerTest, IsAllowNetConnect002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsAllowNetConnect002 Start";
    try {
        std::string bundleName = "test";
        int32_t userId = 100;
        networkSetManager_->netMap_.Clear();
        networkSetManager_->netMap_.EnsureInsert(std::to_string(userId) + "/" + bundleName, true);
        bool ret = networkSetManager_->IsAllowNetConnect(bundleName, userId);
        EXPECT_EQ(ret, true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsAllowNetConnect002 FAILED";
    }
    GTEST_LOG_(INFO) << "IsAllowNetConnect002 End";
}

/**
 * @tc.name: NetWorkChangeStopUploadTask001
 * @tc.desc: Verify the NetWorkChangeStopUploadTask func
 * @tc.type: FUNC
 * @tc.require: ICEGLJ
 */
HWTEST_F(NetworkSetManagerTest, NetWorkChangeStopUploadTask001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NetWorkChangeStopUploadTask001 Start";
    try {
        networkSetManager_->cellularNetMap_.Clear();
        networkSetManager_->cellularNetMap_.EnsureInsert("", false);
        networkSetManager_->NetWorkChangeStopUploadTask();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "NetWorkChangeStopUploadTask001 FAILED";
    }
    GTEST_LOG_(INFO) << "NetWorkChangeStopUploadTask001 End";
}

/**
 * @tc.name: NetWorkChangeStopUploadTask002
 * @tc.desc: Verify the NetWorkChangeStopUploadTask func
 * @tc.type: FUNC
 * @tc.require: ICEGLJ
 */
HWTEST_F(NetworkSetManagerTest, NetWorkChangeStopUploadTask002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NetWorkChangeStopUploadTask002 Start";
    try {
        networkSetManager_->cellularNetMap_.Clear();
        networkSetManager_->cellularNetMap_.EnsureInsert("bundle_test", false);
        networkSetManager_->NetWorkChangeStopUploadTask();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "NetWorkChangeStopUploadTask002 FAILED";
    }
    GTEST_LOG_(INFO) << "NetWorkChangeStopUploadTask002 End";
}

/**
 * @tc.name: NetWorkChangeStopUploadTask003
 * @tc.desc: Verify the NetWorkChangeStopUploadTask func
 * @tc.type: FUNC
 * @tc.require: ICEGLJ
 */
HWTEST_F(NetworkSetManagerTest, NetWorkChangeStopUploadTask003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NetWorkChangeStopUploadTask003 Start";
    try {
        networkSetManager_->cellularNetMap_.Clear();
        networkSetManager_->cellularNetMap_.EnsureInsert("com.ohos.photos", true);
        networkSetManager_->NetWorkChangeStopUploadTask();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "NetWorkChangeStopUploadTask003 FAILED";
    }
    GTEST_LOG_(INFO) << "NetWorkChangeStopUploadTask003 End";
}

/**
 * @tc.name: NetWorkChangeStopUploadTask004
 * @tc.desc: Verify the NetWorkChangeStopUploadTask func
 * @tc.type: FUNC
 * @tc.require: ICEGLJ
 */
HWTEST_F(NetworkSetManagerTest, NetWorkChangeStopUploadTask004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NetWorkChangeStopUploadTask004 Start";
    try {
        networkSetManager_->cellularNetMap_.Clear();
        networkSetManager_->cellularNetMap_.EnsureInsert("com.ohos.photos", false);
        networkSetManager_->NetWorkChangeStopUploadTask();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "NetWorkChangeStopUploadTask004 FAILED";
    }
    GTEST_LOG_(INFO) << "NetWorkChangeStopUploadTask004 End";
}

/**
 * @tc.name: NetWorkChangeStopUploadTask005
 * @tc.desc: Verify the NetWorkChangeStopUploadTask func
 * @tc.type: FUNC
 * @tc.require: ICEGLJ
 */
HWTEST_F(NetworkSetManagerTest, NetWorkChangeStopUploadTask005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NetWorkChangeStopUploadTask005 Start";
    try {
        networkSetManager_->cellularNetMap_.Clear();
        networkSetManager_->cellularNetMap_.EnsureInsert("test/com.ohos.photos", false);
        networkSetManager_->NetWorkChangeStopUploadTask();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "NetWorkChangeStopUploadTask005 FAILED";
    }
    GTEST_LOG_(INFO) << "NetWorkChangeStopUploadTask005 End";
}

/**
 * @tc.name: NetWorkChangeStopUploadTask006
 * @tc.desc: Verify the NetWorkChangeStopUploadTask func
 * @tc.type: FUNC
 * @tc.require: ICEGLJ
 */
HWTEST_F(NetworkSetManagerTest, NetWorkChangeStopUploadTask006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NetWorkChangeStopUploadTask006 Start";
    try {
        networkSetManager_->cellularNetMap_.Clear();
        networkSetManager_->cellularNetMap_.EnsureInsert("100/com.ohos.photos", false);
        networkSetManager_->dataSyncManager_ = nullptr;
        networkSetManager_->NetWorkChangeStopUploadTask();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "NetWorkChangeStopUploadTask006 FAILED";
    }
    GTEST_LOG_(INFO) << "NetWorkChangeStopUploadTask006 End";
}

/**
 * @tc.name: NetWorkChangeStopUploadTask007
 * @tc.desc: Verify the NetWorkChangeStopUploadTask func
 * @tc.type: FUNC
 * @tc.require: ICEGLJ
 */
HWTEST_F(NetworkSetManagerTest, NetWorkChangeStopUploadTask007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NetWorkChangeStopUploadTask007 Start";
    try {
        networkSetManager_->cellularNetMap_.Clear();
        networkSetManager_->cellularNetMap_.EnsureInsert("100/com.ohos.photos", false);
        networkSetManager_->dataSyncManager_ = std::make_shared<CloudFile::DataSyncManager>();
        networkSetManager_->NetWorkChangeStopUploadTask();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "NetWorkChangeStopUploadTask007 FAILED";
    }
    GTEST_LOG_(INFO) << "NetWorkChangeStopUploadTask007 End";
}

/**
 * @tc.name: NetWorkChangeStopUploadTask008
 * @tc.desc: Verify the NetWorkChangeStopUploadTask func
 * @tc.type: FUNC
 * @tc.require: ICEGLJ
 */
HWTEST_F(NetworkSetManagerTest, NetWorkChangeStopUploadTask008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NetWorkChangeStopUploadTask008 Start";
    try {
        networkSetManager_->cellularNetMap_.Clear();
        networkSetManager_->NetWorkChangeStopUploadTask();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "NetWorkChangeStopUploadTask008 FAILED";
    }
    GTEST_LOG_(INFO) << "NetWorkChangeStopUploadTask008 End";
}

/**
 * @tc.name: InitNetworkSetManagerTest001
 * @tc.desc: Verify the InitNetworkSetManager function when driveKit is not null.
 * @tc.type: FUNC
 * @tc.require: ICEGLJ
 */
HWTEST_F(NetworkSetManagerTest, InitNetworkSetManager001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InitNetworkSetManager001 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.ohos.photos";
        auto driveKit = new (std::nothrow) CloudFilekitImplMock();
        CloudFile::CloudFileKit::instance_ = driveKit;
        EXPECT_CALL(*driveKit, GetAppConfigParams(_, _, _)).WillRepeatedly(Return(E_OK));
        networkSetManager_->InitNetworkSetManager(bundleName, userId);
        EXPECT_TRUE(true);
        delete driveKit;
        driveKit = nullptr;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "InitNetworkSetManager001 FAILED";
    }
    GTEST_LOG_(INFO) << "InitNetworkSetManager001 End";
}

/**
 * @tc.name: InitNetworkSetManagerTest002
 * @tc.desc: Verify the InitNetworkSetManager function when driveKit is not null.
 * @tc.type: FUNC
 * @tc.require: ICEGLJ
 */
HWTEST_F(NetworkSetManagerTest, InitNetworkSetManager002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InitNetworkSetManager002 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.ohos.ailife";
        networkSetManager_->InitNetworkSetManager(bundleName, userId);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "InitNetworkSetManager002 FAILED";
    }
    GTEST_LOG_(INFO) << "InitNetworkSetManager002 End";
}

/**
 * @tc.name: InitNetworkSetManagerTest003
 * @tc.desc: Verify the InitNetworkSetManager function when driveKit is not null.
 * @tc.type: FUNC
 * @tc.require: ICEGLJ
 */
HWTEST_F(NetworkSetManagerTest, InitNetworkSetManager003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InitNetworkSetManager003 Start";
    try {
        int32_t userId = 100;
        string bundleName = "xxxxxxx";
        networkSetManager_->InitNetworkSetManager(bundleName, userId);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "InitNetworkSetManager003 FAILED";
    }
    GTEST_LOG_(INFO) << "InitNetworkSetManager003 End";
}
}