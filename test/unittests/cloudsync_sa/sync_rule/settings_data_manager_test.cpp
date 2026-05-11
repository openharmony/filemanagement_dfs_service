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

#include <cstdint>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "datashare_helper_mock.h"
#include "datashare_result_set_mock.h"
#include "dfs_error.h"
#include "os_account_manager_mock_ext.h"
#include "service_registry_mock.h"
#include "settings_data_manager.h"

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using namespace OHOS::DataShare;

static const std::string SYNC_SWITCH_KEY = "photos_sync_options";                       // "0", "1", "2"
static const std::string NETWORK_CONNECTION_KEY = "photo_network_connection_status";    // "on", "off"
static const std::string LOCAL_SPACE_FREE_KEY = "photos_local_space_free";              // "1", "0"
static const std::string LOCAL_SPACE_DAYS_KEY = "photos_local_space_free_day_count";    // "30"
static const std::string MOBILE_DATA_SYNC_KEY = "photos_mobile_data_sync";              // "1", "0"
static const std::string USER_SUFFIX = "_user";

class SettingsDataManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<DataShareHelperMock> dataShareHelperMock_ = nullptr;
    static inline std::shared_ptr<DataShareResultSetMock> resultSetMock_ = nullptr;
    static inline std::shared_ptr<OsAccountManagerMethodMockExt> OsAccountMethodMock_ = nullptr;
    static inline std::shared_ptr<ServiceRegistryMock> serviceRegistryMock_ = nullptr;
};

void SettingsDataManagerTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    dataShareHelperMock_ = std::make_shared<DataShareHelperMock>();
    resultSetMock_ = std::make_shared<DataShareResultSetMock>();
    DataShareHelperMock::proxy_ = dataShareHelperMock_;
    DataShareResultSetMock::proxy_ = resultSetMock_;
    OsAccountMethodMock_ = make_shared<OsAccountManagerMethodMockExt>();
    IOsAccountManagerMethod::osMethod_ = OsAccountMethodMock_;
    serviceRegistryMock_ = make_shared<ServiceRegistryMock>();
    ServiceRegistryMock::proxy_ = serviceRegistryMock_;
}

void SettingsDataManagerTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    dataShareHelperMock_ = nullptr;
    DataShareHelperMock::proxy_ = nullptr;
    resultSetMock_ = nullptr;
    DataShareResultSetMock::proxy_ = nullptr;
    OsAccountMethodMock_ = nullptr;
    IOsAccountManagerMethod::osMethod_ = nullptr;
    serviceRegistryMock_ = nullptr;
    ServiceRegistryMock::proxy_ = nullptr;
}

void SettingsDataManagerTest::SetUp(void)
{
    DataShareHelperMock::proxy_ = dataShareHelperMock_;
    DataShareResultSetMock::proxy_ = resultSetMock_;
}

void SettingsDataManagerTest::TearDown(void) {}

HWTEST_F(SettingsDataManagerTest, GetQueryKeyTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetQueryKeyTest001 Start";
    try {
        std::string expect = SYNC_SWITCH_KEY + USER_SUFFIX;
        SettingsDataManager::supportUserSettingsData_ = true;
        SettingsDataManager::currentUserId_ = 100;
        std::string ret = SettingsDataManager::GetQueryKey(SYNC_SWITCH_KEY);
        EXPECT_EQ(ret, expect);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetQueryKeyTest001 Failed";
    }

    GTEST_LOG_(INFO) << "GetQueryKeyTest001 End";
}

HWTEST_F(SettingsDataManagerTest, GetQueryKeyTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetQueryKeyTest002 Start";
    try {
        std::string expect = SYNC_SWITCH_KEY;
        SettingsDataManager::supportUserSettingsData_ = false;
        SettingsDataManager::currentUserId_ = 100;
        std::string ret = SettingsDataManager::GetQueryKey(SYNC_SWITCH_KEY);
        EXPECT_EQ(ret, expect);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetQueryKeyTest002 Failed";
    }

    GTEST_LOG_(INFO) << "GetQueryKeyTest002 End";
}

HWTEST_F(SettingsDataManagerTest, GetQueryKeyTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetQueryKeyTest003 Start";
    try {
        std::string expect = LOCAL_SPACE_DAYS_KEY;
        SettingsDataManager::supportUserSettingsData_ = true;
        SettingsDataManager::currentUserId_ = 100;
        std::string ret = SettingsDataManager::GetQueryKey(LOCAL_SPACE_DAYS_KEY);
        EXPECT_EQ(ret, expect);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetQueryKeyTest003 Failed";
    }

    GTEST_LOG_(INFO) << "GetQueryKeyTest003 End";
}

HWTEST_F(SettingsDataManagerTest, GetQueryKeyTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetQueryKeyTest004 Start";
    try {
        std::string expect = LOCAL_SPACE_DAYS_KEY;
        SettingsDataManager::supportUserSettingsData_ = false;
        SettingsDataManager::currentUserId_ = 100;
        std::string ret = SettingsDataManager::GetQueryKey(LOCAL_SPACE_DAYS_KEY);
        EXPECT_EQ(ret, expect);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetQueryKeyTest004 Failed";
    }

    GTEST_LOG_(INFO) << "GetQueryKeyTest004 End";
}

HWTEST_F(SettingsDataManagerTest, GetSettingsDataCommonUriTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSettingsDataCommonUriTest001 Start";
    try {
        std::string expect = "datashare:///com.ohos.settingsdata/entry/settingsdata/USER_SETTINGSDATA_100?Proxy=true";
        SettingsDataManager::supportUserSettingsData_ = true;
        SettingsDataManager::currentUserId_ = 100;
        std::string ret = SettingsDataManager::GetSettingsDataCommonUri();
        EXPECT_EQ(ret, expect);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetSettingsDataCommonUriTest001 Failed";
    }

    GTEST_LOG_(INFO) << "GetSettingsDataCommonUriTest001 End";
}

HWTEST_F(SettingsDataManagerTest, GetSettingsDataCommonUriTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSettingsDataCommonUriTest002 Start";
    try {
        std::string expect = "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true";
        SettingsDataManager::supportUserSettingsData_ = false;
        SettingsDataManager::currentUserId_ = 100;
        std::string ret = SettingsDataManager::GetSettingsDataCommonUri();
        EXPECT_EQ(ret, expect);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetSettingsDataCommonUriTest002 Failed";
    }

    GTEST_LOG_(INFO) << "GetSettingsDataCommonUriTest002 End";
}

HWTEST_F(SettingsDataManagerTest, GetSettingsDataUriTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSettingsDataUriTest001 Start";
    try {
        std::string expect =
            "datashare:///com.ohos.settingsdata/entry/settingsdata/USER_SETTINGSDATA_100?Proxy=true&key=" +
            SYNC_SWITCH_KEY + USER_SUFFIX;
        SettingsDataManager::supportUserSettingsData_ = true;
        SettingsDataManager::currentUserId_ = 100;
        std::string ret = SettingsDataManager::GetSettingsDataUri(SYNC_SWITCH_KEY);
        EXPECT_EQ(ret, expect);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetSettingsDataUriTest001 Failed";
    }

    GTEST_LOG_(INFO) << "GetSettingsDataUriTest001 End";
}

HWTEST_F(SettingsDataManagerTest, GetSettingsDataUriTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSettingsDataUriTest002 Start";
    try {
        std::string expect = "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true&key=" +
            SYNC_SWITCH_KEY;
        SettingsDataManager::supportUserSettingsData_ = false;
        SettingsDataManager::currentUserId_ = 100;
        std::string ret = SettingsDataManager::GetSettingsDataUri(SYNC_SWITCH_KEY);
        EXPECT_EQ(ret, expect);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetSettingsDataUriTest002 Failed";
    }

    GTEST_LOG_(INFO) << "GetSettingsDataUriTest002 End";
}

HWTEST_F(SettingsDataManagerTest, GetSettingsDataUriTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSettingsDataUriTest003 Start";
    try {
        std::string expect = "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true&key=" +
            LOCAL_SPACE_DAYS_KEY;
        SettingsDataManager::supportUserSettingsData_ = true;
        SettingsDataManager::currentUserId_ = 100;
        std::string ret = SettingsDataManager::GetSettingsDataUri(LOCAL_SPACE_DAYS_KEY);
        EXPECT_EQ(ret, expect);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetSettingsDataUriTest003 Failed";
    }

    GTEST_LOG_(INFO) << "GetSettingsDataUriTest003 End";
}

