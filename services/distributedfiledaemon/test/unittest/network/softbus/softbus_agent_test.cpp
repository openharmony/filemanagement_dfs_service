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

#include "device_manager_agent_mock.h"
#include "device_manager_impl_mock.h"
#include "dm_constants.h"
#include "softbus_error_code.h"

#include "network/softbus/softbus_session_dispatcher.h"
#include "network/softbus/softbus_session.h"
#include "utils_log.h"
#include "socket_mock.h"

using namespace std;
namespace {
const string NETWORKID_ONE = "45656596896323231";
const string NETWORKID_TWO = "45656596896323232";
const string NETWORKID_THREE = "45656596896323233";
constexpr int32_t NETWORKTYPE_WITH_WIFI = 2;
constexpr int32_t NETWORKTYPE_NONE_WIFI = 4;
}
namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace Test {
using namespace testing;
using namespace testing::ext;
constexpr int USER_ID = 100;
constexpr int MAX_RETRY_COUNT = 7;
constexpr int DEFAULT_VALUE = -1;
static const string SAME_ACCOUNT = "account";
const string SESSION_NAME = "DistributedFileService/mnt/hmdfs/100/account";

class SoftbusAgentTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() {};
    void TearDown() {};
    void CreateDeviceList(std::vector<DmDeviceInfo> &deviceList);

    static inline shared_ptr<SocketMock> socketMock_ = nullptr;
    static inline std::shared_ptr<DeviceManagerAgentMock> deviceManagerAgentMock_ = nullptr;
    static inline shared_ptr<DeviceManagerImplMock> deviceManagerImplMock_ = nullptr;
};


void SoftbusAgentTest::CreateDeviceList(std::vector<DmDeviceInfo> &deviceList)
{
    DmDeviceInfo testInfo;
    (void)memcpy_s(testInfo.networkId, DM_MAX_DEVICE_ID_LEN - 1,
                   NETWORKID_ONE.c_str(), NETWORKID_ONE.size());
    testInfo.authForm = DmAuthForm::IDENTICAL_ACCOUNT;
    deviceList.push_back(testInfo);

    DmDeviceInfo testInfo1;
    (void)memcpy_s(testInfo1.networkId, DM_MAX_DEVICE_ID_LEN - 1,
                   NETWORKID_TWO.c_str(), NETWORKID_TWO.size());
    testInfo1.authForm = DmAuthForm::PEER_TO_PEER;
    deviceList.push_back(testInfo1);
}

void SoftbusAgentTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    socketMock_ = make_shared<SocketMock>();
    SocketMock::dfsSocket = socketMock_;
    deviceManagerAgentMock_ = std::make_shared<DeviceManagerAgentMock>();
    IDeviceManagerAgentMock::iDeviceManagerAgentMock_ = deviceManagerAgentMock_;
    deviceManagerImplMock_ = make_shared<DeviceManagerImplMock>();
    DeviceManagerImplMock::dfsDeviceManagerImpl = deviceManagerImplMock_;
}

void SoftbusAgentTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    SocketMock::dfsSocket = nullptr;
    socketMock_ = nullptr;
    IDeviceManagerAgentMock::iDeviceManagerAgentMock_ = nullptr;
    deviceManagerAgentMock_ = nullptr;
    DeviceManagerImplMock::dfsDeviceManagerImpl = nullptr;
    deviceManagerImplMock_ = nullptr;
}

/**
 * @tc.name: SoftbusAgentTest_SoftbusAgent_0100
 * @tc.desc: Verify the SoftbusAgent function.
 * @tc.type: FUNC
 * @tc.require: I9JXPR
 */
HWTEST_F(SoftbusAgentTest, SoftbusAgentTest_SoftbusAgent_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusAgentTest_SoftbusAgent_0100 start";
    auto mp = make_unique<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(USER_ID, SAME_ACCOUNT));
    shared_ptr<MountPoint> smp = move(mp);
    weak_ptr<MountPoint> wmp(smp);
    std::shared_ptr<SoftbusAgent> agent = std::make_shared<SoftbusAgent>(wmp);
    EXPECT_EQ(agent->sessionName_, SESSION_NAME);

    weak_ptr<MountPoint> nullwmp;
    std::shared_ptr<SoftbusAgent> agent2 = std::make_shared<SoftbusAgent>(nullwmp);
    EXPECT_EQ(agent2->sessionName_, SESSION_NAME);
    GTEST_LOG_(INFO) << "SoftbusAgentTest_SoftbusAgent_0100 end";
}

