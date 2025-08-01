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

class SettingsDataManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<DataShareHelperMock> dataShareHelperMock_ = nullptr;
    static inline shared_ptr<DataShareResultSetMock> resultSetMock_ = nullptr;
};

void SettingsDataManagerTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    dataShareHelperMock_ = std::make_shared<DataShareHelperMock>();
    resultSetMock_ = std::make_shared<DataShareResultSetMock>();
    DataShareHelperMock::proxy_ = dataShareHelperMock_;
    DataShareResultSetMock::proxy_ = resultSetMock_;
}

void SettingsDataManagerTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    dataShareHelperMock_ = nullptr;
    DataShareHelperMock::proxy_ = nullptr;
    resultSetMock_ = nullptr;
    DataShareResultSetMock::proxy_ = nullptr;
}

void SettingsDataManagerTest::SetUp(void)
{
    DataShareHelperMock::proxy_ = dataShareHelperMock_;
    DataShareResultSetMock::proxy_ = resultSetMock_;
}

void SettingsDataManagerTest::TearDown(void) {}

HWTEST_F(SettingsDataManagerTest, QueryParamInSettingsDataTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryParamInSettingsDataTest001 Start";
    DataShareResultSetMock::proxy_ = nullptr;
    EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(nullptr));
    std::string key = "";
    std::string value;
    int32_t ret = SettingsDataManager::QueryParamInSettingsData(key, value);
    EXPECT_EQ(ret, E_RDB);

    GTEST_LOG_(INFO) << "QueryParamInSettingsDataTest001 End";
}

HWTEST_F(SettingsDataManagerTest, QueryParamInSettingsDataTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryParamInSettingsDataTest002 Start";
    DataShareResultSetMock::proxy_ = nullptr;
    std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
    std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
    EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(dataShareHelper));
    EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
    std::string key = "";
    std::string value;
    int32_t ret = SettingsDataManager::QueryParamInSettingsData(key, value);
    EXPECT_EQ(ret, E_OK);

    GTEST_LOG_(INFO) << "QueryParamInSettingsDataTest002 End";
}

HWTEST_F(SettingsDataManagerTest, QueryParamInSettingsDataTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryParamInSettingsDataTest003 Start";
    DataShareResultSetMock::proxy_ = nullptr;
    std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
    EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(dataShareHelper));
    EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(nullptr));
    std::string key = "";
    std::string value;
    int32_t ret = SettingsDataManager::QueryParamInSettingsData(key, value);
    EXPECT_EQ(ret, E_RDB);

    GTEST_LOG_(INFO) << "QueryParamInSettingsDataTest003 End";
}

HWTEST_F(SettingsDataManagerTest, QueryParamInSettingsDataTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryParamInSettingsDataTest004 Start";
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

    GTEST_LOG_(INFO) << "QueryParamInSettingsDataTest004 End";
}

HWTEST_F(SettingsDataManagerTest, QueryParamInSettingsDataTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryParamInSettingsDataTest005 Start";
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

    GTEST_LOG_(INFO) << "QueryParamInSettingsDataTest005 End";
}

HWTEST_F(SettingsDataManagerTest, QueryParamInSettingsDataTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryParamInSettingsDataTest006 Start";
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

    GTEST_LOG_(INFO) << "QueryParamInSettingsDataTest006 End";
}

HWTEST_F(SettingsDataManagerTest, QueryParamInSettingsDataTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryParamInSettingsDataTest007 Start";
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

    GTEST_LOG_(INFO) << "QueryParamInSettingsDataTest007 End";
}

HWTEST_F(SettingsDataManagerTest, GetSwitchStatusByCacheTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSwitchStatusByCacheTest001 Start";
    EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(nullptr));
    EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).Times(0);
    SwitchStatus status = SettingsDataManager::GetSwitchStatusByCache();
    EXPECT_EQ(status, SwitchStatus::NONE);

    GTEST_LOG_(INFO) << "GetSwitchStatusByCacheTest001 End";
}

