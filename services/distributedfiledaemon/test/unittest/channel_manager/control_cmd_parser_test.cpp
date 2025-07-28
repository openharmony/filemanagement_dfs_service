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

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "system_notifier.h"
#include "control_cmd_parser.h"
#include "dfs_error.h"
#include "device_manager.h"
#include "utils_log.h"
#include "parameter.h"
#include <nlohmann/json.hpp>

namespace {
    int32_t g_publishNotification = 0;
    int32_t g_cancelNotification = 0;
}  // namespace

namespace OHOS {
namespace Storage {
namespace DistributedFile {

int32_t SystemNotifier::CreateLocalLiveView(const std::string &networkId)
{
    return g_publishNotification;
}

int32_t SystemNotifier::DestroyNotifyByNetworkId(const std::string &networkId)
{
    return g_cancelNotification;
}

namespace Test {
using namespace testing;
using namespace testing::ext;
using OHOS::FileManagement::ERR_BAD_VALUE;
using OHOS::FileManagement::ERR_OK;

class ControlCmdParserTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        GTEST_LOG_(INFO) << "ControlCmdParserTest SetUpTestCase";
    }

    static void TearDownTestCase()
    {
        GTEST_LOG_(INFO) << "ControlCmdParserTest TearDownTestCase";
    }

    void SetUp()
    {
        GTEST_LOG_(INFO) << "ControlCmdParserTest SetUp";
        g_publishNotification = 0;
        g_cancelNotification = 0;
    }

