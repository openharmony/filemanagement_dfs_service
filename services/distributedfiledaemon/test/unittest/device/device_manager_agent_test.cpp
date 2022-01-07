/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "device/device_manager_agent.h"
#include <gtest/gtest.h>

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace Test {
using namespace testing::ext;

class DeviceManagerAgentTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DeviceManagerAgentTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
}

void DeviceManagerAgentTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void DeviceManagerAgentTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void DeviceManagerAgentTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

/**
 * @tc.name: integer_sub_001
 * @tc.desc: Verify the sub function.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DeviceManagerAgentTest, test_001, TestSize.Level1)
{
    // step 1:调用函数获取结果
    int actual = 4;
    // Step 2:使用断言比较预期与实际结果
    EXPECT_EQ(4, actual);
}
} // namespace Test
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS