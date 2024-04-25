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

#include <memory>
#include <unistd.h>

#include "gtest/gtest.h"
#include "network/softbus/softbus_session_pool.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace Test {
using namespace testing::ext;
using namespace std;

constexpr int SESSION_ID_ONE = 1;
constexpr int SESSION_ID_TWO = 2;
constexpr int UID_ONE = 1;
constexpr int UID_TWO = 2;
constexpr int32_t SESSION_COUNT = 10;
const string SESSION_NAME_PREFIX = "DistributedFileService";

class SoftbusSessionPoolTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};
};
/**
 * @tc.name: SoftbusSessionPoolTest_SessionInfo_0100
 * @tc.desc: test SoftBusSessionPool function.
 * @tc.type: FUNC
 * @tc.require: I9JKYU
 */
HWTEST_F(SoftbusSessionPoolTest, SoftbusSessionPoolTest_SessionInfo_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusSessionPoolTest_SessionInfo_0100 start";
    string sessionName1 = "test1";
    string sessionName2 = "DistributedFileService0";
    SoftBusSessionPool::SessionInfo sessionInfo1{.sessionId = SESSION_ID_ONE,
                                                 .srcUri = "file://com.demo.a/test/1",
                                                 .dstPath = "/data/test/1",
                                                 .uid = UID_ONE};
    SoftBusSessionPool::SessionInfo sessionInfo2{.sessionId = SESSION_ID_TWO,
                                                 .srcUri = "file://com.demo.a/test/2",
                                                 .dstPath = "/data/test/2",
                                                 .uid = UID_TWO};
    SoftBusSessionPool::GetInstance().AddSessionInfo(sessionName1, sessionInfo1);
    string sessionName = SoftBusSessionPool::GetInstance().GenerateSessionName();
    EXPECT_EQ(sessionName, sessionName2);
    SoftBusSessionPool::GetInstance().AddSessionInfo(sessionName2, sessionInfo2);
    sessionName = SoftBusSessionPool::GetInstance().GenerateSessionName();
    EXPECT_EQ(sessionName, "DistributedFileService1");
    SoftBusSessionPool::SessionInfo sessionInfo;
    bool flag = SoftBusSessionPool::GetInstance().GetSessionInfo(sessionName1, sessionInfo);
    EXPECT_EQ(flag, true);
    EXPECT_EQ(sessionInfo.sessionId, sessionInfo1.sessionId);
    EXPECT_EQ(sessionInfo.dstPath, sessionInfo1.dstPath);
    EXPECT_EQ(sessionInfo.srcUri, sessionInfo1.srcUri);
    EXPECT_EQ(sessionInfo.uid, sessionInfo1.uid);
    flag = SoftBusSessionPool::GetInstance().GetSessionInfo(sessionName2, sessionInfo);
    EXPECT_EQ(flag, true);
    EXPECT_EQ(sessionInfo.sessionId, sessionInfo2.sessionId);
    EXPECT_EQ(sessionInfo.dstPath, sessionInfo2.dstPath);
    EXPECT_EQ(sessionInfo.srcUri, sessionInfo2.srcUri);
    EXPECT_EQ(sessionInfo.uid, sessionInfo2.uid);
    SoftBusSessionPool::GetInstance().DeleteSessionInfo(sessionName1);
    flag = SoftBusSessionPool::GetInstance().GetSessionInfo(sessionName1, sessionInfo);
    EXPECT_EQ(flag, false);
    SoftBusSessionPool::GetInstance().DeleteSessionInfo(sessionName2);
    flag = SoftBusSessionPool::GetInstance().GetSessionInfo(sessionName2, sessionInfo);
    EXPECT_EQ(flag, false);
    GTEST_LOG_(INFO) << "SoftbusSessionPoolTest_SessionInfo_0100 end";
}

/**
 * @tc.name: SoftbusSessionPoolTest_GenerateSessionName_0100
 * @tc.desc: test GenerateSessionName function.
 * @tc.type: FUNC
 * @tc.require: I9JKYU
 */
HWTEST_F(SoftbusSessionPoolTest, SoftbusSessionPoolTest_GenerateSessionName_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusSessionPoolTest_GenerateSessionName_0100 start";
    string sessionName = SoftBusSessionPool::GetInstance().GenerateSessionName();
    EXPECT_EQ(sessionName, "DistributedFileService0");
    SoftBusSessionPool::SessionInfo sessionInfo1{.sessionId = SESSION_ID_ONE,
                                                 .srcUri = "file://com.demo.a/test/1",
                                                 .dstPath = "/data/test/1",
                                                 .uid = UID_ONE};
    for (int32_t i = 0; i < SESSION_COUNT - 1; i++) {
        std::string testName = std::string(SESSION_NAME_PREFIX) + std::to_string(i);
        SoftBusSessionPool::GetInstance().AddSessionInfo(testName, sessionInfo1);
    }
    
    sessionName = SoftBusSessionPool::GetInstance().GenerateSessionName();
    EXPECT_EQ(sessionName, "DistributedFileService9");
    SoftBusSessionPool::GetInstance().AddSessionInfo(sessionName, sessionInfo1);
    sessionName = SoftBusSessionPool::GetInstance().GenerateSessionName();
    EXPECT_EQ(sessionName, "");

    for (int32_t i = 0; i < SESSION_COUNT; i++) {
        std::string testName = std::string(SESSION_NAME_PREFIX) + std::to_string(i);
        SoftBusSessionPool::GetInstance().DeleteSessionInfo(testName);
    }

    sessionName = SoftBusSessionPool::GetInstance().GenerateSessionName();
    EXPECT_EQ(sessionName, "DistributedFileService0");
    GTEST_LOG_(INFO) << "SoftbusSessionPoolTest_GenerateSessionName_0100 end";
}
} // namespace Test
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
