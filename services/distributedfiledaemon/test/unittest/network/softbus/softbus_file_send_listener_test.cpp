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
#include "network/softbus/softbus_file_send_listener.h"

#include <memory>
#include <unistd.h>

#include "gtest/gtest.h"
#include "utils_log.h"

#include "network/softbus/softbus_handler.h"
#include "network/softbus/softbus_session_pool.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace Test {
using namespace testing::ext;
using namespace std;

constexpr int32_t socket = 0;
constexpr int32_t socketEmpty = 1;
constexpr int SESSION_ID_ONE = 1;
constexpr int UID_ONE = 1;
const string sessionName = "mySessionName";

class SoftBusFileSendListenerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void SoftBusFileSendListenerTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void SoftBusFileSendListenerTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void SoftBusFileSendListenerTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
    SoftBusHandler::clientSessNameMap_.insert(std::make_pair(socket, sessionName));
}

void SoftBusFileSendListenerTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
    SoftBusHandler::clientSessNameMap_.erase(socket);
}

/**
 * @tc.name: SoftBusFileSendListenerTest_OnSendFileProcess_0100
 * @tc.desc: test OnSendFileProcess function.
 * @tc.type: FUNC
 * @tc.require: I9JXPR
 */
HWTEST_F(SoftBusFileSendListenerTest, SoftBusFileSendListenerTest_OnSendFileProcess_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftBusFileSendListenerTest_OnSendFileProcess_0100 start";
    FileEvent event = {.type = FILE_EVENT_SEND_PROCESS,
                       .fileCnt = 1,
                       .bytesProcessed = 0,
                       .bytesTotal = 1};

    SoftBusSessionPool::SessionInfo sessionInfo1{.sessionId = SESSION_ID_ONE,
                                                 .srcUri = "file://com.demo.a/test/1",
                                                 .dstPath = "/data/test/1",
                                                 .uid = UID_ONE};
    SoftBusSessionPool::GetInstance().AddSessionInfo(sessionName, sessionInfo1);

    SoftBusFileSendListener::OnFile(socketEmpty, &event);
    SoftBusFileSendListener::OnFile(socket, &event);
    SoftBusSessionPool::SessionInfo sessionInfo;
    bool flag = SoftBusSessionPool::GetInstance().GetSessionInfo(sessionName, sessionInfo);
    EXPECT_EQ(flag, true);

    event.bytesTotal = 0;
    SoftBusFileSendListener::OnFile(socket, &event);
    flag = SoftBusSessionPool::GetInstance().GetSessionInfo(sessionName, sessionInfo);
    EXPECT_EQ(flag, false);

    GTEST_LOG_(INFO) << "SoftBusFileSendListenerTest_OnSendFileProcess_0100 end";
}

/**
 * @tc.name: SoftBusFileSendListenerTest_OnSendFileFinished_0100
 * @tc.desc: test OnSendFileFinished function.
 * @tc.type: FUNC
 * @tc.require: I9JXPR
 */
HWTEST_F(SoftBusFileSendListenerTest, SoftBusFileSendListenerTest_OnSendFileFinished_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftBusFileSendListenerTest_OnSendFileFinished_0100 start";
    FileEvent event = {.type = FILE_EVENT_SEND_FINISH,
                       .fileCnt = 1,
                       .bytesProcessed = 0,
                       .bytesTotal = 1};

    SoftBusHandler::GetInstance().serverIdMap_.insert(std::make_pair(sessionName, socket));

    SoftBusFileSendListener::OnFile(socketEmpty, &event);
    SoftBusFileSendListener::OnFile(socket, &event);
    auto it = SoftBusHandler::GetInstance().serverIdMap_.find(sessionName);
    if (it == SoftBusHandler::GetInstance().serverIdMap_.end()) {
        EXPECT_TRUE(true);
    } else {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "SoftBusFileSendListenerTest_OnSendFileFinished_0100 end";
}

/**
 * @tc.name: SoftBusFileSendListenerTest_OnFileTransError_0100
 * @tc.desc: test OnFileTransError function.
 * @tc.type: FUNC
 * @tc.require: I9JXPR
 */
HWTEST_F(SoftBusFileSendListenerTest, SoftBusFileSendListenerTest_OnFileTransError_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftBusFileSendListenerTest_OnFileTransError_0100 start";
    FileEvent event = {.type = FILE_EVENT_SEND_ERROR,
                       .fileCnt = 1,
                       .bytesProcessed = 0,
                       .bytesTotal = 1};

    SoftBusHandler::GetInstance().serverIdMap_.insert(std::make_pair(sessionName, socket));

    SoftBusFileSendListener::OnFile(socketEmpty, &event);
    SoftBusFileSendListener::OnFile(socket, &event);
    auto it = SoftBusHandler::GetInstance().serverIdMap_.find(sessionName);
    if (it == SoftBusHandler::GetInstance().serverIdMap_.end()) {
        EXPECT_TRUE(true);
    } else {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "SoftBusFileSendListenerTest_OnFileTransError_0100 end";
}

/**
 * @tc.name: SoftBusFileSendListenerTest_OnFile_0100
 * @tc.desc: test OnFile function.
 * @tc.type: FUNC
 * @tc.require: I9JXPR
 */
HWTEST_F(SoftBusFileSendListenerTest, SoftBusFileSendListenerTest_OnFile_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftBusFileSendListenerTest_OnFile_0100 start";
    FileEvent event = {.type = FILE_EVENT_BUTT,
                       .fileCnt = 1,
                       .bytesProcessed = 0,
                       .bytesTotal = 1};

    try {
        SoftBusFileSendListener::OnFile(socketEmpty, nullptr);
        SoftBusFileSendListener::OnFile(socketEmpty, &event);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "SoftBusFileSendListenerTest_OnFile_0100 end";
}
}
}
}
}