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
#include <gtest/gtest.h>
#include <memory>
#include <unistd.h>

#include "device_manager_impl_mock.h"
#include "dfs_error.h"
#include "ipc_skeleton.h"
#include "mock_other_method.h"
#include "mountpoint/mount_point.h"
#include "network/softbus/softbus_agent.h"


namespace {
uint32_t g_callingId = 0;
}
namespace OHOS {
#ifdef CONFIG_IPC_SINGLE
using namespace IPC_SINGLE;
#endif
uint32_t IPCSkeleton::GetCallingTokenID()
{
    return g_callingId;
}
}

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
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
}

void DeviceManagerAgentSupTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void DeviceManagerAgentSupTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
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

void DeviceManagerAgentSupTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
    DfsDeviceOtherMethodMock::otherMethod = nullptr;
    otherMethodMock_ = nullptr;
    DeviceManagerImplMock::dfsDeviceManagerImpl = nullptr;
    deviceManagerImplMock_ = nullptr;
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
    MountCountInfo tmpInfo("test", 0);
    testPtr->mountDfsCount_.emplace("test", tmpInfo);
    flag = testPtr->MountDfsCountOnly("test");
    EXPECT_EQ(flag, true);
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
    // map can not find
    EXPECT_EQ(testPtr->UMountDfsCountOnly("", true), true);

    // map find and needclear is true
    MountCountInfo info1("networkId", 1);
    testPtr->mountDfsCount_.emplace("test", info1);
    EXPECT_EQ(testPtr->UMountDfsCountOnly("test", false), false);

    // map find & needclear is false & countMap can find
    testPtr->mountDfsCount_.emplace("test", info1);
    testPtr->mountDfsCount_.find("test")->second.callingCountMap_.emplace(1, 2);
    g_callingId = 1;
    EXPECT_EQ(testPtr->UMountDfsCountOnly("test", false), true);

    // map find & needclear is false & countMap can not find
    testPtr->mountDfsCount_.emplace("test", info1);
    g_callingId = 2;
    EXPECT_EQ(testPtr->UMountDfsCountOnly("test", false), false);

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
 * @tc.name: DeviceManagerAgentTest_MountDfsDocs_0100
 * @tc.desc: Verify the MountDfsDocs function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(DeviceManagerAgentSupTest, DeviceManagerAgentTest_MountDfsDocs_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_MountDfsDocs_0100 start";
    auto testPtr = DeviceManagerAgent::GetInstance();
    uint32_t tokenId = 1;

    // networkId or deviceId is invalid
    EXPECT_EQ(testPtr->MountDfsDocs("", "a", tokenId), INVALID_USER_ID);
    EXPECT_EQ(testPtr->MountDfsDocs("a", "", tokenId), INVALID_USER_ID);

    // test mountDfsCountOnly false
    EXPECT_NE(testPtr->MountDfsDocs("a", "a", tokenId), NO_ERROR);

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

    // networkId or deviceId is invalid
    EXPECT_EQ(testPtr->UMountDfsDocs("", "a", true), INVALID_USER_ID);
    EXPECT_EQ(testPtr->UMountDfsDocs("a", "", true), INVALID_USER_ID);

    // networkId or deviceId is valid
    EXPECT_EQ(testPtr->UMountDfsDocs("a", "a", true), NO_ERROR);

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

/**
 * @tc.name: DeviceManagerAgentTest_IncreaseMountDfsCount_Basic_0100
 * @tc.desc: Verify the basic functionality of IncreaseMountDfsCount function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(DeviceManagerAgentSupTest, DeviceManagerAgentTest_IncreaseMountDfsCount_Basic_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_IncreaseMountDfsCount_Basic_0100 start";
    auto testPtr = DeviceManagerAgent::GetInstance();

    const std::string networkId = "test_network_id";
    const std::string mountPath = "/test/mount/path";
    const uint32_t callingTokenId = 1001;

    // 测试场景1: 首次为某个mountPath增加计数（map中不存在）
    testPtr->IncreaseMountDfsCount(networkId, mountPath, callingTokenId);

    // 验证结果
    auto iter = testPtr->mountDfsCount_.find(mountPath);
    ASSERT_NE(iter, testPtr->mountDfsCount_.end());
    EXPECT_EQ(iter->second.networkId_, networkId);

    auto &callingCountMap = iter->second.callingCountMap_;
    auto callingIter = callingCountMap.find(callingTokenId);
    ASSERT_NE(callingIter, callingCountMap.end());
    EXPECT_EQ(callingIter->second, 1);

    // 清理测试数据
    testPtr->mountDfsCount_.clear();
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_IncreaseMountDfsCount_Basic_0100 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_IncreaseMountDfsCount_SameToken_0200
 * @tc.desc: Verify IncreaseMountDfsCount with same callingTokenId multiple times.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(DeviceManagerAgentSupTest, DeviceManagerAgentTest_IncreaseMountDfsCount_SameToken_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_IncreaseMountDfsCount_SameToken_0200 start";
    auto testPtr = DeviceManagerAgent::GetInstance();

    const std::string networkId = "test_network_id";
    const std::string mountPath = "/test/mount/path";
    const uint32_t callingTokenId = 1001;

    // 第一次增加
    testPtr->IncreaseMountDfsCount(networkId, mountPath, callingTokenId);
    
    // 第二次增加（同一个callingTokenId）
    testPtr->IncreaseMountDfsCount(networkId, mountPath, callingTokenId);

    // 验证计数增加
    auto iter = testPtr->mountDfsCount_.find(mountPath);
    ASSERT_NE(iter, testPtr->mountDfsCount_.end());

    auto &callingCountMap = iter->second.callingCountMap_;
    auto callingIter = callingCountMap.find(callingTokenId);
    ASSERT_NE(callingIter, callingCountMap.end());
    EXPECT_EQ(callingIter->second, 2);

    // 清理测试数据
    testPtr->mountDfsCount_.clear();
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_IncreaseMountDfsCount_SameToken_0200 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_IncreaseMountDfsCount_DifferentToken_0300
 * @tc.desc: Verify IncreaseMountDfsCount with different callingTokenIds.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(DeviceManagerAgentSupTest, DeviceManagerAgentTest_IncreaseMountDfsCount_DifferentToken_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_IncreaseMountDfsCount_DifferentToken_0300 start";
    auto testPtr = DeviceManagerAgent::GetInstance();

    const std::string networkId = "test_network_id";
    const std::string mountPath = "/test/mount/path";
    const uint32_t callingTokenId = 1001;
    const uint32_t anotherCallingTokenId = 1002;

    // 为第一个token增加两次
    testPtr->IncreaseMountDfsCount(networkId, mountPath, callingTokenId);
    testPtr->IncreaseMountDfsCount(networkId, mountPath, callingTokenId);

    // 为第二个token增加一次
    testPtr->IncreaseMountDfsCount(networkId, mountPath, anotherCallingTokenId);

    // 验证新的token被添加
    auto iter = testPtr->mountDfsCount_.find(mountPath);
    ASSERT_NE(iter, testPtr->mountDfsCount_.end());

    auto &callingCountMap = iter->second.callingCountMap_;
    
    // 验证第二个token
    auto callingIter = callingCountMap.find(anotherCallingTokenId);
    ASSERT_NE(callingIter, callingCountMap.end());
    EXPECT_EQ(callingIter->second, 1);

    // 验证第一个token计数不变
    callingIter = callingCountMap.find(callingTokenId);
    ASSERT_NE(callingIter, callingCountMap.end());
    EXPECT_EQ(callingIter->second, 2);

    // 清理测试数据
    testPtr->mountDfsCount_.clear();
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_IncreaseMountDfsCount_DifferentToken_0300 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_RemoveMountDfsCount_0400
 * @tc.desc: Verify the RemoveMountDfsCount function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(DeviceManagerAgentSupTest, DeviceManagerAgentTest_RemoveMountDfsCount_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_RemoveMountDfsCount_0400 start";
    auto testPtr = DeviceManagerAgent::GetInstance();

    const std::string networkId = "test_network_id";
    const std::string mountPath = "/test/mount/path";
    const uint32_t callingTokenId = 1001;

    // 先增加计数
    testPtr->IncreaseMountDfsCount(networkId, mountPath, callingTokenId);

    // 移除存在的mountPath
    testPtr->RemoveMountDfsCount(mountPath);

    // 验证mountPath已被移除
    auto iter = testPtr->mountDfsCount_.find(mountPath);
    EXPECT_EQ(iter, testPtr->mountDfsCount_.end());

    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_RemoveMountDfsCount_0400 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_RemoveMountDfsCount_NonExistent_0500
 * @tc.desc: Verify RemoveMountDfsCount with non-existent mountPath.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(DeviceManagerAgentSupTest, DeviceManagerAgentTest_RemoveMountDfsCount_NonExistent_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_RemoveMountDfsCount_NonExistent_0500 start";
    auto testPtr = DeviceManagerAgent::GetInstance();

    const std::string nonExistentPath = "/non/existent/path";

    // 移除不存在的mountPath（错误路径）
    testPtr->RemoveMountDfsCount(nonExistentPath);

    // 验证map状态不变
    auto iter = testPtr->mountDfsCount_.find(nonExistentPath);
    EXPECT_EQ(iter, testPtr->mountDfsCount_.end());

    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_RemoveMountDfsCount_NonExistent_0500 end";
}

/**
 * @tc.name: DeviceManagerAgentTest_IncreaseMountDfsCount_Concurrent_0600
 * @tc.desc: Verify the IncreaseMountDfsCount function with concurrent access.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(DeviceManagerAgentSupTest, DeviceManagerAgentTest_IncreaseMountDfsCount_Concurrent_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_IncreaseMountDfsCount_Concurrent_0600 start";
    auto testPtr = DeviceManagerAgent::GetInstance();

    const std::string mountPath = "/test/mount/path";
    const uint32_t callingTokenId = 1001;
    const int threadCount = 5;
    const int operationsPerThread = 100;

    std::vector<std::thread> threads;

    // 启动多个线程并发增加计数
    for (int i = 0; i < threadCount; ++i) {
        threads.emplace_back([&, i]() {
            for (int j = 0; j < operationsPerThread; ++j) {
                testPtr->IncreaseMountDfsCount("network_" + std::to_string(i), mountPath + std::to_string(i),
                                               callingTokenId + i);
            }
        });
    }

    // 等待所有线程完成
    for (auto &thread : threads) {
        thread.join();
    }

    // 验证结果：每个mountPath都应该有正确的计数
    for (int i = 0; i < threadCount; ++i) {
        std::string currentPath = mountPath + std::to_string(i);
        auto iter = testPtr->mountDfsCount_.find(currentPath);
        ASSERT_NE(iter, testPtr->mountDfsCount_.end());

        auto &callingCountMap = iter->second.callingCountMap_;
        auto callingIter = callingCountMap.find(callingTokenId + i);
        ASSERT_NE(callingIter, callingCountMap.end());
        EXPECT_EQ(callingIter->second, operationsPerThread);
    }

    // 清理测试数据
    testPtr->mountDfsCount_.clear();
    GTEST_LOG_(INFO) << "DeviceManagerAgentTest_IncreaseMountDfsCount_Concurrent_0600 end";
}

} // namespace Test
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
