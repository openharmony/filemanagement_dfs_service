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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "battery_status.h"
#include "cloud_pref_impl_mock.h"
#include "cycle_task_runner.h"
#include "datashare_helper_mock.h"
#include "datashare_result_set_mock.h"
#include "dfs_error.h"
#include "parameters.h"
#include "periodic_clean_task.h"
#include "screen_status.h"
#include "screen_status_mock.h"
#include "settings_data_manager.h"
#include "system_func_mock.h"

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using namespace OHOS::DataShare;
using namespace OHOS::system;

static const std::string PHOTOS_KEY = "persist.kernel.bundle_name.photos";
static const std::string GLOBAL_CONFIG_FILE_PATH = "globalConfig.xml";

class PeriodicCleanTaskTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<CloudFile::DataSyncManager> dataSyncManagerPtr_ = nullptr;
    static inline std::shared_ptr<PeriodicCleanTask> periodicCleanTask_ = nullptr;
    static inline std::shared_ptr<DataShareHelperMock> dataShareHelperMock_ = nullptr;
    static inline std::shared_ptr<DataShareResultSetMock> resultSetMock_ = nullptr;
    static inline std::shared_ptr<CloudPrefImplMock> cloudPrefImplMock_ = nullptr;
    static inline std::shared_ptr<ParameterMock> parameterMock_ = nullptr;
    static inline std::shared_ptr<SystemFuncMock> systemFuncMock_ = nullptr;
    static inline std::shared_ptr<ScreenStatusMock> screenStatusMock_ = nullptr;
};

void PeriodicCleanTaskTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    dataSyncManagerPtr_ = std::make_shared<CloudFile::DataSyncManager>();
    periodicCleanTask_ = std::make_shared<PeriodicCleanTask>(dataSyncManagerPtr_);
    dataShareHelperMock_ = std::make_shared<DataShareHelperMock>();
    DataShareHelperMock::proxy_ = dataShareHelperMock_;
    resultSetMock_ = std::make_shared<DataShareResultSetMock>();
    DataShareResultSetMock::proxy_ = resultSetMock_;
    cloudPrefImplMock_ = std::make_shared<CloudPrefImplMock>();
    CloudPrefImplMock::proxy_ = cloudPrefImplMock_;
    parameterMock_ = std::make_shared<ParameterMock>();
    ParameterMock::proxy_ = parameterMock_;
    systemFuncMock_ = std::make_shared<SystemFuncMock>();
    SystemFuncMock::proxy_ = systemFuncMock_;
    screenStatusMock_ = std::make_shared<ScreenStatusMock>();
    ScreenStatusMock::proxy_ = screenStatusMock_;
}

void PeriodicCleanTaskTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    dataSyncManagerPtr_ = nullptr;
    periodicCleanTask_ = nullptr;
    dataShareHelperMock_ = nullptr;
    DataShareHelperMock::proxy_ = nullptr;
    resultSetMock_ = nullptr;
    DataShareResultSetMock::proxy_ = nullptr;
    cloudPrefImplMock_ = nullptr;
    CloudPrefImplMock::proxy_ = nullptr;
    parameterMock_ = nullptr;
    ParameterMock::proxy_ = nullptr;
    systemFuncMock_ = nullptr;
    SystemFuncMock::proxy_ = nullptr;
    screenStatusMock_ = nullptr;
    ScreenStatusMock::proxy_ = nullptr;
}

void PeriodicCleanTaskTest::SetUp(void)
{
    std::system("rm -rf /data/test_tdd");
}

void PeriodicCleanTaskTest::TearDown(void)
{
    std::system("rm -rf /data/test_tdd");
}

/*
  * @tc.name: RunTaskTest001
  * @tc.desc: Verify the RunTask function.
  * @tc.type: FUNC
  */
