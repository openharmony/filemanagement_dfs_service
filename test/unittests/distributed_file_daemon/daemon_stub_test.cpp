/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "dfs_error.h"
#include "dfsu_access_token_helper.h"
#include "ipc/daemon_stub.h"
#include "ipc/hmdfs_info.h"
#include "ipc/distributed_file_daemon_ipc_interface_code.h"
#include "i_daemon_mock.h"
#include "utils_log.h"

namespace {
    bool g_checkCallerPermissionTrue = true;
}
namespace OHOS::FileManagement {
bool DfsuAccessTokenHelper::CheckCallerPermission(const std::string &permissionName)
{
    if (g_checkCallerPermissionTrue) {
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

namespace {
DistributedHardware::DmDeviceInfo deviceInfo = {
    .deviceId = "testdevid",
    .deviceName = "testdevname",
    .networkId = "testnetworkid",
};
}

const std::string SRC_URI = "file://docs/storage/Users/currentUser/""Documents?networkid=xxxxx";
const std::string DST_URI = "file://docs/storage/Users/currentUser/Documents";
const std::string SRC_DEVICE_ID = "testSrcDeviceId";
const sptr<IRemoteObject> LISTENER = sptr(new DaemonServiceMock());
const std::string COPY_PATH = "tmpDir";
const std::string sessionName = "DistributedDevice0";

HmdfsInfo fileInfo = {
    .copyPath = COPY_PATH,
    .dirExistFlag = false,
    .sessionName = sessionName,
};

class MockDaemonStub : public DaemonStub {
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

    MOCK_METHOD3(PushAsset,
                 int32_t(int32_t userId,
                         const sptr<AssetObj> &assetObj,
                         const sptr<IAssetSendCallback> &sendCallback));
    MOCK_METHOD1(RegisterAssetCallback, int32_t(const sptr<IAssetRecvCallback> &recvCallback));
    MOCK_METHOD1(UnRegisterAssetCallback, int32_t(const sptr<IAssetRecvCallback> &recvCallback));
};

class DaemonStubTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<MockDaemonStub> daemonStub_;
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
}

void DaemonStubTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: DaemonStubOnRemoteRequestTest001
 * @tc.desc: Verify the OnRemoteRequest function
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DaemonStubTest, DaemonStubOnRemoteRequestTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonStubOnRemoteRequestTest001 Start";
    try {
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        int ret = daemonStub_->OnRemoteRequest(
            static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_OPEN_P2P_CONNECTION), data,
            reply, option);
        EXPECT_EQ(ret, IDaemon::DFS_DAEMON_DESCRIPTOR_IS_EMPTY);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DaemonStubOnRemoteRequestTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "DaemonStubOnRemoteRequestTest001 End";
}

/**
 * @tc.name: DaemonStubOnRemoteRequestTest002
 * @tc.desc: Verify the OnRemoteRequest function
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DaemonStubTest, DaemonStubOnRemoteRequestTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonStubOnRemoteRequestTest002 Start";
    try {
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        int ret = daemonStub_->OnRemoteRequest(
            static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_CLOSE_P2P_CONNECTION), data,
            reply, option);
        EXPECT_EQ(ret, IDaemon::DFS_DAEMON_DESCRIPTOR_IS_EMPTY);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DaemonStubOnRemoteRequestTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "DaemonStubOnRemoteRequestTest002 End";
}

/**
 * @tc.name: DaemonStubOnRemoteRequestTest003
 * @tc.desc: Verify the OnRemoteRequest function
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DaemonStubTest, DaemonStubOnRemoteRequestTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonStubOnRemoteRequestTest003 Start";
    try {
        int32_t code = 1000;
        MessageParcel data;
        EXPECT_TRUE(data.WriteInterfaceToken(IDaemon::GetDescriptor()));
        MessageParcel reply;
        MessageOption option;
        int ret = daemonStub_->OnRemoteRequest(code, data, reply, option);
        EXPECT_NE(ret, 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DaemonStubOnRemoteRequestTest003 ERROR";
    }
    GTEST_LOG_(INFO) << "DaemonStubOnRemoteRequestTest003 End";
}

/**
 * @tc.name: DaemonStubOnRemoteRequestTest004
 * @tc.desc: Verify the DaemonStubOnRemoteRequest function
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DaemonStubTest, DaemonStubOnRemoteRequestTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonStubOnRemoteRequestTest004 Start";
    try {
        MessageParcel data;
        EXPECT_TRUE(data.WriteInterfaceToken(IDaemon::GetDescriptor()));
        MessageParcel reply;
        MessageOption option;

        EXPECT_TRUE(data.WriteCString(deviceInfo.deviceId));
        EXPECT_TRUE(data.WriteCString(deviceInfo.deviceName));
        EXPECT_TRUE(data.WriteCString(deviceInfo.networkId));
        g_checkCallerPermissionTrue = true;
        EXPECT_CALL(*daemonStub_, OpenP2PConnection(_)).WillOnce(Return(0));
        int ret = daemonStub_->OnRemoteRequest(
            static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_OPEN_P2P_CONNECTION), data,
            reply, option);
        EXPECT_EQ(ret, 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DaemonStubOnRemoteRequestTest004 ERROR";
    }
    GTEST_LOG_(INFO) << "DaemonStubOnRemoteRequestTest004 End";
}

/**
 * @tc.name: DaemonStubOnRemoteRequestTest005
 * @tc.desc: Verify the DaemonStubOnRemoteRequest function
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DaemonStubTest, DaemonStubOnRemoteRequestTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonStubOnRemoteRequestTest005 Start";
    try {
        MessageParcel data;
        EXPECT_TRUE(data.WriteInterfaceToken(IDaemon::GetDescriptor()));
        MessageParcel reply;
        MessageOption option;

        EXPECT_TRUE(data.WriteCString(deviceInfo.deviceId));
        EXPECT_TRUE(data.WriteCString(deviceInfo.deviceName));
        EXPECT_TRUE(data.WriteCString(deviceInfo.networkId));
        g_checkCallerPermissionTrue = true;
        EXPECT_CALL(*daemonStub_, CloseP2PConnection(_)).WillOnce(Return(0));
        int ret = daemonStub_->OnRemoteRequest(
            static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_CLOSE_P2P_CONNECTION), data,
            reply, option);
        EXPECT_EQ(ret, 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DaemonStubOnRemoteRequestTest005 ERROR";
    }
    GTEST_LOG_(INFO) << "DaemonStubOnRemoteRequestTest005 End";
}

/**
 * @tc.name: DaemonStubOnRemoteRequestTest006
 * @tc.desc: Verify the DaemonStubOnRemoteRequest function
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DaemonStubTest, DaemonStubOnRemoteRequestTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonStubOnRemoteRequestTest006 Start";
    try {
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;

        EXPECT_TRUE(data.WriteString(SRC_URI));
        EXPECT_TRUE(data.WriteString(DST_URI));
        EXPECT_TRUE(data.WriteString(SRC_DEVICE_ID));
        EXPECT_TRUE(data.WriteRemoteObject(LISTENER));
        EXPECT_TRUE(data.WriteString(COPY_PATH));
        int ret = daemonStub_->OnRemoteRequest(
            static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_PREPARE_SESSION), data,
            reply, option);
        EXPECT_EQ(ret, IDaemon::DFS_DAEMON_DESCRIPTOR_IS_EMPTY);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DaemonStubOnRemoteRequestTest006 ERROR";
    }
    GTEST_LOG_(INFO) << "DaemonStubOnRemoteRequestTest006 End";
}

/**
 * @tc.name: DaemonStubOnRemoteRequestTest007
 * @tc.desc: Verify the DaemonStubOnRemoteRequest function
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DaemonStubTest, DaemonStubOnRemoteRequestTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonStubOnRemoteRequestTest007 Start";
    try {
        MessageParcel data;
        EXPECT_TRUE(data.WriteInterfaceToken(IDaemon::GetDescriptor()));
        MessageParcel reply;
        MessageOption option;

        int ret = daemonStub_->OnRemoteRequest(
            static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_REQUEST_SEND_FILE), data,
            reply, option);
        EXPECT_EQ(ret, E_PERMISSION_DENIED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DaemonStubOnRemoteRequestTest007 ERROR";
    }
    GTEST_LOG_(INFO) << "DaemonStubOnRemoteRequestTest007 End";
}

/**
 * @tc.name: DaemonStubOnRemoteRequestTest008
 * @tc.desc: Verify the DaemonStubOnRemoteRequest function
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DaemonStubTest, DaemonStubOnRemoteRequestTest008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonStubOnRemoteRequestTest008 Start";
    try {
        MessageParcel data;
        EXPECT_TRUE(data.WriteInterfaceToken(IDaemon::GetDescriptor()));
        MessageParcel reply;
        MessageOption option;

        int ret = daemonStub_->OnRemoteRequest(
            static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_GET_REMOTE_COPY_INFO), data,
            reply, option);
        EXPECT_EQ(ret, E_PERMISSION_DENIED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DaemonStubOnRemoteRequestTest008 ERROR";
    }
    GTEST_LOG_(INFO) << "DaemonStubOnRemoteRequestTest008 End";
}

/**
 * @tc.name: DaemonStubOnRemoteRequestTest009
 * @tc.desc: Verify the DaemonStubOnRemoteRequest function
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DaemonStubTest, DaemonStubOnRemoteRequestTest009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonStubOnRemoteRequestTest009 Start";
    try {
        MessageParcel data;
        EXPECT_TRUE(data.WriteInterfaceToken(IDaemon::GetDescriptor()));
        MessageParcel reply;
        MessageOption option;

        int ret = daemonStub_->OnRemoteRequest(
            static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_CANCEL_COPY_TASK), data,
            reply, option);
        EXPECT_EQ(ret, E_IPC_READ_FAILED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DaemonStubOnRemoteRequestTest009 ERROR";
    }
    GTEST_LOG_(INFO) << "DaemonStubOnRemoteRequestTest009 End";
}

/**
 * @tc.name: DaemonStubOnRemoteRequestTest010
 * @tc.desc: Verify the DaemonStubOnRemoteRequest function
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DaemonStubTest, DaemonStubOnRemoteRequestTest010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonStubOnRemoteRequestTest010 Start";
    try {
        MessageParcel data;
        EXPECT_TRUE(data.WriteInterfaceToken(IDaemon::GetDescriptor()));
        MessageParcel reply;
        MessageOption option;

        int ret = daemonStub_->OnRemoteRequest(
            static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_OPEN_P2P_CONNECTION_EX), data,
            reply, option);
        EXPECT_EQ(ret, E_IPC_READ_FAILED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DaemonStubOnRemoteRequestTest010 ERROR";
    }
    GTEST_LOG_(INFO) << "DaemonStubOnRemoteRequestTest010 End";
}

/**
 * @tc.name: DaemonStubOnRemoteRequestTest011
 * @tc.desc: Verify the DaemonStubOnRemoteRequest function
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DaemonStubTest, DaemonStubOnRemoteRequestTest011, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonStubOnRemoteRequestTest011 Start";
    try {
        MessageParcel data;
        EXPECT_TRUE(data.WriteInterfaceToken(IDaemon::GetDescriptor()));
        MessageParcel reply;
        MessageOption option;

        int ret = daemonStub_->OnRemoteRequest(
            static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_CLOSE_P2P_CONNECTION_EX), data,
            reply, option);
        EXPECT_EQ(ret, E_IPC_READ_FAILED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DaemonStubOnRemoteRequestTest011 ERROR";
    }
    GTEST_LOG_(INFO) << "DaemonStubOnRemoteRequestTest011 End";
}

/**
 * @tc.name: DaemonStubOnRemoteRequestTest012
 * @tc.desc: Verify the DaemonStubOnRemoteRequest function
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DaemonStubTest, DaemonStubOnRemoteRequestTest012, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonStubOnRemoteRequestTest012 Start";
    try {
        MessageParcel data;
        EXPECT_TRUE(data.WriteInterfaceToken(IDaemon::GetDescriptor()));
        MessageParcel reply;
        MessageOption option;

        int ret = daemonStub_->OnRemoteRequest(
            static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_REGISTER_ASSET_CALLBACK), data,
            reply, option);
        EXPECT_EQ(ret, E_IPC_READ_FAILED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DaemonStubOnRemoteRequestTest012 ERROR";
    }
    GTEST_LOG_(INFO) << "DaemonStubOnRemoteRequestTest012 End";
}

/**
 * @tc.name: DaemonStubOnRemoteRequestTest013
 * @tc.desc: Verify the DaemonStubOnRemoteRequest function
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DaemonStubTest, DaemonStubOnRemoteRequestTest013, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonStubOnRemoteRequestTest013 Start";
    try {
        MessageParcel data;
        EXPECT_TRUE(data.WriteInterfaceToken(IDaemon::GetDescriptor()));
        MessageParcel reply;
        MessageOption option;

        int ret = daemonStub_->OnRemoteRequest(
            static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_UN_REGISTER_ASSET_CALLBACK),
            data, reply, option);
        EXPECT_EQ(ret, E_IPC_READ_FAILED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DaemonStubOnRemoteRequestTest013 ERROR";
    }
    GTEST_LOG_(INFO) << "DaemonStubOnRemoteRequestTest013 End";
}

/**
 * @tc.name: DaemonStubOnRemoteRequestTest011
 * @tc.desc: Verify the DaemonStubOnRemoteRequest function
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DaemonStubTest, DaemonStubOnRemoteRequestTest014, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonStubOnRemoteRequestTest014 Start";
    try {
        MessageParcel data;
        EXPECT_TRUE(data.WriteInterfaceToken(IDaemon::GetDescriptor()));
        MessageParcel reply;
        MessageOption option;

        int ret = daemonStub_->OnRemoteRequest(
            static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_PUSH_ASSET), data,
            reply, option);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DaemonStubOnRemoteRequestTest014 ERROR";
    }
    GTEST_LOG_(INFO) << "DaemonStubOnRemoteRequestTest014 End";
}
} // namespace OHOS::Storage::DistributedFile::Test