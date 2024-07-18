/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
#include <gtest/gtest.h>
#include <memory>
#include <unistd.h>

#include "device_manager_impl_mock.h"
#include "dfs_error.h"
#include "mock_other_method.h"
#include "mountpoint/mount_point.h"
#include "network/softbus/softbus_agent.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

const int32_t MOUNT_DFS_COUNT_ONE = 1;
const int32_t INVALID_USER_ID = -1;

DistributedHardware::DmDeviceInfo deviceInfo = {
    .deviceId = "testdevid",
    .deviceName = "testdevname",
    .deviceTypeId = 1,
    .networkId = "testNetWork",
};

class DeviceManagerAgentSupTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<DfsDeviceOtherMethodMock> otherMethodMock_ = nullptr;
    static inline shared_ptr<DeviceManagerImplMock> deviceManagerImplMock_ = nullptr;
};

void DeviceManagerAgentSupTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
    otherMethodMock_ = make_shared<DfsDeviceOtherMethodMock>();
    DfsDeviceOtherMethodMock::otherMethod = otherMethodMock_;
    deviceManagerImplMock_ = make_shared<DeviceManagerImplMock>();
    DeviceManagerImplMock::dfsDeviceManagerImpl = deviceManagerImplMock_;
    EXPECT_CALL(*deviceManagerImplMock_, InitDeviceManager(_, _)).WillRepeatedly(Return(0));
    EXPECT_CALL(*deviceManagerImplMock_, RegisterDevStateCallback(_, _, _)).WillRepeatedly(Return(0));
    EXPECT_CALL(*deviceManagerImplMock_, GetLocalNodeDeviceInfo(_, _)).WillRepeatedly(Return(0));
    EXPECT_CALL(*deviceManagerImplMock_, UnRegisterDevStateCallback(_)).WillRepeatedly(Return(0));
    EXPECT_CALL(*deviceManagerImplMock_, UnInitDeviceManager(_)).WillRepeatedly(Return(0));
    EXPECT_CALL(*deviceManagerImplMock_, UnInitDeviceManager(_)).WillRepeatedly(Return(0));
    EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _)).WillRepeatedly(Return(0));
}

void DeviceManagerAgentSupTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
    DfsDeviceOtherMethodMock::otherMethod = nullptr;
    otherMethodMock_ = nullptr;
    DeviceManagerImplMock::dfsDeviceManagerImpl = nullptr;
    deviceManagerImplMock_ = nullptr;
}

void DeviceManagerAgentSupTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void DeviceManagerAgentSupTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

