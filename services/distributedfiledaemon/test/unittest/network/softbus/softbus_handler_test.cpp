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

#include <memory>
#include <unistd.h>
#include <utility>
#include <securec.h>

#include "gtest/gtest.h"

#include "dfs_error.h"
#include "device_manager_impl.h"
#include "dm_constants.h"
#include "utils_directory.h"
#include "utils_log.h"

#include "network/softbus/softbus_session_pool.h"

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
using namespace testing::ext;
class SoftbusHandlerTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};
};

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
    handler.serverIdMap_.insert(std::make_pair(sessionName, 2));
    int32_t result = handler.CreateSessionServer(packageName, sessionName, role, physicalPath);
    handler.serverIdMap_.erase(sessionName);
    EXPECT_EQ(result, E_OK);
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

    int32_t result = handler.CreateSessionServer(packageName, sessionName, role, physicalPath);
    EXPECT_EQ(result, E_SOFTBUS_SESSION_FAILED);

    result = handler.CreateSessionServer("", sessionName, role, physicalPath);
    EXPECT_EQ(result, E_SOFTBUS_SESSION_FAILED);

    result = handler.CreateSessionServer(packageName, "", role, physicalPath);
    EXPECT_EQ(result, E_SOFTBUS_SESSION_FAILED);

    result = handler.CreateSessionServer(packageName, sessionName, role, "");
    EXPECT_EQ(result, E_SOFTBUS_SESSION_FAILED);
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

    int32_t result = handler.OpenSession(packageName, sessionName, physicalPath, role);
    EXPECT_EQ(result, E_OPEN_SESSION);
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
    SoftBusHandler handler;
    handler.ChangeOwnerIfNeeded(1, "");
    handler.ChangeOwnerIfNeeded(1, "sessionName");
    int32_t result = handler.OpenSession("packname", "sessionName", "/data/test", DFS_CHANNLE_ROLE_SOURCE);
    EXPECT_EQ(result, E_OPEN_SESSION);

    SoftBusSessionPool::SessionInfo sessionInfo1{.sessionId = SESSION_ID_ONE,
                                                 .srcUri = "file://com.demo.a/test/1",
                                                 .dstPath = "/data/test/1",
                                                 .uid = UID_ONE};
    string sessionName1 = "sessionName1";
    SoftBusSessionPool::GetInstance().AddSessionInfo(sessionName1, sessionInfo1);
    handler.ChangeOwnerIfNeeded(1, sessionName1);
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
}
} // namespace Test
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