HWTEST_F(SettingsDataManagerTest, GetSettingsDataUriTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSettingsDataUriTest004 Start";
    try {
        std::string expect = "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true&key=" +
            LOCAL_SPACE_DAYS_KEY;
        SettingsDataManager::supportUserSettingsData_ = false;
        SettingsDataManager::currentUserId_ = 101;
        std::string ret = SettingsDataManager::GetSettingsDataUri(LOCAL_SPACE_DAYS_KEY);
        EXPECT_EQ(ret, expect);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetSettingsDataUriTest004 Failed";
    }

    GTEST_LOG_(INFO) << "GetSettingsDataUriTest004 End";
}

HWTEST_F(SettingsDataManagerTest, GetUserSettingsDataUriTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetUserSettingsDataUriTest001 Start";
    try {
        std::string expect =
            "datashare:///com.ohos.settingsdata/entry/settingsdata/USER_SETTINGSDATA_100?Proxy=true&key=" +
            SYNC_SWITCH_KEY + USER_SUFFIX;
        SettingsDataManager::supportUserSettingsData_ = true;
        SettingsDataManager::currentUserId_ = 100;
        std::string ret = SettingsDataManager::GetUserSettingsDataUri(SYNC_SWITCH_KEY);
        EXPECT_EQ(ret, expect);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetUserSettingsDataUriTest001 Failed";
    }

    GTEST_LOG_(INFO) << "GetUserSettingsDataUriTest001 End";
}

HWTEST_F(SettingsDataManagerTest, QueryParamInSettingsDataTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryParamInSettingsDataTest001 Start";
    try {
        DataShareResultSetMock::proxy_ = nullptr;
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(nullptr));
        std::string key = "";
        std::string value;
        int32_t ret = SettingsDataManager::QueryParamInSettingsData(key, value);
        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "QueryParamInSettingsDataTest001 Failed";
    }

    GTEST_LOG_(INFO) << "QueryParamInSettingsDataTest001 End";
}

HWTEST_F(SettingsDataManagerTest, QueryParamInSettingsDataTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryParamInSettingsDataTest002 Start";
    try {
        DataShareResultSetMock::proxy_ = nullptr;
        std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
        std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(dataShareHelper));
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
        std::string key = "";
        std::string value;
        int32_t ret = SettingsDataManager::QueryParamInSettingsData(key, value);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "QueryParamInSettingsDataTest002 Failed";
    }

    GTEST_LOG_(INFO) << "QueryParamInSettingsDataTest002 End";
}

HWTEST_F(SettingsDataManagerTest, QueryParamInSettingsDataTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryParamInSettingsDataTest003 Start";
    try {
        DataShareResultSetMock::proxy_ = nullptr;
        std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(dataShareHelper));
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(nullptr));
        std::string key = "";
        std::string value;
        int32_t ret = SettingsDataManager::QueryParamInSettingsData(key, value);
        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "QueryParamInSettingsDataTest003 Failed";
    }

    GTEST_LOG_(INFO) << "QueryParamInSettingsDataTest003 End";
}

HWTEST_F(SettingsDataManagerTest, QueryParamInSettingsDataTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryParamInSettingsDataTest004 Start";
    try {
        std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
        std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(dataShareHelper));
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
        EXPECT_CALL(*resultSetMock_, GoToFirstRow()).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetString(_, _)).WillOnce(Return(E_OK));
        std::string key = "";
        std::string value;
        int32_t ret = SettingsDataManager::QueryParamInSettingsData(key, value);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "QueryParamInSettingsDataTest004 Failed";
    }

    GTEST_LOG_(INFO) << "QueryParamInSettingsDataTest004 End";
}

HWTEST_F(SettingsDataManagerTest, QueryParamInSettingsDataTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryParamInSettingsDataTest005 Start";
    try {
        std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
        std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(dataShareHelper));
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
        EXPECT_CALL(*resultSetMock_, GoToFirstRow()).WillOnce(Return(-1));
        EXPECT_CALL(*resultSetMock_, GetColumnIndex(_, _)).Times(0);
        EXPECT_CALL(*resultSetMock_, GetString(_, _)).Times(0);
        std::string key = "";
        std::string value;
        int32_t ret = SettingsDataManager::QueryParamInSettingsData(key, value);
        EXPECT_EQ(ret, E_INNER_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "QueryParamInSettingsDataTest005 Failed";
    }

    GTEST_LOG_(INFO) << "QueryParamInSettingsDataTest005 End";
}

HWTEST_F(SettingsDataManagerTest, QueryParamInSettingsDataTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryParamInSettingsDataTest006 Start";
    try {
        std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
        std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(dataShareHelper));
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
        EXPECT_CALL(*resultSetMock_, GoToFirstRow()).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetColumnIndex(_, _)).WillOnce(Return(-1));
        EXPECT_CALL(*resultSetMock_, GetString(_, _)).Times(0);
        std::string key = "";
        std::string value;
        int32_t ret = SettingsDataManager::QueryParamInSettingsData(key, value);
        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "QueryParamInSettingsDataTest006 Failed";
    }

    GTEST_LOG_(INFO) << "QueryParamInSettingsDataTest006 End";
}

HWTEST_F(SettingsDataManagerTest, QueryParamInSettingsDataTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryParamInSettingsDataTest007 Start";
    try {
        std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
        std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(dataShareHelper));
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
        EXPECT_CALL(*resultSetMock_, GoToFirstRow()).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetString(_, _)).WillOnce(Return(-1));
        std::string key = "";
        std::string value;
        int32_t ret = SettingsDataManager::QueryParamInSettingsData(key, value);
        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "QueryParamInSettingsDataTest007 Failed";
    }

    GTEST_LOG_(INFO) << "QueryParamInSettingsDataTest007 End";
}

HWTEST_F(SettingsDataManagerTest, QueryParamInUserSettingsDataTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryParamInUserSettingsDataTest001 Start";
    try {
        DataShareResultSetMock::proxy_ = nullptr;
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(nullptr));
        std::string key = "";
        std::string value;
        int32_t ret = SettingsDataManager::QueryParamInUserSettingsData(key, value);
        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "QueryParamInUserSettingsDataTest001 Failed";
    }

    GTEST_LOG_(INFO) << "QueryParamInUserSettingsDataTest001 End";
}

HWTEST_F(SettingsDataManagerTest, QueryParamInUserSettingsDataTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryParamInUserSettingsDataTest002 Start";
    try {
        DataShareResultSetMock::proxy_ = nullptr;
        std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
        std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(dataShareHelper));
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
        std::string key = "";
        std::string value;
        int32_t ret = SettingsDataManager::QueryParamInUserSettingsData(key, value);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "QueryParamInUserSettingsDataTest002 Failed";
    }

    GTEST_LOG_(INFO) << "QueryParamInUserSettingsDataTest002 End";
}

HWTEST_F(SettingsDataManagerTest, QueryParamInUserSettingsDataTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryParamInUserSettingsDataTest003 Start";
    try {
        DataShareResultSetMock::proxy_ = nullptr;
        std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(dataShareHelper));
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(nullptr));
        std::string key = "";
        std::string value;
        int32_t ret = SettingsDataManager::QueryParamInUserSettingsData(key, value);
        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "QueryParamInUserSettingsDataTest003 Failed";
    }

    GTEST_LOG_(INFO) << "QueryParamInUserSettingsDataTest003 End";
}

HWTEST_F(SettingsDataManagerTest, QueryParamInUserSettingsDataTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryParamInUserSettingsDataTest004 Start";
    try {
        std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
        std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(dataShareHelper));
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
        EXPECT_CALL(*resultSetMock_, GoToFirstRow()).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetString(_, _)).WillOnce(Return(E_OK));
        std::string key = "";
        std::string value;
        int32_t ret = SettingsDataManager::QueryParamInUserSettingsData(key, value);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "QueryParamInUserSettingsDataTest004 Failed";
    }

    GTEST_LOG_(INFO) << "QueryParamInUserSettingsDataTest004 End";
}

