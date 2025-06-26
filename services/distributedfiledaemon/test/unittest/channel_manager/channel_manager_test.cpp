/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "channel_manager.h"
#include "control_cmd_parser.h"
#include "device_manager.h"
#include "dfs_error.h"
#include "network/softbus/softbus_permission_check.h"
#include "socket_mock.h"
#include "system_notifier.h"
#include "utils_log.h"
#include <nlohmann/json.hpp>

#include <thread>

namespace {
int32_t g_mockSocket = -1;
bool g_checkSrcPermission = true;
bool g_setAccessInfoToSocket = true;
bool g_transCallerInfo = true;
bool g_fillLocalInfo = true;

int g_callSerializeToJsonTimes = 0;
bool g_resSerializeToJson = true;
bool g_isSameAccount = true;
} // namespace

namespace OHOS {
namespace Storage {
namespace DistributedFile {

bool SoftBusPermissionCheck::CheckSrcPermission(const std::string &sinkNetworkId)
{
    GTEST_LOG_(INFO) << "enter mock CheckSrcPermission";
    return g_checkSrcPermission;
}

bool SoftBusPermissionCheck::SetAccessInfoToSocket(const int32_t sessionId)
{
    GTEST_LOG_(INFO) << "enter mock SetAccessInfoToSocket";
    return g_setAccessInfoToSocket;
}

bool SoftBusPermissionCheck::TransCallerInfo(SocketAccessInfo *callerInfo,
                                             AccountInfo &callerAccountInfo,
                                             const std::string &networkId)
{
    GTEST_LOG_(INFO) << "enter mock TransCallerInfo";
    return g_transCallerInfo;
}

bool SoftBusPermissionCheck::FillLocalInfo(SocketAccessInfo *localInfo)
{
    GTEST_LOG_(INFO) << "enter mock FillLocalInfo";
    return g_fillLocalInfo;
}

bool SoftBusPermissionCheck::CheckSinkPermission(const AccountInfo &callerAccountInfo)
{
    GTEST_LOG_(INFO) << "enter mock CheckSinkPermission";
    return true;
}

bool ControlCmdParser::SerializeToJson(const ControlCmd &cmd, std::string &outJsonStr)
{
    GTEST_LOG_(INFO) << "enter mock SerializeToJson";
    g_callSerializeToJsonTimes++;
    return g_resSerializeToJson;
}

bool SoftBusPermissionCheck::IsSameAccount(const std::string &networkId)
{
    return g_isSameAccount;
}

namespace Test {
using namespace std;
using namespace OHOS::Storage::DistributedFile;
using namespace testing;
using namespace testing::ext;
using namespace FileManagement;
using OHOS::FileManagement::ERR_BAD_VALUE;
using OHOS::FileManagement::ERR_OK;

class ChannelManagerTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        GTEST_LOG_(INFO) << "ChannelManagerTest SetUpTestCase";
    }

    static void TearDownTestCase()
    {
        GTEST_LOG_(INFO) << "ChannelManagerTest TearDownTestCase";
    }

    void SetUp()
    {
        GTEST_LOG_(INFO) << "ChannelManagerTest SetUp";
        socketMock_ = make_shared<SocketMock>();
        SocketMock::dfsSocket = socketMock_;
        g_mockSocket = -1;
        g_checkSrcPermission = true;
        g_setAccessInfoToSocket = true;
        g_transCallerInfo = true;
        g_fillLocalInfo = true;
        g_callSerializeToJsonTimes = 0;
        g_resSerializeToJson = true;
        g_isSameAccount = true;
    }

