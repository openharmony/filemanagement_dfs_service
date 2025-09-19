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
#include "distributed_file_daemon_proxy.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "asset_recv_callback_mock.h"
#include "asset_send_callback_mock.h"
#include "dfs_error.h"
#include "file_dfs_listener_mock.h"
#include "message_parcel_mock.h"
#include "i_daemon_mock.h"
#include "copy/ipc_wrapper.h"

namespace {
    bool g_writeBatchUrisTrue = true;
    int32_t g_readBatchUris = OHOS::FileManagement::E_OK;
    std::string g_context = "1";
}
namespace OHOS::Storage::DistributedFile {
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
        uriVec.emplace_back(g_context);
    }
    return g_readBatchUris;
}
};

namespace OHOS::Storage::DistributedFile::Test {
using namespace OHOS::FileManagement;
using namespace testing;
using namespace testing::ext;
using namespace std;

class DistributedFileDaemonProxyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
public:
    static inline shared_ptr<DistributedFileDaemonProxy> proxy_ = nullptr;
    static inline sptr<DaemonServiceMock> mock_ = nullptr;
    static inline shared_ptr<MessageParcelMock> messageParcelMock_ = nullptr;
};

void DistributedFileDaemonProxyTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void DistributedFileDaemonProxyTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void DistributedFileDaemonProxyTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
    mock_ = sptr(new DaemonServiceMock());
    proxy_ = make_shared<DistributedFileDaemonProxy>(mock_);
    messageParcelMock_ = make_shared<MessageParcelMock>();
    MessageParcelMock::messageParcel = messageParcelMock_;
}

void DistributedFileDaemonProxyTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
    mock_ = nullptr;
    proxy_ = nullptr;
    messageParcelMock_ = nullptr;
    MessageParcelMock::messageParcel = nullptr;
}