HWTEST_F(SettingsDataManagerTest, QueryParamInUserSettingsDataTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryParamInUserSettingsDataTest005 Start";
    try {
        std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
        std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(dataShareHelper));
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
        EXPECT_CALL(*resultSetMock_, GoToFirstRow()).WillOnce(Return(-1));
        EXPECT_CALL(*resultSetMock_, GetColumnIndex(_, _)).Times(0);
        EXPECT_CALL(*resultSetMock_, GetString(_, _)).Times(0);
        std::string key = "";
        std::string value;
        int32_t ret = SettingsDataManager::QueryParamInUserSettingsData(key, value);
        EXPECT_EQ(ret, E_INNER_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "QueryParamInUserSettingsDataTest005 Failed";
    }

    GTEST_LOG_(INFO) << "QueryParamInUserSettingsDataTest005 End";
}

HWTEST_F(SettingsDataManagerTest, QueryParamInUserSettingsDataTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryParamInUserSettingsDataTest006 Start";
    try {
        std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
        std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(dataShareHelper));
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
        EXPECT_CALL(*resultSetMock_, GoToFirstRow()).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetColumnIndex(_, _)).WillOnce(Return(-1));
        EXPECT_CALL(*resultSetMock_, GetString(_, _)).Times(0);
        std::string key = "";
        std::string value;
        int32_t ret = SettingsDataManager::QueryParamInUserSettingsData(key, value);
        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "QueryParamInUserSettingsDataTest006 Failed";
    }

    GTEST_LOG_(INFO) << "QueryParamInUserSettingsDataTest006 End";
}

HWTEST_F(SettingsDataManagerTest, QueryParamInUserSettingsDataTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryParamInUserSettingsDataTest007 Start";
    try {
        std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
        std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(dataShareHelper));
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
        EXPECT_CALL(*resultSetMock_, GoToFirstRow()).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetString(_, _)).WillOnce(Return(-1));
        std::string key = "";
        std::string value;
        int32_t ret = SettingsDataManager::QueryParamInUserSettingsData(key, value);
        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "QueryParamInUserSettingsDataTest007 Failed";
    }

    GTEST_LOG_(INFO) << "QueryParamInUserSettingsDataTest007 End";
}

HWTEST_F(SettingsDataManagerTest, InitUserSettingsTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InitUserSettingsTest001 Start";
    try {
        EXPECT_CALL(*serviceRegistryMock_, GetSystemAbilityManager()).WillOnce(Return(nullptr));
        int32_t ret = SettingsDataManager::InitUserSettings();
        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "InitUserSettingsTest001 Failed";
    }

    GTEST_LOG_(INFO) << "InitUserSettingsTest001 End";
}

HWTEST_F(SettingsDataManagerTest, InitUserSettingsTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InitUserSettingsTest002 Start";
    try {
        sptr<IfSystemAbilityManagerMock> systemAbilityManagerMock = sptr(new IfSystemAbilityManagerMock);
        EXPECT_CALL(*serviceRegistryMock_, GetSystemAbilityManager()).WillOnce(Return(systemAbilityManagerMock));
        EXPECT_CALL(*systemAbilityManagerMock, CheckSystemAbility(_)).WillOnce(Return(nullptr));
        int32_t ret = SettingsDataManager::InitUserSettings();
        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "InitUserSettingsTest002 Failed";
    }

    GTEST_LOG_(INFO) << "InitUserSettingsTest002 End";
}

HWTEST_F(SettingsDataManagerTest, InitUserSettingsTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InitUserSettingsTest003 Start";
    try {
        sptr<IfSystemAbilityManagerMock> systemAbilityManagerMock = sptr(new IfSystemAbilityManagerMock);
        sptr<IRemoteObjectMock> remoteObjectMock = sptr(new IRemoteObjectMock);
        EXPECT_CALL(*serviceRegistryMock_, GetSystemAbilityManager()).WillOnce(Return(systemAbilityManagerMock));
        EXPECT_CALL(*systemAbilityManagerMock, CheckSystemAbility(_)).WillOnce(Return(remoteObjectMock));
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _, _, _)).WillOnce(Return(nullptr));
        int32_t ret = SettingsDataManager::InitUserSettings();
        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "InitUserSettingsTest003 Failed";
    }

    GTEST_LOG_(INFO) << "InitUserSettingsTest003 End";
}

HWTEST_F(SettingsDataManagerTest, InitUserSettingsTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InitUserSettingsTest004 Start";
    try {
        sptr<IfSystemAbilityManagerMock> systemAbilityManagerMock = sptr(new IfSystemAbilityManagerMock);
        sptr<IRemoteObjectMock> remoteObjectMock = sptr(new IRemoteObjectMock);
        std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
        EXPECT_CALL(*serviceRegistryMock_, GetSystemAbilityManager()).WillOnce(Return(systemAbilityManagerMock));
        EXPECT_CALL(*systemAbilityManagerMock, CheckSystemAbility(_)).WillOnce(Return(remoteObjectMock));
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _, _, _)).WillOnce(Return(dataShareHelper));
        EXPECT_CALL(*dataShareHelperMock_, UpdateEx(_, _, _)).WillOnce(Return(std::make_pair(E_OK, 1)));
        int32_t ret = SettingsDataManager::InitUserSettings();
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "InitUserSettingsTest004 Failed";
    }

    GTEST_LOG_(INFO) << "InitUserSettingsTest004 End";
}

HWTEST_F(SettingsDataManagerTest, InitUserSettingsTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InitUserSettingsTest005 Start";
    try {
        sptr<IfSystemAbilityManagerMock> systemAbilityManagerMock = sptr(new IfSystemAbilityManagerMock);
        sptr<IRemoteObjectMock> remoteObjectMock = sptr(new IRemoteObjectMock);
        std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
        EXPECT_CALL(*serviceRegistryMock_, GetSystemAbilityManager()).WillOnce(Return(systemAbilityManagerMock));
        EXPECT_CALL(*systemAbilityManagerMock, CheckSystemAbility(_)).WillOnce(Return(remoteObjectMock));
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _, _, _)).WillOnce(Return(dataShareHelper));
        EXPECT_CALL(*dataShareHelperMock_, UpdateEx(_, _, _)).WillOnce(Return(std::make_pair(10, 1)));
        int32_t ret = SettingsDataManager::InitUserSettings();
        EXPECT_EQ(ret, 10);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "InitUserSettingsTest005 Failed";
    }

    GTEST_LOG_(INFO) << "InitUserSettingsTest005 End";
}

HWTEST_F(SettingsDataManagerTest, InitAndQuerySettingsDataTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InitAndQuerySettingsDataTest001 Start";
    try {
        EXPECT_CALL(*serviceRegistryMock_, GetSystemAbilityManager()).WillOnce(Return(nullptr));

        std::string key = "";
        std::string value;
        int32_t ret = SettingsDataManager::InitAndQuerySettingsData(key, value, false);
        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "InitAndQuerySettingsDataTest001 Failed";
    }

    GTEST_LOG_(INFO) << "InitAndQuerySettingsDataTest001 End";
}

HWTEST_F(SettingsDataManagerTest, InitAndQuerySettingsDataTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InitAndQuerySettingsDataTest002 Start";
    try {
        std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
        sptr<IfSystemAbilityManagerMock> systemAbilityManagerMock = sptr(new IfSystemAbilityManagerMock);
        sptr<IRemoteObjectMock> remoteObjectMock = sptr(new IRemoteObjectMock);
        EXPECT_CALL(*serviceRegistryMock_, GetSystemAbilityManager()).WillOnce(Return(systemAbilityManagerMock));
        EXPECT_CALL(*systemAbilityManagerMock, CheckSystemAbility(_)).WillOnce(Return(remoteObjectMock));
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _, _, _)).WillOnce(Return(dataShareHelper));
        EXPECT_CALL(*dataShareHelperMock_, UpdateEx(_, _, _)).WillOnce(Return(std::make_pair(E_OK, 1)));
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(nullptr));

        std::string key = "";
        std::string value;
        int32_t ret = SettingsDataManager::InitAndQuerySettingsData(key, value, false);
        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "InitAndQuerySettingsDataTest002 Failed";
    }

    GTEST_LOG_(INFO) << "InitAndQuerySettingsDataTest002 End";
}

