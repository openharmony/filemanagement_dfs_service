/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "device_manager_impl_mock.h"
#include "dm_constants.h"

#include "dfs_error.h"
#include "mock_other_method.h"
#include "network/softbus/softbus_file_receive_listener.h"
#include "network/softbus/softbus_session_pool.h"
#include "socket_mock.h"
#include "utils_directory.h"
#include "utils_log.h"

using namespace OHOS::DistributedHardware;
using namespace OHOS::FileManagement;
using namespace std;

namespace {
const string TEST_NETWORKID = "45656596896323231";
const string TEST_NETWORKID_TWO = "45656596896323232";
const string TEST_NETWORKID_THREE = "45656596896323233";
constexpr int SESSION_ID_ONE = 1;
constexpr int UID_ONE = 1;
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
    void CheckSrcSameAccountPass();
    void CheckSrcDiffAccountPass();
    void CheckSrcBothSamePass();
    void CheckSrcBothDiffPass();
    static inline shared_ptr<DfsDeviceOtherMethodMock> otherMethodMock_ = nullptr;
    static inline shared_ptr<SocketMock> socketMock_ = nullptr;
    static inline shared_ptr<DeviceManagerImplMock> deviceManagerImplMock_ = nullptr;
};

void SoftbusHandlerTest::CheckSrcSameAccountPass()
{
    AccountSA::OhosAccountInfo osAccountInfo;
    osAccountInfo.uid_ = "test";
    std::vector<int32_t> userIds{100, 101};
    EXPECT_CALL(*otherMethodMock_, QueryActiveOsAccountIds(_))
        .WillOnce(DoAll(SetArgReferee<0>(userIds), Return(FileManagement::E_OK)));
    EXPECT_CALL(*otherMethodMock_, GetOhosAccountInfo(_))
        .WillOnce(DoAll(SetArgReferee<0>(osAccountInfo), Return(FileManagement::E_OK)));
    EXPECT_CALL(*deviceManagerImplMock_, GetLocalDeviceInfo(_, _)).WillOnce(Return(0));
    EXPECT_CALL(*deviceManagerImplMock_, CheckSrcIsSameAccount(_, _)).WillOnce(Return(true));
}

void SoftbusHandlerTest::CheckSrcDiffAccountPass()
{
    std::vector<int32_t> userIds{100, 101};
    EXPECT_CALL(*otherMethodMock_, QueryActiveOsAccountIds(_))
        .WillOnce(DoAll(SetArgReferee<0>(userIds), Return(FileManagement::E_OK)));
    EXPECT_CALL(*deviceManagerImplMock_, GetLocalDeviceInfo(_, _)).WillOnce(Return(0));
}

void SoftbusHandlerTest::CheckSrcBothSamePass()
{
    AccountSA::OhosAccountInfo osAccountInfo;
    osAccountInfo.uid_ = "test";
    std::vector<int32_t> userIds{100, 101};
    EXPECT_CALL(*otherMethodMock_, QueryActiveOsAccountIds(_))
        .WillOnce(DoAll(SetArgReferee<0>(userIds), Return(FileManagement::E_OK)))
        .WillOnce(DoAll(SetArgReferee<0>(userIds), Return(FileManagement::E_OK)));
    EXPECT_CALL(*otherMethodMock_, GetOhosAccountInfo(_))
        .WillOnce(DoAll(SetArgReferee<0>(osAccountInfo), Return(FileManagement::E_OK)))
        .WillOnce(DoAll(SetArgReferee<0>(osAccountInfo), Return(FileManagement::E_OK)));
    EXPECT_CALL(*deviceManagerImplMock_, GetLocalDeviceInfo(_, _)).WillOnce(Return(0)).WillOnce(Return(0));
    EXPECT_CALL(*deviceManagerImplMock_, CheckSrcIsSameAccount(_, _)).WillOnce(Return(true));
    EXPECT_CALL(*socketMock_, SetAccessInfo(_, _)).WillOnce(Return(FileManagement::E_OK));
}

