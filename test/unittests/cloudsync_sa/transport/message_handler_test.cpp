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

#include <algorithm>
#include <cstring>
#include <securec.h>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "cloud_sync_common.h"
#include "dfs_error.h"
#include "message_handler.h"
#include "socket_mock.h"
#include "task_state_manager.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement::CloudSync {
namespace Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class MessageHandlerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void MessageHandlerTest::SetUpTestCase(void)
{
    std::cout << "SetUpTestCase" << std::endl;
}

void MessageHandlerTest::TearDownTestCase(void)
{
    TaskStateManager::GetInstance().CancelUnloadTask();
    std::cout << "TearDownTestCase" << std::endl;
}

void MessageHandlerTest::SetUp(void)
{
    std::cout << "SetUp" << std::endl;
}

void MessageHandlerTest::TearDown(void)
{
    std::cout << "TearDown" << std::endl;
}

vector<uint8_t> CreateTestData(const MessageInputInfo &inputInfo)
{
    vector<uint8_t> data;
    MessageHeader header;
    header.magic = 0xF8;
    header.version = 0x1;
    header.msgType = htole16(inputInfo.msgType);
    header.dataLen = htole32(0);
    header.errorCode = htole32(inputInfo.errorCode);

    uint8_t *headerPtr = reinterpret_cast<uint8_t *>(&header);
    data.insert(data.end(), headerPtr, headerPtr + sizeof(MessageHeader));

    SessionDeviceInfo deviceInfo;
    memset_s(&deviceInfo, sizeof(SessionDeviceInfo), 0, sizeof(SessionDeviceInfo));
    strcpy_s(deviceInfo.sourceNetworkId, NETWORK_ID_SIZE_MAX, inputInfo.srcNetworkId.c_str());
    strcpy_s(deviceInfo.tartgeNetworkId, NETWORK_ID_SIZE_MAX, inputInfo.dstNetworkId.c_str());
    uint8_t *devicePtr = reinterpret_cast<uint8_t *>(&deviceInfo);
    data.insert(data.end(), devicePtr, devicePtr + sizeof(SessionDeviceInfo));

    uint64_t leTaskId = htole64(inputInfo.taskId);
    uint32_t leUserId = htole32(inputInfo.userId);
    uint32_t leUriLen = htole32(static_cast<uint32_t>(inputInfo.uri.length()));

    uint8_t *taskIdPtr = reinterpret_cast<uint8_t *>(&leTaskId);
    data.insert(data.end(), taskIdPtr, taskIdPtr + sizeof(leTaskId));
    uint8_t *userIdPtr = reinterpret_cast<uint8_t *>(&leUserId);
    data.insert(data.end(), userIdPtr, userIdPtr + sizeof(leUserId));
    uint8_t *uriLenPtr = reinterpret_cast<uint8_t *>(&leUriLen);
    data.insert(data.end(), uriLenPtr, uriLenPtr + sizeof(leUriLen));

    data.insert(data.end(), inputInfo.uri.begin(), inputInfo.uri.end());
    return data;
}

/**
 * @tc.name: GetDataTest001
 * @tc.desc: Verify the MessageHandler function
 * @tc.type: FUNC
 * @tc.require: IB3T80
 */
HWTEST_F(MessageHandlerTest, GetDataTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetDataTest001 start";
    try {
        MessageInputInfo inputInfo;
        inputInfo.srcNetworkId = "srcNetworkId";
        inputInfo.dstNetworkId = "dstNetworkId";
        inputInfo.uri = "uritest";
        inputInfo.msgType = MSG_DOWNLOAD_FILE_REQ;
        inputInfo.errorCode = 1;
        inputInfo.userId = 100;
        inputInfo.taskId = 100;
        auto messageHandler = make_shared<MessageHandler>(inputInfo);
        EXPECT_TRUE(messageHandler);

        EXPECT_EQ(messageHandler->GetMsgType(), MSG_DOWNLOAD_FILE_REQ);
        EXPECT_EQ(messageHandler->GetSrcNetworkId(), "srcNetworkId");
        EXPECT_EQ(messageHandler->GetDstNetworkId(), "dstNetworkId");
        EXPECT_EQ(messageHandler->GetUri(), "uritest");
        EXPECT_EQ(messageHandler->GetErrorCode(), 1);
        EXPECT_EQ(messageHandler->GetUserId(), 100);
        EXPECT_EQ(messageHandler->GetTaskId(), 100);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetDataTest001 failed";
    }
    GTEST_LOG_(INFO) << "GetDataTest001 end";
}

