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

#include "battery_status.h"
#include "system_load.h"
#include "dfs_error.h"
#include "ffrt_inner.h"
#include "parameters.h"
#include "utils_log.h"
#include "task_state_manager.h"

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class SytemLoadTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
#ifdef support_thermal_manager
    shared_ptr<SystemLoadStatus> systemLoadStatus_ = nullptr;
    shared_ptr<SystemLoadEvent> systemLoadEvent_ = nullptr;
#endif
};

void SytemLoadTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void SytemLoadTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void SytemLoadTest::SetUp(void)
{
#ifdef support_thermal_manager
    GTEST_LOG_(INFO) << "SetUp";
    systemLoadStatus_ = make_shared<SystemLoadStatus>();
    systemLoadEvent_ = make_shared<SystemLoadEvent>();
    systemLoadStatus_->thermalEvent_ = systemLoadEvent_;
#endif
}

void SytemLoadTest::TearDown(void)
{
#ifdef support_thermal_manager
    GTEST_LOG_(INFO) << "TearDown";
    TaskStateManager::GetInstance().CancelUnloadTask();
    systemLoadStatus_ = nullptr;
    systemLoadEvent_ = nullptr;
#endif
}

#ifdef support_thermal_manager
/**
 * @tc.name: RegisterSystemloadCallbackTest001
 * @tc.desc: Verify the RegisterSystemloadCallback function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SytemLoadTest, RegisterSystemloadCallbackTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterSystemloadCallbackTest Start";
    try {
        std::shared_ptr<CloudFile::DataSyncManager> dataSyncManager;
        systemLoadStatus_->RegisterSystemloadCallback(dataSyncManager);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RegisterSystemloadCallbackTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "RegisterSystemloadCallbackTest001 End";
}

/**
 * @tc.name: RegisterSystemloadCallbackTest002
 * @tc.desc: Verify the RegisterSystemloadCallback function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SytemLoadTest, RegisterSystemloadCallbackTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterSystemloadCallbackTest002 Start";
    try {
        systemLoadStatus_->thermalEvent_ = nullptr;
        std::shared_ptr<CloudFile::DataSyncManager> dataSyncManager;
        systemLoadStatus_->RegisterSystemloadCallback(dataSyncManager);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RegisterSystemloadCallbackTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "RegisterSystemloadCallbackTest002 End";
}

/**
 * @tc.name: OnThermalLevelResultTest001
 * @tc.desc: Verify the OnThermalLevelResult function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SytemLoadTest, OnThermalLevelResultTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnThermalLevelResultTest001 Start";
    try {
        systemLoadEvent_->OnThermalLevelResult(PowerMgr::ThermalLevel::HOT);
        ffrt::wait();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnThermalLevelResultTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "OnThermalLevelResultTest001 End";
}

/**
 * @tc.name: OnThermalLevelResultTest002
 * @tc.desc: Verify the OnThermalLevelResult function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SytemLoadTest, OnThermalLevelResultTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnThermalLevelResultTest002 Start";
    try {
        std::shared_ptr<CloudFile::DataSyncManager> dataSyncManager;
        systemLoadEvent_->dataSyncManager_ = make_shared<CloudFile::DataSyncManager>();
        systemLoadEvent_->OnThermalLevelResult(PowerMgr::ThermalLevel::NORMAL);
        ffrt::wait();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnThermalLevelResultTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "OnThermalLevelResultTest002 End";
}

/**
 * @tc.name: OnThermalLevelResultTest003
 * @tc.desc: Verify the OnThermalLevelResult function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SytemLoadTest, OnThermalLevelResultTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnThermalLevelResultTest003 Start";
    try {
        system::SetParameter(TEMPERATURE_SYSPARAM_SYNC, "true");
        auto dataSyncManager = std::make_shared<CloudFile::DataSyncManager>();
        systemLoadEvent_->SetDataSyncer(dataSyncManager);
        systemLoadEvent_->OnThermalLevelResult(PowerMgr::ThermalLevel::NORMAL);
        ffrt::wait();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnThermalLevelResultTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "OnThermalLevelResultTest003 End";
}

/**
 * @tc.name: OnThermalLevelResultTest004
 * @tc.desc: 充电状态下，NORMAL档位时恢复缩略图下载
 * @tc.type: FUNC
 * @tc.require: ICR23A
 */
