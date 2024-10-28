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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>

#include "daemon_execute.h"
#include "daemon.h"
#include "system_ability_definition.h"
#include "dfs_error.h"
#include "i_file_trans_listener.h"
#include "dfsu_access_token_helper.h"
#include "connection_detector_mock.h"
#include "device_manager_agent_mock.h"
#include "system_ability_manager_client_mock.h"
#include "daemon_mock.h"
#include "softbus_session_listener_mock.h"
#include "sandbox_helper.h"
#include "ipc_skeleton.h"
#include "network/softbus/softbus_session_listener.h"
#include "softbus_session_pool.h"
#include "asset_recv_callback_mock.h"
#include "asset_send_callback_mock.h"
#include "softbus_handler_asset_mock.h"
#include "softbus_handler_mock.h"
#include "asset_callback_manager.h"
#include "common_event_manager.h"

namespace {
bool g_checkCallerPermission = true;
bool g_checkCallerPermissionDatasync = true;
int g_getHapTokenInfo = 0;
std::string g_physicalPath;
int32_t g_getPhysicalPath = 0;
bool g_checkValidPath = false;
bool g_isFolder = false;
bool g_isFile = false;
pid_t g_getCallingUid = 100;
bool g_publish;
bool g_subscribeCommonEvent;
bool g_unSubscribeCommonEvent;
}

namespace {
const std::string FILE_MANAGER_AUTHORITY = "docs";
const std::string MEDIA_AUTHORITY = "media";
const int32_t E_PERMISSION_DENIED_NAPI = 201;
const int32_t E_INVAL_ARG_NAPI = 401;
const int32_t E_CONNECTION_FAILED = 13900045;
const int32_t E_UNMOUNT = 13600004;
}

namespace OHOS::FileManagement {
bool DfsuAccessTokenHelper::CheckCallerPermission(const std::string &permissionName)
{
    if (permissionName == PERM_DISTRIBUTED_DATASYNC) {
        return g_checkCallerPermissionDatasync;
    } else {
        return g_checkCallerPermission;
    }
}
} // namespace OHOS::FileManagement

namespace OHOS::Security::AccessToken {
int AccessTokenKit::GetHapTokenInfo(AccessTokenID callerToken, HapTokenInfo &hapTokenInfoRes)
{
    int32_t userID = 100;
    hapTokenInfoRes.bundleName = "com.example.app2backup";
    hapTokenInfoRes.userID = userID;
    return g_getHapTokenInfo;
}
} // namespace OHOS::Security::AccessToken

namespace OHOS::AppFileService {
int32_t SandboxHelper::GetPhysicalPath(const std::string &fileUri, const std::string &userId,
                                       std::string &physicalPath)
{
    physicalPath = g_physicalPath;
    return g_getPhysicalPath;
}

bool SandboxHelper::CheckValidPath(const std::string &filePath)
{
    return g_checkValidPath;
}
} // namespace OHOS::AppFileService

namespace OHOS::EventFwk {
bool CommonEventManager::SubscribeCommonEvent(const std::shared_ptr<CommonEventSubscriber> &subscriber)
{
    return g_subscribeCommonEvent;
}

bool CommonEventManager::UnSubscribeCommonEvent(const std::shared_ptr<CommonEventSubscriber> &subscriber)
{
    return g_unSubscribeCommonEvent;
}
} // namespace OHOS::EventFwk

namespace OHOS {
#ifdef CONFIG_IPC_SINGLE
using namespace IPC_SINGLE;
#endif
pid_t IPCSkeleton::GetCallingUid()
{
    return g_getCallingUid;
}

bool SystemAbility::Publish(sptr<IRemoteObject> systemAbility)
{
    return g_publish;
}

bool SystemAbility::AddSystemAbilityListener(int32_t systemAbilityId)
{
    return true;
}
} // namespace OHOS

namespace OHOS::Storage::DistributedFile {
namespace Utils {
bool IsFolder(const std::string &name)
{
    return g_isFolder;
}

bool IsFile(const std::string &path)
{
    return g_isFile;
}
} // namespace Utils

namespace Test {
using namespace OHOS::FileManagement;
using namespace testing;
using namespace testing::ext;
using namespace std;

class DaemonTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<Daemon> daemon_;

public:
    static inline std::shared_ptr<ConnectionDetectorMock> connectionDetectorMock_ = nullptr;
    static inline std::shared_ptr<DeviceManagerAgentMock> deviceManagerAgentMock_ = nullptr;
    static inline std::shared_ptr<DfsSystemAbilityManagerClientMock> smc_ = nullptr;
    static inline std::shared_ptr<SoftBusSessionListenerMock> softBusSessionListenerMock_ = nullptr;
    static inline std::shared_ptr<SoftBusHandlerAssetMock> softBusHandlerAssetMock_ = nullptr;
    static inline std::shared_ptr<SoftBusHandlerMock> softBusHandlerMock_ = nullptr;
};

class FileTransListenerMock : public IRemoteStub<IFileTransListener> {
public:
    MOCK_METHOD2(OnFileReceive, int32_t(uint64_t totalBytes, uint64_t processedBytes));
    MOCK_METHOD2(OnFailed, int32_t(const std::string &sessionName, int32_t errorCode));
    MOCK_METHOD1(OnFinished, int32_t(const std::string &sessionName));
};

class FileDfsListenerMock : public IRemoteStub<IFileDfsListener> {
public:
    MOCK_METHOD2(OnStatus, void(const std::string &networkId, int32_t status));
};

void DaemonTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    connectionDetectorMock_ = std::make_shared<ConnectionDetectorMock>();
    IConnectionDetectorMock::iConnectionDetectorMock_ = connectionDetectorMock_;
    deviceManagerAgentMock_ = std::make_shared<DeviceManagerAgentMock>();
    IDeviceManagerAgentMock::iDeviceManagerAgentMock_ = deviceManagerAgentMock_;
    smc_ = std::make_shared<DfsSystemAbilityManagerClientMock>();
    DfsSystemAbilityManagerClient::smc = smc_;
    softBusSessionListenerMock_ = std::make_shared<SoftBusSessionListenerMock>();
    ISoftBusSessionListenerMock::iSoftBusSessionListenerMock_ = softBusSessionListenerMock_;
    softBusHandlerAssetMock_ = std::make_shared<SoftBusHandlerAssetMock>();
    ISoftBusHandlerAssetMock::iSoftBusHandlerAssetMock_ = softBusHandlerAssetMock_;
    softBusHandlerMock_ = std::make_shared<SoftBusHandlerMock>();
    ISoftBusHandlerMock::iSoftBusHandlerMock_ = softBusHandlerMock_;