/**
 * @tc.name: DeviceManagerAgentTest_MountDfsCountOnly_0100
 * @tc.desc: Verify the MountDfsCountOnly function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(DeviceManagerAgentSupTest, DeviceManagerAgentTest_MountDfsCountOnly_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_MountDfsCountOnly_0100 start";
    auto testPtr = DeviceManagerAgent::GetInstance();
    bool flag = testPtr->MountDfsCountOnly("");
    EXPECT_EQ(flag, false);
    flag = testPtr->MountDfsCountOnly("test");
    EXPECT_EQ(flag, false);
    testPtr->mountDfsCount_["test"] = 1;
    flag = testPtr->MountDfsCountOnly("test");
    EXPECT_EQ(flag, true);
    testPtr->mountDfsCount_["test"] = -3;
    flag = testPtr->MountDfsCountOnly("test");
    EXPECT_EQ(flag, false);
    testPtr->mountDfsCount_.erase("test");
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_MountDfsCountOnly_0100 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_UMountDfsCountOnly_0100
 * @tc.desc: Verify the UMountDfsCountOnly function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(DeviceManagerAgentSupTest, DeviceManagerAgentTest_UMountDfsCountOnly_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_UMountDfsCountOnly_0100 start";
    auto testPtr = DeviceManagerAgent::GetInstance();
    bool flag = testPtr->UMountDfsCountOnly("", true);
    EXPECT_EQ(flag, false);
    flag = testPtr->UMountDfsCountOnly("test", true);
    EXPECT_EQ(flag, true);

    testPtr->mountDfsCount_["test"] = MOUNT_DFS_COUNT_ONE + 1;
    flag = testPtr->UMountDfsCountOnly("test", true);
    EXPECT_EQ(flag, false);
    EXPECT_EQ(testPtr->mountDfsCount_["test"], 0);

    testPtr->mountDfsCount_["test"] = MOUNT_DFS_COUNT_ONE + 1;
    flag = testPtr->UMountDfsCountOnly("test", false);
    EXPECT_EQ(flag, true);
    EXPECT_EQ(testPtr->mountDfsCount_["test"], MOUNT_DFS_COUNT_ONE);

    flag = testPtr->UMountDfsCountOnly("test", false);
    EXPECT_EQ(flag, false);

    auto itCount = testPtr->mountDfsCount_.find("test");
    if (itCount == testPtr->mountDfsCount_.end()) {
        EXPECT_TRUE(false);
    } else {
        EXPECT_TRUE(true);
    }
    testPtr->mountDfsCount_.erase("test");
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_UMountDfsCountOnly_0100 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_GetCurrentUserId_0100
 * @tc.desc: Verify the GetCurrentUserId function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(DeviceManagerAgentSupTest, DeviceManagerAgentTest_GetCurrentUserId_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_GetCurrentUserId_0100 start";
    EXPECT_CALL(*otherMethodMock_, QueryActiveOsAccountIds(_)).WillOnce(Return(INVALID_USER_ID));
    auto testPtr = DeviceManagerAgent::GetInstance();
    auto userId = testPtr->GetCurrentUserId();
    EXPECT_EQ(userId, INVALID_USER_ID);

    EXPECT_CALL(*otherMethodMock_, QueryActiveOsAccountIds(_)).WillOnce(Return(NO_ERROR));
    userId = testPtr->GetCurrentUserId();
    EXPECT_EQ(userId, INVALID_USER_ID);
    
    std::vector<int32_t> userIds{100, 101};
    EXPECT_CALL(*otherMethodMock_, QueryActiveOsAccountIds(_))
        .WillOnce(DoAll(SetArgReferee<0>(userIds), Return(NO_ERROR)));
    userId = testPtr->GetCurrentUserId();
    EXPECT_EQ(userId, userIds[0]);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_GetCurrentUserId_0100 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_AddNetworkId_0100
 * @tc.desc: Verify the AddNetworkId function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(DeviceManagerAgentSupTest, DeviceManagerAgentTest_AddNetworkId_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_AddNetworkId_0100 start";
    unordered_set<string> exceptSet;
    exceptSet.insert("test1");
    exceptSet.insert("test2");
    auto testPtr = DeviceManagerAgent::GetInstance();
    testPtr->AddNetworkId(100, "test1");
    testPtr->AddNetworkId(100, "test2");
    EXPECT_EQ(exceptSet, testPtr->GetNetworkIds(100));
    exceptSet.clear();
    testPtr->RemoveNetworkId(100);
    EXPECT_EQ(exceptSet, testPtr->GetNetworkIds(100));
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_AddNetworkId_0100 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_RemoveNetworkIdByOne_0100
 * @tc.desc: Verify the RemoveNetworkIdByOne function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(DeviceManagerAgentSupTest, DeviceManagerAgentTest_RemoveNetworkIdByOne_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_RemoveNetworkIdByOne_0100 start";
    unordered_set<string> exceptSet;
    exceptSet.insert("test1");
    exceptSet.insert("test2");
    auto testPtr = DeviceManagerAgent::GetInstance();
    testPtr->AddNetworkId(100, "test1");
    testPtr->AddNetworkId(100, "test2");
    EXPECT_EQ(exceptSet, testPtr->GetNetworkIds(100));
    testPtr->RemoveNetworkIdByOne(100, "test1");
    exceptSet.erase("test1");
    EXPECT_EQ(exceptSet, testPtr->GetNetworkIds(100));

    testPtr->RemoveNetworkIdByOne(100, "test2");
    exceptSet.erase("test2");
    auto it = testPtr->networkIdMap_.find(100);
    if (it != testPtr->networkIdMap_.end()) {
        EXPECT_TRUE(false);
    } else {
        EXPECT_TRUE(true);
    }

    testPtr->RemoveNetworkIdByOne(100, "test2");
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_RemoveNetworkIdByOne_0100 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_RemoveNetworkIdForAllToken_0100
 * @tc.desc: Verify the RemoveNetworkIdForAllToken function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(DeviceManagerAgentSupTest, DeviceManagerAgentTest_RemoveNetworkIdForAllToken_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_RemoveNetworkIdForAllToken_0100 start";
    auto testPtr = DeviceManagerAgent::GetInstance();
    testPtr->AddNetworkId(100, "test1");
    testPtr->AddNetworkId(100, "test2");
    testPtr->RemoveNetworkIdForAllToken("");
    testPtr->RemoveNetworkIdForAllToken("test1");
    unordered_set<string> exceptSet;
    exceptSet.insert("test2");
    EXPECT_EQ(exceptSet, testPtr->GetNetworkIds(100));

    testPtr->RemoveNetworkIdForAllToken("test2");
    exceptSet.clear();
    EXPECT_EQ(exceptSet, testPtr->GetNetworkIds(100));
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_RemoveNetworkIdForAllToken_0100 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_ClearNetworkId_0100
 * @tc.desc: Verify the ClearNetworkId function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(DeviceManagerAgentSupTest, DeviceManagerAgentTest_ClearNetworkId_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_ClearNetworkId_0100 start";
    unordered_set<string> exceptSet;
    exceptSet.insert("test1");
    exceptSet.insert("test2");
    auto testPtr = DeviceManagerAgent::GetInstance();
    testPtr->AddNetworkId(100, "test1");
    testPtr->AddNetworkId(100, "test2");
    EXPECT_EQ(exceptSet, testPtr->GetNetworkIds(100));
    testPtr->ClearNetworkId();
    EXPECT_EQ(testPtr->networkIdMap_.size(), 0);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_ClearNetworkId_0100 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_MountDfsDocs_0100
 * @tc.desc: Verify the MountDfsDocs function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(DeviceManagerAgentSupTest, DeviceManagerAgentTest_MountDfsDocs_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_MountDfsDocs_0100 start";
    auto testPtr = DeviceManagerAgent::GetInstance();
    constexpr int32_t testCount = 1;
    testPtr->MountDfsDocs("", "");
    testPtr->MountDfsDocs("test", "");
    testPtr->mountDfsCount_["deviceId"] = testCount;
    testPtr->MountDfsDocs("test", "deviceId");
    EXPECT_EQ(testPtr->mountDfsCount_["deviceId"], testCount + 1);
    
    testPtr->mountDfsCount_["deviceId"] = 0;
    EXPECT_CALL(*otherMethodMock_, QueryActiveOsAccountIds(_)).WillOnce(Return(INVALID_USER_ID));
    testPtr->MountDfsDocs("test", "deviceId");
    EXPECT_EQ(testPtr->mountDfsCount_["deviceId"], 0);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_MountDfsDocs_0100 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_UMountDfsDocs_0100
 * @tc.desc: Verify the UMountDfsDocs function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(DeviceManagerAgentSupTest, DeviceManagerAgentTest_UMountDfsDocs_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_UMountDfsDocs_0100 start";
    auto testPtr = DeviceManagerAgent::GetInstance();
    testPtr->UMountDfsDocs("", "", false);
    testPtr->UMountDfsDocs("test", "", false);
    testPtr->mountDfsCount_["deviceId"] = MOUNT_DFS_COUNT_ONE + 1;
    testPtr->UMountDfsDocs("test", "deviceId", false);
    EXPECT_EQ(testPtr->mountDfsCount_["deviceId"], MOUNT_DFS_COUNT_ONE);
    
    EXPECT_CALL(*otherMethodMock_, QueryActiveOsAccountIds(_)).WillOnce(Return(INVALID_USER_ID));
    testPtr->UMountDfsDocs("test", "deviceId", true);
    auto it = testPtr->mountDfsCount_.find("deviceId");
    if (it != testPtr->mountDfsCount_.end()) {
        EXPECT_TRUE(false);
    } else {
        EXPECT_TRUE(true);
    }
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_UMountDfsDocs_0100 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_IsSupportedDevice_0100
 * @tc.desc: Verify the IsSupportedDevice function.
 * @tc.type: FUNC
 * @tc.require: IA6UD3
 */
