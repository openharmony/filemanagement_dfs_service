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
#include "mock_other_method.h"

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
    .networkId = "testNetwork",
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
    static inline shared_ptr<DfsDeviceOtherMethodMock> otherMethodMock_ = nullptr;
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
    otherMethodMock_ = make_shared<DfsDeviceOtherMethodMock>();
    DfsDeviceOtherMethodMock::otherMethod = otherMethodMock_;
    deviceManagerImplMock_ = make_shared<DeviceManagerImplMock>();
    DeviceManagerImplMock::dfsDeviceManagerImpl = deviceManagerImplMock_;
    EXPECT_CALL(*deviceManagerImplMock_, InitDeviceManager(_, _)).WillRepeatedly(Return(0));
    EXPECT_CALL(*deviceManagerImplMock_, RegisterDevStateCallback(_, _, _)).WillRepeatedly(Return(0));
    EXPECT_CALL(*deviceManagerImplMock_, UnRegisterDevStateCallback(_)).WillRepeatedly(Return(0));
    EXPECT_CALL(*deviceManagerImplMock_, UnInitDeviceManager(_)).WillRepeatedly(Return(0));
    EXPECT_CALL(*deviceManagerImplMock_, UnInitDeviceManager(_)).WillRepeatedly(Return(0));
    EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _)).WillRepeatedly(Return(0));
}

void DeviceManagerAgentTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
    DeviceManagerImplMock::dfsDeviceManagerImpl = nullptr;
    deviceManagerImplMock_ = nullptr;
    DfsDeviceOtherMethodMock::otherMethod = nullptr;
    otherMethodMock_ = nullptr;
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
        DeviceManagerAgent::GetInstance()->OnDeviceOnline(deviceInfo);
        DeviceManagerAgent::GetInstance()->cidNetTypeRecord_.erase(NETWORKID_TWO);
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
 * @tc.name: DeviceManagerAgentTest_OnDeviceOffline_0300
 * @tc.desc: Verify the OnDeviceOffline function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_OnDeviceOffline_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_OnDeviceOffline_0300 start";
    bool res = true;

    try {
        EXPECT_CALL(*deviceManagerImplMock_, GetLocalDeviceInfo(_, _)).WillRepeatedly(Return(0));
        DeviceManagerAgent::GetInstance()->InitLocalNodeInfo();
        auto smp = make_shared<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(100, "relativePath"));
        auto agent1 = make_shared<SoftbusAgent>(smp);
        (void)memcpy_s(deviceInfo.networkId, DM_MAX_DEVICE_NAME_LEN - 1,
                       NETWORKID_TWO.c_str(), NETWORKID_TWO.size());
        DeviceManagerAgent::GetInstance()->cidNetTypeRecord_.insert({ NETWORKID_TWO, agent1 });
        DeviceManagerAgent::GetInstance()->OnDeviceOnline(deviceInfo);
        DeviceManagerAgent::GetInstance()->OnDeviceOffline(deviceInfo);
        DeviceManagerAgent::GetInstance()->cidNetTypeRecord_.erase(NETWORKID_TWO);

        DeviceManagerAgent::GetInstance()->cidNetTypeRecord_.insert({ NETWORKID_TWO, nullptr });
        DeviceManagerAgent::GetInstance()->OnDeviceOffline(deviceInfo);
        EXPECT_NE(DeviceManagerAgent::GetInstance()->cidNetTypeRecord_.size(), 0);
        DeviceManagerAgent::GetInstance()->cidNetTypeRecord_.clear();
    } catch (const exception &e) {
        GTEST_LOG_(INFO) << e.what();
        res = false;
    }

    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_OnDeviceOffline_0300 end";
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
        EXPECT_EQ(ret, 0);

        EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _))
            .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0))).WillRepeatedly(Return(0));
        devicePtr->cidNetTypeRecord_[testNetWorkId] = nullptr;
        ret = devicePtr->OnDeviceP2POnline(deviceInfo);
        EXPECT_EQ(ret, DeviceManagerAgent::P2PErrCode::P2P_FAILED);
        EXPECT_EQ(devicePtr->cidNetTypeRecord_.size(), 1);
        devicePtr->cidNetTypeRecord_.erase(testNetWorkId);
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
        devicePtr->cidNetTypeRecord_.insert({ NETWORKID_TWO, agent1 });
        auto ret = devicePtr->OnDeviceP2POffline(deviceInfo);
        EXPECT_EQ(ret, 0);
        auto iter = devicePtr->cidNetTypeRecord_.find(NETWORKID_TWO);
        if (iter != devicePtr->cidNetTypeRecord_.end()) {
            res = false;
        }
        devicePtr->cidNetTypeRecord_.insert({NETWORKID_TWO, nullptr});
        ret = devicePtr->OnDeviceP2POffline(deviceInfo);
        EXPECT_EQ(ret, DeviceManagerAgent::P2PErrCode::P2P_FAILED);
        EXPECT_NE(devicePtr->cidNetTypeRecord_.size(), 0);
        devicePtr->cidNetTypeRecord_.clear();
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
    auto smp = make_shared<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(100, "relativePath"));
    try {
        DeviceManagerAgent::GetInstance()->JoinGroup(smp);
        DeviceManagerAgent::GetInstance()->JoinGroup(smp);
    } catch (const exception &e) {
        EXPECT_EQ(string(e.what()), "Failed to join group: Mountpoint existed");
        DeviceManagerAgent::GetInstance()->QuitGroup(smp);
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
        DeviceManagerAgent::GetInstance()->mpToNetworks_.clear();
        auto smp = make_shared<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(100, "relativePath"));
        DeviceManagerAgent::GetInstance()->mpToNetworks_.emplace(smp->GetID(), nullptr);
        DeviceManagerAgent::GetInstance()->QuitGroup(smp);
        EXPECT_EQ(DeviceManagerAgent::GetInstance()->mpToNetworks_.size(), 0);

        shared_ptr<MountPoint> nullsmp;
        DeviceManagerAgent::GetInstance()->QuitGroup(nullsmp);
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