/**
 * @tc.name: SoftbusAgentTest_IsSameAccount_0100
 * @tc.desc: Verify the IsSameAccount.
 * @tc.type: FUNC
 * @tc.require: I9JXPR
 */
HWTEST_F(SoftbusAgentTest, SoftbusAgentTest_IsSameAccount_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusAgentTest_IsSameAccount_0100 start";
    auto mp = make_unique<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(USER_ID, SAME_ACCOUNT));
    shared_ptr<MountPoint> smp = move(mp);
    weak_ptr<MountPoint> wmp(smp);
    std::shared_ptr<SoftbusAgent> agent = std::make_shared<SoftbusAgent>(wmp);
    bool flag = false;
#ifdef SUPPORT_SAME_ACCOUNT
    std::vector<DmDeviceInfo> deviceList;
    EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0)));
    flag = agent->IsSameAccount(NETWORKID_TWO);
    EXPECT_EQ(flag, false);

    CreateDeviceList(deviceList);
    EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0)));
    flag = agent->IsSameAccount(NETWORKID_TWO);
    EXPECT_EQ(flag, false);

    EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0)));
    flag = agent->IsSameAccount(NETWORKID_THREE);
    EXPECT_EQ(flag, false);

    EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0)));
    flag = agent->IsSameAccount(NETWORKID_ONE);
    EXPECT_EQ(flag, true);
#else
    flag = agent->IsSameAccount(NETWORKID_ONE);
    EXPECT_EQ(flag, true);
#endif
    GTEST_LOG_(INFO) << "SoftbusAgentTest_IsSameAccount_0100 end";
}

/**
 * @tc.name: SoftbusAgentTest_QuitDomain_0100
 * @tc.desc: Verify the QuitDomain.
 * @tc.type: FUNC
 * @tc.require: I9JXPR
 */
HWTEST_F(SoftbusAgentTest, SoftbusAgentTest_QuitDomain_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusAgentTest_QuitDomain_0100 start";
    auto mp = make_unique<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(USER_ID, SAME_ACCOUNT));
    shared_ptr<MountPoint> smp = move(mp);
    weak_ptr<MountPoint> wmp(smp);
    std::shared_ptr<SoftbusAgent> agent = std::make_shared<SoftbusAgent>(wmp);
    bool res = true;
    try {
        agent->QuitDomain();
    } catch (const exception &e) {
        res = false;
        GTEST_LOG_(INFO) << e.what();
    }
    EXPECT_TRUE(res == false);
    GTEST_LOG_(INFO) << "SoftbusAgentTest_QuitDomain_0100 end";
}

/**
 * @tc.name: SoftbusAgentTest_QuitDomain_0200
 * @tc.desc: Verify the QuitDomain.
 * @tc.type: FUNC
 * @tc.require: I9JXPR
 */
HWTEST_F(SoftbusAgentTest, SoftbusAgentTest_QuitDomain_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusAgentTest_QuitDomain_0200 start";
    auto mp = make_unique<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(USER_ID, SAME_ACCOUNT));
    shared_ptr<MountPoint> smp = move(mp);
    weak_ptr<MountPoint> wmp(smp);
    std::shared_ptr<SoftbusAgent> agent = std::make_shared<SoftbusAgent>(wmp);
    weak_ptr<SoftbusAgent> wsba(agent);
    string busName = "test";
    bool res = true;
    try {
        agent->serverIdMap_.insert(std::make_pair(busName, 1));
        agent->QuitDomain();
        agent->serverIdMap_.erase(busName);
    } catch (const exception &e) {
        res = false;
        GTEST_LOG_(INFO) << e.what();
    }
    EXPECT_TRUE(res == false);
    GTEST_LOG_(INFO) << "SoftbusAgentTest_QuitDomain_0200 end";
}

/**
 * @tc.name: SoftbusAgentTest_QuitDomain_0300
 * @tc.desc: Verify the QuitDomain.
 * @tc.type: FUNC
 * @tc.require: I9JXPR
 */
HWTEST_F(SoftbusAgentTest, SoftbusAgentTest_QuitDomain_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusAgentTest_QuitDomain_0300 start";
    auto mp = make_unique<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(USER_ID, SAME_ACCOUNT));
    shared_ptr<MountPoint> smp = move(mp);
    weak_ptr<MountPoint> wmp(smp);
    std::shared_ptr<SoftbusAgent> agent = std::make_shared<SoftbusAgent>(wmp);
    bool res = true;
    weak_ptr<SoftbusAgent> wsba(agent);
    string busName = agent->sessionName_;
    try {
        SoftbusSessionDispatcher::RegisterSessionListener(busName, wsba);
        agent->serverIdMap_.insert(std::make_pair(busName, 1));
        agent->QuitDomain();
        auto agentRlt = SoftbusSessionDispatcher::busNameToAgent_.find(busName);
        if (agentRlt != SoftbusSessionDispatcher::busNameToAgent_.end()) {
            EXPECT_TRUE(false);
        } else {
            EXPECT_TRUE(true);
        }
        agent->serverIdMap_.erase(busName);
    } catch (const exception &e) {
        res = false;
        GTEST_LOG_(INFO) << e.what();
    }
    EXPECT_TRUE(res);
    GTEST_LOG_(INFO) << "SoftbusAgentTest_QuitDomain_0300 end";
}

/**
 * @tc.name: SoftbusAgentTest_OnSessionClosed_0100
 * @tc.desc: Verify the OnSessionClosed function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(SoftbusAgentTest, SoftbusAgentTest_OnSessionClosed_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusAgentTest_OnSessionClosed_0100 start";
    auto mp = make_unique<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(USER_ID, SAME_ACCOUNT));
    shared_ptr<MountPoint> smp = move(mp);
    weak_ptr<MountPoint> wmp(smp);
    std::shared_ptr<SoftbusAgent> agent = std::make_shared<SoftbusAgent>(wmp);
    const int sessionId = 1;
    bool res = true;
    std::string peerDeviceId = "f6d4c0864707aefte7a78f09473aa122ff57fc8";
    try {
        agent->OnSessionClosed(sessionId, peerDeviceId);
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }
    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "SoftbusAgentTest_OnSessionClosed_0100 end";
}

/**
 * @tc.name: SoftbusAgentTest_JoinDomain_0100
 * @tc.desc: Verify the Start function.
 * @tc.type: FUNC
 * @tc.require: I9JXPR
 */
HWTEST_F(SoftbusAgentTest, SoftbusAgentTest_JoinDomain_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusAgentTest_JoinDomain_0100 start";
    auto mp = make_unique<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(USER_ID, SAME_ACCOUNT));
    shared_ptr<MountPoint> smp = move(mp);
    weak_ptr<MountPoint> wmp(smp);
    std::shared_ptr<SoftbusAgent> agent = std::make_shared<SoftbusAgent>(wmp);
    bool res = true;
    try {
        EXPECT_CALL(*socketMock_, Socket(_)).WillOnce(Return(-1));
        agent->JoinDomain();
        EXPECT_NE(SoftbusSessionDispatcher::busNameToAgent_.find(agent->sessionName_),
            SoftbusSessionDispatcher::busNameToAgent_.end());
        EXPECT_EQ(agent->serverIdMap_.find(agent->sessionName_),
            agent->serverIdMap_.end());
        SoftbusSessionDispatcher::busNameToAgent_.erase(agent->sessionName_);

        EXPECT_CALL(*socketMock_, Socket(_)).WillOnce(Return(1));
        EXPECT_CALL(*socketMock_, Listen(_, _, _, _)).WillOnce(Return(0));
        agent->JoinDomain();
        EXPECT_NE(agent->serverIdMap_.find(agent->sessionName_),
            agent->serverIdMap_.end());
        agent->serverIdMap_.erase(agent->sessionName_);
        SoftbusSessionDispatcher::busNameToAgent_.erase(agent->sessionName_);
        GTEST_LOG_(INFO) << "3";
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }
    EXPECT_TRUE(res);
    GTEST_LOG_(INFO) << "SoftbusAgentTest_JoinDomain_0100 end";
}

