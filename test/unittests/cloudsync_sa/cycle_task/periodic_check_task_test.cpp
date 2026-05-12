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

#include "battery_status.h"
#include "dfs_error.h"
#include "periodic_check_task.h"
#include "screen_status.h"
#include "screen_status_mock.h"
#include "system_load.h"

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class PeriodicCheckTaskTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<CloudFile::DataSyncManager> dataSyncManagerPtr_ = nullptr;
    static inline std::shared_ptr<PeriodicCheckTask> periodicCheckTask_ = nullptr;
    static inline std::shared_ptr<ScreenStatusMock> screenStatusMock_ = nullptr;
};

void PeriodicCheckTaskTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    dataSyncManagerPtr_ = std::make_shared<CloudFile::DataSyncManager>();
    periodicCheckTask_ = std::make_shared<PeriodicCheckTask>(dataSyncManagerPtr_);
    screenStatusMock_ = std::make_shared<ScreenStatusMock>();
    ScreenStatusMock::proxy_ = screenStatusMock_;
}

void PeriodicCheckTaskTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    dataSyncManagerPtr_ = nullptr;
    periodicCheckTask_ = nullptr;
    screenStatusMock_ = nullptr;
    ScreenStatusMock::proxy_ = nullptr;
}

void PeriodicCheckTaskTest::SetUp(void)
{
}

void PeriodicCheckTaskTest::TearDown(void)
{
}

