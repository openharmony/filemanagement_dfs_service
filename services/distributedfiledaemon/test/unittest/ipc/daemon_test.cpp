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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <unistd.h>

#include "asset_callback_manager.h"
#include "asset_recv_callback_mock.h"
#include "asset_send_callback_mock.h"
#include "channel_manager_mock.h"
#include "common_event_manager.h"
#include "connect_count/connect_count.h"
#include "connection_detector_mock.h"
#include "control_cmd_parser.h"
#include "copy/file_size_utils.h"
#include "daemon.h"
#include "daemon_execute.h"
#include "daemon_mock.h"
#include "device/device_profile_adapter.h"
#include "device_manager_agent_mock.h"
#include "device_manager_impl.h"
#include "device_manager_impl_mock.h"
#include "device_profile_adapter.h"
#include "dfs_error.h"
#include "dfsu_access_token_helper.h"
#include "i_file_trans_listener.h"
#include "ipc/daemon.h"
#include "ipc_skeleton.h"
#include "iremote_object.h"
#include "network/devsl_dispatcher.h"
#include "network/softbus/softbus_session_listener.h"
#include "remote_file_share.h"
#include "sandbox_helper.h"
#include "securec.h"
#include "softbus_handler_asset_mock.h"
#include "softbus_handler_mock.h"
#include "softbus_session_listener_mock.h"
#include "softbus_session_pool.h"
#include "system_ability_definition.h"
#include "system_ability_manager_client_mock.h"
#include "utils_log.h"

namespace {
bool g_isLocalItDevice = false;
bool g_checkCallerPermission = true;
bool g_checkCallerPermissionDatasync = true;
int g_getHapTokenInfo = 0;
std::string g_physicalPath;
int32_t g_getPhysicalPath = 0;
bool g_checkValidPath = false;
bool g_isFolder = false;
bool g_isFile = false;
pid_t g_getCallingUid = 100;
std::string g_getCallingNetworkId;
bool g_publish;
bool g_subscribeCommonEvent;
bool g_unSubscribeCommonEvent;
int32_t g_getDfsVersionFromNetworkId = 0;
OHOS::Storage::DistributedFile::DfsVersion g_dfsVersion;
bool g_isRemoteDfsVersionLowerThanGiven = false;
int32_t g_getDeviceStatus = 0;
int32_t g_putDeviceStatus = 0;
bool g_checkSrcPermission = true;
bool g_getLocalAccountInfo = true;
bool g_checkSinkPermission = true;
} // namespace

namespace {
const std::string FILE_MANAGER_AUTHORITY = "docs";
const std::string MEDIA_AUTHORITY = "media";
const int32_t E_INVAL_ARG_NAPI = 401;
const int32_t E_CONNECTION_FAILED = 13900045;
const std::string NETWORKID_ONE = "testNetWork1";
const std::string NETWORKID_TWO = "testNetWork2";
OHOS::DistributedHardware::DmDeviceInfo deviceInfo;
constexpr int32_t ERR_DP_CAN_NOT_FIND = 98566199;
int32_t g_getDfsUrisDirFromLocal = OHOS::FileManagement::E_OK;
constexpr pid_t PASTE_BOARD_UID = 3816;
constexpr pid_t UDMF_UID = 3012;
} // namespace

namespace OHOS::Storage::DistributedFile {
int32_t DeviceProfileAdapter::GetDfsVersionFromNetworkId(const std::string &networkId,
                                                         DfsVersion &dfsVersion,
                                                         VersionPackageName packageName)
{
    if (networkId == NETWORKID_ONE) {
        dfsVersion.majorVersionNum = 0;
        return OHOS::FileManagement::E_OK;
    }

    if (networkId == NETWORKID_TWO) {
        dfsVersion.majorVersionNum = 1;
        return OHOS::FileManagement::E_OK;
    }
    dfsVersion = g_dfsVersion;
    return g_getDfsVersionFromNetworkId;
}

int32_t DeviceProfileAdapter::GetDeviceStatus(const std::string &networkId, bool &status)
{
    return g_getDeviceStatus;
}

int32_t DeviceProfileAdapter::PutDeviceStatus(bool status)
{
    return g_putDeviceStatus;
}

bool ControlCmdParser::IsLocalItDevice()
{
    return g_isLocalItDevice;
}

bool DeviceProfileAdapter::IsRemoteDfsVersionLowerThanGiven(const std::string &remoteNetworkId,
                                                            const DfsVersion &givenDfsVersion,
                                                            VersionPackageName packageName)
{
    return g_isRemoteDfsVersionLowerThanGiven;
}

bool SoftBusPermissionCheck::CheckSrcPermission(const std::string &sinkNetworkId, int32_t userId)
{
    return g_checkSrcPermission;
}

bool SoftBusPermissionCheck::GetLocalAccountInfo(AccountInfo &localAccountInfo,  int32_t userId)
{
    return g_getLocalAccountInfo;
}

bool SoftBusPermissionCheck::CheckSinkPermission(const AccountInfo &callerAccountInfo)
{
    return g_checkSinkPermission;
}
} // namespace OHOS::Storage::DistributedFile

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
int32_t SandboxHelper::GetPhysicalPath(const std::string &fileUri, const std::string &userId, std::string &physicalPath)
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

namespace OHOS::AppFileService::ModuleRemoteFileShare {
int32_t RemoteFileShare::GetDfsUrisDirFromLocal(
    const std::vector<std::string> &uriList,
    const int32_t &userId,
    std::unordered_map<std::string, AppFileService::ModuleRemoteFileShare::HmdfsUriInfo> &uriToDfsUriMaps)
{
    return g_getDfsUrisDirFromLocal;
}
} // namespace OHOS::AppFileService::ModuleRemoteFileShare

namespace OHOS {
namespace DistributedHardware {
DeviceManager &DeviceManager::GetInstance()
{
    GTEST_LOG_(INFO) << "GetInstance start";
    return DeviceManagerImpl::GetInstance();
}
} // namespace DistributedHardware
} // namespace OHOS

namespace OHOS {
#ifdef CONFIG_IPC_SINGLE
using namespace IPC_SINGLE;
#endif
pid_t IPCSkeleton::GetCallingUid()
{
    return g_getCallingUid;
}

std::string IPCSkeleton::GetCallingDeviceID()
{
    return g_getCallingNetworkId;
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
using namespace OHOS::Storage::DistributedFile;

class DaemonTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    sptr<Daemon> daemon_;

public:
    static inline std::shared_ptr<ConnectionDetectorMock> connectionDetectorMock_ = nullptr;
    static inline std::shared_ptr<DeviceManagerAgentMock> deviceManagerAgentMock_ = nullptr;
    static inline std::shared_ptr<DfsSystemAbilityManagerClientMock> smc_ = nullptr;
    static inline std::shared_ptr<SoftBusSessionListenerMock> softBusSessionListenerMock_ = nullptr;
    static inline std::shared_ptr<SoftBusHandlerAssetMock> softBusHandlerAssetMock_ = nullptr;
    static inline std::shared_ptr<SoftBusHandlerMock> softBusHandlerMock_ = nullptr;
    static inline std::shared_ptr<DeviceManagerImplMock> deviceManagerImplMock_ = nullptr;
    static inline std::shared_ptr<ChannelManagerMock> channelManagerMock_ = nullptr;
};

class FileTransListenerMock : public IRemoteStub<IFileTransListener> {
public:
    MOCK_METHOD2(OnFileReceive, int32_t(uint64_t totalBytes, uint64_t processedBytes));
    MOCK_METHOD2(OnFailed, int32_t(const std::string &sessionName, int32_t errorCode));
    MOCK_METHOD1(OnFinished, int32_t(const std::string &sessionName));
};

class FileDfsListenerMock : public IRemoteStub<IFileDfsListener> {
public:
    MOCK_METHOD4(OnStatus, void(const std::string &networkId, int32_t status, const std::string &path, int32_t type));
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

    std::string path = "/mnt/hmdfs/100/account/device_view/local/data/com.example.app";
    if (!std::filesystem::exists(path)) {
        std::filesystem::create_directory(path);
        std::filesystem::create_directory(path + "/docs");
    }
    std::string path2 = "/mnt/hmdfs/100/account/device_view/local/files/Docs/Download/111";
    if (std::filesystem::exists(path2)) {
        std::filesystem::remove_all(path2);
    }
    std::ofstream file(path + "/docs/1.txt");
    std::ofstream file1(path + "/docs/1@.txt");
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
    ISoftBusSessionListenerMock::iSoftBusSessionListenerMock_ = nullptr;
    softBusSessionListenerMock_ = nullptr;
    ISoftBusHandlerAssetMock::iSoftBusHandlerAssetMock_ = nullptr;
    softBusHandlerAssetMock_ = nullptr;
    ISoftBusHandlerMock::iSoftBusHandlerMock_ = nullptr;
    softBusHandlerMock_ = nullptr;

    std::string path = "/mnt/hmdfs/100/account/device_view/local/data/com.example.app";
    if (std::filesystem::exists(path)) {
        std::filesystem::remove_all(path);
    }
}

void DaemonTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
    int32_t saID = FILEMANAGEMENT_DISTRIBUTED_FILE_DAEMON_SA_ID;
    bool runOnCreate = true;
    daemon_ = new (std::nothrow) Daemon(saID, runOnCreate);
    ASSERT_TRUE(daemon_ != nullptr) << "daemon_ assert failed!";
    g_isRemoteDfsVersionLowerThanGiven = false;
    g_getDeviceStatus = 0;
    g_putDeviceStatus = 0;
    deviceInfo = {
        .deviceId = "testdevid",
        .deviceName = "testdevname",
        .deviceTypeId = 1,
        .networkId = "testNetWork1",
        .authForm = OHOS::DistributedHardware::DmAuthForm::IDENTICAL_ACCOUNT,
        .extraData = "{\"OS_TYPE\":10}",
    };
    g_isLocalItDevice = false;
    channelManagerMock_ = std::make_shared<ChannelManagerMock>();
    IChannelManagerMock::iChannelManagerMock = channelManagerMock_;
    deviceManagerImplMock_ = std::make_shared<DeviceManagerImplMock>();
    DfsDeviceManagerImpl::dfsDeviceManagerImpl = deviceManagerImplMock_;

    g_getDfsVersionFromNetworkId = 0;
    g_dfsVersion = {0, 0, 0};
    g_isRemoteDfsVersionLowerThanGiven = false;
    g_checkSrcPermission = true;
    g_getLocalAccountInfo = true;
    g_checkSinkPermission = true;
}

void DaemonTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
    daemon_ = nullptr;
    channelManagerMock_ = nullptr;
    IChannelManagerMock::iChannelManagerMock = nullptr;
    deviceManagerImplMock_ = nullptr;
    DfsDeviceManagerImpl::dfsDeviceManagerImpl = nullptr;
}

/**
 * @tc.name: OnStopTest
 * @tc.desc: Verify the OnStop function
 * @tc.type: FUNC
 * @tc.require: issueI7M6L1
 */
HWTEST_F(DaemonTest, OnStopTest, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "OnStop Start";

    // testcase 1
    daemon_->state_ = ServiceRunningState::STATE_NOT_START;
    daemon_->registerToService_ = false;
    EXPECT_NO_THROW(daemon_->OnStop());

    GTEST_LOG_(INFO) << "OnStop End";
}

/**
 * @tc.name: DaemonTest_PublishSA_0100
 * @tc.desc: Verify the PublishSA function.
 * @tc.type: FUNC
 * @tc.require: issueI7SP3A
 */
HWTEST_F(DaemonTest, DaemonTest_PublishSA_0100, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "DaemonTest_PublishSA_0100 start";

    // testcase 1
    daemon_->registerToService_ = true;
    daemon_->PublishSA();
    EXPECT_TRUE(daemon_->registerToService_);

    GTEST_LOG_(INFO) << "DaemonTest_PublishSA_0100 end";
}

/**
 * @tc.name: DaemonTest_OnStart_0100
 * @tc.desc: Verify the OnStart function.
 * @tc.type: FUNC
 * @tc.require: issueI7SP3A
 */
HWTEST_F(DaemonTest, DaemonTest_OnStart_0100, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "DaemonTest_OnStart_0100 start";

    // testcase 1
    daemon_->state_ = ServiceRunningState::STATE_RUNNING;
    daemon_->OnStart();
    EXPECT_EQ(daemon_->state_, ServiceRunningState::STATE_RUNNING);

    GTEST_LOG_(INFO) << "DaemonTest_OnStart_0100 end";
}

/**
 * @tc.name: DaemonTest_OnRemoveSystemAbility_0100
 * @tc.desc: Verify the OnRemoveSystemAbility function.
 * @tc.type: FUNC
 * @tc.require: issueI7SP3A
 */
HWTEST_F(DaemonTest, DaemonTest_OnRemoveSystemAbility_0100, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "DaemonTest_OnRemoveSystemAbility_0100 start";

    // testcase 1
    EXPECT_NO_THROW(daemon_->OnRemoveSystemAbility(COMMON_EVENT_SERVICE_ID + 1, ""));

    // testcase 2
    daemon_->subScriber_ = nullptr;
    EXPECT_NO_THROW(daemon_->OnRemoveSystemAbility(COMMON_EVENT_SERVICE_ID, ""));

    GTEST_LOG_(INFO) << "DaemonTest_OnRemoveSystemAbility_0100 end";
}

/**
 * @tc.name: DaemonTest_PrepareSession_0100
 * @tc.desc: Verify the PrepareSession function.
 * @tc.type: FUNC
 * @tc.require: issueI90MOB
 */
HWTEST_F(DaemonTest, DaemonTest_PrepareSession_0100, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "DaemonTest_PrepareSession_0100 start";

    const std::string srcUri = "file://docs/storage/Users/currentUser/Documents?networkid=xxxxx";
    const std::string dstUri = "file://docs/storage/Users/currentUser/Documents";
    const std::string srcDeviceId = "testSrcDeviceId";
    auto listener = sptr<IRemoteObject>(new FileTransListenerMock());
    const std::string copyPath = "tmpDir";
    const std::string sessionName = "DistributedDevice0";
    HmdfsInfo fileInfo = {
        .copyPath = copyPath,
        .dirExistFlag = false,
        .sessionName = sessionName,
    };
    EXPECT_EQ(daemon_->PrepareSession(srcUri, dstUri, srcDeviceId, listener, fileInfo),
              FileManagement::E_SA_LOAD_FAILED);

    GTEST_LOG_(INFO) << "DaemonTest_PrepareSession_0100 end";
}

/**
 * @tc.name: DaemonTest_GetDfsUrisDirFromLocal_0100
 * @tc.desc: Verify the GetDfsUrisDirFromLocal function.
 * @tc.type: FUNC
 * @tc.require: issueI90MOB
 */
HWTEST_F(DaemonTest, DaemonTest_GetDfsUrisDirFromLocal_0100, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "DaemonTest_GetDfsUrisDirFromLocal_0100 start";
    std::vector<std::string> uriList;
    int32_t userId = 100;
    std::string normalUri = "file://docs/data/storage/el2/cloud";
    std::string errUri = "file://docs/data/storage/../el2/cloud";
    std::unordered_map<std::string, AppFileService::ModuleRemoteFileShare::HmdfsUriInfo> uriToDfsUriMaps;
    g_getCallingUid = -1;
    auto ret = daemon_->GetDfsUrisDirFromLocal(uriList, userId, uriToDfsUriMaps);
    EXPECT_EQ(ret, FileManagement::E_PERMISSION_DENIED);

    g_getCallingUid = PASTE_BOARD_UID;
    uriList.emplace_back(normalUri);
    uriList.emplace_back(errUri);
    ret = daemon_->GetDfsUrisDirFromLocal(uriList, userId, uriToDfsUriMaps);
    EXPECT_EQ(ret, FileManagement::E_ILLEGAL_URI);

    uriList.erase(uriList.begin() + 1); // 1: errUri
    g_getDfsUrisDirFromLocal = FileManagement::E_INVAL_ARG;
    ret = daemon_->GetDfsUrisDirFromLocal(uriList, userId, uriToDfsUriMaps);
    EXPECT_EQ(ret, FileManagement::E_INVAL_ARG);

    g_getDfsUrisDirFromLocal = OHOS::FileManagement::E_OK;
    ret = daemon_->GetDfsUrisDirFromLocal(uriList, userId, uriToDfsUriMaps);
    EXPECT_EQ(ret, OHOS::FileManagement::E_OK);

    g_getCallingUid = UDMF_UID;
    uriList.emplace_back(errUri);
    ret = daemon_->GetDfsUrisDirFromLocal(uriList, userId, uriToDfsUriMaps);
    EXPECT_EQ(ret, FileManagement::E_ILLEGAL_URI);

    uriList.erase(uriList.begin() + 1); // 1: errUri
    g_getDfsUrisDirFromLocal = FileManagement::E_INVAL_ARG;
    ret = daemon_->GetDfsUrisDirFromLocal(uriList, userId, uriToDfsUriMaps);
    EXPECT_EQ(ret, FileManagement::E_INVAL_ARG);

    g_getDfsUrisDirFromLocal = OHOS::FileManagement::E_OK;
    ret = daemon_->GetDfsUrisDirFromLocal(uriList, userId, uriToDfsUriMaps);
    EXPECT_EQ(ret, OHOS::FileManagement::E_OK);

    GTEST_LOG_(INFO) << "DaemonTest_GetDfsUrisDirFromLocal_0100 end";
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
    ASSERT_NE(daemon_, nullptr);
    daemon_->registerToService_ = true;
    try {
        daemon_->PublishSA();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
    }
    daemon_->registerToService_ = false;
    g_publish = true;
    try {
        daemon_->PublishSA();
        EXPECT_EQ(daemon_->registerToService_, true);
    } catch (...) {
        EXPECT_TRUE(false);
    }
}

