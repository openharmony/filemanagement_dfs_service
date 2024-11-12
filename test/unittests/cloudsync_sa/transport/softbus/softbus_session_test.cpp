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
#include "softbus_adapter.h"
#include "softbus_session.h"
#include "socket_mock.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement::CloudSync {
namespace Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class SoftbusSessionTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<Storage::DistributedFile::SocketMock> socketMock_ = nullptr;
};

void SoftbusSessionTest::SetUpTestCase(void)
{
    std::cout << "SetUpTestCase" << std::endl;
    socketMock_ = make_shared<Storage::DistributedFile::SocketMock>();
    Storage::DistributedFile::SocketMock::dfsSocket = socketMock_;
}

void SoftbusSessionTest::TearDownTestCase(void)
{
    std::cout << "TearDownTestCase" << std::endl;
    Storage::DistributedFile::SocketMock::dfsSocket = nullptr;
    socketMock_ = nullptr;
}

void SoftbusSessionTest::SetUp(void)
{
    std::cout << "SetUp" << std::endl;
}

void SoftbusSessionTest::TearDown(void)
{
    std::cout << "TearDown" << std::endl;
}

/**
 * @tc.name: StartTest001
 * @tc.desc: Verify the Start function
 * @tc.type: FUNC
 * @tc.require: IB3T9R
 */
HWTEST_F(SoftbusSessionTest, StartTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartTest001 start";
    try {
        string peerNetworkId = "test peerNetworkId";
        auto type = SoftbusSession::TYPE_BYTES;
        auto softbusSession = make_shared<SoftbusSession>(peerNetworkId, "test session", type);
        EXPECT_TRUE(softbusSession != nullptr);
        EXPECT_CALL(*socketMock_, Socket(_)).WillOnce(Return(-1));
        int32_t result = softbusSession->Start();
        EXPECT_NE(result, 0);
        result = softbusSession->Stop();
        EXPECT_EQ(result, 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StartTest001 failed";
    }
    GTEST_LOG_(INFO) << "StartTest001 end";
}

/**
 * @tc.name: SendDataTest001
 * @tc.desc: Verify the SendData function
 * @tc.type: FUNC
 * @tc.require: IB3T9R
 */
HWTEST_F(SoftbusSessionTest, SendDataTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SendDataTest001 start";
    try {
        char data[] = "test data";
        string peerNetworkId = "test peerNetworkId";
        auto type = SoftbusSession::TYPE_BYTES;
        auto softbusSession = make_shared<SoftbusSession>(peerNetworkId, "test session", type);
        EXPECT_TRUE(softbusSession != nullptr);
        int32_t result = softbusSession->SendData(data, sizeof(data));
        EXPECT_NE(result, 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SendDataTest001 failed";
    }
    GTEST_LOG_(INFO) << "SendDataTest001 end";
}

/**
 * @tc.name: SendFileTest001
 * @tc.desc: Verify the SendFile function
 * @tc.type: FUNC
 * @tc.require: IB3T9R
 */
HWTEST_F(SoftbusSessionTest, SendFileTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SendFileTest001 start";
    try {
        char data[] = "test data";
        string peerNetworkId = "test peerNetworkId";
        auto type = SoftbusSession::TYPE_BYTES;
        auto softbusSession = make_shared<SoftbusSession>(peerNetworkId, "test session", type);
        EXPECT_TRUE(softbusSession != nullptr);
        int32_t result = softbusSession->SendFile({"data/test"}, {"data/test"});
        EXPECT_EQ(result, 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SendFileTest001 failed";
    }
    GTEST_LOG_(INFO) << "SendFileTest001 end";
}

/**
 * @tc.name: ToBeAnonymousTest001
 * @tc.desc: Verify the ToBeAnonymous function
 * @tc.type: FUNC
 * @tc.require: IB3T9R
 */
HWTEST_F(SoftbusSessionTest, ToBeAnonymousTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToBeAnonymousTest001 start";
    try {
        char data[] = "test data";
        string peerNetworkId = "test peerNetworkId";
        auto type = SoftbusSession::TYPE_BYTES;
        auto softbusSession = make_shared<SoftbusSession>(peerNetworkId, "test session", type);
        EXPECT_TRUE(softbusSession != nullptr);
        SoftbusSession::DataType dataType = softbusSession->GetDataType();
        EXPECT_TRUE(dataType == SoftbusSession::TYPE_BYTES);
        string deviceId = softbusSession->GetPeerDeviceId();
        EXPECT_TRUE(deviceId == "test peerNetworkId");
        EXPECT_EQ(softbusSession->ToBeAnonymous(deviceId), "tes***kId");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ToBeAnonymousTest001 failed";
    }
    GTEST_LOG_(INFO) << "ToBeAnonymousTest001 end";
}

/**
 * @tc.name: WaitSessionOpenedTest001
 * @tc.desc: Verify the WaitSessionOpened function
 * @tc.type: FUNC
 * @tc.require: IB3T9R
 */
HWTEST_F(SoftbusSessionTest, WaitSessionOpenedTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WaitSessionOpenedTest001 start";
    try {
        char data[] = "test data";
        string peerNetworkId = "test peerNetworkId";
        auto type = SoftbusSession::TYPE_BYTES;
        auto softbusSession = make_shared<SoftbusSession>(peerNetworkId, "test session", type);
        EXPECT_TRUE(softbusSession != nullptr);
        int32_t sessionId = softbusSession->GetSessionId();
        SoftbusAdapter::GetInstance().AcceptSesion(sessionId, "test session", peerNetworkId);
        int32_t result = softbusSession->WaitSessionOpened(sessionId);
        EXPECT_EQ(result, 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "WaitSessionOpenedTest001 failed";
    }
    GTEST_LOG_(INFO) << "WaitSessionOpenedTest001 end";
}
} // namespace Test
} // namespace CloudSync
} // namespace OHOS