HWTEST_F(DeviceManagerAgentSupTest, DeviceManagerAgentTest_IsSupportedDevice_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_IsSupportedDevice_0100 start";
    DmDeviceInfo testInfo;
    EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _)).WillOnce(Return(0));
    auto testPtr = DeviceManagerAgent::GetInstance();
    EXPECT_EQ(testPtr->IsSupportedDevice(testInfo), FileManagement::ERR_BAD_VALUE);
    
    std::vector<DmDeviceInfo> deviceList;
    deviceList.push_back(deviceInfo);
    EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0)));
    EXPECT_EQ(testPtr->IsSupportedDevice(testInfo), FileManagement::ERR_BAD_VALUE);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_IsSupportedDevice_0100 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_IsSupportedDevice_0200
 * @tc.desc: Verify the IsSupportedDevice function.
 * @tc.type: FUNC
 * @tc.require: IA6UD3
 */
HWTEST_F(DeviceManagerAgentSupTest, DeviceManagerAgentTest_IsSupportedDevice_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_IsSupportedDevice_0200 start";
    DmDeviceInfo testInfo = {
        .deviceId = "testdevid",
        .deviceName = "testdevname",
        .deviceTypeId = 1,
        .networkId = "testNetWork",
    };
    
    deviceInfo.extraData = "test";
    std::vector<DmDeviceInfo> deviceList;
    deviceList.push_back(deviceInfo);
    EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0)));
    auto testPtr = DeviceManagerAgent::GetInstance();
    EXPECT_EQ(testPtr->IsSupportedDevice(testInfo), FileManagement::ERR_BAD_VALUE);

    deviceInfo.extraData = R"({"OS_TYPE":"3"})";
    deviceList.clear();
    deviceList.push_back(deviceInfo);
    EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0)));
    EXPECT_EQ(testPtr->IsSupportedDevice(testInfo), FileManagement::ERR_BAD_VALUE);

    deviceInfo.extraData = R"({"OS_TYPE":3})";
    deviceList.clear();
    deviceList.push_back(deviceInfo);
    EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0)));
    EXPECT_EQ(testPtr->IsSupportedDevice(testInfo), FileManagement::ERR_BAD_VALUE);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_IsSupportedDevice_0200 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_IsSupportedDevice_0300
 * @tc.desc: Verify the IsSupportedDevice function.
 * @tc.type: FUNC
 * @tc.require: IA6UD3
 */
HWTEST_F(DeviceManagerAgentSupTest, DeviceManagerAgentTest_IsSupportedDevice_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_IsSupportedDevice_0300 start";
    DmDeviceInfo testInfo = {
        .deviceId = "testdevid",
        .deviceName = "testdevname",
        .deviceTypeId = 1,
        .networkId = "testNetWork",
    };

    std::vector<DmDeviceInfo> deviceList;
    deviceInfo.extraData = R"({"OS_TYPE":10})";
    deviceList.push_back(deviceInfo);
    EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0)));
    auto testPtr = DeviceManagerAgent::GetInstance();
    EXPECT_EQ(testPtr->IsSupportedDevice(testInfo), FileManagement::ERR_OK);
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_IsSupportedDevice_0300 end";
}
} // namespace Test
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
