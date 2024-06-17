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
#include "ipc/daemon_stub.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ipc_skeleton.h"

#include "dfs_error.h"
#include "dfsu_access_token_helper.h"
#include "ipc/hmdfs_info.h"
#include "ipc/distributed_file_daemon_ipc_interface_code.h"
#include "i_daemon_mock.h"
#include "message_parcel_mock.h"
#include "utils_log.h"

namespace {
    bool g_getCallingUidTrue = true;
    bool g_checkUriPermissionTrue = true;
    const pid_t UID = 1009;
}

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
}

namespace OHOS::FileManagement {
bool DfsuAccessTokenHelper::CheckUriPermission(const std::string &uriStr)
{
    if (g_checkUriPermissionTrue) {
        return true;
    }

    return false;
}
}

namespace OHOS::Storage::DistributedFile::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using namespace OHOS::FileManagement;

class MockDaemonSupStub : public DaemonStub {
public:
    MOCK_METHOD1(OpenP2PConnection, int32_t(const DistributedHardware::DmDeviceInfo &deviceInfo));
    MOCK_METHOD1(CloseP2PConnection, int32_t(const DistributedHardware::DmDeviceInfo &deviceInfo));
    MOCK_METHOD2(OpenP2PConnectionEx, int32_t(const std::string &networkId, sptr<IFileDfsListener> remoteReverseObj));
    MOCK_METHOD1(CloseP2PConnectionEx, int32_t(const std::string &networkId));
    MOCK_METHOD4(RequestSendFile,
                 int32_t(const std::string &srcUri,
                         const std::string &dstPath,
                         const std::string &remoteDeviceId,
                         const std::string &sessionName));
    MOCK_METHOD5(PrepareSession,
                 int32_t(const std::string &srcUri,
                         const std::string &dstUri,
                         const std::string &srcDeviceId,
                         const sptr<IRemoteObject> &listener,
                         HmdfsInfo &fileInfo));
    MOCK_METHOD1(CancelCopyTask, int32_t(const std::string &sessionName));
    MOCK_METHOD3(GetRemoteCopyInfo, int32_t(const std::string &srcUri, bool &isFile, bool &isDir));
    MOCK_METHOD3(PushAsset, int32_t(int32_t userId,
                                    const sptr<AssetObj> &assetObj,
                                    const sptr<IAssetSendCallback> &sendCallback));
    MOCK_METHOD1(RegisterAssetCallback, int32_t(const sptr<IAssetRecvCallback> &recvCallback));
    MOCK_METHOD1(UnRegisterAssetCallback, int32_t(const sptr<IAssetRecvCallback> &recvCallback));
};

class DaemonStubSupPTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<MockDaemonSupStub> daemonStub_;
    static inline shared_ptr<MessageParcelMock> messageParcelMock_ = nullptr;
};

void DaemonStubSupPTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    daemonStub_ = std::make_shared<MockDaemonSupStub>();
    messageParcelMock_ = make_shared<MessageParcelMock>();
    MessageParcelMock::messageParcel = messageParcelMock_;
}

void DaemonStubSupPTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void DaemonStubSupPTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void DaemonStubSupPTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: DaemonStubSupOnRemoteRequestTest001
 * @tc.desc: Verify the OnRemoteRequest function
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DaemonStubSupPTest, DaemonStubSupOnRemoteRequestTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonStubSupOnRemoteRequestTest001 Start";
    uint32_t code = static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_OPEN_P2P_CONNECTION);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    EXPECT_CALL(*messageParcelMock_, ReadInterfaceToken()).WillOnce(Return(u"test"));
    auto ret = daemonStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, MockDaemonSupStub::DFS_DAEMON_DESCRIPTOR_IS_EMPTY);

    EXPECT_CALL(*messageParcelMock_, ReadInterfaceToken()).WillOnce(Return(IDaemon::GetDescriptor()));
    uint32_t errCode = 65535;
    ret = daemonStub_->OnRemoteRequest(errCode, data, reply, option);
    EXPECT_EQ(ret, 305);

    daemonStub_->opToInterfaceMap_.insert(make_pair(errCode, nullptr));
    EXPECT_CALL(*messageParcelMock_, ReadInterfaceToken()).WillOnce(Return(IDaemon::GetDescriptor()));
    ret = daemonStub_->OnRemoteRequest(errCode, data, reply, option);
    EXPECT_EQ(ret, 305);
    daemonStub_->opToInterfaceMap_.erase(errCode);
    GTEST_LOG_(INFO) << "DaemonStubSupOnRemoteRequestTest001 End";
}

