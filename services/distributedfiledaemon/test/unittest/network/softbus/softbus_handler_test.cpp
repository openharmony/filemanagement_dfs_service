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
#include "network/softbus/softbus_handler.h"

#include "gtest/gtest.h"
#include <memory>
#include <unistd.h>
#include <utility>
#include <securec.h>

#include "device_manager_impl.h"
#include "dm_constants.h"

#include "dfs_error.h"
#include "network/softbus/softbus_file_receive_listener.h"
#include "network/softbus/softbus_session_pool.h"
#include "socket_mock.h"
#include "utils_directory.h"
#include "utils_log.h"

using namespace OHOS::DistributedHardware;
using namespace OHOS::FileManagement;
using namespace std;

namespace {
bool g_mockGetTrustedDeviceList = true;
const string TEST_NETWORKID = "45656596896323231";
const string TEST_NETWORKID_TWO = "45656596896323232";
const string TEST_NETWORKID_THREE = "45656596896323233";
constexpr int SESSION_ID_ONE = 1;
constexpr int UID_ONE = 1;
}

namespace OHOS {
namespace DistributedHardware {
int32_t DeviceManagerImpl::GetTrustedDeviceList(const std::string &pkgName, const std::string &extra,
                                                std::vector<DmDeviceInfo> &deviceList)
{
    if (!g_mockGetTrustedDeviceList) {
        return ERR_DM_INPUT_PARA_INVALID;
    }

    DmDeviceInfo testInfo;
    (void)memcpy_s(testInfo.networkId, DM_MAX_DEVICE_NAME_LEN - 1,
                   TEST_NETWORKID.c_str(), TEST_NETWORKID.size());
    testInfo.authForm = DmAuthForm::IDENTICAL_ACCOUNT;
    deviceList.push_back(testInfo);
    
    DmDeviceInfo testInfo1;
    (void)memcpy_s(testInfo1.networkId, DM_MAX_DEVICE_NAME_LEN - 1,
                   TEST_NETWORKID_TWO.c_str(), TEST_NETWORKID_TWO.size());
    testInfo1.authForm = DmAuthForm::PEER_TO_PEER;
    deviceList.push_back(testInfo1);
    return DM_OK;
}
}
}

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace Test {
using namespace testing;
using namespace testing::ext;
class SoftbusHandlerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<SocketMock> socketMock_ = nullptr;
};

void SoftbusHandlerTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    socketMock_ = make_shared<SocketMock>();
    SocketMock::dfsSocket = socketMock_;
}

void SoftbusHandlerTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    SocketMock::dfsSocket = nullptr;
    socketMock_ = nullptr;
}

void SoftbusHandlerTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void SoftbusHandlerTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: SoftbusHandlerTest_CreateSessionServer_0100
 * @tc.desc: Verify the OccupySession by Cid function.
 * @tc.type: FUNC
 * @tc.require: I9JXPR
 */
HWTEST_F(SoftbusHandlerTest, SoftbusHandlerTest_CreateSessionServer_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusHandlerTest_CreateSessionServer_0100 start";
    SoftBusHandler handler;
    std::string packageName = "com.example.test";
    std::string sessionName = "testSession";
    DFS_CHANNEL_ROLE role = DFS_CHANNLE_ROLE_SOURCE;
    std::string physicalPath = "/data/test";
    EXPECT_CALL(*socketMock_, Socket(_)).WillOnce(Return(-1));
    int32_t result = handler.CreateSessionServer(packageName, sessionName, role, physicalPath);
    handler.serverIdMap_.erase(sessionName);
    EXPECT_EQ(result, ERR_BAD_VALUE);

    EXPECT_CALL(*socketMock_, Socket(_)).WillOnce(Return(0));
    EXPECT_CALL(*socketMock_, Listen(_, _, _, _)).WillOnce(Return(-1));
    result = handler.CreateSessionServer(packageName, sessionName, role, physicalPath);
    handler.serverIdMap_.erase(sessionName);
    EXPECT_EQ(result, ERR_BAD_VALUE);

    EXPECT_CALL(*socketMock_, Socket(_)).WillOnce(Return(0));
    EXPECT_CALL(*socketMock_, Listen(_, _, _, _)).WillOnce(Return(0));
    result = handler.CreateSessionServer(packageName, sessionName, role, physicalPath);
    EXPECT_EQ(result, E_OK);
    if (handler.serverIdMap_.find(sessionName) != handler.serverIdMap_.end()) {
        EXPECT_TRUE(true);
    } else {
        EXPECT_TRUE(false);
    }
    handler.serverIdMap_.erase(sessionName);
    EXPECT_EQ(string(SoftBusFileReceiveListener::GetRecvPath()), physicalPath);
    GTEST_LOG_(INFO) << "SoftbusHandlerTest_CreateSessionServer_0100 end";
}

