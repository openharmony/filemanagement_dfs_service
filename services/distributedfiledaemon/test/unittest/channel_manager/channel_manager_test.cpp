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

/**
 * @tc.name: ChannelManagerTest_OnBytesReceived_001
 * @tc.desc: Verify rejection of null/empty/oversized data
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ChannelManagerTest, ChannelManagerTest_OnBytesReceived_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChannelManagerTest_OnBytesReceived_001 start";

    // Test invalid socket ID (<= 0)
    const char *testData = "dummy";
    EXPECT_NO_THROW(ChannelManager::GetInstance().OnBytesReceived(-1, testData, strlen(testData)));

    // Test null data
    EXPECT_NO_THROW(ChannelManager::GetInstance().OnBytesReceived(1, nullptr, 10));

    // Test zero length
    const char *emptyData = "";
    EXPECT_NO_THROW(ChannelManager::GetInstance().OnBytesReceived(1, emptyData, 0));

    // Test oversized data
    std::vector<char> bigData(80 * 1024 * 1024 + 1, 'A');
    EXPECT_NO_THROW(ChannelManager::GetInstance().OnBytesReceived(1, bigData.data(), bigData.size()));

    // Verify ParseFromJson failure logs
    const char *invalidJson = "{invalid: json}";
    EXPECT_NO_THROW(ChannelManager::GetInstance().OnBytesReceived(1, invalidJson, strlen(invalidJson)));

    GTEST_LOG_(INFO) << "ChannelManagerTest_OnBytesReceived_001 end";
}

/**
 * @tc.name: ChannelManagerTest_OnBytesReceived_002
 * @tc.desc: Verify RESPONSE message handling
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ChannelManagerTest, ChannelManagerTest_OnBytesReceived_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChannelManagerTest_OnBytesReceived_002 start";

    // Init
    EXPECT_CALL(*socketMock_, Socket(_)).WillOnce(Return(1));
    EXPECT_CALL(*socketMock_, Listen(_, _, _, _)).WillOnce(Return(0));
    EXPECT_EQ(ChannelManager::GetInstance().Init(), ERR_OK);

    // Setup test response
    nlohmann::json responseJson = {{"msgId", 123}, {"msgType", ControlCmdType::CMD_MSG_RESPONSE}, {"msgBody", "test"}};
    std::string jsonStr = responseJson.dump();

    // Register mock waiter
    auto waiter = std::make_shared<ChannelManager::ResponseWaiter>();
    {
        std::lock_guard<std::mutex> lock(ChannelManager::GetInstance().mtx_);
        ChannelManager::GetInstance().pendingResponses_[123] = waiter;
    }

    // Trigger response
    ChannelManager::GetInstance().OnBytesReceived(1, jsonStr.data(), jsonStr.size());

    // Wait for async task to complete (关键修改点)
    std::unique_lock<std::mutex> lock(waiter->mutex);
    waiter->cv.wait(lock, [&waiter] { return waiter->received; });

    // Verify
    EXPECT_TRUE(waiter->received);

    GTEST_LOG_(INFO) << "ChannelManagerTest_OnBytesReceived_002 end";
}

/**
 * @tc.name: ChannelManagerTest_OnBytesReceived_004
 * @tc.desc: Verify REQUEST message handling with successful response
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ChannelManagerTest, ChannelManagerTest_OnBytesReceived_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChannelManagerTest_OnBytesReceived_003 start";

    // init
    EXPECT_CALL(*socketMock_, Socket(_)).WillOnce(Return(1));
    EXPECT_CALL(*socketMock_, Listen(_, _, _, _)).WillOnce(Return(0));
    EXPECT_EQ(ChannelManager::GetInstance().Init(), ERR_OK);

    // Mock a valid request
    nlohmann::json requestJson = {{"msgId", 456},
                                  {"msgType", OHOS::Storage::DistributedFile::ControlCmdType::CMD_CHECK_ALLOW_CONNECT},
                                  {"msgBody", "query"}};
    std::string jsonStr = requestJson.dump();

    // Setup mock for successful handling

    // Expect DoSendBytes to be called
    EXPECT_CALL(*socketMock_, SendBytes(_, _, _)).WillOnce(Return(ERR_OK));

    ChannelManager::GetInstance().OnBytesReceived(1, jsonStr.data(), jsonStr.size());

    GTEST_LOG_(INFO) << "ChannelManagerTest_OnBytesReceived_003 end";
}

/**
 * @tc.name: ChannelManagerTest_OnBytesReceived_004
 * @tc.desc: Verify REQUEST message handling with failed processing
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ChannelManagerTest, ChannelManagerTest_OnBytesReceived_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChannelManagerTest_OnBytesReceived_004 start";

    nlohmann::json requestJson = {{"msgId", 789}, {"msgType", -1}, {"msgBody", "invalid"}};
    std::string jsonStr = requestJson.dump();

    // Verify no SendBytes is called for invalid requests
    EXPECT_CALL(*socketMock_, SendBytes(_, _, _)).Times(0);

    ChannelManager::GetInstance().OnBytesReceived(1, jsonStr.data(), jsonStr.size());
    // Verify error logging

    GTEST_LOG_(INFO) << "ChannelManagerTest_OnBytesReceived_004 end";
}

/**
 * @tc.name: ChannelManagerTest_OnBytesReceived_005
 * @tc.desc: Verify unknown message type handling
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ChannelManagerTest, ChannelManagerTest_OnBytesReceived_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChannelManagerTest_OnBytesReceived_005 start";

    nlohmann::json unknownJson = {{"msgId", 999}, {"msgType", ControlCmdType::CMD_UNKNOWN}, {"msgBody", "unknown"}};
    std::string jsonStr = unknownJson.dump();

    // Expect no response sent
    EXPECT_CALL(*socketMock_, SendBytes(_, _, _)).Times(0);

    ChannelManager::GetInstance().OnBytesReceived(1, jsonStr.data(), jsonStr.size());

    GTEST_LOG_(INFO) << "ChannelManagerTest_OnBytesReceived_005 end";
}

/**
 * @tc.name: ChannelManagerTest_SendRequest_001
 * @tc.desc: Verify SendRequest returns ERR_NO_EXIST_CHANNEL when networkId not found
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ChannelManagerTest, ChannelManagerTest_SendRequest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChannelManagerTest_SendRequest_001 start";

    ControlCmd request, response;
    request.msgId = 1;
    request.msgType = ControlCmdType::CMD_CHECK_ALLOW_CONNECT;

    // Mock empty client map
    EXPECT_CALL(*socketMock_, Socket(_)).Times(0); // Ensure no socket operation

    // Call with non-existent networkId
    int32_t ret = ChannelManager::GetInstance().SendRequest("invalid_network", request, response, false);
    EXPECT_EQ(ret, ERR_NO_EXIST_CHANNEL);

    GTEST_LOG_(INFO) << "ChannelManagerTest_SendRequest_001 end";
}

/**
 * @tc.name: ChannelManagerTest_SendRequest_002
 * @tc.desc: Verify SendRequest handles serialization failure
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ChannelManagerTest, ChannelManagerTest_SendRequest_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChannelManagerTest_SendRequest_002 start";

    // init
    InitChannelManager();

    ControlCmd request, response;
    request.msgId = 2;
    request.msgType = ControlCmdType::CMD_CHECK_ALLOW_CONNECT;

    // Setup valid networkId
    ChannelManager::GetInstance().clientNetworkSocketMap_["valid_network"] = 1;

    int32_t ret = ChannelManager::GetInstance().SendRequest("valid_network", request, response, false);
    EXPECT_EQ(ret, E_OK);

    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_EQ(g_callSerializeToJsonTimes, 1);

    GTEST_LOG_(INFO) << "ChannelManagerTest_SendRequest_002 end";
}

/**
 * @tc.name: ChannelManagerTest_SendRequest_003
 * @tc.desc: Verify SendRequest handles task post failure
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ChannelManagerTest, ChannelManagerTest_SendRequest_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChannelManagerTest_SendRequest_003 start";

    ControlCmd request, response;
    request.msgId = 3;
    request.msgType = ControlCmdType::CMD_CHECK_ALLOW_CONNECT;

    // Setup valid networkId and mock serialization
    ChannelManager::GetInstance().clientNetworkSocketMap_["valid_network"] = 1;

    // Mock task post failure
    int32_t ret = ChannelManager::GetInstance().SendRequest("valid_network", request, response, true);
    EXPECT_EQ(ret, ERR_NULL_EVENT_HANDLER);

    // Verify pendingResponses_ is cleaned up
    std::lock_guard<std::mutex> lock(ChannelManager::GetInstance().mtx_);
    EXPECT_EQ(ChannelManager::GetInstance().pendingResponses_.count(3), 0);

    GTEST_LOG_(INFO) << "ChannelManagerTest_SendRequest_003 end";
}

/**
 * @tc.name: ChannelManagerTest_SendRequest_004
 * @tc.desc: Verify successful SendRequest without response
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ChannelManagerTest, ChannelManagerTest_SendRequest_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChannelManagerTest_SendRequest_004 start";

    // init
    InitChannelManager();

    ControlCmd request, response;
    request.msgId = 4;
    request.msgType = ControlCmdType::CMD_CHECK_ALLOW_CONNECT;

    // Setup valid networkId
    ChannelManager::GetInstance().clientNetworkSocketMap_["valid_network"] = 1;

    EXPECT_CALL(*socketMock_, SendBytes(_, _, _)).Times(1).WillOnce(Return(ERR_OK));

    int32_t ret = ChannelManager::GetInstance().SendRequest("valid_network", request, response, false);
    EXPECT_EQ(ret, E_OK);

    // wait for SendBytes to be called
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Verify no pending response added
    std::lock_guard<std::mutex> lock(ChannelManager::GetInstance().mtx_);
    EXPECT_EQ(ChannelManager::GetInstance().pendingResponses_.count(4), 0);

    GTEST_LOG_(INFO) << "ChannelManagerTest_SendRequest_004 end";
}

/**
 * @tc.name: ChannelManagerTest_SendRequest_005
 * @tc.desc: Verify SendRequest timeout when waiting for response
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ChannelManagerTest, ChannelManagerTest_SendRequest_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChannelManagerTest_SendRequest_005 start";

    // init
    InitChannelManager();

    ControlCmd request, response;
    request.msgId = 5;
    request.msgType = ControlCmdType::CMD_CHECK_ALLOW_CONNECT;

    // Setup valid networkId
    ChannelManager::GetInstance().clientNetworkSocketMap_["valid_network"] = 1;

    // Mock successful task post but no response

    int32_t ret = ChannelManager::GetInstance().SendRequest("valid_network", request, response, true);
    EXPECT_EQ(ret, E_TIMEOUT);

    // Verify pending response is cleaned up
    std::lock_guard<std::mutex> lock(ChannelManager::GetInstance().mtx_);
    EXPECT_EQ(ChannelManager::GetInstance().pendingResponses_.count(5), 0);

    GTEST_LOG_(INFO) << "ChannelManagerTest_SendRequest_005 end";
}

/**
 * @tc.name: ChannelManagerTest_SendRequest_006
 * @tc.desc: Verify successful SendRequest with response
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ChannelManagerTest, ChannelManagerTest_SendRequest_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChannelManagerTest_SendRequest_006 start";

    // init
    InitChannelManager();

    ControlCmd request, response;
    request.msgId = 6;
    request.msgType = ControlCmdType::CMD_CHECK_ALLOW_CONNECT;

    // Setup valid networkId
    ChannelManager::GetInstance().clientNetworkSocketMap_["valid_network"] = 1;

    // Mock successful path with simulated response
    nlohmann::json responseJson = {{"msgId", 6}, {"msgType", ControlCmdType::CMD_MSG_RESPONSE}, {"msgBody", "test"}};
    std::string jsonStr = responseJson.dump();

    // Start a thread to simulate delayed response
    std::thread responseThread([jsonStr]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        ChannelManager::GetInstance().OnBytesReceived(1, jsonStr.c_str(), jsonStr.size());
    });
    responseThread.detach(); // Detach the thread to run independently

    int32_t ret = ChannelManager::GetInstance().SendRequest("valid_network", request, response, true);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(response.msgId, 6); // Verify response content

    GTEST_LOG_(INFO) << "ChannelManagerTest_SendRequest_006 end";
}

/**
 * @tc.name: ChannelManagerTest_PostTask_001
 * @tc.desc: Verify PostTask failed with null func callback
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ChannelManagerTest, ChannelManagerTest_PostTask_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChannelManagerTest_PostTask_001 start";

    // init
    InitChannelManager();

    // test nullptr
    int32_t ret = ChannelManager::GetInstance().PostTask(nullptr, AppExecFwk::EventQueue::Priority::IMMEDIATE);
    EXPECT_EQ(ret, ERR_POST_TASK_FAILED);

    GTEST_LOG_(INFO) << "ChannelManagerTest_PostTask_001 end";
}

/**
 * @tc.name: ChannelManagerTest_PostCallbackTask_001
 * @tc.desc: Verify PostCallbackTask failed with null func callback
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ChannelManagerTest, ChannelManagerTest_PostCallbackTask_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChannelManagerTest_PostCallbackTask_001 start";

    // init
    InitChannelManager();

    // test nullptr
    int32_t ret = ChannelManager::GetInstance().PostCallbackTask(nullptr, AppExecFwk::EventQueue::Priority::IMMEDIATE);
    EXPECT_EQ(ret, ERR_POST_TASK_FAILED);

    GTEST_LOG_(INFO) << "ChannelManagerTest_PostCallbackTask_001 end";
}

/**
 * @tc.name: ChannelManagerTest_NotifyClient_001
 * @tc.desc: Verify NotifyClient returns ERR_NO_EXIST_CHANNEL when networkId is not found
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ChannelManagerTest, ChannelManagerTest_NotifyClient_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChannelManagerTest_NotifyClient_001 start";

    // Setup: No networkId in serverNetworkSocketMap_
    ControlCmd request;
    request.msgId = 1;
    request.msgType = ControlCmdType::CMD_CHECK_ALLOW_CONNECT;

    // Test: Call NotifyClient with non-existent networkId
    int32_t ret = ChannelManager::GetInstance().NotifyClient("invalid_network", request);
    EXPECT_EQ(ret, ERR_NO_EXIST_CHANNEL);

    GTEST_LOG_(INFO) << "ChannelManagerTest_NotifyClient_001 end";
}

/**
 * @tc.name: ChannelManagerTest_NotifyClient_002
 * @tc.desc: Verify NotifyClient handles serialization failure
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ChannelManagerTest, ChannelManagerTest_NotifyClient_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChannelManagerTest_NotifyClient_002 start";

    // Initialize ChannelManager
    InitChannelManager();

    // Setup: Add valid networkId to serverNetworkSocketMap_
    ChannelManager::GetInstance().serverNetworkSocketMap_["valid_network"] = 1;

    // Mock serialization failure
    g_resSerializeToJson = false;
    ControlCmd request;
    request.msgId = 2;
    request.msgType = ControlCmdType::CMD_CHECK_ALLOW_CONNECT;

    // Test: Call NotifyClient with serialization failure
    int32_t ret = ChannelManager::GetInstance().NotifyClient("valid_network", request);
    EXPECT_EQ(ret, ERR_DATA_INVALID);

    GTEST_LOG_(INFO) << "ChannelManagerTest_NotifyClient_002 end";
}

/**
 * @tc.name: ChannelManagerTest_NotifyClient_003
 * @tc.desc: Verify NotifyClient handles DoSendBytes failure
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ChannelManagerTest, ChannelManagerTest_NotifyClient_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChannelManagerTest_NotifyClient_003 start";

    // Initialize ChannelManager
    InitChannelManager();

    // Setup: Add valid networkId to serverNetworkSocketMap_
    ChannelManager::GetInstance().serverNetworkSocketMap_["valid_network"] = 1;

    // Mock successful serialization but failed send
    g_resSerializeToJson = true;
    EXPECT_CALL(*socketMock_, SendBytes(_, _, _)).WillOnce(Return(ERR_SEND_DATA_BY_SOFTBUS_FAILED));

    ControlCmd request;
    request.msgId = 3;
    request.msgType = ControlCmdType::CMD_CHECK_ALLOW_CONNECT;

    // Test: Call NotifyClient with DoSendBytes failure
    int32_t ret = ChannelManager::GetInstance().NotifyClient("valid_network", request);
    EXPECT_EQ(ret, ERR_SEND_DATA_BY_SOFTBUS_FAILED);

    GTEST_LOG_(INFO) << "ChannelManagerTest_NotifyClient_003 end";
}

/**
 * @tc.name: ChannelManagerTest_NotifyClient_004
 * @tc.desc: Verify successful NotifyClient execution
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ChannelManagerTest, ChannelManagerTest_NotifyClient_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChannelManagerTest_NotifyClient_004 start";

    // Initialize ChannelManager
    InitChannelManager();

    // Setup: Add valid networkId to serverNetworkSocketMap_
    ChannelManager::GetInstance().serverNetworkSocketMap_["valid_network"] = 1;

    // Mock successful serialization and send
    g_resSerializeToJson = true;
    EXPECT_CALL(*socketMock_, SendBytes(_, _, _)).WillOnce(Return(ERR_OK));

    ControlCmd request;
    request.msgId = 4;
    request.msgType = ControlCmdType::CMD_CHECK_ALLOW_CONNECT;

    // Test: Call NotifyClient with successful path
    int32_t ret = ChannelManager::GetInstance().NotifyClient("valid_network", request);
    EXPECT_EQ(ret, ERR_OK);

    // Verify serialization was called
    EXPECT_EQ(g_callSerializeToJsonTimes, 1);

    GTEST_LOG_(INFO) << "ChannelManagerTest_NotifyClient_004 end";
}

} // namespace Test
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS