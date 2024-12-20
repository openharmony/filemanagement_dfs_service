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

#include <fcntl.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/stat.h>

#include "asset_recv_callback_mock.h"
#include "asset_send_callback_mock.h"
#include "dfs_error.h"
#include "distributed_file_daemon_manager_impl.h"
#include "distributed_file_daemon_proxy.h"
#include "ipc/hmdfs_info.h"
#include "i_daemon_mock.h"
#include "file_dfs_listener_mock.h"
#include "utils_log.h"

namespace OHOS::Storage::DistributedFile {
sptr<IDaemon> DistributedFileDaemonProxy::GetInstance()
{
    daemonProxy_ = iface_cast<IDaemon>(sptr(new DaemonServiceMock()));
    return daemonProxy_;
}
namespace Test {
using namespace OHOS::FileManagement;
using namespace testing;
using namespace testing::ext;
using namespace std;

namespace {
DistributedHardware::DmDeviceInfo deviceInfo = {
    .deviceId = "testdevid",
    .deviceName = "testdevname",
    .networkId = "testnetworkid",
};
}

class DistributedDaemonManagerImplTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<DistributedFileDaemonManagerImpl> distributedDaemonManagerImpl_;
};

void DistributedDaemonManagerImplTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void DistributedDaemonManagerImplTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void DistributedDaemonManagerImplTest::SetUp(void)
{
    if (distributedDaemonManagerImpl_ == nullptr) {
        distributedDaemonManagerImpl_ = std::make_shared<DistributedFileDaemonManagerImpl>();
        ASSERT_TRUE(distributedDaemonManagerImpl_ != nullptr) << "CallbackManager failed";
    }
    GTEST_LOG_(INFO) << "SetUp";
}

void DistributedDaemonManagerImplTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: GetInstanceTest
 * @tc.desc: Verify the GetInstance function
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DistributedDaemonManagerImplTest, GetInstanceTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetInstanceTest Start";
    try {
        DistributedFileDaemonManagerImpl::GetInstance();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetInstanceTest  ERROR";
    }
    GTEST_LOG_(INFO) << "GetInstanceTest End";
}

