/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "asset_recv_callback_mock.h"
#include "copy/ipc_wrapper.h"
#include "daemon_stub_mock.h"
#include "dfs_error.h"
#include "dfsu_access_token_helper.h"
#include "file_dfs_listener_mock.h"
#include "ipc/daemon_stub.h"
#include "ipc/distributed_file_daemon_ipc_interface_code.h"
#include "ipc/hmdfs_info.h"
#include "ipc_skeleton.h"
#include "message_parcel_mock.h"
#include "utils_log.h"

namespace {
bool g_getCallingUidTrue = true;
bool g_checkUriPermissionTrue = true;
bool g_checkCallerPermissionTrue = true;
bool g_isLocalCalling = true;
constexpr pid_t DATA_UID = 3012;
constexpr pid_t DAEMON_UID = 1009;
static pid_t UID = DAEMON_UID;
bool g_writeBatchUrisTrue = true;
int32_t g_readBatchUris = OHOS::FileManagement::E_OK;
} // namespace

namespace OHOS {
#ifdef CONFIG_IPC_SINGLE
using namespace IPC_SINGLE;
#endif
pid_t IPCSkeleton::GetCallingUid()
{
    if (!g_getCallingUidTrue) {
        return -1;
    }

    return UID;
}

bool IPCSkeleton::IsLocalCalling()
{
    return g_isLocalCalling;
}
} // namespace OHOS

namespace OHOS::FileManagement {
bool DfsuAccessTokenHelper::CheckUriPermission(const std::string &uriStr)
{
    return g_checkUriPermissionTrue;
}

bool DfsuAccessTokenHelper::CheckCallerPermission(const std::string &permissionName)
{
    return g_checkCallerPermissionTrue;
}
} // namespace OHOS::FileManagement

namespace OHOS {
namespace Storage {
namespace DistributedFile {

bool IpcWrapper::WriteUriByRawData(MessageParcel &data, const std::vector<std::string> &uriVec)
{
    return true;
}

bool IpcWrapper::GetData(void *&buffer, size_t size, const void *data)
{
    return true;
}

bool IpcWrapper::ReadBatchUriByRawData(MessageParcel &data, std::vector<std::string> &uriVec)
{
    return true;
}

bool IpcWrapper::WriteBatchUris(MessageParcel &data, const std::vector<std::string> &uriVec)
{
    return g_writeBatchUrisTrue;
}

int32_t IpcWrapper::ReadBatchUris(MessageParcel &data, std::vector<std::string> &uriVec)
{
    if (g_readBatchUris == FileManagement::E_OK) {
        uriVec.emplace_back("key");
        uriVec.emplace_back("uri");
        uriVec.emplace_back("1");
    }
    return g_readBatchUris;
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS

namespace OHOS::Storage::DistributedFile::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using namespace OHOS::FileManagement;

namespace {
DistributedHardware::DmDeviceInfo deviceInfo = {
    .deviceId = "testdevid",
    .deviceName = "testdevname",
    .networkId = "testnetworkid",
};
}

const std::string SRC_URI =
    "file://docs/storage/Users/currentUser/"
    "Documents?networkid=xxxxx";
const std::string DST_URI = "file://docs/storage/Users/currentUser/Documents";
const std::string SRC_DEVICE_ID = "testSrcDeviceId";
const sptr<IRemoteObject> LISTENER = sptr(new MockDaemonStub());
const std::string COPY_PATH = "tmpDir";
const std::string sessionName = "DistributedDevice0";

HmdfsInfo fileInfo = {
    .copyPath = COPY_PATH,
    .dirExistFlag = false,
    .sessionName = sessionName,
};

class DaemonStubTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<MockDaemonStub> daemonStub_;
    static inline shared_ptr<MessageParcelMock> messageParcelMock_ = nullptr;
};

void DaemonStubTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    daemonStub_ = std::make_shared<MockDaemonStub>();
}

void DaemonStubTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    daemonStub_ = nullptr;
}

void DaemonStubTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
    g_checkUriPermissionTrue = true;
    UID = DATA_UID;
    messageParcelMock_ = make_shared<MessageParcelMock>();
    MessageParcelMock::messageParcel = messageParcelMock_;
}

void DaemonStubTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
    messageParcelMock_ = nullptr;
    MessageParcelMock::messageParcel = nullptr;
}

