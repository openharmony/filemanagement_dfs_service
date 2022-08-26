/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include <memory>
#include <unistd.h>

#include "gtest/gtest.h"
#include "network/softbus/softbus_agent.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace Test {
using namespace testing::ext;
using namespace std;

constexpr int E_OK = 0;
static const string SAME_ACCOUNT = "account";

class SoftbusAgentTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void SoftbusAgentTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
}

void SoftbusAgentTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void SoftbusAgentTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void SoftbusAgentTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

/**
 * @tc.name: SoftbusAgentTest_OnSessionOpened_010
 * @tc.desc: Verify the OnSessionOpened function.
 * @tc.type: FUNC
 * @tc.require: 
 */
HWTEST_F(SoftbusAgentTest, SoftbusAgentTest_OnSessionOpened_010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusAgentTest_OnSessionOpened_010 start";
    auto mp = make_unique<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(100, SAME_ACCOUNT));
    shared_ptr<MountPoint> smp = move(mp);
    weak_ptr<MountPoint> wmp(smp);
    std::shared_ptr<SoftbusAgent> agent = std::make_shared<SoftbusAgent>(wmp);

    auto execFun = [](std::shared_ptr<SoftbusAgent> ag) {
        const int sessionId = 1;
        const int result = E_OK;
        int ret = ag->OnSessionOpened(sessionId, result);
        EXPECT_TRUE(ret == E_OK);
    };

    std::thread execThread(execFun, agent);
    sleep(1);
    execThread.join();
    GTEST_LOG_(INFO) << "SoftbusAgentTest_OnSessionOpened_010 end";
}

/**
 * @tc.name: SoftbusAgentTest_OnSessionOpened_020
 * @tc.desc: Verify the OnSessionOpened function.
 * @tc.type: FUNC
 * @tc.require: 
 */
HWTEST_F(SoftbusAgentTest, SoftbusAgentTest_OnSessionOpened_020, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusAgentTest_OnSessionOpened_010 start";
    auto mp = make_unique<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(100, SAME_ACCOUNT));
    shared_ptr<MountPoint> smp = move(mp);
    weak_ptr<MountPoint> wmp(smp);
    std::shared_ptr<SoftbusAgent> agent = std::make_shared<SoftbusAgent>(wmp);

    auto execFun = [](std::shared_ptr<SoftbusAgent> ag) {
        const int sessionId = 1;
        const int result = -1;
        int ret = ag->OnSessionOpened(sessionId, result);
        EXPECT_TRUE(ret == result);
    };

    std::thread execThread(execFun, agent);
    sleep(1);
    execThread.join();
    GTEST_LOG_(INFO) << "SoftbusAgentTest_OnSessionOpened_010 end";
}

/**
 * @tc.name: SoftbusAgentTest_OnSessionClosed_010
 * @tc.desc: Verify the OnSessionClosed function.
 * @tc.type: FUNC
 * @tc.require: 
 */
HWTEST_F(SoftbusAgentTest, SoftbusAgentTest_OnSessionClosed_010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusAgentTest_OnSessionClosed_010 start";
    auto mp = make_unique<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(100, SAME_ACCOUNT));
    shared_ptr<MountPoint> smp = move(mp);
    weak_ptr<MountPoint> wmp(smp);
    std::shared_ptr<SoftbusAgent> agent = std::make_shared<SoftbusAgent>(wmp);
    const int sessionId = 1;
    bool res = true;
    try {
        agent->OnSessionClosed(sessionId);
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }
    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "SoftbusAgentTest_OnSessionClosed_010 end";
}
} // namespace Test
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
