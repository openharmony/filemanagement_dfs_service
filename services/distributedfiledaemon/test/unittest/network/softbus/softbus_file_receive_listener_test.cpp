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
#include "network/softbus/softbus_file_receive_listener.h"

#include <memory>
#include <unistd.h>

#include "gtest/gtest.h"
#include "utils_log.h"

#include "network/softbus/softbus_handler.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace Test {
using namespace testing::ext;
using namespace std;
const string testPath = "/data/test";

class SoftBusFileReceiveListenerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void SoftBusFileReceiveListenerTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void SoftBusFileReceiveListenerTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void SoftBusFileReceiveListenerTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
    SoftBusFileReceiveListener::SetRecvPath(testPath);
}

void SoftBusFileReceiveListenerTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
    SoftBusFileReceiveListener::path_.clear();
}
/**
 * @tc.name: SoftBusFileReceiveListenerTest_SetRecvPath_0100
 * @tc.desc: test SetRecvPath function.
 * @tc.type: FUNC
 * @tc.require: I9JXPR
 */
HWTEST_F(SoftBusFileReceiveListenerTest, SoftBusFileReceiveListenerTest_SetRecvPath_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusSessionPoolTest_SessionInfo_0100 start";
    string path = string(SoftBusFileReceiveListener::GetRecvPath());
    EXPECT_EQ(path, testPath);
    SoftBusFileReceiveListener::SetRecvPath("");
    path.clear();
    path = string(SoftBusFileReceiveListener::GetRecvPath());
    EXPECT_EQ(path, testPath);
    SoftBusFileReceiveListener::SetRecvPath("/data/test2");
    path.clear();
    path = string(SoftBusFileReceiveListener::GetRecvPath());
    EXPECT_EQ(path, testPath);
    GTEST_LOG_(INFO) << "SoftbusSessionPoolTest_SessionInfo_0100 end";
}

/**
 * @tc.name: SoftBusFileReceiveListenerTest_GetLocalSessionName_0100
 * @tc.desc: test SetRecvPath function.
 * @tc.type: FUNC
 * @tc.require: I9JXPR
 */
HWTEST_F(SoftBusFileReceiveListenerTest, SoftBusFileReceiveListenerTest_GetLocalSessionName_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftBusFileReceiveListenerTest_GetLocalSessionName_0100 start";
    string testSessionName = "mySessionName";
    int32_t sessionId = 1;
    SoftBusHandler::clientSessNameMap_.insert(std::make_pair(sessionId, testSessionName));
    string sessionName = SoftBusFileReceiveListener::GetLocalSessionName(sessionId);
    EXPECT_EQ(sessionName, testSessionName);
    SoftBusHandler::clientSessNameMap_.erase(sessionId);
    sessionName = SoftBusFileReceiveListener::GetLocalSessionName(sessionId);
    EXPECT_EQ(sessionName, "");
    GTEST_LOG_(INFO) << "SoftBusFileReceiveListenerTest_GetLocalSessionName_0100 end";
}

/**
 * @tc.name: SoftBusFileReceiveListenerTest_OnFile_0100
 * @tc.desc: test SetRecvPath function.
 * @tc.type: FUNC
 * @tc.require: I9JXPR
 */
HWTEST_F(SoftBusFileReceiveListenerTest, SoftBusFileReceiveListenerTest_OnFile_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftBusFileReceiveListenerTest_OnFile_0100 start";
    try {
        FileEvent event = {.type = FILE_EVENT_RECV_START,
                           .fileCnt = 1,
                           .bytesProcessed = 0,
                           .bytesTotal = 0};
        int32_t socket = 0;
        SoftBusFileReceiveListener::OnFile(socket, nullptr);
        SoftBusFileReceiveListener::OnFile(socket, &event);

        SoftBusHandler::clientSessNameMap_.insert(std::make_pair(socket, "mySessionName"));
        event.type = FILE_EVENT_RECV_PROCESS;

        SoftBusFileReceiveListener::OnFile(socket, &event);
        int32_t socket2 = 1;
        SoftBusFileReceiveListener::OnFile(socket2, &event);

        event.type = FILE_EVENT_RECV_FINISH;
        SoftBusFileReceiveListener::OnFile(socket, &event);
        SoftBusFileReceiveListener::OnFile(socket2, &event);

        event.type = FILE_EVENT_RECV_ERROR;
        SoftBusFileReceiveListener::OnFile(socket, &event);
        SoftBusFileReceiveListener::OnFile(socket2, &event);

        event.type = FILE_EVENT_RECV_UPDATE_PATH;
        SoftBusFileReceiveListener::OnFile(socket, &event);
        EXPECT_EQ(string((event.UpdateRecvPath)()), testPath);

        event.type = FILE_EVENT_BUTT;
        SoftBusFileReceiveListener::OnFile(socket, &event);
        EXPECT_TRUE(true);
        SoftBusHandler::clientSessNameMap_.erase(socket);
    } catch (...) {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "SoftBusFileReceiveListenerTest_OnFile_0100 end";
}
} // namespace Test
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