HWTEST_F(PeriodicCleanTaskTest, RunTaskTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskTest001 Start";
    try {
        EXPECT_CALL(*cloudPrefImplMock_, GetLong(_, _)).WillOnce(DoAll(SetArgReferee<1>(0)));

        int32_t userId = -1;
        periodicCleanTask_->runTaskForSwitchOff_ = true;
        periodicCleanTask_->RunTask(userId);
        EXPECT_EQ(periodicCleanTask_->runTaskForSwitchOff_, true);
        EXPECT_EQ(periodicCleanTask_->bundleNames_.size(), 1);
        EXPECT_EQ(periodicCleanTask_->taskName_, "periodic_clean_task");
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "RunTaskTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskTest001 End";
}

/*
  * @tc.name: RunTaskTest002
  * @tc.desc: Verify the RunTask function.
  * @tc.type: FUNC
  */
HWTEST_F(PeriodicCleanTaskTest, RunTaskTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskTest002 Start";
    try {
        EXPECT_CALL(*cloudPrefImplMock_, GetLong(_, _)).Times(0);

        int32_t userId = -1;
        periodicCleanTask_->runTaskForSwitchOff_ = false;
        periodicCleanTask_->RunTask(userId);
        EXPECT_EQ(periodicCleanTask_->runTaskForSwitchOff_, false);
        EXPECT_EQ(periodicCleanTask_->bundleNames_.size(), 1);
        EXPECT_EQ(periodicCleanTask_->taskName_, "periodic_clean_task");
        EXPECT_EQ(periodicCleanTask_->runnableBundleNames_, nullptr);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "RunTaskTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskTest002 End";
}

/*
  * @tc.name: RunTaskTest003
  * @tc.desc: Verify the RunTask function.
  * @tc.type: FUNC
  */
HWTEST_F(PeriodicCleanTaskTest, RunTaskTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskTest003 Start";
    try {
        EXPECT_CALL(*cloudPrefImplMock_, GetLong(_, _)).WillOnce(DoAll(SetArgReferee<1>(0)));

        int32_t userId = -1;
        std::shared_ptr<std::set<std::string>> runnableBundleNames = make_shared<std::set<std::string>>();
        runnableBundleNames->insert("xxxx");
        periodicCleanTask_->runTaskForSwitchOff_ = false;
        periodicCleanTask_->SetRunnableBundleNames(runnableBundleNames);
        periodicCleanTask_->RunTask(userId);
        EXPECT_EQ(periodicCleanTask_->runTaskForSwitchOff_, false);
        EXPECT_EQ(periodicCleanTask_->bundleNames_.size(), 1);
        EXPECT_EQ(periodicCleanTask_->taskName_, "periodic_clean_task");
        EXPECT_NE(periodicCleanTask_->runnableBundleNames_, nullptr);
        periodicCleanTask_->runTaskForSwitchOff_ = true;
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "RunTaskTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskTest003 End";
}

/*
  * @tc.name: RunTaskForBundleTest001
  * @tc.desc: Verify the RunTaskForBundle function.
  * @tc.type: FUNC
  */
HWTEST_F(PeriodicCleanTaskTest, RunTaskForBundleTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskForBundleTest001 Start";
    try {
        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillOnce(Return(true));
        BatteryStatus::SetChargingStatus(true);

        int32_t userId = -1;
        std::string bundleName = "xxx";
        int32_t ret = periodicCleanTask_->RunTaskForBundle(userId, bundleName);
        EXPECT_EQ(ret, E_STOP);
        EXPECT_EQ(periodicCleanTask_->runTaskForSwitchOff_, true);
        EXPECT_EQ(periodicCleanTask_->bundleNames_.size(), 1);
        EXPECT_EQ(periodicCleanTask_->taskName_, "periodic_clean_task");
        EXPECT_EQ(periodicCleanTask_->periodicCleanConfig_, nullptr);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "RunTaskForBundleTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskForBundleTest001 End";
}

/*
  * @tc.name: RunTaskForBundleTest002
  * @tc.desc: Verify the RunTaskForBundle function.
  * @tc.type: FUNC
  */
HWTEST_F(PeriodicCleanTaskTest, RunTaskForBundleTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskForBundleTest002 Start";
    try {
        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillOnce(Return(true));
        BatteryStatus::SetChargingStatus(false);

        int32_t userId = -1;
        std::string bundleName = "xxx";
        int32_t ret = periodicCleanTask_->RunTaskForBundle(userId, bundleName);
        EXPECT_EQ(ret, E_STOP);
        EXPECT_EQ(periodicCleanTask_->runTaskForSwitchOff_, true);
        EXPECT_EQ(periodicCleanTask_->bundleNames_.size(), 1);
        EXPECT_EQ(periodicCleanTask_->taskName_, "periodic_clean_task");
        EXPECT_EQ(periodicCleanTask_->periodicCleanConfig_, nullptr);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "RunTaskForBundleTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskForBundleTest002 End";
}

/*
  * @tc.name: RunTaskForBundleTest003
  * @tc.desc: Verify the RunTaskForBundle function.
  * @tc.type: FUNC
  */
HWTEST_F(PeriodicCleanTaskTest, RunTaskForBundleTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskForBundleTest003 Start";
    try {
        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillOnce(Return(false));
        BatteryStatus::SetChargingStatus(false);

        int32_t userId = -1;
        std::string bundleName = "xxx";
        int32_t ret = periodicCleanTask_->RunTaskForBundle(userId, bundleName);
        EXPECT_EQ(ret, E_STOP);
        EXPECT_EQ(periodicCleanTask_->runTaskForSwitchOff_, true);
        EXPECT_EQ(periodicCleanTask_->bundleNames_.size(), 1);
        EXPECT_EQ(periodicCleanTask_->taskName_, "periodic_clean_task");
        EXPECT_EQ(periodicCleanTask_->periodicCleanConfig_, nullptr);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "RunTaskForBundleTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskForBundleTest003 End";
}

/*
  * @tc.name: RunTaskForBundleTest004
  * @tc.desc: Verify the RunTaskForBundle function.
  * @tc.type: FUNC
  */
HWTEST_F(PeriodicCleanTaskTest, RunTaskForBundleTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskForBundleTest004 Start";
    try {
        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillOnce(Return(false));
        BatteryStatus::SetChargingStatus(true);

        std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
        std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(dataShareHelper));
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
        EXPECT_CALL(*resultSetMock_, GoToFirstRow()).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetString(_, _)).WillOnce(DoAll(SetArgReferee<1>("0"), Return(E_OK)));
        EXPECT_CALL(*cloudPrefImplMock_, GetBool(_, _)).WillOnce(DoAll(SetArgReferee<1>(false)));
        EXPECT_CALL(*parameterMock_, GetParameter(_, _)).WillRepeatedly(Return(""));

        int32_t userId = -1;
        std::string bundleName = "xxx";
        periodicCleanTask_->periodicCleanConfig_ = nullptr;
        int32_t ret = periodicCleanTask_->RunTaskForBundle(userId, bundleName);
        EXPECT_EQ(ret, E_STOP);
        EXPECT_EQ(periodicCleanTask_->runTaskForSwitchOff_, true);
        EXPECT_EQ(periodicCleanTask_->bundleNames_.size(), 1);
        EXPECT_EQ(periodicCleanTask_->taskName_, "periodic_clean_task");
        EXPECT_NE(periodicCleanTask_->periodicCleanConfig_, nullptr);
        periodicCleanTask_->periodicCleanConfig_ = nullptr;
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "RunTaskForBundleTest004 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskForBundleTest004 End";
}

/*
  * @tc.name: RunTaskForBundleTest005
  * @tc.desc: Verify the RunTaskForBundle function.
  * @tc.type: FUNC
  */
HWTEST_F(PeriodicCleanTaskTest, RunTaskForBundleTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskForBundleTest005 Start";
    try {
        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillOnce(Return(false));
        BatteryStatus::SetChargingStatus(true);

        std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
        std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(dataShareHelper));
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
        EXPECT_CALL(*resultSetMock_, GoToFirstRow()).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetString(_, _)).WillOnce(DoAll(SetArgReferee<1>("0"), Return(E_OK)));
        EXPECT_CALL(*cloudPrefImplMock_, GetBool(_, _)).WillOnce(DoAll(SetArgReferee<1>(false)));
        EXPECT_CALL(*parameterMock_, GetParameter(_, _)).WillOnce(Return("")).WillOnce(Return(""));

        int32_t userId = 100;
        std::string bundleName = "xxx";
        periodicCleanTask_->periodicCleanConfig_ =
            std::make_unique<CloudPrefImpl>(userId, bundleName, GLOBAL_CONFIG_FILE_PATH);
        int32_t ret = periodicCleanTask_->RunTaskForBundle(userId, bundleName);
        EXPECT_EQ(ret, E_STOP);
        EXPECT_EQ(periodicCleanTask_->runTaskForSwitchOff_, true);
        EXPECT_EQ(periodicCleanTask_->bundleNames_.size(), 1);
        EXPECT_EQ(periodicCleanTask_->taskName_, "periodic_clean_task");
        EXPECT_NE(periodicCleanTask_->periodicCleanConfig_, nullptr);
        periodicCleanTask_->periodicCleanConfig_ = nullptr;
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "RunTaskForBundleTest005 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskForBundleTest005 End";
}

/*
  * @tc.name: RunTaskForBundleTest006
  * @tc.desc: Verify the RunTaskForBundle function.
  * @tc.type: FUNC
  */
HWTEST_F(PeriodicCleanTaskTest, RunTaskForBundleTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskForBundleTest006 Start";
    try {
        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillOnce(Return(false));
        BatteryStatus::SetChargingStatus(true);

        std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
        std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(dataShareHelper));
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
        EXPECT_CALL(*resultSetMock_, GoToFirstRow()).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetString(_, _)).WillOnce(DoAll(SetArgReferee<1>("0"), Return(E_OK)));
        EXPECT_CALL(*cloudPrefImplMock_, GetBool(_, _)).WillOnce(DoAll(SetArgReferee<1>(false)));
        EXPECT_CALL(*parameterMock_, GetParameter(_, _)).WillOnce(Return("")).WillOnce(Return(""));

        int32_t userId = -1;
        std::string bundleName = "xxx";
        periodicCleanTask_->periodicCleanConfig_ =
            std::make_unique<CloudPrefImpl>(userId, bundleName, GLOBAL_CONFIG_FILE_PATH);
        int32_t ret = periodicCleanTask_->RunTaskForBundle(userId, bundleName);
        EXPECT_EQ(ret, E_STOP);
        EXPECT_EQ(periodicCleanTask_->runTaskForSwitchOff_, true);
        EXPECT_EQ(periodicCleanTask_->bundleNames_.size(), 1);
        EXPECT_EQ(periodicCleanTask_->taskName_, "periodic_clean_task");
        EXPECT_NE(periodicCleanTask_->periodicCleanConfig_, nullptr);
        periodicCleanTask_->periodicCleanConfig_ = nullptr;
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "RunTaskForBundleTest006 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskForBundleTest006 End";
}

/*
  * @tc.name: RunTaskForBundleTest007
  * @tc.desc: Verify the RunTaskForBundle function.
  * @tc.type: FUNC
  */
HWTEST_F(PeriodicCleanTaskTest, RunTaskForBundleTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskForBundleTest007 Start";
    try {
        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillOnce(Return(false));
        BatteryStatus::SetChargingStatus(true);

        std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
        std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(dataShareHelper));
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
        EXPECT_CALL(*resultSetMock_, GoToFirstRow()).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetString(_, _)).WillOnce(DoAll(SetArgReferee<1>("0"), Return(E_OK)));
        EXPECT_CALL(*cloudPrefImplMock_, GetBool(_, _)).WillOnce(DoAll(SetArgReferee<1>(false)));
        EXPECT_CALL(*parameterMock_, GetParameter(_, _)).WillOnce(Return("")).WillOnce(Return("com.xxx"));

        int32_t userId = -1;
        std::string bundleName = "xxx";
        periodicCleanTask_->periodicCleanConfig_ =
            std::make_unique<CloudPrefImpl>(userId, bundleName, GLOBAL_CONFIG_FILE_PATH);
        int32_t ret = periodicCleanTask_->RunTaskForBundle(userId, bundleName);
        EXPECT_EQ(ret, E_RENAME_FAIL);
        EXPECT_EQ(periodicCleanTask_->runTaskForSwitchOff_, true);
        EXPECT_EQ(periodicCleanTask_->bundleNames_.size(), 1);
        EXPECT_EQ(periodicCleanTask_->taskName_, "periodic_clean_task");
        EXPECT_NE(periodicCleanTask_->periodicCleanConfig_, nullptr);
        periodicCleanTask_->periodicCleanConfig_ = nullptr;
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "RunTaskForBundleTest007 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskForBundleTest007 End";
}

/*
  * @tc.name: RunTaskForBundleTest008
  * @tc.desc: Verify the RunTaskForBundle function.
  * @tc.type: FUNC
  */
HWTEST_F(PeriodicCleanTaskTest, RunTaskForBundleTest008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskForBundleTest008 Start";
    try {
        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillOnce(Return(false));
        BatteryStatus::SetChargingStatus(true);

        std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
        std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(dataShareHelper));
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
        EXPECT_CALL(*resultSetMock_, GoToFirstRow()).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetString(_, _)).WillOnce(DoAll(SetArgReferee<1>("0"), Return(E_OK)));
        EXPECT_CALL(*cloudPrefImplMock_, GetBool(_, _)).WillOnce(DoAll(SetArgReferee<1>(false)));
        EXPECT_CALL(*parameterMock_, GetParameter(_, _)).WillOnce(Return("com.xxx")).WillOnce(Return(""));

        int32_t userId = -1;
        std::string bundleName = "xxx";
        periodicCleanTask_->periodicCleanConfig_ =
            std::make_unique<CloudPrefImpl>(userId, bundleName, GLOBAL_CONFIG_FILE_PATH);
        int32_t ret = periodicCleanTask_->RunTaskForBundle(userId, bundleName);
        EXPECT_EQ(ret, E_STOP);
        EXPECT_EQ(periodicCleanTask_->runTaskForSwitchOff_, true);
        EXPECT_EQ(periodicCleanTask_->bundleNames_.size(), 1);
        EXPECT_EQ(periodicCleanTask_->taskName_, "periodic_clean_task");
        EXPECT_NE(periodicCleanTask_->periodicCleanConfig_, nullptr);
        periodicCleanTask_->periodicCleanConfig_ = nullptr;
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "RunTaskForBundleTest008 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskForBundleTest008 End";
}

/*
  * @tc.name: RunTaskForBundleTest009
  * @tc.desc: Verify the RunTaskForBundle function.
  * @tc.type: FUNC
  */
HWTEST_F(PeriodicCleanTaskTest, RunTaskForBundleTest009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskForBundleTest009 Start";
    try {
        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillOnce(Return(false));
        BatteryStatus::SetChargingStatus(true);

        std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
        std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(dataShareHelper));
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
        EXPECT_CALL(*resultSetMock_, GoToFirstRow()).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetString(_, _)).WillOnce(DoAll(SetArgReferee<1>("0"), Return(E_OK)));
        EXPECT_CALL(*cloudPrefImplMock_, GetBool(_, _)).WillOnce(DoAll(SetArgReferee<1>(false)));
        EXPECT_CALL(*parameterMock_, GetParameter(_, _)).WillOnce(Return("com.xxx")).WillOnce(Return("com.xxx"));

        int32_t userId = -1;
        std::string bundleName = "xxx";
        periodicCleanTask_->periodicCleanConfig_ =
            std::make_unique<CloudPrefImpl>(userId, bundleName, GLOBAL_CONFIG_FILE_PATH);
        int32_t ret = periodicCleanTask_->RunTaskForBundle(userId, bundleName);
        EXPECT_EQ(ret, E_OK);
        EXPECT_EQ(periodicCleanTask_->runTaskForSwitchOff_, true);
        EXPECT_EQ(periodicCleanTask_->bundleNames_.size(), 1);
        EXPECT_EQ(periodicCleanTask_->taskName_, "periodic_clean_task");
        EXPECT_NE(periodicCleanTask_->periodicCleanConfig_, nullptr);
        periodicCleanTask_->periodicCleanConfig_ = nullptr;
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "RunTaskForBundleTest009 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskForBundleTest009 End";
}

