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
#include "file_transfer_manager.h"
#include "softbus_adapter_mock.h"
#include "softbus_session_mock.h"
#include "session_manager.h"
#include "socket_mock.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement::CloudSync {
namespace Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class SessionManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    static inline std::shared_ptr<SoftbusAdapterMock> softbusAdapterMock_ = nullptr;
    static inline std::shared_ptr<SoftbusSessionMock> softbusSessionMock_ = nullptr;
};

void SessionManagerTest::SetUpTestCase(void)
{
    std::cout << "SetUpTestCase" << std::endl;
}

void SessionManagerTest::TearDownTestCase(void)
{
    std::cout << "TearDownTestCase" << std::endl;
}

void SessionManagerTest::SetUp(void)
{
    std::cout << "SetUp" << std::endl;
    softbusAdapterMock_ = std::make_shared<SoftbusAdapterMock>();
    ISoftbusAdapterMock::iSoftbusAdapterMock_ = softbusAdapterMock_;

    softbusSessionMock_ = std::make_shared<SoftbusSessionMock>();
    ISoftbusSessionMock::iSoftbusSessionMock_ = softbusSessionMock_;
}

void SessionManagerTest::TearDown(void)
{
    std::cout << "TearDown" << std::endl;
    ISoftbusAdapterMock::iSoftbusAdapterMock_ = nullptr;
    softbusAdapterMock_ = nullptr;

    ISoftbusSessionMock::iSoftbusSessionMock_ = nullptr;
    softbusSessionMock_ = nullptr;
}

/**
 * @tc.name: InitTest001
 * @tc.desc: Verify the Init function
 * @tc.type: FUNC
 * @tc.require: IB3T9R
 */
HWTEST_F(SessionManagerTest, InitTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InitTest001 start";
    try {
        auto sessionManager = make_shared<SessionManager>();
        EXPECT_NE(sessionManager, nullptr);
        EXPECT_CALL(*softbusAdapterMock_, CreateSessionServer(_, _)).WillOnce(Return(-1));
        sessionManager->Init();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "InitTest001 failed";
    }
    GTEST_LOG_(INFO) << "InitTest001 end";
}

/**
 * @tc.name: SendDataTest001
 * @tc.desc: Verify the SendData function
 * @tc.type: FUNC
 * @tc.require: IB3T9R
 */
HWTEST_F(SessionManagerTest, SendDataTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SendDataTest001 start";
    try {
        auto sessionManager = make_shared<SessionManager>();
        char data[] = "test data";
        string peerNetworkId = "test peerNetworkId";
        EXPECT_CALL(*softbusSessionMock_, Start()).WillOnce(Return(0));
        int32_t ret = sessionManager->SendData(peerNetworkId, data, sizeof(data));
        EXPECT_EQ(ret, 0);
        sessionManager->ReleaseSession(SoftbusSession::TYPE_BYTES, peerNetworkId);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SendDataTest001 failed";
    }
    GTEST_LOG_(INFO) << "SendDataTest001 end";
}

/**
 * @tc.name: SendDataTest001
 * @tc.desc: Verify the SendFile function
 * @tc.type: FUNC
 * @tc.require: IB3T9R
 */