/**
 * @tc.name: SoftbusAgentTest_JoinDomain_0200
 * @tc.desc: Verify the Start function.
 * @tc.type: FUNC
 * @tc.require: I9JXPR
 */
HWTEST_F(SoftbusAgentTest, SoftbusAgentTest_JoinDomain_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusAgentTest_JoinDomain_0200 start";
    auto mp = make_unique<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(USER_ID, SAME_ACCOUNT));
    shared_ptr<MountPoint> smp = move(mp);
    weak_ptr<MountPoint> wmp(smp);
    std::shared_ptr<SoftbusAgent> agent = std::make_shared<SoftbusAgent>(wmp);
    bool res = true;
    try {
        EXPECT_CALL(*socketMock_, Socket(_)).WillOnce(Return(1));
        EXPECT_CALL(*socketMock_, Listen(_, _, _, _)).WillOnce(Return(-1));
        agent->JoinDomain();
    } catch (...) {
        res = false;
        EXPECT_NE(SoftbusSessionDispatcher::busNameToAgent_.find(agent->sessionName_),
            SoftbusSessionDispatcher::busNameToAgent_.end());
        SoftbusSessionDispatcher::busNameToAgent_.erase(agent->sessionName_);
    }
    EXPECT_FALSE(res);
    GTEST_LOG_(INFO) << "SoftbusAgentTest_JoinDomain_0200 end";
}

/**
 * @tc.name: SoftbusAgentTest_Stop_0100
 * @tc.desc: Verify the Stop function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(SoftbusAgentTest, SoftbusAgentTest_Stop_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusAgentTest_Stop_0100 start";
    auto mp = make_unique<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(USER_ID, SAME_ACCOUNT));
    shared_ptr<MountPoint> smp = move(mp);
    weak_ptr<MountPoint> wmp(smp);
    std::shared_ptr<SoftbusAgent> agent = std::make_shared<SoftbusAgent>(wmp);
    bool res = true;
    try {
        SoftbusSessionDispatcher::busNameToAgent_.erase(agent->sessionName_);
        agent->Stop();
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }
    EXPECT_FALSE(res);
    GTEST_LOG_(INFO) << "SoftbusAgentTest_Stop_0100 end";
}

/**
 * @tc.name: SoftbusAgentTest_ConnectOnlineDevices_0100
 * @tc.desc: Verify the ConnectOnlineDevices function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(SoftbusAgentTest, SoftbusAgentTest_ConnectOnlineDevices_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusAgentTest_ConnectOnlineDevices_0100 start";
    auto mp = make_unique<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(USER_ID, SAME_ACCOUNT));
    shared_ptr<MountPoint> smp = move(mp);
    weak_ptr<MountPoint> wmp(smp);
    std::shared_ptr<SoftbusAgent> agent = std::make_shared<SoftbusAgent>(wmp);
    bool res = true;
    try {
        vector<DeviceInfo> deviceInfos;
        DeviceInfo deviceInfo;
        deviceInfo.SetCid("test");
        deviceInfos.emplace_back(deviceInfo);
        EXPECT_CALL(*deviceManagerAgentMock_, GetRemoteDevicesInfo())
            .WillOnce(Return(deviceInfos));
        agent->ConnectOnlineDevices();

        deviceInfo.extraData_ = "  ";
        deviceInfos.clear();
        deviceInfos.emplace_back(deviceInfo);
        EXPECT_CALL(*deviceManagerAgentMock_, GetRemoteDevicesInfo())
            .WillOnce(Return(deviceInfos));
        agent->ConnectOnlineDevices();

        deviceInfo.extraData_ = "{\"OS_TYPE\":\"test\"}";
        deviceInfos.clear();
        deviceInfos.emplace_back(deviceInfo);
        EXPECT_CALL(*deviceManagerAgentMock_, GetRemoteDevicesInfo())
            .WillOnce(Return(deviceInfos));
        agent->ConnectOnlineDevices();

        deviceInfo.extraData_ = "{\"OS_TYPE\":0}";
        deviceInfos.clear();
        deviceInfos.emplace_back(deviceInfo);
        EXPECT_CALL(*deviceManagerAgentMock_, GetRemoteDevicesInfo())
            .WillOnce(Return(deviceInfos));
        agent->ConnectOnlineDevices();
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }
    EXPECT_TRUE(res);
    GTEST_LOG_(INFO) << "SoftbusAgentTest_ConnectOnlineDevices_0100 end";
}

/**
 * @tc.name: SoftbusAgentTest_ConnectOnlineDevices_0200
 * @tc.desc: Verify the ConnectOnlineDevices function.
 * @tc.type: FUNC
 * @tc.require: I9JXPR
 */