HWTEST_F(SettingsDataManagerTest, InitAndQuerySettingsDataTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InitAndQuerySettingsDataTest003 Start";
    try {
        std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
        std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
        sptr<IfSystemAbilityManagerMock> systemAbilityManagerMock = sptr(new IfSystemAbilityManagerMock);
        sptr<IRemoteObjectMock> remoteObjectMock = sptr(new IRemoteObjectMock);
        EXPECT_CALL(*serviceRegistryMock_, GetSystemAbilityManager()).WillOnce(Return(systemAbilityManagerMock));
        EXPECT_CALL(*systemAbilityManagerMock, CheckSystemAbility(_)).WillOnce(Return(remoteObjectMock));
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _, _, _)).WillOnce(Return(dataShareHelper));
        EXPECT_CALL(*dataShareHelperMock_, UpdateEx(_, _, _)).WillOnce(Return(std::make_pair(E_OK, 1)));
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(dataShareHelper));
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
        EXPECT_CALL(*resultSetMock_, GoToFirstRow()).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetString(_, _)).WillOnce(Return(E_OK));

        std::string key = "";
        std::string value;
        int32_t ret = SettingsDataManager::InitAndQuerySettingsData(key, value, false);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "InitAndQuerySettingsDataTest003 Failed";
    }

    GTEST_LOG_(INFO) << "InitAndQuerySettingsDataTest003 End";
}

HWTEST_F(SettingsDataManagerTest, InitAndQuerySettingsDataTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InitAndQuerySettingsDataTest004 Start";
    try {
        std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
        std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(dataShareHelper));
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
        EXPECT_CALL(*resultSetMock_, GoToFirstRow()).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetString(_, _)).WillOnce(Return(E_OK));

        std::string key = "";
        std::string value;
        int32_t ret = SettingsDataManager::InitAndQuerySettingsData(key, value, true);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "InitAndQuerySettingsDataTest004 Failed";
    }

    GTEST_LOG_(INFO) << "InitAndQuerySettingsDataTest004 End";
}

HWTEST_F(SettingsDataManagerTest, InitAndQuerySettingsDataTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InitAndQuerySettingsDataTest005 Start";
    try {
        std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
        std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(dataShareHelper));
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
        EXPECT_CALL(*resultSetMock_, GoToFirstRow()).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetString(_, _)).WillOnce(Return(-1));
        EXPECT_CALL(*serviceRegistryMock_, GetSystemAbilityManager()).WillOnce(Return(nullptr));

        std::string key = "";
        std::string value;
        int32_t ret = SettingsDataManager::InitAndQuerySettingsData(key, value, true);
        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "InitAndQuerySettingsDataTest005 Failed";
    }

    GTEST_LOG_(INFO) << "InitAndQuerySettingsDataTest005 End";
}

HWTEST_F(SettingsDataManagerTest, InitAndQuerySettingsDataTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InitAndQuerySettingsDataTest006 Start";
    try {
        std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
        std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
        sptr<IfSystemAbilityManagerMock> systemAbilityManagerMock = sptr(new IfSystemAbilityManagerMock);
        sptr<IRemoteObjectMock> remoteObjectMock = sptr(new IRemoteObjectMock);
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(nullptr)).WillOnce(Return(nullptr));
        EXPECT_CALL(*serviceRegistryMock_, GetSystemAbilityManager()).WillOnce(Return(systemAbilityManagerMock));
        EXPECT_CALL(*systemAbilityManagerMock, CheckSystemAbility(_)).WillOnce(Return(remoteObjectMock));
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _, _, _)).WillOnce(Return(dataShareHelper));
        EXPECT_CALL(*dataShareHelperMock_, UpdateEx(_, _, _)).WillOnce(Return(std::make_pair(E_OK, 1)));

        std::string key = "";
        std::string value;
        int32_t ret = SettingsDataManager::InitAndQuerySettingsData(key, value, true);
        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "InitAndQuerySettingsDataTest006 Failed";
    }

    GTEST_LOG_(INFO) << "InitAndQuerySettingsDataTest006 End";
}

HWTEST_F(SettingsDataManagerTest, InitAndQuerySettingsDataTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InitAndQuerySettingsDataTest007 Start";
    try {
        std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
        std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
        sptr<IfSystemAbilityManagerMock> systemAbilityManagerMock = sptr(new IfSystemAbilityManagerMock);
        sptr<IRemoteObjectMock> remoteObjectMock = sptr(new IRemoteObjectMock);
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(nullptr))
            .WillOnce(Return(dataShareHelper));
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
        EXPECT_CALL(*resultSetMock_, GoToFirstRow()).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetString(_, _)).WillOnce(Return(E_OK));
        EXPECT_CALL(*serviceRegistryMock_, GetSystemAbilityManager()).WillOnce(Return(systemAbilityManagerMock));
        EXPECT_CALL(*systemAbilityManagerMock, CheckSystemAbility(_)).WillOnce(Return(remoteObjectMock));
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _, _, _)).WillOnce(Return(dataShareHelper));
        EXPECT_CALL(*dataShareHelperMock_, UpdateEx(_, _, _)).WillOnce(Return(std::make_pair(E_OK, 1)));

        std::string key = "";
        std::string value;
        int32_t ret = SettingsDataManager::InitAndQuerySettingsData(key, value, true);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "InitAndQuerySettingsDataTest007 Failed";
    }

    GTEST_LOG_(INFO) << "InitAndQuerySettingsDataTest007 End";
}

HWTEST_F(SettingsDataManagerTest, UpdateIsSupportUserSettingsDataTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateIsSupportUserSettingsDataTest001 Start";
    try {
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(nullptr);
        EXPECT_CALL(*serviceRegistryMock_, GetSystemAbilityManager()).WillOnce(Return(nullptr));

        SettingsDataManager::UpdateIsSupportUserSettingsData();
        EXPECT_EQ(SettingsDataManager::supportUserSettingsData_, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UpdateIsSupportUserSettingsDataTest001 Failed";
    }

    GTEST_LOG_(INFO) << "UpdateIsSupportUserSettingsDataTest001 End";
}

HWTEST_F(SettingsDataManagerTest, UpdateIsSupportUserSettingsDataTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateIsSupportUserSettingsDataTest002 Start";
    try {
        std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
        sptr<IfSystemAbilityManagerMock> systemAbilityManagerMock = sptr(new IfSystemAbilityManagerMock);
        sptr<IRemoteObjectMock> remoteObjectMock = sptr(new IRemoteObjectMock);
        EXPECT_CALL(*serviceRegistryMock_, GetSystemAbilityManager()).WillOnce(Return(systemAbilityManagerMock));
        EXPECT_CALL(*systemAbilityManagerMock, CheckSystemAbility(_)).WillOnce(Return(remoteObjectMock));
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _, _, _)).WillOnce(Return(dataShareHelper));
        EXPECT_CALL(*dataShareHelperMock_, UpdateEx(_, _, _)).WillOnce(Return(std::make_pair(E_OK, 1)));
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(nullptr).WillOnce(nullptr);

        SettingsDataManager::UpdateIsSupportUserSettingsData();
        EXPECT_EQ(SettingsDataManager::supportUserSettingsData_, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UpdateIsSupportUserSettingsDataTest002 Failed";
    }

    GTEST_LOG_(INFO) << "UpdateIsSupportUserSettingsDataTest002 End";
}

HWTEST_F(SettingsDataManagerTest, UpdateIsSupportUserSettingsDataTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateIsSupportUserSettingsDataTest003 Start";
    try {
        std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
        std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
        sptr<IfSystemAbilityManagerMock> systemAbilityManagerMock = sptr(new IfSystemAbilityManagerMock);
        sptr<IRemoteObjectMock> remoteObjectMock = sptr(new IRemoteObjectMock);
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(dataShareHelper));
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
        EXPECT_CALL(*resultSetMock_, GoToFirstRow()).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetString(_, _)).WillOnce(Return(E_OK));

        std::string key = "";
        std::string value;
        SettingsDataManager::UpdateIsSupportUserSettingsData();
        EXPECT_EQ(SettingsDataManager::supportUserSettingsData_, true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UpdateIsSupportUserSettingsDataTest003 Failed";
    }

    GTEST_LOG_(INFO) << "UpdateIsSupportUserSettingsDataTest003 End";
}

