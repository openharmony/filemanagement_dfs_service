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

#include "sync_rule/battery_status.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

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
 * @tc.name: IsBatteryCapcityOkayTest001
 * @tc.desc: Verify the IsBatteryCapcityOkay function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusTest, IsBatteryCapcityOkayTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsBatteryCapcityOkayTest Start";
    try {
        batteryStatus_->SetChargingStatus(true);
        bool ret = batteryStatus_->IsBatteryCapcityOkay();
        EXPECT_EQ(ret, true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsBatteryCapcityOkayTest FAILED";
    }
    GTEST_LOG_(INFO) << "IsBatteryCapcityOkayTest End";
}
} // namespace OHOS::FileManagement::CloudSync::Test
