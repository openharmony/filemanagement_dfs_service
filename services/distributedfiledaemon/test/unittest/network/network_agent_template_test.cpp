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

#include <memory>
#include <unistd.h>

#include "gtest/gtest.h"
#include "network/network_agent_template.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace Test {
using namespace testing::ext;
using namespace std;

constexpr int TEST_SESSION_ID = 10;

class NetworkAgentTemplateTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.name: NetworkAgentTemplateTest_Start_Stop_0100
 * @tc.desc: Verify the Start/Stop function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(NetworkAgentTemplateTest, NetworkAgentTemplateTest_Start_Stop_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NetworkAgentTemplateTest_Start_Stop_0100 start";
    bool res = true;
    try {
        NetworkAgentTemplate::Start();
        NetworkAgentTemplate::Stop();
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }
    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "NetworkAgentTemplateTest_Start_Stop_0100 end";
}

/**
 * @tc.name: NetworkAgentTemplateTest_ConnectDeviceAsync_0100
 * @tc.desc: Verify the ConnectDeviceAsync function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(NetworkAgentTemplateTest, NetworkAgentTemplateTest_ConnectDeviceAsync_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NetworkAgentTemplateTest_ConnectDeviceAsync_0100 start";
    DistributedHardware::DmDeviceInfo info = {
        .deviceId = "testdevid",
        .deviceName = "testdevname",
        .deviceTypeId = 1,
    };
    DeviceInfo devInfo(info);
    bool res = true;
    try {
        NetworkAgentTemplate::ConnectDeviceAsync(devInfo);
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }
    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "NetworkAgentTemplateTest_ConnectDeviceAsync_0100 end";
}

/**
 * @tc.name: NetworkAgentTemplateTest_ConnectDeviceByP2PAsync_0100
 * @tc.desc: Verify the ConnectDeviceByP2PAsync function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(NetworkAgentTemplateTest, NetworkAgentTemplateTest_ConnectDeviceByP2PAsync_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NetworkAgentTemplateTest_ConnectDeviceByP2PAsync_0100 start";
    DistributedHardware::DmDeviceInfo info = {
        .deviceId = "testdevid",
        .deviceName = "testdevname",
        .deviceTypeId = 1,
    };
    DeviceInfo devInfo(info);
    bool res = true;
    try {
        NetworkAgentTemplate::ConnectDeviceByP2PAsync(devInfo);
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }
    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "NetworkAgentTemplateTest_ConnectDeviceByP2PAsync_0100 end";
}

/**
 * @tc.name: NetworkAgentTemplateTest_ConnectOnlineDevices_0100
 * @tc.desc: Verify the ConnectOnlineDevices function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(NetworkAgentTemplateTest, NetworkAgentTemplateTest_ConnectDeviceAsync_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NetworkAgentTemplateTest_ConnectDeviceAsync_0100 start";
    bool res = true;
    try {
        NetworkAgentTemplate::ConnectOnlineDevices();
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }
    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "NetworkAgentTemplateTest_ConnectDeviceAsync_0100 end";
}

/**
 * @tc.name: NetworkAgentTemplateTest_DisconnectAllDevices_0100
 * @tc.desc: Verify the DisconnectAllDevices function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(NetworkAgentTemplateTest, NetworkAgentTemplateTest_DisconnectAllDevices_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NetworkAgentTemplateTest_DisconnectAllDevices_0100 start";
    bool res = true;
    try {
        NetworkAgentTemplate::DisconnectAllDevices();
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }
    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "NetworkAgentTemplateTest_DisconnectAllDevices_0100 end";
}

/**
 * @tc.name: NetworkAgentTemplateTest_DisconnectDevice_0100
 * @tc.desc: Verify the DisconnectDevice function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(NetworkAgentTemplateTest, NetworkAgentTemplateTest_DisconnectDevice_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NetworkAgentTemplateTest_DisconnectDevice_0100 start";
    DistributedHardware::DmDeviceInfo info = {
        .deviceId = "testdevid",
        .deviceName = "testdevname",
        .deviceTypeId = 1,
    };
    DeviceInfo devInfo(info);
    bool res = true;
    try {
        NetworkAgentTemplate::DisconnectDevice(devInfo);
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }
    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "NetworkAgentTemplateTest_DisconnectDevice_0100 end";
}

/**
 * @tc.name: NetworkAgentTemplateTest_DisconnectDeviceByP2P_0100
 * @tc.desc: Verify the DisconnectDeviceByP2P function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(NetworkAgentTemplateTest, NetworkAgentTemplateTest_DisconnectDeviceByP2P_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NetworkAgentTemplateTest_DisconnectDeviceByP2P_0100 start";
    DistributedHardware::DmDeviceInfo info = {
        .deviceId = "testdevid",
        .deviceName = "testdevname",
        .deviceTypeId = 1,
    };
    DeviceInfo devInfo(info);
    bool res = true;
    try {
        NetworkAgentTemplate::DisconnectDeviceByP2P(devInfo);
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }
    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "NetworkAgentTemplateTest_DisconnectDeviceByP2P_0100 end";
}

/**
 * @tc.name: NetworkAgentTemplateTest_OccupySession_0100
 * @tc.desc: Verify the OccupySession function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(NetworkAgentTemplateTest, NetworkAgentTemplateTest_OccupySession_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NetworkAgentTemplateTest_OccupySession_0100 start";
    bool res = true;
    try {
        NetworkAgentTemplate::OccupySession(TEST_SESSION_ID, 1);
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }
    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "NetworkAgentTemplateTest_OccupySession_0100 end";
}

/**
 * @tc.name: NetworkAgentTemplateTest_FindSession_0100
 * @tc.desc: Verify the FindSession function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(NetworkAgentTemplateTest, NetworkAgentTemplateTest_FindSession_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NetworkAgentTemplateTest_FindSession_0100 start";
    bool res = true;
    try {
        NetworkAgentTemplate::FindSession(TEST_SESSION_ID);
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }
    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "NetworkAgentTemplateTest_FindSession_0100 end";
}

/**
 * @tc.name: NetworkAgentTemplateTest_CloseSessionForOneDevice_0100
 * @tc.desc: Verify the CloseSessionForOneDevice function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(NetworkAgentTemplateTest, NetworkAgentTemplateTest_CloseSessionForOneDevice_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NetworkAgentTemplateTest_CloseSessionForOneDevice_0100 start";
    bool res = true;
    try {
        NetworkAgentTemplate::CloseSessionForOneDevice("testSession");
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }
    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "NetworkAgentTemplateTest_CloseSessionForOneDevice_0100 end";
}

/**
 * @tc.name: NetworkAgentTemplateTest_AcceptSession_0100
 * @tc.desc: Verify the AcceptSession function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(NetworkAgentTemplateTest, NetworkAgentTemplateTest_AcceptSession_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NetworkAgentTemplateTest_AcceptSession_0100 start";
    auto session = make_shared<SoftbusSession>(TEST_SESSION_ID);
    bool res = true;
    try {
        NetworkAgentTemplate::AcceptSession(session);
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }
    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "NetworkAgentTemplateTest_AcceptSession_0100 end";
}

/**
 * @tc.name: NetworkAgentTemplateTest_AcceptSessionInner_0100
 * @tc.desc: Verify the AcceptSessionInner function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(NetworkAgentTemplateTest, NetworkAgentTemplateTest_AcceptSessionInner_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NetworkAgentTemplateTest_AcceptSessionInner_0100 start";
    auto session = make_shared<SoftbusSession>(TEST_SESSION_ID);
    bool res = true;
    try {
        NetworkAgentTemplate::AcceptSessionInner(session);
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }
    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "NetworkAgentTemplateTest_AcceptSessionInner_0100 end";
}

/**
 * @tc.name: NetworkAgentTemplateTest_GetSessionProcess_0100
 * @tc.desc: Verify the GetSessionProcess function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(NetworkAgentTemplateTest, NetworkAgentTemplateTest_GetSessionProcess_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NetworkAgentTemplateTest_GetSessionProcess_0100 start";
    NotifyParam param;
    bool res = true;
    try {
        NetworkAgentTemplate::GetSessionProcess(param);
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }
    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "NetworkAgentTemplateTest_GetSessionProcess_0100 end";
}

/**
 * @tc.name: NetworkAgentTemplateTest_GetSessionProcessInner_0100
 * @tc.desc: Verify the GetSessionProcessInner function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(NetworkAgentTemplateTest, NetworkAgentTemplateTest_GetSessionProcessInner_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NetworkAgentTemplateTest_GetSessionProcessInner_0100 start";
    NotifyParam param;
    bool res = true;
    try {
        NetworkAgentTemplate::GetSessionProcessInner(param);
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }
    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "NetworkAgentTemplateTest_GetSessionProcessInner_0100 end";
}

/**
 * @tc.name: NetworkAgentTemplateTest_GetSession_0100
 * @tc.desc: Verify the GetSession function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(NetworkAgentTemplateTest, NetworkAgentTemplateTest_GetSession_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NetworkAgentTemplateTest_GetSession_0100 start";
    bool res = true;
    try {
        NetworkAgentTemplate::GetSession("testsession", 1);
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }
    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "NetworkAgentTemplateTest_GetSession_0100 end";
}
} // namespace Test
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