HWTEST_F(SettingsDataManagerTest, UpdateIsSupportUserSettingsDataTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateIsSupportUserSettingsDataTest004 Start";
    try {
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(nullptr));

        std::string key = "";
        std::string value;
        SettingsDataManager::UpdateIsSupportUserSettingsData(true);
        EXPECT_EQ(SettingsDataManager::supportUserSettingsData_, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UpdateIsSupportUserSettingsDataTest004 Failed";
    }

    GTEST_LOG_(INFO) << "UpdateIsSupportUserSettingsDataTest004 End";
}

HWTEST_F(SettingsDataManagerTest, GetSwitchStatusByCacheTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSwitchStatusByCacheTest001 Start";
    try {
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(nullptr));
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).Times(0);
        SettingsDataManager::supportUserSettingsData_ = false;
        SwitchStatus status = SettingsDataManager::GetSwitchStatusByCache();
        EXPECT_EQ(status, SwitchStatus::NONE);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetSwitchStatusByCacheTest001 Failed";
    }

    GTEST_LOG_(INFO) << "GetSwitchStatusByCacheTest001 End";
}

HWTEST_F(SettingsDataManagerTest, GetSwitchStatusByCacheTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSwitchStatusByCacheTest002 Start";
    try {
        std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
        std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(dataShareHelper));
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
        EXPECT_CALL(*resultSetMock_, GoToFirstRow()).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetColumnIndex(_, _)).WillOnce(Return(-1));
        EXPECT_CALL(*resultSetMock_, GetString(_, _)).Times(0);
        SettingsDataManager::supportUserSettingsData_ = false;
        SwitchStatus status = SettingsDataManager::GetSwitchStatusByCache();
        EXPECT_EQ(status, SwitchStatus::NONE);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetSwitchStatusByCacheTest002 Failed";
    }

    GTEST_LOG_(INFO) << "GetSwitchStatusByCacheTest002 End";
}

HWTEST_F(SettingsDataManagerTest, GetSwitchStatusByCacheTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSwitchStatusByCacheTest003 Start";
    try {
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).Times(0);
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).Times(0);
        SettingsDataManager::settingsDataMap_.EnsureInsert(SYNC_SWITCH_KEY, "0");
        SettingsDataManager::supportUserSettingsData_ = false;
        SwitchStatus status = SettingsDataManager::GetSwitchStatusByCache();
        EXPECT_EQ(status, SwitchStatus::NONE);

        SettingsDataManager::settingsDataMap_.Clear();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetSwitchStatusByCacheTest003 Failed";
    }

    GTEST_LOG_(INFO) << "GetSwitchStatusByCacheTest003 End";
}

HWTEST_F(SettingsDataManagerTest, GetSwitchStatusByCacheTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSwitchStatusByCacheTest004 Start";
    try {
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).Times(0);
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).Times(0);
        SettingsDataManager::settingsDataMap_.EnsureInsert(SYNC_SWITCH_KEY, "1");
        SettingsDataManager::supportUserSettingsData_ = false;
        SwitchStatus status = SettingsDataManager::GetSwitchStatusByCache();
        EXPECT_EQ(status, SwitchStatus::CLOUD_SPACE);

        SettingsDataManager::settingsDataMap_.Clear();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetSwitchStatusByCacheTest004 Failed";
    }

    GTEST_LOG_(INFO) << "GetSwitchStatusByCacheTest004 End";
}

HWTEST_F(SettingsDataManagerTest, GetSwitchStatusByCacheTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSwitchStatusByCacheTest005 Start";
    try {
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).Times(0);
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).Times(0);
        SettingsDataManager::settingsDataMap_.EnsureInsert(SYNC_SWITCH_KEY, "2");
        SettingsDataManager::supportUserSettingsData_ = false;
        SwitchStatus status = SettingsDataManager::GetSwitchStatusByCache();
        EXPECT_EQ(status, SwitchStatus::AI_FAMILY);

        SettingsDataManager::settingsDataMap_.Clear();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetSwitchStatusByCacheTest005 Failed";
    }

    GTEST_LOG_(INFO) << "GetSwitchStatusByCacheTest005 End";
}

HWTEST_F(SettingsDataManagerTest, GetSwitchStatusByCacheTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSwitchStatusByCacheTest006 Start";
    try {
        std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
        std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(dataShareHelper));
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
        EXPECT_CALL(*resultSetMock_, GoToFirstRow()).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetColumnIndex(_, _)).WillOnce(Return(-1));
        EXPECT_CALL(*resultSetMock_, GetString(_, _)).Times(0);
        SettingsDataManager::supportUserSettingsData_ = false;
        SwitchStatus status = SettingsDataManager::GetSwitchStatusByCache();
        EXPECT_EQ(status, SwitchStatus::NONE);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetSwitchStatusByCacheTest006 Failed";
    }

    GTEST_LOG_(INFO) << "GetSwitchStatusByCacheTest006 End";
}

HWTEST_F(SettingsDataManagerTest, QuerySwitchStatusTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QuerySwitchStatusTest001 Start";
    try {
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(nullptr));
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).Times(0);
        SettingsDataManager::supportUserSettingsData_ = false;
        std::string value;
        int32_t ret = SettingsDataManager::QuerySwitchStatus(value);
        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "QuerySwitchStatusTest001 Failed";
    }

    GTEST_LOG_(INFO) << "QuerySwitchStatusTest001 End";
}

HWTEST_F(SettingsDataManagerTest, QuerySwitchStatusTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QuerySwitchStatusTest002 Start";
    try {
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(nullptr));
        EXPECT_CALL(*serviceRegistryMock_, GetSystemAbilityManager()).WillOnce(Return(nullptr));
        SettingsDataManager::supportUserSettingsData_ = true;
        std::string value;
        int32_t ret = SettingsDataManager::QuerySwitchStatus(value);
        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "QuerySwitchStatusTest002 Failed";
    }

    GTEST_LOG_(INFO) << "QuerySwitchStatusTest002 End";
}

HWTEST_F(SettingsDataManagerTest, QuerySwitchStatusTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QuerySwitchStatusTest003 Start";
    try {
        std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
        std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
        sptr<IfSystemAbilityManagerMock> systemAbilityManagerMock = sptr(new IfSystemAbilityManagerMock);
        sptr<IRemoteObjectMock> remoteObjectMock = sptr(new IRemoteObjectMock);
        EXPECT_CALL(*serviceRegistryMock_, GetSystemAbilityManager()).WillOnce(Return(systemAbilityManagerMock));
        EXPECT_CALL(*systemAbilityManagerMock, CheckSystemAbility(_)).WillOnce(Return(remoteObjectMock));
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _, _, _)).WillOnce(Return(dataShareHelper));
        EXPECT_CALL(*dataShareHelperMock_, UpdateEx(_, _, _)).WillOnce(Return(std::make_pair(E_OK, 1)));
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(nullptr))
            .WillOnce(Return(dataShareHelper));
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
        EXPECT_CALL(*resultSetMock_, GoToFirstRow()).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetString(_, _)).WillOnce(Return(E_OK));
        SettingsDataManager::supportUserSettingsData_ = true;
        std::string value;
        int32_t ret = SettingsDataManager::QuerySwitchStatus(value);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "QuerySwitchStatusTest003 Failed";
    }

    GTEST_LOG_(INFO) << "QuerySwitchStatusTest003 End";
}

HWTEST_F(SettingsDataManagerTest, GetSwitchStatusTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSwitchStatusTest001 Start";
    try {
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(nullptr));
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).Times(0);
        SettingsDataManager::supportUserSettingsData_ = false;
        SwitchStatus status = SettingsDataManager::GetSwitchStatus();
        EXPECT_EQ(status, SwitchStatus::NONE);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetSwitchStatusTest001 Failed";
    }

    GTEST_LOG_(INFO) << "GetSwitchStatusTest001 End";
}