/**
 * @tc.name: SoftbusHandlerTest_CreateSessionServer_0200
 * @tc.desc: Verify the OccupySession by Cid function.
 * @tc.type: FUNC
 * @tc.require: I9JXPR
 */
HWTEST_F(SoftbusHandlerTest, SoftbusHandlerTest_CreateSessionServer_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusHandlerTest_CreateSessionServer_0200 start";
    SoftBusHandler handler;
    std::string packageName = "com.example.test";
    std::string sessionName = "testSession";
    DFS_CHANNEL_ROLE role = DFS_CHANNLE_ROLE_SOURCE;
    std::string physicalPath = "/data/test";

    auto result = handler.CreateSessionServer("", sessionName, role, physicalPath);
    EXPECT_EQ(result, ERR_BAD_VALUE);

    result = handler.CreateSessionServer(packageName, "", role, physicalPath);
    EXPECT_EQ(result, ERR_BAD_VALUE);

    result = handler.CreateSessionServer(packageName, sessionName, role, "");
    EXPECT_EQ(result, ERR_BAD_VALUE);
    GTEST_LOG_(INFO) << "SoftbusHandlerTest_CreateSessionServer_0200 end";
}

/**
 * @tc.name: SoftbusHandlerTest_OpenSession_0100
 * @tc.desc: Verify the OccupySession by Cid function.
 * @tc.type: FUNC
 * @tc.require: I9JXPR
 */
HWTEST_F(SoftbusHandlerTest, SoftbusHandlerTest_OpenSession_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusHandlerTest_OpenSession_0100 start";
    SoftBusHandler handler;
    std::string packageName = "com.example.test";
    std::string sessionName = "testSession";
    DFS_CHANNEL_ROLE role = DFS_CHANNLE_ROLE_SOURCE;
    std::string physicalPath = "/data/test";

    EXPECT_CALL(*socketMock_, Socket(_)).WillOnce(Return(-1));
    int32_t result = handler.OpenSession(packageName, sessionName, physicalPath, role);
    EXPECT_EQ(result, ERR_BAD_VALUE);

    int32_t socketId = 0;
    EXPECT_CALL(*socketMock_, Socket(_)).WillOnce(Return(socketId));
    EXPECT_CALL(*socketMock_, Bind(_, _, _, _)).WillOnce(Return(-1));
    result = handler.OpenSession(packageName, sessionName, physicalPath, role);
    EXPECT_EQ(result, ERR_BAD_VALUE);

    EXPECT_CALL(*socketMock_, Socket(_)).WillOnce(Return(socketId));
    EXPECT_CALL(*socketMock_, Bind(_, _, _, _)).WillOnce(Return(0));
    result = handler.OpenSession(packageName, sessionName, physicalPath, role);
    EXPECT_EQ(result, 0);

    if (handler.clientSessNameMap_.find(socketId) != handler.clientSessNameMap_.end()) {
        EXPECT_TRUE(true);
    } else {
        EXPECT_TRUE(false);
    }

    handler.clientSessNameMap_.erase(socketId);
    GTEST_LOG_(INFO) << "SoftbusHandlerTest_OpenSession_0100 end";
}

/**
 * @tc.name: SoftbusHandlerTest_OpenSession_0200
 * @tc.desc: Verify the OpenSession by Cid function.
 * @tc.type: FUNC
 * @tc.require: I9JXPR
 */