HWTEST_F(SoftbusAgentTest, SoftbusAgentTest_ConnectOnlineDevices_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusAgentTest_ConnectOnlineDevices_0200 start";
    auto mp = make_unique<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(USER_ID, SAME_ACCOUNT));
    shared_ptr<MountPoint> smp = move(mp);
    weak_ptr<MountPoint> wmp(smp);
    std::shared_ptr<SoftbusAgent> agent = std::make_shared<SoftbusAgent>(wmp);
    bool res = true;
    try {
        vector<DeviceInfo> deviceInfos;
        DeviceInfo deviceInfo;
        deviceInfo.SetCid("test");
        deviceInfo.extraData_ = "{\"OS_TYPE\":10}";
        deviceInfos.emplace_back(deviceInfo);
        EXPECT_CALL(*deviceManagerAgentMock_, GetRemoteDevicesInfo())
            .WillOnce(Return(deviceInfos));
        EXPECT_CALL(*deviceManagerImplMock_, GetNetworkTypeByNetworkId(_, _, _))
            .WillOnce(Return(DEFAULT_VALUE));
        agent->ConnectOnlineDevices();

        deviceInfo.extraData_ = "{\"OS_TYPE\":10}";
        deviceInfos.clear();
        deviceInfos.emplace_back(deviceInfo);
        int32_t networkType = 0;
        networkType = NETWORKTYPE_WITH_WIFI;
        EXPECT_CALL(*deviceManagerAgentMock_, GetRemoteDevicesInfo())
            .WillOnce(Return(deviceInfos));
        EXPECT_CALL(*deviceManagerImplMock_, GetNetworkTypeByNetworkId(_, _, _))
            .WillOnce(DoAll(SetArgReferee<2>(networkType), Return(0)));
        agent->ConnectOnlineDevices();

        deviceInfo.extraData_ = "{\"OS_TYPE\":10}";
        deviceInfos.clear();
        deviceInfos.emplace_back(deviceInfo);
        networkType = NETWORKTYPE_NONE_WIFI;
        EXPECT_CALL(*deviceManagerAgentMock_, GetRemoteDevicesInfo())
            .WillOnce(Return(deviceInfos));
        EXPECT_CALL(*deviceManagerImplMock_, GetNetworkTypeByNetworkId(_, _, _))
            .WillOnce(DoAll(SetArgReferee<2>(networkType), Return(0)));
        agent->ConnectOnlineDevices();
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }
    EXPECT_TRUE(res);
    GTEST_LOG_(INFO) << "SoftbusAgentTest_ConnectOnlineDevices_0200 end";
}

