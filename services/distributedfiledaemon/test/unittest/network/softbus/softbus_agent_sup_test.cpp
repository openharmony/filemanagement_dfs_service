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
#include "network/softbus/softbus_agent.h"

#include "gtest/gtest.h"
#include <memory>
#include <unistd.h>

#include "dm_constants.h"

#include "device_manager_agent_mock.h"
#include "device_manager_impl_mock.h"
#include "dm_constants.h"
#include "network/softbus/softbus_session_dispatcher.h"
#include "network/softbus/softbus_session.h"
#include "utils_log.h"

using namespace std;

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace Test {
using namespace testing::ext;
using namespace testing;

const std::string NETWORKID_ONE = "45656596896323231";
const std::string NETWORKID_TWO = "45656596896323232";
constexpr int32_t NETWORKTYPE_WITH_WIFI = 2;
constexpr int32_t NETWORKTYPE_NONE_WIFI = 4;
constexpr int USER_ID = 100;
static const string SAME_ACCOUNT = "account";

class SoftbusAgentSupTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() {};
    void TearDown() {};
    static inline std::shared_ptr<DeviceManagerAgentMock> deviceManagerAgentMock_ = nullptr;
    static inline shared_ptr<DeviceManagerImplMock> deviceManagerImplMock_ = nullptr;
};

void SoftbusAgentSupTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    deviceManagerAgentMock_ = std::make_shared<DeviceManagerAgentMock>();
    IDeviceManagerAgentMock::iDeviceManagerAgentMock_ = deviceManagerAgentMock_;
    deviceManagerImplMock_ = make_shared<DeviceManagerImplMock>();
    DeviceManagerImplMock::dfsDeviceManagerImpl = deviceManagerImplMock_;
}

void SoftbusAgentSupTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    IDeviceManagerAgentMock::iDeviceManagerAgentMock_ = nullptr;
    deviceManagerAgentMock_ = nullptr;
    DeviceManagerImplMock::dfsDeviceManagerImpl = nullptr;
    deviceManagerImplMock_ = nullptr;
}

/**
 * @tc.name: SoftbusAgentSupTest_ConnectOnlineDevices_0100
 * @tc.desc: Verify the ConnectOnlineDevices function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(SoftbusAgentSupTest, SoftbusAgentSupTest_ConnectOnlineDevices_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusAgentSupTest_ConnectOnlineDevices_0100 start";
    auto mp = make_unique<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(USER_ID, SAME_ACCOUNT));
    shared_ptr<MountPoint> smp = move(mp);
    weak_ptr<MountPoint> wmp(smp);
    std::shared_ptr<SoftbusAgent> agent = std::make_shared<SoftbusAgent>(wmp);
    bool res = true;
    try {
        vector<DeviceInfo> infos;
        DeviceInfo testA;
        infos.push_back(testA);
        EXPECT_CALL(*deviceManagerAgentMock_, GetRemoteDevicesInfo()).WillOnce(Return(infos));
        agent->ConnectOnlineDevices();

        DistributedHardware::DmDeviceInfo nodeInfo{.extraData = "test"};
        DeviceInfo testB(nodeInfo);
        infos.clear();
        infos.push_back(testB);
        EXPECT_CALL(*deviceManagerAgentMock_, GetRemoteDevicesInfo()).WillOnce(Return(infos));
        agent->ConnectOnlineDevices();

        nodeInfo.extraData = R"({"OS_TYPE":"test"})";
        DeviceInfo testC(nodeInfo);
        infos.clear();
        infos.push_back(testC);
        EXPECT_CALL(*deviceManagerAgentMock_, GetRemoteDevicesInfo()).WillOnce(Return(infos));
        agent->ConnectOnlineDevices();

        nodeInfo.extraData = R"({"OS_TYPE":3})";
        DeviceInfo testD(nodeInfo);
        infos.clear();
        infos.push_back(testD);
        EXPECT_CALL(*deviceManagerAgentMock_, GetRemoteDevicesInfo()).WillOnce(Return(infos));
        agent->ConnectOnlineDevices();
    
        nodeInfo.extraData = R"({"OS_TYPE":10})";
        memset_s(nodeInfo.networkId, DM_MAX_DEVICE_ID_LEN, 0, DM_MAX_DEVICE_ID_LEN);
        (void)memcpy_s(nodeInfo.networkId, DM_MAX_DEVICE_ID_LEN - 1,
                NETWORKID_ONE.c_str(), NETWORKID_ONE.size());
        DeviceInfo testE(nodeInfo);
        infos.clear();
        infos.push_back(testE);
        EXPECT_CALL(*deviceManagerAgentMock_, GetRemoteDevicesInfo()).WillOnce(Return(infos));
        EXPECT_CALL(*deviceManagerImplMock_, GetNetworkTypeByNetworkId(_, _, _))
            .WillOnce(Return(ERR_DM_INPUT_PARA_INVALID));
        agent->ConnectOnlineDevices();
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }
    EXPECT_TRUE(res);
    GTEST_LOG_(INFO) << "SoftbusAgentSupTest_ConnectOnlineDevices_0100 end";
}

/**
 * @tc.name: SoftbusAgentSupTest_ConnectOnlineDevices_0200
 * @tc.desc: Verify the ConnectOnlineDevices function.
 * @tc.type: FUNC
 * @tc.require: I9JXPR
 */
HWTEST_F(SoftbusAgentSupTest, SoftbusAgentSupTest_ConnectOnlineDevices_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusAgentSupTest_ConnectOnlineDevices_0200 start";
    auto mp = make_unique<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(USER_ID, SAME_ACCOUNT));
    shared_ptr<MountPoint> smp = move(mp);
    weak_ptr<MountPoint> wmp(smp);
    std::shared_ptr<SoftbusAgent> agent = std::make_shared<SoftbusAgent>(wmp);
    bool res = true;
    try {
        vector<DeviceInfo> infos;
        DistributedHardware::DmDeviceInfo nodeInfo{.networkId = "testNetWork",
            .extraData = R"({"OS_TYPE":10})"};
        memset_s(nodeInfo.networkId, DM_MAX_DEVICE_ID_LEN, 0, DM_MAX_DEVICE_ID_LEN);
        (void)memcpy_s(nodeInfo.networkId, DM_MAX_DEVICE_ID_LEN - 1,
                NETWORKID_ONE.c_str(), NETWORKID_ONE.size());
        DeviceInfo testC(nodeInfo);
        infos.push_back(testC);
        memset_s(nodeInfo.networkId, DM_MAX_DEVICE_ID_LEN, 0, DM_MAX_DEVICE_ID_LEN);
        (void)memcpy_s(nodeInfo.networkId, DM_MAX_DEVICE_ID_LEN - 1,
                NETWORKID_TWO.c_str(), NETWORKID_TWO.size());
        DeviceInfo testD(nodeInfo);
        infos.push_back(testD);

        EXPECT_CALL(*deviceManagerAgentMock_, GetRemoteDevicesInfo()).WillOnce(Return(infos));
        EXPECT_CALL(*deviceManagerImplMock_, GetNetworkTypeByNetworkId(_, _, _))
            .WillOnce(DoAll(SetArgReferee<2>(NETWORKTYPE_WITH_WIFI), Return(DM_OK)))
            .WillOnce(DoAll(SetArgReferee<2>(NETWORKTYPE_NONE_WIFI), Return(DM_OK)));
        agent->ConnectOnlineDevices();
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }
    EXPECT_TRUE(res);
    GTEST_LOG_(INFO) << "SoftbusAgentSupTest_ConnectOnlineDevices_0200 end";
}
} // namespace Test
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