HWTEST_F(SytemLoadTest, OnThermalLevelResultTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnThermalLevelResultTest004 Start";
    try {
        system::SetParameter(TEMPERATURE_SYSPARAM_THUMB, "true");
        BatteryStatus::SetChargingStatus(true);
        auto dataSyncManager = std::make_shared<CloudFile::DataSyncManager>();
        systemLoadEvent_->SetDataSyncer(dataSyncManager);
        systemLoadEvent_->OnThermalLevelResult(PowerMgr::ThermalLevel::NORMAL);
        ffrt::wait();
        std::string systemLoadThumb = system::GetParameter(TEMPERATURE_SYSPARAM_THUMB, "");
        EXPECT_EQ("false", systemLoadThumb);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnThermalLevelResultTest004 FAILED";
    }
    GTEST_LOG_(INFO) << "OnThermalLevelResultTest004 End";
}

/**
 * @tc.name: OnThermalLevelResultTest005
 * @tc.desc: 充电状态下，warm档位时不恢复缩略图下载
 * @tc.type: FUNC
 * @tc.require: ICR23A
 */
HWTEST_F(SytemLoadTest, OnThermalLevelResultTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnThermalLevelResultTest005 Start";
    try {
        system::SetParameter(TEMPERATURE_SYSPARAM_THUMB, "true");
        BatteryStatus::SetChargingStatus(true);
        auto dataSyncManager = std::make_shared<CloudFile::DataSyncManager>();
        systemLoadEvent_->SetDataSyncer(dataSyncManager);
        systemLoadEvent_->OnThermalLevelResult(PowerMgr::ThermalLevel::HOT);
        ffrt::wait();
        std::string systemLoadThumb = system::GetParameter(TEMPERATURE_SYSPARAM_THUMB, "");
        EXPECT_EQ("true", systemLoadThumb);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnThermalLevelResultTest005 FAILED";
    }
    GTEST_LOG_(INFO) << "OnThermalLevelResultTest005 End";
}

/**
 * @tc.name: OnThermalLevelResultTest006
 * @tc.desc: 不充电状态下，normal档位时恢复缩略图下载
 * @tc.type: FUNC
 * @tc.require: ICR23A
 */
HWTEST_F(SytemLoadTest, OnThermalLevelResultTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnThermalLevelResultTest006 Start";
    try {
        system::SetParameter(TEMPERATURE_SYSPARAM_THUMB, "true");
        BatteryStatus::SetChargingStatus(false);
        auto dataSyncManager = std::make_shared<CloudFile::DataSyncManager>();
        systemLoadEvent_->SetDataSyncer(dataSyncManager);
        systemLoadEvent_->OnThermalLevelResult(PowerMgr::ThermalLevel::NORMAL);
        ffrt::wait();
        std::string systemLoadThumb = system::GetParameter(TEMPERATURE_SYSPARAM_THUMB, "");
        EXPECT_EQ("false", systemLoadThumb);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnThermalLevelResultTest006 FAILED";
    }
    GTEST_LOG_(INFO) << "OnThermalLevelResultTest006 End";
}

/**
 * @tc.name: OnThermalLevelResultTest007
 * @tc.desc: 不充电状态下，cool档位时恢复缩略图下载
 * @tc.type: FUNC
 * @tc.require: ICR23A
 */
HWTEST_F(SytemLoadTest, OnThermalLevelResultTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnThermalLevelResultTest007 Start";
    try {
        system::SetParameter(TEMPERATURE_SYSPARAM_THUMB, "true");
        BatteryStatus::SetChargingStatus(false);
        auto dataSyncManager = std::make_shared<CloudFile::DataSyncManager>();
        systemLoadEvent_->SetDataSyncer(dataSyncManager);
        systemLoadEvent_->OnThermalLevelResult(PowerMgr::ThermalLevel::COOL);
        ffrt::wait();
        std::string systemLoadThumb = system::GetParameter(TEMPERATURE_SYSPARAM_THUMB, "");
        EXPECT_EQ("false", systemLoadThumb);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnThermalLevelResultTest007 FAILED";
    }
    GTEST_LOG_(INFO) << "OnThermalLevelResultTest007 End";
}