HWTEST_F(SoftbusHandlerTest, SoftbusHandlerTest_OpenSession_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusHandlerTest_OpenSession_0100 start";
    SoftBusHandler handler;
    std::string packageName = "com.example.test";
    std::string sessionName = "testSession";
    DFS_CHANNEL_ROLE role = DFS_CHANNLE_ROLE_SOURCE;
    std::string physicalPath = "/data/test";

    int32_t result = handler.OpenSession("", sessionName, physicalPath, role);
    EXPECT_EQ(result, ERR_BAD_VALUE);

    result = handler.OpenSession(packageName, "", physicalPath, role);
    EXPECT_EQ(result, ERR_BAD_VALUE);

    result = handler.OpenSession(packageName, sessionName, "", role);
    EXPECT_EQ(result, ERR_BAD_VALUE);
    GTEST_LOG_(INFO) << "SoftbusHandlerTest_OpenSession_0100 end";
}

/**
 * @tc.name: SoftbusHandlerTest_ChangeOwnerIfNeeded_0100
 * @tc.desc: Verify the ChangeOwnerIfNeeded by Cid function.
 * @tc.type: FUNC
 * @tc.require: I9JXPR
 */
HWTEST_F(SoftbusHandlerTest, SoftbusHandlerTest_ChangeOwnerIfNeeded_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusHandlerTest_ChangeOwnerIfNeeded_0100 start";
    bool res = true;
    try {
        SoftBusHandler handler;
        handler.ChangeOwnerIfNeeded(1, "");
        handler.ChangeOwnerIfNeeded(1, "sessionName");

        SoftBusSessionPool::SessionInfo sessionInfo1{.sessionId = SESSION_ID_ONE,
                                                    .srcUri = "file://com.demo.a/test/1",
                                                    .dstPath = "/data/test/1",
                                                    .uid = UID_ONE};
        string sessionName1 = "sessionName1";
        SoftBusSessionPool::GetInstance().AddSessionInfo(sessionName1, sessionInfo1);
        handler.ChangeOwnerIfNeeded(1, sessionName1);
        SoftBusSessionPool::GetInstance().DeleteSessionInfo(sessionName1);
    } catch(...) {
        res = false;
    }

    EXPECT_TRUE(res);
    GTEST_LOG_(INFO) << "SoftbusHandlerTest_ChangeOwnerIfNeeded_0100 end";
}

/**
 * @tc.name: SoftbusHandlerTest_GetSessionName_0100
 * @tc.desc: Verify the GetSessionName by Cid function.
 * @tc.type: FUNC
 * @tc.require: I9JXPR
 */
HWTEST_F(SoftbusHandlerTest, SoftbusHandlerTest_GetSessionName_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusHandlerTest_GetSessionName_0100 start";
    string testSessionName = "mySessionName";
    int32_t sessionId = 1;
    SoftBusHandler::clientSessNameMap_.insert(std::make_pair(sessionId, testSessionName));
    SoftBusHandler handler;
    string sessionName = handler.GetSessionName(sessionId);
    EXPECT_EQ(sessionName, testSessionName);
    SoftBusHandler::clientSessNameMap_.erase(sessionId);
    sessionName = handler.GetSessionName(sessionId);
    EXPECT_EQ(sessionName, "");
    GTEST_LOG_(INFO) << "SoftbusHandlerTest_GetSessionName_0100 end";
}

/**
 * @tc.name: SoftbusHandlerTest_IsSameAccount_0100
 * @tc.desc: Verify the IsSameAccount.
 * @tc.type: FUNC
 * @tc.require: I9JXPR
 */
HWTEST_F(SoftbusHandlerTest, SoftbusHandlerTest_IsSameAccount_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusHandlerTest_IsSameAccount_0100 start";
    g_mockGetTrustedDeviceList = true;
    SoftBusHandler handler;
    bool flag = handler.IsSameAccount(TEST_NETWORKID);
    EXPECT_EQ(flag, true);
    flag = handler.IsSameAccount(TEST_NETWORKID_TWO);
    EXPECT_EQ(flag, false);
    flag = handler.IsSameAccount(TEST_NETWORKID_THREE);
    EXPECT_EQ(flag, false);
    g_mockGetTrustedDeviceList = false;
    flag = handler.IsSameAccount(TEST_NETWORKID);
    EXPECT_EQ(flag, false);
    g_mockGetTrustedDeviceList = true;
    GTEST_LOG_(INFO) << "SoftbusHandlerTest_IsSameAccount_0100 end";
}