HWTEST_F(SessionManagerTest, SendFileTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SendFileTest001 start";
    try {
        auto sessionManager = make_shared<SessionManager>();
        string peerNetworkId = "test peerNetworkId";
        auto type = SoftbusSession::TYPE_BYTES;
        auto session = make_shared<SoftbusSession>(peerNetworkId, "test session", type);
        EXPECT_TRUE(session != nullptr);

        EXPECT_CALL(*softbusSessionMock_, Start()).WillOnce(Return(0));
        EXPECT_CALL(*softbusSessionMock_, SendFile(_, _)).WillOnce(Return(0));
        int32_t ret = sessionManager->SendFile(peerNetworkId, {"data/test"}, {"data/test"});
        EXPECT_EQ(ret, 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SendFileTest001 failed";
    }
    GTEST_LOG_(INFO) << "SendFileTest001 end";
}

/**
 * @tc.name: OnSessionOpenedTest001
 * @tc.desc: Verify the OnSessionOpened function
 * @tc.type: FUNC
 * @tc.require: IB3T9R
 */
HWTEST_F(SessionManagerTest, OnSessionOpenedTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnSessionOpenedTest001 start";
    try {
        auto sessionManager = make_shared<SessionManager>();
        string peerNetworkId = "test peerNetworkId";
        auto type = SoftbusSession::TYPE_BYTES;
        auto session = make_shared<SoftbusSession>(peerNetworkId, "test session", type);
        EXPECT_TRUE(session != nullptr);
        int sessionId = session->GetSessionId();
        sessionManager->OnSessionOpened(sessionId, -1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnSessionOpenedTest001 failed";
    }
    GTEST_LOG_(INFO) << "OnSessionOpenedTest001 end";
}

/**
 * @tc.name: OnSessionClosedTest001
 * @tc.desc: Verify the OnSessionClosed function
 * @tc.type: FUNC
 * @tc.require: IB3T9R
 */
HWTEST_F(SessionManagerTest, OnSessionClosedTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnSessionClosedTest001 start";
    try {
        auto sessionManager = make_shared<SessionManager>();
        string peerNetworkId = "test peerNetworkId";
        auto type = SoftbusSession::TYPE_BYTES;
        auto session = make_shared<SoftbusSession>(peerNetworkId, "test session", type);
        EXPECT_TRUE(session != nullptr);
        int sessionId = session->GetSessionId();
        sessionManager->OnSessionClosed(sessionId);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnSessionClosedTest001 failed";
    }
    GTEST_LOG_(INFO) << "OnSessionClosedTest001 end";
}

/**
 * @tc.name: OnDataReceivedTest001
 * @tc.desc: Verify the OnDataReceived function
 * @tc.type: FUNC
 * @tc.require: IB3T9R
 */
HWTEST_F(SessionManagerTest, OnDataReceivedTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnDataReceivedTest001 start";
    try {
        auto sessionManager = make_shared<SessionManager>();
        char data[] = "test data";
        string peerNetworkId = "test peerNetworkId";
        auto type = SoftbusSession::TYPE_BYTES;
        auto session = make_shared<SoftbusSession>(peerNetworkId, "test session", type);
        EXPECT_TRUE(session != nullptr);
        int sessionId = session->GetSessionId();
        sessionManager->OnDataReceived(peerNetworkId, sessionId, data, sizeof(data));
        EXPECT_EQ(sessionManager->dataHandler_, nullptr);
        auto fileTransferManager = make_shared<FileTransferManager>(sessionManager);
        sessionManager->RegisterDataHandler(fileTransferManager);
        sessionManager->OnDataReceived(peerNetworkId, sessionId, data, sizeof(data));
        EXPECT_NE(sessionManager->dataHandler_, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnDataReceivedTest001 failed";
    }
    GTEST_LOG_(INFO) << "OnDataReceivedTest001 end";
}

/**
 * @tc.name: OnFileReceivedTest001
 * @tc.desc: Verify the OnFileReceived function
 * @tc.type: FUNC
 * @tc.require: IB3T9R
 */
HWTEST_F(SessionManagerTest, OnFileReceivedTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnFileReceivedTest001 start";
    try {
        auto sessionManager = make_shared<SessionManager>();
        char data[] = "test data";
        string peerNetworkId = "test peerNetworkId";
        sessionManager->OnFileReceived(peerNetworkId, "data/test", 0);
        EXPECT_EQ(sessionManager->dataHandler_, nullptr);
        auto fileTransferManager = make_shared<FileTransferManager>(sessionManager);
        sessionManager->RegisterDataHandler(fileTransferManager);
        sessionManager->OnFileReceived(peerNetworkId, "data/test", 0);
        EXPECT_NE(sessionManager->dataHandler_, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnFileReceivedTest001 failed";
    }
    GTEST_LOG_(INFO) << "OnFileReceivedTest001 end";
}

/**
 * @tc.name: OnUserUnlockedTest001
 * @tc.desc: Verify the OnUserUnlocked function
 * @tc.type: FUNC
 * @tc.require: IB3T9R
 */
HWTEST_F(SessionManagerTest, OnUserUnlockedTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnUserUnlockedTest001 start";
    try {
        auto sessionManager = make_shared<SessionManager>();
        sessionManager->OnUserUnlocked();
        EXPECT_EQ(sessionManager->dataHandler_, nullptr);
        auto fileTransferManager = make_shared<FileTransferManager>(sessionManager);
        sessionManager->RegisterDataHandler(fileTransferManager);
        sessionManager->OnUserUnlocked();
        EXPECT_NE(sessionManager->dataHandler_, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnUserUnlockedTest001 failed";
    }
    GTEST_LOG_(INFO) << "OnUserUnlockedTest001 end";
}
} // namespace Test
} // namespace CloudSync
} // namespace OHOS