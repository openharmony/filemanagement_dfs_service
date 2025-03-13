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
#include "device/device_manager_agent.h"

#include <atomic>
#include <exception>
#include <memory>
#include <unistd.h>

#include "gtest/gtest.h"

#include "device_manager_impl_mock.h"
#include "mountpoint/mount_point.h"
#include "network/softbus/softbus_agent.h"

namespace OHOS {
bool g_parameterVaule = true;
namespace system {
bool GetBoolParameter(const std::string& key, bool def)
{
    return g_parameterVaule;
}
}
}

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

DistributedHardware::DmDeviceInfo deviceInfo = {
    .deviceId = "testdevid",
    .deviceName = "testdevname",
    .deviceTypeId = 1,
    .networkId = "testNetWork",
    .extraData = R"({"OS_TYPE":10})",
};

const std::string NETWORKID_ONE = "45656596896323231";
const std::string NETWORKID_TWO = "45656596896323232";
const std::string NETWORKID_THREE = "45656596896323233";
constexpr int32_t NETWORKTYPE_WITH_WIFI = 2;
constexpr int32_t NETWORKTYPE_NONE_WIFI = 4;

class DeviceManagerAgentTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<DeviceManagerImplMock> deviceManagerImplMock_ = nullptr;
};

void DeviceManagerAgentTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
    deviceManagerImplMock_ = make_shared<DeviceManagerImplMock>();
    DeviceManagerImplMock::dfsDeviceManagerImpl = deviceManagerImplMock_;
    EXPECT_CALL(*deviceManagerImplMock_, InitDeviceManager(_, _)).WillRepeatedly(Return(0));
    EXPECT_CALL(*deviceManagerImplMock_, RegisterDevStateCallback(_, _, _)).WillRepeatedly(Return(0));
    EXPECT_CALL(*deviceManagerImplMock_, UnRegisterDevStateCallback(_)).WillRepeatedly(Return(0));
    EXPECT_CALL(*deviceManagerImplMock_, UnInitDeviceManager(_)).WillRepeatedly(Return(0));
    EXPECT_CALL(*deviceManagerImplMock_, UnInitDeviceManager(_)).WillRepeatedly(Return(0));
    EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _)).WillRepeatedly(Return(0));
}

void DeviceManagerAgentTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
    DeviceManagerImplMock::dfsDeviceManagerImpl = nullptr;
    deviceManagerImplMock_ = nullptr;
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
 * @tc.name: DeviceManagerAgentTest_OnDeviceOnline_0100
 * @tc.desc: Verify the OnDeviceOnline function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_OnDeviceOnline_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_OnDeviceOnline_0100 start";
    bool res = true;

    try {
        DeviceManagerAgent::GetInstance()->OnDeviceOnline(deviceInfo);
    } catch (const exception &e) {
        LOGE("Error:%{public}s", e.what());
        res = false;
    }

    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_OnDeviceOnline_0100 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_OnDeviceOnline_0400
 * @tc.desc: Verify the OnDeviceOnline function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_OnDeviceOnline_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_OnDeviceOnline_0400 start";
    bool res = true;

    try {
        auto smp = make_shared<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(100, "relativePath"));
        auto agent1 = make_shared<SoftbusAgent>(smp);
        (void)memcpy_s(deviceInfo.networkId, DM_MAX_DEVICE_NAME_LEN - 1,
                       NETWORKID_TWO.c_str(), NETWORKID_TWO.size());
        DeviceManagerAgent::GetInstance()->cidNetTypeRecord_.insert({ NETWORKID_TWO, agent1 });
        DeviceManagerAgent::GetInstance()->cidNetworkType_.insert({ NETWORKID_TWO, NETWORKTYPE_NONE_WIFI });
        DeviceManagerAgent::GetInstance()->OnDeviceOnline(deviceInfo);
        DeviceManagerAgent::GetInstance()->cidNetTypeRecord_.erase(NETWORKID_TWO);
        DeviceManagerAgent::GetInstance()->cidNetworkType_.erase(NETWORKID_TWO);
    } catch (const exception &e) {
        GTEST_LOG_(INFO) << e.what();
        res = false;
    }

    EXPECT_TRUE(res);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_OnDeviceOnline_0400 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_OnDeviceOffline_0100
 * @tc.desc: Verify the OnDeviceOffline function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_OnDeviceOffline_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_OnDeviceOffline_0100 start";
    bool res = true;

    try {
        DeviceManagerAgent::GetInstance()->OnDeviceOffline(deviceInfo);
    } catch (const exception &e) {
        LOGE("Error:%{public}s", e.what());
        res = false;
    }

    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_OnDeviceOffline_0100 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_OnDeviceOffline_0200
 * @tc.desc: Verify the OnDeviceOffline function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_OnDeviceOffline_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_OnDeviceOffline_0200 start";
    bool res = true;

    try {
        DeviceManagerAgent::GetInstance()->OnDeviceOnline(deviceInfo);
        DeviceManagerAgent::GetInstance()->OnDeviceOffline(deviceInfo);
    } catch (const exception &e) {
        LOGE("Error:%{public}s", e.what());
        res = false;
    }

    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_OnDeviceOffline_0200 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_OnDeviceChanged_0100
 * @tc.desc: Verify the OnDeviceChanged function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_OnDeviceChanged_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_OnDeviceChanged_0100 start";
    bool res = true;

    try {
        DeviceManagerAgent::GetInstance()->OnDeviceChanged(deviceInfo);
    } catch (const exception &e) {
        LOGE("Error:%{public}s", e.what());
        res = false;
    }

    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_OnDeviceChanged_0100 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_OnDeviceChanged_0200
 * @tc.desc: Verify the OnDeviceChanged function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_OnDeviceChanged_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_OnDeviceChanged_0200 start";
    bool res = true;

    try {
        deviceInfo.networkType = -1;
        DeviceManagerAgent::GetInstance()->OnDeviceChanged(deviceInfo);
        deviceInfo.networkType = 1;
        DeviceManagerAgent::GetInstance()->OnDeviceChanged(deviceInfo);
    } catch (const exception &e) {
        GTEST_LOG_(INFO) << e.what();
        res = false;
    }

    EXPECT_TRUE(res);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_OnDeviceChanged_0200 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_OnDeviceChanged_0300
 * @tc.desc: Verify the OnDeviceChanged function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_OnDeviceChanged_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_OnDeviceChanged_0300 start";
    bool res = true;

    try {
        deviceInfo.networkType = NETWORKTYPE_NONE_WIFI;
        auto smp = make_shared<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(100, "relativePath"));
        auto agent1 = make_shared<SoftbusAgent>(smp);
        (void)memcpy_s(deviceInfo.networkId, DM_MAX_DEVICE_NAME_LEN - 1,
                       NETWORKID_THREE.c_str(), NETWORKID_THREE.size());
        std::vector<DmDeviceInfo> deviceList;
        deviceList.push_back(deviceInfo);
        EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _))
            .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0)));
        auto devicePtr = DeviceManagerAgent::GetInstance();
        devicePtr->cidNetTypeRecord_.insert({ NETWORKID_THREE, agent1 });
        devicePtr->OnDeviceChanged(deviceInfo);

        devicePtr->cidNetworkType_.insert({ NETWORKID_THREE, NETWORKTYPE_NONE_WIFI });
        EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _))
            .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0)));
        devicePtr->OnDeviceChanged(deviceInfo);

        deviceInfo.networkType = NETWORKTYPE_WITH_WIFI;
        EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _))
            .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0)));
        devicePtr->OnDeviceChanged(deviceInfo);
        devicePtr->cidNetTypeRecord_.erase(NETWORKID_THREE);
        devicePtr->cidNetworkType_.erase(NETWORKID_THREE);
    } catch (const exception &e) {
        GTEST_LOG_(INFO) << e.what();
        res = false;
    }

    EXPECT_TRUE(res);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_OnDeviceChanged_0300 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_OnDeviceChanged_0400
 * @tc.desc: Verify the OnDeviceChanged function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_OnDeviceChanged_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_OnDeviceChanged_0400 start";
    bool res = true;

    try {
        deviceInfo.networkType = NETWORKTYPE_WITH_WIFI;
        auto smp = make_shared<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(100, "relativePath"));
        auto agent1 = make_shared<SoftbusAgent>(smp);
        (void)memcpy_s(deviceInfo.networkId, DM_MAX_DEVICE_NAME_LEN - 1,
                       NETWORKID_TWO.c_str(), NETWORKID_TWO.size());
        std::vector<DmDeviceInfo> deviceList;
        deviceList.push_back(deviceInfo);
        EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _))
            .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0)));
        auto devicePtr = DeviceManagerAgent::GetInstance();
        devicePtr->cidNetTypeRecord_.insert({ NETWORKID_TWO, agent1 });
        devicePtr->cidNetworkType_.insert({ NETWORKID_TWO, NETWORKTYPE_WITH_WIFI });
        devicePtr->OnDeviceChanged(deviceInfo);

        deviceInfo.networkType = NETWORKTYPE_NONE_WIFI;
        EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _))
            .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0))).WillOnce(Return(0));
        devicePtr->OnDeviceChanged(deviceInfo);
        devicePtr->cidNetTypeRecord_.erase(NETWORKID_TWO);
        devicePtr->cidNetworkType_.erase(NETWORKID_TWO);
    } catch (const exception &e) {
        GTEST_LOG_(INFO) << e.what();
        res = false;
    }

    EXPECT_TRUE(res);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_OnDeviceChanged_0400 end";
}
/**
 * @tc.name: DeviceManagerAgentTest_OnDeviceReady_0100
 * @tc.desc: Verify the OnDeviceReady function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_OnDeviceReady_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_OnDeviceReady_0100 start";
    bool res = true;

    try {
        DeviceManagerAgent::GetInstance()->OnDeviceReady(deviceInfo);
    } catch (const exception &e) {
        LOGE("Error:%{public}s", e.what());
        res = false;
    }

    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_OnDeviceReady_0100 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_OnDeviceReady_0200
 * @tc.desc: Verify the OnDeviceReady function.
 * @tc.type: FUNC
 * @tc.require: IA6UD3
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_OnDeviceReady_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_OnDeviceReady_0200 start";
    bool res = true;

    try {
        (void)memcpy_s(deviceInfo.networkId, DM_MAX_DEVICE_NAME_LEN - 1,
                       NETWORKID_TWO.c_str(), NETWORKID_TWO.size());
        std::vector<DmDeviceInfo> deviceList;
        deviceList.push_back(deviceInfo);
        EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _))
            .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0)));
        auto devicePtr = DeviceManagerAgent::GetInstance();
        devicePtr->OnDeviceReady(deviceInfo);
        
        auto smp = make_shared<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(100, "relativePath"));
        auto agent1 = make_shared<SoftbusAgent>(smp);
        devicePtr->cidNetTypeRecord_.insert({ NETWORKID_TWO, agent1 });
        EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _))
            .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0)));
        devicePtr->OnDeviceReady(deviceInfo);

        devicePtr->cidNetworkType_.insert({ NETWORKID_TWO, NETWORKTYPE_WITH_WIFI });
        EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _))
            .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0)));
        devicePtr->OnDeviceReady(deviceInfo);
    } catch (const exception &e) {
        LOGE("Error:%{public}s", e.what());
        res = false;
    }

    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_OnDeviceReady_0200 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_OnDeviceReady_0300
 * @tc.desc: Verify the OnDeviceReady function.
 * @tc.type: FUNC
 * @tc.require: IA6UD3
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_OnDeviceReady_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_OnDeviceReady_0300 start";
    bool res = true;

    try {
        auto smp = make_shared<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(100, "relativePath"));
        auto agent1 = make_shared<SoftbusAgent>(smp);
        (void)memcpy_s(deviceInfo.networkId, DM_MAX_DEVICE_NAME_LEN - 1,
                       NETWORKID_THREE.c_str(), NETWORKID_THREE.size());
        std::vector<DmDeviceInfo> deviceList;
        deviceList.push_back(deviceInfo);
        EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _))
            .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0))).WillOnce(Return(0));
        auto devicePtr = DeviceManagerAgent::GetInstance();
        devicePtr->cidNetTypeRecord_.insert({ NETWORKID_THREE, agent1 });
        devicePtr->cidNetworkType_.insert({ NETWORKID_THREE, NETWORKTYPE_NONE_WIFI });
        devicePtr->OnDeviceReady(deviceInfo);
        devicePtr->cidNetTypeRecord_.erase(NETWORKID_THREE);
        devicePtr->cidNetworkType_.erase(NETWORKID_THREE);
    } catch (const exception &e) {
        GTEST_LOG_(INFO) << e.what();
        res = false;
    }

    EXPECT_TRUE(res);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_OnDeviceReady_0300 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_OnDeviceP2POnline_0100
 * @tc.desc: Verify the OnDeviceP2POnline function.
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_OnDeviceP2POnline_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_OnDeviceP2POnline_0100 start";
    bool res = true;

    try {
        auto ret = DeviceManagerAgent::GetInstance()->OnDeviceP2POnline(deviceInfo);
        EXPECT_EQ(ret, 1);
    } catch (const exception &e) {
        LOGE("Error:%{public}s", e.what());
        res = false;
    }

    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_OnDeviceP2POnline_0100 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_OnDeviceP2POnline_0200
 * @tc.desc: Verify the OnDeviceP2POnline function.
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_OnDeviceP2POnline_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_OnDeviceP2POnline_0200 start";
    bool res = true;

    try {
        std::vector<DmDeviceInfo> deviceList;
        deviceList.push_back(deviceInfo);
        EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _))
            .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0)));
        auto smp = make_shared<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(100, "relativePath"));
        auto agent1 = make_shared<SoftbusAgent>(smp);
        string testNetWorkId = deviceInfo.networkId;
        auto devicePtr = DeviceManagerAgent::GetInstance();
        devicePtr->cidNetTypeRecord_.insert({ testNetWorkId, agent1 });
        auto ret = DeviceManagerAgent::GetInstance()->OnDeviceP2POnline(deviceInfo);
        EXPECT_EQ(ret, 1);

        EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _))
            .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0))).WillRepeatedly(Return(0));
        devicePtr->cidNetworkType_.insert({ testNetWorkId, NETWORKTYPE_NONE_WIFI });
        ret = devicePtr->OnDeviceP2POnline(deviceInfo);
        EXPECT_EQ(ret, 0);
        devicePtr->cidNetTypeRecord_.erase(testNetWorkId);
        devicePtr->cidNetworkType_.erase(testNetWorkId);
    } catch (const exception &e) {
        GTEST_LOG_(INFO) << e.what();
        res = false;
    }

    EXPECT_TRUE(res);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_OnDeviceP2POnline_0200 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_OnDeviceP2POffline_0100
 * @tc.desc: Verify the OnDeviceP2POffline function.
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_OnDeviceP2POffline_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_OnDeviceP2POffline_0100 start";
    bool res = true;

    try {
        auto ret = DeviceManagerAgent::GetInstance()->OnDeviceP2POffline(deviceInfo);
        EXPECT_EQ(ret, 1);
    } catch (const exception &e) {
        LOGE("Error:%{public}s", e.what());
        res = false;
    }

    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_OnDeviceP2POffline_0100 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_OnDeviceP2POffline_0200
 * @tc.desc: Verify the OnDeviceP2POffline function.
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_OnDeviceP2POffline_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_OnDeviceP2POffline_0200 start";
    bool res = true;

    try {
        DeviceManagerAgent::GetInstance()->OnDeviceP2POnline(deviceInfo);
        DeviceManagerAgent::GetInstance()->OnDeviceP2POffline(deviceInfo);
    } catch (const exception &e) {
        LOGE("Error:%{public}s", e.what());
        res = false;
    }

    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_OnDeviceP2POffline_0200 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_OnDeviceP2POffline_0300
 * @tc.desc: Verify the OnDeviceP2POffline function.
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_OnDeviceP2POffline_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_OnDeviceP2POffline_0300 start";
    bool res = true;

    try {
        auto smp = make_shared<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(100, "relativePath"));
        auto agent1 = make_shared<SoftbusAgent>(smp);
        (void)memcpy_s(deviceInfo.networkId, DM_MAX_DEVICE_NAME_LEN - 1,
                       NETWORKID_TWO.c_str(), NETWORKID_TWO.size());
        auto devicePtr = DeviceManagerAgent::GetInstance();
        devicePtr->cidNetTypeRecord_.clear();
        devicePtr->cidNetworkType_.clear();
        devicePtr->cidNetTypeRecord_.insert({ NETWORKID_TWO, agent1 });
        auto ret = devicePtr->OnDeviceP2POffline(deviceInfo);
        EXPECT_EQ(ret, 1);
        devicePtr->cidNetworkType_.insert({ NETWORKID_TWO, NETWORKTYPE_NONE_WIFI });
        ret = devicePtr->OnDeviceP2POffline(deviceInfo);
        EXPECT_EQ(ret, 0);
        auto iter = devicePtr->cidNetTypeRecord_.find(NETWORKID_TWO);
        if (iter != devicePtr->cidNetTypeRecord_.end()) {
            res = false;
        }

        auto iterType = devicePtr->cidNetworkType_.find(NETWORKID_TWO);
        if (iterType != devicePtr->cidNetworkType_.end()) {
            res = false;
        }
    } catch (const exception &e) {
        GTEST_LOG_(INFO) << e.what();
        res = false;
    }

    EXPECT_TRUE(res);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_OnDeviceP2POffline_0300 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_StopInstance_0100
 * @tc.desc: Verify the StopInstance function.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_StopInstance_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_StopInstance_0100 start";
    bool res = true;

    try {
        DeviceManagerAgent::GetInstance()->StopInstance();
    } catch (const exception &e) {
        LOGE("Error:%{public}s", e.what());
        res = false;
    }

    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_StopInstance_0100 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_Stop_0100
 * @tc.desc: Verify the Stop function.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_Stop_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_Stop_0100 start";
    bool res = true;

    try {
        DeviceManagerAgent::GetInstance()->Start();
        DeviceManagerAgent::GetInstance()->Stop();
    } catch (const exception &e) {
        LOGE("Error:%{public}s", e.what());
        res = false;
    }

    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_Stop_0100 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_JoinGroup_0100
 * @tc.desc: Verify the JoinGroup function.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_JoinGroup_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_JoinGroup_0100 start";
    bool res = true;

    try {

        auto smp = make_shared<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(100, "relativePath"));
        DeviceManagerAgent::GetInstance()->JoinGroup(smp);
        GTEST_LOG_(INFO) << smp->GetID();
        auto it = DeviceManagerAgent::GetInstance()->mpToNetworks_.find(smp->GetID());
        if (it == DeviceManagerAgent::GetInstance()->mpToNetworks_.end()) {
            EXPECT_TRUE(false);
        } else {
            EXPECT_TRUE(true);
        }
        DeviceManagerAgent::GetInstance()->QuitGroup(smp);
        it = DeviceManagerAgent::GetInstance()->mpToNetworks_.find(smp->GetID());
        if (it == DeviceManagerAgent::GetInstance()->mpToNetworks_.end()) {
            EXPECT_TRUE(true);
        } else {
            EXPECT_TRUE(false);
        }
    } catch (const exception &e) {
        GTEST_LOG_(INFO) << e.what();
        res = false;
    }

    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_JoinGroup_0100 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_JoinGroup_0200
 * @tc.desc: Verify the JoinGroup function.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_JoinGroup_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_JoinGroup_0200 start";
    bool res = true;

    try {
        weak_ptr<MountPoint> nullwmp;
        DeviceManagerAgent::GetInstance()->JoinGroup(nullwmp);
    } catch (const exception &e) {
        EXPECT_EQ(string(e.what()), "Failed to join group: Received empty mountpoint");
        res = false;
    }

    EXPECT_FALSE(res);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_JoinGroup_0200 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_JoinGroup_0100
 * @tc.desc: Verify the JoinGroup function.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_JoinGroup_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_JoinGroup_0300 start";
    bool res = true;

    try {
        auto smp = make_shared<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(100, "relativePath"));
        DeviceManagerAgent::GetInstance()->JoinGroup(smp);
        DeviceManagerAgent::GetInstance()->JoinGroup(smp);
        DeviceManagerAgent::GetInstance()->QuitGroup(smp);
    } catch (const exception &e) {
        EXPECT_EQ(string(e.what()), "Failed to join group: Mountpoint existed");
        res = false;
    }

    EXPECT_FALSE(res);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_JoinGroup_0300 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_QuitGroup_0100
 * @tc.desc: Verify the QuitGroup function.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_QuitGroup_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_QuitGroup_0100 start";
    bool res = true;

    try {
        auto smp = make_shared<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(100, "relativePath"));
        DeviceManagerAgent::GetInstance()->QuitGroup(smp);
    } catch (const exception &e) {
        EXPECT_EQ(string(e.what()), "Failed to quit group: Mountpoint didn't exist ");
        res = false;
    }

    EXPECT_TRUE(res != true);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_QuitGroup_0100 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_QuitGroup_0200
 * @tc.desc: Verify the QuitGroup function.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_QuitGroup_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_QuitGroup_0200 start";
    bool res = true;

    try {
        shared_ptr<MountPoint> nullwmp;
        DeviceManagerAgent::GetInstance()->QuitGroup(nullwmp);
    } catch (const exception &e) {
        EXPECT_EQ(string(e.what()), "Failed to quit group: Received empty mountpoint");
        res = false;
    }

    EXPECT_FALSE(res);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_QuitGroup_0200 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_OfflineAllDevice_0100
 * @tc.desc: Verify the OfflineAllDevice function.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_OfflineAllDevice_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_OfflineAllDevice_0100 start";
    bool res = true;

    try {
        DeviceManagerAgent::GetInstance()->OfflineAllDevice();
    } catch (const exception &e) {
        LOGE("Error:%{public}s", e.what());
        res = false;
    }

    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_OfflineAllDevice_0100 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_OfflineAllDevice_0200
 * @tc.desc: Verify the OfflineAllDevice function.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_OfflineAllDevice_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_OfflineAllDevice_0200 start";
    bool res = true;

    try {
        auto smp = make_shared<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(100, "relativePath"));
        auto agent1 = make_shared<SoftbusAgent>(smp);
        DeviceManagerAgent::GetInstance()->cidNetTypeRecord_.insert({ "test1", agent1 });
        DeviceManagerAgent::GetInstance()->cidNetTypeRecord_.insert({ "test2", nullptr });

        DeviceManagerAgent::GetInstance()->OfflineAllDevice();
        DeviceManagerAgent::GetInstance()->cidNetTypeRecord_.erase("test1");
        DeviceManagerAgent::GetInstance()->cidNetTypeRecord_.erase("test2");
    } catch (const exception &e) {
        GTEST_LOG_(INFO) << e.what();
        res = false;
    }

    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_OfflineAllDevice_0200 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_ReconnectOnlineDevices_0100
 * @tc.desc: Verify the ReconnectOnlineDevices function.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_ReconnectOnlineDevices_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_ReconnectOnlineDevices_0100 start";
    bool res = true;

    try {
        DeviceManagerAgent::GetInstance()->ReconnectOnlineDevices();
    } catch (const exception &e) {
        GTEST_LOG_(INFO) << e.what();
        res = false;
    }

    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_ReconnectOnlineDevices_0100 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_ReconnectOnlineDevices_0200
 * @tc.desc: Verify the ReconnectOnlineDevices function.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_ReconnectOnlineDevices_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_ReconnectOnlineDevices_0200 start";
    bool res = true;

    try {
        auto smp = make_shared<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(100, "relativePath"));
        auto agent1 = make_shared<SoftbusAgent>(smp);
        DeviceManagerAgent::GetInstance()->cidNetTypeRecord_.insert({ "test1", agent1 });
        DeviceManagerAgent::GetInstance()->cidNetTypeRecord_.insert({ "test2", nullptr });
        DeviceManagerAgent::GetInstance()->ReconnectOnlineDevices();
        DeviceManagerAgent::GetInstance()->cidNetTypeRecord_.erase("test1");
        DeviceManagerAgent::GetInstance()->cidNetTypeRecord_.erase("test2");
    } catch (const exception &e) {
        GTEST_LOG_(INFO) << e.what();
        res = false;
    }

    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_ReconnectOnlineDevices_0200 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_FindNetworkBaseTrustRelation_0100
 * @tc.desc: Verify the FindNetworkBaseTrustRelation function.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_FindNetworkBaseTrustRelation_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_FindNetworkBaseTrustRelation_0100 start";
    bool res = true;

    try {
        auto rltPtr = DeviceManagerAgent::GetInstance()->FindNetworkBaseTrustRelation(true);
        EXPECT_EQ(rltPtr, nullptr);
        rltPtr = DeviceManagerAgent::GetInstance()->FindNetworkBaseTrustRelation(false);
        EXPECT_EQ(rltPtr, nullptr);
    } catch (const exception &e) {
        LOGE("Error:%{public}s", e.what());
        res = false;
    }

    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_FindNetworkBaseTrustRelation_0100 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_FindNetworkBaseTrustRelation_0200
 * @tc.desc: Verify the FindNetworkBaseTrustRelation function.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_FindNetworkBaseTrustRelation_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_FindNetworkBaseTrustRelation_0200 start";
    bool res = true;

    try {
        auto smp = make_shared<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(100, "relativePath"));
        DeviceManagerAgent::GetInstance()->JoinGroup(smp);
        auto rltPtr = DeviceManagerAgent::GetInstance()->FindNetworkBaseTrustRelation(true);
        EXPECT_EQ(rltPtr, nullptr);
        rltPtr = DeviceManagerAgent::GetInstance()->FindNetworkBaseTrustRelation(false);
        EXPECT_NE(rltPtr, nullptr);
        DeviceManagerAgent::GetInstance()->QuitGroup(smp);

    } catch (const exception &e) {
        LOGE("Error:%{public}s", e.what());
        res = false;
    }

    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_FindNetworkBaseTrustRelation_0200 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_FindNetworkBaseTrustRelation_0300
 * @tc.desc: Verify the FindNetworkBaseTrustRelation function.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_FindNetworkBaseTrustRelation_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_FindNetworkBaseTrustRelation_0300 start";
    bool res = true;

    try {
        auto nonmp = make_shared<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(101, "non_account"));
        DeviceManagerAgent::GetInstance()->JoinGroup(nonmp);
        auto id = nonmp->GetID() + 1;
        DeviceManagerAgent::GetInstance()->mpToNetworks_.insert({ id, nullptr });
        shared_ptr<SoftbusAgent> agent = nullptr;
        auto rltPtr = DeviceManagerAgent::GetInstance()->FindNetworkBaseTrustRelation(true);
        EXPECT_NE(rltPtr, nullptr);
        rltPtr = DeviceManagerAgent::GetInstance()->FindNetworkBaseTrustRelation(false);
        EXPECT_EQ(rltPtr, nullptr);
        DeviceManagerAgent::GetInstance()->QuitGroup(nonmp);
        DeviceManagerAgent::GetInstance()->mpToNetworks_.erase(id);
    } catch (const exception &e) {
        LOGE("Error:%{public}s", e.what());
        res = false;
    }

    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_FindNetworkBaseTrustRelation_0300 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_GetNetworkType_0100
 * @tc.desc: Verify the GetNetworkType function.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_GetNetworkType_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_GetNetworkType_0100 start";
    bool res = true;

    try {
        int ret = DeviceManagerAgent::GetInstance()->GetNetworkType(NETWORKID_ONE);
        EXPECT_EQ(ret, 0);
        ret = DeviceManagerAgent::GetInstance()->GetNetworkType(NETWORKID_TWO);
        EXPECT_EQ(ret, NETWORKTYPE_WITH_WIFI);
        ret = DeviceManagerAgent::GetInstance()->GetNetworkType(NETWORKID_THREE);
        EXPECT_EQ(ret, NETWORKTYPE_NONE_WIFI);
    } catch (const exception &e) {
        LOGE("Error:%{public}s", e.what());
        res = false;
    }

    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_GetNetworkType_0100 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_IsWifiNetworkType_0100
 * @tc.desc: Verify the IsWifiNetworkType function.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_IsWifiNetworkType_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_IsWifiNetworkType_0100 start";
    bool res = true;

    try {
        int ret = DeviceManagerAgent::GetInstance()->IsWifiNetworkType(-1);
        EXPECT_FALSE(ret);
        ret = DeviceManagerAgent::GetInstance()->IsWifiNetworkType(1);
        EXPECT_FALSE(ret);
        ret = DeviceManagerAgent::GetInstance()->IsWifiNetworkType(3);
        EXPECT_TRUE(ret);
    } catch (const exception &e) {
        LOGE("Error:%{public}s", e.what());
        res = false;
    }

    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_IsWifiNetworkType_0100 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_CheckIsAccountless_0100
 * @tc.desc: Verify the CheckIsAccountless function.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_CheckIsAccountless_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_CheckIsAccountless_0100 start";
    bool res = true;

    try {
        GroupInfo group{};
        g_parameterVaule = false;
        auto ret = DeviceManagerAgent::GetInstance()->CheckIsAccountless(group);
        EXPECT_TRUE(ret);
    } catch (const exception &e) {
        GTEST_LOG_(INFO) << e.what();
        res = false;
    }

    EXPECT_TRUE(res);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_CheckIsAccountless_0100 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_CheckIsAccountless_0200
 * @tc.desc: Verify the CheckIsAccountless function.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_CheckIsAccountless_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_CheckIsAccountless_0200 start";
    bool res = true;

    try {
        GroupInfo group{};
        g_parameterVaule = true;
        auto ret = DeviceManagerAgent::GetInstance()->CheckIsAccountless(group);
        EXPECT_FALSE(ret);
    } catch (const exception &e) {
        GTEST_LOG_(INFO) << e.what();
        res = false;
    }

    EXPECT_TRUE(res);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_CheckIsAccountless_0200 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_InitDeviceInfos_0100
 * @tc.desc: Verify the InitDeviceInfos function.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_InitDeviceInfos_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_InitDeviceInfos_0100 start";
    bool res = true;

    try {
        std::vector<DmDeviceInfo> deviceList;
        deviceList.push_back(deviceInfo);
        EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _))
            .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0)))
            .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0)));
        DeviceManagerAgent::GetInstance()->InitDeviceInfos();
    } catch (const exception &e) {
        GTEST_LOG_(INFO) << e.what();
        res = false;
    }

    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_InitDeviceInfos_0100 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_InitDeviceInfos_0200
 * @tc.desc: Verify the InitDeviceInfos function.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_InitDeviceInfos_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_InitDeviceInfos_0200 start";
    bool res = true;

    try {
        EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _)).WillOnce(Return(-1));
        DeviceManagerAgent::GetInstance()->InitDeviceInfos();
    } catch (const exception &e) {
        GTEST_LOG_(INFO) << e.what();
        res = false;
    }

    EXPECT_FALSE(res);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_InitDeviceInfos_0200 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_InitDeviceInfos_0300
 * @tc.desc: Verify the InitDeviceInfos function.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_InitDeviceInfos_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_InitDeviceInfos_0300 start";
    bool res = true;

    try {
        std::vector<DmDeviceInfo> deviceList;
        deviceList.push_back(deviceInfo);
        EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _))
            .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0))).WillOnce(Return(0));
        DeviceManagerAgent::GetInstance()->InitDeviceInfos();
    } catch (const exception &e) {
        GTEST_LOG_(INFO) << e.what();
        res = false;
    }

    EXPECT_TRUE(res);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_InitDeviceInfos_0300 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_InitLocalNodeInfo_0100
 * @tc.desc: Verify the InitLocalNodeInfo function.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_InitLocalNodeInfo_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_InitLocalNodeInfo_0100 start";
    bool res = true;

    try {
        EXPECT_CALL(*deviceManagerImplMock_, GetLocalDeviceInfo(_, _)).WillOnce(Return(0));
        DeviceManagerAgent::GetInstance()->InitLocalNodeInfo();
    } catch (const exception &e) {
        LOGE("Error:%{public}s", e.what());
        res = false;
    }

    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_InitLocalNodeInfo_0100 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_InitLocalNodeInfo_0100
 * @tc.desc: Verify the InitLocalNodeInfo function.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_InitLocalNodeInfo_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_InitLocalNodeInfo_0200 start";
    bool res = true;

    try {
        EXPECT_CALL(*deviceManagerImplMock_, GetLocalDeviceInfo(_, _)).WillOnce(Return(-1));
        DeviceManagerAgent::GetInstance()->InitLocalNodeInfo();
    } catch (const exception &e) {
        GTEST_LOG_(INFO) << e.what();
        res = false;
    }

    EXPECT_FALSE(res);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_InitLocalNodeInfo_0200 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_OnRemoteDied_0100
 * @tc.desc: Verify the OnRemoteDied function.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_OnRemoteDied_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_OnRemoteDied_0100 start";
    bool res = true;

    try {
        DeviceManagerAgent::GetInstance()->OnRemoteDied();
    } catch (const exception &e) {
        LOGE("Error:%{public}s", e.what());
        res = false;
    }

    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_OnRemoteDied_0100 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_GetLocalDeviceInfo_0100
 * @tc.desc: Verify the GetLocalDeviceInfo function.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_GetLocalDeviceInfo_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_GetLocalDeviceInfo_0100 start";
    bool res = true;

    try {
        DeviceManagerAgent::GetInstance()->GetLocalDeviceInfo();
    } catch (const exception &e) {
        LOGE("Error:%{public}s", e.what());
        res = false;
    }

    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_GetLocalDeviceInfo_0100 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_GetRemoteDevicesInfo_0100
 * @tc.desc: Verify the GetRemoteDevicesInfo function.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_GetRemoteDevicesInfo_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_GetRemoteDevicesInfo_0100 start";
    bool res = true;

    try {
        std::vector<DmDeviceInfo> deviceList;
        deviceList.push_back(deviceInfo);
        EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _))
            .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0)));
        auto info = DeviceManagerAgent::GetInstance()->GetRemoteDevicesInfo();
        EXPECT_NE(info.size(), 0);
    } catch (const exception &e) {
        GTEST_LOG_(INFO) << e.what();
        res = false;
    }

    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_GetRemoteDevicesInfo_0100 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_GetRemoteDevicesInfo_0200
 * @tc.desc: Verify the GetRemoteDevicesInfo function.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_GetRemoteDevicesInfo_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_GetRemoteDevicesInfo_0200 start";
    bool res = true;

    try {
        EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _)).WillOnce(Return(-1));
        auto info = DeviceManagerAgent::GetInstance()->GetRemoteDevicesInfo();
        EXPECT_NE(info.size(), 0);
    } catch (const exception &e) {
        GTEST_LOG_(INFO) << e.what();
        res = false;
    }

    EXPECT_FALSE(res);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_GetRemoteDevicesInfo_0200 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_RegisterToExternalDm_0100
 * @tc.desc: Verify the RegisterToExternalDm function.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_RegisterToExternalDm_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_RegisterToExternalDm_0100 start";
    bool res = true;

    try {
        EXPECT_CALL(*deviceManagerImplMock_, InitDeviceManager(_, _)).WillOnce(Return(-1));
        DeviceManagerAgent::GetInstance()->RegisterToExternalDm();
    } catch (const exception &e) {
        string errMsg = e.what();
        EXPECT_NE(errMsg.find("Failed to InitDeviceManager"), string::npos);
        res = false;
    }

    EXPECT_FALSE(res);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_RegisterToExternalDm_0100 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_RegisterToExternalDm_0100
 * @tc.desc: Verify the RegisterToExternalDm function.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_RegisterToExternalDm_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_RegisterToExternalDm_0200 start";
    bool res = true;

    try {
        EXPECT_CALL(*deviceManagerImplMock_, InitDeviceManager(_, _)).WillOnce(Return(0));
        EXPECT_CALL(*deviceManagerImplMock_, RegisterDevStateCallback(_, _, _)).WillOnce(Return(-1));
        DeviceManagerAgent::GetInstance()->RegisterToExternalDm();
    } catch (const exception &e) {
        string errMsg = e.what();
        EXPECT_NE(errMsg.find("Failed to RegisterDevStateCallback"), string::npos);
        res = false;
    }

    EXPECT_FALSE(res);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_RegisterToExternalDm_0200 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_RegisterToExternalDm_0300
 * @tc.desc: Verify the RegisterToExternalDm function.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_RegisterToExternalDm_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_RegisterToExternalDm_0300 start";
    bool res = true;

    try {
        EXPECT_CALL(*deviceManagerImplMock_, InitDeviceManager(_, _)).WillOnce(Return(0));
        EXPECT_CALL(*deviceManagerImplMock_, RegisterDevStateCallback(_, _, _)).WillOnce(Return(0));
        DeviceManagerAgent::GetInstance()->RegisterToExternalDm();
    } catch (const exception &e) {
        GTEST_LOG_(INFO) << e.what();
        res = false;
    }

    EXPECT_TRUE(res);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_RegisterToExternalDm_0300 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_UnregisterFromExternalDm_0100
 * @tc.desc: Verify the UnregisterFromExternalDm function.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_UnregisterFromExternalDm_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_UnregisterFromExternalDm_0100 start";
    bool res = true;

    try {
        EXPECT_CALL(*deviceManagerImplMock_, UnRegisterDevStateCallback(_)).WillOnce(Return(-1));
        DeviceManagerAgent::GetInstance()->UnregisterFromExternalDm();
    } catch (const exception &e) {
        string errMsg = e.what();
        EXPECT_NE(errMsg.find("Failed to UnRegisterDevStateCallback"), string::npos);
        res = false;
    }

    EXPECT_FALSE(res);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_UnregisterFromExternalDm_0100 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_UnregisterFromExternalDm_0200
 * @tc.desc: Verify the UnregisterFromExternalDm function.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_UnregisterFromExternalDm_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_UnregisterFromExternalDm_0200 start";
    bool res = true;

    try {
        EXPECT_CALL(*deviceManagerImplMock_, UnRegisterDevStateCallback(_)).WillOnce(Return(0));
        EXPECT_CALL(*deviceManagerImplMock_, UnInitDeviceManager(_)).WillOnce(Return(-1));
        DeviceManagerAgent::GetInstance()->UnregisterFromExternalDm();
    } catch (const exception &e) {
        string errMsg = e.what();
        EXPECT_NE(errMsg.find("Failed to UnInitDeviceManager"), string::npos);
        res = false;
    }

    EXPECT_FALSE(res);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_UnregisterFromExternalDm_0200 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_UnregisterFromExternalDm_0300
 * @tc.desc: Verify the UnregisterFromExternalDm function.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_UnregisterFromExternalDm_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_UnregisterFromExternalDm_0300 start";
    bool res = true;

    try {
        EXPECT_CALL(*deviceManagerImplMock_, UnRegisterDevStateCallback(_)).WillOnce(Return(0));
        EXPECT_CALL(*deviceManagerImplMock_, UnInitDeviceManager(_)).WillOnce(Return(0));
        DeviceManagerAgent::GetInstance()->UnregisterFromExternalDm();
    } catch (const exception &e) {
        GTEST_LOG_(INFO) << e.what();
        res = false;
    }

    EXPECT_TRUE(res);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_UnregisterFromExternalDm_0300 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_QueryRelatedGroups_0100
 * @tc.desc: Verify the QueryRelatedGroups function.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_QueryRelatedGroups_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_QueryRelatedGroups_0100 start";
    bool res = true;

    try {
        DeviceManagerAgent::GetInstance()->QueryRelatedGroups("100", "100");
    } catch (const exception &e) {
        LOGE("Error:%{public}s", e.what());
        res = false;
    }

    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_QueryRelatedGroups_0100 end";
}
} // namespace Test
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