HWTEST_F(SettingsDataManagerTest, GetSwitchStatusByCacheTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSwitchStatusByCacheTest002 Start";
    std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
    std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
    EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(dataShareHelper));
    EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
    EXPECT_CALL(*resultSetMock_, GoToFirstRow()).WillOnce(Return(-1));
    EXPECT_CALL(*resultSetMock_, GetColumnIndex(_, _)).Times(0);
    EXPECT_CALL(*resultSetMock_, GetString(_, _)).Times(0);
    SwitchStatus status = SettingsDataManager::GetSwitchStatusByCache();
    EXPECT_EQ(status, SwitchStatus::CLOUD_SPACE);

    GTEST_LOG_(INFO) << "GetSwitchStatusByCacheTest002 End";
}

HWTEST_F(SettingsDataManagerTest, GetSwitchStatusByCacheTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSwitchStatusByCacheTest003 Start";
    EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).Times(0);
    EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).Times(0);
    SettingsDataManager::settingsDataMap_.EnsureInsert(SYNC_SWITCH_KEY, "0");
    SwitchStatus status = SettingsDataManager::GetSwitchStatusByCache();
    EXPECT_EQ(status, SwitchStatus::NONE);

    SettingsDataManager::settingsDataMap_.Clear();
    GTEST_LOG_(INFO) << "GetSwitchStatusByCacheTest003 End";
}

HWTEST_F(SettingsDataManagerTest, GetSwitchStatusByCacheTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSwitchStatusByCacheTest004 Start";
    EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).Times(0);
    EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).Times(0);
    SettingsDataManager::settingsDataMap_.EnsureInsert(SYNC_SWITCH_KEY, "1");
    SwitchStatus status = SettingsDataManager::GetSwitchStatusByCache();
    EXPECT_EQ(status, SwitchStatus::CLOUD_SPACE);

    SettingsDataManager::settingsDataMap_.Clear();
    GTEST_LOG_(INFO) << "GetSwitchStatusByCacheTest004 End";
}

HWTEST_F(SettingsDataManagerTest, GetSwitchStatusByCacheTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSwitchStatusByCacheTest005 Start";
    EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).Times(0);
    EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).Times(0);
    SettingsDataManager::settingsDataMap_.EnsureInsert(SYNC_SWITCH_KEY, "2");
    SwitchStatus status = SettingsDataManager::GetSwitchStatusByCache();
    EXPECT_EQ(status, SwitchStatus::AI_FAMILY);

    SettingsDataManager::settingsDataMap_.Clear();
    GTEST_LOG_(INFO) << "GetSwitchStatusByCacheTest005 End";
}

HWTEST_F(SettingsDataManagerTest, GetSwitchStatusByCacheTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSwitchStatusByCacheTest006 Start";
    std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
    std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
    EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(dataShareHelper));
    EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
    EXPECT_CALL(*resultSetMock_, GoToFirstRow()).WillOnce(Return(E_OK));
    EXPECT_CALL(*resultSetMock_, GetColumnIndex(_, _)).WillOnce(Return(-1));
    EXPECT_CALL(*resultSetMock_, GetString(_, _)).Times(0);
    SwitchStatus status = SettingsDataManager::GetSwitchStatusByCache();
    EXPECT_EQ(status, SwitchStatus::NONE);

    GTEST_LOG_(INFO) << "GetSwitchStatusByCacheTest006 End";
}

HWTEST_F(SettingsDataManagerTest, GetSwitchStatusTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSwitchStatusTest001 Start";
    EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(nullptr));
    EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).Times(0);
    SwitchStatus status = SettingsDataManager::GetSwitchStatus();
    EXPECT_EQ(status, SwitchStatus::NONE);

    GTEST_LOG_(INFO) << "GetSwitchStatusTest001 End";
}

HWTEST_F(SettingsDataManagerTest, GetSwitchStatusTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSwitchStatusTest002 Start";
    std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
    std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
    EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(dataShareHelper));
    EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
    EXPECT_CALL(*resultSetMock_, GoToFirstRow()).WillOnce(Return(-1));
    EXPECT_CALL(*resultSetMock_, GetColumnIndex(_, _)).Times(0);
    EXPECT_CALL(*resultSetMock_, GetString(_, _)).Times(0);
    SwitchStatus status = SettingsDataManager::GetSwitchStatus();
    EXPECT_EQ(status, SwitchStatus::CLOUD_SPACE);

    GTEST_LOG_(INFO) << "GetSwitchStatusTest002 End";
}