    std::string path =  "/mnt/hmdfs/100/account/device_view/local/data/com.huawei.hmos.example/docs";
    if (!std::filesystem::exists(path)) {
        std::filesystem::create_directory(path);
    }
    std::ofstream file(path + "/1.txt");
    std::ofstream file1(path + "/1@.txt");
}

void DaemonTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    IConnectionDetectorMock::iConnectionDetectorMock_ = nullptr;
    connectionDetectorMock_ = nullptr;
    IDeviceManagerAgentMock::iDeviceManagerAgentMock_ = nullptr;
    deviceManagerAgentMock_ = nullptr;
    DfsSystemAbilityManagerClient::smc = nullptr;
    smc_ = nullptr;

    std::string path =  "/mnt/hmdfs/100/account/device_view/local/data/com.huawei.hmos.example";
    if (std::filesystem::exists(path)) {
        std::filesystem::remove_all(path);
    }
}

void DaemonTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
    int32_t saID = FILEMANAGEMENT_DISTRIBUTED_FILE_DAEMON_SA_ID;
    bool runOnCreate = true;
    daemon_ = std::make_shared<Daemon>(saID, runOnCreate);
}

void DaemonTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
    daemon_ = nullptr;
}

/**
 * @tc.name: DaemonTest_PublishSA_001
 * @tc.desc: verify PublishSA.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_PublishSA_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_PublishSA_001 begin";
    daemon_->registerToService_ = true;
    try {
        daemon_->PublishSA();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
    }

    daemon_->registerToService_ = false;
    g_publish = false;
    try {
        daemon_->PublishSA();
        EXPECT_TRUE(false);
    } catch (...) {
        EXPECT_TRUE(true);
    }

    daemon_->registerToService_ = false;
    g_publish = true;
    try {
        daemon_->PublishSA();
        EXPECT_EQ(daemon_->registerToService_, true);
    } catch (...) {
        EXPECT_TRUE(true);
    }
    GTEST_LOG_(INFO) << "DaemonTest_PublishSA_001 end";
}

/**
 * @tc.name: DaemonTest_RegisterOsAccount_001
 * @tc.desc: verify RegisterOsAccount.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_RegisterOsAccount_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_RegisterOsAccount_001 begin";
    g_subscribeCommonEvent = true;
    try {
        daemon_->RegisterOsAccount();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
    }

    g_subscribeCommonEvent = false;
    try {
        daemon_->RegisterOsAccount();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "DaemonTest_RegisterOsAccount_001 end";
}

/**
 * @tc.name: DaemonTest_OnStart_001
 * @tc.desc: verify OnStart.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_OnStart_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_OnStart_001 begin";
    daemon_->state_ = ServiceRunningState::STATE_RUNNING;
    try {
        daemon_->OnStart();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
    }

    daemon_->state_ = ServiceRunningState::STATE_NOT_START;
    try {
        daemon_->OnStart();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "DaemonTest_OnStart_001 end";
}

/**
 * @tc.name: DaemonTest_OnStop_001
 * @tc.desc: verify OnStop.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_OnStop_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_OnStop_001 begin";
    g_unSubscribeCommonEvent = false;
    try {
        daemon_->OnStop();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
    }

    g_unSubscribeCommonEvent = true;
    try {
        daemon_->OnStop();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "DaemonTest_OnStop_001 end";
}

/**
 * @tc.name: DaemonTest_OnAddSystemAbility_001
 * @tc.desc: verify OnAddSystemAbility.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_OnAddSystemAbility_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_OnAddSystemAbility_001 begin";
    g_subscribeCommonEvent = false;
    try {
        daemon_->OnAddSystemAbility(COMMON_EVENT_SERVICE_ID, "");
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
    }

    try {
        daemon_->OnAddSystemAbility(SOFTBUS_SERVER_SA_ID, "");
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "DaemonTest_OnAddSystemAbility_001 end";
}

/**
 * @tc.name: DaemonTest_OnRemoveSystemAbility_001
 * @tc.desc: verify OnRemoveSystemAbility.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_OnRemoveSystemAbility_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_OnRemoveSystemAbility_001 begin";
    daemon_->subScriber_ = nullptr;
    try {
        daemon_->OnRemoveSystemAbility(COMMON_EVENT_SERVICE_ID, "");
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
    }

    g_unSubscribeCommonEvent = true;
    daemon_->RegisterOsAccount();;
    try {
        daemon_->OnRemoveSystemAbility(COMMON_EVENT_SERVICE_ID, "");
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
    }

    try {
        daemon_->OnRemoveSystemAbility(SOFTBUS_SERVER_SA_ID, "");
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "DaemonTest_OnRemoveSystemAbility_001 end";
}

/**
 * @tc.name: DaemonTest_OpenP2PConnection_001
 * @tc.desc: verify OpenP2PConnection.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_OpenP2PConnection_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_OpenP2PConnection_001 begin";
    DistributedHardware::DmDeviceInfo deviceInfo;
    EXPECT_CALL(*connectionDetectorMock_, GetConnectionStatus(_, _)).WillOnce(Return(true));
    EXPECT_EQ(daemon_->OpenP2PConnection(deviceInfo), E_OK);

    EXPECT_CALL(*connectionDetectorMock_, GetConnectionStatus(_, _)).WillOnce(Return(false));
    EXPECT_CALL(*deviceManagerAgentMock_, OnDeviceP2POnline(_)).WillOnce(Return(ERR_BAD_VALUE));
    EXPECT_CALL(*deviceManagerAgentMock_, OnDeviceP2POffline(_)).WillOnce(Return(0));
    EXPECT_EQ(daemon_->OpenP2PConnection(deviceInfo), ERR_BAD_VALUE);

    EXPECT_CALL(*connectionDetectorMock_, GetConnectionStatus(_, _)).WillOnce(Return(false));
    EXPECT_CALL(*deviceManagerAgentMock_, OnDeviceP2POnline(_)).WillOnce(Return(E_OK));
    EXPECT_CALL(*connectionDetectorMock_, RepeatGetConnectionStatus(_, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(daemon_->OpenP2PConnection(deviceInfo), E_OK);
    GTEST_LOG_(INFO) << "DaemonTest_OpenP2PConnection_001 end";
}

/**
 * @tc.name: DaemonTest_ConnectionCount_001
 * @tc.desc: verify ConnectionCount.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_ConnectionCount_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_ConnectionCount_001 begin";
    DistributedHardware::DmDeviceInfo deviceInfo;
    EXPECT_CALL(*deviceManagerAgentMock_, OnDeviceP2POnline(_)).WillOnce(Return(E_OK));
    EXPECT_CALL(*connectionDetectorMock_, RepeatGetConnectionStatus(_, _)).WillOnce(Return(E_OK))
        .WillOnce(Return(E_OK));
    EXPECT_EQ(daemon_->OpenP2PConnection(deviceInfo), E_OK);

    EXPECT_CALL(*deviceManagerAgentMock_, OnDeviceP2POnline(_)).WillOnce(Return(ERR_BAD_VALUE));
    EXPECT_CALL(*connectionDetectorMock_, RepeatGetConnectionStatus(_, _)).WillOnce(Return(ERR_BAD_VALUE))
        .WillOnce(Return(ERR_BAD_VALUE));
    EXPECT_EQ(daemon_->OpenP2PConnection(deviceInfo), ERR_BAD_VALUE);
    GTEST_LOG_(INFO) << "DaemonTest_ConnectionCount_001 end";
}

/**
 * @tc.name: DaemonTest_CleanUp_001
 * @tc.desc: verify CleanUp.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_CleanUp_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_CleanUp_001 begin";
    DistributedHardware::DmDeviceInfo deviceInfo;
    EXPECT_CALL(*deviceManagerAgentMock_, RemoveRemoteReverseObj(_, _)).WillOnce(Return(ERR_BAD_VALUE));
    EXPECT_CALL(*deviceManagerAgentMock_, OnDeviceP2POffline(_)).WillOnce(Return((E_OK)));
    EXPECT_EQ(daemon_->CleanUp(deviceInfo, "", 100), E_OK);

    EXPECT_CALL(*deviceManagerAgentMock_, RemoveRemoteReverseObj(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*deviceManagerAgentMock_, OnDeviceP2POffline(_)).WillOnce(Return((E_OK)));
    EXPECT_EQ(daemon_->CleanUp(deviceInfo, "", 100), E_OK);
    GTEST_LOG_(INFO) << "DaemonTest_CleanUp_001 end";
}

/**
 * @tc.name: DaemonTest_ConnectionAndMount_001
 * @tc.desc: verify ConnectionAndMount.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_ConnectionAndMount_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_ConnectionAndMount_001 begin";
    DistributedHardware::DmDeviceInfo deviceInfo;
    EXPECT_CALL(*deviceManagerAgentMock_, OnDeviceP2POnline(_)).WillOnce(Return(ERR_BAD_VALUE));
    EXPECT_CALL(*connectionDetectorMock_, RepeatGetConnectionStatus(_, _)).WillOnce(Return(ERR_BAD_VALUE))
        .WillOnce(Return(ERR_BAD_VALUE));
    EXPECT_EQ(daemon_->ConnectionAndMount(deviceInfo, "", 100), ERR_BAD_VALUE);

    EXPECT_CALL(*deviceManagerAgentMock_, OnDeviceP2POnline(_)).WillOnce(Return(E_OK));
    EXPECT_CALL(*connectionDetectorMock_, RepeatGetConnectionStatus(_, _)).WillOnce(Return(E_OK))
        .WillOnce(Return(E_OK));
    g_checkCallerPermission = false;
    EXPECT_EQ(daemon_->ConnectionAndMount(deviceInfo, "", 100), E_OK);

    EXPECT_CALL(*deviceManagerAgentMock_, OnDeviceP2POnline(_)).WillOnce(Return(E_OK));
    EXPECT_CALL(*connectionDetectorMock_, RepeatGetConnectionStatus(_, _)).WillOnce(Return(E_OK))
        .WillOnce(Return(E_OK));
    g_checkCallerPermission = true;
    EXPECT_CALL(*deviceManagerAgentMock_, GetDeviceIdByNetworkId(_)).WillOnce(Return("test"));
    EXPECT_CALL(*deviceManagerAgentMock_, MountDfsDocs(_, _)).WillOnce(Return(ERR_BAD_VALUE));
    EXPECT_EQ(daemon_->ConnectionAndMount(deviceInfo, "", 100), ERR_BAD_VALUE);

    EXPECT_CALL(*deviceManagerAgentMock_, OnDeviceP2POnline(_)).WillOnce(Return(E_OK));
    EXPECT_CALL(*connectionDetectorMock_, RepeatGetConnectionStatus(_, _)).WillOnce(Return(E_OK))
        .WillOnce(Return(E_OK));
    g_checkCallerPermission = true;
    EXPECT_CALL(*deviceManagerAgentMock_, GetDeviceIdByNetworkId(_)).WillOnce(Return("test"));
    EXPECT_CALL(*deviceManagerAgentMock_, MountDfsDocs(_, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(daemon_->ConnectionAndMount(deviceInfo, "", 100), E_OK);
    GTEST_LOG_(INFO) << "DaemonTest_ConnectionAndMount_001 end";
}

/**
 * @tc.name: DaemonTest_OpenP2PConnectionEx_001
 * @tc.desc: verify OpenP2PConnectionEx.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_OpenP2PConnectionEx_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_OpenP2PConnectionEx_001 begin";
    g_checkCallerPermissionDatasync = false;
    EXPECT_EQ(daemon_->OpenP2PConnectionEx("", nullptr), E_PERMISSION_DENIED_NAPI);

    g_checkCallerPermissionDatasync = true;
    daemon_->dfsListenerDeathRecipient_ = nullptr;
    EXPECT_EQ(daemon_->OpenP2PConnectionEx("", nullptr), E_INVAL_ARG_NAPI);

    auto listener = sptr<IFileDfsListener>(new FileDfsListenerMock());
    EXPECT_EQ(daemon_->OpenP2PConnectionEx("", listener), E_INVAL_ARG_NAPI);

    std::string networkId = "test";
    EXPECT_CALL(*deviceManagerAgentMock_, AddRemoteReverseObj(_, _)).WillOnce(Return(ERR_BAD_VALUE));
    EXPECT_CALL(*deviceManagerAgentMock_, OnDeviceP2POnline(_)).WillOnce(Return(ERR_BAD_VALUE));
    EXPECT_CALL(*connectionDetectorMock_, RepeatGetConnectionStatus(_, _)).WillOnce(Return(ERR_BAD_VALUE))
        .WillOnce(Return(ERR_BAD_VALUE));
    EXPECT_CALL(*deviceManagerAgentMock_, RemoveRemoteReverseObj(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*deviceManagerAgentMock_, OnDeviceP2POffline(_)).WillOnce(Return(E_OK));
    EXPECT_EQ(daemon_->OpenP2PConnectionEx(networkId, listener), E_CONNECTION_FAILED);

    EXPECT_CALL(*deviceManagerAgentMock_, AddRemoteReverseObj(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*deviceManagerAgentMock_, OnDeviceP2POnline(_)).WillOnce(Return(E_OK));
    EXPECT_CALL(*connectionDetectorMock_, RepeatGetConnectionStatus(_, _)).WillOnce(Return(E_OK))
        .WillOnce(Return(E_OK));
    g_checkCallerPermission = true;
    EXPECT_CALL(*deviceManagerAgentMock_, GetDeviceIdByNetworkId(_)).WillOnce(Return("test"));
    EXPECT_CALL(*deviceManagerAgentMock_, MountDfsDocs(_, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(daemon_->OpenP2PConnectionEx(networkId, listener), E_OK);
    GTEST_LOG_(INFO) << "DaemonTest_OpenP2PConnectionEx_001 end";
}

/**
 * @tc.name: DaemonTest_CloseP2PConnectionEx_001
 * @tc.desc: verify CloseP2PConnectionEx.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_CloseP2PConnectionEx_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_CloseP2PConnectionEx_001 begin";
    g_checkCallerPermissionDatasync = false;
    EXPECT_EQ(daemon_->CloseP2PConnectionEx(""), E_PERMISSION_DENIED_NAPI);

    g_checkCallerPermissionDatasync = true;
    EXPECT_EQ(daemon_->CloseP2PConnectionEx(""), E_INVAL_ARG_NAPI);

    std::string networkId = "test";
    EXPECT_CALL(*deviceManagerAgentMock_, GetDeviceIdByNetworkId(_)).WillOnce(Return(""));
    EXPECT_EQ(daemon_->CloseP2PConnectionEx(networkId), E_CONNECTION_FAILED);

    g_checkCallerPermission = true;
    EXPECT_CALL(*deviceManagerAgentMock_, GetDeviceIdByNetworkId(_)).WillOnce(Return("test"));
    EXPECT_CALL(*deviceManagerAgentMock_, UMountDfsDocs(_, _, _)).WillOnce(Return(ERR_BAD_VALUE));
    EXPECT_EQ(daemon_->CloseP2PConnectionEx(networkId), E_UNMOUNT);

    g_checkCallerPermission = true;
    EXPECT_CALL(*deviceManagerAgentMock_, GetDeviceIdByNetworkId(_)).WillOnce(Return("test"));
    EXPECT_CALL(*deviceManagerAgentMock_, UMountDfsDocs(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*deviceManagerAgentMock_, RemoveRemoteReverseObj(_, _)).WillOnce(Return(ERR_BAD_VALUE));
    EXPECT_CALL(*deviceManagerAgentMock_, RemoveRemoteReverseObj(_, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(daemon_->CloseP2PConnectionEx(networkId), E_OK);

    g_checkCallerPermission = false;
    EXPECT_CALL(*deviceManagerAgentMock_, GetDeviceIdByNetworkId(_)).WillOnce(Return("test"));
    EXPECT_CALL(*deviceManagerAgentMock_, RemoveRemoteReverseObj(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*deviceManagerAgentMock_, RemoveRemoteReverseObj(_, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(daemon_->CloseP2PConnectionEx(networkId), E_OK);
    GTEST_LOG_(INFO) << "DaemonTest_CloseP2PConnectionEx_001 end";
}

/**
 * @tc.name: DaemonTest_RequestSendFile_001
 * @tc.desc: verify RequestSendFile.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_RequestSendFile_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_RequestSendFile_001 begin";
    daemon_->eventHandler_ = nullptr;
    EXPECT_EQ(daemon_->RequestSendFile("", "", "", ""), E_EVENT_HANDLER);

    daemon_->StartEventHandler();
    EXPECT_EQ(daemon_->RequestSendFile("", "", "", ""), ERR_BAD_VALUE);
    GTEST_LOG_(INFO) << "DaemonTest_RequestSendFile_001 end";
}

/**
 * @tc.name: DaemonTest_PrepareSession_001
 * @tc.desc: verify PrepareSession.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_PrepareSession_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_PrepareSession_001 begin";
    HmdfsInfo hmdfsInfo;
    EXPECT_EQ(daemon_->PrepareSession("", "", "", nullptr, hmdfsInfo), E_NULLPTR);

    auto listener = sptr<IRemoteObject>(new FileTransListenerMock());
    auto sysAbilityManager = sptr<ISystemAbilityManagerMock>(new ISystemAbilityManagerMock());
    EXPECT_CALL(*smc_, GetSystemAbilityManager()).WillOnce(Return(sysAbilityManager));
    EXPECT_CALL(*sysAbilityManager, GetSystemAbility(_, _)).WillOnce(Return(nullptr));
    EXPECT_EQ(daemon_->PrepareSession("", "", "", listener, hmdfsInfo), E_SA_LOAD_FAILED);

    auto daemon = (new (std::nothrow) DaemonMock());
    EXPECT_CALL(*smc_, GetSystemAbilityManager()).WillOnce(Return(sysAbilityManager));
    EXPECT_CALL(*sysAbilityManager, GetSystemAbility(_, _)).WillOnce(Return(daemon));
    EXPECT_CALL(*daemon, GetRemoteCopyInfo(_, _, _)).WillOnce(Return(ERR_BAD_VALUE));
    EXPECT_EQ(daemon_->PrepareSession("", "", "", listener, hmdfsInfo), E_SOFTBUS_SESSION_FAILED);

    g_getHapTokenInfo = Security::AccessToken::AccessTokenKitRet::RET_SUCCESS;
    g_getPhysicalPath = E_OK;
    g_isFile = false;
    g_isFolder = true;
    hmdfsInfo.dirExistFlag = true;
    g_physicalPath = "tes@t/test";
    std::string dstUri = "file://docs/data/storage/el2/distributedfiles/images/1.png";
    EXPECT_CALL(*smc_, GetSystemAbilityManager()).WillOnce(Return(sysAbilityManager));
    EXPECT_CALL(*sysAbilityManager, GetSystemAbility(_, _)).WillOnce(Return(daemon));
    EXPECT_CALL(*daemon, GetRemoteCopyInfo(_, _, _)).WillOnce(Return(E_OK));
    SoftBusSessionPool::SessionInfo sessionInfo;
    while (true) {
        auto ret = SoftBusSessionPool::GetInstance().GenerateSessionName(sessionInfo);
        if (ret.empty()) {
            break;
        }
    }
    EXPECT_EQ(daemon_->PrepareSession("", dstUri, "", listener, hmdfsInfo), E_SOFTBUS_SESSION_FAILED);
    SoftBusSessionPool::GetInstance().sessionMap_.clear();
    GTEST_LOG_(INFO) << "DaemonTest_PrepareSession_001 end";
}

/**
 * @tc.name: DaemonTest_PrepareSession_002
 * @tc.desc: verify PrepareSession.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_PrepareSession_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_PrepareSession_002 begin";
    HmdfsInfo hmdfsInfo;
    hmdfsInfo.dirExistFlag = true;
    std::string dstUri = "file://docs/data/storage/el2/distributedfiles/images/1.png";
    auto listener = sptr<IRemoteObject>(new FileTransListenerMock());
    auto daemon = (new (std::nothrow) DaemonMock());
    auto sysAbilityManager = sptr<ISystemAbilityManagerMock>(new ISystemAbilityManagerMock());
    EXPECT_CALL(*smc_, GetSystemAbilityManager()).WillOnce(Return(sysAbilityManager));
    EXPECT_CALL(*sysAbilityManager, GetSystemAbility(_, _)).WillOnce(Return(daemon));
    EXPECT_CALL(*daemon, GetRemoteCopyInfo(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*softBusHandlerMock_, CreateSessionServer(_, _, _, _)).WillOnce(Return(-1));
    EXPECT_EQ(daemon_->PrepareSession("", dstUri, "", listener, hmdfsInfo), E_SOFTBUS_SESSION_FAILED);

    EXPECT_CALL(*smc_, GetSystemAbilityManager()).WillOnce(Return(sysAbilityManager));
    EXPECT_CALL(*sysAbilityManager, GetSystemAbility(_, _)).WillOnce(Return(daemon));
    EXPECT_CALL(*daemon, GetRemoteCopyInfo(_, _, _)).WillOnce(Return(E_OK));
    SoftBusSessionPool::GetInstance().sessionMap_.clear();
    EXPECT_CALL(*softBusHandlerMock_, CreateSessionServer(_, _, _, _)).WillOnce(Return(1));
    hmdfsInfo.authority = FILE_MANAGER_AUTHORITY;
    EXPECT_CALL(*daemon, RequestSendFile(_, _, _, _)).WillOnce(Return(ERR_BAD_VALUE));
    EXPECT_EQ(daemon_->PrepareSession("", dstUri, "", listener, hmdfsInfo), E_SOFTBUS_SESSION_FAILED);

    EXPECT_CALL(*smc_, GetSystemAbilityManager()).WillOnce(Return(sysAbilityManager));
    EXPECT_CALL(*sysAbilityManager, GetSystemAbility(_, _)).WillOnce(Return(daemon));
    EXPECT_CALL(*daemon, GetRemoteCopyInfo(_, _, _)).WillOnce(Return(E_OK));
    SoftBusSessionPool::GetInstance().sessionMap_.clear();
    EXPECT_CALL(*softBusHandlerMock_, CreateSessionServer(_, _, _, _)).WillOnce(Return(1));
    hmdfsInfo.authority = MEDIA_AUTHORITY;
    EXPECT_CALL(*daemon, RequestSendFile(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(daemon_->PrepareSession("", dstUri, "", listener, hmdfsInfo), E_OK);

    EXPECT_CALL(*smc_, GetSystemAbilityManager()).WillOnce(Return(sysAbilityManager));
    EXPECT_CALL(*sysAbilityManager, GetSystemAbility(_, _)).WillOnce(Return(daemon));
    EXPECT_CALL(*daemon, GetRemoteCopyInfo(_, _, _)).WillOnce(Return(E_OK));
    SoftBusSessionPool::GetInstance().sessionMap_.clear();
    EXPECT_CALL(*softBusHandlerMock_, CreateSessionServer(_, _, _, _)).WillOnce(Return(1));
    hmdfsInfo.authority = "test";
    EXPECT_CALL(*daemon, RequestSendFile(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(daemon_->PrepareSession("", dstUri, "", listener, hmdfsInfo), E_OK);
    GTEST_LOG_(INFO) << "DaemonTest_PrepareSession_002 end";
}

/**
 * @tc.name: DaemonTest_GetRealPath_001
 * @tc.desc: verify GetRealPath.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_GetRealPath_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_GetRealPath_001 begin";
    std::string physicalPath;
    HmdfsInfo info;
    EXPECT_EQ(daemon_->GetRealPath("", "", physicalPath, info, nullptr), E_INVAL_ARG_NAPI);

    auto daemon = new (std::nothrow) DaemonMock();
    EXPECT_CALL(*daemon, GetRemoteCopyInfo(_, _, _)).WillOnce(Return(ERR_BAD_VALUE));
    EXPECT_EQ(daemon_->GetRealPath("", "", physicalPath, info, daemon), E_SOFTBUS_SESSION_FAILED);

    g_getHapTokenInfo = ERR_BAD_VALUE;
    EXPECT_CALL(*daemon, GetRemoteCopyInfo(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(daemon_->GetRealPath("", "", physicalPath, info, daemon), E_GET_USER_ID);

    g_getHapTokenInfo = Security::AccessToken::AccessTokenKitRet::RET_SUCCESS;
    g_getPhysicalPath = ERR_BAD_VALUE;
    EXPECT_CALL(*daemon, GetRemoteCopyInfo(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(daemon_->GetRealPath("", "", physicalPath, info, daemon), E_GET_PHYSICAL_PATH_FAILED);

    g_getPhysicalPath = E_OK;
    g_checkValidPath = false;
    info.dirExistFlag = false;
    EXPECT_CALL(*daemon, GetRemoteCopyInfo(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(daemon_->GetRealPath("", "", physicalPath, info, daemon), E_GET_PHYSICAL_PATH_FAILED);

    g_checkValidPath = true;
    g_physicalPath = "test@test/test";
    info.dirExistFlag = true;
    std::string dstUri = "file://com.huawei.hmos.example/data/storage/el2/distributedfiles/images/1.png";
    EXPECT_CALL(*daemon, GetRemoteCopyInfo(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(daemon_->GetRealPath("", "", physicalPath, info, daemon), E_OK);
    GTEST_LOG_(INFO) << "DaemonTest_GetRealPath_001 end";
}

/**
 * @tc.name: DaemonTest_CheckCopyRule_001
 * @tc.desc: verify CheckCopyRule.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_CheckCopyRule_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_CheckCopyRule_001 begin";
    std::string physicalPath;
    HapTokenInfo hapTokenInfo;
    HmdfsInfo info;

    g_isFolder = false;
    EXPECT_EQ(daemon_->CheckCopyRule(physicalPath, "", hapTokenInfo, true, info), E_GET_PHYSICAL_PATH_FAILED); // 1 5

    g_isFolder = false;
    physicalPath = "/mnt/hmdfs/100/account/device_view/local/data/com.huawei.hmos.example/docs/1.txt";
    info.dirExistFlag = false;
    g_checkValidPath = false;
    EXPECT_EQ(daemon_->CheckCopyRule(physicalPath, "", hapTokenInfo, true, info), E_GET_PHYSICAL_PATH_FAILED); // 1 6 10 12 13

    g_isFolder = false;
    physicalPath = "test";
    info.dirExistFlag = false;
    EXPECT_EQ(daemon_->CheckCopyRule(physicalPath, "", hapTokenInfo, false, info), E_GET_PHYSICAL_PATH_FAILED); // 3 8 11

    g_isFolder = true;
    physicalPath = "/mnt/hmdfs/100/account/device_view/local/data/com.huawei.hmos.example/docs/1.txt";
    info.dirExistFlag = true;
    g_checkValidPath = false;
    EXPECT_EQ(daemon_->CheckCopyRule(physicalPath, "", hapTokenInfo, true, info), E_GET_PHYSICAL_PATH_FAILED); // 2 9 12 13

    g_isFolder = true;
    physicalPath = "te@st";
    info.dirExistFlag = true;
    std::string dstUri = "file://docs/data/storage/el2/distributedfiles/images/1.png";
    EXPECT_EQ(daemon_->CheckCopyRule(physicalPath, dstUri, hapTokenInfo, false, info), E_OK); // 4 7 16 21

    g_isFolder = false;
    physicalPath = "tes@t/test";
    info.dirExistFlag = false;
    g_checkValidPath = true;
    dstUri = "file://media/data/storage/el2/distributedfiles/images/1.png";
    EXPECT_EQ(daemon_->CheckCopyRule(physicalPath, dstUri, hapTokenInfo, false, info), E_OK); // 2 8 12 14 17 21
    GTEST_LOG_(INFO) << "DaemonTest_CheckCopyRule_001 end";
}

/**
 * @tc.name: DaemonTest_CheckCopyRule_002
 * @tc.desc: verify CheckCopyRule.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_CheckCopyRule_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_CheckCopyRule_002 begin";
    std::string physicalPath;
    HapTokenInfo hapTokenInfo;
    HmdfsInfo info;

    g_isFolder = true;
    physicalPath = "/mnt/hmdfs/100/account/device_view/local/data/com.huawei.hmos.example/docs/1.txt";
    info.dirExistFlag = true;
    std::string dstUri = "file://docs/data/storage/el2/distributedfiles/images/1.png";
    EXPECT_EQ(daemon_->CheckCopyRule(physicalPath, dstUri, hapTokenInfo, false, info), E_OK); // 4 9 16 22

    g_isFolder = true;
    physicalPath = "/mnt/hmdfs/100/account/device_view/local/data/com.huawei.hmos.example/docs/1@.txt";
    info.dirExistFlag = true;
    dstUri = "file://docs/data/storage/el2/distributedfiles/images/1.png";
    EXPECT_EQ(daemon_->CheckCopyRule(physicalPath, dstUri, hapTokenInfo, false, info), E_OK); // 4 9 16 23

    g_isFolder = true;
    physicalPath = "/mnt/hmdfs/100/account/device_view/local/data/com.huawei.hmos.example/docs/111";
    info.dirExistFlag = true;
    dstUri = "file://docs/data/storage/el2/distributedfiles/images/1.png";
    EXPECT_EQ(daemon_->CheckCopyRule(physicalPath, dstUri, hapTokenInfo, false, info), E_OK); // 4 9 16 20 25

    g_isFolder = true;
    physicalPath = "/mnt/hmdfs/100/account/device_view/local/data/com.huawei.hmos.example/docs/111";
    info.dirExistFlag = true;
    dstUri = "file://docs/data/storage/el2/distributedfiles/images/1.png";
    EXPECT_EQ(daemon_->CheckCopyRule(physicalPath, dstUri, hapTokenInfo, false, info), E_GET_PHYSICAL_PATH_FAILED); // 4 9 16 20 24

    g_isFolder = true;
    physicalPath = "te@st";
    info.dirExistFlag = true;
    hapTokenInfo.userID = 100;
    g_checkValidPath = false;
    dstUri = "file://com.huawei.hmco.example/data/storage/el2/distributedfiles/images/1.png";
    EXPECT_EQ(daemon_->CheckCopyRule(physicalPath, dstUri, hapTokenInfo, false, info), E_GET_PHYSICAL_PATH_FAILED); // 4 7 15 18

    g_isFolder = true;
    physicalPath = "te@st";
    info.dirExistFlag = true;
    hapTokenInfo.userID = 100;
    g_checkValidPath = true;
    dstUri = "file://com.huawei.hmco.example/data/storage/el2/distributedfiles/images/1.png";
    EXPECT_EQ(daemon_->CheckCopyRule(physicalPath, dstUri, hapTokenInfo, false, info), E_OK); // 4 7 15 19
    GTEST_LOG_(INFO) << "DaemonTest_CheckCopyRule_002 end";
}

/**
 * @tc.name: DaemonTest_GetRemoteCopyInfo_001
 * @tc.desc: verify GetRemoteCopyInfo.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_GetRemoteCopyInfo_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_GetRemoteCopyInfo_001 begin";
    EXPECT_CALL(*softBusSessionListenerMock_, GetRealPath(_)).WillOnce(Return(""));
    bool isSrcFile = false;
    bool srcIsDir = false;
    EXPECT_EQ(daemon_->GetRemoteCopyInfo("", isSrcFile, srcIsDir), E_SOFTBUS_SESSION_FAILED);

    EXPECT_CALL(*softBusSessionListenerMock_, GetRealPath(_)).WillOnce(Return("test"));
    EXPECT_EQ(daemon_->GetRemoteCopyInfo("", isSrcFile, srcIsDir), E_OK);
    GTEST_LOG_(INFO) << "DaemonTest_GetRemoteCopyInfo_001 end";
}

/**
 * @tc.name: DaemonTest_GetRemoteSA_001
 * @tc.desc: verify GetRemoteSA.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_GetRemoteSA_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_GetRemoteSA_001 begin";
    EXPECT_CALL(*smc_, GetSystemAbilityManager()).WillOnce(Return(nullptr));
    EXPECT_TRUE(daemon_->GetRemoteSA("") == nullptr);

    auto sysAbilityManager = sptr<ISystemAbilityManagerMock>(new ISystemAbilityManagerMock());
    EXPECT_CALL(*smc_, GetSystemAbilityManager()).WillOnce(Return(sysAbilityManager));
    EXPECT_CALL(*sysAbilityManager, GetSystemAbility(_, _)).WillOnce(Return(nullptr));
    EXPECT_TRUE(daemon_->GetRemoteSA("") == nullptr);

    auto daemon = new (std::nothrow) DaemonMock();
    EXPECT_CALL(*smc_, GetSystemAbilityManager()).WillOnce(Return(sysAbilityManager));
    EXPECT_CALL(*sysAbilityManager, GetSystemAbility(_, _)).WillOnce(Return(daemon));
    EXPECT_FALSE(daemon_->GetRemoteSA("") == nullptr);
    GTEST_LOG_(INFO) << "DaemonTest_GetRemoteSA_001 end";
}

/**
 * @tc.name: DaemonTest_Copy_001
 * @tc.desc: verify Copy.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_Copy_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_Copy_001 begin";
    EXPECT_EQ(daemon_->Copy("", "", nullptr, ""), E_INVAL_ARG_NAPI);

    auto daemon = new (std::nothrow) DaemonMock();
    EXPECT_CALL(*daemon, RequestSendFile(_, _, _, _)).WillOnce(Return(ERR_BAD_VALUE));
    EXPECT_EQ(daemon_->Copy("", "", daemon, ""), E_SA_LOAD_FAILED);

    EXPECT_CALL(*daemon, RequestSendFile(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(daemon_->Copy("", "", daemon, ""), E_OK);
    GTEST_LOG_(INFO) << "DaemonTest_Copy_001 end";
}

/**
 * @tc.name: DaemonTest_CancelCopyTask_001
 * @tc.desc: verify CancelCopyTask.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_CancelCopyTask_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_CancelCopyTask_001 begin";
    SoftBusSessionPool::GetInstance().DeleteSessionInfo("test");
    EXPECT_EQ(daemon_->CancelCopyTask("test"), E_INVAL_ARG_NAPI);

    SoftBusSessionPool::SessionInfo sessionInfo{.uid = 100};
    g_getCallingUid = 101;
    SoftBusSessionPool::GetInstance().AddSessionInfo("test", sessionInfo);
    EXPECT_EQ(daemon_->CancelCopyTask("test"), E_PERMISSION_DENIED);

    g_getCallingUid = 100;
    EXPECT_EQ(daemon_->CancelCopyTask("test"), E_OK);
    SoftBusSessionPool::GetInstance().DeleteSessionInfo("test");
    GTEST_LOG_(INFO) << "DaemonTest_CancelCopyTask_001 end";
}

/**
 * @tc.name: DaemonTest_PushAsset_001
 * @tc.desc: verify PushAsset.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_PushAsset_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_PushAsset_001 begin";
    int32_t userId = 100;
    sptr<AssetObj> assetObj(new (std::nothrow) AssetObj());
    auto assetSendCallback = new (std::nothrow) IAssetSendCallbackMock();
    EXPECT_EQ(daemon_->PushAsset(userId, nullptr, nullptr), E_NULLPTR);
    EXPECT_EQ(daemon_->PushAsset(userId, assetObj, nullptr), E_NULLPTR);
    EXPECT_EQ(daemon_->PushAsset(userId, nullptr, assetSendCallback), E_NULLPTR);
    EXPECT_EQ(daemon_->PushAsset(userId, assetObj, assetSendCallback), E_NULLPTR);

    assetObj->srcBundleName_ = "test";
    daemon_->eventHandler_ = nullptr;
    EXPECT_EQ(daemon_->PushAsset(userId, assetObj, assetSendCallback), E_EVENT_HANDLER);

    daemon_->StartEventHandler();
    EXPECT_CALL(*softBusHandlerAssetMock_, AssetBind(_, _)).WillOnce(Return(ERR_BAD_VALUE));
    EXPECT_EQ(daemon_->PushAsset(userId, assetObj, assetSendCallback), E_OK);
    GTEST_LOG_(INFO) << "DaemonTest_PushAsset_001 end";
}

/**
 * @tc.name: DaemonTest_RegisterAssetCallback_001
 * @tc.desc: verify RegisterAssetCallback.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_RegisterAssetCallback_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_RegisterAssetCallback_001 begin";
    EXPECT_EQ(daemon_->RegisterAssetCallback(nullptr), E_NULLPTR);

    auto assetRecvCallback = new (std::nothrow) IAssetRecvCallbackMock();
    EXPECT_EQ(daemon_->RegisterAssetCallback(assetRecvCallback), E_OK);
    AssetCallbackManager::GetInstance().RemoveRecvCallback(assetRecvCallback);
    GTEST_LOG_(INFO) << "DaemonTest_RegisterAssetCallback_001 end";
}

/**
 * @tc.name: DaemonTest_UnRegisterAssetCallback_001
 * @tc.desc: verify UnRegisterAssetCallback.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_UnRegisterAssetCallback_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_UnRegisterAssetCallback_001 begin";
    EXPECT_EQ(daemon_->UnRegisterAssetCallback(nullptr), E_NULLPTR);

    auto assetRecvCallback = new (std::nothrow) IAssetRecvCallbackMock();
    EXPECT_EQ(daemon_->UnRegisterAssetCallback(assetRecvCallback), E_OK);
    GTEST_LOG_(INFO) << "DaemonTest_UnRegisterAssetCallback_001 end";
}

/**
 * @tc.name: DaemonTest_StartEventHandler_001
 * @tc.desc: verify StartEventHandler.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_StartEventHandler_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_StartEventHandler_001 begin";
    daemon_->daemonExecute_ = std::make_shared<DaemonExecute>();
    daemon_->StartEventHandler();
    GTEST_LOG_(INFO) << "DaemonTest_StartEventHandler_001 end";
}

/**
 * @tc.name: DaemonTest_DfsListenerDeathRecipient_001
 * @tc.desc: verify DfsListenerDeathRecipient::OnRemoteDied.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_DfsListenerDeathRecipient_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_DfsListenerDeathRecipient_001 begin";
    sptr<Daemon::DfsListenerDeathRecipient> dfsListenerDeathRecipient_;
    auto daemon = new (std::nothrow) DaemonMock();

    EXPECT_CALL(*deviceManagerAgentMock_, FindListenerByObject(_, _, _)).WillOnce(Return(1));
    dfsListenerDeathRecipient_->OnRemoteDied(daemon);

    EXPECT_CALL(*deviceManagerAgentMock_, FindListenerByObject(_, _, _)).WillOnce(Return(2));
    dfsListenerDeathRecipient_->OnRemoteDied(daemon);

    auto listener = sptr<IFileDfsListener>(new FileDfsListenerMock());
    EXPECT_CALL(*deviceManagerAgentMock_, FindListenerByObject(_, _, _)).WillOnce(Return(3));
    EXPECT_CALL(*deviceManagerAgentMock_, GetDfsListener()).WillOnce(Return(listener));
    dfsListenerDeathRecipient_->OnRemoteDied(daemon);

    EXPECT_CALL(*deviceManagerAgentMock_, FindListenerByObject(_, _, _)).WillOnce(Return(4));
    EXPECT_CALL(*deviceManagerAgentMock_, GetDfsListener()).WillOnce(Return(listener));
    EXPECT_CALL(*deviceManagerAgentMock_, RemoveRemoteReverseObj(_, _)).WillOnce(Return(ERR_BAD_VALUE));
    dfsListenerDeathRecipient_->OnRemoteDied(daemon);

    EXPECT_CALL(*deviceManagerAgentMock_, FindListenerByObject(_, _, _)).WillOnce(Return(4));
    EXPECT_CALL(*deviceManagerAgentMock_, GetDfsListener()).WillOnce(Return(listener));
    EXPECT_CALL(*deviceManagerAgentMock_, RemoveRemoteReverseObj(_, _)).WillOnce(Return(E_OK));
    std::unordered_set<std::string> set1;
    EXPECT_CALL(*deviceManagerAgentMock_, GetNetworkIds(_)).WillOnce(Return(set1));
    dfsListenerDeathRecipient_->OnRemoteDied(daemon);

    EXPECT_CALL(*deviceManagerAgentMock_, FindListenerByObject(_, _, _)).WillOnce(Return(4));
    EXPECT_CALL(*deviceManagerAgentMock_, GetDfsListener()).WillOnce(Return(listener));
    EXPECT_CALL(*deviceManagerAgentMock_, RemoveRemoteReverseObj(_, _)).WillOnce(Return(E_OK));
    std::unordered_set<std::string> set2{"", "test01", "test02"};
    EXPECT_CALL(*deviceManagerAgentMock_, GetNetworkIds(_)).WillOnce(Return(set2));
    EXPECT_CALL(*deviceManagerAgentMock_, GetDeviceIdByNetworkId(_)).WillOnce(Return(""))
        .WillOnce(Return("test"));
    EXPECT_CALL(*deviceManagerAgentMock_, UMountDfsDocs(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*deviceManagerAgentMock_, OnDeviceP2POffline(_)).WillOnce(Return(E_OK));
    dfsListenerDeathRecipient_->OnRemoteDied(daemon);
    GTEST_LOG_(INFO) << "DaemonTest_DfsListenerDeathRecipient_001 end";
}
} // namespace Test
} // namespace OHOS::Storage::DistributedFile