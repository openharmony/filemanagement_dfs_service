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

#include "network/softbus/softbus_session_listener.h"

#include <fcntl.h>
#include "gtest/gtest.h"
#include <memory>
#include <unistd.h>

#include "device_manager_impl.h"
#include "dm_constants.h"
#include "sandbox_helper.h"

#include "dfs_error.h"
#include "network/softbus/softbus_session_pool.h"
#include "socket_mock.h"
#include "utils_directory.h"
#include "utils_log.h"

using namespace OHOS::AppFileService;
using namespace OHOS::FileManagement;
using namespace std;
using namespace testing;
namespace {
bool g_mockGetTrustedDeviceList = true;
const string TEST_URI = "file://com.demo.a/data/test/el2/base/files/test.txt";
const string TEST_ERR_URI = "file://com.demo.a/data/test/el2/base/files/test2.txt";
const string TEST_MEDIA_URI = "file://media/data/test/el2/base/files/test.txt";
const string TEST_PATH = "/data/test/tdd";
const string TEST_ERR_PATH = "/data/test2/";
const string TEST_NETWORKID = "45656596896323231";
const string TEST_NETWORKID_TWO = "45656596896323232";
}

namespace OHOS {
namespace AppFileService {
int32_t SandboxHelper::GetPhysicalPath(const std::string &fileUri, const std::string &userId,
                                       std::string &physicalPath)
{
    if (fileUri == TEST_ERR_URI) {
        return -EINVAL;
    }

    if (fileUri == TEST_URI) {
        physicalPath = TEST_PATH;
        return E_OK;
    }

    if (fileUri == TEST_MEDIA_URI) {
        physicalPath = "";
        return E_OK;
    }

    physicalPath = TEST_ERR_PATH;
    return E_OK;
}

bool SandboxHelper::CheckValidPath(const std::string &filePath)
{
    if (filePath == TEST_ERR_PATH) {
        return false;
    }

    return true;
}
}

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

class SoftBusSessionListenerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() {};
    void TearDown() {};
    static inline shared_ptr<SocketMock> socketMock_ = nullptr;
};

void SoftBusSessionListenerTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    socketMock_ = make_shared<SocketMock>();
    SocketMock::dfsSocket = socketMock_;
    mode_t mode = (S_IRWXU | S_IRWXG | S_IRWXO); // 00777
    OHOS::Storage::DistributedFile::Utils::ForceCreateDirectory(TEST_PATH, mode);
}

void SoftBusSessionListenerTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    SocketMock::dfsSocket = nullptr;
    socketMock_ = nullptr;
    OHOS::Storage::DistributedFile::Utils::ForceRemoveDirectory(TEST_PATH);
}
/**
 * @tc.name: SoftBusSessionListenerTest_GetBundleName_0100
 * @tc.desc: test GetBundleName function.
 * @tc.type: FUNC
 * @tc.require: I9JKYU
 */
HWTEST_F(SoftBusSessionListenerTest, SoftBusSessionListenerTest_GetBundleName_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftBusSessionListenerTest_GetBundleName_0100 start";
    string bundleName = SoftBusSessionListener::GetBundleName(TEST_URI);
    EXPECT_EQ(bundleName, "com.demo.a");
    string testUri = "datashare:///com.demo.a/data/test/el2/base/files/test.txt";
    bundleName = SoftBusSessionListener::GetBundleName(testUri);
    EXPECT_EQ(bundleName, "");
    testUri = "file://";
    bundleName = SoftBusSessionListener::GetBundleName(testUri);
    EXPECT_EQ(bundleName, "");
    testUri = "file://com.demo.a";
    bundleName = SoftBusSessionListener::GetBundleName(testUri);
    EXPECT_EQ(bundleName, "");
    GTEST_LOG_(INFO) << "SoftBusSessionListenerTest_GetBundleName_0100 end";
}

/**
 * @tc.name: SoftBusSessionListenerTest_GetLocalUri_0100
 * @tc.desc: test GetLocalUri function.
 * @tc.type: FUNC
 * @tc.require: I9JKYU
 */