void SoftbusHandlerTest::CheckSrcBothDiffPass()
{
    std::vector<int32_t> userIds{100, 101};
    EXPECT_CALL(*otherMethodMock_, QueryActiveOsAccountIds(_))
        .WillOnce(DoAll(SetArgReferee<0>(userIds), Return(FileManagement::E_OK)))
        .WillOnce(DoAll(SetArgReferee<0>(userIds), Return(FileManagement::E_OK)));
    EXPECT_CALL(*deviceManagerImplMock_, GetLocalDeviceInfo(_, _)).WillOnce(Return(0)).WillOnce(Return(0));
}

void SoftbusHandlerTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    otherMethodMock_ = make_shared<DfsDeviceOtherMethodMock>();
    DfsDeviceOtherMethodMock::otherMethod = otherMethodMock_;
    socketMock_ = make_shared<SocketMock>();
    SocketMock::dfsSocket = socketMock_;
    deviceManagerImplMock_ = make_shared<DeviceManagerImplMock>();
    DeviceManagerImplMock::dfsDeviceManagerImpl = deviceManagerImplMock_;
}

void SoftbusHandlerTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    SocketMock::dfsSocket = nullptr;
    socketMock_ = nullptr;
    DeviceManagerImplMock::dfsDeviceManagerImpl = nullptr;
    deviceManagerImplMock_ = nullptr;
    DfsDeviceOtherMethodMock::otherMethod = nullptr;
    otherMethodMock_ = nullptr;
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
 * @tc.name: SoftbusHandlerTest_CreateSessionServer_0300
 * @tc.desc: Verify the CreateSessionServer by Cid function.
 * @tc.type: FUNC
 * @tc.require: I9JXPR
 */
HWTEST_F(SoftbusHandlerTest, SoftbusHandlerTest_CreateSessionServer_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusHandlerTest_CreateSessionServer_0300 start";
    std::string packageName = "com.example.test";
    std::string sessionName = "sessionName";
    DFS_CHANNEL_ROLE role = DFS_CHANNLE_ROLE_SOURCE;
    std::string physicalPath = "/data/test";
    
    auto result = SoftBusHandler::GetInstance().CreateSessionServer("", sessionName, role, physicalPath);
    EXPECT_EQ(result, ERR_BAD_VALUE);
    GTEST_LOG_(INFO) << "SoftbusHandlerTest_CreateSessionServer_0300 end";
}

/**
 * @tc.name: SoftbusHandlerTest_OpenSession_0100
 * @tc.desc: Verify the OccupySession by Cid function.
 * @tc.type: FUNC
 * @tc.require: I9JXPR
 */
HWTEST_F(SoftbusHandlerTest, SoftbusHandlerTest_OpenSession_0100, TestSize.Level1)
{
#ifndef SUPPORT_SAME_ACCOUNT
    GTEST_LOG_(INFO) << "SoftbusHandlerTest_OpenSession_0100 start";
    SoftBusHandler handler;
    std::string packageName = "com.example.test";
    std::string sessionName = "testSession";
    int32_t socketId;

    CheckSrcDiffAccountPass();
    EXPECT_CALL(*socketMock_, Socket(_)).WillOnce(Return(-1));
    int32_t result = handler.OpenSession(packageName, sessionName, TEST_NETWORKID, socketId);
    EXPECT_EQ(result, ERR_BAD_VALUE);

    CheckSrcBothDiffPass();
    EXPECT_CALL(*socketMock_, Socket(_)).WillOnce(Return(1));
    EXPECT_CALL(*socketMock_, Bind(_, _, _, _)).WillOnce(Return(-1));
    result = handler.OpenSession(packageName, sessionName, TEST_NETWORKID, socketId);
    EXPECT_EQ(result, ERR_BAD_VALUE);

    CheckSrcBothDiffPass();
    EXPECT_CALL(*socketMock_, Socket(_)).WillOnce(Return(1));
    EXPECT_CALL(*socketMock_, Bind(_, _, _, _)).WillOnce(Return(0));
    result = handler.OpenSession(packageName, sessionName, TEST_NETWORKID, socketId);
    EXPECT_EQ(result, 0);

    if (handler.clientSessNameMap_.find(socketId) != handler.clientSessNameMap_.end()) {
        EXPECT_TRUE(true);
    } else {
        EXPECT_TRUE(false);
    }

    handler.clientSessNameMap_.erase(socketId);
#endif
    GTEST_LOG_(INFO) << "SoftbusHandlerTest_OpenSession_0100 end";
}