    void TearDown(void)
    {
        GTEST_LOG_(INFO) << "ControlCmdParserTest TearDown";
    }
};
/**
 * @tc.name: ControlCmdParserTest_ParseValidJson_001
 * @tc.desc: Verify ParseFromJson with valid complete input
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ControlCmdParserTest, ParseValidJson_001, TestSize.Level1)
{
    std::string validJson = R"({
        "version": 1,
        "msgId": 1001,
        "msgType": 1,
        "msgBody": "testbody",
        "networkId": "test-network"
    })";

    ControlCmd cmd;
    EXPECT_TRUE(ControlCmdParser::ParseFromJson(validJson, cmd));
    EXPECT_EQ(cmd.version, 1);
    EXPECT_EQ(cmd.msgId, 1001);
    EXPECT_EQ(cmd.msgType, 1);
    EXPECT_EQ(cmd.msgBody, "testbody");
    EXPECT_EQ(cmd.networkId, "test-network");
}

/**
 * @tc.name: ControlCmdParserTest_ParseEmptyString_002
 * @tc.desc: Verify ParseFromJson with empty input string
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ControlCmdParserTest, ParseEmptyString_002, TestSize.Level1)
{
    ControlCmd cmd;
    EXPECT_FALSE(ControlCmdParser::ParseFromJson("", cmd));
}

/**
 * @tc.name: ControlCmdParserTest_ParseInvalidSyntax_003
 * @tc.desc: Verify ParseFromJson with invalid JSON syntax
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ControlCmdParserTest, ParseInvalidSyntax_003, TestSize.Level1)
{
    std::string invalidJson = R"({
        "version": 1,
        "msgId": 1001,
        "msgType": 1,
        "msgBody": "testbody",
        "networkId": "test-network",
    })";

    ControlCmd cmd;
    EXPECT_FALSE(ControlCmdParser::ParseFromJson(invalidJson, cmd));
}

/**
 * @tc.name: ControlCmdParserTest_MissingMsgId_004
 * @tc.desc: Verify ParseFromJson with missing msgId field
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ControlCmdParserTest, MissingMsgId_004, TestSize.Level1)
{
    std::string missingMsgId = R"({
        "version": 1,
        "msgType": 1,
        "msgBody": "testbody",
        "networkId": "test-network"
    })";

    ControlCmd cmd;
    EXPECT_FALSE(ControlCmdParser::ParseFromJson(missingMsgId, cmd));
}

/**
 * @tc.name: ControlCmdParserTest_MissingMsgType_005
 * @tc.desc: Verify ParseFromJson with missing msgType field
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ControlCmdParserTest, MissingMsgType_005, TestSize.Level1)
{
    std::string missingMsgType = R"({
        "version": 1,
        "msgId": 1001,
        "msgBody": "testbody",
        "networkId": "test-network"
    })";

    ControlCmd cmd;
    EXPECT_FALSE(ControlCmdParser::ParseFromJson(missingMsgType, cmd));
}

/**
 * @tc.name: ControlCmdParserTest_MsgIdTypeMismatch_006
 * @tc.desc: Verify ParseFromJson with msgId type mismatch
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ControlCmdParserTest, MsgIdTypeMismatch_006, TestSize.Level1)
{
    std::string wrongTypeMsgId = R"({
        "version": 1,
        "msgId": "1001",
        "msgType": 1,
        "msgBody": "testbody",
        "networkId": "test-network"
    })";

    ControlCmd cmd;
    EXPECT_FALSE(ControlCmdParser::ParseFromJson(wrongTypeMsgId, cmd));
}

/**
 * @tc.name: ControlCmdParserTest_MsgTypeTypeMismatch_007
 * @tc.desc: Verify ParseFromJson with msgType type mismatch
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ControlCmdParserTest, MsgTypeTypeMismatch_007, TestSize.Level1)
{
    std::string wrongTypeMsgType = R"({
        "version": 1,
        "msgId": 1001,
        "msgType": true,
        "msgBody": "testbody",
        "networkId": "test-network"
    })";

    ControlCmd cmd;
    EXPECT_FALSE(ControlCmdParser::ParseFromJson(wrongTypeMsgType, cmd));
}

/**
 * @tc.name: ControlCmdParserTest_VersionOutOfRange_008
 * @tc.desc: Verify ParseFromJson with version out of range
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ControlCmdParserTest, VersionOutOfRange_008, TestSize.Level1)
{
    std::string versionOutOfRange = R"({
        "version": 70000,
        "msgId": 1001,
        "msgType": 1,
        "msgBody": "testbody",
        "networkId": "test-network"
    })";

    ControlCmd cmd;
    EXPECT_TRUE(ControlCmdParser::ParseFromJson(versionOutOfRange, cmd));
}

/**
 * @tc.name: ControlCmdParserTest_MinimalValidInput_009
 * @tc.desc: Verify ParseFromJson with minimal valid input
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ControlCmdParserTest, MinimalValidInput_009, TestSize.Level1)
{
    std::string minimalValid = R"({
        "msgId": 1001,
        "msgType": 1
    })";

    ControlCmd cmd;
    EXPECT_TRUE(ControlCmdParser::ParseFromJson(minimalValid, cmd));
    EXPECT_EQ(cmd.msgId, 1001);
    EXPECT_EQ(cmd.msgType, 1);
}

/**
 * @tc.name: ControlCmdParserTest_MsgBodyTypeMismatch_010
 * @tc.desc: Verify ParseFromJson with msgBody type mismatch
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ControlCmdParserTest, MsgBodyTypeMismatch_010, TestSize.Level1)
{
    std::string wrongTypeMsgBody = R"({
        "msgId": 1001,
        "msgType": 1,
        "msgBody": 12345,
        "networkId": "test-network"
    })";

    ControlCmd cmd;
    EXPECT_TRUE(ControlCmdParser::ParseFromJson(wrongTypeMsgBody, cmd));
}

/**
 * @tc.name: ControlCmdParserTest_NegativeVersion_011
 * @tc.desc: Verify ParseFromJson with negative version number
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ControlCmdParserTest, NegativeVersion_011, TestSize.Level1)
{
    std::string negativeVersion = R"({
        "version": -1,
        "msgId": 1001,
        "msgType": 1,
        "msgBody": "testbody",
        "networkId": "test-network"
    })";

    ControlCmd cmd;
    EXPECT_TRUE(ControlCmdParser::ParseFromJson(negativeVersion, cmd));
}

/**
 * @tc.name: SerializeToJson_ValidInput
 * @tc.desc: Test SerializeToJson with valid ControlCmd
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ControlCmdParserTest, SerializeToJson_ValidInput, TestSize.Level1)
{
    ControlCmd cmd;
    cmd.version = 1;
    cmd.msgId = 1001;
    cmd.msgType = 1;
    cmd.msgBody = "test body";
    cmd.networkId = "test-network";

    std::string jsonStr;
    EXPECT_TRUE(ControlCmdParser::SerializeToJson(cmd, jsonStr));
    EXPECT_FALSE(jsonStr.empty());

    // Verify the serialized JSON can be parsed back
    ControlCmd parsedCmd;
    EXPECT_TRUE(ControlCmdParser::ParseFromJson(jsonStr, parsedCmd));
    EXPECT_EQ(parsedCmd.version, cmd.version);
    EXPECT_EQ(parsedCmd.msgId, cmd.msgId);
    EXPECT_EQ(parsedCmd.msgType, cmd.msgType);
    EXPECT_EQ(parsedCmd.msgBody, cmd.msgBody);
    EXPECT_EQ(parsedCmd.networkId, cmd.networkId);
}

/**
 * @tc.name: HandleRequest_CheckAllowConnect_Success
 * @tc.desc: Test HandleRequest with CMD_CHECK_ALLOW_CONNECT when device type matches
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ControlCmdParserTest, HandleRequest_CheckAllowConnect_Success, TestSize.Level1)
{
    ControlCmd inCmd;
    ControlCmd outCmd;
    inCmd.msgType = ControlCmdType::CMD_CHECK_ALLOW_CONNECT;
    inCmd.msgId = 1001;

    EXPECT_TRUE(ControlCmdParser::HandleRequest(inCmd, outCmd));
    EXPECT_EQ(outCmd.msgType, ControlCmdType::CMD_MSG_RESPONSE);
    EXPECT_EQ(outCmd.msgId, inCmd.msgId);
    EXPECT_EQ(outCmd.msgBody, "false");
}

/**
 * @tc.name: HandleRequest_PublishNotification_Success
 * @tc.desc: Test HandleRequest with CMD_PUBLISH_NOTIFICATION when successful
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ControlCmdParserTest, HandleRequest_PublishNotification, TestSize.Level1)
{
    ControlCmd inCmd;
    ControlCmd outCmd;
    inCmd.msgType = ControlCmdType::CMD_PUBLISH_NOTIFICATION;
    inCmd.networkId = "test-network";

    EXPECT_TRUE(ControlCmdParser::HandleRequest(inCmd, outCmd));

    g_publishNotification = 1;
    EXPECT_FALSE(ControlCmdParser::HandleRequest(inCmd, outCmd));
}

/**
 * @tc.name: HandleRequest_CancelNotification_Success
 * @tc.desc: Test HandleRequest with CMD_CANCEL_NOTIFICATION when successful
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ControlCmdParserTest, HandleRequest_CancelNotification_Success, TestSize.Level1)
{
    ControlCmd inCmd;
    ControlCmd outCmd;
    inCmd.msgType = ControlCmdType::CMD_CANCEL_NOTIFICATION;
    inCmd.networkId = "test-network";

    EXPECT_TRUE(ControlCmdParser::HandleRequest(inCmd, outCmd));

    g_cancelNotification = 1;
    EXPECT_FALSE(ControlCmdParser::HandleRequest(inCmd, outCmd));
}

/**
 * @tc.name: HandleRequest_DisconnectByRemote_NoCallback
 * @tc.desc: Test HandleRequest with CMD_ACTIVE_DISCONNECT when no callback is registered
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ControlCmdParserTest, HandleRequest_DisconnectByRemote_NoCallback, TestSize.Level1)
{
    ControlCmd inCmd;
    inCmd.msgType = ControlCmdType::CMD_ACTIVE_DISCONNECT;

    ControlCmd outCmd;
    EXPECT_FALSE(ControlCmdParser::HandleRequest(inCmd, outCmd));
}

/**
 * @tc.name: HandleRequest_DisconnectByRemote_WithCallback
 * @tc.desc: Test HandleRequest with CMD_ACTIVE_DISCONNECT when callback is registered
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ControlCmdParserTest, HandleRequest_DisconnectByRemote_WithCallback, TestSize.Level1)
{
    ControlCmd inCmd;
    inCmd.msgType = ControlCmdType::CMD_ACTIVE_DISCONNECT;
    inCmd.msgBody = "disconnect-msg";

    ControlCmd outCmd;

    bool callbackCalled = false;
    auto callback = [&callbackCalled](std::string msg) {
        callbackCalled = true;
        EXPECT_EQ(msg, "disconnect-msg");
    };

    ControlCmdParser::RegisterDisconnectCallback(callback);
    EXPECT_TRUE(ControlCmdParser::HandleRequest(inCmd, outCmd));
    EXPECT_TRUE(callbackCalled);
}

/**
 * @tc.name: HandleRequest_UnknownType
 * @tc.desc: Test HandleRequest with unknown message type
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ControlCmdParserTest, HandleRequest_UnknownType, TestSize.Level1)
{
    ControlCmd inCmd;
    inCmd.msgType = 999;  // Unknown type

    ControlCmd outCmd;
    EXPECT_FALSE(ControlCmdParser::HandleRequest(inCmd, outCmd));
}

}  // namespace Test
}  // namespace DistributedFile
}  // namespace Storage
}  // namespace OHOS