HWTEST_F(SoftBusSessionListenerTest, SoftBusSessionListenerTest_GetLocalUri_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftBusSessionListenerTest_GetLocalUri_0100 start";
    string localUri = SoftBusSessionListener::GetLocalUri(TEST_URI);
    EXPECT_EQ(localUri, "");
    string testUri2 = TEST_URI + "?networkid=454553656565656656";
    localUri = SoftBusSessionListener::GetLocalUri(testUri2);
    EXPECT_EQ(localUri, TEST_URI);
    GTEST_LOG_(INFO) << "SoftBusSessionListenerTest_GetLocalUri_0100 end";
}

/**
 * @tc.name: SoftBusSessionListenerTest_GetSandboxPath_0100
 * @tc.desc: test GetSandboxPath function.
 * @tc.type: FUNC
 * @tc.require: I9JKYU
 */
HWTEST_F(SoftBusSessionListenerTest, SoftBusSessionListenerTest_GetSandboxPath_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftBusSessionListenerTest_GetSandboxPath_0100 start";
    string sandboxPath = SoftBusSessionListener::GetSandboxPath(TEST_URI);
    EXPECT_EQ(sandboxPath, "");
    string testUri2 = "file://com.demo.a/distributedfiles/.remote_share/data/test/el2/base/files/test.txt";
    sandboxPath = SoftBusSessionListener::GetSandboxPath(testUri2);
    EXPECT_EQ(sandboxPath, "data/test/el2/base/files/test.txt");
    GTEST_LOG_(INFO) << "SoftBusSessionListenerTest_GetSandboxPath_0100 end";
}

/**
 * @tc.name: SoftBusSessionListenerTest_GetRealPath_0100
 * @tc.desc: test GetRealPath function.
 * @tc.type: FUNC
 * @tc.require: I9JKYU
 */
HWTEST_F(SoftBusSessionListenerTest, SoftBusSessionListenerTest_GetRealPath_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftBusSessionListenerTest_GetRealPath_0100 start";
    string netWorkId = "?networkid=454553656565656656";
    string realPath = SoftBusSessionListener::GetRealPath(TEST_URI);
    EXPECT_EQ(realPath, "");
    string testUri2 = "datashare:///com.demo.a/data/test/el2/base/files/test.txt" + netWorkId;
    realPath = SoftBusSessionListener::GetRealPath(testUri2);
    EXPECT_EQ(realPath, "");
    string testUri3 = TEST_URI + netWorkId;
    realPath = SoftBusSessionListener::GetRealPath(testUri3);
    EXPECT_EQ(realPath, TEST_PATH);

    string testUri4 = "file://docs/data/test/el2/base/files/test2.txt" + netWorkId;
    realPath = SoftBusSessionListener::GetRealPath(testUri4);
    EXPECT_EQ(realPath, "");

    string testUri5 = TEST_MEDIA_URI + netWorkId;
    realPath = SoftBusSessionListener::GetRealPath(testUri5);
    EXPECT_EQ(realPath, "");
    GTEST_LOG_(INFO) << "SoftBusSessionListenerTest_GetRealPath_0100 end";
}

/**
 * @tc.name: SoftBusSessionListenerTest_GetFileName_0100
 * @tc.desc: test GetFileName function.
 * @tc.type: FUNC
 * @tc.require: I9JKYU
 */
HWTEST_F(SoftBusSessionListenerTest, SoftBusSessionListenerTest_GetFileName_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftBusSessionListenerTest_GetFileName_0100 start";
    vector<string> fileList;
    fileList.push_back("/data/test/test1/t1.txt");
    fileList.push_back("/data/test/test1/t2.txt");
    vector<string> rltList;
    rltList = SoftBusSessionListener::GetFileName(fileList, "/data/test", "/data/test");
    EXPECT_EQ(rltList.size(), 2);
    EXPECT_EQ(rltList[0], "test1/t1.txt");
    EXPECT_EQ(rltList[1], "test1/t2.txt");
    rltList.clear();
    rltList = SoftBusSessionListener::GetFileName(fileList, "/data/test/test1/t1.txt", "/data/test");
    EXPECT_EQ(rltList.size(), 1);
    EXPECT_EQ(rltList[0], "t1.txt");

    rltList.clear();
    rltList = SoftBusSessionListener::GetFileName(fileList, "/data/test/test1/t1.txt", "??data/test/test1/t2.txt");
    EXPECT_EQ(rltList.size(), 1);
    EXPECT_EQ(rltList[0], "t2.txt");
    GTEST_LOG_(INFO) << "SoftBusSessionListenerTest_GetFileName_0100 end";
}