/**
 * @tc.name: SoftbusHandlerTest_OpenSession_0200
 * @tc.desc: Verify the OccupySession by Cid function.
 * @tc.type: FUNC
 * @tc.require: I9JXPR
 */
HWTEST_F(SoftbusHandlerTest, SoftbusHandlerTest_OpenSession_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusHandlerTest_OpenSession_0200 start";
#ifdef SUPPORT_SAME_ACCOUNT
    SoftBusHandler handler;
    std::string packageName = "com.example.test";
    std::string sessionName = "testSession";
    int32_t socketId;

    CheckSrcSameAccountPass();
    std::vector<DmDeviceInfo> deviceList;
    DmDeviceInfo deviceInfo;
    deviceInfo.authForm = DmAuthForm::IDENTICAL_ACCOUNT;
    deviceInfo.extraData = "{\"OS_TYPE\":10}";
    auto res = strcpy_s(deviceInfo.networkId, DM_MAX_DEVICE_ID_LEN, TEST_NETWORKID.c_str());
    if (res != 0) {
        GTEST_LOG_(INFO) << "strcpy_s failed";
        return;
    }
    deviceList.push_back(deviceInfo);
    EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0)));
    EXPECT_CALL(*socketMock_, Socket(_)).WillOnce(Return(-1));
    int32_t result = handler.OpenSession(packageName, sessionName, TEST_NETWORKID, socketId);
    EXPECT_EQ(result, ERR_BAD_VALUE);

    CheckSrcBothSamePass();
    EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0)));
    EXPECT_CALL(*socketMock_, Socket(_)).WillOnce(Return(1));
    EXPECT_CALL(*socketMock_, Bind(_, _, _, _)).WillOnce(Return(-1));
    result = handler.OpenSession(packageName, sessionName, TEST_NETWORKID, socketId);
    EXPECT_EQ(result, ERR_BAD_VALUE);

    CheckSrcBothSamePass();
    EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0)));
    EXPECT_CALL(*socketMock_, Socket(_)).WillOnce(Return(1));
    EXPECT_CALL(*socketMock_, Bind(_, _, _, _)).WillOnce(Return(0));
    result = handler.OpenSession(packageName, sessionName, TEST_NETWORKID, socketId);
    EXPECT_EQ(result, 0);

    if (handler.clientSessNameMap_.find(socketId) != handler.clientSessNameMap_.end()) {
        EXPECT_TRUE(true);
    } else {
        EXPECT_TRUE(false);
    }

    handler.clientSessNameMap_.erase(socketId);
#endif
    GTEST_LOG_(INFO) << "SoftbusHandlerTest_OpenSession_0200 end";
}

/**
 * @tc.name: SoftbusHandlerTest_OpenSession_0300
 * @tc.desc: Verify the OpenSession by Cid function.
 * @tc.type: FUNC
 * @tc.require: I9JXPR
 */
HWTEST_F(SoftbusHandlerTest, SoftbusHandlerTest_OpenSession_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusHandlerTest_OpenSession_0300 start";
#ifndef SUPPORT_SAME_ACCOUNT
    SoftBusHandler handler;
    std::string packageName = "com.example.test";
    std::string sessionName = "testSession";
    int32_t socketId;
    std::string physicalPath = "/data/test";
    CheckSrcDiffAccountPass();
    int32_t result = handler.OpenSession("", sessionName, physicalPath, socketId);
    EXPECT_EQ(result, ERR_BAD_VALUE);

    CheckSrcDiffAccountPass();
    result = handler.OpenSession(packageName, "", physicalPath, socketId);
    EXPECT_EQ(result, ERR_BAD_VALUE);

    CheckSrcDiffAccountPass();
    result = handler.OpenSession(packageName, sessionName, "", socketId);
    EXPECT_EQ(result, ERR_BAD_VALUE);
#endif
    GTEST_LOG_(INFO) << "SoftbusHandlerTest_OpenSession_0300 end";
}

