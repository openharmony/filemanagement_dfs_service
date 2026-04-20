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
#include <vector>
#include "battery_client_mock.h"
#include "battery_status.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using ChargeState = PowerMgr::BatteryChargeState;
using PluggedType = PowerMgr::BatteryPluggedType;
constexpr int32_t STOP_CAPACITY_LIMIT = 10;
constexpr int32_t PAUSE_CAPACITY_LIMIT = 15;
constexpr int32_t FULL_BATTERY_CAPACITY = 100;
constexpr int32_t DEFAULT_BATTERY_CAPCITY = -1;

class BatteryStatusTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<PowerMgr::BatteryClientMethodMock> batteryClientMock_ = nullptr;
};

void BatteryStatusTest::SetUpTestCase(void)
{
    batteryClientMock_ = make_shared<PowerMgr::BatteryClientMethodMock>();
    PowerMgr::BatteryClientMethod::batteryClientMethod_ = batteryClientMock_;
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void BatteryStatusTest::TearDownTestCase(void)
{
    PowerMgr::BatteryClientMethod::batteryClientMethod_ = nullptr;
    batteryClientMock_ = nullptr;
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void BatteryStatusTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
    BatteryStatus::SetChargingStatus(false);
    BatteryStatus::SetCapacity(DEFAULT_BATTERY_CAPCITY);
}

void BatteryStatusTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: IsAllowSyncTest001
 * @tc.desc: Verify the IsAllowSync function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, IsAllowSyncTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsAllowSyncTest001 Start";
    try {
        BatteryStatus::SetChargingStatus(true);
        bool ret = BatteryStatus::IsAllowSync(true);
        EXPECT_EQ(ret, true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsAllowSyncTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "IsAllowSyncTest001 End";
}

/**
 * @tc.name: IsAllowSyncTest002
 * @tc.desc: Verify the IsAllowSync function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, IsAllowSyncTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsAllowSyncTest002 Start";
    try {
        BatteryStatus::SetChargingStatus(false);
        EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(STOP_CAPACITY_LIMIT - 1));
        bool ret = BatteryStatus::IsAllowSync(true);
        EXPECT_EQ(ret, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsAllowSyncTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "IsAllowSyncTest002 End";
}

/**
 * @tc.name: IsAllowSyncTest003
 * @tc.desc: Verify the IsAllowSync function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, IsAllowSyncTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsAllowSyncTest003 Start";
    EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(STOP_CAPACITY_LIMIT - 1));
    BatteryStatus::SetChargingStatus(false);
    bool ret = BatteryStatus::IsAllowSync(false);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "IsAllowSyncTest003 End";
}

/**
 * @tc.name: IsAllowSyncTest004
 * @tc.desc: Verify the IsAllowSync function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, IsAllowSyncTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsAllowSyncTest004 Start";
    EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(PAUSE_CAPACITY_LIMIT - 1));
    BatteryStatus::SetChargingStatus(false);
    bool ret = BatteryStatus::IsAllowSync(false);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "IsAllowSyncTest004 End";
}

/**
 * @tc.name: IsAllowSyncTest005
 * @tc.desc: Verify the IsAllowSync function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, IsAllowSyncTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsAllowSyncTest005 Start";
    EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(PAUSE_CAPACITY_LIMIT - 1));
    BatteryStatus::SetChargingStatus(false);
    bool ret = BatteryStatus::IsAllowSync(true);
    EXPECT_TRUE(ret);
    GTEST_LOG_(INFO) << "IsAllowSyncTest005 End";
}

/**
 * @tc.name: IsAllowSyncTest006
 * @tc.desc: Verify the IsAllowSync function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, IsAllowSyncTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsAllowSyncTest006 Start";
    EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(PAUSE_CAPACITY_LIMIT + 1));
    BatteryStatus::SetChargingStatus(false);
    bool ret = BatteryStatus::IsAllowSync(true);
    EXPECT_TRUE(ret);
    GTEST_LOG_(INFO) << "IsAllowSyncTest006 End";
}

/**
 * @tc.name: IsAllowSyncTest007
 * @tc.desc: Test IsAllowSync with zero battery capacity and not charging
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, IsAllowSyncTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsAllowSyncTest007 Start";
    try {
        EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(0));
        BatteryStatus::SetChargingStatus(false);
        bool ret = BatteryStatus::IsAllowSync(true);
        EXPECT_FALSE(ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsAllowSyncTest007 FAILED";
    }
    GTEST_LOG_(INFO) << "IsAllowSyncTest007 End";
}

/**
 * @tc.name: IsAllowSyncTest008
 * @tc.desc: Test IsAllowSync with full battery capacity
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, IsAllowSyncTest008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsAllowSyncTest008 Start";
    try {
        EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(FULL_BATTERY_CAPACITY));
        BatteryStatus::SetChargingStatus(false);
        bool ret = BatteryStatus::IsAllowSync(true);
        EXPECT_TRUE(ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsAllowSyncTest008 FAILED";
    }
    GTEST_LOG_(INFO) << "IsAllowSyncTest008 End";
}

/**
 * @tc.name: IsAllowSyncTest009
 * @tc.desc: Test IsAllowSync with capacity exactly at STOP_CAPACITY_LIMIT
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, IsAllowSyncTest009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsAllowSyncTest009 Start";
    try {
        EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(STOP_CAPACITY_LIMIT));
        BatteryStatus::SetChargingStatus(false);
        bool ret = BatteryStatus::IsAllowSync(true);
        EXPECT_TRUE(ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsAllowSyncTest009 FAILED";
    }
    GTEST_LOG_(INFO) << "IsAllowSyncTest009 End";
}

/**
 * @tc.name: IsAllowSyncTest010
 * @tc.desc: Test IsAllowSync with capacity exactly at PAUSE_CAPACITY_LIMIT
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, IsAllowSyncTest010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsAllowSyncTest010 Start";
    try {
        EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(PAUSE_CAPACITY_LIMIT));
        BatteryStatus::SetChargingStatus(false);
        bool ret = BatteryStatus::IsAllowSync(true);
        EXPECT_TRUE(ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsAllowSyncTest010 FAILED";
    }
    GTEST_LOG_(INFO) << "IsAllowSyncTest010 End";
}

/**
 * @tc.name: IsAllowSyncTest011
 * @tc.desc: Test IsAllowSync with medium battery capacity (50%)
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, IsAllowSyncTest011, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsAllowSyncTest011 Start";
    try {
        EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(50));
        BatteryStatus::SetChargingStatus(false);
        bool ret = BatteryStatus::IsAllowSync(true);
        EXPECT_TRUE(ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsAllowSyncTest011 FAILED";
    }
    GTEST_LOG_(INFO) << "IsAllowSyncTest011 End";
}

/**
 * @tc.name: IsAllowSyncTest012
 * @tc.desc: Test IsAllowSync with charging status and various capacities
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, IsAllowSyncTest012, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsAllowSyncTest012 Start";
    try {
        BatteryStatus::SetChargingStatus(true);
        bool ret = BatteryStatus::IsAllowSync(false);
        EXPECT_TRUE(ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsAllowSyncTest012 FAILED";
    }
    GTEST_LOG_(INFO) << "IsAllowSyncTest012 End";
}

/**
 * @tc.name: IsBatteryCapcityOkayTest001
 * @tc.desc: Verify the IsBatteryCapcityOkay function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, IsBatteryCapcityOkayTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsBatteryCapcityOkayTest001 Start";
    try {
        BatteryStatus::SetChargingStatus(true);
        bool ret = BatteryStatus::IsBatteryCapcityOkay();
        EXPECT_EQ(ret, true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsBatteryCapcityOkayTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "IsBatteryCapcityOkayTest001 End";
}

/**
 * @tc.name: IsBatteryCapcityOkayTest002
 * @tc.desc: Verify the IsBatteryCapcityOkay function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, IsBatteryCapcityOkayTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsBatteryCapcityOkayTest Start";
    try {
        EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(STOP_CAPACITY_LIMIT + 1));
        BatteryStatus::SetChargingStatus(false);
        bool ret = BatteryStatus::IsBatteryCapcityOkay();
        EXPECT_EQ(ret, true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsBatteryCapcityOkayTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "IsBatteryCapcityOkayTest002 End";
}

/**
 * @tc.name: IsBatteryCapcityOkayTest003
 * @tc.desc: Verify the IsBatteryCapcityOkay function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, IsBatteryCapcityOkayTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsBatteryCapcityOkayTest003 Start";
    try {
        EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(STOP_CAPACITY_LIMIT - 1));
        BatteryStatus::SetChargingStatus(false);
        bool ret = BatteryStatus::IsBatteryCapcityOkay();
        EXPECT_EQ(ret, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsBatteryCapcityOkayTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "IsBatteryCapcityOkayTest003 End";
}

/**
 * @tc.name: IsBatteryCapcityOkayTest004
 * @tc.desc: Test IsBatteryCapcityOkay with capacity exactly at STOP_CAPACITY_LIMIT
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, IsBatteryCapcityOkayTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsBatteryCapcityOkayTest004 Start";
    try {
        EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(STOP_CAPACITY_LIMIT));
        BatteryStatus::SetChargingStatus(false);
        bool ret = BatteryStatus::IsBatteryCapcityOkay();
        EXPECT_TRUE(ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsBatteryCapcityOkayTest004 FAILED";
    }
    GTEST_LOG_(INFO) << "IsBatteryCapcityOkayTest004 End";
}

/**
 * @tc.name: IsBatteryCapcityOkayTest005
 * @tc.desc: Test IsBatteryCapcityOkay with zero capacity
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, IsBatteryCapcityOkayTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsBatteryCapcityOkayTest005 Start";
    try {
        EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(0));
        BatteryStatus::SetChargingStatus(false);
        bool ret = BatteryStatus::IsBatteryCapcityOkay();
        EXPECT_FALSE(ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsBatteryCapcityOkayTest005 FAILED";
    }
    GTEST_LOG_(INFO) << "IsBatteryCapcityOkayTest005 End";
}

/**
 * @tc.name: IsBatteryCapcityOkayTest006
 * @tc.desc: Test IsBatteryCapcityOkay with full capacity
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, IsBatteryCapcityOkayTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsBatteryCapcityOkayTest006 Start";
    try {
        EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(FULL_BATTERY_CAPACITY));
        BatteryStatus::SetChargingStatus(false);
        bool ret = BatteryStatus::IsBatteryCapcityOkay();
        EXPECT_TRUE(ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsBatteryCapcityOkayTest006 FAILED";
    }
    GTEST_LOG_(INFO) << "IsBatteryCapcityOkayTest006 End";
}

/**
 * @tc.name: IsBatteryCapcityOkayTest007
 * @tc.desc: Test IsBatteryCapcityOkay with capacity between STOP and PAUSE limits
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, IsBatteryCapcityOkayTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsBatteryCapcityOkayTest007 Start";
    try {
        EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(12));
        BatteryStatus::SetChargingStatus(false);
        bool ret = BatteryStatus::IsBatteryCapcityOkay();
        EXPECT_TRUE(ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsBatteryCapcityOkayTest007 FAILED";
    }
    GTEST_LOG_(INFO) << "IsBatteryCapcityOkayTest007 End";
}

HWTEST_F(BatteryStatusTest, GetInitChargingStatus_Plugged_BUTT, TestSize.Level1)
{
    EXPECT_CALL(*batteryClientMock_, GetPluggedType())
        .WillOnce(Return(PluggedType::PLUGGED_TYPE_BUTT));
    EXPECT_CALL(*batteryClientMock_, GetCapacity())
        .WillOnce(Return(50));

    BatteryStatus::GetInitChargingStatus();
    EXPECT_FALSE(BatteryStatus::IsCharging());
}

HWTEST_F(BatteryStatusTest, GetInitChargingStatus_Plugged_AC, TestSize.Level1)
{
    EXPECT_CALL(*batteryClientMock_, GetPluggedType())
        .WillOnce(Return(PluggedType::PLUGGED_TYPE_AC));
    EXPECT_CALL(*batteryClientMock_, GetCapacity())
        .WillOnce(Return(50));

    BatteryStatus::GetInitChargingStatus();
    EXPECT_TRUE(BatteryStatus::IsCharging());
}

HWTEST_F(BatteryStatusTest, GetInitChargingStatus_Plugged_USB, TestSize.Level1)
{
    EXPECT_CALL(*batteryClientMock_, GetPluggedType())
        .WillOnce(Return(PluggedType::PLUGGED_TYPE_USB));
    EXPECT_CALL(*batteryClientMock_, GetCapacity())
        .WillOnce(Return(50));

    BatteryStatus::GetInitChargingStatus();
    EXPECT_TRUE(BatteryStatus::IsCharging());
}

HWTEST_F(BatteryStatusTest, GetInitChargingStatus_NotCharging_NotPlugged, TestSize.Level1)
{
    EXPECT_CALL(*batteryClientMock_, GetPluggedType())
        .WillOnce(Return(PluggedType::PLUGGED_TYPE_NONE));
    EXPECT_CALL(*batteryClientMock_, GetCapacity())
        .WillOnce(Return(50));

    BatteryStatus::GetInitChargingStatus();
    EXPECT_FALSE(BatteryStatus::IsCharging());
}

HWTEST_F(BatteryStatusTest, GetCapacityLevelTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCapacityLevelTest001 Start";
    EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(STOP_CAPACITY_LIMIT - 1));
    BatteryStatus::SetChargingStatus(false);
    BatteryStatus::IsAllowSync(false);
    EXPECT_EQ(BatteryStatus::GetCapacityLevel(), BatteryStatus::LEVEL_TOO_LOW);
    GTEST_LOG_(INFO) << "GetCapacityLevelTest001 End";
}

HWTEST_F(BatteryStatusTest, GetCapacityLevelTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCapacityLevelTest002 Start";
    EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(PAUSE_CAPACITY_LIMIT - 1));
    BatteryStatus::SetChargingStatus(false);
    BatteryStatus::IsAllowSync(false);
    EXPECT_EQ(BatteryStatus::GetCapacityLevel(), BatteryStatus::LEVEL_LOW);
    GTEST_LOG_(INFO) << "GetCapacityLevelTest002 End";
}

HWTEST_F(BatteryStatusTest, GetCapacityLevelTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCapacityLevelTest003 Start";
    EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(PAUSE_CAPACITY_LIMIT + 1));
    BatteryStatus::SetChargingStatus(false);
    BatteryStatus::IsAllowSync(false);
    EXPECT_EQ(BatteryStatus::GetCapacityLevel(), BatteryStatus::LEVEL_NORMAL);
    GTEST_LOG_(INFO) << "GetCapacityLevelTest003 End";
}

/**
 * @tc.name: GetCapacityLevelTest004
 * @tc.desc: Test GetCapacityLevel with capacity exactly at PAUSE_CAPACITY_LIMIT
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, GetCapacityLevelTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCapacityLevelTest004 Start";
    EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(PAUSE_CAPACITY_LIMIT));
    BatteryStatus::SetChargingStatus(false);
    BatteryStatus::IsAllowSync(false);
    EXPECT_EQ(BatteryStatus::GetCapacityLevel(), BatteryStatus::LEVEL_NORMAL);
    GTEST_LOG_(INFO) << "GetCapacityLevelTest004 End";
}

/**
 * @tc.name: GetCapacityLevelTest005
 * @tc.desc: Test GetCapacityLevel with zero capacity
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, GetCapacityLevelTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCapacityLevelTest005 Start";
    EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(0));
    BatteryStatus::SetChargingStatus(false);
    BatteryStatus::IsAllowSync(false);
    EXPECT_EQ(BatteryStatus::GetCapacityLevel(), BatteryStatus::LEVEL_TOO_LOW);
    GTEST_LOG_(INFO) << "GetCapacityLevelTest005 End";
}

/**
 * @tc.name: GetCapacityLevelTest006
 * @tc.desc: Test GetCapacityLevel with full capacity
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, GetCapacityLevelTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCapacityLevelTest006 Start";
    EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(FULL_BATTERY_CAPACITY));
    BatteryStatus::SetChargingStatus(false);
    BatteryStatus::IsAllowSync(false);
    EXPECT_EQ(BatteryStatus::GetCapacityLevel(), BatteryStatus::LEVEL_NORMAL);
    GTEST_LOG_(INFO) << "GetCapacityLevelTest006 End";
}

HWTEST_F(BatteryStatusTest, GetCapacityTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCapacityTest001 Start";
    BatteryStatus::SetCapacity(PAUSE_CAPACITY_LIMIT);
    int32_t capacity = BatteryStatus::GetCapacity();
    EXPECT_EQ(capacity, PAUSE_CAPACITY_LIMIT);
    GTEST_LOG_(INFO) << "GetCapacityTest001 End";
}

HWTEST_F(BatteryStatusTest, GetCapacityTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCapacityTest002 Start";
    EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(PAUSE_CAPACITY_LIMIT));
    BatteryStatus::SetCapacity(DEFAULT_BATTERY_CAPCITY);
    int32_t capacity = BatteryStatus::GetCapacity();
    EXPECT_EQ(capacity, PAUSE_CAPACITY_LIMIT);
    GTEST_LOG_(INFO) << "GetCapacityTest002 End";
}

/**
 * @tc.name: GetCapacityTest003
 * @tc.desc: Test GetCapacity with zero value
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, GetCapacityTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCapacityTest003 Start";
    BatteryStatus::SetCapacity(0);
    int32_t capacity = BatteryStatus::GetCapacity();
    EXPECT_EQ(capacity, 0);
    GTEST_LOG_(INFO) << "GetCapacityTest003 End";
}

/**
 * @tc.name: GetCapacityTest004
 * @tc.desc: Test GetCapacity with full battery value
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, GetCapacityTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCapacityTest004 Start";
    BatteryStatus::SetCapacity(FULL_BATTERY_CAPACITY);
    int32_t capacity = BatteryStatus::GetCapacity();
    EXPECT_EQ(capacity, FULL_BATTERY_CAPACITY);
    GTEST_LOG_(INFO) << "GetCapacityTest004 End";
}

/**
 * @tc.name: GetCapacityTest005
 * @tc.desc: Test GetCapacity with negative value
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, GetCapacityTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCapacityTest005 Start";
    EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(-5));
    BatteryStatus::SetCapacity(DEFAULT_BATTERY_CAPCITY);
    int32_t capacity = BatteryStatus::GetCapacity();
    EXPECT_EQ(capacity, -5);
    GTEST_LOG_(INFO) << "GetCapacityTest005 End";
}

HWTEST_F(BatteryStatusTest, SetCapacityTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetCapacityTest001 Start";
    BatteryStatus::SetCapacity(FULL_BATTERY_CAPACITY);
    EXPECT_EQ(BatteryStatus::GetCapacity(), FULL_BATTERY_CAPACITY);
    GTEST_LOG_(INFO) << "SetCapacityTest001 End";
}

/**
 * @tc.name: SetCapacityTest002
 * @tc.desc: Test SetCapacity with zero value
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, SetCapacityTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetCapacityTest002 Start";
    BatteryStatus::SetCapacity(0);
    EXPECT_EQ(BatteryStatus::GetCapacity(), 0);
    GTEST_LOG_(INFO) << "SetCapacityTest002 End";
}

/**
 * @tc.name: SetCapacityTest003
 * @tc.desc: Test SetCapacity with value exceeding full capacity
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, SetCapacityTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetCapacityTest003 Start";
    BatteryStatus::SetCapacity(150);
    EXPECT_EQ(BatteryStatus::GetCapacity(), 150);
    GTEST_LOG_(INFO) << "SetCapacityTest003 End";
}

/**
 * @tc.name: SetCapacityTest004
 * @tc.desc: Test SetCapacity with value at STOP_CAPACITY_LIMIT
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, SetCapacityTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetCapacityTest004 Start";
    BatteryStatus::SetCapacity(STOP_CAPACITY_LIMIT);
    EXPECT_EQ(BatteryStatus::GetCapacity(), STOP_CAPACITY_LIMIT);
    GTEST_LOG_(INFO) << "SetCapacityTest004 End";
}

HWTEST_F(BatteryStatusTest, IsChargingTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsChargingTest001 Start";
    BatteryStatus::SetChargingStatus(true);
    EXPECT_TRUE(BatteryStatus::IsCharging());
    GTEST_LOG_(INFO) << "IsChargingTest001 End";
}

HWTEST_F(BatteryStatusTest, IsChargingTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsChargingTest002 Start";
    BatteryStatus::SetChargingStatus(false);
    EXPECT_FALSE(BatteryStatus::IsCharging());
    GTEST_LOG_(INFO) << "IsChargingTest002 End";
}

/**
 * @tc.name: IsChargingTest003
 * *tc.desc: Test IsCharging after multiple state changes
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, IsChargingTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsChargingTest003 Start";
    BatteryStatus::SetChargingStatus(true);
    EXPECT_TRUE(BatteryStatus::IsCharging());
    BatteryStatus::SetChargingStatus(false);
    EXPECT_FALSE(BatteryStatus::IsCharging());
    BatteryStatus::SetChargingStatus(true);
    EXPECT_TRUE(BatteryStatus::IsCharging());
    GTEST_LOG_(INFO) << "IsChargingTest003 End";
}

/**
 * @tc.name: BatteryStatusTransitionTest001
 * @tc.desc: Test battery status transition from low to normal
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, BatteryStatusTransitionTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BatteryStatusTransitionTest001 Start";
    try {
        EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(STOP_CAPACITY_LIMIT - 1));
        BatteryStatus::SetChargingStatus(false);
        bool ret1 = BatteryStatus::IsAllowSync(true);
        EXPECT_FALSE(ret1);
        
        EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(PAUSE_CAPACITY_LIMIT + 1));
        bool ret2 = BatteryStatus::IsAllowSync(true);
        EXPECT_TRUE(ret2);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "BatteryStatusTransitionTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "BatteryStatusTransitionTest001 End";
}

/**
 * @tc.name: BatteryStatusTransitionTest002
 * @tc.desc: Test battery status transition while charging
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, BatteryStatusTransitionTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BatteryStatusTransitionTest002 Start";
    try {
        BatteryStatus::SetChargingStatus(false);
        EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(STOP_CAPACITY_LIMIT - 1));
        bool ret1 = BatteryStatus::IsAllowSync(true);
        EXPECT_FALSE(ret1);
        
        BatteryStatus::SetChargingStatus(true);
        bool ret2 = BatteryStatus::IsAllowSync(true);
        EXPECT_TRUE(ret2);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "BatteryStatusTransitionTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "BatteryStatusTransitionTest002 End";
}

/**
 * @tc.name: BatteryStatusEdgeCaseTest001
 * @tc.desc: Test battery status with capacity one above limit
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, BatteryStatusEdgeCaseTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BatteryStatusEdgeCaseTest001 Start";
    try {
        EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(STOP_CAPACITY_LIMIT + 1));
        BatteryStatus::SetChargingStatus(false);
        bool ret = BatteryStatus::IsAllowSync(true);
        EXPECT_TRUE(ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "BatteryStatusEdgeCaseTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "BatteryStatusEdgeCaseTest001 End";
}

/**
 * @tc.name: BatteryStatusEdgeCaseTest002
 * @tc.desc: Test battery status with capacity one below limit
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, BatteryStatusEdgeCaseTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BatteryStatusEdgeCaseTest002 Start";
    try {
        EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(PAUSE_CAPACITY_LIMIT - 1));
        BatteryStatus::SetChargingStatus(false);
        bool ret = BatteryStatus::IsAllowSync(true);
        EXPECT_TRUE(ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "BatteryStatusEdgeCaseTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "BatteryStatusEdgeCaseTest002 End";
}

/**
 * @tc.name: BatteryStatusMultipleCallsTest001
 * @tc.desc: Test multiple consecutive calls to IsAllowSync
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, BatteryStatusMultipleCallsTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BatteryStatusMultipleCallsTest001 Start";
    try {
        BatteryStatus::SetChargingStatus(true);
        bool ret1 = BatteryStatus::IsAllowSync(true);
        EXPECT_TRUE(ret1);
        bool ret2 = BatteryStatus::IsAllowSync(false);
        EXPECT_TRUE(ret2);
        bool ret3 = BatteryStatus::IsAllowSync(true);
        EXPECT_TRUE(ret3);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "BatteryStatusMultipleCallsTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "BatteryStatusMultipleCallsTest001 End";
}

/**
 * @tc.name: BatteryStatusMultipleCallsTest002
 * @tc.desc: Test multiple consecutive calls with changing capacity
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, BatteryStatusMultipleCallsTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BatteryStatusMultipleCallsTest002 Start";
    try {
        BatteryStatus::SetChargingStatus(false);
        EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(50));
        bool ret1 = BatteryStatus::IsAllowSync(true);
        EXPECT_TRUE(ret1);
        
        EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(STOP_CAPACITY_LIMIT - 1));
        bool ret2 = BatteryStatus::IsAllowSync(true);
        EXPECT_FALSE(ret2);
        
        EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(50));
        bool ret3 = BatteryStatus::IsAllowSync(true);
        EXPECT_TRUE(ret3);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "BatteryStatusMultipleCallsTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "BatteryStatusMultipleCallsTest002 End";
}

/**
 * @tc.name: BatteryStatusCapacityBoundaryTest001
 * @tc.desc: Test capacity at exact boundary values
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, BatteryStatusCapacityBoundaryTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BatteryStatusCapacityBoundaryTest001 Start";
    try {
        EXPECT_EQ(STOP_CAPACITY_LIMIT, 10);
        EXPECT_EQ(PAUSE_CAPACITY_LIMIT, 15);
        
        EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(10));
        BatteryStatus::SetChargingStatus(false);
        bool ret1 = BatteryStatus::IsAllowSync(true);
        EXPECT_TRUE(ret1);
        
        EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(9));
        bool ret2 = BatteryStatus::IsAllowSync(true);
        EXPECT_FALSE(ret2);
        
        EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(15));
        bool ret3 = BatteryStatus::IsAllowSync(true);
        EXPECT_TRUE(ret3);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "BatteryStatusCapacityBoundaryTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "BatteryStatusCapacityBoundaryTest001 End";
}

/**
 * @tc.name: BatteryStatusConsistencyTest001
 * @tc.desc: Test consistency between GetCapacity and SetCapacity
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, BatteryStatusConsistencyTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BatteryStatusConsistencyTest001 Start";
    try {
        BatteryStatus::SetCapacity(75);
        int32_t capacity = BatteryStatus::GetCapacity();
        EXPECT_EQ(capacity, 75);
        
        BatteryStatus::SetCapacity(25);
        capacity = BatteryStatus::GetCapacity();
        EXPECT_EQ(capacity, 25);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "BatteryStatusConsistencyTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "BatteryStatusConsistencyTest001 End";
}

/**
 * @tc.name: BatteryStatusInitTest001
 * @tc.desc: Test initial battery status state
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, BatteryStatusInitTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BatteryStatusInitTest001 Start";
    try {
        EXPECT_CALL(*batteryClientMock_, GetPluggedType()).WillOnce(Return(PluggedType::PLUGGED_TYPE_NONE));
        EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(50));
        
        BatteryStatus::GetInitChargingStatus();
        EXPECT_FALSE(BatteryStatus::IsCharging());
        
        int32_t capacity = BatteryStatus::GetCapacity();
        EXPECT_EQ(capacity, 50);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "BatteryStatusInitTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "BatteryStatusInitTest001 End";
}

/**
 * @tc.name: BatteryStatusInitTest002
 * @tc.desc: Test initial battery status with AC plugged
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, BatteryStatusInitTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BatteryStatusInitTest002 Start";
    try {
        EXPECT_CALL(*batteryClientMock_, GetPluggedType()).WillOnce(Return(PluggedType::PLUGGED_TYPE_AC));
        EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(80));
        
        BatteryStatus::GetInitChargingStatus();
        EXPECT_TRUE(BatteryStatus::IsCharging());
        
        int32_t capacity = BatteryStatus::GetCapacity();
        EXPECT_EQ(capacity, 80);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "BatteryStatusInitTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "BatteryStatusInitTest002 End";
}

/**
 * @tc.name: BatteryStatusCapacityLevelConsistencyTest001
 * @tc.desc: Test consistency of capacity level with actual capacity
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, BatteryStatusCapacityLevelConsistencyTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BatteryStatusCapacityLevelConsistencyTest001 Start";
    try {
        EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(5));
        BatteryStatus::SetChargingStatus(false);
        BatteryStatus::IsAllowSync(false);
        EXPECT_EQ(BatteryStatus::GetCapacityLevel(), BatteryStatus::LEVEL_TOO_LOW);
        
        EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(12));
        BatteryStatus::IsAllowSync(false);
        EXPECT_EQ(BatteryStatus::GetCapacityLevel(), BatteryStatus::LEVEL_LOW);
        
        EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(50));
        BatteryStatus::IsAllowSync(false);
        EXPECT_EQ(BatteryStatus::GetCapacityLevel(), BatteryStatus::LEVEL_NORMAL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "BatteryStatusCapacityLevelConsistencyTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "BatteryStatusCapacityLevelConsistencyTest001 End";
}

/**
 * @tc.name: BatteryStatusExtremeValuesTest001
 * @tc.desc: Test battery status with extreme capacity values
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, BatteryStatusExtremeValuesTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BatteryStatusExtremeValuesTest001 Start";
    try {
        EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(-100));
        BatteryStatus::SetChargingStatus(false);
        bool ret1 = BatteryStatus::IsAllowSync(true);
        EXPECT_FALSE(ret1);
        
        EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(200));
        bool ret2 = BatteryStatus::IsAllowSync(true);
        EXPECT_TRUE(ret2);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "BatteryStatusExtremeValuesTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "BatteryStatusExtremeValuesTest001 End";
}

/**
 * @tc.name: BatteryStatusChargingStatePersistenceTest001
 * @tc.desc: Test that charging state persists across multiple calls
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, BatteryStatusChargingStatePersistenceTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BatteryStatusChargingStatePersistenceTest001 Start";
    try {
        BatteryStatus::SetChargingStatus(true);
        
        for (int i = 0; i < 5; i++) {
            EXPECT_TRUE(BatteryStatus::IsCharging());
        }
        
        BatteryStatus::SetChargingStatus(false);
        
        for (int i = 0; i < 5; i++) {
            EXPECT_FALSE(BatteryStatus::IsCharging());
        }
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "BatteryStatusChargingStatePersistenceTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "BatteryStatusChargingStatePersistenceTest001 End";
}

/**
 * @tc.name: BatteryStatusCapacityUpdateTest001
 * @tc.desc: Test capacity updates through mock
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, BatteryStatusCapacityUpdateTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BatteryStatusCapacityUpdateTest001 Start";
    try {
        BatteryStatus::SetCapacity(DEFAULT_BATTERY_CAPCITY);
        
        EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(30));
        int32_t capacity1 = BatteryStatus::GetCapacity();
        EXPECT_EQ(capacity1, 30);
        
        EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(70));
        int32_t capacity2 = BatteryStatus::GetCapacity();
        EXPECT_EQ(capacity2, 70);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "BatteryStatusCapacityUpdateTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "BatteryStatusCapacityUpdateTest001 End";
}

/**
 * @tc.name: BatteryStatusBoundaryConditionsTest001
 * @tc.desc: Test boundary conditions for battery capacity
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, BatteryStatusBoundaryConditionsTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BatteryStatusBoundaryConditionsTest001 Start";
    try {
        BatteryStatus::SetChargingStatus(false);
        
        EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(1));
        bool ret1 = BatteryStatus::IsAllowSync(true);
        EXPECT_FALSE(ret1);
        
        EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(99));
        bool ret2 = BatteryStatus::IsAllowSync(true);
        EXPECT_TRUE(ret2);
        
        EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(100));
        bool ret3 = BatteryStatus::IsAllowSync(true);
        EXPECT_TRUE(ret3);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "BatteryStatusBoundaryConditionsTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "BatteryStatusBoundaryConditionsTest001 End";
}

/**
 * @tc.name: BatteryStatusSequentialTest001
 * @tc.desc: Test sequential battery capacity checks
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, BatteryStatusSequentialTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BatteryStatusSequentialTest001 Start";
    try {
        BatteryStatus::SetChargingStatus(false);
        
        std::vector<int32_t> capacities = {5, 10, 15, 20, 25, 30, 50, 75, 100};
        std::vector<bool> expectedResults = {false, true, true, true, true, true, true, true, true};
        
        for (size_t i = 0; i < capacities.size(); i++) {
            EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(capacities[i]));
            bool ret = BatteryStatus::IsAllowSync(true);
            EXPECT_EQ(ret, expectedResults[i]);
        }
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "BatteryStatusSequentialTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "BatteryStatusSequentialTest001 End";
}

/**
 * @tc.name: BatteryStatusNotChargingWithCapacityTest001
 * @tc.desc: Test not charging status combined with various capacity levels
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, BatteryStatusNotChargingWithCapacityTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BatteryStatusNotChargingWithCapacityTest001 Start";
    try {
        BatteryStatus::SetChargingStatus(false);
        
        EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(5));
        bool ret1 = BatteryStatus::IsAllowSync(true);
        EXPECT_FALSE(ret1);
        
        EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(12));
        bool ret2 = BatteryStatus::IsAllowSync(true);
        EXPECT_TRUE(ret2);
        
        EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(50));
        bool ret3 = BatteryStatus::IsAllowSync(true);
        EXPECT_TRUE(ret3);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "BatteryStatusNotChargingWithCapacityTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "BatteryStatusNotChargingWithCapacityTest001 End";
}

/**
 * @tc.name: BatteryStatusCapacityLevelTrackingTest001
 * @tc.desc: Test tracking of capacity level changes
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, BatteryStatusCapacityLevelTrackingTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BatteryStatusCapacityLevelTrackingTest001 Start";
    try {
        BatteryStatus::SetChargingStatus(false);
        
        EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(5));
        BatteryStatus::IsAllowSync(false);
        EXPECT_EQ(BatteryStatus::GetCapacityLevel(), BatteryStatus::LEVEL_TOO_LOW);
        
        EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(14));
        BatteryStatus::IsAllowSync(false);
        EXPECT_EQ(BatteryStatus::GetCapacityLevel(), BatteryStatus::LEVEL_LOW);
        
        EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(20));
        BatteryStatus::IsAllowSync(false);
        EXPECT_EQ(BatteryStatus::GetCapacityLevel(), BatteryStatus::LEVEL_NORMAL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "BatteryStatusCapacityLevelTrackingTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "BatteryStatusCapacityLevelTrackingTest001 End";
}

/**
 * @tc.name: BatteryStatusMockInteractionTest001
 * @tc.desc: Test interaction with battery client mock
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, BatteryStatusMockInteractionTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BatteryStatusMockInteractionTest001 Start";
    try {
        EXPECT_CALL(*batteryClientMock_, GetCapacity()).Times(3).WillRepeatedly(Return(45));
        BatteryStatus::SetChargingStatus(false);
        
        bool ret1 = BatteryStatus::IsAllowSync(true);
        EXPECT_TRUE(ret1);
        
        bool ret2 = BatteryStatus::IsAllowSync(false);
        EXPECT_TRUE(ret2);
        
        bool ret3 = BatteryStatus::IsBatteryCapcityOkay();
        EXPECT_TRUE(ret3);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "BatteryStatusMockInteractionTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "BatteryStatusMockInteractionTest001 End";
}

/**
 * @tc.name: BatteryStatusPerformanceTest001
 * @tc.desc: Test performance of repeated battery status checks
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, BatteryStatusPerformanceTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BatteryStatusPerformanceTest001 Start";
    try {
        BatteryStatus::SetChargingStatus(true);
        
        for (int i = 0; i < 100; i++) {
            bool ret = BatteryStatus::IsAllowSync(true);
            EXPECT_TRUE(ret);
        }
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "BatteryStatusPerformanceTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "BatteryStatusPerformanceTest001 End";
}

/**
 * @tc.name: BatteryStatusErrorHandlingTest001
 * @tc.desc: Test error handling in battery status operations
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, BatteryStatusErrorHandlingTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BatteryStatusErrorHandlingTest001 Start";
    try {
        BatteryStatus::SetChargingStatus(false);
        EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(-1));
        
        bool ret = BatteryStatus::IsAllowSync(true);
        EXPECT_FALSE(ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "BatteryStatusErrorHandlingTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "BatteryStatusErrorHandlingTest001 End";
}

} // namespace OHOS::FileManagement::CloudSync::Test