/**
 * @tc.name: SoftbusHandlerTest_OnSinkSessionOpened_0100
 * @tc.desc: Verify the OnSinkSessionOpened.
 * @tc.type: FUNC
 * @tc.require: I9JXPR
 */
HWTEST_F(SoftbusHandlerTest, SoftbusHandlerTest_OnSinkSessionOpened_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusHandlerTest_OnSinkSessionOpened_0100 start";
    g_mockGetTrustedDeviceList = true;
    string sessionName1 = "sessionName1";
    string sessionName2 = "sessionName2";
    string sessionName3 = "sessionName3";
    int32_t sessionId1 = 1;
    int32_t sessionId2 = 2;
    int32_t sessionId3 = 3;

    PeerSocketInfo info1 = {
        .name = const_cast<char*>(sessionName1.c_str()),
        .networkId = const_cast<char*>(TEST_NETWORKID.c_str()),
    };

    PeerSocketInfo info2 = {
        .name = const_cast<char*>(sessionName2.c_str()),
        .networkId = const_cast<char*>(TEST_NETWORKID_TWO.c_str()),
    };

    PeerSocketInfo info3 = {
        .name = const_cast<char*>(sessionName3.c_str()),
        .networkId = const_cast<char*>(TEST_NETWORKID_THREE.c_str()),
    };

    SoftBusHandler handler;
    handler.serverIdMap_.clear();
    handler.serverIdMap_.insert(std::make_pair(sessionName2, 2));
    handler.OnSinkSessionOpened(sessionId1, info1);
    handler.OnSinkSessionOpened(sessionId2, info2);
    handler.OnSinkSessionOpened(sessionId3, info3);

    EXPECT_EQ(handler.GetSessionName(sessionId1), sessionName1);
    EXPECT_EQ(handler.GetSessionName(sessionId2), sessionName2);
    EXPECT_EQ(handler.GetSessionName(sessionId3), sessionName3);
    auto iter = handler.serverIdMap_.find(sessionName2);
    if (iter != handler.serverIdMap_.end()) {
        handler.serverIdMap_.clear();
        EXPECT_TRUE(true);
    } else {
        EXPECT_TRUE(false);
    }

    handler.clientSessNameMap_.erase(sessionId1);
    handler.clientSessNameMap_.erase(sessionId2);
    handler.clientSessNameMap_.erase(sessionId3);
    GTEST_LOG_(INFO) << "SoftbusHandlerTest_OnSinkSessionOpened_0100 end";
}

/**
 * @tc.name: SoftbusHandlerTest_CloseSession_0100
 * @tc.desc: Verify the CloseSession.
 * @tc.type: FUNC
 * @tc.require: I9JXPR
 */
