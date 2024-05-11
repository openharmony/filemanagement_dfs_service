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
#include "device/device_info.h"

#include <exception>
#include <memory>
#include <unistd.h>

#include "gtest/gtest.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace Test {
using namespace testing::ext;
using namespace std;

const string NETWORKID = "testNetWorkId";

DistributedHardware::DmDeviceInfo deviceInfo = {
    .deviceId = "testdevid",
    .deviceName = "testdevname",
    .deviceTypeId = 1,
    .networkId = "testNetWorkId",
};

class DeviceInfoTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DeviceInfoTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
}

void DeviceInfoTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void DeviceInfoTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void DeviceInfoTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

/**
 * @tc.name: DeviceInfoTest_SetCid_0100
 * @tc.desc: Verify the SetCid function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(DeviceInfoTest, DeviceInfoTest_SetCid_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceInfoTest_SetCid_0100 start";
    DeviceInfo devInfo(deviceInfo);
    try {
        string testCid = "test";
        devInfo.SetCid(testCid);
        EXPECT_EQ(devInfo.GetCid(), NETWORKID);

        devInfo.initCidFlag_ = false;
        devInfo.SetCid(testCid);
        EXPECT_EQ(devInfo.GetCid(), testCid);
    } catch (...) {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "DeviceInfoTest_SetCid_0100 start";
}

/**
 * @tc.name: DeviceInfoTest_GetCid_0100
 * @tc.desc: Verify the GetCid function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(DeviceInfoTest, DeviceInfoTest_GetCid_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceInfoTest_GetCid_0100 start";
    DeviceInfo devInfo(deviceInfo);
    try {
        devInfo.initCidFlag_ = false;
        (void)devInfo.GetCid();
        EXPECT_TRUE(false);
    } catch (...) {
        EXPECT_TRUE(true);
    }
    GTEST_LOG_(INFO) << "DeviceInfoTest_GetCid_0100 start";
}
}
} // namespace Test
} // namespace DistributedFile
}