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

#include "dfs_error.h"
#include "file_dfs_listener_mock.h"
#include "message_parcel_mock.h"
#include "i_daemon_mock.h"

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
    mock_ = sptr(new DaemonServiceMock());
    proxy_ = make_shared<DistributedFileDaemonProxy>(mock_);
    messageParcelMock_ = make_shared<MessageParcelMock>();
    MessageParcelMock::messageParcel = messageParcelMock_;
}

void DistributedFileDaemonProxyTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    mock_ = nullptr;
    proxy_ = nullptr;
    MessageParcelMock::messageParcel = nullptr;
    messageParcelMock_ = nullptr;
}

void DistributedFileDaemonProxyTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void DistributedFileDaemonProxyTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
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
}