/*
  * @tc.name: RunTaskForBundleTest010
  * @tc.desc: Verify the RunTaskForBundle function.
  * @tc.type: FUNC
  */
HWTEST_F(PeriodicCleanTaskTest, RunTaskForBundleTest010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskForBundleTest010 Start";
    try {
        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillOnce(Return(false));
        BatteryStatus::SetChargingStatus(true);

        std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
        std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(dataShareHelper));
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
        EXPECT_CALL(*resultSetMock_, GoToFirstRow()).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetString(_, _)).WillOnce(DoAll(SetArgReferee<1>("1"), Return(E_OK)));
        EXPECT_CALL(*cloudPrefImplMock_, GetBool(_, _)).Times(0);
        EXPECT_CALL(*parameterMock_, GetParameter(_, _)).WillOnce(Return(""));

        int32_t userId = -1;
        std::string bundleName = "xxx";
        int32_t ret = periodicCleanTask_->RunTaskForBundle(userId, bundleName);
        EXPECT_EQ(ret, E_STOP);
        EXPECT_EQ(periodicCleanTask_->runTaskForSwitchOff_, true);
        EXPECT_EQ(periodicCleanTask_->bundleNames_.size(), 1);
        EXPECT_EQ(periodicCleanTask_->taskName_, "periodic_clean_task");
        EXPECT_EQ(periodicCleanTask_->periodicCleanConfig_, nullptr);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "RunTaskForBundleTest010 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskForBundleTest010 End";
}

/*
  * @tc.name: RunTaskForBundleTest011
  * @tc.desc: Verify the RunTaskForBundle function.
  * @tc.type: FUNC
  */
HWTEST_F(PeriodicCleanTaskTest, RunTaskForBundleTest011, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskForBundleTest011 Start";
    try {
        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillOnce(Return(false));
        BatteryStatus::SetChargingStatus(true);

        std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
        std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(dataShareHelper));
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
        EXPECT_CALL(*resultSetMock_, GoToFirstRow()).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetString(_, _)).WillOnce(DoAll(SetArgReferee<1>("2"), Return(E_OK)));
        EXPECT_CALL(*cloudPrefImplMock_, GetBool(_, _)).Times(0);
        EXPECT_CALL(*parameterMock_, GetParameter(_, _)).WillOnce(Return(""));

        int32_t userId = -1;
        std::string bundleName = "xxx";
        int32_t ret = periodicCleanTask_->RunTaskForBundle(userId, bundleName);
        EXPECT_EQ(ret, E_STOP);
        EXPECT_EQ(periodicCleanTask_->runTaskForSwitchOff_, true);
        EXPECT_EQ(periodicCleanTask_->bundleNames_.size(), 1);
        EXPECT_EQ(periodicCleanTask_->taskName_, "periodic_clean_task");
        EXPECT_EQ(periodicCleanTask_->periodicCleanConfig_, nullptr);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "RunTaskForBundleTest011 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskForBundleTest011 End";
}

/*
  * @tc.name: RunTaskForBundleTest012
  * @tc.desc: Verify the RunTaskForBundle function.
  * @tc.type: FUNC
  */
HWTEST_F(PeriodicCleanTaskTest, RunTaskForBundleTest012, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskForBundleTest012 Start";
    try {
        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillOnce(Return(false));
        BatteryStatus::SetChargingStatus(true);

        std::shared_ptr<DataShareHelper> dataShareHelper = std::make_shared<DataShareHelper>();
        std::shared_ptr<DataShareResultSet> resultSet = std::make_shared<DataShareResultSet>();
        EXPECT_CALL(*dataShareHelperMock_, Creator(_, _, _)).WillOnce(Return(dataShareHelper));
        EXPECT_CALL(*dataShareHelperMock_, Query(_, _, _, _)).WillOnce(Return(resultSet));
        EXPECT_CALL(*resultSetMock_, GoToFirstRow()).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetString(_, _)).WillOnce(DoAll(SetArgReferee<1>("2"), Return(E_OK)));
        EXPECT_CALL(*cloudPrefImplMock_, GetBool(_, _)).Times(0);
        EXPECT_CALL(*parameterMock_, GetParameter(_, _)).WillOnce(Return("com.xxx"));

        int32_t userId = -1;
        std::string bundleName = "xxx";
        int32_t ret = periodicCleanTask_->RunTaskForBundle(userId, bundleName);
        EXPECT_EQ(ret, E_OK);
        EXPECT_EQ(periodicCleanTask_->runTaskForSwitchOff_, true);
        EXPECT_EQ(periodicCleanTask_->bundleNames_.size(), 1);
        EXPECT_EQ(periodicCleanTask_->taskName_, "periodic_clean_task");
        EXPECT_EQ(periodicCleanTask_->periodicCleanConfig_, nullptr);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "RunTaskForBundleTest012 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskForBundleTest012 End";
}