/**
 * @tc.name: OnThermalLevelResultTest008
 * @tc.desc: Verify the OnThermalLevelResult function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SytemLoadTest, OnThermalLevelResultTest008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnThermalLevelResultTest008 Start";
    try {
        system::SetParameter(TEMPERATURE_SYSPARAM_SYNC, "true");
        auto dataSyncManager = std::make_shared<CloudFile::DataSyncManager>();
        systemLoadEvent_->SetDataSyncer(dataSyncManager);
        system::SetParameter(TEMPERATURE_SYSPARAM_SYNC, "true");
        systemLoadEvent_->OnThermalLevelResult(PowerMgr::ThermalLevel::NORMAL);
        ffrt::wait();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnThermalLevelResultTest008 FAILED";
    }
    GTEST_LOG_(INFO) << "OnThermalLevelResultTest008 End";
}

/**
 * @tc.name: OnThermalLevelResultTest009
 * @tc.desc: Verify the OnThermalLevelResult function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SytemLoadTest, OnThermalLevelResultTest009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnThermalLevelResultTest009 Start";
    try {
        auto dataSyncManager = std::make_shared<CloudFile::DataSyncManager>();
        systemLoadEvent_->SetDataSyncer(dataSyncManager);
        systemLoadEvent_->OnThermalLevelResult(PowerMgr::ThermalLevel::HOT);
        ffrt::wait();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnThermalLevelResultTest009 FAILED";
    }
    GTEST_LOG_(INFO) << "OnThermalLevelResultTest009 End";
}

/**
 * @tc.name: OnThermalLevelResultTest0010
 * @tc.desc: Verify the OnThermalLevelResult function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SytemLoadTest, OnThermalLevelResultTest0010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnThermalLevelResultTest0010 Start";
    try {
        systemLoadEvent_->SetDataSyncer(nullptr);
        systemLoadEvent_->OnThermalLevelResult(PowerMgr::ThermalLevel::NORMAL);
        ffrt::wait();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnThermalLevelResultTest0010 FAILED";
    }
    GTEST_LOG_(INFO) << "OnThermalLevelResultTest0010 End";
}

/**
 * @tc.name: OnThermalLevelResultTest0011
 * @tc.desc: Verify the OnThermalLevelResult function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SytemLoadTest, OnThermalLevelResultTest0011, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnThermalLevelResultTest0011 Start";
    try {
        auto dataSyncManager = std::make_shared<CloudFile::DataSyncManager>();
        systemLoadEvent_->SetDataSyncer(dataSyncManager);
        systemLoadEvent_->OnThermalLevelResult(PowerMgr::ThermalLevel::NORMAL);
        ffrt::wait();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnThermalLevelResultTest0011 FAILED";
    }
    GTEST_LOG_(INFO) << "OnThermalLevelResultTest0011 End";
}

/**
 * @tc.name: IsSystemLoadAllowedTest001
 * @tc.desc: Verify the IsSystemLoadAllowed function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SytemLoadTest, IsSystemLoadAllowedTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsSystemLoadAllowedTest001 Start";
    try {
        systemLoadStatus_->Setload(PowerMgr::ThermalLevel::OVERHEATED);
    bool ret = systemLoadStatus_->IsSystemLoadAllowed();
        EXPECT_EQ(ret, false);
    } catch (...) {
        EXPECT_TRUE(false);
    GTEST_LOG_(INFO) << "IsSystemLoadAllowedTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "IsSystemLoadAllowedTest001 End";
}

/**
 * @tc.name: IsSystemLoadAllowedTest002
 * @tc.desc: Verify the IsSystemLoadAllowed function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SytemLoadTest, IsSystemLoadAllowedTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsSystemLoadAllowedTest002 Start";
    try {
        systemLoadStatus_->Setload(PowerMgr::ThermalLevel::WARNING);
    bool ret = systemLoadStatus_->IsSystemLoadAllowed();
        EXPECT_EQ(ret, false);
    } catch (...) {
        EXPECT_TRUE(false);
    GTEST_LOG_(INFO) << "IsSystemLoadAllowedTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "IsSystemLoadAllowedTest002 End";
}

/**
 * @tc.name: IsSystemLoadAllowedTest003
 * @tc.desc: Verify the IsSystemLoadAllowed function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SytemLoadTest, IsSystemLoadAllowedTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsSystemLoadAllowedTest003 Start";
    try {
        systemLoadStatus_->Setload(PowerMgr::ThermalLevel::NORMAL);
    bool ret = systemLoadStatus_->IsSystemLoadAllowed();
        EXPECT_EQ(ret, true);
    } catch (...) {
        EXPECT_TRUE(false);
    GTEST_LOG_(INFO) << "IsSystemLoadAllowedTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "IsSystemLoadAllowedTest003 End";
}

/**
 * @tc.name: IsSystemLoadAllowedTest004
 * @tc.desc: Verify the IsSystemLoadAllowed function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SytemLoadTest, IsSystemLoadAllowedTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsSystemLoadAllowedTest004 Start";
    systemLoadStatus_->Setload(PowerMgr::ThermalLevel::OVERHEATED);
    STOPPED_TYPE process = STOPPED_IN_THUMB;
    bool ret = systemLoadStatus_->IsSystemLoadAllowed(process);
    EXPECT_EQ(ret, false);
    GTEST_LOG_(INFO) << "IsSystemLoadAllowedTest004 End";
}

/**
 * @tc.name: IsSystemLoadAllowedTest005
 * @tc.desc: Verify the IsSystemLoadAllowed function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SytemLoadTest, IsSystemLoadAllowedTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsSystemLoadAllowedTest005 Start";
    systemLoadStatus_->Setload(PowerMgr::ThermalLevel::OVERHEATED);
    STOPPED_TYPE process = STOPPED_IN_SYNC;
    bool ret = systemLoadStatus_->IsSystemLoadAllowed(process);
    EXPECT_EQ(ret, false);
    GTEST_LOG_(INFO) << "IsSystemLoadAllowedTest005 End";
}

/**
 * @tc.name: IsSystemLoadAllowedTest006
 * @tc.desc: Verify the IsSystemLoadAllowed function
 * @tc.type: FUNC
 * @tc.require: ICR23A
 */