HWTEST_F(SettingsDataManagerTest, GetSwitchStatusTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSwitchStatusTest002 Start";
    try {
        std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
        std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(dataShareHelper));
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
        EXPECT_CALL(*resultSetMock_, GoToFirstRow()).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetColumnIndex(_, _)).WillOnce(Return(-1));
        EXPECT_CALL(*resultSetMock_, GetString(_, _)).Times(0);
        SettingsDataManager::supportUserSettingsData_ = false;
        SwitchStatus status = SettingsDataManager::GetSwitchStatus();
        EXPECT_EQ(status, SwitchStatus::NONE);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetSwitchStatusTest002 Failed";
    }

    GTEST_LOG_(INFO) << "GetSwitchStatusTest002 End";
}

HWTEST_F(SettingsDataManagerTest, GetSwitchStatusTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSwitchStatusTest003 Start";
    try {
        std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
        std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(dataShareHelper));
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
        EXPECT_CALL(*resultSetMock_, GoToFirstRow()).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetString(_, _)).WillOnce(Return(E_OK));
        SettingsDataManager::settingsDataMap_.EnsureInsert(SYNC_SWITCH_KEY, "2");
        SettingsDataManager::supportUserSettingsData_ = false;
        SwitchStatus status = SettingsDataManager::GetSwitchStatus();
        EXPECT_EQ(status, SwitchStatus::NONE);

        SettingsDataManager::settingsDataMap_.Clear();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetSwitchStatusTest003 Failed";
    }

    GTEST_LOG_(INFO) << "GetSwitchStatusTest003 End";
}

HWTEST_F(SettingsDataManagerTest, GetSwitchStatusTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSwitchStatusTest004 Start";
    try {
        std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
        std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(dataShareHelper));
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
        EXPECT_CALL(*resultSetMock_, GoToFirstRow()).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetString(_, _)).WillOnce(Return(E_OK));
        SettingsDataManager::settingsDataMap_.EnsureInsert(SYNC_SWITCH_KEY, "1");
        SettingsDataManager::supportUserSettingsData_ = false;
        SwitchStatus status = SettingsDataManager::GetSwitchStatus();
        EXPECT_EQ(status, SwitchStatus::NONE);

        SettingsDataManager::settingsDataMap_.Clear();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetSwitchStatusTest004 Failed";
    }

    GTEST_LOG_(INFO) << "GetSwitchStatusTest004 End";
}

HWTEST_F(SettingsDataManagerTest, GetSwitchStatusTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSwitchStatusTest005 Start";
    try {
        std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
        std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(dataShareHelper));
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
        EXPECT_CALL(*resultSetMock_, GoToFirstRow()).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetString(_, _)).WillOnce(Return(E_OK));
        SettingsDataManager::settingsDataMap_.EnsureInsert(SYNC_SWITCH_KEY, "0");
        SettingsDataManager::supportUserSettingsData_ = false;
        SwitchStatus status = SettingsDataManager::GetSwitchStatus();
        EXPECT_EQ(status, SwitchStatus::NONE);

        SettingsDataManager::settingsDataMap_.Clear();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetSwitchStatusTest005 Failed";
    }

    GTEST_LOG_(INFO) << "GetSwitchStatusTest005 End";
}

HWTEST_F(SettingsDataManagerTest, GetSwitchStatusTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSwitchStatusTest006 Start";
    try {
        std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
        std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(dataShareHelper));
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
        EXPECT_CALL(*resultSetMock_, GoToFirstRow()).WillOnce(Return(-1));
        EXPECT_CALL(*resultSetMock_, GetColumnIndex(_, _)).Times(0);
        EXPECT_CALL(*resultSetMock_, GetString(_, _)).Times(0);
        SettingsDataManager::settingsDataMap_.EnsureInsert(SYNC_SWITCH_KEY, "1");
        SettingsDataManager::supportUserSettingsData_ = false;
        SwitchStatus status = SettingsDataManager::GetSwitchStatus();
        EXPECT_EQ(status, SwitchStatus::CLOUD_SPACE);

        SettingsDataManager::settingsDataMap_.Clear();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetSwitchStatusTest006 Failed";
    }

    GTEST_LOG_(INFO) << "GetSwitchStatusTest006 End";
}

HWTEST_F(SettingsDataManagerTest, GetNetworkConnectionStatusTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetNetworkConnectionStatusTest001 Start";
    try {
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(nullptr));
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).Times(0);
        bool status = SettingsDataManager::GetNetworkConnectionStatus();
        EXPECT_EQ(status, true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetNetworkConnectionStatusTest001 Failed";
    }

    GTEST_LOG_(INFO) << "GetNetworkConnectionStatusTest001 End";
}

HWTEST_F(SettingsDataManagerTest, GetNetworkConnectionStatusTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetNetworkConnectionStatusTest002 Start";
    try {
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).Times(0);
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).Times(0);
        SettingsDataManager::settingsDataMap_.EnsureInsert(NETWORK_CONNECTION_KEY, "off");
        bool status = SettingsDataManager::GetNetworkConnectionStatus();
        EXPECT_EQ(status, false);

        SettingsDataManager::settingsDataMap_.Clear();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetNetworkConnectionStatusTest002 Failed";
    }

    GTEST_LOG_(INFO) << "GetNetworkConnectionStatusTest002 End";
}

HWTEST_F(SettingsDataManagerTest, GetNetworkConnectionStatusTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetNetworkConnectionStatusTest003 Start";
    try {
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).Times(0);
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).Times(0);
        SettingsDataManager::settingsDataMap_.EnsureInsert(NETWORK_CONNECTION_KEY, "on");
        bool status = SettingsDataManager::GetNetworkConnectionStatus();
        EXPECT_EQ(status, true);

        SettingsDataManager::settingsDataMap_.Clear();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetNetworkConnectionStatusTest003 Failed";
    }

    GTEST_LOG_(INFO) << "GetNetworkConnectionStatusTest003 End";
}

HWTEST_F(SettingsDataManagerTest, GetLocalSpaceFreeStatusTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetLocalSpaceFreeStatusTest001 Start";
    try {
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(nullptr));
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).Times(0);
        int32_t status = SettingsDataManager::GetLocalSpaceFreeStatus();
        EXPECT_EQ(status, 1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetLocalSpaceFreeStatusTest001 Failed";
    }
    GTEST_LOG_(INFO) << "GetLocalSpaceFreeStatusTest001 End";
}

HWTEST_F(SettingsDataManagerTest, GetLocalSpaceFreeStatusTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetLocalSpaceFreeStatusTest002 Start";
    try {
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).Times(0);
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).Times(0);
        SettingsDataManager::settingsDataMap_.EnsureInsert(LOCAL_SPACE_FREE_KEY, "1");
        int32_t status = SettingsDataManager::GetLocalSpaceFreeStatus();
        EXPECT_EQ(status, 1);
        SettingsDataManager::settingsDataMap_.Clear();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetLocalSpaceFreeStatusTest002 Failed";
    }
    GTEST_LOG_(INFO) << "GetLocalSpaceFreeStatusTest002 End";
}

HWTEST_F(SettingsDataManagerTest, GetLocalSpaceFreeStatusTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetLocalSpaceFreeStatusTest003 Start";
    try {
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).Times(0);
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).Times(0);
        SettingsDataManager::settingsDataMap_.EnsureInsert(LOCAL_SPACE_FREE_KEY, "0");
        int32_t status = SettingsDataManager::GetLocalSpaceFreeStatus();
        EXPECT_EQ(status, 0);
        SettingsDataManager::settingsDataMap_.Clear();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetLocalSpaceFreeStatusTest003 Failed";
    }
    GTEST_LOG_(INFO) << "GetLocalSpaceFreeStatusTest003 End";
}

HWTEST_F(SettingsDataManagerTest, GetLocalSpaceFreeDaysTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetLocalSpaceFreeDaysTest001 Start";
    try {
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(nullptr));
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).Times(0);
        int32_t status = SettingsDataManager::GetLocalSpaceFreeDays();
        EXPECT_EQ(status, 30);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetLocalSpaceFreeDaysTest001 Failed";
    }
    GTEST_LOG_(INFO) << "GetLocalSpaceFreeDaysTest001 End";
}