/**
 * @tc.name: SoftbusHandlerTest_OpenSession_0400
 * @tc.desc: Verify the OpenSession by Cid function.
 * @tc.type: FUNC
 * @tc.require: I9JXPR
 */
HWTEST_F(SoftbusHandlerTest, SoftbusHandlerTest_OpenSession_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusHandlerTest_OpenSession_0400 start";
#ifdef SUPPORT_SAME_ACCOUNT
    SoftBusHandler handler;
    std::string packageName = "com.example.test";
    std::string sessionName = "testSession";
    int32_t socketId;
    std::string physicalPath = "/data/test";
    CheckSrcSameAccountPass();
    int32_t result = handler.OpenSession("", sessionName, physicalPath, socketId);
    EXPECT_EQ(result, ERR_BAD_VALUE);

    CheckSrcSameAccountPass();
    result = handler.OpenSession(packageName, "", physicalPath, socketId);
    EXPECT_EQ(result, ERR_BAD_VALUE);

    CheckSrcSameAccountPass();
    result = handler.OpenSession(packageName, sessionName, "", socketId);
    EXPECT_EQ(result, ERR_BAD_VALUE);
#endif
    GTEST_LOG_(INFO) << "SoftbusHandlerTest_OpenSession_0400 end";
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
 * @tc.name: SoftbusHandlerTest_OnSinkSessionOpened_0100
 * @tc.desc: Verify the OnSinkSessionOpened.
 * @tc.type: FUNC
 * @tc.require: I9JXPR
 */
HWTEST_F(SoftbusHandlerTest, SoftbusHandlerTest_OnSinkSessionOpened_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusHandlerTest_OnSinkSessionOpened_0100 start";
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

    std::vector<DmDeviceInfo> deviceList;
    DmDeviceInfo deviceInfo1;
    deviceInfo1.authForm = DmAuthForm::IDENTICAL_ACCOUNT;
    auto res = strcpy_s(deviceInfo1.networkId, DM_MAX_DEVICE_ID_LEN, TEST_NETWORKID.c_str());
    if (res != 0) {
        GTEST_LOG_(INFO) << "strcpy_s failed";
        return;
    }
    deviceList.push_back(deviceInfo1);

    DmDeviceInfo deviceInfo2;
    deviceInfo2.authForm = DmAuthForm::SHARE;
    res = strcpy_s(deviceInfo2.networkId, DM_MAX_DEVICE_ID_LEN, TEST_NETWORKID_TWO.c_str());
    if (res != 0) {
        GTEST_LOG_(INFO) << "strcpy_s failed";
        return;
    }
    deviceList.push_back(deviceInfo2);

    DmDeviceInfo deviceInfo3;
    deviceInfo3.authForm = DmAuthForm::SHARE;
    res = strcpy_s(deviceInfo3.networkId, DM_MAX_DEVICE_ID_LEN, TEST_NETWORKID_THREE.c_str());
    if (res != 0) {
        GTEST_LOG_(INFO) << "strcpy_s failed";
        return;
    }
    deviceList.push_back(deviceInfo3);

    EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0)));
    handler.OnSinkSessionOpened(sessionId1, info1);

    EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0)));
    handler.OnSinkSessionOpened(sessionId2, info2);

    EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0)));
    handler.OnSinkSessionOpened(sessionId3, info3);

#ifdef SUPPORT_SAME_ACCOUNT
    auto iter = handler.serverIdMap_.find(sessionName2);
    if (iter == handler.serverIdMap_.end()) {
        EXPECT_TRUE(true);
    } else {
        EXPECT_TRUE(false);
    }
#else
    EXPECT_EQ(handler.GetSessionName(sessionId1), sessionName1);
    EXPECT_EQ(handler.GetSessionName(sessionId2), sessionName2);
    EXPECT_EQ(handler.GetSessionName(sessionId3), sessionName3);
#endif
    handler.clientSessNameMap_.clear();
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
        EXPECT_TRUE(false);
    } else {
        EXPECT_TRUE(true);
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
    SoftBusHandler::GetInstance().clientSessNameMap_.insert(make_pair(2, sessionName));
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