/**
 * @tc.name: OpenP2PConnectionTest
 * @tc.desc: Verify the OpenP2PConnection function
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DistributedDaemonManagerImplTest, OpenP2PConnectionTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OpenP2PConnectionTest Start";
    try {
        auto distributedFileDaemonProxy = DistributedFileDaemonProxy::GetInstance();
        EXPECT_NE(distributedFileDaemonProxy, nullptr);
        auto res = distributedDaemonManagerImpl_->OpenP2PConnection(deviceInfo);
        EXPECT_NE(res, E_SA_LOAD_FAILED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OpenP2PConnectionTest  ERROR";
    }
    GTEST_LOG_(INFO) << "OpenP2PConnectionTest End";
}

/**
 * @tc.name: CloseP2PConnectionTest
 * @tc.desc: Verify the CloseP2PConnection function
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DistributedDaemonManagerImplTest, CloseP2PConnectionTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloseP2PConnectionTest Start";
    try {
        auto distributedFileDaemonProxy = DistributedFileDaemonProxy::GetInstance();
        EXPECT_NE(distributedFileDaemonProxy, nullptr);
        auto res = distributedDaemonManagerImpl_->CloseP2PConnection(deviceInfo);
        EXPECT_NE(res, E_SA_LOAD_FAILED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CloseP2PConnectionTest  ERROR";
    }
    GTEST_LOG_(INFO) << "CloseP2PConnectionTest End";
}

/**
 * @tc.name: OpenP2PConnectionExTest
 * @tc.desc: Verify the OpenP2PConnectionEx function
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DistributedDaemonManagerImplTest, OpenP2PConnectionExTest, TestSize.Level1)
{
    auto remoteReverseObj = sptr(new FileDfsListenerMock());
    GTEST_LOG_(INFO) << "OpenP2PConnectionExTest Start";
    try {
        auto distributedFileDaemonProxy = DistributedFileDaemonProxy::GetInstance();
        EXPECT_NE(distributedFileDaemonProxy, nullptr);
        auto res = distributedDaemonManagerImpl_->OpenP2PConnectionEx("test", remoteReverseObj);
        EXPECT_NE(res, E_SA_LOAD_FAILED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OpenP2PConnectionExTest ERROR";
    }
    GTEST_LOG_(INFO) << "OpenP2PConnectionExTest End";
}

/**
 * @tc.name: CloseP2PConnectionEx
 * @tc.desc: Verify the CloseP2PConnectionEx function
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DistributedDaemonManagerImplTest, CloseP2PConnectionExTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloseP2PConnectionEx Start";
    try {
        auto distributedFileDaemonProxy = DistributedFileDaemonProxy::GetInstance();
        EXPECT_NE(distributedFileDaemonProxy, nullptr);
        auto res = distributedDaemonManagerImpl_->CloseP2PConnectionEx("test");
        EXPECT_NE(res, E_SA_LOAD_FAILED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CloseP2PConnectionEx ERROR";
    }
    GTEST_LOG_(INFO) << "CloseP2PConnectionEx End";
}

/**
 * @tc.name: PrepareSessionTest
 * @tc.desc: Verify the PrepareSession function
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DistributedDaemonManagerImplTest, PrepareSessionTest, TestSize.Level1)
{
    const std::string srcUri = "file://docs/storage/Users/currentUser/Documents?networkId=xxxxx";
    const std::string dstUri = "file://docs/storage/Users/currentUser/Documents";
    const std::string srcDeviceId = "testSrcDeviceId";
    const sptr<IRemoteObject> listener = sptr(new DaemonServiceMock());
    const std::string copyPath = "tmpDir";
    const std::string sessionName = "DistributedDevice0";
    HmdfsInfo fileInfo = {
        .copyPath = copyPath,
        .dirExistFlag = false,
        .sessionName = sessionName,
    };
    GTEST_LOG_(INFO) << "PrepareSessionTest Start";
    try {
        auto distributedFileDaemonProxy = DistributedFileDaemonProxy::GetInstance();
        EXPECT_NE(distributedFileDaemonProxy, nullptr);
        auto res = distributedDaemonManagerImpl_->PrepareSession(srcUri, dstUri, srcDeviceId, listener, fileInfo);
        EXPECT_NE(res, E_SA_LOAD_FAILED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PrepareSessionTest  ERROR";
    }
    GTEST_LOG_(INFO) << "PrepareSessionTest End";
}

/**
 * @tc.name: RequestSendFileTest
 * @tc.desc: Verify the RequestSendFile function
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DistributedDaemonManagerImplTest, RequestSendFileTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RequestSendFileTest Start";
    try {
        auto distributedFileDaemonProxy = DistributedFileDaemonProxy::GetInstance();
        EXPECT_NE(distributedFileDaemonProxy, nullptr);
        auto res = distributedDaemonManagerImpl_->RequestSendFile("uri", "path", "deviceId", "test");
        EXPECT_NE(res, E_SA_LOAD_FAILED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RequestSendFileTest ERROR";
    }
    GTEST_LOG_(INFO) << "RequestSendFileTest End";
}

/**
 * @tc.name: GetRemoteCopyInfoTest
 * @tc.desc: Verify the GetRemoteCopyInfo function
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DistributedDaemonManagerImplTest, GetRemoteCopyInfoTest, TestSize.Level1)
{
    bool isFile = false;
    bool isDir = false;
    GTEST_LOG_(INFO) << "GetRemoteCopyInfoTest Start";
    try {
        auto distributedFileDaemonProxy = DistributedFileDaemonProxy::GetInstance();
        EXPECT_NE(distributedFileDaemonProxy, nullptr);
        auto res = distributedDaemonManagerImpl_->GetRemoteCopyInfo("test", isFile, isDir);
        EXPECT_NE(res, E_SA_LOAD_FAILED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetRemoteCopyInfoTest ERROR";
    }
    GTEST_LOG_(INFO) << "GetRemoteCopyInfoTest End";
}

/**
 * @tc.name: PushAssetTest
 * @tc.desc: Verify the PushAsset function
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DistributedDaemonManagerImplTest, PushAssetTest, TestSize.Level1)
{
    sptr<AssetObj> assetObj = nullptr;
    int32_t userId = 800;
    GTEST_LOG_(INFO) << "PushAssetTest Start";
    try {
        sptr<IAssetSendCallbackMock> errPtr = nullptr;
        auto distributedFileDaemonProxy = DistributedFileDaemonProxy::GetInstance();
        EXPECT_NE(distributedFileDaemonProxy, nullptr);
        auto res = distributedDaemonManagerImpl_->PushAsset(userId, assetObj, nullptr);
        EXPECT_NE(res, E_SA_LOAD_FAILED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PushAssetTest ERROR";
    }
    GTEST_LOG_(INFO) << "PushAssetTest End";
}

/**
 * @tc.name: RegisterAssetCallbackTest
 * @tc.desc: Verify the RegisterAssetCallback function
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DistributedDaemonManagerImplTest, RegisterAssetCallbackTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterAssetCallbackTest Start";
    try {
        sptr<IAssetRecvCallbackMock> errPtr = nullptr;
        auto distributedFileDaemonProxy = DistributedFileDaemonProxy::GetInstance();
        EXPECT_NE(distributedFileDaemonProxy, nullptr);
        auto res = distributedDaemonManagerImpl_->RegisterAssetCallback(errPtr);
        EXPECT_EQ(res, E_NULLPTR);
        auto callbackMock = sptr(new IAssetRecvCallbackMock());
        res = distributedDaemonManagerImpl_->RegisterAssetCallback(callbackMock);
        EXPECT_NE(res, E_SA_LOAD_FAILED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RegisterAssetCallbackTest ERROR";
    }
    GTEST_LOG_(INFO) << "RegisterAssetCallbackTest End";
}

/**
 * @tc.name: UnRegisterAssetCallbackTest
 * @tc.desc: Verify the UnRegisterAssetCallback function
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DistributedDaemonManagerImplTest, UnRegisterAssetCallbackTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnRegisterAssetCallbackTest Start";
    try {
        sptr<IAssetRecvCallbackMock> errPtr = nullptr;
        auto distributedFileDaemonProxy = DistributedFileDaemonProxy::GetInstance();
        EXPECT_NE(distributedFileDaemonProxy, nullptr);
        auto res = distributedDaemonManagerImpl_->UnRegisterAssetCallback(errPtr);
        EXPECT_EQ(res, E_NULLPTR);

        auto callbackMock = sptr(new IAssetRecvCallbackMock());
        res = distributedDaemonManagerImpl_->UnRegisterAssetCallback(callbackMock);
        EXPECT_NE(res, E_SA_LOAD_FAILED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UnRegisterAssetCallbackTest ERROR";
    }
    GTEST_LOG_(INFO) << "UnRegisterAssetCallbackTest End";
}
} // namespace Test
} // namespace OHOS::Storage::DistributedFile