/**
 * @tc.name: DaemonTest_PublishSA_002
 * @tc.desc: verify PublishSA.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_PublishSA_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_PublishSA_001 begin";
    ASSERT_NE(daemon_, nullptr);
    daemon_->registerToService_ = false;
    g_publish = false;
    try {
        daemon_->PublishSA();
        EXPECT_TRUE(false);
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
    ASSERT_NE(daemon_, nullptr);
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
    ASSERT_NE(daemon_, nullptr);
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
    ASSERT_NE(daemon_, nullptr);
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
    ASSERT_NE(daemon_, nullptr);
    g_subscribeCommonEvent = false;
    try {
        daemon_->OnAddSystemAbility(COMMON_EVENT_SERVICE_ID, "");
        std::shared_ptr<OsAccountObserver> subScriber = daemon_->subScriber_;
        EXPECT_TRUE(subScriber != nullptr);

        daemon_->OnAddSystemAbility(COMMON_EVENT_SERVICE_ID, "");
        std::shared_ptr<OsAccountObserver> subScriber1 = daemon_->subScriber_;
        EXPECT_TRUE(subScriber == subScriber1);
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
    ASSERT_NE(daemon_, nullptr);
    try {
        daemon_->OnAddSystemAbility(COMMON_EVENT_SERVICE_ID, "");
        daemon_->OnRemoveSystemAbility(COMMON_EVENT_SERVICE_ID, "");
        EXPECT_TRUE(daemon_->subScriber_ != nullptr);

        daemon_->subScriber_ = nullptr;
        daemon_->OnRemoveSystemAbility(COMMON_EVENT_SERVICE_ID, "");
        EXPECT_TRUE(daemon_->subScriber_ == nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
    }

    g_unSubscribeCommonEvent = true;
    daemon_->RegisterOsAccount();
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
 * @tc.name: DaemonTest_ConnectDfs_001
 * @tc.desc: verify ConnectDfs.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_ConnectDfs_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_ConnectDfs_001 begin";
    ASSERT_NE(daemon_, nullptr);
    std::string networkId;
    ConnectCount::GetInstance().RemoveAllConnect();
    EXPECT_CALL(*deviceManagerAgentMock_, OnDeviceP2POnline(_)).WillOnce(Return(ERR_BAD_VALUE));
    EXPECT_EQ(daemon_->ConnectDfs(networkId), ERR_BAD_VALUE);

    EXPECT_CALL(*deviceManagerAgentMock_, OnDeviceP2POnline(_)).WillOnce(Return(E_OK));
    EXPECT_CALL(*connectionDetectorMock_, RepeatGetConnectionStatus(_, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(daemon_->ConnectDfs(networkId), E_OK);
    GTEST_LOG_(INFO) << "DaemonTest_ConnectDfs_001 end";
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
    ASSERT_NE(daemon_, nullptr);
    DistributedHardware::DmDeviceInfo deviceInfo;
    ConnectCount::GetInstance().RemoveAllConnect();
    EXPECT_CALL(*deviceManagerAgentMock_, OnDeviceP2POnline(_)).WillOnce(Return(ERR_BAD_VALUE));
    EXPECT_EQ(daemon_->ConnectionCount(deviceInfo), ERR_BAD_VALUE);

    EXPECT_CALL(*deviceManagerAgentMock_, OnDeviceP2POnline(_)).WillOnce(Return(E_OK));
    EXPECT_CALL(*connectionDetectorMock_, RepeatGetConnectionStatus(_, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(daemon_->ConnectionCount(deviceInfo), E_OK);
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
    ASSERT_NE(daemon_, nullptr);
    string networkId = "testNetworkId";

    ConnectCount::GetInstance().RemoveAllConnect();
    EXPECT_CALL(*deviceManagerAgentMock_, OnDeviceP2POffline(_)).WillOnce(Return((E_OK)));
    EXPECT_EQ(daemon_->CleanUp(networkId), E_EVENT_HANDLER);
    sleep(1);

    EXPECT_CALL(*deviceManagerAgentMock_, OnDeviceP2POffline(_)).WillOnce(Return((ERR_BAD_VALUE)));
    EXPECT_EQ(daemon_->CleanUp(networkId), E_EVENT_HANDLER);
    sleep(1);

    sptr<IFileDfsListener> nullListener = nullptr;
    ConnectCount::GetInstance().AddConnect(333, networkId, nullListener);
    EXPECT_EQ(daemon_->CleanUp(networkId), E_OK);
    sleep(1);
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
    DistributedHardware::DmDeviceInfo deviceInfo = {.networkId = "test"};
    sptr<IFileDfsListener> remoteReverseObj = nullptr;
    ConnectCount::GetInstance().RemoveAllConnect();

    // g_checkCallerPermission is ok but remote reject
    g_checkCallerPermission = true;
    g_isRemoteDfsVersionLowerThanGiven = true;
    EXPECT_CALL(*channelManagerMock_, HasExistChannel(_)).WillRepeatedly(Return(true));
    EXPECT_EQ(daemon_->ConnectionAndMount(deviceInfo, "test", remoteReverseObj), E_PERMISSION);

    // g_checkCallerPermission is false
    g_checkCallerPermission = true;
    g_isRemoteDfsVersionLowerThanGiven = false;
    EXPECT_CALL(*deviceManagerAgentMock_, OnDeviceP2POnline(_)).WillOnce(Return(ERR_BAD_VALUE));
    EXPECT_EQ(daemon_->ConnectionAndMount(deviceInfo, "test", remoteReverseObj), ERR_BAD_VALUE);

    EXPECT_CALL(*deviceManagerAgentMock_, OnDeviceP2POnline(_)).WillOnce(Return(E_OK));
    EXPECT_CALL(*connectionDetectorMock_, RepeatGetConnectionStatus(_, _)).WillOnce(Return(E_OK));
    g_checkCallerPermission = false;
    EXPECT_EQ(daemon_->ConnectionAndMount(deviceInfo, "test", remoteReverseObj), E_OK);

    g_checkCallerPermission = true;
    EXPECT_CALL(*deviceManagerAgentMock_, GetDeviceIdByNetworkId(_)).WillOnce(Return("test"));
    EXPECT_CALL(*deviceManagerAgentMock_, MountDfsDocs(_, _, _)).WillOnce(Return(ERR_BAD_VALUE));
    EXPECT_CALL(*connectionDetectorMock_, RepeatGetConnectionStatus(_, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(daemon_->ConnectionAndMount(deviceInfo, "test", remoteReverseObj), ERR_BAD_VALUE);

    EXPECT_CALL(*deviceManagerAgentMock_, OnDeviceP2POnline(_)).WillRepeatedly(Return(E_OK));
    EXPECT_CALL(*connectionDetectorMock_, RepeatGetConnectionStatus(_, _)).WillRepeatedly(Return(E_OK));
    EXPECT_CALL(*deviceManagerAgentMock_, GetDeviceIdByNetworkId(_)).WillOnce(Return("test"));
    EXPECT_CALL(*deviceManagerAgentMock_, MountDfsDocs(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(daemon_->ConnectionAndMount(deviceInfo, "test", remoteReverseObj), E_OK);
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

    // check fileAccessManager permit failed
    g_checkCallerPermission = true;
    g_isLocalItDevice = true;
    EXPECT_EQ(daemon_->OpenP2PConnectionEx("", nullptr), E_PERMISSION);

    // check permission failed
    g_isLocalItDevice = false;
    g_checkCallerPermissionDatasync = false;
    EXPECT_EQ(daemon_->OpenP2PConnectionEx("", nullptr), E_PERMISSION);

    // networkId length is invalid
    g_checkCallerPermission = false;
    g_checkCallerPermissionDatasync = true;
    EXPECT_EQ(daemon_->OpenP2PConnectionEx("", nullptr), E_INVAL_ARG_NAPI);
    std::string longNetworkId(DM_MAX_DEVICE_ID_LEN, 'a');
    EXPECT_EQ(daemon_->OpenP2PConnectionEx(longNetworkId, nullptr), E_INVAL_ARG_NAPI);

    // networkId is valid and obj is null
    std::string validNetworkId(64, 'a');
    EXPECT_EQ(daemon_->OpenP2PConnectionEx(validNetworkId, nullptr), NO_ERROR);

    // networkId is valid and obj is not null
    auto listener = sptr<IFileDfsListener>(new FileDfsListenerMock());
    EXPECT_EQ(daemon_->OpenP2PConnectionEx(validNetworkId, listener), NO_ERROR);

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

    // check fileAccessManager permit failed
    g_checkCallerPermission = true;
    g_isLocalItDevice = true;
    EXPECT_EQ(daemon_->CloseP2PConnectionEx(""), E_PERMISSION);

    // check permission failed
    g_isLocalItDevice = false;
    g_checkCallerPermissionDatasync = false;
    EXPECT_EQ(daemon_->CloseP2PConnectionEx(""), E_PERMISSION);

    // networkId length is invalid
    g_checkCallerPermission = false;
    g_checkCallerPermissionDatasync = true;
    EXPECT_EQ(daemon_->CloseP2PConnectionEx(""), E_INVAL_ARG_NAPI);
    std::string longNetworkId(DM_MAX_DEVICE_ID_LEN, 'a');
    EXPECT_EQ(daemon_->CloseP2PConnectionEx(longNetworkId), E_INVAL_ARG_NAPI);

    // networkId is valid but without file access permissions
    std::string validNetworkId(64, 'a');
    g_checkCallerPermission = false;
    EXPECT_NE(daemon_->CloseP2PConnectionEx(validNetworkId), NO_ERROR);

    // networkId is valid with file access permissions
    g_checkCallerPermission = true;
    EXPECT_NE(daemon_->CloseP2PConnectionEx(validNetworkId), NO_ERROR);

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
    ASSERT_NE(daemon_, nullptr);
    g_getCallingNetworkId = "testNetWork1";
    std::vector<DmDeviceInfo> deviceList;
    deviceList.push_back(deviceInfo);
#ifdef SUPPORT_SAME_ACCOUNT
    EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0)));
#endif
    daemon_->eventHandler_ = nullptr;
    EXPECT_EQ(daemon_->RequestSendFile("", "", "", ""), E_EVENT_HANDLER);

#ifdef SUPPORT_SAME_ACCOUNT
    EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0)));
#endif
    EXPECT_EQ(daemon_->RequestSendFile("../srcUri", "", "", ""), E_ILLEGAL_URI);

#ifdef SUPPORT_SAME_ACCOUNT
    EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0)));
#endif
    EXPECT_EQ(daemon_->RequestSendFile("", "../dstUri", "", ""), E_ILLEGAL_URI);

    daemon_->StartEventHandler();
#ifdef SUPPORT_SAME_ACCOUNT
    EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0)));
#endif
    EXPECT_EQ(daemon_->RequestSendFile("", "", "", ""), ERR_BAD_VALUE);
    GTEST_LOG_(INFO) << "DaemonTest_RequestSendFile_001 end";
}

/**
 * @tc.name: DaemonTest_RequestSendFile_002
 * @tc.desc: verify RequestSendFile.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_RequestSendFile_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_RequestSendFile_002 begin";
#ifdef SUPPORT_SAME_ACCOUNT
    ASSERT_NE(daemon_, nullptr);
    g_getCallingNetworkId.clear();
    EXPECT_EQ(daemon_->RequestSendFile("", "", "", ""), E_PERMISSION_DENIED);

    g_getCallingNetworkId = "testNetWork1";
    std::vector<DmDeviceInfo> deviceList;
    EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0)));
    EXPECT_EQ(daemon_->RequestSendFile("", "", "", ""), E_PERMISSION_DENIED);

    (void)memcpy_s(deviceInfo.networkId, DM_MAX_DEVICE_NAME_LEN - 1,
        NETWORKID_TWO.c_str(), NETWORKID_TWO.size());
    deviceList.push_back(deviceInfo);
    EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0)));
    EXPECT_EQ(daemon_->RequestSendFile("", "", "", ""), E_PERMISSION_DENIED);
#endif
    GTEST_LOG_(INFO) << "DaemonTest_RequestSendFile_002 end";
}

/**
 * @tc.name: DaemonTest_RequestSendFile_003
 * @tc.desc: verify RequestSendFile.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_RequestSendFile_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_RequestSendFile_003 begin";
#ifdef SUPPORT_SAME_ACCOUNT
    DistributedHardware::DmDeviceInfo deviceInfo1 = {
        .deviceId = "test",
        .deviceName = "testdevname",
        .deviceTypeId = 1,
        .networkId = "testNetWork1",
        .authForm = DmAuthForm::ACROSS_ACCOUNT,
        .extraData = "{\"OS_TYPE\":10}",
    };
    std::vector<DmDeviceInfo> deviceList;
    deviceList.push_back(deviceInfo1);
    g_getCallingNetworkId = "testNetWork1";
    EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0)));
    ASSERT_NE(daemon_, nullptr);
    EXPECT_EQ(daemon_->RequestSendFile("", "", "", ""), E_PERMISSION_DENIED);

    g_getCallingNetworkId = "testNetWork2";
    deviceInfo1.authForm = DmAuthForm::IDENTICAL_ACCOUNT;
    deviceList.clear();
    deviceList.push_back(deviceInfo1);
    EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0)));
    EXPECT_EQ(daemon_->RequestSendFile("", "", "", ""), E_PERMISSION_DENIED);

    g_getCallingNetworkId = "testNetWork1";
    EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0)));
    EXPECT_EQ(daemon_->RequestSendFile("/data/../test/1.txt", "", "", ""), E_ILLEGAL_URI);
#endif
    GTEST_LOG_(INFO) << "DaemonTest_RequestSendFile_003 end";
}

/**
 * @tc.name: DaemonTest_PrepareSession_001
 * @tc.desc: 验证 PrepareSession 方法在多种场景下的分支覆盖
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_PrepareSession_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_PrepareSession_001 start";
    ASSERT_NE(daemon_, nullptr);
    HmdfsInfo hmdfsInfo;
    std::string srcUri = "file://docs/storage/el2/distributedfiles/test.txt";
    std::string dstUri = "file://media/storage/el2/distributedfiles/test.txt?networkid=testDevice";
    std::string srcDeviceId = "testDevice";
    sptr<IRemoteObject> listener = new (std::nothrow) FileTransListenerMock();
    ASSERT_TRUE(listener != nullptr) << "监听器创建失败";

    // 测试用例 1: 空监听器
    EXPECT_EQ(daemon_->PrepareSession("", "", "", nullptr, hmdfsInfo), E_NULLPTR);

    // 测试用例 2: 有效 URI，物理路径无效
    g_getPhysicalPath = ERR_BAD_VALUE;
    g_checkValidPath = false;
    EXPECT_EQ(daemon_->PrepareSession(srcUri, dstUri, srcDeviceId, listener, hmdfsInfo), EINVAL);

    // 测试用例 3: 有效 URI，物理路径有效，stat 失败
    g_getPhysicalPath = E_OK;
    g_physicalPath = "/mnt/hmdfs/100/account/device_view/local/data/com.example.app/docs/test.txt";
    g_checkValidPath = true;
    g_isFile = false;
    EXPECT_EQ(daemon_->PrepareSession(srcUri, dstUri, srcDeviceId, listener, hmdfsInfo), E_SA_LOAD_FAILED);

    // 测试用例 4: 有效 URI，物理路径有效，stat 成功，DFS 版本为 0，文件大小 < 1GB, 走innerCopy
    g_getPhysicalPath = E_OK;
    g_physicalPath = "/mnt/hmdfs/100/account/device_view/local/data/com.example.app/docs/1.txt";
    g_checkValidPath = true;
    g_isFile = true;
    g_isFolder = false;
    std::ofstream file(g_physicalPath);
    ASSERT_TRUE(file.good()) << "创建测试文件失败";
    file.close();
    EXPECT_EQ(daemon_->PrepareSession(srcUri, dstUri, NETWORKID_ONE, listener, hmdfsInfo), E_SA_LOAD_FAILED);

    // 测试用例 5: DFS 有效 URI，物理路径有效，stat 成功，DFS 版本为 1，文件大小 < 1GB, CopyBaseOnRPC
    EXPECT_EQ(daemon_->PrepareSession(srcUri, dstUri, NETWORKID_TWO, listener, hmdfsInfo), 22);

    // 测试用例 6: DFS 有效 URI，物理路径有效，stat 成功，DFS 版本获取失败，文件大小 < 1GB, CopyBaseOnRPC
    EXPECT_EQ(daemon_->PrepareSession(srcUri, dstUri, "invalidDevice", listener, hmdfsInfo), E_SA_LOAD_FAILED);

    // 清理
    if (std::filesystem::exists(g_physicalPath)) {
        std::filesystem::remove(g_physicalPath);
    }

    GTEST_LOG_(INFO) << "DaemonTest_PrepareSession_001 end";
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
    ASSERT_NE(daemon_, nullptr);
    std::string physicalPath;
    HmdfsInfo info;
    EXPECT_EQ(daemon_->GetRealPath("", "", physicalPath, info, nullptr, "networkId"), E_INVAL_ARG_NAPI);

    sptr<DaemonMock> daemon = new (std::nothrow) DaemonMock();
    ASSERT_TRUE(daemon != nullptr) << "daemon assert failed!";

    EXPECT_EQ(daemon_->GetRealPath("../srcUri", "", physicalPath, info, daemon, "networkId"), E_ILLEGAL_URI);

    EXPECT_EQ(daemon_->GetRealPath("", "../dstUri", physicalPath, info, daemon, "networkId"), E_ILLEGAL_URI);

    g_checkSrcPermission = false;
    EXPECT_EQ(daemon_->GetRealPath("", "", physicalPath, info, daemon, "networkId"), ERR_ACL_FAILED);

    g_checkSrcPermission = true;
    g_getLocalAccountInfo = false;
    EXPECT_EQ(daemon_->GetRealPath("", "", physicalPath, info, daemon, "networkId"), ERR_ACL_FAILED);

    g_checkSrcPermission = true;
    g_getLocalAccountInfo = true;
    EXPECT_CALL(*daemon, GetRemoteCopyInfoACL(_, _, _, _)).WillOnce(Return(ERR_BAD_VALUE));
    EXPECT_EQ(daemon_->GetRealPath("", "", physicalPath, info, daemon, "networkId"), E_SOFTBUS_SESSION_FAILED);

    g_isRemoteDfsVersionLowerThanGiven = true;

    EXPECT_CALL(*daemon, GetRemoteCopyInfo(_, _, _)).WillOnce(Return(ERR_BAD_VALUE));
    EXPECT_EQ(daemon_->GetRealPath("", "", physicalPath, info, daemon, "networkId"), E_SOFTBUS_SESSION_FAILED);

    g_getHapTokenInfo = ERR_BAD_VALUE;
    EXPECT_CALL(*daemon, GetRemoteCopyInfo(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(daemon_->GetRealPath("", "", physicalPath, info, daemon, "networkId"), E_GET_USER_ID);

    g_getHapTokenInfo = Security::AccessToken::AccessTokenKitRet::RET_SUCCESS;
    g_getPhysicalPath = ERR_BAD_VALUE;
    EXPECT_CALL(*daemon, GetRemoteCopyInfo(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(daemon_->GetRealPath("", "", physicalPath, info, daemon, "networkId"), E_GET_PHYSICAL_PATH_FAILED);

    g_getPhysicalPath = E_OK;
    g_checkValidPath = false;
    info.dirExistFlag = false;
    EXPECT_CALL(*daemon, GetRemoteCopyInfo(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(daemon_->GetRealPath("", "", physicalPath, info, daemon, "networkId"), E_GET_PHYSICAL_PATH_FAILED);

    g_checkValidPath = true;
    g_physicalPath = "test@test/test";
    info.dirExistFlag = true;
    std::string dstUri = "file://com.example.app/data/storage/el2/distributedfiles/images/1.png";
    EXPECT_CALL(*daemon, GetRemoteCopyInfo(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(daemon_->GetRealPath("", "", physicalPath, info, daemon, "networkId"), E_OK);
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
    ASSERT_NE(daemon_, nullptr);
    std::string physicalPath;
    HapTokenInfo hapTokenInfo;
    HmdfsInfo info;

    g_isFolder = false;
    EXPECT_EQ(daemon_->CheckCopyRule(physicalPath, "", hapTokenInfo, true, info), E_GET_PHYSICAL_PATH_FAILED);

    g_isFolder = false;
    physicalPath = "/mnt/hmdfs/100/account/device_view/local/data/com.example.app/docs/1.txt";
    info.dirExistFlag = false;
    g_checkValidPath = false;
    EXPECT_EQ(daemon_->CheckCopyRule(physicalPath, "", hapTokenInfo, true, info), E_GET_PHYSICAL_PATH_FAILED);

    g_isFolder = false;
    physicalPath = "test";
    info.dirExistFlag = false;
    EXPECT_EQ(daemon_->CheckCopyRule(physicalPath, "", hapTokenInfo, false, info), E_GET_PHYSICAL_PATH_FAILED);

    g_isFolder = true;
    physicalPath = "/mnt/hmdfs/100/account/device_view/local/data/com.example.app/docs/1.txt";
    info.dirExistFlag = true;
    g_checkValidPath = false;
    EXPECT_EQ(daemon_->CheckCopyRule(physicalPath, "", hapTokenInfo, true, info), E_GET_PHYSICAL_PATH_FAILED);

    g_isFolder = true;
    physicalPath = "te@st";
    info.dirExistFlag = true;
    std::string dstUri = "file://docs/data/storage/el2/distributedfiles/images/1.png";
    EXPECT_EQ(daemon_->CheckCopyRule(physicalPath, dstUri, hapTokenInfo, false, info), E_OK);

    g_isFolder = false;
    physicalPath = "tes@t/test";
    info.dirExistFlag = false;
    g_checkValidPath = true;
    dstUri = "file://media/data/storage/el2/distributedfiles/images/1.png";
    EXPECT_EQ(daemon_->CheckCopyRule(physicalPath, dstUri, hapTokenInfo, false, info), E_OK);
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
    ASSERT_NE(daemon_, nullptr);
    std::string physicalPath;
    HapTokenInfo hapTokenInfo;
    HmdfsInfo info;

    g_isFolder = true;
    physicalPath = "/mnt/hmdfs/100/account/device_view/local/data/com.example.app/docs/1.txt";
    info.dirExistFlag = true;
    std::string dstUri = "file://docs/data/storage/el2/distributedfiles/images/1.png";
    EXPECT_EQ(daemon_->CheckCopyRule(physicalPath, dstUri, hapTokenInfo, false, info), E_OK);

    g_isFolder = true;
    physicalPath = "/mnt/hmdfs/100/account/device_view/local/data/com.example.app/docs/1@.txt";
    info.dirExistFlag = true;
    dstUri = "file://docs/data/storage/el2/distributedfiles/images/1.png";
    EXPECT_EQ(daemon_->CheckCopyRule(physicalPath, dstUri, hapTokenInfo, false, info), E_OK);

    g_isFolder = true;
    physicalPath = "/mnt/hmdfs/100/account/device_view/local/files/Docs/Download/111";
    info.dirExistFlag = true;
    dstUri = "file://docs/data/storage/el2/distributedfiles/images/1.png";
    EXPECT_EQ(daemon_->CheckCopyRule(physicalPath, dstUri, hapTokenInfo, false, info), E_OK);

    g_isFolder = true;
    physicalPath = "/mnt/hmdfs/100/account/device_view/local/data/docs/111";
    info.dirExistFlag = true;
    dstUri = "file://docs/data/storage/el2/distributedfiles/images/1.png";
    EXPECT_EQ(daemon_->CheckCopyRule(physicalPath, dstUri, hapTokenInfo, false, info), E_GET_PHYSICAL_PATH_FAILED);

    g_isFolder = true;
    physicalPath = "te@st";
    info.dirExistFlag = true;
    hapTokenInfo.userID = 100;
    g_checkValidPath = false;
    dstUri = "file://com.example.app/data/storage/el2/distributedfiles/images/1.png";
    EXPECT_EQ(daemon_->CheckCopyRule(physicalPath, dstUri, hapTokenInfo, false, info),
              E_GET_PHYSICAL_PATH_FAILED); // 4 7 15 18

    g_isFolder = true;
    physicalPath = "te@st";
    info.dirExistFlag = true;
    hapTokenInfo.userID = 100;
    g_checkValidPath = true;
    dstUri = "file://com.example.app/data/storage/el2/distributedfiles/images/1.png";
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
    ASSERT_NE(daemon_, nullptr);
    g_getCallingNetworkId = "testNetWork1";
    std::vector<DmDeviceInfo> deviceList;
    (void)memcpy_s(deviceInfo.networkId, DM_MAX_DEVICE_NAME_LEN - 1,
        NETWORKID_ONE.c_str(), NETWORKID_ONE.size());
    deviceList.push_back(deviceInfo);
#ifdef SUPPORT_SAME_ACCOUNT
    EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0)));
#endif
    EXPECT_CALL(*softBusSessionListenerMock_, GetRealPath(_)).WillOnce(Return(""));
    bool isSrcFile = false;
    bool srcIsDir = false;
    EXPECT_EQ(daemon_->GetRemoteCopyInfo("", isSrcFile, srcIsDir), E_SOFTBUS_SESSION_FAILED);

#ifdef SUPPORT_SAME_ACCOUNT
    EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0)));
#endif
    EXPECT_CALL(*softBusSessionListenerMock_, GetRealPath(_)).WillOnce(Return("test"));
    EXPECT_EQ(daemon_->GetRemoteCopyInfo("", isSrcFile, srcIsDir), E_OK);
    GTEST_LOG_(INFO) << "DaemonTest_GetRemoteCopyInfo_001 end";
}

/**
 * @tc.name: DaemonTest_GetRemoteCopyInfo_002
 * @tc.desc: verify GetRemoteCopyInfo.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_GetRemoteCopyInfo_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_GetRemoteCopyInfo_002 begin";
#ifdef SUPPORT_SAME_ACCOUNT
    ASSERT_NE(daemon_, nullptr);
    g_getCallingNetworkId.clear();
    bool isSrcFile = false;
    bool srcIsDir = false;
    EXPECT_EQ(daemon_->GetRemoteCopyInfo("", isSrcFile, srcIsDir), E_PERMISSION_DENIED);

    g_getCallingNetworkId = "testNetWork1";
    std::vector<DmDeviceInfo> deviceList;
    EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0)));
    EXPECT_EQ(daemon_->GetRemoteCopyInfo("", isSrcFile, srcIsDir), E_PERMISSION_DENIED);

    (void)memcpy_s(deviceInfo.networkId, DM_MAX_DEVICE_NAME_LEN - 1,
        NETWORKID_TWO.c_str(), NETWORKID_TWO.size());
    deviceList.push_back(deviceInfo);
    EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0)));
    EXPECT_EQ(daemon_->GetRemoteCopyInfo("", isSrcFile, srcIsDir), E_PERMISSION_DENIED);
#endif
    GTEST_LOG_(INFO) << "DaemonTest_GetRemoteCopyInfo_002 end";
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
    ASSERT_NE(daemon_, nullptr);
    EXPECT_CALL(*smc_, GetSystemAbilityManager()).WillOnce(Return(nullptr));
    EXPECT_TRUE(daemon_->GetRemoteSA("") == nullptr);

    auto sysAbilityManager = sptr<ISystemAbilityManagerMock>(new ISystemAbilityManagerMock());
    EXPECT_CALL(*smc_, GetSystemAbilityManager()).WillOnce(Return(sysAbilityManager));
    EXPECT_CALL(*sysAbilityManager, GetSystemAbility(_, _)).WillOnce(Return(nullptr));
    EXPECT_TRUE(daemon_->GetRemoteSA("") == nullptr);

    sptr<DaemonMock> daemon = new (std::nothrow) DaemonMock();
    ASSERT_TRUE(daemon != nullptr) << "daemon assert failed!";
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

    EXPECT_EQ(daemon_->Copy("", "", nullptr, "", "networkId"), E_INVAL_ARG_NAPI);

    sptr<DaemonMock> daemon = new (std::nothrow) DaemonMock();
    ASSERT_TRUE(daemon != nullptr) << "daemon assert failed!";

    string srcUri = "file://docs/storage/media/100/local/files/Docs/../A/1.txt";
    string destPath = "/storage/media/100/local/files/Docs/aa1/";
    EXPECT_EQ(daemon_->Copy(srcUri, destPath, daemon, "", "networkId"), E_INVAL_ARG);

    srcUri = "file://docs/storage/media/100/local/files/Docs/../A/1.txt?networkid=123456";
    EXPECT_EQ(daemon_->Copy(srcUri, destPath, daemon, "", "networkId"), E_INVAL_ARG);

    srcUri = "file://docs/storage/media/100/local/files/Docs/1.txt";
    destPath = "/storage/media/100/local/files/Docs/../aa1/";
    EXPECT_EQ(daemon_->Copy(srcUri, destPath, daemon, "", "networkId"), E_INVAL_ARG);

    destPath = "/storage/media/100/local/files/Docs/../aa1/?networkid=123456";
    EXPECT_EQ(daemon_->Copy(srcUri, destPath, daemon, "", "networkId"), E_INVAL_ARG);

    EXPECT_CALL(*deviceManagerImplMock_, GetLocalDeviceInfo(_, _)).WillOnce(Return(ERR_BAD_VALUE));
    EXPECT_EQ(daemon_->Copy("", "", daemon, "", "networkId"), E_GET_DEVICE_ID);

    EXPECT_CALL(*deviceManagerImplMock_, GetLocalDeviceInfo(_, _)).WillRepeatedly(Return(E_OK));
    g_checkSrcPermission = false;
    EXPECT_EQ(daemon_->Copy("", "", daemon, "", "networkId"), ERR_ACL_FAILED);

    g_checkSrcPermission = true;
    g_getLocalAccountInfo = false;
    EXPECT_EQ(daemon_->Copy("", "", daemon, "", "networkId"), ERR_ACL_FAILED);

    g_checkSrcPermission = true;
    g_getLocalAccountInfo = true;
    EXPECT_CALL(*daemon, RequestSendFileACL(_, _, _, _, _)).WillOnce(Return(ERR_BAD_VALUE));
    EXPECT_EQ(daemon_->Copy("", "", daemon, "", "networkId"), E_SA_LOAD_FAILED);

    g_isRemoteDfsVersionLowerThanGiven = true;
    EXPECT_CALL(*daemon, RequestSendFile(_, _, _, _)).WillOnce(Return(ERR_BAD_VALUE));
    EXPECT_EQ(daemon_->Copy("", "", daemon, "", "networkId"), E_SA_LOAD_FAILED);

    EXPECT_CALL(*deviceManagerImplMock_, GetLocalDeviceInfo(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*daemon, RequestSendFile(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(daemon_->Copy("", "", daemon, "", "networkId"), E_OK);

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
    ASSERT_NE(daemon_, nullptr);
    SoftBusSessionPool::GetInstance().DeleteSessionInfo("test");
    EXPECT_EQ(daemon_->CancelCopyTask("test"), E_INVAL_ARG);

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
    ASSERT_NE(daemon_, nullptr);
    int32_t userId = 100;
    sptr<AssetObj> assetObj(new (std::nothrow) AssetObj());
    ASSERT_TRUE(assetObj != nullptr) << "assetObj assert failed!";
    sptr<IAssetSendCallback> assetSendCallback = new (std::nothrow) IAssetSendCallbackMock();
    ASSERT_TRUE(assetSendCallback != nullptr) << "assetSendCallback assert failed!";
    EXPECT_EQ(daemon_->PushAsset(userId, nullptr, nullptr), E_NULLPTR);
    EXPECT_EQ(daemon_->PushAsset(userId, assetObj, nullptr), E_NULLPTR);
    EXPECT_EQ(daemon_->PushAsset(userId, nullptr, assetSendCallback), E_NULLPTR);
    EXPECT_EQ(daemon_->PushAsset(userId, assetObj, assetSendCallback), E_NULLPTR);

    ASSERT_NE(assetObj, nullptr);
    assetObj->srcBundleName_ = "test";
    daemon_->eventHandler_ = nullptr;
    EXPECT_EQ(daemon_->PushAsset(userId, assetObj, assetSendCallback), E_EVENT_HANDLER);
    GTEST_LOG_(INFO) << "DaemonTest_PushAsset_001 end";
}

/**
 * @tc.name: DaemonTest_PushAsset_002
 * @tc.desc: verify PushAsset.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_PushAsset_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_PushAsset_002 begin";
    ASSERT_NE(daemon_, nullptr);
    daemon_->StartEventHandler();
    int32_t userId = 100;
    sptr<AssetObj> assetObj(new (std::nothrow) AssetObj());
    ASSERT_TRUE(assetObj != nullptr) << "assetObj assert failed!";
    assetObj->dstNetworkId_ = "test";
    DevslDispatcher::devslMap_.clear();
    DevslDispatcher::devslMap_.insert(make_pair("test", static_cast<int32_t>(SecurityLabel::S4)));
    assetObj->uris_.push_back("file://com.example.app/data/storage/el2/distributedfiles/docs/1.txt");
    assetObj->srcBundleName_ = "com.example.app";
    sptr<IAssetSendCallback> assetSendCallback = new (std::nothrow) IAssetSendCallbackMock();
    ASSERT_TRUE(assetSendCallback != nullptr) << "assetSendCallback assert failed!";
    g_getPhysicalPath = E_OK;
    g_checkValidPath = true;
    g_isFolder = false;
    EXPECT_CALL(*softBusHandlerAssetMock_, AssetBind(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*softBusHandlerAssetMock_, AssetSendFile(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(daemon_->PushAsset(userId, assetObj, assetSendCallback), E_OK);

    assetObj->uris_.push_back("../srcUri");
    EXPECT_EQ(daemon_->PushAsset(userId, assetObj, assetSendCallback), E_ILLEGAL_URI);
    sleep(1);
    AssetCallbackManager::GetInstance().RemoveSendCallback(assetObj->srcBundleName_);
    GTEST_LOG_(INFO) << "DaemonTest_PushAsset_002 end";
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
    ASSERT_NE(daemon_, nullptr);
    EXPECT_EQ(daemon_->RegisterAssetCallback(nullptr), E_NULLPTR);

    sptr<IAssetRecvCallback> assetRecvCallback = new (std::nothrow) IAssetRecvCallbackMock();
    ASSERT_TRUE(assetRecvCallback != nullptr) << "assetRecvCallback assert failed!";
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
    ASSERT_NE(daemon_, nullptr);
    EXPECT_EQ(daemon_->UnRegisterAssetCallback(nullptr), E_NULLPTR);

    sptr<IAssetRecvCallback> assetRecvCallback = new (std::nothrow) IAssetRecvCallbackMock();
    ASSERT_TRUE(assetRecvCallback != nullptr) << "assetRecvCallback assert failed!";
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
    ASSERT_NE(daemon_, nullptr);
    daemon_->daemonExecute_ = std::make_shared<DaemonExecute>();
    daemon_->StartEventHandler();
    GTEST_LOG_(INFO) << "DaemonTest_StartEventHandler_001 end";
}

/**
 * @tc.name: DaemonTest_InnerCopy_001
 * @tc.desc: verify InnerCopy.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_InnerCopy_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_InnerCopy_001 begin";
    string srcUri = "file://docs/storage/media/100/local/files/Docs/../A/1.txt";
    string destUri = "file://docs/storage/media/100/local/files/Docs/dest1.txt";
    HmdfsInfo hmdfsInfo;
    EXPECT_EQ(daemon_->InnerCopy(srcUri, destUri, "", nullptr, hmdfsInfo), ERR_BAD_VALUE);

    srcUri = "file://docs/storage/media/100/local/files/Docs/../A/1.txt?networkid=123456";
    EXPECT_EQ(daemon_->InnerCopy(srcUri, destUri, "", nullptr, hmdfsInfo), ERR_BAD_VALUE);

    srcUri = "file://docs/storage/media/100/local/files/Docs/1.txt";
    destUri = "file://docs/storage/media/100/local/files/Docs/../A/dest1.txt";
    EXPECT_EQ(daemon_->InnerCopy(srcUri, destUri, "", nullptr, hmdfsInfo), ERR_BAD_VALUE);

    destUri = "file://docs/storage/media/100/local/files/Docs/../A/dest1.txt?networkid=123456";
    EXPECT_EQ(daemon_->InnerCopy(srcUri, destUri, "", nullptr, hmdfsInfo), ERR_BAD_VALUE);
    GTEST_LOG_(INFO) << "DaemonTest_InnerCopy_001 end";
}

/**
 * @tc.name: DaemonTest_DisconnectByRemote_001
 * @tc.desc: verify DisconnectByRemote with invalid networkId
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_DisconnectByRemote_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_DisconnectByRemote_001 begin";
    ASSERT_NE(daemon_, nullptr);
    // Test empty networkId
    EXPECT_NO_THROW(daemon_->DisconnectByRemote(""));

    // Test invalid networkId length
    std::string longNetworkId(DM_MAX_DEVICE_ID_LEN + 1, 'a');
    EXPECT_NO_THROW(daemon_->DisconnectByRemote(longNetworkId));

    GTEST_LOG_(INFO) << "DaemonTest_DisconnectByRemote_001 end";
}

/**
 * @tc.name: DaemonTest_DisconnectByRemote_002
 * @tc.desc: verify DisconnectByRemote with UMountDfsDocs failed
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_DisconnectByRemote_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_DisconnectByRemote_002 begin";
    ASSERT_NE(daemon_, nullptr);
    // Test normal case with mock
    EXPECT_CALL(*deviceManagerAgentMock_, UMountDfsDocs(_, _, _)).WillOnce(Return(ERR_BAD_VALUE));
    EXPECT_NO_THROW(daemon_->DisconnectByRemote("validNetworkId"));

    GTEST_LOG_(INFO) << "DaemonTest_DisconnectByRemote_002 end";
}

/**
 * @tc.name: DaemonTest_DisconnectByRemote_003
 * @tc.desc: verify DisconnectByRemote with UMountDfsDocs success
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_DisconnectByRemote_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_DisconnectByRemote_003 begin";
    ASSERT_NE(daemon_, nullptr);
    // Test normal case with mock
    EXPECT_CALL(*deviceManagerAgentMock_, UMountDfsDocs(_, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_CALL(*deviceManagerAgentMock_, OnDeviceP2POffline(_)).WillOnce(Return(NO_ERROR));
    EXPECT_NO_THROW(daemon_->DisconnectByRemote("validNetworkId"));

    GTEST_LOG_(INFO) << "DaemonTest_DisconnectByRemote_003 end";
}

/**
 * @tc.name: DaemonTest_CreateControlLink_001
 * @tc.desc: verify CreatControlLink and CancelConrolLink
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_CreateControlLink_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_CreateControlLink_001 begin";
    ASSERT_NE(daemon_, nullptr);
    // Test exist channel now
    EXPECT_CALL(*channelManagerMock_, HasExistChannel(_)).WillOnce(Return(true));
    EXPECT_EQ(daemon_->CreatControlLink("networkId"), FileManagement::ERR_OK);

    // Test no exist channel and not support with return ok
    EXPECT_CALL(*channelManagerMock_, HasExistChannel(_)).WillOnce(Return(false));
    EXPECT_EQ(daemon_->CreatControlLink("networkId"), FileManagement::ERR_OK);

    // Test no exist channel and support create but renturn failed
    g_dfsVersion = {6, 0, 0};
    EXPECT_CALL(*channelManagerMock_, HasExistChannel(_)).WillOnce(Return(false));
    EXPECT_CALL(*channelManagerMock_, CreateClientChannel(_)).WillOnce(Return(FileManagement::ERR_BAD_VALUE));
    EXPECT_EQ(daemon_->CreatControlLink("networkId"), FileManagement::ERR_BAD_VALUE);

    // Test no exist channel and support create but renturn success
    EXPECT_CALL(*channelManagerMock_, HasExistChannel(_)).WillOnce(Return(false));
    EXPECT_CALL(*channelManagerMock_, CreateClientChannel(_)).WillOnce(Return(FileManagement::ERR_OK));
    EXPECT_EQ(daemon_->CreatControlLink("networkId"), FileManagement::ERR_OK);

    GTEST_LOG_(INFO) << "DaemonTest_CreateControlLink_001 end";
}

/**
 * @tc.name: DaemonTest_DestroyControlLink_001
 * @tc.desc: verify CreatControlLink and CancelConrolLink
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_DestroyControlLink_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_DestroyControlLink_001 begin";
    ASSERT_NE(daemon_, nullptr);
    // Test not existing channel
    EXPECT_CALL(*channelManagerMock_, HasExistChannel(_)).WillOnce(Return(false));
    EXPECT_EQ(daemon_->CancelControlLink("networkId"), FileManagement::ERR_OK);

    // Test existing channel and destroy failed
    EXPECT_CALL(*channelManagerMock_, HasExistChannel(_)).WillOnce(Return(true));
    EXPECT_CALL(*channelManagerMock_, DestroyClientChannel(_)).WillOnce(Return(FileManagement::ERR_BAD_VALUE));
    EXPECT_EQ(daemon_->CancelControlLink("networkId"), FileManagement::ERR_BAD_VALUE);

    // Test existing channel and destroy success
    EXPECT_CALL(*channelManagerMock_, HasExistChannel(_)).WillOnce(Return(true));
    EXPECT_CALL(*channelManagerMock_, DestroyClientChannel(_)).WillOnce(Return(FileManagement::ERR_OK));
    EXPECT_EQ(daemon_->CancelControlLink("networkId"), FileManagement::ERR_OK);

    GTEST_LOG_(INFO) << "DaemonTest_DestroyControlLink_001 end";
}

/**
 * @tc.name: DaemonTest_CheckRemoteAllowConnect_001
 * @tc.desc: verify CheckRemoteAllowConnect and notification functions
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_CheckRemoteAllowConnect_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_CheckRemoteAllowConnect_001";
    // Test remoteDfs version is lower
    g_isRemoteDfsVersionLowerThanGiven = true;
    EXPECT_EQ(daemon_->CheckRemoteAllowConnect("networkId"), FileManagement::ERR_VERSION_NOT_SUPPORT);

    // Test getLocalNetworkId failed
    g_isRemoteDfsVersionLowerThanGiven = false;
    EXPECT_CALL(*deviceManagerImplMock_, GetLocalDeviceNetWorkId(_, _)).WillOnce(Return(FileManagement::ERR_BAD_VALUE));
    EXPECT_EQ(daemon_->CheckRemoteAllowConnect("networkId"), FileManagement::ERR_BAD_VALUE);

    // Test SendRequest failed
    EXPECT_CALL(*deviceManagerImplMock_, GetLocalDeviceNetWorkId(_, _)).WillRepeatedly(Return(E_OK));
    EXPECT_CALL(*channelManagerMock_, HasExistChannel(_)).WillOnce(Return(false));
    EXPECT_CALL(*channelManagerMock_, CreateClientChannel(_)).WillOnce(Return(FileManagement::ERR_BAD_VALUE));
    EXPECT_EQ(daemon_->CheckRemoteAllowConnect("networkId"), FileManagement::ERR_BAD_VALUE);

    // Test CheckRemoteAllowConnect success
    EXPECT_CALL(*channelManagerMock_, HasExistChannel(_)).WillOnce(Return(true));
    EXPECT_CALL(*channelManagerMock_, SendRequest(_, _, _, _)).WillOnce(Return(FileManagement::ERR_OK));
    EXPECT_EQ(daemon_->CheckRemoteAllowConnect("networkId"), FileManagement::ERR_OK);

    GTEST_LOG_(INFO) << "DaemonTest_CheckRemoteAllowConnect_001";
}

/**
 * @tc.name: DaemonTest_NotifyRemotePublishNotification_001
 * @tc.desc: verify NotifyRemotePublishNotification
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_NotifyRemotePublishNotification_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_NotifyRemotePublishNotification_001";
    // Test remoteDfs version is lower
    g_isRemoteDfsVersionLowerThanGiven = true;
    EXPECT_EQ(daemon_->NotifyRemotePublishNotification("networkId"), FileManagement::ERR_VERSION_NOT_SUPPORT);

    // Test CreatControlLink failed
    g_isRemoteDfsVersionLowerThanGiven = false;
    g_dfsVersion = {6, 0, 0};
    EXPECT_CALL(*channelManagerMock_, HasExistChannel(_)).WillOnce(Return(false));
    EXPECT_CALL(*channelManagerMock_, CreateClientChannel(_)).WillOnce(Return(FileManagement::ERR_BAD_VALUE));
    EXPECT_EQ(daemon_->NotifyRemotePublishNotification("networkId"), FileManagement::ERR_BAD_VALUE);

    // Test SendRequest failed
    EXPECT_CALL(*channelManagerMock_, HasExistChannel(_)).WillOnce(Return(true));
    EXPECT_CALL(*channelManagerMock_, SendRequest(_, _, _, _)).WillOnce(Return(FileManagement::ERR_BAD_VALUE));
    EXPECT_EQ(daemon_->NotifyRemotePublishNotification("networkId"), FileManagement::ERR_BAD_VALUE);

    // Test CheckRemoteAllowConnect success
    EXPECT_CALL(*channelManagerMock_, HasExistChannel(_)).WillOnce(Return(true));
    EXPECT_CALL(*channelManagerMock_, SendRequest(_, _, _, _)).WillOnce(Return(FileManagement::ERR_OK));
    EXPECT_EQ(daemon_->NotifyRemotePublishNotification("networkId"), FileManagement::ERR_OK);

    GTEST_LOG_(INFO) << "DaemonTest_NotifyRemotePublishNotification_001";
}

/**
 * @tc.name: DaemonTest_NotifyRemoteCancelNotification_001
 * @tc.desc: verify NotifyRemoteCancelNotification and notification functions
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_NotifyRemoteCancelNotification_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_NotifyRemoteCancelNotification_001";
    // Test remoteDfs version is lower
    g_isRemoteDfsVersionLowerThanGiven = true;
    EXPECT_EQ(daemon_->NotifyRemoteCancelNotification("networkId"), FileManagement::ERR_VERSION_NOT_SUPPORT);

    // Test CreatControlLink failed
    g_isRemoteDfsVersionLowerThanGiven = false;
    g_dfsVersion = {6, 0, 0};
    EXPECT_CALL(*channelManagerMock_, HasExistChannel(_)).WillOnce(Return(false));
    EXPECT_CALL(*channelManagerMock_, CreateClientChannel(_)).WillOnce(Return(FileManagement::ERR_BAD_VALUE));
    EXPECT_EQ(daemon_->NotifyRemoteCancelNotification("networkId"), FileManagement::ERR_BAD_VALUE);

    // Test SendRequest failed
    EXPECT_CALL(*channelManagerMock_, HasExistChannel(_)).WillOnce(Return(true));
    EXPECT_CALL(*channelManagerMock_, SendRequest(_, _, _, _)).WillOnce(Return(FileManagement::ERR_BAD_VALUE));
    EXPECT_EQ(daemon_->NotifyRemoteCancelNotification("networkId"), FileManagement::ERR_BAD_VALUE);

    // Test CheckRemoteAllowConnect success
    EXPECT_CALL(*channelManagerMock_, HasExistChannel(_)).WillOnce(Return(true));
    EXPECT_CALL(*channelManagerMock_, SendRequest(_, _, _, _)).WillOnce(Return(FileManagement::ERR_OK));
    EXPECT_EQ(daemon_->NotifyRemoteCancelNotification("networkId"), FileManagement::ERR_OK);

    GTEST_LOG_(INFO) << "DaemonTest_NotifyRemoteCancelNotification_001";
}

/**
 * @tc.name: DaemonTest_IsSameAccountDevice_001
 * @tc.desc: verify IsSameAccountDevice with same account device.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_IsSameAccountDevice_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_IsSameAccountDevice_001 begin";
#ifdef SUPPORT_SAME_ACCOUNT
    bool isSameAccount = false;
    std::vector<DistributedHardware::DmDeviceInfo> deviceList;
    deviceList.push_back(deviceInfo);

    // Test same account device
    EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0)));

    int32_t result = daemon_->IsSameAccountDevice(NETWORKID_ONE, isSameAccount);
    EXPECT_EQ(result, E_OK);
    EXPECT_TRUE(isSameAccount);
    GTEST_LOG_(INFO) << "DaemonTest_IsSameAccountDevice_001 end";
#else
    GTEST_LOG_(INFO) << "DaemonTest_IsSameAccountDevice_001 not support!";
#endif
}

/**
 * @tc.name: DaemonTest_IsSameAccountDevice_002
 * @tc.desc: verify IsSameAccountDevice with different account device.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_IsSameAccountDevice_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_IsSameAccountDevice_002 begin";
#ifdef SUPPORT_SAME_ACCOUNT
    bool isSameAccount = false;
    std::vector<DistributedHardware::DmDeviceInfo> deviceList;

    // Create device with different account
    DistributedHardware::DmDeviceInfo diffAccountDevice = deviceInfo;
    diffAccountDevice.authForm = DistributedHardware::DmAuthForm::ACROSS_ACCOUNT;
    deviceList.push_back(diffAccountDevice);

    // Test different account device
    EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0)));

    int32_t result = daemon_->IsSameAccountDevice(NETWORKID_ONE, isSameAccount);
    EXPECT_EQ(result, E_OK);
    EXPECT_FALSE(isSameAccount);

    GTEST_LOG_(INFO) << "DaemonTest_IsSameAccountDevice_002 end";
#else
    GTEST_LOG_(INFO) << "DaemonTest_IsSameAccountDevice_002 not support!";
#endif
}

/**
 * @tc.name: DaemonTest_IsSameAccountDevice_003
 * @tc.desc: verify IsSameAccountDevice with empty device list.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_IsSameAccountDevice_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_IsSameAccountDevice_003 begin";
#ifdef SUPPORT_SAME_ACCOUNT
    bool isSameAccount = true;
    std::vector<DistributedHardware::DmDeviceInfo> deviceList;

    // Test empty device list
    EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0)));

    int32_t result = daemon_->IsSameAccountDevice(NETWORKID_ONE, isSameAccount);
    EXPECT_EQ(result, E_INVAL_ARG_NAPI);
    EXPECT_FALSE(isSameAccount);

    GTEST_LOG_(INFO) << "DaemonTest_IsSameAccountDevice_003 end";
#else
    GTEST_LOG_(INFO) << "DaemonTest_IsSameAccountDevice_003 not support!";
#endif
}

/**
 * @tc.name: DaemonTest_IsSameAccountDevice_004
 * @tc.desc: verify IsSameAccountDevice with device not in trusted list.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_IsSameAccountDevice_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_IsSameAccountDevice_004 begin";
#ifdef SUPPORT_SAME_ACCOUNT
    bool isSameAccount = true;
    std::vector<DistributedHardware::DmDeviceInfo> deviceList;

    // Create device with different networkId
    DistributedHardware::DmDeviceInfo differentDevice = deviceInfo;
    (void)memcpy_s(differentDevice.networkId, DM_MAX_DEVICE_NAME_LEN - 1,
        "different_network_id", strlen("different_network_id"));
    deviceList.push_back(differentDevice);

    // Test device not in trusted list
    EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0)));

    int32_t result = daemon_->IsSameAccountDevice(NETWORKID_ONE, isSameAccount);
    EXPECT_EQ(result, E_INVAL_ARG_NAPI);
    EXPECT_FALSE(isSameAccount);

    GTEST_LOG_(INFO) << "DaemonTest_IsSameAccountDevice_004 end";
#else
    GTEST_LOG_(INFO) << "DaemonTest_IsSameAccountDevice_004 not support!";
#endif
}

/**
 * @tc.name: DaemonTest_RegisterFileDfsListener_001
 * @tc.desc: verify RegisterFileDfsListener with valid parameters.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_RegisterFileDfsListener_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_RegisterFileDfsListener_001 begin";

    std::string instanceId = "test_instance";
    sptr<IFileDfsListener> listener = new FileDfsListenerMock();

    // Test valid parameters
    int32_t result = daemon_->RegisterFileDfsListener(instanceId, listener);
    EXPECT_EQ(result, E_OK);

    // Verify listener was added
    EXPECT_TRUE(ConnectCount::GetInstance().RmFileConnect(instanceId));

    GTEST_LOG_(INFO) << "DaemonTest_RegisterFileDfsListener_001 end";
}

/**
 * @tc.name: DaemonTest_RegisterFileDfsListener_002
 * @tc.desc: verify RegisterFileDfsListener with empty instanceId.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_RegisterFileDfsListener_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_RegisterFileDfsListener_002 begin";

    std::string emptyInstanceId = "";
    sptr<IFileDfsListener> listener = new FileDfsListenerMock();

    // Test empty instanceId
    int32_t result = daemon_->RegisterFileDfsListener(emptyInstanceId, listener);
    EXPECT_EQ(result, E_INVAL_ARG_NAPI);

    GTEST_LOG_(INFO) << "DaemonTest_RegisterFileDfsListener_002 end";
}

/**
 * @tc.name: DaemonTest_RegisterFileDfsListener_003
 * @tc.desc: verify RegisterFileDfsListener with null listener.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_RegisterFileDfsListener_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_RegisterFileDfsListener_003 begin";

    std::string instanceId = "test_instance";
    sptr<IFileDfsListener> nullListener = nullptr;

    // Test null listener
    int32_t result = daemon_->RegisterFileDfsListener(instanceId, nullListener);
    EXPECT_EQ(result, E_INVAL_ARG_NAPI);

    GTEST_LOG_(INFO) << "DaemonTest_RegisterFileDfsListener_003 end";
}

/**
 * @tc.name: DaemonTest_UnregisterFileDfsListener_001
 * @tc.desc: verify UnregisterFileDfsListener with valid instanceId.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_UnregisterFileDfsListener_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_UnregisterFileDfsListener_001 begin";

    std::string instanceId = "test_instance";
    sptr<IFileDfsListener> listener = new FileDfsListenerMock();

    // First register a listener
    daemon_->RegisterFileDfsListener(instanceId, listener);

    // Test unregister with valid instanceId
    int32_t result = daemon_->UnregisterFileDfsListener(instanceId);
    EXPECT_EQ(result, E_OK);

    GTEST_LOG_(INFO) << "DaemonTest_UnregisterFileDfsListener_001 end";
}

/**
 * @tc.name: DaemonTest_UnregisterFileDfsListener_002
 * @tc.desc: verify UnregisterFileDfsListener with empty instanceId.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_UnregisterFileDfsListener_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_UnregisterFileDfsListener_002 begin";

    std::string emptyInstanceId = "";

    // Test empty instanceId
    int32_t result = daemon_->UnregisterFileDfsListener(emptyInstanceId);
    EXPECT_EQ(result, E_INVAL_ARG_NAPI);

    GTEST_LOG_(INFO) << "DaemonTest_UnregisterFileDfsListener_002 end";
}

/**
 * @tc.name: DaemonTest_UnregisterFileDfsListener_003
 * @tc.desc: verify UnregisterFileDfsListener with non-existent instanceId.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_UnregisterFileDfsListener_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_UnregisterFileDfsListener_003 begin";

    std::string nonExistentInstanceId = "non_existent_instance";

    // Test non-existent instanceId
    int32_t result = daemon_->UnregisterFileDfsListener(nonExistentInstanceId);
    EXPECT_EQ(result, E_INVAL_ARG_NAPI);

    GTEST_LOG_(INFO) << "DaemonTest_UnregisterFileDfsListener_003 end";
}

/**
 * @tc.name: DaemonTest_GetDfsSwitchStatus_001
 * @tc.desc: verify GetDfsSwitchStatus with valid networkId.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_GetDfsSwitchStatus_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_GetDfsSwitchStatus_001 begin";

    // Test valid networkId with success
    g_getDeviceStatus = E_OK;
    int32_t status = 0;

    int32_t result = daemon_->GetDfsSwitchStatus(NETWORKID_ONE, status);
    EXPECT_EQ(result, E_OK);

    GTEST_LOG_(INFO) << "DaemonTest_GetDfsSwitchStatus_001 end";
}

/**
 * @tc.name: DaemonTest_GetDfsSwitchStatus_002
 * @tc.desc: verify GetDfsSwitchStatus with empty networkId.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_GetDfsSwitchStatus_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_GetDfsSwitchStatus_002 begin";

    std::string emptyNetworkId = "";
    int32_t status = 0;

    // Test empty networkId
    int32_t result = daemon_->GetDfsSwitchStatus(emptyNetworkId, status);
    EXPECT_EQ(result, E_INVAL_ARG_NAPI);

    GTEST_LOG_(INFO) << "DaemonTest_GetDfsSwitchStatus_002 end";
}

/**
 * @tc.name: DaemonTest_GetDfsSwitchStatus_003
 * @tc.desc: verify GetDfsSwitchStatus with long networkId.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_GetDfsSwitchStatus_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_GetDfsSwitchStatus_003 begin";

    std::string longNetworkId(DM_MAX_DEVICE_ID_LEN, 'a');
    int32_t status = 0;

    // Test long networkId
    int32_t result = daemon_->GetDfsSwitchStatus(longNetworkId, status);
    EXPECT_EQ(result, E_INVAL_ARG_NAPI);

    GTEST_LOG_(INFO) << "DaemonTest_GetDfsSwitchStatus_003 end";
}

/**
 * @tc.name: DaemonTest_GetDfsSwitchStatus_004
 * @tc.desc: verify GetDfsSwitchStatus with device not found.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_GetDfsSwitchStatus_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_GetDfsSwitchStatus_004 begin";

    // Test device not found
    g_getDeviceStatus = ERR_DP_CAN_NOT_FIND;
    int32_t status = 0;

    int32_t result = daemon_->GetDfsSwitchStatus(NETWORKID_ONE, status);
    EXPECT_EQ(result, ERR_DP_CAN_NOT_FIND);

    GTEST_LOG_(INFO) << "DaemonTest_GetDfsSwitchStatus_004 end";
}

/**
 * @tc.name: DaemonTest_GetDfsSwitchStatus_005
 * @tc.desc: verify GetDfsSwitchStatus with other error.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_GetDfsSwitchStatus_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_GetDfsSwitchStatus_005 begin";

    // Test other error
    g_getDeviceStatus = ERR_BAD_VALUE;
    int32_t status = 0;

    int32_t result = daemon_->GetDfsSwitchStatus(NETWORKID_ONE, status);
    EXPECT_EQ(result, ERR_BAD_VALUE);

    GTEST_LOG_(INFO) << "DaemonTest_GetDfsSwitchStatus_005 end";
}

/**
 * @tc.name: DaemonTest_UpdateDfsSwitchStatus_001
 * @tc.desc: verify UpdateDfsSwitchStatus with valid status.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_UpdateDfsSwitchStatus_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_UpdateDfsSwitchStatus_001 begin";

    // Test valid status with success
    g_putDeviceStatus = E_OK;

    int32_t result = daemon_->UpdateDfsSwitchStatus(1);
    EXPECT_EQ(result, E_OK);

    GTEST_LOG_(INFO) << "DaemonTest_UpdateDfsSwitchStatus_001 end";
}

/**
 * @tc.name: DaemonTest_UpdateDfsSwitchStatus_002
 * @tc.desc: verify UpdateDfsSwitchStatus with PutDeviceStatus failure.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_UpdateDfsSwitchStatus_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_UpdateDfsSwitchStatus_002 begin";

    // Test PutDeviceStatus failure
    g_putDeviceStatus = ERR_BAD_VALUE;

    int32_t result = daemon_->UpdateDfsSwitchStatus(1);
    EXPECT_EQ(result, ERR_BAD_VALUE);

    GTEST_LOG_(INFO) << "DaemonTest_UpdateDfsSwitchStatus_002 end";
}

/**
 * @tc.name: DaemonTest_UpdateDfsSwitchStatus_003
 * @tc.desc: verify UpdateDfsSwitchStatus with status 0 and cleanup.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_UpdateDfsSwitchStatus_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_UpdateDfsSwitchStatus_003 begin";

    // Prepare mount info
    MountCountInfo mountInfo(NETWORKID_ONE, 1);

    std::unordered_map<std::string, MountCountInfo> allMountInfo = {{"test", mountInfo}};

    // Test status 0 with cleanup
    g_putDeviceStatus = E_OK;
    EXPECT_CALL(*deviceManagerAgentMock_, GetAllMountInfo()).WillOnce(Return(allMountInfo));
    EXPECT_CALL(*deviceManagerAgentMock_, OnDeviceP2POffline(_)).WillOnce(Return(E_OK));

    int32_t result = daemon_->UpdateDfsSwitchStatus(0);
    EXPECT_NE(result, E_OK);

    GTEST_LOG_(INFO) << "DaemonTest_UpdateDfsSwitchStatus_003 end";
}

/**
 * @tc.name: DaemonTest_UpdateDfsSwitchStatus_004
 * @tc.desc: verify UpdateDfsSwitchStatus with status 0 and cleanup failure.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_UpdateDfsSwitchStatus_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_UpdateDfsSwitchStatus_004 begin";
    ASSERT_NE(daemon_, nullptr);

    // Prepare mount info
    MountCountInfo mountInfo(NETWORKID_ONE, 1);

    std::unordered_map<std::string, MountCountInfo> allMountInfo = {{"test", mountInfo}};

    // Test status 0 with cleanup failure
    g_putDeviceStatus = E_OK;
    EXPECT_CALL(*deviceManagerAgentMock_, GetAllMountInfo()).WillOnce(Return(allMountInfo));
    EXPECT_CALL(*deviceManagerAgentMock_, OnDeviceP2POffline(_)).WillOnce(Return(ERR_BAD_VALUE));

    int32_t result = daemon_->UpdateDfsSwitchStatus(0);
    EXPECT_EQ(result, E_CONNECTION_FAILED);

    GTEST_LOG_(INFO) << "DaemonTest_UpdateDfsSwitchStatus_004 end";
}

/**
 * @tc.name: DaemonTest_UpdateDfsSwitchStatus_005
 * @tc.desc: verify UpdateDfsSwitchStatus with status 0 and strcpy failure.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_UpdateDfsSwitchStatus_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_UpdateDfsSwitchStatus_005 begin";

    // Prepare mount info with very long networkId to cause strcpy failure
    MountCountInfo mountInfo(std::string(DM_MAX_DEVICE_ID_LEN + 1, 'a'), 1);

    std::unordered_map<std::string, MountCountInfo> allMountInfo = {{"test", mountInfo}};

    // Test status 0 with strcpy failure
    g_putDeviceStatus = E_OK;
    EXPECT_CALL(*deviceManagerAgentMock_, GetAllMountInfo()).WillOnce(Return(allMountInfo));

    int32_t result = daemon_->UpdateDfsSwitchStatus(0);
    EXPECT_EQ(result, E_CONNECTION_FAILED);

    GTEST_LOG_(INFO) << "DaemonTest_UpdateDfsSwitchStatus_005 end";
}

/**
 * @tc.name: DaemonTest_RequestSendFileACL_001
 * @tc.desc: verify RequestSendFileACL.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_RequestSendFileACL_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_RequestSendFileACL_001 begin";

    AccountInfo accountInfo;
    g_checkSinkPermission = false;
    EXPECT_EQ(daemon_->RequestSendFileACL("", "", "", "", accountInfo), ERR_ACL_FAILED);

    g_checkSinkPermission = true;
    EXPECT_NE(daemon_->RequestSendFileACL("", "", "", "", accountInfo), ERR_ACL_FAILED);

    GTEST_LOG_(INFO) << "DaemonTest_RequestSendFileACL_001 end";
}

/**
 * @tc.name: DaemonTest_GetRemoteCopyInfoACL_001
 * @tc.desc: verify GetRemoteCopyInfoACL.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonTest, DaemonTest_GetRemoteCopyInfoACL_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_GetRemoteCopyInfoACL_001 begin";

    AccountInfo accountInfo;
    bool isSrcFile = false;
    bool srcIsDir = false;

    g_checkSinkPermission = false;
    EXPECT_EQ(daemon_->GetRemoteCopyInfoACL("", isSrcFile, srcIsDir, accountInfo), ERR_ACL_FAILED);

    g_checkSinkPermission = true;
    EXPECT_CALL(*softBusSessionListenerMock_, GetRealPath(_)).WillOnce(Return("test"));
    EXPECT_NE(daemon_->GetRemoteCopyInfoACL("", isSrcFile, srcIsDir, accountInfo), ERR_ACL_FAILED);

    GTEST_LOG_(INFO) << "DaemonTest_RequestSendFileACL_001 end";
}
} // namespace Test
} // namespace OHOS::Storage::DistributedFile