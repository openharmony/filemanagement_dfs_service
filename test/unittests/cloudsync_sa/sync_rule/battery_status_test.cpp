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

#include "battery_status.h"

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class BatteryStatusTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};
void BatteryStatusTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void BatteryStatusTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void BatteryStatusTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void BatteryStatusTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: SetChargingStatus
 * @tc.desc: Verify the BatteryStatus::SetChargingStatus function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(BatteryStatusTest, SetChargingStatus, TestSize.Level1)
{
    BatteryStatus batteryStatus;
    batteryStatus.SetChargingStatus(true);
    EXPECT_EQ(batteryStatus.isCharging_, true);
}

/**
 * @tc.name: GetCapacity
 * @tc.desc: Verify the BatteryStatus::GetCapacity function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(BatteryStatusTest, GetCapacity, TestSize.Level1)
{
    BatteryStatus batteryStatus;
    int32_t ret = batteryStatus.GetCapacity();
    EXPECT_EQ(ret, 100);
}

/**
 * @tc.name: IsAllowUpload001
 * @tc.desc: Verify the BatteryStatus::IsAllowUpload function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(BatteryStatusTest, IsAllowUpload001, TestSize.Level1)
{
    BatteryStatus batteryStatus;
    batteryStatus.isCharging_ = true;
    auto ret = batteryStatus.IsAllowUpload(false);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: IsAllowUpload002
 * @tc.desc: Verify the BatteryStatus::IsAllowUpload function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(BatteryStatusTest, IsAllowUpload002, TestSize.Level1)
{
    BatteryStatus batteryStatus;
    batteryStatus.isCharging_ = false;
    auto ret = batteryStatus.IsAllowUpload(true);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: IsAllowUpload003
 * @tc.desc: Verify the BatteryStatus::IsAllowUpload function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(BatteryStatusTest, IsAllowUpload003, TestSize.Level1)
{
    BatteryStatus batteryStatus;
    batteryStatus.isCharging_ = false;
    auto ret = batteryStatus.IsAllowUpload(false);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: IsBatteryCapcityOkay001
 * @tc.desc: Verify the BatteryStatus::IsBatteryCapcityOkay function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(BatteryStatusTest, IsBatteryCapcityOkay001, TestSize.Level1)
{
    BatteryStatus batteryStatus;
    batteryStatus.isCharging_ = true;
    auto ret = batteryStatus.IsBatteryCapcityOkay();
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: IsBatteryCapcityOkay002
 * @tc.desc: Verify the BatteryStatus::IsBatteryCapcityOkay function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(BatteryStatusTest, IsBatteryCapcityOkay002, TestSize.Level1)
{
    BatteryStatus batteryStatus;
    batteryStatus.isCharging_ = false;
    auto ret = batteryStatus.IsBatteryCapcityOkay();
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: GetCapacityLevel
 * @tc.desc: Verify the BatteryStatus::GetCapacityLevel function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(BatteryStatusTest, GetCapacityLevel, TestSize.Level1)
{
    BatteryStatus batteryStatus;
    BatteryStatus::CapacityLevel ret = batteryStatus.GetCapacityLevel();
    EXPECT_EQ(ret, BatteryStatus::LEVEL_NORMAL);
}
} // namespace OHOS::FileManagement::CloudSync::Test
