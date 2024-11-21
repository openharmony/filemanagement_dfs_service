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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "cloud_sync_common.h"
#include "dfs_error.h"
#include "message_handler.h"
#include "socket_mock.h"
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
        EXPECT_TRUE(messageHandler->PackData(dataSubPtr.get(), messageHandler->GetDataSize()));

        MessageHandler msgHandleSub;
        EXPECT_TRUE(msgHandleSub.UnPackData(dataSubPtr.get(), messageHandler->GetDataSize()));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PackDataTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "PackDataTest001 end";
}
} // namespace Test
} // namespace CloudSync
} // namespace OHOS