HWTEST_F(SettingsDataManagerTest, GetSwitchStatusTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSwitchStatusTest003 Start";
    std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
    std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
    EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(dataShareHelper));
    EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
    EXPECT_CALL(*resultSetMock_, GoToFirstRow()).WillOnce(Return(E_OK));
    EXPECT_CALL(*resultSetMock_, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*resultSetMock_, GetString(_, _)).WillOnce(Return(E_OK));
    SettingsDataManager::settingsDataMap_.EnsureInsert(SYNC_SWITCH_KEY, "2");
    SwitchStatus status = SettingsDataManager::GetSwitchStatus();
    EXPECT_EQ(status, SwitchStatus::NONE);

    SettingsDataManager::settingsDataMap_.Clear();
    GTEST_LOG_(INFO) << "GetSwitchStatusTest003 End";
}

HWTEST_F(SettingsDataManagerTest, GetSwitchStatusTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSwitchStatusTest004 Start";
    std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
    std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
    EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(dataShareHelper));
    EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
    EXPECT_CALL(*resultSetMock_, GoToFirstRow()).WillOnce(Return(E_OK));
    EXPECT_CALL(*resultSetMock_, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*resultSetMock_, GetString(_, _)).WillOnce(Return(E_OK));
    SettingsDataManager::settingsDataMap_.EnsureInsert(SYNC_SWITCH_KEY, "1");
    SwitchStatus status = SettingsDataManager::GetSwitchStatus();
    EXPECT_EQ(status, SwitchStatus::NONE);

    SettingsDataManager::settingsDataMap_.Clear();
    GTEST_LOG_(INFO) << "GetSwitchStatusTest004 End";
}

HWTEST_F(SettingsDataManagerTest, GetSwitchStatusTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSwitchStatusTest005 Start";
    std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
    std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
    EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(dataShareHelper));
    EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
    EXPECT_CALL(*resultSetMock_, GoToFirstRow()).WillOnce(Return(E_OK));
    EXPECT_CALL(*resultSetMock_, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*resultSetMock_, GetString(_, _)).WillOnce(Return(E_OK));
    SettingsDataManager::settingsDataMap_.EnsureInsert(SYNC_SWITCH_KEY, "0");
    SwitchStatus status = SettingsDataManager::GetSwitchStatus();
    EXPECT_EQ(status, SwitchStatus::NONE);

    SettingsDataManager::settingsDataMap_.Clear();
    GTEST_LOG_(INFO) << "GetSwitchStatusTest005 End";
}

HWTEST_F(SettingsDataManagerTest, GetNetworkConnectionStatusTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetNetworkConnectionStatusTest001 Start";
    EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(nullptr));
    EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).Times(0);
    bool status = SettingsDataManager::GetNetworkConnectionStatus();
    EXPECT_EQ(status, true);

    GTEST_LOG_(INFO) << "GetNetworkConnectionStatusTest001 End";
}

HWTEST_F(SettingsDataManagerTest, GetNetworkConnectionStatusTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetNetworkConnectionStatusTest002 Start";
    EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).Times(0);
    EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).Times(0);
    SettingsDataManager::settingsDataMap_.EnsureInsert(NETWORK_CONNECTION_KEY, "off");
    bool status = SettingsDataManager::GetNetworkConnectionStatus();
    EXPECT_EQ(status, false);

    SettingsDataManager::settingsDataMap_.Clear();
    GTEST_LOG_(INFO) << "GetNetworkConnectionStatusTest002 End";
}

HWTEST_F(SettingsDataManagerTest, GetNetworkConnectionStatusTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetNetworkConnectionStatusTest003 Start";
    EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).Times(0);
    EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).Times(0);
    SettingsDataManager::settingsDataMap_.EnsureInsert(NETWORK_CONNECTION_KEY, "on");
    bool status = SettingsDataManager::GetNetworkConnectionStatus();
    EXPECT_EQ(status, true);

    SettingsDataManager::settingsDataMap_.Clear();
    GTEST_LOG_(INFO) << "GetNetworkConnectionStatusTest003 End";
}