/**
 * @tc.name: DeviceManagerAgentTest_UMountDisShareFile_0100
 * @tc.desc: Verify the UMountDisShareFile function with invalid userID and networkID.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_UMountDisShareFile_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_UMountDisShareFile_0100 start";
    bool res = true;

    try {
        DeviceManagerAgent::GetInstance()->UMountDisShareFile();
    } catch (const exception &e) {
        LOGE("Error:%{public}s", e.what());
        res = false;
    }

    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_UMountDisShareFile_0100 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_UMountDisShareFile_0200
 * @tc.desc: Verify the UMountDisShareFile function with invalid networkID.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_UMountDisShareFile_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_UMountDisShareFile_0200 start";
    bool res = true;

    try {
        std::vector<int32_t> userIds{100, 101};
        EXPECT_CALL(*otherMethodMock_, QueryActiveOsAccountIds(_))
            .WillOnce(DoAll(SetArgReferee<0>(userIds), Return(NO_ERROR)));

        DeviceManagerAgent::GetInstance()->UMountDisShareFile();
    } catch (const exception &e) {
        GTEST_LOG_(INFO) << e.what();
        res = false;
    }

    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_UMountDisShareFile_0200 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_UMountDisShareFile_0300
 * @tc.desc: Verify the UMountDisShareFile function with invalid userID.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_UMountDisShareFile_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_UMountDisShareFile_0300 start";
    bool res = true;

    try {
        DeviceManagerAgent::GetInstance()->GetLocalDeviceInfo() = deviceInfo;
        DeviceManagerAgent::GetInstance()->UMountDisShareFile();
    } catch (const exception &e) {
        GTEST_LOG_(INFO) << e.what();
        res = false;
    }

    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_UMountDisShareFile_0300 end";
}


/**
 * @tc.name: DeviceManagerAgentTest_UMountDisShareFile_0400
 * @tc.desc: Verify the UMountDisShareFile function handles multiple calls gracefully.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(DeviceManagerAgentTest, DeviceManagerAgentTest_UMountDisShareFile_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_UMountDisShareFile_0400 start";
    bool res = true;

    try {
        std::vector<int32_t> userIds{100, 101};
        EXPECT_CALL(*otherMethodMock_, QueryActiveOsAccountIds(_))
            .WillRepeatedly(DoAll(SetArgReferee<0>(userIds), Return(NO_ERROR)));

        // Call UMountDisShareFile multiple times to ensure it handles gracefully
        DeviceManagerAgent::GetInstance()->UMountDisShareFile();
        DeviceManagerAgent::GetInstance()->UMountDisShareFile();
        DeviceManagerAgent::GetInstance()->UMountDisShareFile();
    } catch (const exception &e) {
        GTEST_LOG_(INFO) << e.what();
        res = false;
    }

    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_UMountDisShareFile_0400 end";
}
} // namespace Test
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