HWTEST_F(SytemLoadTest, IsSystemLoadAllowedTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsSystemLoadAllowedTest006 Start";
    systemLoadStatus_->Setload(PowerMgr::ThermalLevel::HOT);
    STOPPED_TYPE process = STOPPED_IN_SYNC;
    bool ret = systemLoadStatus_->IsSystemLoadAllowed(process, PowerMgr::ThermalLevel::WARM);
    EXPECT_EQ(ret, false);
    GTEST_LOG_(INFO) << "IsSystemLoadAllowedTest006 End";
}

/**
 * @tc.name: GetSystemloadLevel
 * @tc.desc: Verify the GetSystemloadLevel function
 * @tc.type: FUNC
 * @tc.require: ICR23A
 */
HWTEST_F(SytemLoadTest, GetSystemloadLevel, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSystemloadLevel Start";
    try {
        systemLoadStatus_->GetSystemloadLevel();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetSystemloadLevel FAILED";
    }
    GTEST_LOG_(INFO) << "GetSystemloadLevel End";
}

/**
 * @tc.name: SetloadAndGetloadTest
 * @tc.desc: Verify the SetloadAndGetloadTest function
 * @tc.type: FUNC
 * @tc.require: ICR23A
 */
HWTEST_F(SytemLoadTest, SetloadAndGetloadTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetloadAndGetloadTest Start";
    try {
        systemLoadStatus_->Setload(PowerMgr::ThermalLevel::COOL);
        EXPECT_EQ(systemLoadStatus_->Getload(), PowerMgr::ThermalLevel::COOL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetloadAndGetloadTest FAILED";
    }
    GTEST_LOG_(INFO) << "SetloadAndGetloadTest End";
}

/**
 * @tc.name: SetDataSyncerTest
 * @tc.desc: Verify the SetDataSyncerTest function
 * @tc.type: FUNC
 * @tc.require: ICR23A
 */
HWTEST_F(SytemLoadTest, SetDataSyncerTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetDataSyncerTest Start";
    try {
        auto dataSyncManager = std::make_shared<CloudFile::DataSyncManager>();
        systemLoadEvent_->SetDataSyncer(dataSyncManager);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetDataSyncerTest FAILED";
    }
    GTEST_LOG_(INFO) << "SetDataSyncerTest End";
}

/**
 * @tc.name: InitSystemloadTest
 * @tc.desc: Verify the InitSystemloadTest function
 * @tc.type: FUNC
 * @tc.require: ICR23A
 */
HWTEST_F(SytemLoadTest, InitSystemloadTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InitSystemloadTest Start";
    try {
        auto dataSyncManager = std::make_shared<CloudFile::DataSyncManager>();
        systemLoadStatus_->InitSystemload(dataSyncManager);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "InitSystemloadTest FAILED";
    }
    GTEST_LOG_(INFO) << "InitSystemloadTest End";
}
#endif
} // namespace OHOS::FileManagement::CloudSync::Test