HWTEST_F(SettingsDataManagerTest, GetLocalSpaceFreeDaysTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetLocalSpaceFreeDaysTest002 Start";
    try {
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).Times(0);
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).Times(0);
        SettingsDataManager::settingsDataMap_.EnsureInsert(LOCAL_SPACE_DAYS_KEY, "10");
        int32_t status = SettingsDataManager::GetLocalSpaceFreeDays();
        EXPECT_EQ(status, 10);
        SettingsDataManager::settingsDataMap_.Clear();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetLocalSpaceFreeDaysTest002 Failed";
    }
    GTEST_LOG_(INFO) << "GetLocalSpaceFreeDaysTest002 End";
}

HWTEST_F(SettingsDataManagerTest, GetLocalSpaceFreeDaysTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetLocalSpaceFreeDaysTest003 Start";
    try {
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).Times(0);
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).Times(0);
        SettingsDataManager::settingsDataMap_.EnsureInsert(LOCAL_SPACE_DAYS_KEY, "");
        int32_t status = SettingsDataManager::GetLocalSpaceFreeDays();
        EXPECT_EQ(status, 30);
        SettingsDataManager::settingsDataMap_.Clear();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetLocalSpaceFreeDaysTest003 Failed";
    }
    GTEST_LOG_(INFO) << "GetLocalSpaceFreeDaysTest003 End";
}

HWTEST_F(SettingsDataManagerTest, GetLocalSpaceFreeDaysTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetLocalSpaceFreeDaysTest004 Start";
    try {
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).Times(0);
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).Times(0);
        SettingsDataManager::settingsDataMap_.EnsureInsert(LOCAL_SPACE_DAYS_KEY, "xx");
        int32_t status = SettingsDataManager::GetLocalSpaceFreeDays();
        EXPECT_EQ(status, 30);
        SettingsDataManager::settingsDataMap_.Clear();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetLocalSpaceFreeDaysTest004 Failed";
    }
    GTEST_LOG_(INFO) << "GetLocalSpaceFreeDaysTest004 End";
}

HWTEST_F(SettingsDataManagerTest, GetMobileDataStatusTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetMobileDataStatusTest001 Start";
    try {
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(nullptr));
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).Times(0);
        bool status = SettingsDataManager::GetMobileDataStatus();
        EXPECT_EQ(status, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetMobileDataStatusTest001 Failed";
    }

    GTEST_LOG_(INFO) << "GetMobileDataStatusTest001 End";
}

HWTEST_F(SettingsDataManagerTest, GetMobileDataStatusTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetMobileDataStatusTest002 Start";
    try {
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).Times(0);
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).Times(0);
        SettingsDataManager::settingsDataMap_.EnsureInsert(MOBILE_DATA_SYNC_KEY, "0");
        bool status = SettingsDataManager::GetMobileDataStatus();
        EXPECT_EQ(status, false);

        SettingsDataManager::settingsDataMap_.Clear();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetMobileDataStatusTest002 Failed";
    }
    GTEST_LOG_(INFO) << "GetMobileDataStatusTest002 end";
}

HWTEST_F(SettingsDataManagerTest, GetMobileDataStatusTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetMobileDataStatusTest003 Start";
    try {
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).Times(0);
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).Times(0);
        SettingsDataManager::settingsDataMap_.EnsureInsert(MOBILE_DATA_SYNC_KEY, "1");
        bool status = SettingsDataManager::GetMobileDataStatus();
        EXPECT_EQ(status, true);

        SettingsDataManager::settingsDataMap_.Clear();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetMobileDataStatusTest003 Failed";
    }
    GTEST_LOG_(INFO) << "GetMobileDataStatusTest003 end";
}

HWTEST_F(SettingsDataManagerTest, RegisterObserverTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterObserverTest001 Start";
    try {
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(nullptr));
        SettingsDataManager::observerMap_.Clear();
        SettingsDataManager::RegisterObserver("");
        EXPECT_TRUE(SettingsDataManager::observerMap_.IsEmpty());
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RegisterObserverTest001 Failed";
    }

    GTEST_LOG_(INFO) << "RegisterObserverTest001 End";
}

HWTEST_F(SettingsDataManagerTest, RegisterObserverTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterObserverTest002 Start";
    try {
        std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(dataShareHelper));
        SettingsDataManager::observerMap_.Clear();
        SettingsDataManager::RegisterObserver("");
        EXPECT_TRUE(!SettingsDataManager::observerMap_.IsEmpty());
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RegisterObserverTest002 Failed";
    }

    GTEST_LOG_(INFO) << "RegisterObserverTest002 End";
}

HWTEST_F(SettingsDataManagerTest, RegisterObserverTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterObserverTest003 Start";
    try {
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).Times(0);
        SettingsDataManager::RegisterObserver("", nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RegisterObserverTest003 Failed";
    }

    GTEST_LOG_(INFO) << "RegisterObserverTest003 End";
}

HWTEST_F(SettingsDataManagerTest, RegisterObserverTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterObserverTest004 Start";
    try {
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(nullptr));
        sptr<SettingsDataObserver> dataObserver(new (std::nothrow) SettingsDataObserver("key"));
        SettingsDataManager::RegisterObserver("", dataObserver);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RegisterObserverTest004 Failed";
    }

    GTEST_LOG_(INFO) << "RegisterObserverTest004 End";
}

HWTEST_F(SettingsDataManagerTest, RegisterObserverTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterObserverTest005 Start";
    try {
        std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(dataShareHelper));
        sptr<SettingsDataObserver> dataObserver(new (std::nothrow) SettingsDataObserver("key"));
        SettingsDataManager::RegisterObserver("", dataObserver);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RegisterObserverTest005 Failed";
    }

    GTEST_LOG_(INFO) << "RegisterObserverTest005 End";
}

HWTEST_F(SettingsDataManagerTest, UnregisterDemonObserverTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterDemonObserverTest001 Start";
    try {
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).Times(0);
        std::string key = "key1";
        SettingsDataManager::UnregisterDemonObserver(key, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UnregisterDemonObserverTest001 Failed";
    }

    GTEST_LOG_(INFO) << "UnregisterDemonObserverTest001 End";
}

HWTEST_F(SettingsDataManagerTest, UnregisterDemonObserverTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterDemonObserverTest002 Start";
    try {
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(nullptr));
        std::string key = "key1";
        sptr<SettingsDataObserver> dataObserver(new (std::nothrow) SettingsDataObserver(key));
        SettingsDataManager::UnregisterDemonObserver(key, dataObserver);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UnregisterDemonObserverTest002 Failed";
    }

    GTEST_LOG_(INFO) << "UnregisterDemonObserverTest002 End";
}

HWTEST_F(SettingsDataManagerTest, UnregisterDemonObserverTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterDemonObserverTest003 Start";
    try {
        std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(dataShareHelper));
        std::string key = "key1";
        sptr<SettingsDataObserver> dataObserver(new (std::nothrow) SettingsDataObserver(key));
        SettingsDataManager::UnregisterDemonObserver(key, dataObserver);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UnregisterDemonObserverTest003 Failed";
    }

    GTEST_LOG_(INFO) << "UnregisterDemonObserverTest003 End";
}

HWTEST_F(SettingsDataManagerTest, UnregisterObserverTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterObserverTest001 Start";
    try {
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(nullptr));
        std::string key = "key1";
        sptr<SettingsDataObserver> dataObserver(new (std::nothrow) SettingsDataObserver(key));
        SettingsDataManager::observerMap_.EnsureInsert(key, dataObserver);
        SettingsDataManager::UnregisterObserver(key);
        EXPECT_TRUE(true);

        SettingsDataManager::observerMap_.Clear();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UnregisterObserverTest001 Failed";
    }

    GTEST_LOG_(INFO) << "UnregisterObserverTest001 End";
}