/**
 * @tc.name: SoftBusSessionListenerTest_OnSessionOpened_0100
 * @tc.desc: test OnSessionOpened function.
 * @tc.type: FUNC
 * @tc.require: I9JKYU
 */
HWTEST_F(SoftBusSessionListenerTest, SoftBusSessionListenerTest_OnSessionOpened_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftBusSessionListenerTest_OnSessionOpened_0100 start";
    PeerSocketInfo info;
    int32_t sessionId = 1;
    string sessionName1 = "sessionName1";
    info.name = const_cast<char*>(sessionName1.c_str());
    info.networkId = const_cast<char*>(TEST_NETWORKID.c_str());
    g_mockGetTrustedDeviceList = true;
    SoftBusSessionPool::GetInstance().DeleteSessionInfo(sessionName1);
    SoftBusSessionListener::OnSessionOpened(sessionId, info);

    SoftBusSessionPool::SessionInfo sessionInfo1{.sessionId = sessionId,
                                                 .srcUri = "file://com.demo.a/test/1",
                                                 .dstPath = "/data/test/1"};
    SoftBusSessionPool::GetInstance().AddSessionInfo(sessionName1, sessionInfo1);
    SoftBusSessionListener::OnSessionOpened(sessionId, info);

    SoftBusSessionPool::GetInstance().DeleteSessionInfo(sessionName1);
    string netWorkId = "?networkid=454553656565656656";
    sessionInfo1.srcUri = TEST_URI + netWorkId;
    SoftBusSessionPool::GetInstance().AddSessionInfo(sessionName1, sessionInfo1);
    SoftBusSessionListener::OnSessionOpened(sessionId, info);
    EXPECT_NE(SoftBusSessionPool::GetInstance().sessionMap_.find(sessionName1),
        SoftBusSessionPool::GetInstance().sessionMap_.end());

    string fileStr = TEST_PATH + "/test.txt";
    int32_t fd = open(fileStr.c_str(), O_RDWR | O_CREAT);
    ASSERT_TRUE(fd != -1) << "SoftBusSessionListenerTest_OnSessionOpened_0100 Create File Failed!";
    close(fd);
    EXPECT_CALL(*socketMock_, SendFile(_, _, _, _)).WillOnce(Return(-1));
    SoftBusSessionListener::OnSessionOpened(sessionId, info);
    EXPECT_EQ(SoftBusSessionPool::GetInstance().sessionMap_.find(sessionName1),
        SoftBusSessionPool::GetInstance().sessionMap_.end());

    SoftBusSessionPool::GetInstance().AddSessionInfo(sessionName1, sessionInfo1);
    EXPECT_CALL(*socketMock_, SendFile(_, _, _, _)).WillOnce(Return(0));
    SoftBusSessionListener::OnSessionOpened(sessionId, info);
    EXPECT_EQ(SoftBusSessionPool::GetInstance().sessionMap_.find(sessionName1),
        SoftBusSessionPool::GetInstance().sessionMap_.end());
    GTEST_LOG_(INFO) << "SoftBusSessionListenerTest_OnSessionOpened_0100 end";
}

/**
 * @tc.name: SoftBusSessionListenerTest_OnSessionClosed_0100
 * @tc.desc: test OnSessionOpened function.
 * @tc.type: FUNC
 * @tc.require: I9JKYU
 */
HWTEST_F(SoftBusSessionListenerTest, SoftBusSessionListenerTest_OnSessionClosed_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftBusSessionListenerTest_OnSessionClosed_0100 start";
    ShutdownReason reason = SHUTDOWN_REASON_LOCAL;
    int32_t sessionId = 1;
    SoftBusHandler::clientSessNameMap_.insert(make_pair(sessionId, "test"));
    SoftBusSessionListener::OnSessionClosed(sessionId, reason);

    EXPECT_EQ(SoftBusHandler::clientSessNameMap_.find(sessionId),
        SoftBusHandler::clientSessNameMap_.end());
    GTEST_LOG_(INFO) << "SoftBusSessionListenerTest_OnSessionClosed_0100 end";
}
} // namespace Test
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