HWTEST_F(SettingsDataManagerTest, GetLocalSpaceFreeStatusTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetLocalSpaceFreeStatusTest001 Start";
    EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(nullptr));
    EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).Times(0);
    int32_t status = SettingsDataManager::GetLocalSpaceFreeStatus();
    EXPECT_EQ(status, 0);
    GTEST_LOG_(INFO) << "GetLocalSpaceFreeStatusTest001 End";
}

HWTEST_F(SettingsDataManagerTest, GetLocalSpaceFreeStatusTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetLocalSpaceFreeStatusTest002 Start";
    EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).Times(0);
    EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).Times(0);
    SettingsDataManager::settingsDataMap_.EnsureInsert(LOCAL_SPACE_FREE_KEY, "1");
    int32_t status = SettingsDataManager::GetLocalSpaceFreeStatus();
    EXPECT_EQ(status, 1);
    SettingsDataManager::settingsDataMap_.Clear();
    GTEST_LOG_(INFO) << "GetLocalSpaceFreeStatusTest002 End";
}

HWTEST_F(SettingsDataManagerTest, GetLocalSpaceFreeDaysTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetLocalSpaceFreeDaysTest001 Start";
    EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(nullptr));
    EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).Times(0);
    int32_t status = SettingsDataManager::GetLocalSpaceFreeDays();
    EXPECT_EQ(status, 30);
    GTEST_LOG_(INFO) << "GetLocalSpaceFreeDaysTest001 End";
}

HWTEST_F(SettingsDataManagerTest, GetLocalSpaceFreeDaysTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetLocalSpaceFreeDaysTest002 Start";
    EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).Times(0);
    EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).Times(0);
    SettingsDataManager::settingsDataMap_.EnsureInsert(LOCAL_SPACE_DAYS_KEY, "10");
    int32_t status = SettingsDataManager::GetLocalSpaceFreeDays();
    EXPECT_EQ(status, 10);
    SettingsDataManager::settingsDataMap_.Clear();
    GTEST_LOG_(INFO) << "GetLocalSpaceFreeDaysTest002 End";
}

HWTEST_F(SettingsDataManagerTest, GetLocalSpaceFreeDaysTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetLocalSpaceFreeDaysTest003 Start";
    EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).Times(0);
    EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).Times(0);
    SettingsDataManager::settingsDataMap_.EnsureInsert(LOCAL_SPACE_DAYS_KEY, "");
    int32_t status = SettingsDataManager::GetLocalSpaceFreeDays();
    EXPECT_EQ(status, 30);
    SettingsDataManager::settingsDataMap_.Clear();
    GTEST_LOG_(INFO) << "GetLocalSpaceFreeDaysTest003 End";
}

HWTEST_F(SettingsDataManagerTest, GetLocalSpaceFreeDaysTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetLocalSpaceFreeDaysTest004 Start";
    EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).Times(0);
    EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).Times(0);
    SettingsDataManager::settingsDataMap_.EnsureInsert(LOCAL_SPACE_DAYS_KEY, "xx");
    int32_t status = SettingsDataManager::GetLocalSpaceFreeDays();
    EXPECT_EQ(status, 30);
    SettingsDataManager::settingsDataMap_.Clear();
    GTEST_LOG_(INFO) << "GetLocalSpaceFreeDaysTest004 End";
}

HWTEST_F(SettingsDataManagerTest, GetMobileDataStatusTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetMobileDataStatusTest001 Start";
    EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(nullptr));
    EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).Times(0);
    bool status = SettingsDataManager::GetMobileDataStatus();
    EXPECT_EQ(status, false);

    GTEST_LOG_(INFO) << "GetMobileDataStatusTest001 End";
}

HWTEST_F(SettingsDataManagerTest, GetMobileDataStatusTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetMobileDataStatusTest002 Start";
    EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).Times(0);
    EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).Times(0);
    SettingsDataManager::settingsDataMap_.EnsureInsert(MOBILE_DATA_SYNC_KEY, "0");
    bool status = SettingsDataManager::GetMobileDataStatus();
    EXPECT_EQ(status, false);

    SettingsDataManager::settingsDataMap_.Clear();
    GTEST_LOG_(INFO) << "GetMobileDataStatusTest002 end";
}

