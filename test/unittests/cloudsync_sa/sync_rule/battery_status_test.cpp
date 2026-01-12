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
 * @tc.name: IsAllowAsyncTest001
 * @tc.desc: Verify the IsAllowSync function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, IsAllowAsyncTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsAllowAsyncTest001 Start";
    try {
        BatteryStatus::SetChargingStatus(true);
        bool ret = BatteryStatus::IsAllowSync(true);
        EXPECT_EQ(ret, true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsAllowAsyncTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "IsAllowAsyncTest001 End";
}

/**
 * @tc.name: IsAllowAsyncTest002
 * @tc.desc: Verify the IsAllowSync function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, IsAllowAsyncTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsAllowAsyncTest002 Start";
    try {
        BatteryStatus::SetChargingStatus(false);
        EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(STOP_CAPACITY_LIMIT - 1));
        bool ret = BatteryStatus::IsAllowSync(true);
        EXPECT_EQ(ret, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsAllowAsyncTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "IsAllowAsyncTest002 End";
}

/**
 * @tc.name: IsAllowAsyncTest003
 * @tc.desc: Verify the IsAllowSync function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, IsAllowAsyncTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsAllowAsyncTest003 Start";
    EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(STOP_CAPACITY_LIMIT - 1));
    BatteryStatus::SetChargingStatus(false);
    bool ret = BatteryStatus::IsAllowSync(false);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "IsAllowAsyncTest003 End";
}

/**
 * @tc.name: IsAllowAsyncTest004
 * @tc.desc: Verify the IsAllowSync function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, IsAllowAsyncTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsAllowAsyncTest004 Start";
    EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(PAUSE_CAPACITY_LIMIT - 1));
    BatteryStatus::SetChargingStatus(false);
    bool ret = BatteryStatus::IsAllowSync(false);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "IsAllowAsyncTest004 End";
}

/**
 * @tc.name: IsAllowAsyncTest005
 * @tc.desc: Verify the IsAllowSync function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, IsAllowAsyncTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsAllowAsyncTest005 Start";
    EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(PAUSE_CAPACITY_LIMIT - 1));
    BatteryStatus::SetChargingStatus(false);
    bool ret = BatteryStatus::IsAllowSync(true);
    EXPECT_TRUE(ret);
    GTEST_LOG_(INFO) << "IsAllowAsyncTest005 End";
}

/**
 * @tc.name: IsAllowAsyncTest006
 * @tc.desc: Verify the IsAllowSync function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, IsAllowAsyncTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsAllowAsyncTest006 Start";
    EXPECT_CALL(*batteryClientMock_, GetCapacity()).WillOnce(Return(PAUSE_CAPACITY_LIMIT + 1));
    BatteryStatus::SetChargingStatus(false);
    bool ret = BatteryStatus::IsAllowSync(true);
    EXPECT_TRUE(ret);
    GTEST_LOG_(INFO) << "IsAllowAsyncTest006 End";
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

HWTEST_F(BatteryStatusTest, SetCapacityTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetCapacityTest001 Start";
    BatteryStatus::SetCapacity(FULL_BATTERY_CAPACITY);
    EXPECT_EQ(BatteryStatus::GetCapacity(), FULL_BATTERY_CAPACITY);
    GTEST_LOG_(INFO) << "SetCapacityTest001 End";
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
} // namespace OHOS::FileManagement::CloudSync::Test