HWTEST_F(SoftbusHandlerTest, SoftbusHandlerTest_CloseSession_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusHandlerTest_CloseSession_0100 start";
    string sessionName = "sessionName";
    SoftBusHandler::GetInstance().serverIdMap_.clear();
    SoftBusHandler::GetInstance().serverIdMap_.insert(std::make_pair(sessionName, 2));
    SoftBusSessionPool::SessionInfo sessionInfo1{.sessionId = SESSION_ID_ONE,
                                                 .srcUri = "file://com.demo.a/test/1",
                                                 .dstPath = "/data/test/1",
                                                 .uid = UID_ONE};
    string sessionName1 = "sessionName1";
    SoftBusSessionPool::GetInstance().AddSessionInfo(sessionName1, sessionInfo1);
    SoftBusSessionPool::SessionInfo sessionInfo;
    bool flag = SoftBusSessionPool::GetInstance().GetSessionInfo(sessionName1, sessionInfo);
    EXPECT_EQ(flag, true);
    SoftBusHandler::GetInstance().CloseSession(1, "sessionName1");
    flag = SoftBusSessionPool::GetInstance().GetSessionInfo(sessionName1, sessionInfo);
    EXPECT_EQ(flag, false);
    EXPECT_EQ(SoftBusHandler::GetInstance().serverIdMap_.size(), 1);

    SoftBusSessionPool::GetInstance().AddSessionInfo(sessionName, sessionInfo1);
    flag = SoftBusSessionPool::GetInstance().GetSessionInfo(sessionName, sessionInfo);
    EXPECT_EQ(flag, true);
    SoftBusHandler::GetInstance().CloseSession(1, "");
    flag = SoftBusSessionPool::GetInstance().GetSessionInfo(sessionName, sessionInfo);
    EXPECT_EQ(flag, true);
    EXPECT_EQ(SoftBusHandler::GetInstance().serverIdMap_.size(), 1);
    SoftBusHandler::GetInstance().CloseSession(1, sessionName);
    EXPECT_EQ(SoftBusHandler::GetInstance().serverIdMap_.size(), 0);
    flag = SoftBusSessionPool::GetInstance().GetSessionInfo(sessionName, sessionInfo);
    EXPECT_EQ(flag, false);
    GTEST_LOG_(INFO) << "SoftbusHandlerTest_CloseSession_0100 end";
}

/**
 * @tc.name: SoftbusHandlerTest_CloseSessionWithSessionName_0100
 * @tc.desc: Verify the CloseSession.
 * @tc.type: FUNC
 * @tc.require: I9JXPR
 */
HWTEST_F(SoftbusHandlerTest, SoftbusHandlerTest_CloseSessionWithSessionName_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusHandlerTest_CloseSessionWithSessionName_0100 start";

    SoftBusHandler::GetInstance().CloseSessionWithSessionName("");

    string sessionName = "sessionName";
    SoftBusHandler::GetInstance().serverIdMap_.insert(std::make_pair(sessionName, 2));
    SoftBusHandler::GetInstance().CloseSessionWithSessionName(sessionName);
    auto iter = SoftBusHandler::GetInstance().serverIdMap_.find(sessionName);
    if (iter == SoftBusHandler::GetInstance().serverIdMap_.end()) {
        EXPECT_TRUE(true);
    } else {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "SoftbusHandlerTest_CloseSessionWithSessionName_0100 end";
}

/**
 * @tc.name: SoftbusHandlerTest_CloseSessionWithSessionName_0200
 * @tc.desc: Verify the CloseSessionWithSessionName.
 * @tc.type: FUNC
 * @tc.require: I9JXPR
 */
HWTEST_F(SoftbusHandlerTest, SoftbusHandlerTest_CloseSessionWithSessionName_0200, TestSize.Level1)
{
    string sessionName = "sessionName";
    SoftBusHandler::GetInstance().clientSessNameMap_.insert(make_pair(0, sessionName));
    SoftBusHandler::GetInstance().clientSessNameMap_.insert(make_pair(1, "test"));
    SoftBusHandler::GetInstance().serverIdMap_.insert(std::make_pair(sessionName, 2));
    SoftBusHandler::GetInstance().CloseSessionWithSessionName(sessionName);
    auto iter = SoftBusHandler::GetInstance().serverIdMap_.find(sessionName);
    if (iter == SoftBusHandler::GetInstance().serverIdMap_.end()) {
        EXPECT_TRUE(true);
    } else {
        EXPECT_TRUE(false);
    }

    auto iterClient = SoftBusHandler::GetInstance().clientSessNameMap_.find(0);
    if (iterClient == SoftBusHandler::GetInstance().clientSessNameMap_.end()) {
        EXPECT_TRUE(true);
    } else {
        EXPECT_TRUE(false);
    }

    iterClient = SoftBusHandler::GetInstance().clientSessNameMap_.find(1);
    if (iterClient == SoftBusHandler::GetInstance().clientSessNameMap_.end()) {
        EXPECT_TRUE(false);
    } else {
        EXPECT_TRUE(true);
    }
    GTEST_LOG_(INFO) << "SoftbusHandlerTest_CloseSessionWithSessionName_0200 end";
}
} // namespace Test
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