HWTEST_F(SettingsDataManagerTest, GetMobileDataStatusTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetMobileDataStatusTest003 Start";
    EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).Times(0);
    EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).Times(0);
    SettingsDataManager::settingsDataMap_.EnsureInsert(MOBILE_DATA_SYNC_KEY, "1");
    bool status = SettingsDataManager::GetMobileDataStatus();
    EXPECT_EQ(status, true);

    SettingsDataManager::settingsDataMap_.Clear();
    GTEST_LOG_(INFO) << "GetMobileDataStatusTest003 end";
}

HWTEST_F(SettingsDataManagerTest, RegisterObserverTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterObserverTest001 Start";
    EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(nullptr));
    EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).Times(0);
    SettingsDataManager::RegisterObserver("");
    EXPECT_TRUE(true);

    GTEST_LOG_(INFO) << "RegisterObserverTest001 End";
}

HWTEST_F(SettingsDataManagerTest, RegisterObserverTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterObserverTest002 Start";
    std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
    EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(dataShareHelper));
    EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).Times(0);
    SettingsDataManager::RegisterObserver("");
    EXPECT_TRUE(true);

    GTEST_LOG_(INFO) << "RegisterObserverTest002 End";
}

HWTEST_F(SettingsDataManagerTest, RegisterObserverTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterObserverTest003 Start";
    EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(nullptr));
    EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).Times(0);
    SettingsDataManager::RegisterObserver("", nullptr);
    EXPECT_TRUE(true);

    GTEST_LOG_(INFO) << "RegisterObserverTest003 End";
}

HWTEST_F(SettingsDataManagerTest, RegisterObserverTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterObserverTest004 Start";
    std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
    EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(dataShareHelper));
    EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).Times(0);
    SettingsDataManager::RegisterObserver("", nullptr);
    EXPECT_TRUE(true);

    GTEST_LOG_(INFO) << "RegisterObserverTest004 End";
}

HWTEST_F(SettingsDataManagerTest, UnregisterObserverTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterObserverTest001 Start";
    EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(nullptr));
    EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).Times(0);
    SettingsDataManager::UnregisterObserver("");
    EXPECT_TRUE(true);

    GTEST_LOG_(INFO) << "UnregisterObserverTest001 End";
}

HWTEST_F(SettingsDataManagerTest, UnregisterObserverTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterObserverTest002 Start";
    std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
    EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(dataShareHelper));
    EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).Times(0);
    SettingsDataManager::UnregisterObserver("");
    EXPECT_TRUE(true);

    GTEST_LOG_(INFO) << "UnregisterObserverTest002 End";
}

HWTEST_F(SettingsDataManagerTest, QueryTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryTest001 Start";
    DataShareHelperMock::proxy_ = nullptr;
    DataShareResultSetMock::proxy_ = nullptr;
    int32_t ret = -1;
    int32_t size = -1;
    ret = SettingsDataManager::QuerySwitchStatus();
    EXPECT_EQ(ret, E_OK);
    size = SettingsDataManager::settingsDataMap_.Size();
    EXPECT_EQ(size, 1);

    ret = SettingsDataManager::QueryNetworkConnectionStatus();
    EXPECT_EQ(ret, E_OK);
    size = SettingsDataManager::settingsDataMap_.Size();
    EXPECT_EQ(size, 2);

    ret = SettingsDataManager::QueryLocalSpaceFreeStatus();
    EXPECT_EQ(ret, E_OK);
    size = SettingsDataManager::settingsDataMap_.Size();
    EXPECT_EQ(size, 3);

    ret = SettingsDataManager::QueryLocalSpaceFreeDays();
    EXPECT_EQ(ret, E_OK);
    size = SettingsDataManager::settingsDataMap_.Size();
    EXPECT_EQ(size, 4);

    ret = SettingsDataManager::QueryMobileDataStatus();
    EXPECT_EQ(ret, E_OK);
    size = SettingsDataManager::settingsDataMap_.Size();
    EXPECT_EQ(size, 5);

    GTEST_LOG_(INFO) << "QueryTest001 End";
}

HWTEST_F(SettingsDataManagerTest, OnChangeTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnChangeTest001 Start";
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

    GTEST_LOG_(INFO) << "OnChangeTest001 End";
}
}