HWTEST_F(SettingsDataManagerTest, UnregisterObserverTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterObserverTest002 Start";
    try {
        std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(dataShareHelper));
        std::string key = "key1";
        sptr<SettingsDataObserver> dataObserver(new (std::nothrow) SettingsDataObserver(key));
        SettingsDataManager::observerMap_.EnsureInsert(key, dataObserver);
        SettingsDataManager::UnregisterObserver(key);
        EXPECT_TRUE(true);

        SettingsDataManager::observerMap_.Clear();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UnregisterObserverTest002 Failed";
    }

    GTEST_LOG_(INFO) << "UnregisterObserverTest002 End";
}

HWTEST_F(SettingsDataManagerTest, UnregisterObserverTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterObserverTest003 Start";
    try {
        std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).Times(0);
        SettingsDataManager::UnregisterObserver("key1");
        EXPECT_TRUE(true);

        SettingsDataManager::observerMap_.Clear();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UnregisterObserverTest003 Failed";
    }

    GTEST_LOG_(INFO) << "UnregisterObserverTest003 End";
}

HWTEST_F(SettingsDataManagerTest, QueryTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryTest001 Start";
    try {
        DataShareHelperMock::proxy_ = nullptr;
        DataShareResultSetMock::proxy_ = nullptr;
        int32_t ret = -1;
        int32_t size = -1;
        std::string value;
        ret = SettingsDataManager::QuerySwitchStatus(value);
        EXPECT_EQ(ret, E_OK);
        size = SettingsDataManager::settingsDataMap_.Size();
        EXPECT_EQ(size, 1);

        ret = SettingsDataManager::QueryNetworkConnectionStatus(value);
        EXPECT_EQ(ret, E_OK);
        size = SettingsDataManager::settingsDataMap_.Size();
        EXPECT_EQ(size, 2);

        ret = SettingsDataManager::QueryLocalSpaceFreeStatus(value);
        EXPECT_EQ(ret, E_OK);
        size = SettingsDataManager::settingsDataMap_.Size();
        EXPECT_EQ(size, 3);

        ret = SettingsDataManager::QueryLocalSpaceFreeDays(value);
        EXPECT_EQ(ret, E_OK);
        size = SettingsDataManager::settingsDataMap_.Size();
        EXPECT_EQ(size, 4);

        ret = SettingsDataManager::QueryMobileDataStatus(value);
        EXPECT_EQ(ret, E_OK);
        size = SettingsDataManager::settingsDataMap_.Size();
        EXPECT_EQ(size, 5);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "QueryTest001 Failed";
    }

    GTEST_LOG_(INFO) << "QueryTest001 End";
}

HWTEST_F(SettingsDataManagerTest, QueryTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryTest002 Start";
    try {
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillRepeatedly(Return(nullptr));
        SettingsDataManager::settingsDataMap_.Clear();
        int32_t ret = -1;
        int32_t size = -1;
        std::string value;

        ret = SettingsDataManager::QueryNetworkConnectionStatus(value);
        EXPECT_EQ(ret, E_RDB);
        size = SettingsDataManager::settingsDataMap_.Size();
        EXPECT_EQ(size, 0);

        ret = SettingsDataManager::QueryLocalSpaceFreeStatus(value);
        EXPECT_EQ(ret, E_RDB);
        size = SettingsDataManager::settingsDataMap_.Size();
        EXPECT_EQ(size, 0);

        ret = SettingsDataManager::QueryLocalSpaceFreeDays(value);
        EXPECT_EQ(ret, E_RDB);
        size = SettingsDataManager::settingsDataMap_.Size();
        EXPECT_EQ(size, 0);

        ret = SettingsDataManager::QueryMobileDataStatus(value);
        EXPECT_EQ(ret, E_RDB);
        size = SettingsDataManager::settingsDataMap_.Size();
        EXPECT_EQ(size, 0);

        SettingsDataManager::settingsDataMap_.Clear();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "QueryTest002 Failed";
    }

    GTEST_LOG_(INFO) << "QueryTest002 End";
}

HWTEST_F(SettingsDataManagerTest, OnChangeTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnChangeTest001 Start";
    try {
        DataShareHelperMock::proxy_ = nullptr;
        DataShareResultSetMock::proxy_ = nullptr;

        SettingsDataObserver observer1(SYNC_SWITCH_KEY);
        observer1.OnChange();

        SettingsDataObserver observer2(NETWORK_CONNECTION_KEY);
        observer2.OnChange();

        SettingsDataObserver observer3(LOCAL_SPACE_FREE_KEY);
        observer3.OnChange();

        SettingsDataObserver observer4(LOCAL_SPACE_DAYS_KEY);
        observer4.OnChange();

        SettingsDataObserver observer5(MOBILE_DATA_SYNC_KEY);
        observer5.OnChange();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnChangeTest001 Failed";
    }

    GTEST_LOG_(INFO) << "OnChangeTest001 End";
}

HWTEST_F(SettingsDataManagerTest, UpdateCurrentUserIdTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateCurrentUserIdTest001 Start";
    try {
        EXPECT_CALL(*OsAccountMethodMock_, GetForegroundOsAccountLocalId(_))
            .WillOnce(DoAll(SetArgReferee<0>(100), Return(E_OK)));
        SettingsDataManager::currentUserId_ = 10;
        bool ret = SettingsDataManager::UpdateCurrentUserId();
        EXPECT_EQ(SettingsDataManager::currentUserId_, 100);
        EXPECT_EQ(ret, true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UpdateCurrentUserIdTest001 Failed";
    }

    GTEST_LOG_(INFO) << "UpdateCurrentUserIdTest001 End";
}

HWTEST_F(SettingsDataManagerTest, UpdateCurrentUserIdTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateCurrentUserIdTest002 Start";
    try {
        EXPECT_CALL(*OsAccountMethodMock_, GetForegroundOsAccountLocalId(_))
            .WillOnce(DoAll(SetArgReferee<0>(100), Return(-1)));
        SettingsDataManager::currentUserId_ = 10;
        bool ret = SettingsDataManager::UpdateCurrentUserId();
        EXPECT_EQ(SettingsDataManager::currentUserId_, 10);
        EXPECT_EQ(ret, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UpdateCurrentUserIdTest002 Failed";
    }

    GTEST_LOG_(INFO) << "UpdateCurrentUserIdTest002 End";
}

HWTEST_F(SettingsDataManagerTest, OnUserSwitchedTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnUserSwitchedTest001 Start";
    try {
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillRepeatedly(Return(nullptr));
        EXPECT_CALL(*serviceRegistryMock_, GetSystemAbilityManager()).WillOnce(Return(nullptr));
        SettingsDataManager::currentUserId_ = 10;
        SettingsDataManager::supportUserSettingsData_ = true;
        SettingsDataManager::OnUserSwitched(100);
        EXPECT_EQ(SettingsDataManager::currentUserId_, 100);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnUserSwitchedTest001 Failed";
    }

    GTEST_LOG_(INFO) << "OnUserSwitchedTest001 End";
}

HWTEST_F(SettingsDataManagerTest, OnUserSwitchedTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnUserSwitchedTest002 Start";
    try {
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).Times(0);
        SettingsDataManager::currentUserId_ = 10;
        SettingsDataManager::supportUserSettingsData_ = false;
        SettingsDataManager::OnUserSwitched(100);
        EXPECT_EQ(SettingsDataManager::currentUserId_, 10);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnUserSwitchedTest002 Failed";
    }

    GTEST_LOG_(INFO) << "OnUserSwitchedTest002 End";
}

HWTEST_F(SettingsDataManagerTest, InitSettingsDataManagerTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InitSettingsDataManagerTest001 Start";
    try {
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillRepeatedly(Return(nullptr));
        EXPECT_CALL(*OsAccountMethodMock_, GetForegroundOsAccountLocalId(_))
            .WillOnce(DoAll(SetArgReferee<0>(100), Return(E_OK)));
        EXPECT_CALL(*serviceRegistryMock_, GetSystemAbilityManager()).WillOnce(Return(nullptr));
        SettingsDataManager::currentUserId_ = 10;
        SettingsDataManager::supportUserSettingsData_ = true;
        SettingsDataManager::InitSettingsDataManager();
        EXPECT_EQ(SettingsDataManager::currentUserId_, 100);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "InitSettingsDataManagerTest001 Failed";
    }

    GTEST_LOG_(INFO) << "InitSettingsDataManagerTest001 End";
}
}