/**
 * @tc.name: RunTaskForBundleTest001
 * @tc.desc: RunTaskForBundle calls UpdateCachedFileSize and TriggerStartSync successfully
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(PeriodicCheckTaskTest, RunTaskForBundleTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskForBundleTest001 Start";
    try {
        int32_t userId = 100;
        std::string bundleName = "com.ohos.test101";

        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillOnce(Return(false));
        BatteryStatus::SetChargingStatus(true);
        SystemLoadStatus::Setload(PowerMgr::ThermalLevel::COOL);

        int32_t ret = periodicCheckTask_->RunTaskForBundle(userId, bundleName);
        EXPECT_EQ(ret, E_OK);
        EXPECT_EQ(periodicCheckTask_->taskName_, "periodic_check_task");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "RunTaskForBundleTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskForBundleTest001 End";
}

/**
 * @tc.name: RunTaskForBundleTest002
 * @tc.desc: RunTaskForBundle stops when screen is on
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(PeriodicCheckTaskTest, RunTaskForBundleTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskForBundleTest002 Start";
    try {
        int32_t userId = 100;
        std::string bundleName = "com.ohos.test102";

        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillOnce(Return(true));
        BatteryStatus::SetChargingStatus(true);
        SystemLoadStatus::Setload(PowerMgr::ThermalLevel::COOL);

        int32_t ret = periodicCheckTask_->RunTaskForBundle(userId, bundleName);
        EXPECT_EQ(ret, E_STOP);
        EXPECT_EQ(periodicCheckTask_->taskName_, "periodic_check_task");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "RunTaskForBundleTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskForBundleTest002 End";
}

/**
 * @tc.name: RunTaskForBundleTest003
 * @tc.desc: RunTaskForBundle stops when battery not charging
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(PeriodicCheckTaskTest, RunTaskForBundleTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskForBundleTest003 Start";
    try {
        int32_t userId = 100;
        std::string bundleName = "com.ohos.test103";

        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillOnce(Return(false));
        BatteryStatus::SetChargingStatus(false);
        SystemLoadStatus::Setload(PowerMgr::ThermalLevel::COOL);

        int32_t ret = periodicCheckTask_->RunTaskForBundle(userId, bundleName);
        EXPECT_EQ(ret, E_STOP);
        EXPECT_EQ(periodicCheckTask_->taskName_, "periodic_check_task");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "RunTaskForBundleTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskForBundleTest003 End";
}

/**
 * @tc.name: RunTaskForBundleTest004
 * @tc.desc: RunTaskForBundle stops when system load not allowed
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(PeriodicCheckTaskTest, RunTaskForBundleTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskForBundleTest004 Start";
    try {
        int32_t userId = 100;
        std::string bundleName = "com.ohos.test104";

        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillOnce(Return(false));
        BatteryStatus::SetChargingStatus(true);
        SystemLoadStatus::Setload(PowerMgr::ThermalLevel::EMERGENCY);

        int32_t ret = periodicCheckTask_->RunTaskForBundle(userId, bundleName);
        EXPECT_EQ(ret, E_STOP);
        EXPECT_EQ(periodicCheckTask_->taskName_, "periodic_check_task");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "RunTaskForBundleTest004 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskForBundleTest004 End";
}

/**
 * @tc.name: RunTaskForBundleTest005
 * @tc.desc: RunTaskForBundle with different userId values
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(PeriodicCheckTaskTest, RunTaskForBundleTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskForBundleTest005 Start";
    try {
        int32_t userId = 0;
        std::string bundleName = "com.ohos.test105";

        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillOnce(Return(false));
        BatteryStatus::SetChargingStatus(true);
        SystemLoadStatus::Setload(PowerMgr::ThermalLevel::COOL);

        int32_t ret = periodicCheckTask_->RunTaskForBundle(userId, bundleName);
        EXPECT_EQ(ret, E_OK);
        EXPECT_EQ(periodicCheckTask_->taskName_, "periodic_check_task");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "RunTaskForBundleTest005 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskForBundleTest005 End";
}

/**
 * @tc.name: RunTaskForBundleTest006
 * @tc.desc: RunTaskForBundle with different bundleName
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(PeriodicCheckTaskTest, RunTaskForBundleTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskForBundleTest006 Start";
    try {
        int32_t userId = 200;
        std::string bundleName = "com.ohos.gallery";

        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillOnce(Return(false));
        BatteryStatus::SetChargingStatus(true);
        SystemLoadStatus::Setload(PowerMgr::ThermalLevel::COOL);

        int32_t ret = periodicCheckTask_->RunTaskForBundle(userId, bundleName);
        EXPECT_EQ(ret, E_OK);
        EXPECT_EQ(periodicCheckTask_->taskName_, "periodic_check_task");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "RunTaskForBundleTest006 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskForBundleTest006 End";
}

/**
 * @tc.name: RunTaskForBundleTest007
 * @tc.desc: RunTaskForBundle with valid conditions and empty bundleName
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(PeriodicCheckTaskTest, RunTaskForBundleTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskForBundleTest007 Start";
    try {
        int32_t userId = 100;
        std::string bundleName = "";

        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillOnce(Return(false));
        BatteryStatus::SetChargingStatus(true);
        SystemLoadStatus::Setload(PowerMgr::ThermalLevel::COOL);

        int32_t ret = periodicCheckTask_->RunTaskForBundle(userId, bundleName);
        EXPECT_EQ(ret, E_OK);
        EXPECT_EQ(periodicCheckTask_->taskName_, "periodic_check_task");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "RunTaskForBundleTest007 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskForBundleTest007 End";
}

/**
 * @tc.name: RunTaskForBundleTest008
 * @tc.desc: RunTaskForBundle with screen status changing
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(PeriodicCheckTaskTest, RunTaskForBundleTest008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskForBundleTest008 Start";
    try {
        int32_t userId = 100;
        std::string bundleName = "com.ohos.test108";

        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillOnce(Return(false));
        BatteryStatus::SetChargingStatus(true);
        SystemLoadStatus::Setload(PowerMgr::ThermalLevel::COOL);

        int32_t ret = periodicCheckTask_->RunTaskForBundle(userId, bundleName);
        EXPECT_EQ(ret, E_OK);

        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillOnce(Return(true));
        BatteryStatus::SetChargingStatus(true);
        SystemLoadStatus::Setload(PowerMgr::ThermalLevel::COOL);

        ret = periodicCheckTask_->RunTaskForBundle(userId, bundleName);
        EXPECT_EQ(ret, E_STOP);
        EXPECT_EQ(periodicCheckTask_->taskName_, "periodic_check_task");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "RunTaskForBundleTest008 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskForBundleTest008 End";
}

/**
 * @tc.name: RunTaskForBundleTest009
 * @tc.desc: RunTaskForBundle with screen status changing
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(PeriodicCheckTaskTest, RunTaskForBundleTest009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskForBundleTest009 Start";
    try {
        int32_t userId = 2;
        std::string bundleName = "com.ohos.test108";

        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillOnce(Return(false));
        BatteryStatus::SetChargingStatus(true);
        SystemLoadStatus::Setload(PowerMgr::ThermalLevel::COOL);

        auto ret = periodicCheckTask_->RunTaskForBundle(userId, bundleName);
        EXPECT_EQ(ret, 1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "RunTaskForBundleTest009 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskForBundleTest009 End";
}

/**
 * @tc.name: RunTaskForBundleTest010
 * @tc.desc: RunTaskForBundle with screen status changing
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(PeriodicCheckTaskTest, RunTaskForBundleTest010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskForBundleTest010 Start";
    try {
        int32_t userId = 3;
        std::string bundleName = "com.ohos.test108";

        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillOnce(Return(false));
        BatteryStatus::SetChargingStatus(true);
        SystemLoadStatus::Setload(PowerMgr::ThermalLevel::COOL);

        auto ret = periodicCheckTask_->RunTaskForBundle(userId, bundleName);
        EXPECT_EQ(ret, 1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "RunTaskForBundleTest010 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskForBundleTest010 End";
}

} // namespace OHOS::FileManagement::CloudSync::Test