/**
 * @tc.name: DistributedFileDaemon_OpenP2PConnection_0100
 * @tc.desc: The execution of the OpenP2PConnection failed.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DistributedFileDaemonProxyTest, DistributedFileDaemon_OpenP2PConnection_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DistributedFileDaemon_OpenP2PConnection_0100 Start";
    DistributedHardware::DmDeviceInfo deviceInfo;
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(false));
    auto ret = proxy_->OpenP2PConnection(deviceInfo);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteCString(_)).WillOnce(Return(false));
    ret = proxy_->OpenP2PConnection(deviceInfo);
    EXPECT_EQ(ret, E_INVAL_ARG);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteCString(_)).WillOnce(Return(true))
        .WillOnce(Return(false));
    ret = proxy_->OpenP2PConnection(deviceInfo);
    EXPECT_EQ(ret, E_INVAL_ARG);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteCString(_)).WillOnce(Return(true))
        .WillOnce(Return(true)).WillOnce(Return(false));
    ret = proxy_->OpenP2PConnection(deviceInfo);
    EXPECT_EQ(ret, E_INVAL_ARG);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteCString(_)).WillOnce(Return(true))
        .WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteUint16(_)).WillOnce(Return(false));
    ret = proxy_->OpenP2PConnection(deviceInfo);
    EXPECT_EQ(ret, E_INVAL_ARG);
    GTEST_LOG_(INFO) << "DistributedFileDaemon_OpenP2PConnection_0100 End";
}

/**
 * @tc.name: DistributedFileDaemon_CloseP2PConnection_0100
 * @tc.desc: The execution of the CloseP2PConnection failed.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DistributedFileDaemonProxyTest, DistributedFileDaemon_CloseP2PConnection_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DistributedFileDaemon_CloseP2PConnection_0100 Start";
    DistributedHardware::DmDeviceInfo deviceInfo;
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(false));
    auto ret = proxy_->OpenP2PConnection(deviceInfo);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteCString(_)).WillOnce(Return(false));
    ret = proxy_->CloseP2PConnection(deviceInfo);
    EXPECT_EQ(ret, E_INVAL_ARG);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteCString(_)).WillOnce(Return(true))
        .WillOnce(Return(false));
    ret = proxy_->CloseP2PConnection(deviceInfo);
    EXPECT_EQ(ret, E_INVAL_ARG);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteCString(_)).WillOnce(Return(true))
        .WillOnce(Return(true)).WillOnce(Return(false));
    ret = proxy_->CloseP2PConnection(deviceInfo);
    EXPECT_EQ(ret, E_INVAL_ARG);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteCString(_)).WillOnce(Return(true))
        .WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteUint16(_)).WillOnce(Return(false));
    ret = proxy_->CloseP2PConnection(deviceInfo);
    EXPECT_EQ(ret, E_INVAL_ARG);
    GTEST_LOG_(INFO) << "DistributedFileDaemon_CloseP2PConnection_0100 End";
}

/**
 * @tc.name: DistributedFileDaemon_OpenP2PConnectionEx_0100
 * @tc.desc: verify OpenP2PConnectionEx
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DistributedFileDaemonProxyTest, DistributedFileDaemon_OpenP2PConnectionEx_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DistributedFileDaemon_OpenP2PConnectionEx_0100 Start";
    auto remoteReverseObj = sptr(new FileDfsListenerMock());
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(false));
    auto ret = proxy_->OpenP2PConnectionEx("test", remoteReverseObj);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(false));
    ret = proxy_->OpenP2PConnectionEx("test", remoteReverseObj);
    EXPECT_EQ(ret, E_INVAL_ARG);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteRemoteObject(An<const sptr<IRemoteObject>&>()))
        .WillOnce(Return(false));
    ret = proxy_->OpenP2PConnectionEx("test", remoteReverseObj);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteRemoteObject(An<const sptr<IRemoteObject>&>()))
        .WillOnce(Return(true));
    auto testProxy = make_shared<DistributedFileDaemonProxy>(nullptr);
    ret = testProxy->OpenP2PConnectionEx("test", remoteReverseObj);
    EXPECT_EQ(ret, E_BROKEN_IPC);
    GTEST_LOG_(INFO) << "DistributedFileDaemon_OpenP2PConnectionEx_0100 End";
}

/**
 * @tc.name: DistributedFileDaemon_OpenP2PConnectionEx_0200
 * @tc.desc: verify OpenP2PConnectionEx
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DistributedFileDaemonProxyTest, DistributedFileDaemon_OpenP2PConnectionEx_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DistributedFileDaemon_OpenP2PConnectionEx_0200 Start";
    auto remoteReverseObj = sptr(new FileDfsListenerMock());
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteRemoteObject(An<const sptr<IRemoteObject>&>()))
        .WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_INVAL_ARG));
    auto ret = proxy_->OpenP2PConnectionEx("test", remoteReverseObj);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteRemoteObject(An<const sptr<IRemoteObject>&>()))
        .WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, ReadInt32()).WillOnce(Return(E_INVAL_ARG));
    ret = proxy_->OpenP2PConnectionEx("test", remoteReverseObj);
    EXPECT_EQ(ret, E_INVAL_ARG);

    sptr<FileDfsListenerMock> errPtr = nullptr;
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, ReadInt32()).WillOnce(Return(E_OK));
    ret = proxy_->OpenP2PConnectionEx("test", errPtr);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "DistributedFileDaemon_OpenP2PConnectionEx_0200 End";
}

/**
 * @tc.name: DistributedFileDaemon_CloseP2PConnectionEx_0100
 * @tc.desc: verify CloseP2PConnectionEx
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DistributedFileDaemonProxyTest, DistributedFileDaemon_CloseP2PConnectionEx_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DistributedFileDaemon_CloseP2PConnectionEx_0100 Start";
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(false));
    auto ret = proxy_->CloseP2PConnectionEx("test");
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(false));
    ret = proxy_->CloseP2PConnectionEx("test");
    EXPECT_EQ(ret, E_INVAL_ARG);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    auto testProxy = make_shared<DistributedFileDaemonProxy>(nullptr);
    ret = testProxy->CloseP2PConnectionEx("test");
    EXPECT_EQ(ret, E_BROKEN_IPC);
    GTEST_LOG_(INFO) << "DistributedFileDaemon_CloseP2PConnectionEx_0100 End";
}

/**
 * @tc.name: DistributedFileDaemon_CloseP2PConnectionEx_0200
 * @tc.desc: verify CloseP2PConnectionEx
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DistributedFileDaemonProxyTest, DistributedFileDaemon_CloseP2PConnectionEx_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DistributedFileDaemon_CloseP2PConnectionEx_0200 Start";
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_INVAL_ARG));
    auto ret = proxy_->CloseP2PConnectionEx("test");
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, ReadInt32()).WillOnce(Return(E_INVAL_ARG));
    ret = proxy_->CloseP2PConnectionEx("test");
    EXPECT_EQ(ret, E_INVAL_ARG);
    GTEST_LOG_(INFO) << "DistributedFileDaemon_CloseP2PConnectionEx_0200 End";
}

/**
 * @tc.name: DistributedFileDaemon_PrepareSession_0100
 * @tc.desc: verify PrepareSession
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DistributedFileDaemonProxyTest, DistributedFileDaemon_PrepareSession_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DistributedFileDaemon_PrepareSession_0100 Start";
    auto remoteReverseObj = sptr(new FileDfsListenerMock());
    HmdfsInfo fileInfo;
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(false));
    auto ret = proxy_->PrepareSession("srcUri", "dstUri", "deviceId",
        remoteReverseObj->AsObject(), fileInfo);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(false));
    ret = proxy_->PrepareSession("srcUri", "dstUri", "deviceId",
        remoteReverseObj->AsObject(), fileInfo);
    EXPECT_EQ(ret, E_INVAL_ARG);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true)).WillOnce(Return(false));
    ret = proxy_->PrepareSession("srcUri", "dstUri", "deviceId",
        remoteReverseObj->AsObject(), fileInfo);
    EXPECT_EQ(ret, E_INVAL_ARG);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true)).WillOnce(Return(true))
        .WillOnce(Return(false));
    ret = proxy_->PrepareSession("srcUri", "dstUri", "deviceId",
        remoteReverseObj->AsObject(), fileInfo);
    EXPECT_EQ(ret, E_INVAL_ARG);
    GTEST_LOG_(INFO) << "DistributedFileDaemon_PrepareSession_0100 End";
}

/**
 * @tc.name: DistributedFileDaemon_PrepareSession_0200
 * @tc.desc: verify PrepareSession
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DistributedFileDaemonProxyTest, DistributedFileDaemon_PrepareSession_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DistributedFileDaemon_PrepareSession_0200 Start";
    auto remoteReverseObj = sptr(new FileDfsListenerMock());
    HmdfsInfo fileInfo;
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true)).WillOnce(Return(true))
        .WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteRemoteObject(An<const sptr<IRemoteObject>&>()))
        .WillOnce(Return(false));
    auto ret = proxy_->PrepareSession("srcUri", "dstUri", "deviceId",
        remoteReverseObj->AsObject(), fileInfo);
    EXPECT_EQ(ret, E_INVAL_ARG);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true)).WillOnce(Return(true))
        .WillOnce(Return(true)).WillOnce(Return(false));
    EXPECT_CALL(*messageParcelMock_, WriteRemoteObject(An<const sptr<IRemoteObject>&>()))
        .WillOnce(Return(true));
    ret = proxy_->PrepareSession("srcUri", "dstUri", "deviceId",
        remoteReverseObj->AsObject(), fileInfo);
    EXPECT_EQ(ret, E_INVAL_ARG);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true)).WillOnce(Return(true))
        .WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteRemoteObject(An<const sptr<IRemoteObject>&>()))
        .WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteBool(_)).WillOnce(Return(false));
    ret = proxy_->PrepareSession("srcUri", "dstUri", "deviceId",
        remoteReverseObj->AsObject(), fileInfo);
    EXPECT_EQ(ret, E_INVAL_ARG);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true)).WillOnce(Return(true))
        .WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteRemoteObject(An<const sptr<IRemoteObject>&>()))
        .WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteBool(_)).WillOnce(Return(true));
    auto testProxy = make_shared<DistributedFileDaemonProxy>(nullptr);
    ret = testProxy->PrepareSession("srcUri", "dstUri", "deviceId",
        remoteReverseObj->AsObject(), fileInfo);
    EXPECT_EQ(ret, E_BROKEN_IPC);
    GTEST_LOG_(INFO) << "DistributedFileDaemon_PrepareSession_0200 End";
}

/**
 * @tc.name: DistributedFileDaemon_PrepareSession_0300
 * @tc.desc: verify PrepareSession
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DistributedFileDaemonProxyTest, DistributedFileDaemon_PrepareSession_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DistributedFileDaemon_PrepareSession_0300 Start";
    auto remoteReverseObj = sptr(new FileDfsListenerMock());
    HmdfsInfo fileInfo;
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true)).WillOnce(Return(true))
        .WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteRemoteObject(An<const sptr<IRemoteObject>&>()))
        .WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteBool(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_INVAL_ARG));
    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(false));
    auto ret = proxy_->PrepareSession("srcUri", "dstUri", "deviceId",
        remoteReverseObj->AsObject(), fileInfo);
    EXPECT_EQ(ret, E_INVAL_ARG);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true)).WillOnce(Return(true))
        .WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteRemoteObject(An<const sptr<IRemoteObject>&>()))
        .WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteBool(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_INVAL_ARG));
    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(true));
    ret = proxy_->PrepareSession("srcUri", "dstUri", "deviceId",
        remoteReverseObj->AsObject(), fileInfo);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true)).WillOnce(Return(true))
        .WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteRemoteObject(An<const sptr<IRemoteObject>&>()))
        .WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteBool(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, ReadInt32()).WillOnce(Return(E_INVAL_ARG));
    ret = proxy_->PrepareSession("srcUri", "dstUri", "deviceId",
        remoteReverseObj->AsObject(), fileInfo);
    EXPECT_EQ(ret, E_INVAL_ARG);
    GTEST_LOG_(INFO) << "DistributedFileDaemon_PrepareSession_0300 End";
}

/**
 * @tc.name: DistributedFileDaemon_RequestSendFile_0100
 * @tc.desc: verify RequestSendFile.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DistributedFileDaemonProxyTest, DistributedFileDaemon_RequestSendFile_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DistributedFileDaemon_RequestSendFile_0100 Start";
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(false));
    auto ret = proxy_->RequestSendFile("uri", "path", "deviceId", "test");
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(false));
    ret = proxy_->RequestSendFile("uri", "path", "deviceId", "test");
    EXPECT_EQ(ret, E_INVAL_ARG);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true)).WillOnce(Return(false));
    ret = proxy_->RequestSendFile("uri", "path", "deviceId", "test");
    EXPECT_EQ(ret, E_INVAL_ARG);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true)).WillOnce(Return(true))
        .WillOnce(Return(false));
    ret = proxy_->RequestSendFile("uri", "path", "deviceId", "test");
    EXPECT_EQ(ret, E_INVAL_ARG);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true)).WillOnce(Return(true))
        .WillOnce(Return(true)).WillOnce(Return(false));
    ret = proxy_->RequestSendFile("uri", "path", "deviceId", "test");
    EXPECT_EQ(ret, E_INVAL_ARG);
    GTEST_LOG_(INFO) << "DistributedFileDaemon_RequestSendFile_0100 End";
}

/**
 * @tc.name: DistributedFileDaemon_RequestSendFile_0200
 * @tc.desc: verify RequestSendFile.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DistributedFileDaemonProxyTest, DistributedFileDaemon_RequestSendFile_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DistributedFileDaemon_RequestSendFile_0200 Start";
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true)).WillOnce(Return(true))
        .WillOnce(Return(true)).WillOnce(Return(true));

    auto testProxy = make_shared<DistributedFileDaemonProxy>(nullptr);
    auto ret = testProxy->RequestSendFile("uri", "path", "deviceId", "test");
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true)).WillOnce(Return(true))
        .WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_INVAL_ARG));
    ret = proxy_->RequestSendFile("uri", "path", "deviceId", "test");
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true)).WillOnce(Return(true))
        .WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, ReadInt32()).WillOnce(Return(E_INVAL_ARG));
    ret = proxy_->RequestSendFile("uri", "path", "deviceId", "test");
    EXPECT_EQ(ret, E_INVAL_ARG);
    GTEST_LOG_(INFO) << "DistributedFileDaemon_RequestSendFile_0200 End";
}

/**
 * @tc.name: DistributedFileDaemon_GetRemoteCopyInfo_0100
 * @tc.desc: verify GetRemoteCopyInfo.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DistributedFileDaemonProxyTest, DistributedFileDaemon_GetRemoteCopyInfo_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DistributedFileDaemon_GetRemoteCopyInfo_0100 Start";
    bool isFile = false;
    bool isDir = false;
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(false));
    auto ret = proxy_->GetRemoteCopyInfo("test", isFile, isDir);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(false));
    ret = proxy_->GetRemoteCopyInfo("test", isFile, isDir);
    EXPECT_EQ(ret, E_INVAL_ARG);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    auto testProxy = make_shared<DistributedFileDaemonProxy>(nullptr);
    ret = testProxy->GetRemoteCopyInfo("test", isFile, isDir);
    EXPECT_EQ(ret, E_BROKEN_IPC);
    GTEST_LOG_(INFO) << "DistributedFileDaemon_GetRemoteCopyInfo_0100 End";
}

/**
 * @tc.name: DistributedFileDaemon_GetRemoteCopyInfo_0200
 * @tc.desc: verify GetRemoteCopyInfo.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DistributedFileDaemonProxyTest, DistributedFileDaemon_GetRemoteCopyInfo_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DistributedFileDaemon_GetRemoteCopyInfo_0200 Start";
    bool isFile = false;
    bool isDir = false;
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_INVAL_ARG));
    auto ret = proxy_->GetRemoteCopyInfo("test", isFile, isDir);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, ReadBool(_)).WillOnce(Return(false));
    ret = proxy_->GetRemoteCopyInfo("test", isFile, isDir);
    EXPECT_EQ(ret, E_INVAL_ARG);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, ReadBool(_)).WillOnce(Return(true)).WillOnce(Return(false));
    ret = proxy_->GetRemoteCopyInfo("test", isFile, isDir);
    EXPECT_EQ(ret, E_INVAL_ARG);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, ReadBool(_)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(Return(false));
    ret = proxy_->GetRemoteCopyInfo("test", isFile, isDir);
    EXPECT_EQ(ret, E_INVAL_ARG);
    GTEST_LOG_(INFO) << "DistributedFileDaemon_GetRemoteCopyInfo_0200 End";
}

/**
 * @tc.name: DistributedFileDaemon_GetRemoteCopyInfo_0300
 * @tc.desc: The execution of the GetRemoteCopyInfo success.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DistributedFileDaemonProxyTest, DistributedFileDaemon_GetRemoteCopyInfo_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DistributedFileDaemon_GetRemoteCopyInfo_0300 Start";
    bool isFile = false;
    bool isDir = false;
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, ReadBool(_)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_))
        .WillOnce(DoAll(SetArgReferee<0>(E_OK), Return(true)));
    auto ret = proxy_->GetRemoteCopyInfo("test", isFile, isDir);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "DistributedFileDaemon_GetRemoteCopyInfo_0300 End";
}

/**
 * @tc.name: DistributedFileDaemon_CancelCopyTask_0100
 * @tc.desc: verify CancelCopyTask.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DistributedFileDaemonProxyTest, DistributedFileDaemon_CancelCopyTask_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DistributedFileDaemon_CancelCopyTask_0100 Start";
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(false));
    auto ret = proxy_->CancelCopyTask("test");
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(false));
    ret = proxy_->CancelCopyTask("test");
    EXPECT_EQ(ret, E_INVAL_ARG);
    
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    auto testProxy = make_shared<DistributedFileDaemonProxy>(nullptr);
    ret = testProxy->CancelCopyTask("test");
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_INVAL_ARG));
    ret = proxy_->CancelCopyTask("test");
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, ReadInt32()).WillOnce(Return(E_INVAL_ARG));
    ret = proxy_->CancelCopyTask("test");
    EXPECT_EQ(ret, E_INVAL_ARG);
    GTEST_LOG_(INFO) << "DistributedFileDaemon_CancelCopyTask_0100 End";
}

/**
 * @tc.name: DistributedFileDaemon_PushAsset_0100
 * @tc.desc: verify PushAsset.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DistributedFileDaemonProxyTest, DistributedFileDaemon_PushAsset_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DistributedFileDaemon_PushAsset_0100 Start";
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(false));
    sptr<AssetObj> assetObj(new (std::nothrow) AssetObj());
    auto callbackMock = sptr(new IAssetSendCallbackMock());
    auto ret = proxy_->PushAsset(100, assetObj, callbackMock);
    EXPECT_EQ(ret, E_BROKEN_IPC);
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(false));
    ret = proxy_->PushAsset(100, assetObj, callbackMock);
    EXPECT_EQ(ret, E_INVAL_ARG);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteParcelable(_)).WillOnce(Return(false));
    ret = proxy_->PushAsset(100, assetObj, callbackMock);
    EXPECT_EQ(ret, E_INVAL_ARG);

    sptr<IAssetSendCallbackMock> errPtr = nullptr;
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteParcelable(_)).WillOnce(Return(true));
    ret = proxy_->PushAsset(100, assetObj, errPtr);
    EXPECT_EQ(ret, E_INVAL_ARG);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteParcelable(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteRemoteObject(An<const sptr<IRemoteObject>&>()))
        .WillOnce(Return(false));
    ret = proxy_->PushAsset(100, assetObj, callbackMock);
    EXPECT_EQ(ret, E_INVAL_ARG);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteParcelable(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteRemoteObject(An<const sptr<IRemoteObject>&>()))
        .WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_INVAL_ARG));
    ret = proxy_->PushAsset(100, assetObj, callbackMock);
    EXPECT_EQ(ret, E_BROKEN_IPC);
    GTEST_LOG_(INFO) << "DistributedFileDaemon_PushAsset_0100 End";
}

/**
 * @tc.name: DistributedFileDaemon_PushAsset_0200
 * @tc.desc: verify PushAsset.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DistributedFileDaemonProxyTest, DistributedFileDaemon_PushAsset_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DistributedFileDaemon_PushAsset_0200 Start";
    auto assetObj (new (std::nothrow) AssetObj());
    auto callbackmock = sptr(new IAssetSendCallbackMock());

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteParcelable(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteRemoteObject(An<const sptr<IRemoteObject>&>()))
        .WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, ReadInt32()).WillOnce(Return(E_OK));
    auto ret = proxy_->PushAsset(100, assetObj, callbackmock);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "DistributedFileDaemon_PushAsset_0200 End";
}

/**
 * @tc.name: DistributedFileDaemon_PushAsset_0300
 * @tc.desc: verify PushAsset.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DistributedFileDaemonProxyTest, DistributedFileDaemon_PushAsset_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DistributedFileDaemon_PushAsset_0300 Start";
    auto assetObj (new (std::nothrow) AssetObj());
    auto callbackmock = sptr(new IAssetSendCallbackMock());

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteParcelable(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteRemoteObject(An<const sptr<IRemoteObject>&>()))
        .WillOnce(Return(true));
    auto testProxy = make_shared<DistributedFileDaemonProxy>(nullptr);
    auto ret = testProxy->PushAsset(100, assetObj, callbackmock);
    EXPECT_EQ(ret, E_BROKEN_IPC);
    GTEST_LOG_(INFO) << "DistributedFileDaemon_PushAsset_0300 End";
}

/**
 * @tc.name: DistributedFileDaemon_RegisterAssetCallback_0100
 * @tc.desc: verify RegisterAssetCallback.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DistributedFileDaemonProxyTest, DistributedFileDaemon_RegisterAssetCallback_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DistributedFileDaemon_RegisterAssetCallback_0100 Start";
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(false));
    auto recvCallback = sptr(new IAssetRecvCallbackMock());
    auto ret = proxy_->RegisterAssetCallback(recvCallback);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    sptr<IAssetRecvCallbackMock> errCallBack = nullptr;
    ret = proxy_->RegisterAssetCallback(errCallBack);
    EXPECT_EQ(ret, E_INVAL_ARG);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteRemoteObject(An<const sptr<IRemoteObject>&>()))
        .WillOnce(Return(false));
    ret = proxy_->RegisterAssetCallback(recvCallback);
    EXPECT_EQ(ret, E_INVAL_ARG);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteRemoteObject(An<const sptr<IRemoteObject>&>()))
        .WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_INVAL_ARG));
    ret = proxy_->RegisterAssetCallback(recvCallback);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteRemoteObject(An<const sptr<IRemoteObject>&>()))
        .WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, ReadInt32()).WillOnce(Return(E_OK));
    ret = proxy_->RegisterAssetCallback(recvCallback);
    EXPECT_EQ(ret, E_OK);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteRemoteObject(An<const sptr<IRemoteObject>&>()))
        .WillOnce(Return(true));
    auto testProxy = make_shared<DistributedFileDaemonProxy>(nullptr);
    ret = testProxy->RegisterAssetCallback(recvCallback);
    EXPECT_EQ(ret, E_BROKEN_IPC);
    GTEST_LOG_(INFO) << "DistributedFileDaemon_RegisterAssetCallback_0100 End";
}


/**
 * @tc.name: DistributedFileDaemon_UnRegisterAssetCallback_0100
 * @tc.desc: verify UnRegisterAssetCallback.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DistributedFileDaemonProxyTest, DistributedFileDaemon_UnRegisterAssetCallback_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DistributedFileDaemon_UnRegisterAssetCallback_0100 Start";
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(false));
    auto recvCallback = sptr(new IAssetRecvCallbackMock());
    auto ret = proxy_->UnRegisterAssetCallback(recvCallback);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    sptr<IAssetRecvCallbackMock> errCallBack = nullptr;
    ret = proxy_->UnRegisterAssetCallback(errCallBack);
    EXPECT_EQ(ret, E_INVAL_ARG);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteRemoteObject(An<const sptr<IRemoteObject>&>()))
        .WillOnce(Return(false));
    ret = proxy_->UnRegisterAssetCallback(recvCallback);
    EXPECT_EQ(ret, E_INVAL_ARG);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteRemoteObject(An<const sptr<IRemoteObject>&>()))
        .WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_INVAL_ARG));
    ret = proxy_->UnRegisterAssetCallback(recvCallback);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteRemoteObject(An<const sptr<IRemoteObject>&>()))
        .WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, ReadInt32()).WillOnce(Return(E_OK));
    ret = proxy_->UnRegisterAssetCallback(recvCallback);
    EXPECT_EQ(ret, E_OK);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteRemoteObject(An<const sptr<IRemoteObject>&>()))
        .WillOnce(Return(true));
    auto testProxy = make_shared<DistributedFileDaemonProxy>(nullptr);
    ret = testProxy->UnRegisterAssetCallback(recvCallback);
    EXPECT_EQ(ret, E_BROKEN_IPC);
    GTEST_LOG_(INFO) << "DistributedFileDaemon_UnRegisterAssetCallback_0100 End";
}

/**
 * @tc.name: DistributedFileDaemon_GetDfsUrisDirFromLocal_0100
 * @tc.desc: The execution of the GetDfsUrisDirFromLocal failed and success.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DistributedFileDaemonProxyTest, DistributedFileDaemon_GetDfsUrisDirFromLocal_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DistributedFileDaemon_GetDfsUrisDirFromLocal_0100 Start";
    std::vector<std::string> uriList = {""};
    int32_t userId = 101;
    AppFileService::ModuleRemoteFileShare::HmdfsUriInfo uriInfo{"test", 2};
    std::unordered_map<std::string, AppFileService::ModuleRemoteFileShare::HmdfsUriInfo> uriToDfsUriMaps;
    uriToDfsUriMaps.insert({"test", uriInfo});
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(false));
    auto ret = proxy_->GetDfsUrisDirFromLocal(uriList, userId, uriToDfsUriMaps);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    g_writeBatchUrisTrue = false;
    ret = proxy_->GetDfsUrisDirFromLocal(uriList, userId, uriToDfsUriMaps);
    EXPECT_EQ(ret, E_INVAL_ARG);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    g_writeBatchUrisTrue = true;
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(false));
    ret = proxy_->GetDfsUrisDirFromLocal(uriList, userId, uriToDfsUriMaps);
    EXPECT_EQ(ret, E_INVAL_ARG);

    auto testProxy = make_shared<DistributedFileDaemonProxy>(nullptr);
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    ret = testProxy->GetDfsUrisDirFromLocal(uriList, userId, uriToDfsUriMaps);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_INVAL_ARG));
    ret = proxy_->GetDfsUrisDirFromLocal(uriList, userId, uriToDfsUriMaps);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(Return(false));
    ret = proxy_->GetDfsUrisDirFromLocal(uriList, userId, uriToDfsUriMaps);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(DoAll(SetArgReferee<0>(E_INVAL_ARG), Return(true)));
    ret = proxy_->GetDfsUrisDirFromLocal(uriList, userId, uriToDfsUriMaps);
    EXPECT_EQ(ret, E_INVAL_ARG);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(DoAll(SetArgReferee<0>(E_OK), Return(true)));
    ret = proxy_->GetDfsUrisDirFromLocal(uriList, userId, uriToDfsUriMaps);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "DistributedFileDaemon_GetDfsUrisDirFromLocal_0100 End";
}

/**
 * @tc.name: DistributedFileDaemon_GetDfsUrisDirFromLocal_0200
 * @tc.desc: The execution of the GetDfsUrisDirFromLocal failed.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DistributedFileDaemonProxyTest, DistributedFileDaemon_GetDfsUrisDirFromLocal_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DistributedFileDaemon_GetDfsUrisDirFromLocal_0200 Start";
    std::vector<std::string> uriList = {""};
    int32_t userId = 101;
    AppFileService::ModuleRemoteFileShare::HmdfsUriInfo uriInfo{"test", 2};
    std::unordered_map<std::string, AppFileService::ModuleRemoteFileShare::HmdfsUriInfo> uriToDfsUriMaps;
    uriToDfsUriMaps.insert({"test", uriInfo});

    g_context = "aa";
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(DoAll(SetArgReferee<0>(E_OK), Return(true)));
    auto ret = proxy_->GetDfsUrisDirFromLocal(uriList, userId, uriToDfsUriMaps);
    EXPECT_EQ(ret, E_INVAL_ARG);

    g_context = "1";
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(DoAll(SetArgReferee<0>(E_OK), Return(true)));
    g_readBatchUris = E_INVAL_ARG;
    ret = proxy_->GetDfsUrisDirFromLocal(uriList, userId, uriToDfsUriMaps);
    EXPECT_EQ(ret, E_IPC_READ_FAILED);
    GTEST_LOG_(INFO) << "DistributedFileDaemon_GetDfsUrisDirFromLocal_0200 End";
}

/**
 * @tc.name: DistributedFileDaemon_GetDfsSwitchStatus
 * @tc.desc: Verify GetDfsSwitchStatus function
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DistributedFileDaemonProxyTest, DistributedFileDaemon_GetDfsSwitchStatus, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DistributedFileDaemon_GetDfsSwitchStatus Start";
    int32_t switchStatus = 0;
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(false));
    auto ret = proxy_->GetDfsSwitchStatus("networkId", switchStatus);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(false));
    ret = proxy_->GetDfsSwitchStatus("networkId", switchStatus);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    auto tmpProxy = make_shared<DistributedFileDaemonProxy>(nullptr);
    ret = tmpProxy->GetDfsSwitchStatus("networkId", switchStatus);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_INVAL_ARG));
    ret = proxy_->GetDfsSwitchStatus("networkId", switchStatus);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(Return(false));
    ret = proxy_->GetDfsSwitchStatus("networkId", switchStatus);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(DoAll(SetArgReferee<0>(E_INVAL_ARG), Return(true)));
    ret = proxy_->GetDfsSwitchStatus("networkId", switchStatus);
    EXPECT_EQ(ret, E_INVAL_ARG);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(DoAll(SetArgReferee<0>(E_OK), Return(true)))
        .WillOnce(Return(false));
    ret = proxy_->GetDfsSwitchStatus("networkId", switchStatus);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(DoAll(SetArgReferee<0>(E_OK), Return(true)))
        .WillOnce(DoAll(SetArgReferee<0>(1), Return(true)));
    ret = proxy_->GetDfsSwitchStatus("networkId", switchStatus);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(switchStatus, 1);
    GTEST_LOG_(INFO) << "DistributedFileDaemon_GetDfsSwitchStatus End";
}

/**
 * @tc.name: DistributedFileDaemon_UpdateDfsSwitchStatus
 * @tc.desc: Verify UpdateDfsSwitchStatus function
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DistributedFileDaemonProxyTest, DistributedFileDaemon_UpdateDfsSwitchStatus, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DistributedFileDaemon_UpdateDfsSwitchStatus Start";
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(false));
    auto ret = proxy_->UpdateDfsSwitchStatus(1);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(false));
    ret = proxy_->UpdateDfsSwitchStatus(1);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    auto tmpProxy = make_shared<DistributedFileDaemonProxy>(nullptr);
    ret = tmpProxy->UpdateDfsSwitchStatus(1);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_INVAL_ARG));
    ret = proxy_->UpdateDfsSwitchStatus(1);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(Return(false));
    ret = proxy_->UpdateDfsSwitchStatus(1);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(DoAll(SetArgReferee<0>(E_OK), Return(true)));
    ret = proxy_->UpdateDfsSwitchStatus(1);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "DistributedFileDaemon_UpdateDfsSwitchStatus End";
}

/**
 * @tc.name: DistributedFileDaemon_GetConnectedDeviceList
 * @tc.desc: Verify GetConnectedDeviceList function
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DistributedFileDaemonProxyTest, DistributedFileDaemon_GetConnectedDeviceList, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DistributedFileDaemon_GetConnectedDeviceList Start";
    std::vector<DfsDeviceInfo> deviceList;
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(false));
    auto ret = proxy_->GetConnectedDeviceList(deviceList);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_INVAL_ARG));
    ret = proxy_->GetConnectedDeviceList(deviceList);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(Return(false));
    ret = proxy_->GetConnectedDeviceList(deviceList);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(DoAll(SetArgReferee<0>(E_INVAL_ARG), Return(true)));
    ret = proxy_->GetConnectedDeviceList(deviceList);
    EXPECT_EQ(ret, E_INVAL_ARG);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(DoAll(SetArgReferee<0>(E_OK), Return(true)))
        .WillOnce(Return(false));
    ret = proxy_->GetConnectedDeviceList(deviceList);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(DoAll(SetArgReferee<0>(E_OK), Return(true)))
        .WillOnce(DoAll(SetArgReferee<0>(1), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(false));
    ret = proxy_->GetConnectedDeviceList(deviceList);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(DoAll(SetArgReferee<0>(E_OK), Return(true)))
        .WillOnce(DoAll(SetArgReferee<0>(1), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(true)).WillOnce(Return(false));
    ret = proxy_->GetConnectedDeviceList(deviceList);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(DoAll(SetArgReferee<0>(E_OK), Return(true)))
        .WillOnce(DoAll(SetArgReferee<0>(1), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(true)).WillOnce(Return(true));
    ret = proxy_->GetConnectedDeviceList(deviceList);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "DistributedFileDaemon_GetConnectedDeviceList End";
}

/**
 * @tc.name: DistributedFileDaemon_RegisterFileDfsListener
 * @tc.desc: Verify RegisterFileDfsListener function
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DistributedFileDaemonProxyTest, DistributedFileDaemon_RegisterFileDfsListener, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DistributedFileDaemon_RegisterFileDfsListener Start";
    auto listener = sptr(new FileDfsListenerMock());
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(false));
    auto ret = proxy_->RegisterFileDfsListener("instance", listener);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(false));
    ret = proxy_->RegisterFileDfsListener("instance", listener);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    ret = proxy_->RegisterFileDfsListener("instance", nullptr);
    EXPECT_EQ(ret, E_INVAL_ARG);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteRemoteObject(An<const sptr<IRemoteObject>&>()))
        .WillOnce(Return(false));
    ret = proxy_->RegisterFileDfsListener("instance", listener);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteRemoteObject(An<const sptr<IRemoteObject>&>()))
        .WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_INVAL_ARG));
    ret = proxy_->RegisterFileDfsListener("instance", listener);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteRemoteObject(An<const sptr<IRemoteObject>&>()))
        .WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(Return(false));
    ret = proxy_->RegisterFileDfsListener("instance", listener);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteRemoteObject(An<const sptr<IRemoteObject>&>()))
        .WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(DoAll(SetArgReferee<0>(E_OK), Return(true)));
    ret = proxy_->RegisterFileDfsListener("instance", listener);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "DistributedFileDaemon_RegisterFileDfsListener End";
}

/**
 * @tc.name: DistributedFileDaemon_UnregisterFileDfsListener
 * @tc.desc: Verify UnregisterFileDfsListener function
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DistributedFileDaemonProxyTest, DistributedFileDaemon_UnregisterFileDfsListener, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DistributedFileDaemon_UnregisterFileDfsListener Start";
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(false));
    auto ret = proxy_->UnregisterFileDfsListener("instance");
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(false));
    ret = proxy_->UnregisterFileDfsListener("instance");
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    auto tmpProxy = make_shared<DistributedFileDaemonProxy>(nullptr);
    ret = tmpProxy->UnregisterFileDfsListener("instance");
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_INVAL_ARG));
    ret = proxy_->UnregisterFileDfsListener("instance");
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(Return(false));
    ret = proxy_->UnregisterFileDfsListener("instance");
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(DoAll(SetArgReferee<0>(E_OK), Return(true)));
    ret = proxy_->UnregisterFileDfsListener("instance");
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "DistributedFileDaemon_UnregisterFileDfsListener End";
}

/**
 * @tc.name: DistributedFileDaemon_IsSameAccountDevice
 * @tc.desc: Verify IsSameAccountDevice function
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DistributedFileDaemonProxyTest, DistributedFileDaemon_IsSameAccountDevice, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DistributedFileDaemon_IsSameAccountDevice Start";
    bool isSameAccount = false;
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(false));
    auto ret = proxy_->IsSameAccountDevice("networkId", isSameAccount);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(false));
    ret = proxy_->IsSameAccountDevice("networkId", isSameAccount);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    auto tmpProxy = make_shared<DistributedFileDaemonProxy>(nullptr);
    ret = tmpProxy->IsSameAccountDevice("networkId", isSameAccount);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_INVAL_ARG));
    ret = proxy_->IsSameAccountDevice("networkId", isSameAccount);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(Return(false));
    ret = proxy_->IsSameAccountDevice("networkId", isSameAccount);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(DoAll(SetArgReferee<0>(E_INVAL_ARG), Return(true)));
    ret = proxy_->IsSameAccountDevice("networkId", isSameAccount);
    EXPECT_EQ(ret, E_INVAL_ARG);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(DoAll(SetArgReferee<0>(E_OK), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadBool(_)).WillOnce(Return(false));
    ret = proxy_->IsSameAccountDevice("networkId", isSameAccount);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(DoAll(SetArgReferee<0>(E_OK), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadBool(_)).WillOnce(Return(true));
    ret = proxy_->IsSameAccountDevice("networkId", isSameAccount);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "DistributedFileDaemon_IsSameAccountDevice End";
}

/**
 * @tc.name: DistributedFileDaemon_RequestSendFileACL_0100
 * @tc.desc: Verify RequestSendFileACL for parameter serialization failures.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DistributedFileDaemonProxyTest, DistributedFileDaemon_RequestSendFileACL_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DistributedFileDaemon_RequestSendFileACL_0100 Start";
    AccountInfo accountInfo{123, 0, "account_id", "network_id"};

    // Test WriteInterfaceToken failure
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).Times(1).WillOnce(Return(false));
    auto ret = proxy_->RequestSendFileACL("uri", "path", "deviceId", "test", accountInfo);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    // Test WriteString(srcUri) failure
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).Times(1).WillOnce(Return(false));
    ret = proxy_->RequestSendFileACL("uri", "path", "deviceId", "test", accountInfo);
    EXPECT_EQ(ret, E_INVAL_ARG);

    // Test WriteString(dstPath) failure
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).Times(2).WillOnce(Return(true)).WillOnce(Return(false));
    ret = proxy_->RequestSendFileACL("uri", "path", "deviceId", "test", accountInfo);
    EXPECT_EQ(ret, E_INVAL_ARG);

    // Test WriteString(remoteDeviceId) failure
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).Times(3).WillOnce(Return(true)).WillOnce(Return(true))
        .WillOnce(Return(false));
    ret = proxy_->RequestSendFileACL("uri", "path", "deviceId", "test", accountInfo);
    EXPECT_EQ(ret, E_INVAL_ARG);

    // Test WriteString(sessionName) failure
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).Times(4).WillOnce(Return(true)).WillOnce(Return(true))
        .WillOnce(Return(true)).WillOnce(Return(false));
    ret = proxy_->RequestSendFileACL("uri", "path", "deviceId", "test", accountInfo);
    EXPECT_EQ(ret, E_INVAL_ARG);

    // Test WriteInt32(userId_) failure
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).Times(4).WillRepeatedly(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).Times(1).WillOnce(Return(false));
    ret = proxy_->RequestSendFileACL("uri", "path", "deviceId", "test", accountInfo);
    EXPECT_EQ(ret, E_INVAL_ARG);

    // Test WriteString(accountId_) failure
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).Times(5).WillOnce(Return(true)).WillOnce(Return(true))
        .WillOnce(Return(true)).WillOnce(Return(true)).WillOnce(Return(false));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).Times(1).WillOnce(Return(true));
    ret = proxy_->RequestSendFileACL("uri", "path", "deviceId", "test", accountInfo);
    EXPECT_EQ(ret, E_INVAL_ARG);

    // Test WriteString(networkId_) failure
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).Times(6).WillOnce(Return(true)).WillOnce(Return(true))
        .WillOnce(Return(true)).WillOnce(Return(true)).WillOnce(Return(true)).WillOnce(Return(false));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).Times(1).WillOnce(Return(true));
    ret = proxy_->RequestSendFileACL("uri", "path", "deviceId", "test", accountInfo);
    EXPECT_EQ(ret, E_INVAL_ARG);

    GTEST_LOG_(INFO) << "DistributedFileDaemon_RequestSendFileACL_0100 End";
}

/**
 * @tc.name: DistributedFileDaemon_RequestSendFileACL_0200
 * @tc.desc: Verify RequestSendFileACL for remote object, IPC, and reply read failures, and success case.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DistributedFileDaemonProxyTest, DistributedFileDaemon_RequestSendFileACL_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DistributedFileDaemon_RequestSendFileACL_0200 Start";
    AccountInfo accountInfo{123, 0, "account_id", "network_id"};

    // Test remote object null
    auto testProxy = make_shared<DistributedFileDaemonProxy>(nullptr);
    auto ret = testProxy->RequestSendFileACL("uri", "path", "deviceId", "test", accountInfo);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    // Test SendRequest failure
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).Times(6).WillRepeatedly(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(E_INVAL_ARG));
    ret = proxy_->RequestSendFileACL("uri", "path", "deviceId", "test", accountInfo);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    // Test success case
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).Times(6).WillRepeatedly(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, ReadInt32()).Times(1).WillOnce(Return(E_OK));
    ret = proxy_->RequestSendFileACL("uri", "path", "deviceId", "test", accountInfo);
    EXPECT_EQ(ret, E_OK);

    GTEST_LOG_(INFO) << "DistributedFileDaemon_RequestSendFileACL_0200 End";
}

/**
 * @tc.name: DistributedFileDaemon_GetRemoteCopyInfoACL_0100
 * @tc.desc: Verify GetRemoteCopyInfoACL for parameter serialization failures.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DistributedFileDaemonProxyTest, DistributedFileDaemon_GetRemoteCopyInfoACL_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DistributedFileDaemon_GetRemoteCopyInfoACL_0100 Start";
    AccountInfo accountInfo{123, 0, "account_id", "network_id"};
    bool isFile = false;
    bool isDir = false;

    // Test WriteInterfaceToken failure
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).Times(1).WillOnce(Return(false));
    auto ret = proxy_->GetRemoteCopyInfoACL("uri", isFile, isDir, accountInfo);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    // Test WriteString(srcUri) failure
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).Times(1).WillOnce(Return(false));
    ret = proxy_->GetRemoteCopyInfoACL("uri", isFile, isDir, accountInfo);
    EXPECT_EQ(ret, E_INVAL_ARG);

    // Test WriteInt32(userId_) failure
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).Times(1).WillOnce(Return(false));
    ret = proxy_->GetRemoteCopyInfoACL("uri", isFile, isDir, accountInfo);
    EXPECT_EQ(ret, E_INVAL_ARG);

    // Test WriteString(accountId_) failure
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).Times(2).WillOnce(Return(true)).WillOnce(Return(false));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).Times(1).WillOnce(Return(true));
    ret = proxy_->GetRemoteCopyInfoACL("uri", isFile, isDir, accountInfo);
    EXPECT_EQ(ret, E_INVAL_ARG);

    // Test WriteString(networkId_) failure
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).Times(3).WillOnce(Return(true)).WillOnce(Return(true))
        .WillOnce(Return(false));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).Times(1).WillOnce(Return(true));
    ret = proxy_->GetRemoteCopyInfoACL("uri", isFile, isDir, accountInfo);
    EXPECT_EQ(ret, E_INVAL_ARG);

    GTEST_LOG_(INFO) << "DistributedFileDaemon_GetRemoteCopyInfoACL_0100 End";
}

/**
 * @tc.name: DistributedFileDaemon_GetRemoteCopyInfoACL_0200
 * @tc.desc: Verify GetRemoteCopyInfoACL for remote object, IPC, and reply read failures, and success case.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DistributedFileDaemonProxyTest, DistributedFileDaemon_GetRemoteCopyInfoACL_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DistributedFileDaemon_GetRemoteCopyInfoACL_0200 Start";
    AccountInfo accountInfo{123, 0, "account_id", "network_id"};
    bool isFile = false;
    bool isDir = false;

    // Test remote object null
    auto testProxy = make_shared<DistributedFileDaemonProxy>(nullptr);
    auto ret = testProxy->GetRemoteCopyInfoACL("uri", isFile, isDir, accountInfo);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    // Test SendRequest failure
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).Times(3).WillRepeatedly(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(E_INVAL_ARG));
    ret = proxy_->GetRemoteCopyInfoACL("uri", isFile, isDir, accountInfo);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    // Test ReadBool(isFile) failure
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).Times(3).WillRepeatedly(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, ReadBool(_)).Times(1).WillOnce(Return(false));
    ret = proxy_->GetRemoteCopyInfoACL("uri", isFile, isDir, accountInfo);
    EXPECT_EQ(ret, E_INVAL_ARG);

    // Test ReadBool(isDir) failure
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).Times(3).WillRepeatedly(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, ReadBool(_)).Times(2).WillOnce(Return(true)).WillOnce(Return(false));
    ret = proxy_->GetRemoteCopyInfoACL("uri", isFile, isDir, accountInfo);
    EXPECT_EQ(ret, E_INVAL_ARG);

    // Test ReadInt32 failure
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).Times(3).WillRepeatedly(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, ReadBool(_)).Times(2).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).Times(1).WillOnce(Return(false));
    ret = proxy_->GetRemoteCopyInfoACL("uri", isFile, isDir, accountInfo);
    EXPECT_EQ(ret, E_INVAL_ARG);

    // Test success case
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).Times(3).WillRepeatedly(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, ReadBool(_)).Times(2).WillOnce(DoAll(SetArgReferee<0>(true), Return(true)))
        .WillOnce(DoAll(SetArgReferee<0>(false), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).Times(1).WillOnce(Return(true));
    ret = proxy_->GetRemoteCopyInfoACL("uri", isFile, isDir, accountInfo);
    EXPECT_EQ(ret, E_OK);
    EXPECT_TRUE(isFile);
    EXPECT_FALSE(isDir);

    GTEST_LOG_(INFO) << "DistributedFileDaemon_GetRemoteCopyInfoACL_0200 End";
}
} // namespace OHOS::Storage::DistributedFile::Test