    void TearDown(void)
    {
        GTEST_LOG_(INFO) << "ChannelManagerTest TearDown";
        ChannelManager::GetInstance().DeInit();
        SocketMock::dfsSocket = nullptr;
        socketMock_ = nullptr;
    }

private:
    static inline shared_ptr<SocketMock> socketMock_ = nullptr;
    static inline std::string defaultNetworkId = "networkId-1";
    static inline void InitChannelManager()
    {
        EXPECT_CALL(*socketMock_, Socket(_)).WillOnce(Return(1));
        EXPECT_CALL(*socketMock_, Listen(_, _, _, _)).WillOnce(Return(0));
        EXPECT_EQ(ChannelManager::GetInstance().Init(), ERR_OK);
    }
};

/**
 * @tc.name: ChannelManagerTest_Init_001
 * @tc.desc: Verify successful initialization when both socket creation and listening succeed
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ChannelManagerTest, ChannelManagerTest_Init_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChannelManagerTest_Init_001 start";

    // Setup mock expectations for successful initialization path
    EXPECT_CALL(*socketMock_, Socket(_)).WillOnce(Return(1));
    EXPECT_CALL(*socketMock_, Listen(_, _, _, _)).WillOnce(Return(0));
    EXPECT_EQ(ChannelManager::GetInstance().Init(), ERR_OK);

    GTEST_LOG_(INFO) << "ChannelManagerTest_Init_001 end";
}

/**
 * @tc.name: ChannelManagerTest_Init_002
 * @tc.desc: Verify initialization failure when socket creation fails
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ChannelManagerTest, ChannelManagerTest_Init_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChannelManagerTest_Init_002 start";

    // Test error handling for socket creation failure
    EXPECT_CALL(*socketMock_, Socket(_)).WillOnce(Return(-1));
    EXPECT_EQ(ChannelManager::GetInstance().Init(), ERR_CREATE_SOCKET_FAILED);

    GTEST_LOG_(INFO) << "ChannelManagerTest_Init_002 end";
}

/**
 * @tc.name: ChannelManagerTest_Init_003
 * @tc.desc: Verify initialization failure when socket listening fails
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ChannelManagerTest, ChannelManagerTest_Init_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChannelManagerTest_Init_003 start";

    // Test error handling for listen failure after successful socket creation
    EXPECT_CALL(*socketMock_, Socket(_)).WillOnce(Return(1));
    EXPECT_CALL(*socketMock_, Listen(_, _, _, _)).WillOnce(Return(-1));
    EXPECT_EQ(ChannelManager::GetInstance().Init(), ERR_LISTEN_SOCKET_FAILED);

    GTEST_LOG_(INFO) << "ChannelManagerTest_Init_003 end";
}

/**
 * @tc.name: ChannelManagerTest_Init_004
 * @tc.desc: Verify idempotent behavior when initializing already initialized manager
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ChannelManagerTest, ChannelManagerTest_Init_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChannelManagerTest_Init_004 start";

    // First initialization should succeed
    EXPECT_CALL(*socketMock_, Socket(_)).WillOnce(Return(1));
    EXPECT_CALL(*socketMock_, Listen(_, _, _, _)).WillOnce(Return(0));
    EXPECT_EQ(ChannelManager::GetInstance().Init(), ERR_OK);

    // Verify subsequent initialization returns success without reinitializing
    EXPECT_EQ(ChannelManager::GetInstance().Init(), ERR_OK);

    GTEST_LOG_(INFO) << "ChannelManagerTest_Init_004 end";
}

HWTEST_F(ChannelManagerTest, ChannelManagerTest_OnNegotiate2_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChannelManagerTest_OnNegotiate2_001 start";

    // Setup test varaible
    PeerSocketInfo peerSocketInfo;
    char networkId[] = "test-networkId";
    peerSocketInfo.networkId = networkId;

    SocketAccessInfo socketAccessInfo;
    SocketAccessInfo socketAccessInfo2;

    // Test success case
    g_transCallerInfo = true;
    g_fillLocalInfo = true;
    EXPECT_TRUE(ChannelManager::GetInstance().OnNegotiate2(100, peerSocketInfo, &socketAccessInfo, &socketAccessInfo2));

    // Test TransCallerInfo failure case
    g_transCallerInfo = false;
    g_fillLocalInfo = true;
    EXPECT_FALSE(
        ChannelManager::GetInstance().OnNegotiate2(100, peerSocketInfo, &socketAccessInfo, &socketAccessInfo2));

    // Test FillLocalInfo failure case
    g_transCallerInfo = true;
    g_fillLocalInfo = false;
    EXPECT_FALSE(
        ChannelManager::GetInstance().OnNegotiate2(100, peerSocketInfo, &socketAccessInfo, &socketAccessInfo2));

    GTEST_LOG_(INFO) << "ChannelManagerTest_OnNegotiate2_001 end";
}

/**
 * @tc.name: ChannelManagerTest_CreateClientChannel_001
 * @tc.desc: Verify channel creation fails when source permission check fails
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ChannelManagerTest, ChannelManagerTest_CreateClientChannel_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChannelManagerTest_CreateClientChannel_001 start";

    // Test permission check failure path
    g_checkSrcPermission = false;
    EXPECT_EQ(ChannelManager::GetInstance().CreateClientChannel(defaultNetworkId), ERR_CHECK_PERMISSION_FAILED);

    GTEST_LOG_(INFO) << "ChannelManagerTest_CreateClientChannel_001 end";
}

/**
 * @tc.name: ChannelManagerTest_CreateClientChannel_002
 * @tc.desc: Verify channel creation fails when socket creation fails
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ChannelManagerTest, ChannelManagerTest_CreateClientChannel_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChannelManagerTest_CreateClientChannel_002 start";

    // Test socket creation failure path
    int32_t expectedSocketId = -1;
    EXPECT_CALL(*socketMock_, Socket(_)).WillOnce(Return(expectedSocketId));
    EXPECT_EQ(ChannelManager::GetInstance().CreateClientChannel(defaultNetworkId), expectedSocketId);

    GTEST_LOG_(INFO) << "ChannelManagerTest_CreateClientChannel_002 end";
}

/**
 * @tc.name: ChannelManagerTest_CreateClientChannel_003
 * @tc.desc: Verify channel creation fails when setting access info fails
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ChannelManagerTest, ChannelManagerTest_CreateClientChannel_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChannelManagerTest_CreateClientChannel_003 start";

    // Test access info setting failure path
    g_setAccessInfoToSocket = false;
    int32_t expectedSocketId = 1;
    EXPECT_CALL(*socketMock_, Socket(_)).WillOnce(Return(expectedSocketId));
    EXPECT_EQ(ChannelManager::GetInstance().CreateClientChannel(defaultNetworkId), ERR_BAD_VALUE);

    GTEST_LOG_(INFO) << "ChannelManagerTest_CreateClientChannel_003 end";
}

/**
 * @tc.name: ChannelManagerTest_CreateClientChannel_004
 * @tc.desc: Verify channel creation fails when socket binding fails
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ChannelManagerTest, ChannelManagerTest_CreateClientChannel_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChannelManagerTest_CreateClientChannel_004 start";

    // Test socket binding failure path
    int32_t expectedSocketId = 1;
    EXPECT_CALL(*socketMock_, Socket(_)).WillOnce(Return(expectedSocketId));
    EXPECT_CALL(*socketMock_, Bind(_, _, _, _)).WillOnce(Return(ERR_BAD_VALUE));
    EXPECT_EQ(ChannelManager::GetInstance().CreateClientChannel(defaultNetworkId), ERR_BIND_SOCKET_FAILED);

    GTEST_LOG_(INFO) << "ChannelManagerTest_CreateClientChannel_004 end";
}

/**
 * @tc.name: ChannelManagerTest_CreateClientChannel_005
 * @tc.desc: Verify successful channel creation and duplicate channel handling
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ChannelManagerTest, ChannelManagerTest_CreateClientChannel_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChannelManagerTest_CreateClientChannel_005 start";

    // Test successful channel creation path
    int32_t expectedSocketId = 1;
    EXPECT_CALL(*socketMock_, Socket(_)).WillOnce(Return(expectedSocketId));
    EXPECT_CALL(*socketMock_, Bind(_, _, _, _)).WillOnce(Return(ERR_OK));
    EXPECT_EQ(ChannelManager::GetInstance().CreateClientChannel(defaultNetworkId), ERR_OK);

    // Test duplicate channel creation returns success without recreation
    EXPECT_EQ(ChannelManager::GetInstance().CreateClientChannel(defaultNetworkId), ERR_OK);

    GTEST_LOG_(INFO) << "ChannelManagerTest_CreateClientChannel_005 end";
}

/**
 * @tc.name: ChannelManagerTest_DestroyClientChannel_001
 * @tc.desc: Verify DestroyClientChannel returns error when destroying non-existent channel
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ChannelManagerTest, ChannelManagerTest_DestroyClientChannel_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChannelManagerTest_DestroyClientChannel_001 start";

    // Setup: Create a channel but set invalid socket ID (-1)
    int32_t expectedSocketId = 1;
    EXPECT_CALL(*socketMock_, Socket(_)).WillOnce(Return(expectedSocketId));
    EXPECT_CALL(*socketMock_, Bind(_, _, _, _)).WillOnce(Return(ERR_OK));
    EXPECT_EQ(ChannelManager::GetInstance().CreateClientChannel(defaultNetworkId), ERR_OK);

    // Test case 1: Attempt to destroy channel that was never created
    // Expected: First attempt should success (channel exists above)
    EXPECT_EQ(ChannelManager::GetInstance().DestroyClientChannel(defaultNetworkId), ERR_OK);

    // Test case 2: Verify repeated destruction attempt also fails
    // Expected: Consistent error for non-existent channel
    EXPECT_EQ(ChannelManager::GetInstance().DestroyClientChannel(defaultNetworkId), ERR_BAD_VALUE);

    GTEST_LOG_(INFO) << "ChannelManagerTest_DestroyClientChannel_001 end";
}

/**
 * @tc.name: ChannelManagerTest_DestroyClientChannel_002
 * @tc.desc: Verify DestroyClientChannel fails when socket ID is invalid
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ChannelManagerTest, ChannelManagerTest_DestroyClientChannel_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChannelManagerTest_DestroyClientChannel_002 start";

    // Setup: Create a channel but set invalid socket ID (-1)
    int32_t expectedSocketId = 1;
    EXPECT_CALL(*socketMock_, Socket(_)).WillOnce(Return(expectedSocketId));
    EXPECT_CALL(*socketMock_, Bind(_, _, _, _)).WillOnce(Return(ERR_OK));
    EXPECT_EQ(ChannelManager::GetInstance().CreateClientChannel(defaultNetworkId), ERR_OK);

    // Force invalid socket ID for test
    ChannelManager::GetInstance().clientNetworkSocketMap_[defaultNetworkId] = -1;

    // Test: Attempt to destroy channel with invalid socket ID
    // Expected: Should return error due to invalid socket
    EXPECT_EQ(ChannelManager::GetInstance().DestroyClientChannel(defaultNetworkId), ERR_BAD_VALUE);

    GTEST_LOG_(INFO) << "ChannelManagerTest_DestroyClientChannel_002 end";
}

/**
 * @tc.name: ChannelManagerTest_OnSocketClosed_001
 * @tc.desc: Verify OnSocketClosed no throw
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ChannelManagerTest, ChannelManagerTest_OnSocketClosed_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChannelManagerTest_OnSocketClosed_001 start";

    auto &instance = ChannelManager::GetInstance();
    instance.clientNetworkSocketMap_[defaultNetworkId + "client-1"] = 11;
    instance.clientNetworkSocketMap_[defaultNetworkId + "client-2"] = 22;
    instance.serverNetworkSocketMap_[defaultNetworkId + "server-1"] = 33;
    instance.serverNetworkSocketMap_[defaultNetworkId + "server-2"] = 44;

    EXPECT_NO_THROW(ChannelManager::GetInstance().OnSocketClosed(22, SHUTDOWN_REASON_UNKNOWN));
    EXPECT_TRUE(instance.clientNetworkSocketMap_.size() == 1);
    EXPECT_TRUE(instance.serverNetworkSocketMap_.size() == 2);

    EXPECT_NO_THROW(ChannelManager::GetInstance().OnSocketClosed(44, SHUTDOWN_REASON_UNKNOWN));
    EXPECT_TRUE(instance.serverNetworkSocketMap_.size() == 1);

    GTEST_LOG_(INFO) << "ChannelManagerTest_OnSocketClosed_001 end";
}

} // namespace Test
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS