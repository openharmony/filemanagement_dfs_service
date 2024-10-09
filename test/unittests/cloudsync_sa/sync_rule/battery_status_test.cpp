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
#include "battersrvclient_mock.h"
#include "battery_status.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
constexpr int32_t STOP_CAPACITY_LIMIT = 10;
constexpr int32_t PAUSE_CAPACITY_LIMIT = 15;

class BatteryStatusMock final : public BatteryStatus {
public:
    BatteryStatusMock() : BatteryStatus() {}
    MOCK_METHOD0(GetCapacity, int32_t());
};

class BatteryStatusTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    shared_ptr<BatteryStatusMock> batteryStatus_ = nullptr;
    static inline shared_ptr<BatterySrvClientMock> dfsBatterySrvClient_ = nullptr;
};

void BatteryStatusTest::SetUpTestCase(void)
{
    dfsBatterySrvClient_ = make_shared<BatterySrvClientMock>();
    BatterySrvClientMock::dfsBatterySrvClient = dfsBatterySrvClient_;
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void BatteryStatusTest::TearDownTestCase(void)
{
    BatterySrvClientMock::dfsBatterySrvClient = nullptr;
    dfsBatterySrvClient_ = nullptr;
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void BatteryStatusTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
    batteryStatus_ = make_shared<BatteryStatusMock>();
}

void BatteryStatusTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
    batteryStatus_ = nullptr;
}

/**
 * @tc.name: IsAllowUploadTest001
 * @tc.desc: Verify the IsAllowUpload function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, IsAllowUploadTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsAllowUploadTest001 Start";
    try {
        batteryStatus_->SetChargingStatus(true);
        bool ret = batteryStatus_->IsAllowUpload(true);
        EXPECT_EQ(ret, true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsAllowUploadTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "IsAllowUploadTest001 End";
}

/**
 * @tc.name: IsAllowUploadTest001
 * @tc.desc: Verify the IsAllowUpload function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, IsAllowUploadTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsAllowUploadTest002 Start";
    try {
        batteryStatus_->SetChargingStatus(false);
        bool ret = batteryStatus_->IsAllowUpload(true);
        EXPECT_EQ(ret, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsAllowUploadTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "IsAllowUploadTest002 End";
}

/**
 * @tc.name: IsAllowUploadTest003
 * @tc.desc: Verify the IsAllowUpload function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, IsAllowUploadTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsAllowUploadTest003 Start";
    EXPECT_CALL(*dfsBatterySrvClient_, GetCapacity()).WillOnce(Return(STOP_CAPACITY_LIMIT - 1));
    batteryStatus_->SetChargingStatus(false);
    bool ret = batteryStatus_->IsAllowUpload(false);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "IsAllowUploadTest003 End";
}

/**
 * @tc.name: IsAllowUploadTest004
 * @tc.desc: Verify the IsAllowUpload function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, IsAllowUploadTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsAllowUploadTest004 Start";
    EXPECT_CALL(*dfsBatterySrvClient_, GetCapacity()).WillOnce(Return(PAUSE_CAPACITY_LIMIT - 1));
    batteryStatus_->SetChargingStatus(false);
    bool ret = batteryStatus_->IsAllowUpload(false);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "IsAllowUploadTest004 End";
}

/**
 * @tc.name: IsAllowUploadTest004
 * @tc.desc: Verify the IsAllowUpload function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, IsAllowUploadTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsAllowUploadTest005 Start";
    EXPECT_CALL(*dfsBatterySrvClient_, GetCapacity()).WillOnce(Return(PAUSE_CAPACITY_LIMIT - 1));
    batteryStatus_->SetChargingStatus(false);
    bool ret = batteryStatus_->IsAllowUpload(true);
    EXPECT_TRUE(ret);
    GTEST_LOG_(INFO) << "IsAllowUploadTest005 End";
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
        batteryStatus_->SetChargingStatus(true);
        bool ret = batteryStatus_->IsBatteryCapcityOkay();
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
        EXPECT_CALL(*dfsBatterySrvClient_, GetCapacity()).WillOnce(Return(STOP_CAPACITY_LIMIT + 1));
        batteryStatus_->SetChargingStatus(false);
        bool ret = batteryStatus_->IsBatteryCapcityOkay();
        EXPECT_EQ(ret, true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsBatteryCapcityOkayTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "IsBatteryCapcityOkayTest002 End";
}
} // namespace OHOS::FileManagement::CloudSync::Test