/**
 * @tc.name: DaemonStubOnRemoteRequestTest001
 * @tc.desc: Verify the OnRemoteRequest function
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DaemonStubTest, DaemonStubOnRemoteRequestTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "DaemonStubOnRemoteRequestTest001 Start";
    uint32_t code = static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_OPEN_P2P_CONNECTION);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    EXPECT_CALL(*messageParcelMock_, ReadInterfaceToken()).WillOnce(Return(u"test"));
    auto ret = daemonStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, MockDaemonStub::DFS_DAEMON_DESCRIPTOR_IS_EMPTY);

    EXPECT_CALL(*messageParcelMock_, ReadInterfaceToken()).WillOnce(Return(IDaemon::GetDescriptor()));
    uint32_t errCode = 65535;
    g_isLocalCalling = false;
    ret = daemonStub_->OnRemoteRequest(errCode, data, reply, option);
    EXPECT_EQ(ret, E_ALLOW_LOCAL_CALL_ONLY);

    EXPECT_CALL(*messageParcelMock_, ReadInterfaceToken()).WillOnce(Return(IDaemon::GetDescriptor()));
    code = static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_REQUEST_SEND_FILE);
    ret = daemonStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, E_IPC_READ_FAILED);

    EXPECT_CALL(*messageParcelMock_, ReadInterfaceToken()).WillOnce(Return(IDaemon::GetDescriptor()));
    code = static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_GET_REMOTE_COPY_INFO);
    ret = daemonStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, E_IPC_READ_FAILED);

    g_isLocalCalling = true;
    daemonStub_->opToInterfaceMap_.insert(make_pair(errCode, nullptr));
    EXPECT_CALL(*messageParcelMock_, ReadInterfaceToken()).WillOnce(Return(IDaemon::GetDescriptor()));
    ret = daemonStub_->OnRemoteRequest(errCode, data, reply, option);
    EXPECT_EQ(ret, 305);
    daemonStub_->opToInterfaceMap_.erase(errCode);
    GTEST_LOG_(INFO) << "DaemonStubOnRemoteRequestTest001 End";
}

/**
 * @tc.name: DaemonStubHandleConnectDfsTest001
 * @tc.desc: Verify the HandleConnectDfs function
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DaemonStubTest, DaemonStubHandleConnectDfsTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "DaemonStubHandleConnectDfsTest001 Start";
    MessageParcel data;
    MessageParcel reply;

    g_checkCallerPermissionTrue = false;
    auto ret = daemonStub_->HandleConnectDfs(data, reply);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);

    g_checkCallerPermissionTrue = true;
    EXPECT_CALL(*messageParcelMock_, ReadCString()).WillOnce(Return(nullptr));
    ret = daemonStub_->HandleConnectDfs(data, reply);
    EXPECT_EQ(ret, -1);

    EXPECT_CALL(*messageParcelMock_, ReadCString()).WillOnce(Return("testDeviceId")).WillOnce(Return(nullptr));
    ret = daemonStub_->HandleConnectDfs(data, reply);
    EXPECT_EQ(ret, -1);

    EXPECT_CALL(*messageParcelMock_, ReadCString())
        .WillOnce(Return("testDeviceId"))
        .WillOnce(Return("testDeviceName"))
        .WillOnce(Return(nullptr));
    ret = daemonStub_->HandleConnectDfs(data, reply);
    EXPECT_EQ(ret, -1);

    EXPECT_CALL(*messageParcelMock_, ReadCString())
        .WillOnce(Return("testDeviceId"))
        .WillOnce(Return("testDeviceName"))
        .WillOnce(Return("testNetworkId"));
    EXPECT_CALL(*messageParcelMock_, ReadUint16()).WillOnce(Return(1));
    EXPECT_CALL(*messageParcelMock_, ReadUint32()).WillOnce(Return(1));
    EXPECT_CALL(*messageParcelMock_, ReadInt32()).WillOnce(Return(0));
    EXPECT_CALL(*daemonStub_, ConnectDfs(_)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    ret = daemonStub_->HandleConnectDfs(data, reply);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "DaemonStubHandleConnectDfsTest001 End";
}

/**
 * @tc.name: DaemonStubHandleDisconnectDfsTest001
 * @tc.desc: Verify the HandleDisconnectDfs function
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DaemonStubTest, DaemonStubHandleDisconnectDfsTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "DaemonStubHandleDisconnectDfsTest001 Start";
    MessageParcel data;
    MessageParcel reply;

    g_checkCallerPermissionTrue = false;
    auto ret = daemonStub_->HandleDisconnectDfs(data, reply);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);

    g_checkCallerPermissionTrue = true;
    EXPECT_CALL(*messageParcelMock_, ReadCString()).WillOnce(Return(nullptr));
    ret = daemonStub_->HandleDisconnectDfs(data, reply);
    EXPECT_EQ(ret, -1);

    EXPECT_CALL(*messageParcelMock_, ReadCString()).WillOnce(Return("testDeviceId")).WillOnce(Return(nullptr));
    ret = daemonStub_->HandleDisconnectDfs(data, reply);
    EXPECT_EQ(ret, -1);

    EXPECT_CALL(*messageParcelMock_, ReadCString())
        .WillOnce(Return("testDeviceId"))
        .WillOnce(Return("testDeviceName"))
        .WillOnce(Return(nullptr));
    ret = daemonStub_->HandleDisconnectDfs(data, reply);
    EXPECT_EQ(ret, -1);

    EXPECT_CALL(*messageParcelMock_, ReadCString())
        .WillOnce(Return("testDeviceId"))
        .WillOnce(Return("testDeviceName"))
        .WillOnce(Return("testNetworkId"));
    EXPECT_CALL(*messageParcelMock_, ReadUint16()).WillOnce(Return(1));
    EXPECT_CALL(*messageParcelMock_, ReadUint32()).WillOnce(Return(1));
    EXPECT_CALL(*messageParcelMock_, ReadInt32()).WillOnce(Return(0));
    EXPECT_CALL(*daemonStub_, DisconnectDfs(_)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    ret = daemonStub_->HandleDisconnectDfs(data, reply);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "DaemonStubHandleDisconnectDfsTest001 End";
}

/**
 * @tc.name: DaemonStubHandleOpenP2PConnectionExTest001
 * @tc.desc: Verify the DaemonStubHandleOpenP2PConnectionExTest001 function
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DaemonStubTest, DaemonStubHandleOpenP2PConnectionExTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "DaemonStubHandleOpenP2PConnectionExTest001 Start";
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(false));
    auto ret = daemonStub_->HandleOpenP2PConnectionEx(data, reply);
    EXPECT_EQ(ret, E_IPC_READ_FAILED);

    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, ReadRemoteObject()).WillOnce(Return(nullptr));
    EXPECT_CALL(*daemonStub_, OpenP2PConnectionEx(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    ret = daemonStub_->HandleOpenP2PConnectionEx(data, reply);
    EXPECT_EQ(ret, E_OK);

    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(true));
    sptr<IRemoteObject> listenerPtr = sptr(new FileDfsListenerMock());
    EXPECT_CALL(*messageParcelMock_, ReadRemoteObject()).WillOnce(Return(listenerPtr));
    EXPECT_CALL(*daemonStub_, OpenP2PConnectionEx(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    ret = daemonStub_->HandleOpenP2PConnectionEx(data, reply);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "DaemonStubHandleOpenP2PConnectionExTest001 End";
}

/**
 * @tc.name: DaemonStubHandleCloseP2PConnectionExTest001
 * @tc.desc: Verify the HandleCloseP2PConnectionEx function
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DaemonStubTest, DaemonStubHandleCloseP2PConnectionExTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "DaemonStubHandleCloseP2PConnectionExTest001 Start";
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(false));
    auto ret = daemonStub_->HandleCloseP2PConnectionEx(data, reply);
    EXPECT_EQ(ret, E_IPC_READ_FAILED);

    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(true));
    EXPECT_CALL(*daemonStub_, CloseP2PConnectionEx(_)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    ret = daemonStub_->HandleCloseP2PConnectionEx(data, reply);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "DaemonStubHandleCloseP2PConnectionExTest001 End";
}

/**
 * @tc.name: DaemonStubHandlePrepareSessionTest001
 * @tc.desc: Verify the HandlePrepareSession function
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DaemonStubTest, DaemonStubHandlePrepareSessionTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "DaemonStubHandlePrepareSessionTest001 Start";
    MessageParcel data;
    MessageParcel reply;

    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(false));
    auto ret = daemonStub_->HandlePrepareSession(data, reply);
    EXPECT_EQ(ret, E_IPC_READ_FAILED);

    g_checkUriPermissionTrue = false;
    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(true));
    ret = daemonStub_->HandlePrepareSession(data, reply);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);

    g_checkUriPermissionTrue = true;
    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(true)).WillOnce(Return(false));
    ret = daemonStub_->HandlePrepareSession(data, reply);
    EXPECT_EQ(ret, E_IPC_READ_FAILED);

    EXPECT_CALL(*messageParcelMock_, ReadString(_))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(false));
    ret = daemonStub_->HandlePrepareSession(data, reply);
    EXPECT_EQ(ret, E_IPC_READ_FAILED);

    EXPECT_CALL(*messageParcelMock_, ReadString(_))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, ReadRemoteObject()).WillOnce(Return(nullptr));
    ret = daemonStub_->HandlePrepareSession(data, reply);
    EXPECT_EQ(ret, E_IPC_READ_FAILED);

    sptr<IRemoteObject> listenerPtr = sptr(new MockDaemonStub());
    EXPECT_CALL(*messageParcelMock_, ReadString(_))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(false));
    EXPECT_CALL(*messageParcelMock_, ReadRemoteObject()).WillOnce(Return(listenerPtr));
    ret = daemonStub_->HandlePrepareSession(data, reply);
    EXPECT_EQ(ret, E_IPC_READ_FAILED);
    GTEST_LOG_(INFO) << "DaemonStubHandlePrepareSessionTest001 End";
}

/**
 * @tc.name: DaemonStubHandlePrepareSessionTest002
 * @tc.desc: Verify the HandlePrepareSession function
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DaemonStubTest, DaemonStubHandlePrepareSessionTest002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "DaemonStubHandlePrepareSessionTest002 Start";
    MessageParcel data;
    MessageParcel reply;
    g_checkUriPermissionTrue = true;
    sptr<IRemoteObject> listenerPtr = sptr(new MockDaemonStub());
    EXPECT_CALL(*messageParcelMock_, ReadString(_))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, ReadRemoteObject()).WillOnce(Return(listenerPtr));
    EXPECT_CALL(*messageParcelMock_, ReadBool(_)).WillOnce(Return(false));
    auto ret = daemonStub_->HandlePrepareSession(data, reply);
    EXPECT_EQ(ret, E_IPC_READ_FAILED);

    EXPECT_CALL(*messageParcelMock_, ReadString(_))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, ReadRemoteObject()).WillOnce(Return(listenerPtr));
    EXPECT_CALL(*messageParcelMock_, ReadBool(_)).WillOnce(Return(true));
    EXPECT_CALL(*daemonStub_, PrepareSession(_, _, _, _, _)).WillOnce(Return(E_INVAL_ARG));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(false));
    ret = daemonStub_->HandlePrepareSession(data, reply);
    EXPECT_EQ(ret, E_IPC_WRITE_FAILED);

    EXPECT_CALL(*messageParcelMock_, ReadString(_))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, ReadRemoteObject()).WillOnce(Return(listenerPtr));
    EXPECT_CALL(*messageParcelMock_, ReadBool(_)).WillOnce(Return(true));
    EXPECT_CALL(*daemonStub_, PrepareSession(_, _, _, _, _)).WillOnce(Return(E_INVAL_ARG));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    ret = daemonStub_->HandlePrepareSession(data, reply);
    EXPECT_EQ(ret, E_INVAL_ARG);
    GTEST_LOG_(INFO) << "DaemonStubHandlePrepareSessionTest002 End";
}

/**
 * @tc.name: DaemonStubHandleCancelCopyTaskTest001
 * @tc.desc: Verify the HandleCancelCopyTask function
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DaemonStubTest, DaemonStubHandleCancelCopyTaskTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "DaemonStubHandleCancelCopyTaskTest001 Start";
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(false));
    auto ret = daemonStub_->HandleCancelCopyTask(data, reply);
    EXPECT_EQ(ret, E_IPC_READ_FAILED);

    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(true));
    EXPECT_CALL(*daemonStub_, CancelCopyTask(_)).WillOnce(Return(E_OK));
    ret = daemonStub_->HandleCancelCopyTask(data, reply);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "DaemonStubHandleCancelCopyTaskTest001 End";
}

/**
 * @tc.name: DaemonStubHandleInnerCancelCopyTaskTest001
 * @tc.desc: Verify the HandleInnerCancelCopyTask function
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DaemonStubTest, DaemonStubHandleInnerCancelCopyTaskTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "DaemonStubHandleInnerCancelCopyTaskTest001 Start";
    MessageParcel data;
    MessageParcel reply;

    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(false));
    auto ret = daemonStub_->HandleInnerCancelCopyTask(data, reply);
    EXPECT_EQ(ret, E_IPC_READ_FAILED);

    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(true)).WillOnce(Return(false));
    ret = daemonStub_->HandleInnerCancelCopyTask(data, reply);
    EXPECT_EQ(ret, E_IPC_READ_FAILED);

    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*daemonStub_, CancelCopyTask(_, _)).WillOnce(Return(E_OK));
    ret = daemonStub_->HandleInnerCancelCopyTask(data, reply);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "DaemonStubHandleInnerCancelCopyTaskTest001 End";
}

/**
 * @tc.name: DaemonStubHandleRequestSendFileTest001
 * @tc.desc: Verify the HandleRequestSendFile function
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DaemonStubTest, DaemonStubHandleRequestSendFileTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "DaemonStubHandleRequestSendFileTest001 Start";
    UID = DAEMON_UID;
    MessageParcel data;
    MessageParcel reply;

    g_getCallingUidTrue = true;
    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(false));
    auto ret = daemonStub_->HandleRequestSendFile(data, reply);
    EXPECT_EQ(ret, E_IPC_READ_FAILED);

    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(true)).WillOnce(Return(false));
    ret = daemonStub_->HandleRequestSendFile(data, reply);
    EXPECT_EQ(ret, E_IPC_READ_FAILED);

    EXPECT_CALL(*messageParcelMock_, ReadString(_))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(false));
    ret = daemonStub_->HandleRequestSendFile(data, reply);
    EXPECT_EQ(ret, E_IPC_READ_FAILED);

    EXPECT_CALL(*messageParcelMock_, ReadString(_))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(false));
    ret = daemonStub_->HandleRequestSendFile(data, reply);
    EXPECT_EQ(ret, E_IPC_READ_FAILED);

    EXPECT_CALL(*messageParcelMock_, ReadString(_))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(true));
    EXPECT_CALL(*daemonStub_, RequestSendFile(_, _, _, _)).WillOnce(Return(E_INVAL_ARG));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    ret = daemonStub_->HandleRequestSendFile(data, reply);
    EXPECT_EQ(ret, E_INVAL_ARG);
    GTEST_LOG_(INFO) << "DaemonStubHandleRequestSendFileTest001 End";
}

/**
 * @tc.name: DaemonStubHandleRequestSendFileACLTest_0001
 * @tc.desc: Verify the HandleRequestSendFileACL function (read failures)
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DaemonStubTest, DaemonStubHandleRequestSendFileACLTest_0001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "DaemonStubHandleRequestSendFileACLTest_0001 Start";
    MessageParcel data;
    MessageParcel reply;

    g_getCallingUidTrue = true;
    // Test srcUri read failure
    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(false));
    auto ret = daemonStub_->HandleRequestSendFileACL(data, reply);
    EXPECT_EQ(ret, E_IPC_READ_FAILED);

    // Test dstPath read failure
    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(true)).WillOnce(Return(false));
    ret = daemonStub_->HandleRequestSendFileACL(data, reply);
    EXPECT_EQ(ret, E_IPC_READ_FAILED);

    // Test dstDeviceId read failure
    EXPECT_CALL(*messageParcelMock_, ReadString(_))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(false));
    ret = daemonStub_->HandleRequestSendFileACL(data, reply);
    EXPECT_EQ(ret, E_IPC_READ_FAILED);

    // Test sessionName read failure
    EXPECT_CALL(*messageParcelMock_, ReadString(_))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(false));
    ret = daemonStub_->HandleRequestSendFileACL(data, reply);
    EXPECT_EQ(ret, E_IPC_READ_FAILED);

    // Test userId read failure
    EXPECT_CALL(*messageParcelMock_, ReadString(_))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(Return(false));
    ret = daemonStub_->HandleRequestSendFileACL(data, reply);
    EXPECT_EQ(ret, E_IPC_READ_FAILED);

    GTEST_LOG_(INFO) << "DaemonStubHandleRequestSendFileACLTest_0001 End";
}

/**
 * @tc.name: DaemonStubHandleRequestSendFileACLTest_0002
 * @tc.desc: Verify the HandleRequestSendFileACL function (success cases)
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DaemonStubTest, DaemonStubHandleRequestSendFileACLTest_0002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "DaemonStubHandleRequestSendFileACLTest_0002 Start";
    MessageParcel data;
    MessageParcel reply;

    g_getCallingUidTrue = true;
    // Test accountId read failure
    EXPECT_CALL(*messageParcelMock_, ReadString(_))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(false));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(Return(true));
    auto ret = daemonStub_->HandleRequestSendFileACL(data, reply);
    EXPECT_EQ(ret, E_IPC_READ_FAILED);

    // Test networkId read failure
    EXPECT_CALL(*messageParcelMock_, ReadString(_))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(false));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(Return(true));
    ret = daemonStub_->HandleRequestSendFileACL(data, reply);
    EXPECT_EQ(ret, E_IPC_READ_FAILED);

    // Test success path
    EXPECT_CALL(*messageParcelMock_, ReadString(_))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*daemonStub_, RequestSendFileACL(_, _, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    ret = daemonStub_->HandleRequestSendFileACL(data, reply);
    EXPECT_EQ(ret, E_OK);

    // Test RequestSendFileACL returns error
    EXPECT_CALL(*messageParcelMock_, ReadString(_))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*daemonStub_, RequestSendFileACL(_, _, _, _, _)).WillOnce(Return(E_INVAL_ARG));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    ret = daemonStub_->HandleRequestSendFileACL(data, reply);
    EXPECT_EQ(ret, E_INVAL_ARG);

    GTEST_LOG_(INFO) << "DaemonStubHandleRequestSendFileACLTest_0002 End";
}

/**
 * @tc.name: DaemonStubHandleGetRemoteCopyInfoTest001
 * @tc.desc: Verify the HandleGetRemoteCopyInfo function
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DaemonStubTest, DaemonStubHandleGetRemoteCopyInfoTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "DaemonStubHandleGetRemoteCopyInfoTest001 Start";
    UID = DAEMON_UID;
    MessageParcel data;
    MessageParcel reply;

    g_getCallingUidTrue = true;
    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(false));
    auto ret = daemonStub_->HandleGetRemoteCopyInfo(data, reply);
    EXPECT_EQ(ret, E_IPC_READ_FAILED);

    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(true));
    EXPECT_CALL(*daemonStub_, GetRemoteCopyInfo(_, _, _)).WillOnce(Return(E_INVAL_ARG));
    ret = daemonStub_->HandleGetRemoteCopyInfo(data, reply);
    EXPECT_EQ(ret, E_IPC_READ_FAILED);

    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(true));
    EXPECT_CALL(*daemonStub_, GetRemoteCopyInfo(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, WriteBool(_)).WillOnce(Return(false));
    ret = daemonStub_->HandleGetRemoteCopyInfo(data, reply);
    EXPECT_EQ(ret, E_IPC_READ_FAILED);

    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(true));
    EXPECT_CALL(*daemonStub_, GetRemoteCopyInfo(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, WriteBool(_)).WillOnce(Return(true)).WillOnce(Return(false));
    ret = daemonStub_->HandleGetRemoteCopyInfo(data, reply);
    EXPECT_EQ(ret, E_IPC_READ_FAILED);

    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(true));
    EXPECT_CALL(*daemonStub_, GetRemoteCopyInfo(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, WriteBool(_)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(false));
    ret = daemonStub_->HandleGetRemoteCopyInfo(data, reply);
    EXPECT_EQ(ret, E_IPC_READ_FAILED);

    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(true));
    EXPECT_CALL(*daemonStub_, GetRemoteCopyInfo(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, WriteBool(_)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    ret = daemonStub_->HandleGetRemoteCopyInfo(data, reply);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "DaemonStubHandleGetRemoteCopyInfoTest001 End";
}

/**
 * @tc.name: DaemonStubHandleGetRemoteCopyInfoTest002
 * @tc.desc: Verify the HandleGetRemoteCopyInfo function
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DaemonStubTest, DaemonStubHandleGetRemoteCopyInfoTest002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "DaemonStubHandleGetRemoteCopyInfoTest002 Start";
    UID = DAEMON_UID;
    MessageParcel data;
    MessageParcel reply;

    g_getCallingUidTrue = true;
    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(true));
    EXPECT_CALL(*daemonStub_, GetRemoteCopyInfo(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, WriteBool(_)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    auto ret = daemonStub_->HandleGetRemoteCopyInfo(data, reply);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "DaemonStubHandleGetRemoteCopyInfoTest002 End";
}

/**
 * @tc.name: DaemonStubHandleGetRemoteCopyInfoACLTest_0001
 * @tc.desc: Verify the HandleGetRemoteCopyInfoACL function (read failures)
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DaemonStubTest, DaemonStubHandleGetRemoteCopyInfoACLTest_0001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "DaemonStubHandleGetRemoteCopyInfoACLTest_0001 Start";
    MessageParcel data;
    MessageParcel reply;

    g_getCallingUidTrue = true;
    // Test srcUri read failure
    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(false));
    auto ret = daemonStub_->HandleGetRemoteCopyInfoACL(data, reply);
    EXPECT_EQ(ret, E_IPC_READ_FAILED);

    // Test userId read failure
    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(Return(false));
    ret = daemonStub_->HandleGetRemoteCopyInfoACL(data, reply);
    EXPECT_EQ(ret, E_IPC_READ_FAILED);

    // Test accountId read failure
    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(true)).WillOnce(Return(false));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(Return(true));
    ret = daemonStub_->HandleGetRemoteCopyInfoACL(data, reply);
    EXPECT_EQ(ret, E_IPC_READ_FAILED);

    // Test networkId read failure
    EXPECT_CALL(*messageParcelMock_, ReadString(_))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(false));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(Return(true));
    ret = daemonStub_->HandleGetRemoteCopyInfoACL(data, reply);
    EXPECT_EQ(ret, E_IPC_READ_FAILED);

    GTEST_LOG_(INFO) << "DaemonStubHandleGetRemoteCopyInfoACLTest_0001 End";
}

/**
 * @tc.name: DaemonStubHandleGetRemoteCopyInfoACLTest_0002
 * @tc.desc: Verify the HandleGetRemoteCopyInfoACL function (write and success cases)
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DaemonStubTest, DaemonStubHandleGetRemoteCopyInfoACLTest_0002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "DaemonStubHandleGetRemoteCopyInfoACLTest_0002 Start";
    MessageParcel data;
    MessageParcel reply;

    g_getCallingUidTrue = true;
    // Test isFile write failure
    EXPECT_CALL(*messageParcelMock_, ReadString(_))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*daemonStub_, GetRemoteCopyInfoACL(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, WriteBool(_)).WillOnce(Return(false));
    auto ret = daemonStub_->HandleGetRemoteCopyInfoACL(data, reply);
    EXPECT_EQ(ret, E_IPC_WRITE_FAILED);

    // Test isDir write failure
    EXPECT_CALL(*messageParcelMock_, ReadString(_))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*daemonStub_, GetRemoteCopyInfoACL(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, WriteBool(_)).WillOnce(Return(true)).WillOnce(Return(false));
    ret = daemonStub_->HandleGetRemoteCopyInfoACL(data, reply);
    EXPECT_EQ(ret, E_IPC_WRITE_FAILED);

    // Test success path
    EXPECT_CALL(*messageParcelMock_, ReadString(_))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*daemonStub_, GetRemoteCopyInfoACL(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, WriteBool(_)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    ret = daemonStub_->HandleGetRemoteCopyInfoACL(data, reply);
    EXPECT_EQ(ret, E_OK);

    // Test GetRemoteCopyInfoACL returns error
    EXPECT_CALL(*messageParcelMock_, ReadString(_))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*daemonStub_, GetRemoteCopyInfoACL(_, _, _, _)).WillOnce(Return(E_INVAL_ARG));
    EXPECT_CALL(*messageParcelMock_, WriteBool(_)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    ret = daemonStub_->HandleGetRemoteCopyInfoACL(data, reply);
    EXPECT_EQ(ret, E_INVAL_ARG);

    GTEST_LOG_(INFO) << "DaemonStubHandleGetRemoteCopyInfoACLTest_0002 End";
}

/**
 * @tc.name: DaemonStubHandlePushAssetTest001
 * @tc.desc: Verify the HandlePushAsset function - UID permission denied
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DaemonStubTest, DaemonStubHandlePushAssetTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "DaemonStubHandlePushAssetTest001 Start";
    MessageParcel data;
    MessageParcel reply;

    // Test case 1: UID is not DATA_UID
    UID = DATA_UID + 1;
    auto ret = daemonStub_->HandlePushAsset(data, reply);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);

    // Test case 2: UID is DATA_UID but permission check fails
    UID = DATA_UID;
    g_checkCallerPermissionTrue = false;
    ret = daemonStub_->HandlePushAsset(data, reply);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);
    GTEST_LOG_(INFO) << "DaemonStubHandlePushAssetTest001 End";
}

/**
 * @tc.name: DaemonStubHandlePushAssetTest002
 * @tc.desc: Verify the HandlePushAsset function - ReadInt32 failures
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DaemonStubTest, DaemonStubHandlePushAssetTest002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "DaemonStubHandlePushAssetTest002 Start";
    MessageParcel data;
    MessageParcel reply;

    UID = DATA_UID;
    g_checkCallerPermissionTrue = true;

    // Test case: ReadInt32 for userId fails
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(Return(false));
    auto ret = daemonStub_->HandlePushAsset(data, reply);
    EXPECT_EQ(ret, E_INVAL_ARG);
    GTEST_LOG_(INFO) << "DaemonStubHandlePushAssetTest002 End";
}

/**
 * @tc.name: DaemonStubHandlePushAssetTest003
 * @tc.desc: Verify the HandlePushAsset function - AssetObj read failures
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DaemonStubTest, DaemonStubHandlePushAssetTest003, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "DaemonStubHandlePushAssetTest003 Start";
    MessageParcel data;
    MessageParcel reply;

    UID = DATA_UID;
    g_checkCallerPermissionTrue = true;

    // Test case: ReadParcelable returns nullptr (AssetObj read failure)
    ASSERT_NE(messageParcelMock_, nullptr) << "DaemonStubHandlePushAssetTest003 messageParcelMock_ is nullptr";
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_))
        .WillOnce(DoAll(SetArgReferee<0>(-1), Return(true))); // -1: Invalid userId

    auto ret = daemonStub_->HandlePushAsset(data, reply);
    EXPECT_EQ(ret, E_INVAL_ARG);

    // Test case: ReadParcelable returns nullptr (AssetObj read failure)
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_))
        .WillOnce(DoAll(SetArgReferee<0>(100), Return(true))); // 100: Valid userId

    ret = daemonStub_->HandlePushAsset(data, reply);
    EXPECT_EQ(ret, E_INVAL_ARG);
    GTEST_LOG_(INFO) << "DaemonStubHandlePushAssetTest003 End";
}

/**
 * @tc.name: DaemonStubHandleRegisterRecvCallback001
 * @tc.desc: Verify the HandleRegisterRecvCallback function
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DaemonStubTest, DaemonStubHandleRegisterRecvCallback001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "DaemonStubHandleRegisterRecvCallback001 Start";
    UID = DATA_UID;
    MessageParcel data;
    MessageParcel reply;
    g_checkCallerPermissionTrue = false;
    auto ret = daemonStub_->HandleRegisterRecvCallback(data, reply);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);

    g_checkCallerPermissionTrue = true;
    EXPECT_CALL(*messageParcelMock_, ReadRemoteObject()).WillOnce(Return(nullptr));
    ret = daemonStub_->HandleRegisterRecvCallback(data, reply);
    EXPECT_EQ(ret, E_IPC_READ_FAILED);

    auto recvCallback = sptr(new IAssetRecvCallbackMock());
    EXPECT_CALL(*messageParcelMock_, ReadRemoteObject()).WillOnce(Return(recvCallback));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(false));
    EXPECT_CALL(*daemonStub_, RegisterAssetCallback(_)).WillOnce(Return(E_OK));
    ret = daemonStub_->HandleRegisterRecvCallback(data, reply);
    EXPECT_EQ(ret, E_IPC_READ_FAILED);

    EXPECT_CALL(*messageParcelMock_, ReadRemoteObject()).WillOnce(Return(recvCallback));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*daemonStub_, RegisterAssetCallback(_)).WillOnce(Return(E_OK));
    ret = daemonStub_->HandleRegisterRecvCallback(data, reply);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "DaemonStubHandleRegisterRecvCallback001 End";
}

/**
 * @tc.name: DaemonStubHandleUnRegisterRecvCallback001
 * @tc.desc: Verify the HandleUnRegisterRecvCallback function
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DaemonStubTest, DaemonStubHandleUnRegisterRecvCallback001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "DaemonStubHandleUnRegisterRecvCallback001 Start";
    UID = DATA_UID;
    MessageParcel data;
    MessageParcel reply;
    g_checkCallerPermissionTrue = false;
    auto ret = daemonStub_->HandleUnRegisterRecvCallback(data, reply);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);

    g_checkCallerPermissionTrue = true;
    EXPECT_CALL(*messageParcelMock_, ReadRemoteObject()).WillOnce(Return(nullptr));
    ret = daemonStub_->HandleUnRegisterRecvCallback(data, reply);
    EXPECT_EQ(ret, E_IPC_READ_FAILED);

    auto recvCallback = sptr(new IAssetRecvCallbackMock());
    EXPECT_CALL(*messageParcelMock_, ReadRemoteObject()).WillOnce(Return(recvCallback));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(false));
    EXPECT_CALL(*daemonStub_, UnRegisterAssetCallback(_)).WillOnce(Return(E_OK));
    ret = daemonStub_->HandleUnRegisterRecvCallback(data, reply);
    EXPECT_EQ(ret, E_IPC_READ_FAILED);

    EXPECT_CALL(*messageParcelMock_, ReadRemoteObject()).WillOnce(Return(recvCallback));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*daemonStub_, UnRegisterAssetCallback(_)).WillOnce(Return(E_OK));
    ret = daemonStub_->HandleUnRegisterRecvCallback(data, reply);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "DaemonStubHandleUnRegisterRecvCallback001 End";
}

/**
 * @tc.name: DaemonStubHandleGetDfsUrisDirFromLocalTest
 * @tc.desc: Verify URI list read failure in HandleGetDfsUrisDirFromLocal
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DaemonStubTest, DaemonStubHandleGetDfsUrisDirFromLocalTest, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "DaemonStubHandleGetDfsUrisDirFromLocalTest Start";
    MessageParcel data;
    MessageParcel reply;
    g_readBatchUris = E_IPC_READ_FAILED;
    auto ret = daemonStub_->HandleGetDfsUrisDirFromLocal(data, reply);
    EXPECT_EQ(ret, E_IPC_READ_FAILED);

    g_readBatchUris = E_OK;
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(Return(false));
    ret = daemonStub_->HandleGetDfsUrisDirFromLocal(data, reply);
    EXPECT_EQ(ret, E_IPC_READ_FAILED);

    std::unordered_map<std::string, AppFileService::ModuleRemoteFileShare::HmdfsUriInfo> uriToDfsUriMaps;
    uriToDfsUriMaps.emplace("key", AppFileService::ModuleRemoteFileShare::HmdfsUriInfo{"uri", 1}); // 1: test size
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*daemonStub_, GetDfsUrisDirFromLocal(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(uriToDfsUriMaps), Return(E_OK)));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(false));
    ret = daemonStub_->HandleGetDfsUrisDirFromLocal(data, reply);
    EXPECT_EQ(ret, E_IPC_WRITE_FAILED);

    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*daemonStub_, GetDfsUrisDirFromLocal(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(uriToDfsUriMaps), Return(E_IPC_WRITE_FAILED)));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    ret = daemonStub_->HandleGetDfsUrisDirFromLocal(data, reply);
    EXPECT_EQ(ret, E_OK);

    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*daemonStub_, GetDfsUrisDirFromLocal(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(uriToDfsUriMaps), Return(E_OK)));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    g_writeBatchUrisTrue = false;
    ret = daemonStub_->HandleGetDfsUrisDirFromLocal(data, reply);
    EXPECT_EQ(ret, E_IPC_WRITE_FAILED);

    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*daemonStub_, GetDfsUrisDirFromLocal(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(uriToDfsUriMaps), Return(E_OK)));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    g_writeBatchUrisTrue = true;
    ret = daemonStub_->HandleGetDfsUrisDirFromLocal(data, reply);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "DaemonStubHandleGetDfsUrisDirFromLocalTest End";
}

/**
 * @tc.name: DaemonStubHandleGetDfsSwitchStatus
 * @tc.desc: Verify the HandleGetDfsSwitchStatus function
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DaemonStubTest, DaemonStubHandleGetDfsSwitchStatus, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "DaemonStubHandleGetDfsSwitchStatus Start";
    MessageParcel data;
    MessageParcel reply;
    g_checkCallerPermissionTrue = false;
    auto ret = daemonStub_->HandleGetDfsSwitchStatus(data, reply);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);

    g_checkCallerPermissionTrue = true;
    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(false));
    ret = daemonStub_->HandleGetDfsSwitchStatus(data, reply);
    EXPECT_EQ(ret, E_IPC_READ_FAILED);

    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(false));
    EXPECT_CALL(*daemonStub_, GetDfsSwitchStatus(_, _)).WillOnce(Return(E_OK));
    ret = daemonStub_->HandleGetDfsSwitchStatus(data, reply);
    EXPECT_EQ(ret, E_IPC_WRITE_FAILED);

    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true)).WillOnce(Return(false));
    EXPECT_CALL(*daemonStub_, GetDfsSwitchStatus(_, _)).WillOnce(Return(E_INVAL_ARG));
    ret = daemonStub_->HandleGetDfsSwitchStatus(data, reply);
    EXPECT_EQ(ret, E_IPC_WRITE_FAILED);

    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*daemonStub_, GetDfsSwitchStatus(_, _)).WillOnce(Return(E_INVAL_ARG));
    ret = daemonStub_->HandleGetDfsSwitchStatus(data, reply);
    EXPECT_EQ(ret, E_OK);

    GTEST_LOG_(INFO) << "DaemonStubHandleGetDfsSwitchStatus End";
}

/**
 * @tc.name: DaemonStubHandleUpdateDfsSwitchStatus
 * @tc.desc: Verify the HandleUpdateDfsSwitchStatus function
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DaemonStubTest, DaemonStubHandleUpdateDfsSwitchStatus, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "DaemonStubHandleUpdateDfsSwitchStatus Start";
    MessageParcel data;
    MessageParcel reply;
    g_checkCallerPermissionTrue = false;
    auto ret = daemonStub_->HandleUpdateDfsSwitchStatus(data, reply);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);

    g_checkCallerPermissionTrue = true;
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(Return(false));
    ret = daemonStub_->HandleUpdateDfsSwitchStatus(data, reply);
    EXPECT_EQ(ret, E_IPC_READ_FAILED);

    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(false));
    EXPECT_CALL(*daemonStub_, UpdateDfsSwitchStatus(_)).WillOnce(Return(E_OK));
    ret = daemonStub_->HandleUpdateDfsSwitchStatus(data, reply);
    EXPECT_EQ(ret, E_IPC_WRITE_FAILED);

    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*daemonStub_, UpdateDfsSwitchStatus(_)).WillOnce(Return(E_OK));
    ret = daemonStub_->HandleUpdateDfsSwitchStatus(data, reply);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "DaemonStubHandleUpdateDfsSwitchStatus End";
}

/**
 * @tc.name: DaemonStubHandleGetConnectedDeviceList
 * @tc.desc: Verify the HandleGetConnectedDeviceList function
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DaemonStubTest, DaemonStubHandleGetConnectedDeviceList, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "DaemonStubHandleGetConnectedDeviceList Start";
    MessageParcel data;
    MessageParcel reply;
    g_checkCallerPermissionTrue = false;
    auto ret = daemonStub_->HandleGetConnectedDeviceList(data, reply);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);

    g_checkCallerPermissionTrue = true;
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(false));
    EXPECT_CALL(*daemonStub_, GetConnectedDeviceList(_)).WillOnce(Return(E_OK));
    ret = daemonStub_->HandleGetConnectedDeviceList(data, reply);
    EXPECT_EQ(ret, E_IPC_WRITE_FAILED);

    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true)).WillOnce(Return(false));
    EXPECT_CALL(*daemonStub_, GetConnectedDeviceList(_)).WillOnce(Return(E_OK));
    ret = daemonStub_->HandleGetConnectedDeviceList(data, reply);
    EXPECT_EQ(ret, E_IPC_WRITE_FAILED);

    DfsDeviceInfo testDevice;
    testDevice.networkId_ = "networkId";
    testDevice.path_ = "test_path";
    std::vector<DfsDeviceInfo> deviceList = {testDevice};
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(false));
    EXPECT_CALL(*daemonStub_, GetConnectedDeviceList(_)).WillOnce(DoAll(SetArgReferee<0>(deviceList), Return(E_OK)));
    ret = daemonStub_->HandleGetConnectedDeviceList(data, reply);
    EXPECT_EQ(ret, E_IPC_WRITE_FAILED);

    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true)).WillOnce(Return(false));
    EXPECT_CALL(*daemonStub_, GetConnectedDeviceList(_)).WillOnce(DoAll(SetArgReferee<0>(deviceList), Return(E_OK)));
    ret = daemonStub_->HandleGetConnectedDeviceList(data, reply);
    EXPECT_EQ(ret, E_IPC_WRITE_FAILED);

    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*daemonStub_, GetConnectedDeviceList(_)).WillOnce(DoAll(SetArgReferee<0>(deviceList), Return(E_OK)));
    ret = daemonStub_->HandleGetConnectedDeviceList(data, reply);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "DaemonStubHandleGetConnectedDeviceList End";
}

/**
 * @tc.name: DaemonStubHandleRegisterFileDfsListener
 * @tc.desc: Verify the HandleRegisterFileDfsListener function
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DaemonStubTest, DaemonStubHandleRegisterFileDfsListener, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "DaemonStubHandleRegisterFileDfsListener Start";
    MessageParcel data;
    MessageParcel reply;
    g_checkCallerPermissionTrue = false;
    auto ret = daemonStub_->HandleRegisterFileDfsListener(data, reply);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);

    g_checkCallerPermissionTrue = true;
    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(false));
    ret = daemonStub_->HandleRegisterFileDfsListener(data, reply);
    EXPECT_EQ(ret, E_IPC_READ_FAILED);

    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, ReadRemoteObject()).WillOnce(Return(nullptr));
    ret = daemonStub_->HandleRegisterFileDfsListener(data, reply);
    EXPECT_EQ(ret, E_IPC_READ_FAILED);

    auto listener = sptr(new FileDfsListenerMock());
    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return("instanceId"));
    EXPECT_CALL(*messageParcelMock_, ReadRemoteObject()).WillOnce(Return(listener));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(false));
    EXPECT_CALL(*daemonStub_, RegisterFileDfsListener(_, _)).WillOnce(Return(E_OK));
    ret = daemonStub_->HandleRegisterFileDfsListener(data, reply);
    EXPECT_EQ(ret, E_IPC_WRITE_FAILED);

    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return("instanceId"));
    EXPECT_CALL(*messageParcelMock_, ReadRemoteObject()).WillOnce(Return(listener));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*daemonStub_, RegisterFileDfsListener(_, _)).WillOnce(Return(E_OK));
    ret = daemonStub_->HandleRegisterFileDfsListener(data, reply);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "DaemonStubHandleRegisterFileDfsListener End";
}

/**
 * @tc.name: DaemonStubHandleUnregisterFileDfsListener
 * @tc.desc: Verify the HandleUnregisterFileDfsListener function
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DaemonStubTest, DaemonStubHandleUnregisterFileDfsListener, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "DaemonStubHandleUnregisterFileDfsListener Start";
    MessageParcel data;
    MessageParcel reply;
    g_checkCallerPermissionTrue = false;
    auto ret = daemonStub_->HandleUnregisterFileDfsListener(data, reply);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);

    g_checkCallerPermissionTrue = true;
    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(false));
    ret = daemonStub_->HandleUnregisterFileDfsListener(data, reply);
    EXPECT_EQ(ret, E_IPC_READ_FAILED);

    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(false));
    EXPECT_CALL(*daemonStub_, UnregisterFileDfsListener(_)).WillOnce(Return(E_OK));
    ret = daemonStub_->HandleUnregisterFileDfsListener(data, reply);
    EXPECT_EQ(ret, E_IPC_WRITE_FAILED);

    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*daemonStub_, UnregisterFileDfsListener(_)).WillOnce(Return(E_OK));
    ret = daemonStub_->HandleUnregisterFileDfsListener(data, reply);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "DaemonStubHandleUnregisterFileDfsListener End";
}

/**
 * @tc.name: DaemonStubHandleIsSameAccountDevice
 * @tc.desc: Verify the HandleIsSameAccountDevice function
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DaemonStubTest, DaemonStubHandleIsSameAccountDevice, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "DaemonStubHandleIsSameAccountDevice Start";
    MessageParcel data;
    MessageParcel reply;
    g_checkCallerPermissionTrue = false;
    auto ret = daemonStub_->HandleIsSameAccountDevice(data, reply);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);

    g_checkCallerPermissionTrue = true;
    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(false));
    ret = daemonStub_->HandleIsSameAccountDevice(data, reply);
    EXPECT_EQ(ret, E_IPC_READ_FAILED);

    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(false));
    EXPECT_CALL(*daemonStub_, IsSameAccountDevice(_, _)).WillOnce(Return(E_OK));
    ret = daemonStub_->HandleIsSameAccountDevice(data, reply);
    EXPECT_EQ(ret, E_IPC_WRITE_FAILED);

    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteBool(_)).WillOnce(Return(false));
    EXPECT_CALL(*daemonStub_, IsSameAccountDevice(_, _)).WillOnce(Return(E_OK));
    ret = daemonStub_->HandleIsSameAccountDevice(data, reply);
    EXPECT_EQ(ret, E_IPC_WRITE_FAILED);

    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteBool(_)).WillOnce(Return(true));
    EXPECT_CALL(*daemonStub_, IsSameAccountDevice(_, _)).WillOnce(DoAll(SetArgReferee<1>(true), Return(E_OK)));
    ret = daemonStub_->HandleIsSameAccountDevice(data, reply);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "DaemonStubHandleIsSameAccountDevice End";
}
} // namespace OHOS::Storage::DistributedFile::Test