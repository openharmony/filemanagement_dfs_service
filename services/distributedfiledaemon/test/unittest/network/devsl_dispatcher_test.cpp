/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#include <cstdio>
#include <unistd.h>

#include "dfs_error.h"
#include "gtest/gtest.h"
#include "network/devsl_dispatcher.h"
#include "network/kernel_talker.h"
#include "softbus_bus_center.h"
#include "utils_log.h"

namespace {
    int32_t g_getHighestSecLevel;
}

int32_t DATASL_GetHighestSecLevel(DEVSLQueryParams *queryParams, uint32_t *levelInfo)
{
    return g_getHighestSecLevel;
}

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace Test {
using namespace testing::ext;
using namespace std;

class DevslDispatcherTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.name: DevslDispatcherTest_Start_Stop_0100
 * @tc.desc: Verify the Start/Stop function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(DevslDispatcherTest, DevslDispatcherTest_Start_Stop_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DevslDispatcherTest_Start_Stop_0100 start";
    bool res = true;
    try {
        DevslDispatcher::Start();
        DevslDispatcher::Stop();
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }
    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "DevslDispatcherTest_Start_Stop_0100 end";
}

/**
 * @tc.name: DevslDispatcherTest_MakeDevslQueryParams_0100
 * @tc.desc: Verify the MakeDevslQueryParams function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(DevslDispatcherTest, DevslDispatcherTest_MakeDevslQueryParams_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DevslDispatcherTest_MakeDevslQueryParams_0100 start";
    bool res = true;
    string udid = "0123";
    try {
        DEVSLQueryParams queryParam = DevslDispatcher::MakeDevslQueryParams(udid);
        EXPECT_TRUE(queryParam.udidLen == udid.size());
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }
    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "DevslDispatcherTest_MakeDevslQueryParams_0100 end";
}

/**
 * @tc.name: DevslDispatcherTest_DevslGottonCallbackAsync_0100
 * @tc.desc: Verify the DevslGottonCallbackAsync function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(DevslDispatcherTest, DevslDispatcherTest_DevslGottonCallbackAsync_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DevslDispatcherTest_DevslGottonCallbackAsync_0100 start";
    bool res = true;
    string udid = "0";
    uint32_t devsl = 1;
    try {
        DevslDispatcher::DevslGottonCallbackAsync(udid, devsl);
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }
    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "DevslDispatcherTest_DevslGottonCallbackAsync_0100 end";
}

/**
 * @tc.name: DevslDispatcherTest_DevslGottonCallback_0100
 * @tc.desc: Verify the DevslGottonCallback function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(DevslDispatcherTest, DevslDispatcherTest_DevslGottonCallback_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DevslDispatcherTest_DevslGottonCallback_0100 start";
    bool res = true;
    uint32_t level = 0;
    DEVSLQueryParams queryParam = {
        .udid = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0},
        .udidLen = 10
    };

    try {
        DevslDispatcher::DevslGottonCallback(&queryParam, 0, level);
        DevslDispatcher::DevslGottonCallback(&queryParam, 1, level);
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }

    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "DevslDispatcherTest_DevslGottonCallback_0100 end";
}

/**
 * @tc.name: DevslDispatcherTest_DevslGetRegister_0100
 * @tc.desc: Verify the DevslGetRegister function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(DevslDispatcherTest, DevslDispatcherTest_DevslGetRegister_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DevslDispatcherTest_DevslGetRegister_0100 start";
    bool res = true;
    const string cid = "scid";
    constexpr int userId = 100;
    auto smp = make_shared<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(userId, "account"));
    weak_ptr<MountPoint> wmp = smp;
    auto kernelTalker = std::make_shared<KernelTalker>(wmp, [](NotifyParam &param) {}, [](const std::string &) {});

    try {
        DevslDispatcher::DevslGetRegister(cid, kernelTalker);
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }

    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "DevslDispatcherTest_DevslGetRegister_0100 end";
}

/**
 * @tc.name: DevslDispatcherTest_CompareDevslWithLocal_0100
 * @tc.desc: Verify the CompareDevslWithLocal function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(DevslDispatcherTest, DevslDispatcherTest_CompareDevslWithLocal_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DevslDispatcherTest_CompareDevslWithLocal_0100 start";
    EXPECT_FALSE(DevslDispatcher::CompareDevslWithLocal("test", {}));

    DevslDispatcher::devslMap_["test"] = FileManagement::ERR_BAD_VALUE;
    EXPECT_FALSE(DevslDispatcher::CompareDevslWithLocal("test", {"path"}));

    DevslDispatcher::devslMap_["test"] = 1;
    EXPECT_FALSE(DevslDispatcher::CompareDevslWithLocal("test", {"path"}));

    DevslDispatcher::devslMap_["test"] = static_cast<int32_t>(SecurityLabel::S4);
    EXPECT_TRUE(DevslDispatcher::CompareDevslWithLocal("test", {"path"}));

    GTEST_LOG_(INFO) << "DevslDsispatcherTest_CompareDevslWithLocal_0100 end";
}

/**
 * @tc.name: DevslDispatcherTest_GetDeviceDevsl_0100
 * @tc.desc: Verify the GetDeviceDevsl function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(DevslDispatcherTest, DevslDispatcherTest_GetDeviceDevsl_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DevslDispatcherTest_GetDeviceDevsl_0100 start";
    EXPECT_EQ(DevslDispatcher::GetDeviceDevsl(""), -1);

    DevslDispatcher::devslMap_.clear();
    DevslDispatcher::devslMap_.insert(make_pair("test", 1));
    EXPECT_EQ(DevslDispatcher::GetDeviceDevsl("test"), 1);

    DevslDispatcher::devslMap_.clear();
    g_getHighestSecLevel = FileManagement::ERR_BAD_VALUE;
    EXPECT_EQ(DevslDispatcher::GetDeviceDevsl("test"), FileManagement::ERR_BAD_VALUE);

    DevslDispatcher::devslMap_.clear();
    g_getHighestSecLevel = FileManagement::E_OK;
    EXPECT_EQ(DevslDispatcher::GetDeviceDevsl("test"), FileManagement::E_OK);
    GTEST_LOG_(INFO) << "DevslDsispatcherTest_GetDeviceDevsl_0100 end";
}
} // namespace Test
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