/**
 * @tc.name: AddTransTaskTest001
 * @tc.desc: Verify the PackData function
 * @tc.type: FUNC
 * @tc.require: IB3T80
 */
HWTEST_F(MessageHandlerTest, PackDataTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PackDataTest001 start";
    try {
        MessageInputInfo inputInfo;
        inputInfo.srcNetworkId = "srcNetworkId";
        inputInfo.dstNetworkId = "dstNetworkId";
        inputInfo.uri = "uritest";
        inputInfo.msgType = MSG_DOWNLOAD_FILE_REQ;
        inputInfo.errorCode = 1;
        inputInfo.userId = 100;
        inputInfo.taskId = 100;
        auto messageHandler = make_shared<MessageHandler>(inputInfo);
        EXPECT_TRUE(messageHandler);
        cout << "messageHandler->GetDataSize():" << messageHandler->GetDataSize() << endl;
        auto dataSubPtr = std::make_unique<uint8_t[]>(messageHandler->GetDataSize());
        EXPECT_FALSE(messageHandler->PackData(nullptr, messageHandler->GetDataSize()));
        EXPECT_FALSE(messageHandler->PackData(dataSubPtr.get(), 0));
        EXPECT_FALSE(messageHandler->PackData(nullptr, 0));
        EXPECT_TRUE(messageHandler->PackData(dataSubPtr.get(), messageHandler->GetDataSize()));

        MessageHandler msgHandleSub;
        EXPECT_FALSE(msgHandleSub.UnPackData(nullptr, messageHandler->GetDataSize()));
        EXPECT_FALSE(msgHandleSub.UnPackData(dataSubPtr.get(), 0));
        EXPECT_FALSE(msgHandleSub.UnPackData(nullptr, 0));
        EXPECT_TRUE(msgHandleSub.UnPackData(dataSubPtr.get(), messageHandler->GetDataSize()));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PackDataTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "PackDataTest001 end";
}

/**
 * @tc.name: PackDataTest002
 * @tc.desc: Verify the MessageHandler function
 * @tc.type: FUNC
 * @tc.require: #NA
 */
HWTEST_F(MessageHandlerTest, PackDataTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PackDataTest002 start";
    try {
        MessageInputInfo inputInfo = {.srcNetworkId = "source-device-001",
                                      .dstNetworkId = "target-device-002",
                                      .uri = "/test/path",
                                      .msgType = 0x1001,
                                      .errorCode = 0,
                                      .userId = 1001,
                                      .taskId = 123456789ULL};
        auto messageHandler = make_shared<MessageHandler>(inputInfo);
        auto data = CreateTestData(inputInfo);
        bool result = messageHandler->UnPackData(data.data(), data.size());
        EXPECT_TRUE(result);
        EXPECT_EQ(messageHandler->GetSrcNetworkId(), inputInfo.srcNetworkId);
        EXPECT_EQ(messageHandler->GetDstNetworkId(), inputInfo.dstNetworkId);
        EXPECT_EQ(messageHandler->GetTaskId(), inputInfo.taskId);
        EXPECT_EQ(messageHandler->GetUserId(), inputInfo.userId);
        EXPECT_EQ(messageHandler->GetUri(), inputInfo.uri);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PackDataTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "PackDataTest002 end";
}

/**
 * @tc.name: PackDataTest003
 * @tc.desc: Verify the MessageHandler function
 * @tc.type: FUNC
 * @tc.require: #NA
 */
HWTEST_F(MessageHandlerTest, PackDataTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PackDataTest003 start";
    try {
        std::vector<uint8_t> shortData(sizeof(MessageHeader) - 1, 0x00);
        MessageInputInfo inputInfo;
        auto messageHandler = make_shared<MessageHandler>(inputInfo);
        bool result = messageHandler->UnPackData(shortData.data(), shortData.size());

        EXPECT_FALSE(result);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PackDataTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "PackDataTest003 end";
}

/**
 * @tc.name: PackDataTest004
 * @tc.desc: Verify the MessageHandler function
 * @tc.type: FUNC
 * @tc.require: #NA
 */
HWTEST_F(MessageHandlerTest, PackDataTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PackDataTest004 start";
    try {
        MessageInputInfo inputInfo = {.srcNetworkId = "source-device-001",
                                      .dstNetworkId = "target-device-002",
                                      .uri = "/test/path",
                                      .msgType = 0x1001,
                                      .errorCode = 0,
                                      .userId = 1001,
                                      .taskId = 123456789ULL};
        auto data = CreateTestData(inputInfo);
        vector<uint8_t> datSub(data.begin(), data.begin() + sizeof(MessageHeader) + sizeof(SessionDeviceInfo) - 1);
        auto messageHandler = make_shared<MessageHandler>(inputInfo);
        bool result = messageHandler->UnPackData(datSub.data(), datSub.size());

        EXPECT_FALSE(result);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PackDataTest004 FAILED";
    }
    GTEST_LOG_(INFO) << "PackDataTest004 end";
}

/**
 * @tc.name: PackDataTest005
 * @tc.desc: Verify the MessageHandler function
 * @tc.type: FUNC
 * @tc.require: #NA
 */
HWTEST_F(MessageHandlerTest, PackDataTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PackDataTest005 start";
    try {
        MessageInputInfo inputInfo = {.srcNetworkId = "source-device-001",
                                      .dstNetworkId = "target-device-002",
                                      .uri = "/test/path",
                                      .msgType = 0x1001,
                                      .errorCode = 0,
                                      .userId = 1001,
                                      .taskId = 123456789ULL};
        auto data = CreateTestData(inputInfo);
        vector<uint8_t> datSub(data.begin(), data.begin() + sizeof(MessageHeader) + sizeof(SessionDeviceInfo) + 1);
        auto messageHandler = make_shared<MessageHandler>(inputInfo);
        bool result = messageHandler->UnPackData(datSub.data(), datSub.size());

        EXPECT_FALSE(result);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PackDataTest005 FAILED";
    }
    GTEST_LOG_(INFO) << "PackDataTest005 end";
}

/**
 * @tc.name: PackDataTest006
 * @tc.desc: Verify the MessageHandler function
 * @tc.type: FUNC
 * @tc.require: #NA
 */
HWTEST_F(MessageHandlerTest, PackDataTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PackDataTest006 start";
    try {
        string tooLongUri(PATH_MAX + 10, 'a');
        MessageInputInfo inputInfo = {.srcNetworkId = "source-device-001",
                                      .dstNetworkId = "target-device-002",
                                      .uri = tooLongUri,
                                      .msgType = 0x1001,
                                      .errorCode = 0,
                                      .userId = 1001,
                                      .taskId = 123456789ULL};
        auto data = CreateTestData(inputInfo);
        auto messageHandler = make_shared<MessageHandler>(inputInfo);
        bool result = messageHandler->UnPackData(data.data(), data.size());

        EXPECT_FALSE(result);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PackDataTest006 FAILED";
    }
    GTEST_LOG_(INFO) << "PackDataTest006 end";
}

/**
 * @tc.name: PackDataTest007
 * @tc.desc: Verify the MessageHandler function
 * @tc.type: FUNC
 * @tc.require: #NA
 */
HWTEST_F(MessageHandlerTest, PackDataTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PackDataTest007 start";
    try {
        string uri(10, 'a');
        MessageInputInfo inputInfo = {.srcNetworkId = "source-device-001",
                                      .dstNetworkId = "target-device-002",
                                      .uri = uri,
                                      .msgType = 0x1001,
                                      .errorCode = 0,
                                      .userId = 1001,
                                      .taskId = 123456789ULL};
        auto data = CreateTestData(inputInfo);
        uint8_t *ptr = data.data();
        ptr += sizeof(MessageHeader) + sizeof(SessionDeviceInfo);
        UserData *userData = reinterpret_cast<UserData *>(ptr);
        userData->uriLen += 1;
        auto messageHandler = make_shared<MessageHandler>(inputInfo);
        bool result = messageHandler->UnPackData(data.data(), data.size());

        EXPECT_FALSE(result);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PackDataTest007 FAILED";
    }
    GTEST_LOG_(INFO) << "PackDataTest007 end";
}
} // namespace Test
} // namespace FileManagement::CloudSync
} // namespace OHOS