/**
 * @tc.name: DaemonStubSupHandleCloseP2PConnectionExTest001
 * @tc.desc: Verify the HandleCloseP2PConnectionEx function
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DaemonStubSupPTest, DaemonStubSupHandleCloseP2PConnectionExTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonStubSupHandleCancelCopyTaskTest001 Start";
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
    GTEST_LOG_(INFO) << "DaemonStubSupHandleCloseP2PConnectionExTest001 End";
}

/**
 * @tc.name: DaemonStubSupHandlePrepareSessionTest001
 * @tc.desc: Verify the HandlePrepareSession function
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DaemonStubSupPTest, DaemonStubSupHandlePrepareSessionTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonStubSupHandlePrepareSessionTest001 Start";
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

    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(true)).WillOnce(Return(true))
        .WillOnce(Return(false));
    ret = daemonStub_->HandlePrepareSession(data, reply);
    EXPECT_EQ(ret, E_IPC_READ_FAILED);
    
    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(true)).WillOnce(Return(true))
        .WillOnce(Return(false));
    EXPECT_CALL(*messageParcelMock_, ReadRemoteObject()).WillOnce(Return(nullptr));
    ret = daemonStub_->HandlePrepareSession(data, reply);
    EXPECT_EQ(ret, E_IPC_READ_FAILED);

    sptr<IRemoteObject> listenerPtr = sptr(new DaemonServiceMock());
    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(true)).WillOnce(Return(true))
        .WillOnce(Return(true)).WillOnce(Return(false));
    EXPECT_CALL(*messageParcelMock_, ReadRemoteObject()).WillOnce(Return(listenerPtr));
    ret = daemonStub_->HandlePrepareSession(data, reply);
    EXPECT_EQ(ret, E_IPC_READ_FAILED);
    GTEST_LOG_(INFO) << "DaemonStubSupHandlePrepareSessionTest001 End";
}

/**
 * @tc.name: DaemonStubSupHandlePrepareSessionTest002
 * @tc.desc: Verify the HandlePrepareSession function
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DaemonStubSupPTest, DaemonStubSupHandlePrepareSessionTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonStubSupHandlePrepareSessionTest002 Start";
    MessageParcel data;
    MessageParcel reply;
    g_checkUriPermissionTrue = true;
    sptr<IRemoteObject> listenerPtr = sptr(new DaemonServiceMock());
    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(true)).WillOnce(Return(true))
        .WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, ReadRemoteObject()).WillOnce(Return(listenerPtr));
    EXPECT_CALL(*messageParcelMock_, ReadBool(_)).WillOnce(Return(false));
    auto ret = daemonStub_->HandlePrepareSession(data, reply);
    EXPECT_EQ(ret, E_IPC_READ_FAILED);

    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(true)).WillOnce(Return(true))
        .WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, ReadRemoteObject()).WillOnce(Return(listenerPtr));
    EXPECT_CALL(*messageParcelMock_, ReadBool(_)).WillOnce(Return(true));
    EXPECT_CALL(*daemonStub_, PrepareSession(_, _, _, _, _)).WillOnce(Return(E_INVAL_ARG));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(false));
    ret = daemonStub_->HandlePrepareSession(data, reply);
    EXPECT_EQ(ret, E_IPC_WRITE_FAILED);

    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(true)).WillOnce(Return(true))
        .WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, ReadRemoteObject()).WillOnce(Return(listenerPtr));
    EXPECT_CALL(*messageParcelMock_, ReadBool(_)).WillOnce(Return(true));
    EXPECT_CALL(*daemonStub_, PrepareSession(_, _, _, _, _)).WillOnce(Return(E_INVAL_ARG));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    ret = daemonStub_->HandlePrepareSession(data, reply);
    EXPECT_EQ(ret, E_INVAL_ARG);
    GTEST_LOG_(INFO) << "DaemonStubSupHandlePrepareSessionTest002 End";
}

/**
 * @tc.name: DaemonStubSupHandleRequestSendFileTest001
 * @tc.desc: Verify the HandleRequestSendFile function
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DaemonStubSupPTest, DaemonStubSupHandleRequestSendFileTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonStubSupHandleRequestSendFileTest001 Start";
    MessageParcel data;
    MessageParcel reply;

    g_getCallingUidTrue = false;
    auto ret = daemonStub_->HandleRequestSendFile(data, reply);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);
    
    g_getCallingUidTrue = true;
    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(false));
    ret = daemonStub_->HandleRequestSendFile(data, reply);
    EXPECT_EQ(ret, E_IPC_READ_FAILED);

    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(true)).WillOnce(Return(false));
    ret = daemonStub_->HandleRequestSendFile(data, reply);
    EXPECT_EQ(ret, E_IPC_READ_FAILED);

    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(true)).WillOnce(Return(true))
        .WillOnce(Return(false));
    ret = daemonStub_->HandleRequestSendFile(data, reply);
    EXPECT_EQ(ret, E_IPC_READ_FAILED);

    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(true)).WillOnce(Return(true))
        .WillOnce(Return(true)).WillOnce(Return(false));
    ret = daemonStub_->HandleRequestSendFile(data, reply);
    EXPECT_EQ(ret, E_IPC_READ_FAILED);

    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(true)).WillOnce(Return(true))
        .WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*daemonStub_, RequestSendFile(_, _, _, _)).WillOnce(Return(E_INVAL_ARG));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    ret = daemonStub_->HandleRequestSendFile(data, reply);
    EXPECT_EQ(ret, E_INVAL_ARG);
    GTEST_LOG_(INFO) << "DaemonStubSupHandleRequestSendFileTest001 End";
}

/**
 * @tc.name: DaemonStubSupHandleGetRemoteCopyInfoTest001
 * @tc.desc: Verify the HandleGetRemoteCopyInfo function
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DaemonStubSupPTest, DaemonStubSupHandleGetRemoteCopyInfoTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonStubSupHandleGetRemoteCopyInfoTest001 Start";
    MessageParcel data;
    MessageParcel reply;
    g_getCallingUidTrue = false;
    auto ret = daemonStub_->HandleGetRemoteCopyInfo(data, reply);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);
    
    g_getCallingUidTrue = true;
    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(false));
    ret = daemonStub_->HandleGetRemoteCopyInfo(data, reply);
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
    GTEST_LOG_(INFO) << "DaemonStubSupHandleGetRemoteCopyInfoTest001 End";
}

/**
 * @tc.name: DaemonStubSupHandleGetRemoteCopyInfoTest002
 * @tc.desc: Verify the HandleGetRemoteCopyInfo function
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DaemonStubSupPTest, DaemonStubSupHandleGetRemoteCopyInfoTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonStubSupHandleGetRemoteCopyInfoTest002 Start";
    MessageParcel data;
    MessageParcel reply;
    
    g_getCallingUidTrue = true;
    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(true));
    EXPECT_CALL(*daemonStub_, GetRemoteCopyInfo(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, WriteBool(_)).WillOnce(Return(true)).WillOnce(Return(true))
        .WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    auto ret = daemonStub_->HandleGetRemoteCopyInfo(data, reply);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "DaemonStubSupHandleGetRemoteCopyInfoTest002 End";
}

/**
 * @tc.name: DaemonStubSupHandleCancelCopyTaskTest001
 * @tc.desc: Verify the HandleCancelCopyTask function
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DaemonStubSupPTest, DaemonStubSupHandleCancelCopyTaskTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonStubSupHandleCancelCopyTaskTest001 Start";
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(false));
    auto ret = daemonStub_->HandleCancelCopyTask(data, reply);
    EXPECT_EQ(ret, E_IPC_READ_FAILED);

    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(true));
    EXPECT_CALL(*daemonStub_, CancelCopyTask(_)).WillOnce(Return(E_OK));
    ret = daemonStub_->HandleCancelCopyTask(data, reply);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "DaemonStubSupHandleCancelCopyTaskTest001 End";
}
} // namespace OHOS::Storage::DistributedFile::Test