/**
 * @tc.name: RenameCacheDirTest001
 * @tc.desc: Verify RenameCacheDir function.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicCleanTaskTest, RenameCacheDirTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RenameCacheDirTest001 Start.";
    try {
        std::system("mkdir -p /data/service/el2/50/hmdfs/cache/account_cache/dentry_cache/cloud");
        std::system("mkdir -p /mnt/hmdfs/50/account/device_view/local/files/.cloud_cache/pread_cache");
        std::system("mkdir -p /mnt/hmdfs/50/account/device_view/local/files/.cloud_cache/download_cache");
        std::system("mkdir -p /mnt/hmdfs/50/account/device_view/local/data/com.photos/.video_cache");
        std::system("touch /data/service/el2/50/hmdfs/cache/account_cache/dentry_cache/cloud/file.txt");
        std::system("touch /mnt/hmdfs/50/account/device_view/local/files/.cloud_cache/pread_cache/file.txt");
        std::system("touch /mnt/hmdfs/50/account/device_view/local/files/.cloud_cache/download_cache/file.txt");
        std::system("touch /mnt/hmdfs/50/account/device_view/local/data/com.photos/.video_cache/file.txt");

        EXPECT_CALL(*parameterMock_, GetParameter(_, _)).Times(0);
        EXPECT_CALL(*systemFuncMock_, rename(_, _)).WillOnce(Return(-1));

        int32_t userId = 50;
        int32_t ret = periodicCleanTask_->RenameCacheDir(userId);
        EXPECT_EQ(ret, E_RENAME_FAIL);
        EXPECT_EQ(periodicCleanTask_->runTaskForSwitchOff_, true);
        EXPECT_EQ(periodicCleanTask_->bundleNames_.size(), 1);
        EXPECT_EQ(periodicCleanTask_->taskName_, "periodic_clean_task");

        std::system("rm -rf /data/service/el2/50/hmdfs/cache/account_cache/dentry_cache/cloud*");
        std::system("rm -rf /mnt/hmdfs/50/account/device_view/local/files/.cloud_cache/pread_cache*");
        std::system("rm -rf /mnt/hmdfs/50/account/device_view/local/files/.cloud_cache/download_cache*");
        std::system("rm -rf /mnt/hmdfs/50/account/device_view/local/data/com.xxxx/.video_cache*");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RenameCacheDirTest001 ERROR.";
    }
    GTEST_LOG_(INFO) << "RenameCacheDirTest001 End";
}

/**
 * @tc.name: RenameCacheDirTest002
 * @tc.desc: Verify RenameCacheDir function.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicCleanTaskTest, RenameCacheDirTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RenameCacheDirTest002 Start.";
    try {
        std::system("mkdir -p /data/service/el2/50/hmdfs/cache/account_cache/dentry_cache/cloud");
        std::system("mkdir -p /mnt/hmdfs/50/account/device_view/local/files/.cloud_cache/pread_cache");
        std::system("mkdir -p /mnt/hmdfs/50/account/device_view/local/files/.cloud_cache/download_cache");
        std::system("mkdir -p /mnt/hmdfs/50/account/device_view/local/data/com.photos/.video_cache");
        std::system("touch /data/service/el2/50/hmdfs/cache/account_cache/dentry_cache/cloud/file.txt");
        std::system("touch /mnt/hmdfs/50/account/device_view/local/files/.cloud_cache/pread_cache/file.txt");
        std::system("touch /mnt/hmdfs/50/account/device_view/local/files/.cloud_cache/download_cache/file.txt");
        std::system("touch /mnt/hmdfs/50/account/device_view/local/data/com.photos/.video_cache/file.txt");

        EXPECT_CALL(*parameterMock_, GetParameter(_, _)).Times(0);
        EXPECT_CALL(*systemFuncMock_, rename(_, _)).WillOnce(Return(0)).WillOnce(Return(-1));
        EXPECT_CALL(*systemFuncMock_, mkdir(_, _)).WillRepeatedly(Return(0));

        int32_t userId = 50;
        int32_t ret = periodicCleanTask_->RenameCacheDir(userId);
        EXPECT_EQ(ret, E_RENAME_FAIL);
        EXPECT_EQ(periodicCleanTask_->runTaskForSwitchOff_, true);
        EXPECT_EQ(periodicCleanTask_->bundleNames_.size(), 1);
        EXPECT_EQ(periodicCleanTask_->taskName_, "periodic_clean_task");

        std::system("rm -rf /data/service/el2/50/hmdfs/cache/account_cache/dentry_cache/cloud*");
        std::system("rm -rf /mnt/hmdfs/50/account/device_view/local/files/.cloud_cache/pread_cache*");
        std::system("rm -rf /mnt/hmdfs/50/account/device_view/local/files/.cloud_cache/download_cache*");
        std::system("rm -rf /mnt/hmdfs/50/account/device_view/local/data/com.xxxx/.video_cache*");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RenameCacheDirTest002 ERROR.";
    }
    GTEST_LOG_(INFO) << "RenameCacheDirTest002 End";
}

/**
 * @tc.name: RenameCacheDirTest003
 * @tc.desc: Verify RenameCacheDir function.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicCleanTaskTest, RenameCacheDirTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RenameCacheDirTest003 Start.";
    try {
        std::system("mkdir -p /data/service/el2/50/hmdfs/cache/account_cache/dentry_cache/cloud");
        std::system("mkdir -p /mnt/hmdfs/50/account/device_view/local/files/.cloud_cache/pread_cache");
        std::system("mkdir -p /mnt/hmdfs/50/account/device_view/local/files/.cloud_cache/download_cache");
        std::system("mkdir -p /mnt/hmdfs/50/account/device_view/local/data/com.photos/.video_cache");
        std::system("touch /data/service/el2/50/hmdfs/cache/account_cache/dentry_cache/cloud/file.txt");
        std::system("touch /mnt/hmdfs/50/account/device_view/local/files/.cloud_cache/pread_cache/file.txt");
        std::system("touch /mnt/hmdfs/50/account/device_view/local/files/.cloud_cache/download_cache/file.txt");
        std::system("touch /mnt/hmdfs/50/account/device_view/local/data/com.photos/.video_cache/file.txt");

        EXPECT_CALL(*parameterMock_, GetParameter(_, _)).Times(0);
        EXPECT_CALL(*systemFuncMock_, rename(_, _)).WillOnce(Return(0)).WillOnce(Return(0)).WillOnce(Return(-1));
        EXPECT_CALL(*systemFuncMock_, mkdir(_, _)).WillOnce(Return(0)).WillOnce(Return(0));

        int32_t userId = 50;
        int32_t ret = periodicCleanTask_->RenameCacheDir(userId);
        EXPECT_EQ(ret, E_RENAME_FAIL);
        EXPECT_EQ(periodicCleanTask_->runTaskForSwitchOff_, true);
        EXPECT_EQ(periodicCleanTask_->bundleNames_.size(), 1);
        EXPECT_EQ(periodicCleanTask_->taskName_, "periodic_clean_task");

        std::system("rm -rf /data/service/el2/50/hmdfs/cache/account_cache/dentry_cache/cloud*");
        std::system("rm -rf /mnt/hmdfs/50/account/device_view/local/files/.cloud_cache/pread_cache*");
        std::system("rm -rf /mnt/hmdfs/50/account/device_view/local/files/.cloud_cache/download_cache*");
        std::system("rm -rf /mnt/hmdfs/50/account/device_view/local/data/com.xxxx/.video_cache*");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RenameCacheDirTest003 ERROR.";
    }
    GTEST_LOG_(INFO) << "RenameCacheDirTest003 End";
}

/**
 * @tc.name: RenameCacheDirTest004
 * @tc.desc: Verify RenameCacheDir function.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicCleanTaskTest, RenameCacheDirTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RenameCacheDirTest004 Start.";
    try {
        std::system("mkdir -p /data/service/el2/50/hmdfs/cache/account_cache/dentry_cache/cloud");
        std::system("mkdir -p /mnt/hmdfs/50/account/device_view/local/files/.cloud_cache/pread_cache");
        std::system("mkdir -p /mnt/hmdfs/50/account/device_view/local/files/.cloud_cache/download_cache");
        std::system("mkdir -p /mnt/hmdfs/50/account/device_view/local/data/com.photos/.video_cache");
        std::system("touch /data/service/el2/50/hmdfs/cache/account_cache/dentry_cache/cloud/file.txt");
        std::system("touch /mnt/hmdfs/50/account/device_view/local/files/.cloud_cache/pread_cache/file.txt");
        std::system("touch /mnt/hmdfs/50/account/device_view/local/files/.cloud_cache/download_cache/file.txt");
        std::system("touch /mnt/hmdfs/50/account/device_view/local/data/com.photos/.video_cache/file.txt");

        EXPECT_CALL(*parameterMock_, GetParameter(_, _)).WillOnce(Return(""));
        EXPECT_CALL(*systemFuncMock_, rename(_, _)).WillOnce(Return(0)).WillOnce(Return(0)).WillOnce(Return(0));
        EXPECT_CALL(*systemFuncMock_, mkdir(_, _)).WillOnce(Return(0)).WillOnce(Return(0)).WillOnce(Return(0));

        int32_t userId = 50;
        int32_t ret = periodicCleanTask_->RenameCacheDir(userId);
        EXPECT_EQ(ret, E_RENAME_FAIL);
        EXPECT_EQ(periodicCleanTask_->runTaskForSwitchOff_, true);
        EXPECT_EQ(periodicCleanTask_->bundleNames_.size(), 1);
        EXPECT_EQ(periodicCleanTask_->taskName_, "periodic_clean_task");

        std::system("rm -rf /data/service/el2/50/hmdfs/cache/account_cache/dentry_cache/cloud*");
        std::system("rm -rf /mnt/hmdfs/50/account/device_view/local/files/.cloud_cache/pread_cache*");
        std::system("rm -rf /mnt/hmdfs/50/account/device_view/local/files/.cloud_cache/download_cache*");
        std::system("rm -rf /mnt/hmdfs/50/account/device_view/local/data/com.xxxx/.video_cache*");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RenameCacheDirTest004 ERROR.";
    }
    GTEST_LOG_(INFO) << "RenameCacheDirTest004 End";
}

/**
 * @tc.name: RenameCacheDirTest005
 * @tc.desc: Verify RenameCacheDir function.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicCleanTaskTest, RenameCacheDirTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RenameCacheDirTest005 Start.";
    try {
        std::system("mkdir -p /data/service/el2/50/hmdfs/cache/account_cache/dentry_cache/cloud");
        std::system("mkdir -p /mnt/hmdfs/50/account/device_view/local/files/.cloud_cache/pread_cache");
        std::system("mkdir -p /mnt/hmdfs/50/account/device_view/local/files/.cloud_cache/download_cache");
        std::system("mkdir -p /mnt/hmdfs/50/account/device_view/local/data/com.photos/.video_cache");
        std::system("touch /data/service/el2/50/hmdfs/cache/account_cache/dentry_cache/cloud/file.txt");
        std::system("touch /mnt/hmdfs/50/account/device_view/local/files/.cloud_cache/pread_cache/file.txt");
        std::system("touch /mnt/hmdfs/50/account/device_view/local/files/.cloud_cache/download_cache/file.txt");
        std::system("touch /mnt/hmdfs/50/account/device_view/local/data/com.photos/.video_cache/file.txt");

        EXPECT_CALL(*parameterMock_, GetParameter(_, _)).WillOnce(Return("com.photos"));
        EXPECT_CALL(*systemFuncMock_, rename(_, _)).WillOnce(Return(0)).WillOnce(Return(0))
            .WillOnce(Return(0)).WillOnce(Return(-1));
        EXPECT_CALL(*systemFuncMock_, mkdir(_, _)).WillOnce(Return(0)).WillOnce(Return(0)).WillOnce(Return(0));

        int32_t userId = 50;
        int32_t ret = periodicCleanTask_->RenameCacheDir(userId);
        EXPECT_EQ(ret, E_RENAME_FAIL);
        EXPECT_EQ(periodicCleanTask_->runTaskForSwitchOff_, true);
        EXPECT_EQ(periodicCleanTask_->bundleNames_.size(), 1);
        EXPECT_EQ(periodicCleanTask_->taskName_, "periodic_clean_task");

        std::system("rm -rf /data/service/el2/50/hmdfs/cache/account_cache/dentry_cache/cloud*");
        std::system("rm -rf /mnt/hmdfs/50/account/device_view/local/files/.cloud_cache/pread_cache*");
        std::system("rm -rf /mnt/hmdfs/50/account/device_view/local/files/.cloud_cache/download_cache*");
        std::system("rm -rf /mnt/hmdfs/50/account/device_view/local/data/com.xxxx/.video_cache*");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RenameCacheDirTest005 ERROR.";
    }
    GTEST_LOG_(INFO) << "RenameCacheDirTest005 End";
}

/**
 * @tc.name: RenameCacheDirTest006
 * @tc.desc: Verify RenameCacheDir function.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicCleanTaskTest, RenameCacheDirTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RenameCacheDirTest006 Start.";
    try {
        std::system("mkdir -p /data/service/el2/50/hmdfs/cache/account_cache/dentry_cache/cloud");
        std::system("mkdir -p /mnt/hmdfs/50/account/device_view/local/files/.cloud_cache/pread_cache");
        std::system("mkdir -p /mnt/hmdfs/50/account/device_view/local/files/.cloud_cache/download_cache");
        std::system("mkdir -p /mnt/hmdfs/50/account/device_view/local/data/com.photos/.video_cache");
        std::system("touch /data/service/el2/50/hmdfs/cache/account_cache/dentry_cache/cloud/file.txt");
        std::system("touch /mnt/hmdfs/50/account/device_view/local/files/.cloud_cache/pread_cache/file.txt");
        std::system("touch /mnt/hmdfs/50/account/device_view/local/files/.cloud_cache/download_cache/file.txt");
        std::system("touch /mnt/hmdfs/50/account/device_view/local/data/com.photos/.video_cache/file.txt");

        EXPECT_CALL(*parameterMock_, GetParameter(_, _)).WillOnce(Return("com.photos"));
        EXPECT_CALL(*systemFuncMock_, rename(_, _)).WillOnce(Return(0)).WillOnce(Return(0))
            .WillOnce(Return(0)).WillOnce(Return(0));
        EXPECT_CALL(*systemFuncMock_, mkdir(_, _)).WillOnce(Return(0)).WillOnce(Return(0))
            .WillOnce(Return(0)).WillOnce(Return(0));

        int32_t userId = 50;
        int32_t ret = periodicCleanTask_->RenameCacheDir(userId);
        EXPECT_EQ(ret, E_OK);
        EXPECT_EQ(periodicCleanTask_->runTaskForSwitchOff_, true);
        EXPECT_EQ(periodicCleanTask_->bundleNames_.size(), 1);
        EXPECT_EQ(periodicCleanTask_->taskName_, "periodic_clean_task");

        std::system("rm -rf /data/service/el2/50/hmdfs/cache/account_cache/dentry_cache/cloud*");
        std::system("rm -rf /mnt/hmdfs/50/account/device_view/local/files/.cloud_cache/pread_cache*");
        std::system("rm -rf /mnt/hmdfs/50/account/device_view/local/files/.cloud_cache/download_cache*");
        std::system("rm -rf /mnt/hmdfs/50/account/device_view/local/data/com.xxxx/.video_cache*");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RenameCacheDirTest006 ERROR.";
    }
    GTEST_LOG_(INFO) << "RenameCacheDirTest006 End";
}

/**
 * @tc.name: RenameTempDirTest001
 * @tc.desc: Verify RenameTempDir function.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicCleanTaskTest, RenameTempDirTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RenameTempDirTest001 Start.";
    try {
        std::string dir = "";
        std::string tempSuffix = "";
        int32_t ret = periodicCleanTask_->RenameTempDir(dir, tempSuffix);
        EXPECT_EQ(ret, E_OK);
        EXPECT_EQ(periodicCleanTask_->runTaskForSwitchOff_, true);
        EXPECT_EQ(periodicCleanTask_->bundleNames_.size(), 1);
        EXPECT_EQ(periodicCleanTask_->taskName_, "periodic_clean_task");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RenameTempDirTest001 ERROR.";
    }
    GTEST_LOG_(INFO) << "RenameTempDirTest001 End";
}

/**
 * @tc.name: RenameTempDirTest002
 * @tc.desc: Verify RenameTempDir function.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicCleanTaskTest, RenameTempDirTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RenameTempDirTest002 Start.";
    try {
        std::string dir = "/data/test_tdd";
        std::string tempSuffix = "_temp_123";
        int32_t ret = periodicCleanTask_->RenameTempDir(dir, tempSuffix);
        EXPECT_EQ(ret, E_OK);
        EXPECT_EQ(periodicCleanTask_->runTaskForSwitchOff_, true);
        EXPECT_EQ(periodicCleanTask_->bundleNames_.size(), 1);
        EXPECT_EQ(periodicCleanTask_->taskName_, "periodic_clean_task");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RenameTempDirTest002 ERROR.";
    }
    GTEST_LOG_(INFO) << "RenameTempDirTest002 End";
}

/**
 * @tc.name: RenameTempDirTest003
 * @tc.desc: Verify RenameTempDir function.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicCleanTaskTest, RenameTempDirTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RenameTempDirTest003 Start.";
    try {
        std::system("touch /data/test_tdd");

        std::string dir = "/data/test_tdd";
        std::string tempSuffix = "_temp_123";
        int32_t ret = periodicCleanTask_->RenameTempDir(dir, tempSuffix);
        EXPECT_EQ(ret, E_OK);
        EXPECT_EQ(periodicCleanTask_->runTaskForSwitchOff_, true);
        EXPECT_EQ(periodicCleanTask_->bundleNames_.size(), 1);
        EXPECT_EQ(periodicCleanTask_->taskName_, "periodic_clean_task");

        std::system("rm /data/test_tdd");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RenameTempDirTest003 ERROR.";
    }
    GTEST_LOG_(INFO) << "RenameTempDirTest003 End";
}

/**
 * @tc.name: RenameTempDirTest004
 * @tc.desc: Verify RenameTempDir function.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicCleanTaskTest, RenameTempDirTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RenameTempDirTest004 Start.";
    try {
        std::system("mkdir /data/test_tdd");

        std::string dir = "/data/test_tdd";
        std::string tempSuffix = "_temp_123";
        int32_t ret = periodicCleanTask_->RenameTempDir(dir, tempSuffix);
        EXPECT_EQ(ret, E_OK);
        EXPECT_EQ(periodicCleanTask_->runTaskForSwitchOff_, true);
        EXPECT_EQ(periodicCleanTask_->bundleNames_.size(), 1);
        EXPECT_EQ(periodicCleanTask_->taskName_, "periodic_clean_task");

        std::system("rm -rf /data/test_tdd");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RenameTempDirTest004 ERROR.";
    }
    GTEST_LOG_(INFO) << "RenameTempDirTest004 End";
}

/**
 * @tc.name: RenameTempDirTest005
 * @tc.desc: Verify RenameTempDir function.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicCleanTaskTest, RenameTempDirTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RenameTempDirTest005 Start.";
    try {
        std::system("mkdir /data/test_tdd");
        std::system("touch /data/test_tdd/file1.txt");
        EXPECT_CALL(*systemFuncMock_, rename(_, _)).WillOnce(Return(-1));
        EXPECT_CALL(*systemFuncMock_, mkdir(_, _)).Times(0);

        std::string dir = "/data/test_tdd";
        std::string tempSuffix = "_temp_123";
        int32_t ret = periodicCleanTask_->RenameTempDir(dir, tempSuffix);
        EXPECT_EQ(ret, E_RENAME_FAIL);
        EXPECT_EQ(periodicCleanTask_->runTaskForSwitchOff_, true);
        EXPECT_EQ(periodicCleanTask_->bundleNames_.size(), 1);
        EXPECT_EQ(periodicCleanTask_->taskName_, "periodic_clean_task");

        std::system("rm -rf /data/test_tdd");
        std::system("rm -rf /data/test_tdd_temp_123");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RenameTempDirTest005 ERROR.";
    }
    GTEST_LOG_(INFO) << "RenameTempDirTest005 End";
}

/**
 * @tc.name: RenameTempDirTest006
 * @tc.desc: Verify RenameTempDir function.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicCleanTaskTest, RenameTempDirTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RenameTempDirTest006 Start.";
    try {
        std::system("mkdir /data/test_tdd");
        std::system("touch /data/test_tdd/file1.txt");
        EXPECT_CALL(*systemFuncMock_, rename(_, _)).WillOnce(Return(0)).WillOnce(Return(0));
        EXPECT_CALL(*systemFuncMock_, mkdir(_, _)).WillOnce(Return(-1));

        std::string dir = "/data/test_tdd";
        std::string tempSuffix = "_temp_123";
        int32_t ret = periodicCleanTask_->RenameTempDir(dir, tempSuffix);
        EXPECT_EQ(ret, E_RENAME_FAIL);
        EXPECT_EQ(periodicCleanTask_->runTaskForSwitchOff_, true);
        EXPECT_EQ(periodicCleanTask_->bundleNames_.size(), 1);
        EXPECT_EQ(periodicCleanTask_->taskName_, "periodic_clean_task");

        std::system("rm -rf /data/test_tdd");
        std::system("rm -rf /data/test_tdd_temp_123");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RenameTempDirTest006 ERROR.";
    }
    GTEST_LOG_(INFO) << "RenameTempDirTest006 End";
}

/**
 * @tc.name: RenameTempDirTest007
 * @tc.desc: Verify RenameTempDir function.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicCleanTaskTest, RenameTempDirTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RenameTempDirTest007 Start.";
    try {
        std::system("mkdir /data/test_tdd");
        std::system("touch /data/test_tdd/file1.txt");
        EXPECT_CALL(*systemFuncMock_, rename(_, _)).WillOnce(Return(0));
        EXPECT_CALL(*systemFuncMock_, mkdir(_, _)).WillOnce(Return(0));

        std::string dir = "/data/test_tdd";
        std::string tempSuffix = "_temp_123";
        int32_t ret = periodicCleanTask_->RenameTempDir(dir, tempSuffix);
        EXPECT_EQ(ret, E_OK);
        EXPECT_EQ(periodicCleanTask_->runTaskForSwitchOff_, true);
        EXPECT_EQ(periodicCleanTask_->bundleNames_.size(), 1);
        EXPECT_EQ(periodicCleanTask_->taskName_, "periodic_clean_task");

        std::system("rm -rf /data/test_tdd");
        std::system("rm -rf /data/test_tdd_temp_123");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RenameTempDirTest007 ERROR.";
    }
    GTEST_LOG_(INFO) << "RenameTempDirTest007 End";
}

/**
 * @tc.name: PeriodicCleanTempDirTest001
 * @tc.desc: Verify PeriodicCleanTempDir function.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicCleanTaskTest, PeriodicCleanTempDirTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PeriodicCleanTempDirTest001 Start.";
    try {
        std::system("mkdir -p /data/service/el2/100/hmdfs/cache/account_cache/dentry_cache/cloud_temp_123");

        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillOnce(Return(true));
        EXPECT_CALL(*parameterMock_, GetParameter(_, _)).Times(0);

        int32_t userId = 100;
        int32_t ret = periodicCleanTask_->PeriodicCleanTempDir(userId);
        EXPECT_EQ(ret, E_STOP);
        EXPECT_EQ(periodicCleanTask_->runTaskForSwitchOff_, true);
        EXPECT_EQ(periodicCleanTask_->bundleNames_.size(), 1);
        EXPECT_EQ(periodicCleanTask_->taskName_, "periodic_clean_task");

        std::system("rm -rf /data/service/el2/100/hmdfs/cache/account_cache/dentry_cache/cloud_temp_123");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PeriodicCleanTempDirTest001 ERROR.";
    }
    GTEST_LOG_(INFO) << "PeriodicCleanTempDirTest001 End";
}

/**
 * @tc.name: PeriodicCleanTempDirTest002
 * @tc.desc: Verify PeriodicCleanTempDir function.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicCleanTaskTest, PeriodicCleanTempDirTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PeriodicCleanTempDirTest002 Start.";
    try {
        std::system("mkdir -p /mnt/hmdfs/100/account/device_view/local/files/.cloud_cache/pread_cache_tmp_123");

        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillOnce(Return(true));
        EXPECT_CALL(*parameterMock_, GetParameter(_, _)).Times(0);

        int32_t userId = 100;
        int32_t ret = periodicCleanTask_->PeriodicCleanTempDir(userId);
        EXPECT_EQ(ret, E_STOP);
        EXPECT_EQ(periodicCleanTask_->runTaskForSwitchOff_, true);
        EXPECT_EQ(periodicCleanTask_->bundleNames_.size(), 1);
        EXPECT_EQ(periodicCleanTask_->taskName_, "periodic_clean_task");

        std::system("rm -rf /mnt/hmdfs/100/account/device_view/local/files/.cloud_cache/pread_cache_tmp_123");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PeriodicCleanTempDirTest002 ERROR.";
    }
    GTEST_LOG_(INFO) << "PeriodicCleanTempDirTest002 End";
}

/**
 * @tc.name: PeriodicCleanTempDirTest003
 * @tc.desc: Verify PeriodicCleanTempDir function.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicCleanTaskTest, PeriodicCleanTempDirTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PeriodicCleanTempDirTest003 Start.";
    try {
        std::system("mkdir -p /mnt/hmdfs/100/account/device_view/local/files/.cloud_cache/download_cache_tmp_123");

        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillOnce(Return(true));
        EXPECT_CALL(*parameterMock_, GetParameter(_, _)).Times(0);

        int32_t userId = 100;
        int32_t ret = periodicCleanTask_->PeriodicCleanTempDir(userId);
        EXPECT_EQ(ret, E_STOP);
        EXPECT_EQ(periodicCleanTask_->runTaskForSwitchOff_, true);
        EXPECT_EQ(periodicCleanTask_->bundleNames_.size(), 1);
        EXPECT_EQ(periodicCleanTask_->taskName_, "periodic_clean_task");

        std::system("rm -rf /mnt/hmdfs/100/account/device_view/local/files/.cloud_cache/download_cache_tmp_123");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PeriodicCleanTempDirTest003 ERROR.";
    }
    GTEST_LOG_(INFO) << "PeriodicCleanTempDirTest003 End";
}

/**
 * @tc.name: PeriodicCleanTempDirTest004
 * @tc.desc: Verify PeriodicCleanTempDir function.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicCleanTaskTest, PeriodicCleanTempDirTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PeriodicCleanTempDirTest004 Start.";
    try {
        std::system("mkdir -p /mnt/hmdfs/100/account/device_view/local/data/com.photos/.video_cache_tmp_123");

        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).Times(0);
        EXPECT_CALL(*parameterMock_, GetParameter(_, _)).WillOnce(Return(""));

        int32_t userId = 100;
        int32_t ret = periodicCleanTask_->PeriodicCleanTempDir(userId);
        EXPECT_EQ(ret, E_STOP);
        EXPECT_EQ(periodicCleanTask_->runTaskForSwitchOff_, true);
        EXPECT_EQ(periodicCleanTask_->bundleNames_.size(), 1);
        EXPECT_EQ(periodicCleanTask_->taskName_, "periodic_clean_task");

        std::system("rm -rf /mnt/hmdfs/100/account/device_view/local/data/com.photos/.video_cache_tmp_123");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PeriodicCleanTempDirTest004 ERROR.";
    }
    GTEST_LOG_(INFO) << "PeriodicCleanTempDirTest004 End";
}

/**
 * @tc.name: PeriodicCleanTempDirTest005
 * @tc.desc: Verify PeriodicCleanTempDir function.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicCleanTaskTest, PeriodicCleanTempDirTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PeriodicCleanTempDirTest005 Start.";
    try {
        std::system("mkdir -p /mnt/hmdfs/100/account/device_view/local/data/com.photos/.video_cache_tmp_123");

        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillOnce(Return(true));
        EXPECT_CALL(*parameterMock_, GetParameter(_, _)).WillOnce(Return("com.photos"));

        int32_t userId = 100;
        int32_t ret = periodicCleanTask_->PeriodicCleanTempDir(userId);
        EXPECT_EQ(ret, E_STOP);
        EXPECT_EQ(periodicCleanTask_->runTaskForSwitchOff_, true);
        EXPECT_EQ(periodicCleanTask_->bundleNames_.size(), 1);
        EXPECT_EQ(periodicCleanTask_->taskName_, "periodic_clean_task");

        std::system("rm -rf /mnt/hmdfs/100/account/device_view/local/data/com.photos/.video_cache_tmp_123");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PeriodicCleanTempDirTest005 ERROR.";
    }
    GTEST_LOG_(INFO) << "PeriodicCleanTempDirTest005 End";
}

/**
 * @tc.name: PeriodicCleanTempDirTest006
 * @tc.desc: Verify PeriodicCleanTempDir function.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicCleanTaskTest, PeriodicCleanTempDirTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PeriodicCleanTempDirTest006 Start.";
    try {
        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).Times(0);
        EXPECT_CALL(*parameterMock_, GetParameter(_, _)).WillOnce(Return("com.photos"));

        int32_t userId = 100;
        int32_t ret = periodicCleanTask_->PeriodicCleanTempDir(userId);
        EXPECT_EQ(ret, E_OK);
        EXPECT_EQ(periodicCleanTask_->runTaskForSwitchOff_, true);
        EXPECT_EQ(periodicCleanTask_->bundleNames_.size(), 1);
        EXPECT_EQ(periodicCleanTask_->taskName_, "periodic_clean_task");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PeriodicCleanTempDirTest006 ERROR.";
    }
    GTEST_LOG_(INFO) << "PeriodicCleanTempDirTest006 End";
}

/**
 * @tc.name: CleanTempDirTest001
 * @tc.desc: Verify CleanTempDir function.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicCleanTaskTest, CleanTempDirTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanTempDirTest001 Start.";
    try {
        const std::string dir = "/data/test_tdd";
        const std::string prefix = "tmp_test";
        int32_t ret = periodicCleanTask_->CleanTempDir(dir, prefix);
        EXPECT_EQ(ret, E_OK);
        EXPECT_EQ(periodicCleanTask_->runTaskForSwitchOff_, true);
        EXPECT_EQ(periodicCleanTask_->bundleNames_.size(), 1);
        EXPECT_EQ(periodicCleanTask_->taskName_, "periodic_clean_task");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CleanTempDirTest001 ERROR.";
    }
    GTEST_LOG_(INFO) << "CleanTempDirTest001 End";
}

/**
 * @tc.name: CleanTempDirTest002
 * @tc.desc: Verify CleanTempDir function.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicCleanTaskTest, CleanTempDirTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanTempDirTest002 Start.";
    try {
        const std::string dir = "/data/test_tdd";
        const std::string prefix = "";
        int32_t ret = periodicCleanTask_->CleanTempDir(dir, prefix);
        EXPECT_EQ(ret, E_OK);
        EXPECT_EQ(periodicCleanTask_->runTaskForSwitchOff_, true);
        EXPECT_EQ(periodicCleanTask_->bundleNames_.size(), 1);
        EXPECT_EQ(periodicCleanTask_->taskName_, "periodic_clean_task");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CleanTempDirTest002 ERROR.";
    }
    GTEST_LOG_(INFO) << "CleanTempDirTest002 End";
}

/**
 * @tc.name: CleanTempDirTest003
 * @tc.desc: Verify CleanTempDir function.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicCleanTaskTest, CleanTempDirTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanTempDirTest003 Start.";
    try {
        std::system("touch /data/test_tdd");
        const std::string dir = "/data/test_tdd";
        const std::string prefix = "tmp_test";
        int32_t ret = periodicCleanTask_->CleanTempDir(dir, prefix);
        EXPECT_EQ(ret, E_OK);
        EXPECT_EQ(periodicCleanTask_->runTaskForSwitchOff_, true);
        EXPECT_EQ(periodicCleanTask_->bundleNames_.size(), 1);
        EXPECT_EQ(periodicCleanTask_->taskName_, "periodic_clean_task");
        std::system("rm /data/test_tdd");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CleanTempDirTest003 ERROR.";
    }
    GTEST_LOG_(INFO) << "CleanTempDirTest003 End";
}

/**
 * @tc.name: CleanTempDirTest004
 * @tc.desc: Verify CleanTempDir function.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicCleanTaskTest, CleanTempDirTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanTempDirTest004 Start.";
    try {
        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillRepeatedly(Return(false));
        EXPECT_CALL(*systemFuncMock_, access(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*systemFuncMock_, remove(_)).WillRepeatedly(Return(0));

        const std::string dir = "/data/test_tdd";
        const std::string prefix = "tmp_test";
        std::system("mkdir /data/test_tdd");
        std::system("mkdir /data/test_tdd/tmp_test_123456");
        std::system("mkdir /data/test_tdd/test");
        std::system("touch /data/test_tdd/file1.txt");
        std::system("touch /data/test_tdd/tmp_test_123456/file2.txt");
        int32_t ret = periodicCleanTask_->CleanTempDir(dir, prefix);
        EXPECT_EQ(ret, E_OK);
        EXPECT_EQ(periodicCleanTask_->runTaskForSwitchOff_, true);
        EXPECT_EQ(periodicCleanTask_->bundleNames_.size(), 1);
        EXPECT_EQ(periodicCleanTask_->taskName_, "periodic_clean_task");
        std::system("rm -rf /data/test_tdd");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CleanTempDirTest004 ERROR.";
    }
    GTEST_LOG_(INFO) << "CleanTempDirTest004 End";
}

/**
 * @tc.name: CleanTempDirTest005
 * @tc.desc: Verify CleanTempDir function.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicCleanTaskTest, CleanTempDirTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanTempDirTest005 Start.";
    try {
        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillOnce(Return(true));
        EXPECT_CALL(*systemFuncMock_, access(_, _)).Times(0);

        const std::string dir = "/data/test_tdd";
        const std::string prefix = "tmp_test";
        std::system("mkdir /data/test_tdd");
        std::system("mkdir /data/test_tdd/tmp_test_123456");
        std::system("mkdir /data/test_tdd/test");
        std::system("touch /data/test_tdd/file1.txt");
        std::system("touch /data/test_tdd/tmp_test_123456/file2.txt");
        int32_t ret = periodicCleanTask_->CleanTempDir(dir, prefix);
        EXPECT_EQ(ret, E_STOP);
        EXPECT_EQ(periodicCleanTask_->runTaskForSwitchOff_, true);
        EXPECT_EQ(periodicCleanTask_->bundleNames_.size(), 1);
        EXPECT_EQ(periodicCleanTask_->taskName_, "periodic_clean_task");
        std::system("rm -rf /data/test_tdd");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CleanTempDirTest005 ERROR.";
    }
    GTEST_LOG_(INFO) << "CleanTempDirTest005 End";
}

/**
 * @tc.name: RemoveDirectoryByScreenOffTest001
 * @tc.desc: Verify CleanTempDir function.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicCleanTaskTest, RemoveDirectoryByScreenOffTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RemoveDirectoryByScreenOffTest001 Start.";
    try {
        std::system("mkdir -p /data/test_tdd");
        std::system("mkdir -p /data/test_tdd/test");

        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillOnce(Return(false)).WillOnce(Return(false))
            .WillOnce(Return(false)).WillOnce(Return(true));
        EXPECT_CALL(*systemFuncMock_, access(_, _)).Times(0);
        EXPECT_CALL(*systemFuncMock_, remove(_)).Times(0);

        std::string path = "/data/test_tdd";
        int32_t ret = periodicCleanTask_->RemoveDirectoryByScreenOff(path);
        EXPECT_EQ(ret, E_STOP);
        EXPECT_EQ(periodicCleanTask_->runTaskForSwitchOff_, true);
        EXPECT_EQ(periodicCleanTask_->bundleNames_.size(), 1);
        EXPECT_EQ(periodicCleanTask_->taskName_, "periodic_clean_task");

        std::system("rm -rf /data/test_tdd");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RemoveDirectoryByScreenOffTest001 ERROR.";
    }
    GTEST_LOG_(INFO) << "RemoveDirectoryByScreenOffTest001 End";
}

/**
 * @tc.name: RemoveDirectoryByScreenOffTest002
 * @tc.desc: Verify CleanTempDir function.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicCleanTaskTest, RemoveDirectoryByScreenOffTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RemoveDirectoryByScreenOffTest002 Start.";
    try {
        std::system("mkdir -p /data/test_tdd");
        std::system("mkdir -p /data/test_tdd/test");
        std::system("touch /data/test_tdd/test/file1.txt");
        std::system("touch /data/test_tdd/test/file2.txt");
        std::system("touch /data/test_tdd/test/file3.txt");

        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillRepeatedly(Return(false));
        EXPECT_CALL(*systemFuncMock_, access(_, _)).WillOnce(Return(-1)).WillOnce(Return(0)).WillOnce(Return(0))
            .WillOnce(Return(0)).WillOnce(Return(-1));
        EXPECT_CALL(*systemFuncMock_, remove(_)).WillOnce(Return(0)).WillOnce(Return(-1)).WillOnce(Return(0));

        std::string path = "/data/test_tdd";
        int32_t ret = periodicCleanTask_->RemoveDirectoryByScreenOff(path);
        EXPECT_EQ(ret, E_DELETE_FAILED);
        EXPECT_EQ(periodicCleanTask_->runTaskForSwitchOff_, true);
        EXPECT_EQ(periodicCleanTask_->bundleNames_.size(), 1);
        EXPECT_EQ(periodicCleanTask_->taskName_, "periodic_clean_task");

        std::system("rm -rf /data/test_tdd");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RemoveDirectoryByScreenOffTest002 ERROR.";
    }
    GTEST_LOG_(INFO) << "RemoveDirectoryByScreenOffTest002 End";
}

/**
 * @tc.name: RemoveDirectoryByScreenOffTest003
 * @tc.desc: Verify CleanTempDir function.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicCleanTaskTest, RemoveDirectoryByScreenOffTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RemoveDirectoryByScreenOffTest003 Start.";
    try {
        std::system("mkdir -p /data/test_tdd");
        std::system("mkdir -p /data/test_tdd/test");
        std::system("touch /data/test_tdd/test/file1.txt");
        std::system("touch /data/test_tdd/test/file2.txt");
        std::system("touch /data/test_tdd/test/file3.txt");

        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillRepeatedly(Return(false));
        EXPECT_CALL(*systemFuncMock_, access(_, _)).WillOnce(Return(-1)).WillOnce(Return(0)).WillOnce(Return(0))
            .WillOnce(Return(0)).WillOnce(Return(-1)).WillOnce(Return(0));
        EXPECT_CALL(*systemFuncMock_, remove(_)).WillOnce(Return(0)).WillOnce(Return(0)).WillOnce(Return(0))
            .WillOnce(Return(-1));

        std::string path = "/data/test_tdd";
        int32_t ret = periodicCleanTask_->RemoveDirectoryByScreenOff(path);
        EXPECT_EQ(ret, E_DELETE_FAILED);
        EXPECT_EQ(periodicCleanTask_->runTaskForSwitchOff_, true);
        EXPECT_EQ(periodicCleanTask_->bundleNames_.size(), 1);
        EXPECT_EQ(periodicCleanTask_->taskName_, "periodic_clean_task");

        std::system("rm -rf /data/test_tdd");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RemoveDirectoryByScreenOffTest003 ERROR.";
    }
    GTEST_LOG_(INFO) << "RemoveDirectoryByScreenOffTest003 End";
}

/**
 * @tc.name: RemoveDirectoryByScreenOffTest004
 * @tc.desc: Verify CleanTempDir function.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicCleanTaskTest, RemoveDirectoryByScreenOffTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RemoveDirectoryByScreenOffTest004 Start.";
    try {
        std::system("mkdir -p /data/test_tdd");
        std::system("mkdir -p /data/test_tdd/test");
        std::system("touch /data/test_tdd/test/file1.txt");
        std::system("touch /data/test_tdd/test/file2.txt");
        std::system("touch /data/test_tdd/test/file3.txt");

        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillRepeatedly(Return(false));
        EXPECT_CALL(*systemFuncMock_, access(_, _)).WillOnce(Return(-1)).WillOnce(Return(0)).WillOnce(Return(0))
            .WillOnce(Return(0)).WillOnce(Return(-1)).WillOnce(Return(0)).WillOnce(Return(0));
        EXPECT_CALL(*systemFuncMock_, remove(_)).WillOnce(Return(0)).WillOnce(Return(0)).WillOnce(Return(0))
            .WillOnce(Return(0));

        std::string path = "/data/test_tdd";
        int32_t ret = periodicCleanTask_->RemoveDirectoryByScreenOff(path);
        EXPECT_EQ(ret, E_DELETE_FAILED);
        EXPECT_EQ(periodicCleanTask_->runTaskForSwitchOff_, true);
        EXPECT_EQ(periodicCleanTask_->bundleNames_.size(), 1);
        EXPECT_EQ(periodicCleanTask_->taskName_, "periodic_clean_task");

        std::system("rm -rf /data/test_tdd");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RemoveDirectoryByScreenOffTest004 ERROR.";
    }
    GTEST_LOG_(INFO) << "RemoveDirectoryByScreenOffTest004 End";
}

/**
 * @tc.name: RemoveDirectoryByScreenOffTest005
 * @tc.desc: Verify CleanTempDir function.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicCleanTaskTest, RemoveDirectoryByScreenOffTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RemoveDirectoryByScreenOffTest005 Start.";
    try {
        std::system("mkdir -p /data/test_tdd");
        std::system("mkdir -p /data/test_tdd/test");
        std::system("touch /data/test_tdd/test/file1.txt");
        std::system("touch /data/test_tdd/test/file2.txt");
        std::system("touch /data/test_tdd/test/file3.txt");

        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillRepeatedly(Return(false));
        EXPECT_CALL(*systemFuncMock_, access(_, _)).WillOnce(Return(-1)).WillOnce(Return(0)).WillOnce(Return(0))
            .WillOnce(Return(0)).WillOnce(Return(-1)).WillOnce(Return(0)).WillOnce(Return(-1));
        EXPECT_CALL(*systemFuncMock_, remove(_)).WillOnce(Return(0)).WillOnce(Return(0)).WillOnce(Return(0))
            .WillOnce(Return(0));

        std::string path = "/data/test_tdd";
        int32_t ret = periodicCleanTask_->RemoveDirectoryByScreenOff(path);
        EXPECT_EQ(ret, E_OK);
        EXPECT_EQ(periodicCleanTask_->runTaskForSwitchOff_, true);
        EXPECT_EQ(periodicCleanTask_->bundleNames_.size(), 1);
        EXPECT_EQ(periodicCleanTask_->taskName_, "periodic_clean_task");

        std::system("rm -rf /data/test_tdd");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RemoveDirectoryByScreenOffTest005 ERROR.";
    }
    GTEST_LOG_(INFO) << "RemoveDirectoryByScreenOffTest005 End";
}

/**
 * @tc.name: RemoveDirectoryByScreenOffTest006
 * @tc.desc: Verify CleanTempDir function.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicCleanTaskTest, RemoveDirectoryByScreenOffTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RemoveDirectoryByScreenOffTest006 Start.";
    try {
        std::system("mkdir -p /data/test_tdd");
        std::system("mkdir -p /data/test_tdd/test");
        std::system("touch /data/test_tdd/test/file1.txt");
        std::system("touch /data/test_tdd/test/file2.txt");
        std::system("touch /data/test_tdd/test/file3.txt");

        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillRepeatedly(Return(false));
        EXPECT_CALL(*systemFuncMock_, access(_, _)).WillOnce(Return(-1)).WillOnce(Return(0)).WillOnce(Return(0))
            .WillOnce(Return(0)).WillOnce(Return(-1)).WillOnce(Return(-1)).WillOnce(Return(0));
        EXPECT_CALL(*systemFuncMock_, remove(_)).WillOnce(Return(0)).WillOnce(Return(0)).WillOnce(Return(0));

        std::string path = "/data/test_tdd";
        int32_t ret = periodicCleanTask_->RemoveDirectoryByScreenOff(path);
        EXPECT_EQ(ret, E_DELETE_FAILED);
        EXPECT_EQ(periodicCleanTask_->runTaskForSwitchOff_, true);
        EXPECT_EQ(periodicCleanTask_->bundleNames_.size(), 1);
        EXPECT_EQ(periodicCleanTask_->taskName_, "periodic_clean_task");

        std::system("rm -rf /data/test_tdd");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RemoveDirectoryByScreenOffTest006 ERROR.";
    }
    GTEST_LOG_(INFO) << "RemoveDirectoryByScreenOffTest006 End";
}

/**
 * @tc.name: RemoveDirectoryByScreenOffTest007
 * @tc.desc: Verify CleanTempDir function.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicCleanTaskTest, RemoveDirectoryByScreenOffTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RemoveDirectoryByScreenOffTest007 Start.";
    try {
        std::system("mkdir -p /data/test_tdd");
        std::system("mkdir -p /data/test_tdd/test");
        std::system("touch /data/test_tdd/test/file1.txt");
        std::system("touch /data/test_tdd/test/file2.txt");
        std::system("touch /data/test_tdd/test/file3.txt");

        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillOnce(Return(true));
        EXPECT_CALL(*systemFuncMock_, access(_, _)).Times(0);
        EXPECT_CALL(*systemFuncMock_, remove(_)).Times(0);

        std::string path = "/data/test_tdd";
        int32_t ret = periodicCleanTask_->RemoveDirectoryByScreenOff(path);
        EXPECT_EQ(ret, E_STOP);
        EXPECT_EQ(periodicCleanTask_->runTaskForSwitchOff_, true);
        EXPECT_EQ(periodicCleanTask_->bundleNames_.size(), 1);
        EXPECT_EQ(periodicCleanTask_->taskName_, "periodic_clean_task");

        std::system("rm -rf /data/test_tdd");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RemoveDirectoryByScreenOffTest007 ERROR.";
    }
    GTEST_LOG_(INFO) << "RemoveDirectoryByScreenOffTest007 End";
}
}