/**
 * @tc.name: SoftbusAgentTest_DisconnectAllDevices_0100
 * @tc.desc: Verify the DisconnectAllDevices function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(SoftbusAgentTest, SoftbusAgentTest_DisconnectAllDevices_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusAgentTest_DisconnectAllDevices_0100 start";
    auto mp = make_unique<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(USER_ID, SAME_ACCOUNT));
    shared_ptr<MountPoint> smp = move(mp);
    weak_ptr<MountPoint> wmp(smp);
    std::shared_ptr<SoftbusAgent> agent = std::make_shared<SoftbusAgent>(wmp);
    bool res = true;
    try {
        agent->DisconnectAllDevices();
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }
    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "SoftbusAgentTest_DisconnectAllDevices_0100 end";
}

/**
 * @tc.name: SoftbusAgentTest_AcceptSession_0100
 * @tc.desc: Verify the AcceptSession function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(SoftbusAgentTest, SoftbusAgentTest_AcceptSession_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusAgentTest_AcceptSession_0100 start";
    auto mp = make_unique<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(USER_ID, SAME_ACCOUNT));
    shared_ptr<MountPoint> smp = move(mp);
    weak_ptr<MountPoint> wmp(smp);
    std::shared_ptr<SoftbusAgent> agent = std::make_shared<SoftbusAgent>(wmp);
    const int testSessionId = 99;
    std::string peerDeviceId = "f6d4c0864707aefte7a78f09473aa122ff57fc8";
    auto session = make_shared<SoftbusSession>(testSessionId, peerDeviceId);
    bool res = true;
    try {
        agent->AcceptSession(session, "Server");
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }
    EXPECT_TRUE(res);
    GTEST_LOG_(INFO) << "SoftbusAgentTest_AcceptSession_0100 end";
}

/**
 * @tc.name: SoftbusAgentTest_GetMountPoint_0100
 * @tc.desc: Verify the GetMountPoint function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(SoftbusAgentTest, SoftbusAgentTest_GetMountPoint_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusAgentTest_GetMountPoint_0100 start";
    auto mp = make_unique<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(USER_ID, SAME_ACCOUNT));
    shared_ptr<MountPoint> smp = move(mp);
    weak_ptr<MountPoint> wmp(smp);
    std::shared_ptr<SoftbusAgent> agent = std::make_shared<SoftbusAgent>(wmp);
    bool res = true;
    try {
        agent->GetMountPoint();
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }
    EXPECT_TRUE(res);
    GTEST_LOG_(INFO) << "SoftbusAgentTest_GetMountPoint_0100 end";
}

/**
 * @tc.name: SoftbusAgentTest_CloseSession_0100
 * @tc.desc: Verify the CloseSession function.
 * @tc.type: FUNC
 * @tc.require: issueI7SP3A
 */
HWTEST_F(SoftbusAgentTest, SoftbusAgentTest_CloseSession_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusAgentTest_CloseSession_0100 start";
    auto mp = make_unique<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(USER_ID, SAME_ACCOUNT));
    shared_ptr<MountPoint> smp = move(mp);
    weak_ptr<MountPoint> wmp(smp);
    std::shared_ptr<SoftbusAgent> agent = std::make_shared<SoftbusAgent>(wmp);
    try {
        agent->CloseSession(nullptr);
        EXPECT_TRUE(true);
    } catch (const exception &e) {
        LOGE("%{public}s", e.what());
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "SoftbusAgentTest_CloseSession_0100 end";
}

/**
 * @tc.name: SoftbusAgentTest_IsContinueRetry_0100
 * @tc.desc: Verify the IsContinueRetry function.
 * @tc.type: FUNC
 * @tc.require: issueI7SP3A
 */
HWTEST_F(SoftbusAgentTest, SoftbusAgentTest_IsContinueRetry_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusAgentTest_IsContinueRetry_0100 start";
    auto mp = make_unique<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(USER_ID, SAME_ACCOUNT));
    shared_ptr<MountPoint> smp = move(mp);
    weak_ptr<MountPoint> wmp(smp);
    std::shared_ptr<SoftbusAgent> agent = std::make_shared<SoftbusAgent>(wmp);
    string cid = "notExitCid";
    try {
        bool ret = agent->IsContinueRetry(cid);
        EXPECT_EQ(agent->OpenSessionRetriedTimesMap_[cid], 1);
        ret = agent->IsContinueRetry(cid);
        EXPECT_EQ(ret, true);
        agent->OpenSessionRetriedTimesMap_[cid] = MAX_RETRY_COUNT;
        ret = agent->IsContinueRetry(cid);
        EXPECT_EQ(ret, false);
    } catch (const exception &e) {
        LOGE("%{public}s", e.what());
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "SoftbusAgentTest_IsContinueRetry_0100 end";
}
} // namespace Test
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
