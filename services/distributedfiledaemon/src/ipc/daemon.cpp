/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "ipc/daemon.h"

#include <exception>
#include <regex>
#include <stdexcept>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <unordered_set>

#include "accesstoken_kit.h"
#include "all_connect/all_connect_manager.h"
#include "asset_callback_manager.h"
#include "channel_manager.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "connect_count/connect_count.h"
#include "connection_detector.h"
#include "control_cmd_parser.h"
#include "copy/file_size_utils.h"
#include "copy/remote_file_copy_manager.h"
#include "device/device_manager_agent.h"
#include "device/device_profile_adapter.h"
#include "dfs_daemon_event_dfx.h"
#include "dfs_error.h"
#include "dfsu_access_token_helper.h"
#include "i_file_dfs_listener.h"
#include "ipc_skeleton.h"
#include "iremote_object.h"
#include "iservice_registry.h"
#include "mountpoint/mount_manager.h"
#include "network/devsl_dispatcher.h"
#include "network/softbus/softbus_handler.h"
#include "network/softbus/softbus_handler_asset.h"
#include "network/softbus/softbus_permission_check.h"
#include "network/softbus/softbus_session_dispatcher.h"
#include "network/softbus/softbus_session_listener.h"
#include "network/softbus/softbus_session_pool.h"
#include "radar_report.h"
#include "remote_file_share.h"
#include "sandbox_helper.h"
#include "system_ability_definition.h"
#include "system_notifier.h"
#include "trans_mananger.h"
#include "utils_directory.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
using namespace std;
using namespace OHOS::AppFileService;
using namespace OHOS::FileManagement;
using namespace OHOS::Storage::DistributedFile;
using FileManagement::ERR_OK;

using HapTokenInfo = OHOS::Security::AccessToken::HapTokenInfo;
using AccessTokenKit = OHOS::Security::AccessToken::AccessTokenKit;

namespace {
const string FILE_MANAGER_AUTHORITY = "docs";
const string MEDIA_AUTHORITY = "media";
const int32_t E_INVAL_ARG_NAPI = 401;
const int32_t E_CONNECTION_FAILED = 13900045;
const int32_t E_UNMOUNT = 13600004;
const int32_t PASTEBOARDUSERID = 3816;
const int32_t UDMFUSERID = 3012;
constexpr mode_t DEFAULT_UMASK = 0002;
constexpr int32_t BLOCK_INTERVAL_SEND_FILE = 10 * 1000;
constexpr int32_t DEFAULT_USER_ID = 100;
constexpr int32_t VALID_MOUNT_NETWORKID_LEN = 16;
constexpr uint64_t INNER_COPY_LIMIT = 1024 * 1024 * 1024;
constexpr int32_t ERR_DP_CAN_NOT_FIND = 98566199;
constexpr int32_t DEFAULT_DP_CANNOTFIND_VALUE = -1;
constexpr const char* HMDFS_FATH = "/storage/hmdfs/";
constexpr DfsVersion FILEMANAGER_VERSION = {6, 0, 1};
constexpr int32_t MIN_NETWORKID_LENGTH = 16;
constexpr int32_t MAX_IPC_THREAD_NUM = 32;
} // namespace

REGISTER_SYSTEM_ABILITY_BY_ID(Daemon, FILEMANAGEMENT_DISTRIBUTED_FILE_DAEMON_SA_ID, true);

static int32_t QueryActiveUserId()
{
    std::vector<int32_t> ids;
    ErrCode errCode = AccountSA::OsAccountManager::QueryActiveOsAccountIds(ids);
    if (errCode != NO_ERROR || ids.empty()) {
        LOGE("Query active userid failed, errCode: %{public}d, ", errCode);
        return DEFAULT_USER_ID;
    }
    return ids[0];
}

void Daemon::PublishSA()
{
    LOGI("Begin to init");
    if (!registerToService_) {
        bool ret = SystemAbility::Publish(this);
        if (!ret) {
            throw runtime_error("Failed to publish the daemon");
        }
        registerToService_ = true;
    }
    LOGI("Init finished successfully");
}

void Daemon::RegisterOsAccount()
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_USER_UNLOCKED);
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subScriber_ = std::make_shared<OsAccountObserver>(subscribeInfo);
    bool subRet = EventFwk::CommonEventManager::SubscribeCommonEvent(subScriber_);
    if (!subRet) {
        LOGE("Subscribe common event failed");
    }
}

void Daemon::OnStart()
{
    LOGI("Begin to start service");
    if (state_ == ServiceRunningState::STATE_RUNNING) {
        LOGD("Daemon has already started");
        return;
    }

    try {
        PublishSA();
        StartEventHandler();
        AddSystemAbilityListener(COMMON_EVENT_SERVICE_ID);
        AddSystemAbilityListener(SOFTBUS_SERVER_SA_ID);
        umask(DEFAULT_UMASK);
        AllConnectManager::GetInstance().InitAllConnectManager();
        IPCSkeleton::SetMaxWorkThreadNum(MAX_IPC_THREAD_NUM);
    } catch (const exception &e) {
        LOGE("%{public}s", e.what());
    }

    state_ = ServiceRunningState::STATE_RUNNING;
    RadarReportAdapter::GetInstance().InitRadar();
    LOGI("Start service successfully");
}

void Daemon::OnStop()
{
    LOGI("Begin to stop");
    state_ = ServiceRunningState::STATE_NOT_START;
    registerToService_ = false;
    bool subRet = EventFwk::CommonEventManager::UnSubscribeCommonEvent(subScriber_);
    if (!subRet) {
        LOGE("UnSubscribe common event failed");
    }
    subScriber_ = nullptr;
    daemonExecute_ = nullptr;
    eventHandler_ = nullptr;
    DeviceManagerAgent::GetInstance()->UMountDisShareFile();
    SoftBusHandlerAsset::GetInstance().DeleteAssetLocalSessionServer();
    AllConnectManager::GetInstance().UnInitAllConnectManager();
    RadarReportAdapter::GetInstance().ReportDfxStatistics();
    RadarReportAdapter::GetInstance().UnInitRadar();
    LOGI("Stop finished successfully");
}

void Daemon::OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    if (systemAbilityId == COMMON_EVENT_SERVICE_ID) {
        LOGI("Daemon::OnAddSystemAbility common event service online");
        if (subScriber_ == nullptr) {
            RegisterOsAccount();
        }
    } else if (systemAbilityId == SOFTBUS_SERVER_SA_ID) {
        SoftBusHandlerAsset::GetInstance().CreateAssetLocalSessionServer();
        ChannelManager::GetInstance().Init();
        ControlCmdParser::RegisterDisconnectCallback([this](std::string networkId) {
            LOGE("callback networkid is %{public}.6s", networkId.c_str());
            this->DisconnectByRemote(networkId);
        });
    }
}

void Daemon::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    (void)deviceId;
    if (systemAbilityId == COMMON_EVENT_SERVICE_ID) {
        if (subScriber_ == nullptr) {
            LOGE("Daemon::OnRemoveSystemAbility subscriberPtr is nullptr");
            return;
        }
        LOGI("OnRemoveSystemAbility common event service offline");
    } else if (systemAbilityId == SOFTBUS_SERVER_SA_ID) {
        SoftBusHandlerAsset::GetInstance().DeleteAssetLocalSessionServer();
        ChannelManager::GetInstance().DeInit();
    }
}

int32_t Daemon::ConnectDfs(const std::string &networkId)
{
    std::lock_guard<std::mutex> lock(connectMutex_);
    RadarParaInfo info = {"ConnectDfs", ReportLevel::DEFAULT, DfxBizStage::SOFTBUS_OPENP2P,
        DEFAULT_PKGNAME, networkId, E_OK, "ConnectDfs Begin"};
    RadarReportAdapter::GetInstance().ReportLinkConnectionAdapter(info);
    uint32_t callingTokenId = IPCSkeleton::GetCallingTokenID();
    if (networkId.length() < MIN_NETWORKID_LENGTH || networkId.length() >= DM_MAX_DEVICE_ID_LEN) {
        LOGE("Daemon::ConnectDfs networkId length is invalid.");
        return E_INVAL_ARG_NAPI;
    }
    DistributedHardware::DmDeviceInfo deviceInfo;
    auto res = strcpy_s(deviceInfo.networkId, DM_MAX_DEVICE_ID_LEN, networkId.c_str());
    if (res != 0) {
        LOGE("ConnectDfs strcpy failed, res = %{public}d, errno = %{public}d", res, errno);
        return E_INVAL_ARG_NAPI;
    }

    LOGI("ConnectDfs networkId %{public}.6s, callingTokenId %{public}s", networkId.c_str(),
         Utils::GetAnonyNumber(callingTokenId).c_str());
    RADAR_REPORT(RadarReporter::DFX_SET_DFS, RadarReporter::DFX_BUILD__LINK, RadarReporter::DFX_SUCCESS,
                 RadarReporter::BIZ_STATE, RadarReporter::DFX_BEGIN);
    sptr<IFileDfsListener> listener = nullptr;
    int32_t ret = ConnectionCount(deviceInfo);
    if (ret == E_OK) {
        LOGI("ConnectDfs Success %{public}s", Utils::GetAnonyString(networkId).c_str());
        ConnectCount::GetInstance().AddConnect(callingTokenId, networkId, listener);
    } else {
        if (ret == ERR_CHECKOUT_COUNT) {
            ConnectCount::GetInstance().RemoveConnect(callingTokenId, networkId);
        }
        LOGE("ConnectDfs failed %{public}s", Utils::GetAnonyString(networkId).c_str());
        info = {"ConnectDfs", ReportLevel::INTERFACE, DfxBizStage::SOFTBUS_OPENP2P,
            DEFAULT_PKGNAME, networkId, ret, "OpenP2PConnection failed"};
        RadarReportAdapter::GetInstance().ReportLinkConnectionAdapter(info);
        CleanUp(networkId);
    }
    RadarReportAdapter::GetInstance().SetUserStatistics(ret == E_OK ? CONNECT_DFS_SUCC_CNT : CONNECT_DFS_FAIL_CNT);
    return ret;
}

int32_t Daemon::DisconnectDfs(const std::string &networkId)
{
    std::lock_guard<std::mutex> lock(connectMutex_);
    RadarParaInfo info = {"DisconnectDfs", ReportLevel::DEFAULT, DfxBizStage::SOFTBUS_CLOSEP2P,
        DEFAULT_PKGNAME, networkId, E_OK, "DisconnectDfs Begin"};
    RadarReportAdapter::GetInstance().ReportLinkConnectionAdapter(info);
    uint32_t callingTokenId = IPCSkeleton::GetCallingTokenID();
    LOGI("DisconnectDfs networkId %{public}.6s, callingTokenId %{public}s", networkId.c_str(),
         Utils::GetAnonyNumber(callingTokenId).c_str());
    ConnectCount::GetInstance().RemoveConnect(callingTokenId, networkId);
    CleanUp(networkId);
    return 0;
}

int32_t Daemon::ConnectionCount(const DistributedHardware::DmDeviceInfo &deviceInfo)
{
    auto path = ConnectionDetector::ParseHmdfsPath();
    stringstream ss;
    auto st_dev = ConnectionDetector::MocklispHash(path);
    if (st_dev == static_cast<uint64_t>(FileManagement::ERR_BAD_VALUE)) {
        return st_dev;
    }
    ss << st_dev;
    auto targetDir = ss.str();
    auto networkId = std::string(deviceInfo.networkId);
    int32_t ret = 0;
    if (!ConnectCount::GetInstance().CheckCount(networkId)) {
        ret = DeviceManagerAgent::GetInstance()->OnDeviceP2POnline(deviceInfo);
        DevslDispatcher::GetDeviceDevsl(networkId);
        if (ret == NO_ERROR) {
            ret = ConnectionDetector::RepeatGetConnectionStatus(targetDir, networkId);
        }
    } else {
        if (ConnectionDetector::RepeatGetConnectionStatus(targetDir, networkId) != E_OK) {
            ret = ERR_CHECKOUT_COUNT;
        }
    }
    if (ret == E_OK) {
        RADAR_REPORT(RadarReporter::DFX_SET_DFS, RadarReporter::DFX_BUILD__LINK, RadarReporter::DFX_SUCCESS,
                     RadarReporter::BIZ_STATE, RadarReporter::DFX_BEGIN);
        LOGI("RepeatGetConnectionStatus end, ret = %{public}d", ret);
    } else {
        RADAR_REPORT(RadarReporter::DFX_SET_DFS, RadarReporter::DFX_BUILD__LINK, RadarReporter::DFX_FAILED,
                     RadarReporter::BIZ_STATE, RadarReporter::DFX_BEGIN);
        LOGE("ConnectDfs failed, ret = %{public}d", ret);
        RadarParaInfo info = {"ConnectionCount", ReportLevel::INNER, DfxBizStage::SOFTBUS_OPENP2P,
            DEFAULT_PKGNAME, deviceInfo.networkId, ret, "ConnectDfs failed"};
        RadarReportAdapter::GetInstance().ReportLinkConnectionAdapter(info);
    }
    return ret;
}

int32_t Daemon::CleanUp(const std::string &networkId)
{
    LOGI("CleanUp start, networkId is %{public}.6s", networkId.c_str());
    if (!ConnectCount::GetInstance().CheckCount(networkId)) {
        DisconnectDevice(networkId);
        auto ret = CancelControlLink(networkId);
        LOGI("cancel control link ret is %{public}d", ret);
        return ret;
    }
    return E_OK;
}

int32_t Daemon::ConnectionAndMount(const DistributedHardware::DmDeviceInfo &deviceInfo,
                                   const std::string &networkId,
                                   sptr<IFileDfsListener> remoteReverseObj)
{
    LOGI("ConnectionAndMount start");
    bool hasFileAccessManager = DfsuAccessTokenHelper::CheckCallerPermission(FILE_ACCESS_MANAGER_PERMISSION);
    if (hasFileAccessManager && CheckRemoteAllowConnect(networkId) != NO_ERROR) {
        LOGE("CheckRemoteAllowConnect failed for %{public}.6s", networkId.c_str());
        RadarParaInfo info = {"ConnectionAndMount", ReportLevel::INNER, DfxBizStage::SOFTBUS_OPENP2P,
            DEFAULT_PKGNAME, networkId, E_PERMISSION, "permission not support"};
        RadarReportAdapter::GetInstance().ReportLinkConnectionAdapter(info);
        return E_PERMISSION;
    }

    auto callingTokenId = IPCSkeleton::GetCallingTokenID();
    auto ret = ConnectionCount(deviceInfo);
    if (ret != NO_ERROR) {
        LOGE("connection failed");
        if (ret == ERR_CHECKOUT_COUNT) {
            ConnectCount::GetInstance().RemoveConnect(callingTokenId, networkId);
        }
        return ret;
    }

    ConnectCount::GetInstance().AddConnect(callingTokenId, networkId, remoteReverseObj);

    if (!hasFileAccessManager) {
        LOGW("permission denied: FILE_ACCESS_MANAGER_PERMISSION");
        return ret;
    }
    std::string mountPath = networkId.substr(0, VALID_MOUNT_NETWORKID_LEN);
    auto deviceManager = DeviceManagerAgent::GetInstance();
    ret = deviceManager->MountDfsDocs(networkId, mountPath, callingTokenId);
    if (ret != NO_ERROR) {
        ConnectCount::GetInstance().RemoveConnect(callingTokenId, networkId);
        LOGE("[MountDfsDocs] failed, ret is %{public}d", ret);
        return ret;
    }
    ConnectCount::GetInstance().NotifyFileStatusChange(networkId, Status::CONNECT_OK, HMDFS_FATH + mountPath,
                                                       StatusType::CONNECTION_STATUS);
    NotifyRemotePublishNotification(networkId);
    return ret;
}

int32_t Daemon::CheckPermission(const std::string &networkId)
{
    LOGI("Daemon::CheckPermission start, networkId %{public}.6s", networkId.c_str());
    if (DfsuAccessTokenHelper::CheckCallerPermission(FILE_ACCESS_MANAGER_PERMISSION) &&
        ControlCmdParser::IsLocalItDevice()) {
        LOGE("FILE_ACCESS_MANAGER_PERMISSION permission has not support it situation");
        RadarParaInfo info = {"OpenP2PConnectionEx", ReportLevel::INTERFACE, DfxBizStage::SOFTBUS_OPENP2P,
            DEFAULT_PKGNAME, networkId, E_PERMISSION, "permission not support"};
        RadarReportAdapter::GetInstance().ReportLinkConnectionAdapter(info);
        RadarReportAdapter::GetInstance().SetUserStatistics(CONNECT_DFS_FAIL_CNT);
        return E_PERMISSION;
    }

    if (!DfsuAccessTokenHelper::CheckCallerPermission(PERM_DISTRIBUTED_DATASYNC)) {
        LOGE("[CheckPermission] DATASYNC permission denied");
        RadarParaInfo info = {"OpenP2PConnectionEx", ReportLevel::INTERFACE, DfxBizStage::SOFTBUS_OPENP2P,
            DEFAULT_PKGNAME, networkId, E_PERMISSION, "permission not support"};
        RadarReportAdapter::GetInstance().ReportLinkConnectionAdapter(info);
        RadarReportAdapter::GetInstance().SetUserStatistics(CONNECT_DFS_FAIL_CNT);
        return E_PERMISSION;
    }

    if (networkId.length() < MIN_NETWORKID_LENGTH || networkId.length() >= DM_MAX_DEVICE_ID_LEN) {
        LOGE("Daemon::OpenP2PConnectionEx networkId length is invalid.");
        RadarParaInfo info = {"OpenP2PConnectionEx", ReportLevel::INTERFACE, DfxBizStage::SOFTBUS_OPENP2P,
            DEFAULT_PKGNAME, networkId, E_INVAL_ARG_NAPI, "networkId length is invalid"};
        RadarReportAdapter::GetInstance().ReportLinkConnectionAdapter(info);
        RadarReportAdapter::GetInstance().SetUserStatistics(CONNECT_DFS_FAIL_CNT);
        return E_INVAL_ARG_NAPI;
    }
    return E_OK;
}

int32_t Daemon::OpenP2PConnectionEx(const std::string &networkId, sptr<IFileDfsListener> remoteReverseObj)
{
    LOGI("Daemon::OpenP2PConnectionEx start, networkId %{public}.6s", networkId.c_str());
    RadarParaInfo info = {"OpenP2PConnectionEx", ReportLevel::DEFAULT, DfxBizStage::SOFTBUS_OPENP2P,
        DEFAULT_PKGNAME, networkId, E_OK, "OpenP2PConnectionEx Begin"};
    RadarReportAdapter::GetInstance().ReportLinkConnectionAdapter(info);
    int32_t ret = CheckPermission(networkId);
    if (ret != E_OK) {
        LOGE("permission denied");
        return ret;
    }
    std::lock_guard<std::mutex> lock(connectMutex_);
    if (remoteReverseObj != nullptr) {
        LOGI("Daemon::OpenP2PConnectionEx remoteReverseObj is not nullptr");
        if (dfsListenerDeathRecipient_ == nullptr) {
            LOGI("Daemon::OpenP2PConnectionEx, new death recipient");
            dfsListenerDeathRecipient_ = new (std::nothrow) DfsListenerDeathRecipient();
        }
        if (dfsListenerDeathRecipient_ == nullptr) {
            LOGE("Daemon::OpenP2PConnectionEx failed to allocate memory for dfsListenerDeathRecipient_");
            info = {"OpenP2PConnectionEx", ReportLevel::INTERFACE, DfxBizStage::SOFTBUS_OPENP2P,
                DEFAULT_PKGNAME, networkId, E_INVAL_ARG_NAPI, "OpenP2PConnectionEx failed"};
            RadarReportAdapter::GetInstance().ReportLinkConnectionAdapter(info);
            RadarReportAdapter::GetInstance().SetUserStatistics(CONNECT_DFS_FAIL_CNT);
            return E_INVAL_ARG_NAPI;
        }
        remoteReverseObj->AsObject()->AddDeathRecipient(dfsListenerDeathRecipient_);
    }

    DistributedHardware::DmDeviceInfo deviceInfo{};
    auto res = strcpy_s(deviceInfo.networkId, DM_MAX_DEVICE_ID_LEN, networkId.c_str());
    if (res != 0) {
        LOGE("OpenP2PConnectionEx strcpy failed, res = %{public}d, errno = %{public}d", res, errno);
        info = {"OpenP2PConnectionEx", ReportLevel::INTERFACE, DfxBizStage::SOFTBUS_OPENP2P,
            DEFAULT_PKGNAME, networkId, res, "strcpy_s failed"};
        RadarReportAdapter::GetInstance().ReportLinkConnectionAdapter(info);
        RadarReportAdapter::GetInstance().SetUserStatistics(CONNECT_DFS_FAIL_CNT);
        return E_INVAL_ARG_NAPI;
    }

    ret = ConnectionAndMount(deviceInfo, networkId, remoteReverseObj);
    RadarReportAdapter::GetInstance().SetUserStatistics(ret == E_OK ? CONNECT_DFS_SUCC_CNT : CONNECT_DFS_FAIL_CNT);
    if (ret != NO_ERROR) {
        LOGE("ConnectionAndMount ret is %{public}d", ret);
        info = {"OpenP2PConnectionEx", ReportLevel::INTERFACE, DfxBizStage::SOFTBUS_OPENP2P,
            DEFAULT_PKGNAME, networkId, ret, "ConnectionAndMount failed"};
        RadarReportAdapter::GetInstance().ReportLinkConnectionAdapter(info);
        CleanUp(networkId);
        return E_CONNECTION_FAILED;
    }
    LOGI("Daemon::OpenP2PConnectionEx end");
    return ret;
}

int32_t Daemon::CloseP2PConnectionEx(const std::string &networkId)
{
    LOGI("Daemon::CloseP2PConnectionEx start, networkId: %{public}.6s", networkId.c_str());
    RadarParaInfo info = {"CloseP2PConnectionEx", ReportLevel::DEFAULT, DfxBizStage::SOFTBUS_CLOSEP2P,
        DEFAULT_PKGNAME, networkId, E_OK, "CloseP2PConnectionEx Begin"};
    RadarReportAdapter::GetInstance().ReportLinkConnectionAdapter(info);
    if (DfsuAccessTokenHelper::CheckCallerPermission(FILE_ACCESS_MANAGER_PERMISSION) &&
        ControlCmdParser::IsLocalItDevice()) {
        LOGW("FILE_ACCESS_MANAGER_PERMISSION permission has not support it situation");
        info = {"CloseP2PConnectionEx", ReportLevel::INNER, DfxBizStage::SOFTBUS_CLOSEP2P,
            DEFAULT_PKGNAME, networkId, E_PERMISSION, "permission denied"};
        RadarReportAdapter::GetInstance().ReportLinkConnectionAdapter(info);
        return E_PERMISSION;
    }

    if (!DfsuAccessTokenHelper::CheckCallerPermission(PERM_DISTRIBUTED_DATASYNC)) {
        LOGE("[CloseP2PConnectionEx] DATASYNC permission denied");
        info = {"CloseP2PConnectionEx", ReportLevel::INNER, DfxBizStage::SOFTBUS_CLOSEP2P,
            DEFAULT_PKGNAME, networkId, E_PERMISSION, "permission denied"};
        RadarReportAdapter::GetInstance().ReportLinkConnectionAdapter(info);
        return E_PERMISSION;
    }

    if (networkId.length() < MIN_NETWORKID_LENGTH || networkId.length() >= DM_MAX_DEVICE_ID_LEN) {
        LOGE("Daemon::CloseP2PConnectionEx networkId length is invalid. len: %{public}zu", networkId.length());
        info = {"CloseP2PConnectionEx", ReportLevel::INNER, DfxBizStage::SOFTBUS_CLOSEP2P,
            DEFAULT_PKGNAME, networkId, E_INVAL_ARG_NAPI, "networkId invalid"};
        RadarReportAdapter::GetInstance().ReportLinkConnectionAdapter(info);
        return E_INVAL_ARG_NAPI;
    }
    std::lock_guard<std::mutex> lock(connectMutex_);
    if (DfsuAccessTokenHelper::CheckCallerPermission(FILE_ACCESS_MANAGER_PERMISSION)) {
        LOGI("[UMountDfsDocs] permission ok: FILE_ACCESS_MANAGER_PERMISSION");
        if (DeviceManagerAgent::GetInstance()->UMountDfsDocs(networkId, networkId.substr(0, VALID_MOUNT_NETWORKID_LEN),
                                                             false) != NO_ERROR) {
            LOGE("[UMountDfsDocs] failed");
            return E_UNMOUNT;
        } else {
            auto res = NotifyRemoteCancelNotification(networkId);
            LOGI("NotifyRemoteCancelNotification ret is %{public}d", res);
        }
    }
    ConnectCount::GetInstance().RemoveConnect(IPCSkeleton::GetCallingTokenID(), networkId);
    int32_t ret = CleanUp(networkId);
    if (ret != NO_ERROR) {
        LOGE("Daemon::CloseP2PConnectionEx disconnection failed");
        return E_CONNECTION_FAILED;
    }
    LOGI("Daemon::CloseP2PConnectionEx end");
    return 0;
}

bool Daemon::IsCallingDeviceTrusted()
{
    std::string callingNetworkId = IPCSkeleton::GetCallingDeviceID();
    LOGI("Daemon::IsCallingDeviceTrusted called, callingNetworkId=%{public}.5s", callingNetworkId.c_str());
    if (callingNetworkId.empty()) {
        LOGE("Get callingNetworkId failed");
        RadarParaInfo info = {"IsCallingDeviceTrusted", ReportLevel::INNER, DfxBizStage::SOFTBUS_COPY,
            "softbus", "", DEFAULT_ERR, "ipc call failed"};
        RadarReportAdapter::GetInstance().ReportFileAccessAdapter(info);
        return false;
    }
    if (!SoftBusPermissionCheck::IsSameAccount(callingNetworkId)) {
        LOGE("The calling device is not trusted");
        return false;
    }
    LOGI("Daemon::IsCallingDeviceTrusted end");
    return true;
}

int32_t Daemon::RequestSendFile(const std::string &srcUri,
                                const std::string &dstPath,
                                const std::string &dstDeviceId,
                                const std::string &sessionName)
{
    LOGI("RequestSendFile begin dstDeviceId: %{public}s", Utils::GetAnonyString(dstDeviceId).c_str());
#ifdef SUPPORT_SAME_ACCOUNT
    if (!IsCallingDeviceTrusted()) {
        LOGE("Check calling device permission failed");
        return FileManagement::E_PERMISSION_DENIED;
    }
#endif
    if (!FileSizeUtils::IsFilePathValid(FileSizeUtils::GetRealUri(srcUri)) ||
        !FileSizeUtils::IsFilePathValid(FileSizeUtils::GetRealUri(dstPath))) {
        LOGE("Path is forbidden");
        RadarParaInfo info = {"RequestSendFile", ReportLevel::INNER, DfxBizStage::SOFTBUS_COPY,
            DEFAULT_PKGNAME, "", E_ILLEGAL_URI, "path is forbidden"};
        RadarReportAdapter::GetInstance().ReportFileAccessAdapter(info);
        return OHOS::FileManagement::E_ILLEGAL_URI;
    }
    auto requestSendFileBlock = std::make_shared<BlockObject<int32_t>>(BLOCK_INTERVAL_SEND_FILE, ERR_BAD_VALUE);
    auto requestSendFileData = std::make_shared<RequestSendFileData>(
        srcUri, dstPath, dstDeviceId, sessionName, requestSendFileBlock);
    auto msgEvent = AppExecFwk::InnerEvent::Get(DEAMON_EXECUTE_REQUEST_SEND_FILE, requestSendFileData, 0);
    {
        std::lock_guard<std::mutex> lock(eventHandlerMutex_);
        if (eventHandler_ == nullptr) {
            LOGE("eventHandler has not find");
            return E_EVENT_HANDLER;
        }
        bool isSucc = eventHandler_->SendEvent(msgEvent, 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
        if (!isSucc) {
            LOGE("Daemon event handler post push asset event fail.");
            RadarParaInfo info = {"RequestSendFile", ReportLevel::INNER, DfxBizStage::SOFTBUS_COPY,
                "AppExecFwk", "", E_EVENT_HANDLER, "handler fail"};
            RadarReportAdapter::GetInstance().ReportFileAccessAdapter(info);
            return E_EVENT_HANDLER;
        }
    }

    auto ret = requestSendFileBlock->GetValue();
    LOGI("RequestSendFile end, ret is %{public}d", ret);
    return ret;
}

int32_t Daemon::RequestSendFileACL(const std::string &srcUri,
                                   const std::string &dstPath,
                                   const std::string &dstDeviceId,
                                   const std::string &sessionName,
                                   const AccountInfo &callerAccountInfo)
{
    LOGI("RequestSendFileACL begin dstDeviceId: %{public}s", Utils::GetAnonyString(dstDeviceId).c_str());
    if (!SoftBusPermissionCheck::CheckSinkPermission(callerAccountInfo)) {
        LOGE("CheckSinkPermission failed");
        return ERR_ACL_FAILED;
    }
    return RequestSendFile(srcUri, dstPath, dstDeviceId, sessionName);
}

int32_t Daemon::InnerCopy(const std::string &srcUri, const std::string &dstUri,
    const std::string &networkId, const sptr<IFileTransListener> &listener, HmdfsInfo &info)
{
    if (!FileSizeUtils::IsFilePathValid(FileSizeUtils::GetRealUri(srcUri)) ||
        !FileSizeUtils::IsFilePathValid(FileSizeUtils::GetRealUri(dstUri))) {
        LOGE("Path is forbidden");
        RadarParaInfo radarInfo = {"InnerCopy", ReportLevel::INNER, DfxBizStage::HMDFS_COPY,
            DEFAULT_PKGNAME, "", ERR_BAD_VALUE, "path is forbidden"};
        RadarReportAdapter::GetInstance().ReportFileAccessAdapter(radarInfo);
        return ERR_BAD_VALUE;
    }
    auto ret = Storage::DistributedFile::RemoteFileCopyManager::GetInstance().RemoteCopy(srcUri, dstUri,
        listener, QueryActiveUserId(), info.copyPath);
    LOGI("InnerCopy end, ret = %{public}d", ret);
    return ret;
}

int32_t Daemon::PrepareSession(const std::string &srcUri,
                               const std::string &dstUri,
                               const std::string &networkId,
                               const sptr<IRemoteObject> &listener,
                               HmdfsInfo &info)
{
    LOGI("PrepareSession networkId: %{public}.6s", networkId.c_str());
    auto listenerCallback = iface_cast<IFileTransListener>(listener);
    if (listenerCallback == nullptr) {
        LOGE("ListenerCallback is nullptr");
        RadarReportAdapter::GetInstance().SetUserStatistics(FILE_ACCESS_FAIL_CNT);
        RadarParaInfo radarInfo = {"PrepareSession", ReportLevel::INTERFACE, DfxBizStage::HMDFS_COPY,
            DEFAULT_PKGNAME, networkId, E_NULLPTR, "Get src path failed"};
        RadarReportAdapter::GetInstance().ReportFileAccessAdapter(radarInfo);
        return E_NULLPTR;
    }

    std::string srcPhysicalPath;
    if (SandboxHelper::GetPhysicalPath(srcUri, std::to_string(QueryActiveUserId()), srcPhysicalPath) != E_OK) {
        RadarReportAdapter::GetInstance().SetUserStatistics(FILE_ACCESS_FAIL_CNT);
        RadarParaInfo radarInfo = {"PrepareSession", ReportLevel::INTERFACE, DfxBizStage::HMDFS_COPY,
            DEFAULT_PKGNAME, networkId, EINVAL, "Get src path failed"};
        RadarReportAdapter::GetInstance().ReportFileAccessAdapter(radarInfo);
        LOGE("Get src path failed, invalid uri");
        return EINVAL;
    }

    uint64_t fileSize = 0;
    struct stat fileStat;
    if (stat(srcPhysicalPath.c_str(), &fileStat) == 0) {
        fileSize = fileStat.st_size;
    } else {
        LOGE("Stat srcPhysicalPath failed.");
    }

    DfsVersion remoteDfsVersion;
    auto ret = DeviceProfileAdapter::GetInstance().GetDfsVersionFromNetworkId(networkId, remoteDfsVersion);
    LOGI("GetRemoteVersion: ret:%{public}d, version:%{public}s", ret, remoteDfsVersion.dump().c_str());
    int32_t result = E_OK;
    if ((ret == FileManagement::ERR_OK) && (remoteDfsVersion.majorVersionNum != 0) && fileSize < INNER_COPY_LIMIT) {
        result = InnerCopy(srcUri, dstUri, networkId, listenerCallback, info);
    } else {
        result = CopyBaseOnRPC(srcUri, dstUri, networkId, listenerCallback, info);
    }
    RadarReportAdapter::GetInstance().SetUserStatistics(result == E_OK ? FILE_ACCESS_SUCC_CNT : FILE_ACCESS_FAIL_CNT);
    if (result != E_OK) {
        RadarParaInfo radarInfo = {"PrepareSession", ReportLevel::INTERFACE, DfxBizStage::HMDFS_COPY,
            DEFAULT_PKGNAME, networkId, result, "copy failed"};
        RadarReportAdapter::GetInstance().ReportFileAccessAdapter(radarInfo);
    }
    return result;
}

int32_t Daemon::CopyBaseOnRPC(const std::string &srcUri,
                              const std::string &dstUri,
                              const std::string &srcNetworkId,
                              const sptr<IFileTransListener> &listenerCallback,
                              HmdfsInfo &info)
{
    auto daemon = GetRemoteSA(srcNetworkId);
    if (daemon == nullptr) {
        LOGE("Daemon is nullptr");
        return E_SA_LOAD_FAILED;
    }

    std::string physicalPath;
    int32_t ret = GetRealPath(srcUri, dstUri, physicalPath, info, daemon, srcNetworkId);
    if (ret != E_OK) {
        LOGE("GetRealPath failed, ret = %{public}d", ret);
        return ret;
    }
    if (StoreSessionAndListener(physicalPath, info.sessionName, listenerCallback) != E_OK) {
        LOGE("SessionServer exceed max");
        RadarParaInfo radarInfo = {"CopyBaseOnRPC", ReportLevel::INNER, DfxBizStage::SOFTBUS_COPY,
            DEFAULT_PKGNAME, "", E_SOFTBUS_SESSION_FAILED, "SessionServer exce max"};
        RadarReportAdapter::GetInstance().ReportFileAccessAdapter(radarInfo);
        return E_SOFTBUS_SESSION_FAILED;
    }

    auto prepareSessionBlock = std::make_shared<BlockObject<int32_t>>(BLOCK_INTERVAL_SEND_FILE, ERR_BAD_VALUE);
    auto prepareSessionData = std::make_shared<PrepareSessionData>(srcUri, physicalPath, info.sessionName, daemon, info,
                                                                   prepareSessionBlock, srcNetworkId);
    auto msgEvent = AppExecFwk::InnerEvent::Get(DEAMON_EXECUTE_PREPARE_SESSION, prepareSessionData, 0);
    {
        std::lock_guard<std::mutex> lock(eventHandlerMutex_);
        if (eventHandler_ == nullptr) {
            LOGE("eventHandler has not find");
            return E_EVENT_HANDLER;
        }
        bool isSucc = eventHandler_->SendEvent(msgEvent, 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
        if (!isSucc) {
            LOGE("Daemon event handler post Prepare Session event fail.");
            return E_EVENT_HANDLER;
        }
    }

    ret = prepareSessionBlock->GetValue();
    if (ret != E_OK) {
        DeleteSessionAndListener(info.sessionName);
    }
    LOGI("PrepareSession end, ret is %{public}d", ret);
    return ret;
}

int32_t Daemon::StoreSessionAndListener(const std::string &physicalPath,
                                        std::string &sessionName,
                                        const sptr<IFileTransListener> &listener)
{
    SoftBusSessionPool::SessionInfo sessionInfo{.dstPath = physicalPath, .uid = IPCSkeleton::GetCallingUid()};
    sessionName = SoftBusSessionPool::GetInstance().GenerateSessionName(sessionInfo);
    if (sessionName.empty()) {
        LOGE("SessionServer exceed max");
        return E_SOFTBUS_SESSION_FAILED;
    }
    TransManager::GetInstance().AddTransTask(sessionName, listener);
    return E_OK;
}

int32_t Daemon::GetRealPath(const std::string &srcUri,
                            const std::string &dstUri,
                            std::string &physicalPath,
                            HmdfsInfo &info,
                            const sptr<IDaemon> &daemon,
                            const std::string &srcNetworkId)
{
    bool isSrcFile = false;
    bool isSrcDir = false;
    if (daemon == nullptr) {
        LOGE("Daemon::GetRealPath daemon is nullptr");
        return E_INVAL_ARG_NAPI;
    }
    if (!FileSizeUtils::IsFilePathValid(FileSizeUtils::GetRealUri(srcUri)) ||
        !FileSizeUtils::IsFilePathValid(FileSizeUtils::GetRealUri(dstUri))) {
        LOGE("Path is forbidden");
        RadarParaInfo radarInfo = {"GetRealPath", ReportLevel::INNER, DfxBizStage::SOFTBUS_COPY,
            DEFAULT_PKGNAME, "", E_ILLEGAL_URI, "path is forbidden"};
        RadarReportAdapter::GetInstance().ReportFileAccessAdapter(radarInfo);
        return OHOS::FileManagement::E_ILLEGAL_URI;
    }
    int32_t ret = E_OK;
    if (!DeviceProfileAdapter::GetInstance().IsRemoteDfsVersionLowerThanGiven(srcNetworkId, FILEMANAGER_VERSION)) {
        LOGI("Version >= 6.0.1, need ACL check");
        if (!SoftBusPermissionCheck::CheckSrcPermission(srcNetworkId)) {
            LOGE("CheckSrcPermission failed");
            return ERR_ACL_FAILED;
        }

        AccountInfo localAccountInfo;
        if (!SoftBusPermissionCheck::GetLocalAccountInfo(localAccountInfo)) {
            LOGE("GetLocalAccountInfo failed");
            return ERR_ACL_FAILED;
        }
        ret = daemon->GetRemoteCopyInfoACL(srcUri, isSrcFile, isSrcDir, localAccountInfo);
    } else {
        ret = daemon->GetRemoteCopyInfo(srcUri, isSrcFile, isSrcDir);
    }

    if (ret != E_OK) {
        LOGE("GetRemoteCopyInfo failed, ret = %{public}d", ret);
        RADAR_REPORT(RadarReporter::DFX_SET_DFS, RadarReporter::DFX_SET_BIZ_SCENE, RadarReporter::DFX_FAILED,
                     RadarReporter::BIZ_STATE, RadarReporter::DFX_END, RadarReporter::ERROR_CODE,
                     RadarReporter::GET_REMOTE_COPY_INFO_ERROR);
        return E_SOFTBUS_SESSION_FAILED;
    }
    return HandleDestinationPathAndPermissions(dstUri, isSrcFile, info, physicalPath);
}

int32_t Daemon::HandleDestinationPathAndPermissions(const std::string &dstUri,
                                                    bool isSrcFile,
                                                    HmdfsInfo &info,
                                                    std::string &physicalPath)
{
    HapTokenInfo hapTokenInfo;
    int result = AccessTokenKit::GetHapTokenInfo(IPCSkeleton::GetCallingTokenID(), hapTokenInfo);
    if (result != Security::AccessToken::AccessTokenKitRet::RET_SUCCESS) {
        LOGE("GetHapTokenInfo failed, errCode = %{public}d", result);
        RADAR_REPORT(RadarReporter::DFX_SET_DFS, RadarReporter::DFX_SET_BIZ_SCENE, RadarReporter::DFX_FAILED,
                     RadarReporter::BIZ_STATE, RadarReporter::DFX_END, RadarReporter::ERROR_CODE,
                     RadarReporter::GET_HAP_TOKEN_INFO_ERROR, RadarReporter::PACKAGE_NAME,
                     RadarReporter::accessTokenKit + to_string(result));
        RadarParaInfo radarInfo = {"HandleDestinationPathAndPermissions", ReportLevel::INNER, DfxBizStage::SOFTBUS_COPY,
            "access", "", result, "GetHapTokenInfo failed"};
        RadarReportAdapter::GetInstance().ReportFileAccessAdapter(radarInfo);
        return E_GET_USER_ID;
    }

    int32_t ret = SandboxHelper::GetPhysicalPath(dstUri, std::to_string(hapTokenInfo.userID), physicalPath);
    if (ret != E_OK) {
        LOGE("invalid uri, ret = %{public}d", ret);
        RadarParaInfo radarInfo = {"HandleDestinationPathAndPermissions", ReportLevel::INNER, DfxBizStage::SOFTBUS_COPY,
            "AFS", "", ret, "invalid uri failed"};
        RadarReportAdapter::GetInstance().ReportFileAccessAdapter(radarInfo);
        return E_GET_PHYSICAL_PATH_FAILED;
    }

    LOGI("GetRealPath userId %{public}s", std::to_string(hapTokenInfo.userID).c_str());
    info.dstPhysicalPath = physicalPath;
    ret = CheckCopyRule(physicalPath, dstUri, hapTokenInfo, isSrcFile, info);
    if (ret != E_OK) {
        LOGE("CheckCopyRule failed, ret = %{public}d", ret);
        RadarParaInfo radarInfo = {"HandleDestinationPathAndPermissions", ReportLevel::INNER, DfxBizStage::SOFTBUS_COPY,
            DEFAULT_PKGNAME, "", ret, "CheckCopyRule failed"};
        RadarReportAdapter::GetInstance().ReportFileAccessAdapter(radarInfo);
        return E_GET_PHYSICAL_PATH_FAILED;
    }
    return E_OK;
}

int32_t Daemon::CheckCopyRule(std::string &physicalPath,
                              const std::string &dstUri,
                              HapTokenInfo &hapTokenInfo,
                              const bool &isSrcFile,
                              HmdfsInfo &info)
{
    auto checkPath = physicalPath;
    if (isSrcFile && !Utils::IsFolder(physicalPath)) {
        auto pos = physicalPath.rfind('/');
        if (pos == std::string::npos) {
            LOGE("invalid file path");
            return E_GET_PHYSICAL_PATH_FAILED;
        }
        physicalPath = physicalPath.substr(0, pos);
    }

    std::error_code errCode;
    if (!std::filesystem::exists(physicalPath, errCode) && info.dirExistFlag) {
        LOGI("Not CheckValidPath");
    } else {
        auto pos = checkPath.rfind('/');
        if (pos == std::string::npos) {
            LOGE("invalid file path");
            return E_GET_PHYSICAL_PATH_FAILED;
        }
        checkPath = checkPath.substr(0, pos);
        if (!SandboxHelper::CheckValidPath(checkPath)) {
            LOGE("invalid path.");
            return E_GET_PHYSICAL_PATH_FAILED;
        }
    }

    Uri uri(dstUri);
    auto authority = uri.GetAuthority();
    info.authority = authority;
    if (authority != FILE_MANAGER_AUTHORITY && authority != MEDIA_AUTHORITY) {
        auto bundleName = SoftBusSessionListener::GetBundleName(dstUri);
        physicalPath = "/data/service/el2/" + to_string(hapTokenInfo.userID) + "/hmdfs/account/data/" + bundleName +
                       "/" + info.copyPath;
        if (!SandboxHelper::CheckValidPath(physicalPath)) {
            LOGE("invalid path.");
            return E_GET_PHYSICAL_PATH_FAILED;
        }
    } else {
        std::regex pathRegex("^[a-zA-Z0-9_\\-/\\\\\u4E00-\u9FA5]*$");
        if (!std::filesystem::exists(physicalPath, errCode) && std::regex_match(physicalPath.c_str(), pathRegex)) {
            std::filesystem::create_directory(physicalPath, errCode);
            if (errCode.value() != 0) {
                LOGE("Create directory failed, errCode %{public}d", errCode.value());
                return E_GET_PHYSICAL_PATH_FAILED;
            }
        }
    }
    return E_OK;
}

int32_t Daemon::GetRemoteCopyInfo(const std::string &srcUri, bool &isSrcFile, bool &srcIsDir)
{
    LOGI("GetRemoteCopyInfo begin.");
#ifdef SUPPORT_SAME_ACCOUNT
    if (!IsCallingDeviceTrusted()) {
        LOGE("Check calling device permission failed");
        return FileManagement::E_PERMISSION_DENIED;
    }
#endif
    auto physicalPath = SoftBusSessionListener::GetRealPath(srcUri);
    if (physicalPath.empty()) {
        LOGE("GetRemoteCopyInfo GetRealPath failed.");
        return E_SOFTBUS_SESSION_FAILED;
    }
    isSrcFile = Utils::IsFile(physicalPath);
    srcIsDir = Utils::IsFolder(physicalPath);
    return E_OK;
}

int32_t Daemon::GetRemoteCopyInfoACL(const std::string &srcUri,
                                     bool &isSrcFile,
                                     bool &srcIsDir,
                                     const AccountInfo &callerAccountInfo)
{
    LOGI("GetRemoteCopyInfoACL begin.");
    if (!SoftBusPermissionCheck::CheckSinkPermission(callerAccountInfo)) {
        LOGE("CheckSinkPermission failed");
        return ERR_ACL_FAILED;
    }
    return GetRemoteCopyInfo(srcUri, isSrcFile, srcIsDir);
}

sptr<IDaemon> Daemon::GetRemoteSA(const std::string &remoteDeviceId)
{
    LOGI("GetRemoteSA begin, DeviceId: %{public}s", Utils::GetAnonyString(remoteDeviceId).c_str());
    auto sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        LOGE("Sam is nullptr");
        RadarParaInfo info = {"GetRemoteSA", ReportLevel::INNER, DfxBizStage::SOFTBUS_COPY,
            "samanager", "", DEFAULT_ERR, "Sam is nullptr"};
        RadarReportAdapter::GetInstance().ReportFileAccessAdapter(info);
        return nullptr;
    }

    auto object = sam->GetSystemAbility(FILEMANAGEMENT_DISTRIBUTED_FILE_DAEMON_SA_ID, remoteDeviceId);
    if (object == nullptr) {
        LOGE("GetSystemAbility failed");
        RADAR_REPORT(RadarReporter::DFX_SET_DFS, RadarReporter::DFX_SET_BIZ_SCENE, RadarReporter::DFX_FAILED,
            RadarReporter::BIZ_STATE, RadarReporter::DFX_END, RadarReporter::ERROR_CODE,
            RadarReporter::GET_SYSTEM_ABILITY_ERROR, RadarReporter::PACKAGE_NAME, RadarReporter::saMgr);
        RadarParaInfo info = {"GetRemoteSA", ReportLevel::INNER, DfxBizStage::SOFTBUS_COPY,
            "samanager", "", DEFAULT_ERR, "GetSystemAbility failed"};
        RadarReportAdapter::GetInstance().ReportFileAccessAdapter(info);
        return nullptr;
    }
    auto daemon = iface_cast<IDaemon>(object);
    if (daemon == nullptr) {
        LOGE("Connect service nullptr");
        RadarParaInfo info = {"GetRemoteSA", ReportLevel::INNER, DfxBizStage::SOFTBUS_COPY,
            "IPC", "", DEFAULT_ERR, "Connect service nullptr"};
        RadarReportAdapter::GetInstance().ReportFileAccessAdapter(info);
        return nullptr;
    }
    LOGI("GetRemoteSA success, DeviceId: %{public}s", Utils::GetAnonyString(remoteDeviceId).c_str());
    return daemon;
}

int32_t Daemon::Copy(const std::string &srcUri,
                     const std::string &dstPath,
                     const sptr<IDaemon> &daemon,
                     const std::string &sessionName,
                     const std::string &srcNetworkId)
{
    if (daemon == nullptr) {
        LOGE("Daemon::Copy daemon is nullptr");
        return E_INVAL_ARG_NAPI;
    }
    if (!FileSizeUtils::IsFilePathValid(FileSizeUtils::GetRealUri(srcUri)) ||
        !FileSizeUtils::IsFilePathValid(FileSizeUtils::GetRealUri(dstPath))) {
        LOGE("Path is forbidden");
        RadarParaInfo info = {"Copy", ReportLevel::INNER, DfxBizStage::SOFTBUS_COPY,
            DEFAULT_PKGNAME, "", E_INVAL_ARG, "path is forbidden"};
        RadarReportAdapter::GetInstance().ReportFileAccessAdapter(info);
        return E_INVAL_ARG;
    }
    auto &deviceManager = DistributedHardware::DeviceManager::GetInstance();
    DistributedHardware::DmDeviceInfo localDeviceInfo{};
    int errCode = deviceManager.GetLocalDeviceInfo(IDaemon::SERVICE_NAME, localDeviceInfo);
    if (errCode != E_OK) {
        LOGE("GetLocalDeviceInfo failed, errCode = %{public}d", errCode);
        return E_GET_DEVICE_ID;
    }
    LOGI("Copy localDeviceInfo.networkId: %{public}s", Utils::GetAnonyString(localDeviceInfo.networkId).c_str());

    int32_t ret = E_OK;
    if (!DeviceProfileAdapter::GetInstance().IsRemoteDfsVersionLowerThanGiven(srcNetworkId, FILEMANAGER_VERSION)) {
        LOGI("Version >= 6.0.1, need ACL check");
        if (!SoftBusPermissionCheck::CheckSrcPermission(srcNetworkId)) {
            LOGE("CheckSrcPermission failed");
            return ERR_ACL_FAILED;
        }

        AccountInfo localAccountInfo;
        if (!SoftBusPermissionCheck::GetLocalAccountInfo(localAccountInfo)) {
            LOGE("GetLocalAccountInfo failed");
            return ERR_ACL_FAILED;
        }

        ret = daemon->RequestSendFileACL(srcUri, dstPath, localDeviceInfo.networkId, sessionName, localAccountInfo);
    } else {
        ret = daemon->RequestSendFile(srcUri, dstPath, localDeviceInfo.networkId, sessionName);
    }
    if (ret != E_OK) {
        LOGE("RequestSendFile failed, ret = %{public}d", ret);
        RADAR_REPORT(RadarReporter::DFX_SET_DFS, RadarReporter::DFX_SET_BIZ_SCENE, RadarReporter::DFX_FAILED,
            RadarReporter::BIZ_STATE, RadarReporter::DFX_END, RadarReporter::ERROR_CODE,
            RadarReporter::REQUEST_SEND_FILE_ERROR, RadarReporter::PACKAGE_NAME, ret);
        return E_SA_LOAD_FAILED;
    }
    return E_OK;
}

int32_t Daemon::CancelCopyTask(const std::string &sessionName)
{
    LOGI("Cancel copy task in. sessionName = %{public}s", sessionName.c_str());
    SoftBusSessionPool::SessionInfo sessionInfo{};
    bool isExist = SoftBusSessionPool::GetInstance().GetSessionInfo(sessionName, sessionInfo);
    if (!isExist) {
        LOGE("CancelCopyTask failed, cannot get session info for input session name=%{public}s.", sessionName.c_str());
        RadarParaInfo info = {"CancelCopyTask", ReportLevel::INNER, DfxBizStage::HMDFS_COPY,
            DEFAULT_PKGNAME, "", E_INVAL_ARG, "CancelCopyTask failed"};
        RadarReportAdapter::GetInstance().ReportFileAccessAdapter(info);
        return E_INVAL_ARG;
    }
    auto callingUid = IPCSkeleton::GetCallingUid();
    if (callingUid != sessionInfo.uid) {
        LOGE("CancelCopyTask failed, calling uid=%{public}d has no permission to cancel copy for uid=%{public}d.",
             callingUid, sessionInfo.uid);
        RadarParaInfo info = {"CancelCopyTask", ReportLevel::INNER, DfxBizStage::HMDFS_COPY,
            "softbus", "", E_PERMISSION_DENIED, "ipc call failed"};
        RadarReportAdapter::GetInstance().ReportFileAccessAdapter(info);
        return E_PERMISSION_DENIED;
    }
    SoftBusHandler::GetInstance().CloseSessionWithSessionName(sessionName);
    return E_OK;
}

int32_t Daemon::CancelCopyTask(const std::string &srcUri, const std::string &dstUri)
{
    Storage::DistributedFile::RemoteFileCopyManager::GetInstance().RemoteCancel(srcUri, dstUri);
    return E_OK;
}

void Daemon::DeleteSessionAndListener(const std::string &sessionName)
{
    SoftBusSessionPool::GetInstance().DeleteSessionInfo(sessionName);
    TransManager::GetInstance().DeleteTransTask(sessionName);
}

void Daemon::DfsListenerDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    sptr<IRemoteObject> diedRemote = remote.promote();
    if (diedRemote == nullptr) {
        LOGE("Daemon::DfsListenerDeathRecipient OnremoteDied received died notify nullptr");
        return;
    }

    uint32_t callingTokenId;
    auto ret = ConnectCount::GetInstance().FindCallingTokenIdForListerner(diedRemote, callingTokenId);
    if (ret != E_OK) {
        LOGE("fail to FindCallingTokenIdForListerner");
        return;
    }
    std::vector<std::string> networkIds = ConnectCount::GetInstance().RemoveConnect(callingTokenId);
    if (networkIds.empty()) {
        LOGE("fail to get networkIdSet");
        return;
    }

    for (auto it = networkIds.begin(); it != networkIds.end(); ++it) {
        if (it->empty()) {
            LOGE("[DfsListenerDeathRecipient] networkId is null");
            continue;
        }
        DistributedHardware::DmDeviceInfo deviceInfo{};
        auto res = strcpy_s(deviceInfo.networkId, DM_MAX_DEVICE_ID_LEN, it->c_str());
        if (res != 0) {
            LOGE("strcpy failed, res = %{public}d, errno = %{public}d", res, errno);
            return;
        }
        if (!ConnectCount::GetInstance().CheckCount(*it)) {
            DeviceManagerAgent::GetInstance()->OnDeviceP2POffline(deviceInfo);
        }
    }
    LOGI("Daemon::DfsListenerDeathRecipient OnremoteDied end");
    return;
}

int32_t Daemon::JudgeEmpty(const sptr<AssetObj> &assetObj, const sptr<IAssetSendCallback> &sendCallback)
{
    RadarParaInfo info = {"PushAsset", ReportLevel::DEFAULT, DfxBizStage::PUSH_ASSERT,
        DEFAULT_PKGNAME, "", E_OK, "PushAsset Begin"};
    RadarReportAdapter::GetInstance().ReportFileAccessAdapter(info);
    if (assetObj == nullptr || sendCallback == nullptr) {
        LOGE("param is nullptr.");
        info = {"JudgeEmpty", ReportLevel::INTERFACE, DfxBizStage::PUSH_ASSERT,
            DEFAULT_PKGNAME, "", E_NULLPTR, "param is nullptr"};
        RadarReportAdapter::GetInstance().ReportFileAccessAdapter(info);
        RadarReportAdapter::GetInstance().SetUserStatistics(FILE_ACCESS_FAIL_CNT);
        return E_NULLPTR;
    }
    return E_OK;
}

int32_t Daemon::PushAsset(int32_t userId,
                          const sptr<AssetObj> &assetObj,
                          const sptr<IAssetSendCallback> &sendCallback)
{
    LOGI("Daemon::PushAsset begin.");
    if (JudgeEmpty(assetObj, sendCallback) != E_OK) {
        return E_NULLPTR;
    }
    const auto &uriVec = assetObj->uris_;
    for (const auto &uri : uriVec) {
        if (!FileSizeUtils::IsFilePathValid(FileSizeUtils::GetRealUri(uri))) {
            LOGE("Path is forbidden");
            RadarParaInfo info = {"PushAsset", ReportLevel::INTERFACE, DfxBizStage::PUSH_ASSERT,
                DEFAULT_PKGNAME, assetObj->dstNetworkId_, E_ILLEGAL_URI, "path is forbidden"};
            RadarReportAdapter::GetInstance().ReportFileAccessAdapter(info);
            RadarReportAdapter::GetInstance().SetUserStatistics(FILE_ACCESS_FAIL_CNT);
            return OHOS::FileManagement::E_ILLEGAL_URI;
        }
    }
    auto taskId = assetObj->srcBundleName_ + assetObj->sessionId_;
    if (taskId.empty()) {
        LOGE("assetObj info is null.");
        RadarParaInfo info = {"PushAsset", ReportLevel::INTERFACE, DfxBizStage::PUSH_ASSERT,
            DEFAULT_PKGNAME, assetObj->dstNetworkId_, E_NULLPTR, "assetObj info is null."};
        RadarReportAdapter::GetInstance().ReportFileAccessAdapter(info);
        RadarReportAdapter::GetInstance().SetUserStatistics(FILE_ACCESS_FAIL_CNT);
        return E_NULLPTR;
    }
    AssetCallbackManager::GetInstance().AddSendCallback(taskId, sendCallback);
    auto pushData = std::make_shared<PushAssetData>(userId, assetObj);
    auto msgEvent = AppExecFwk::InnerEvent::Get(DEAMON_EXECUTE_PUSH_ASSET, pushData, 0);

    std::lock_guard<std::mutex> lock(eventHandlerMutex_);
    if (eventHandler_ == nullptr) {
        LOGE("eventHandler has not find");
        AssetCallbackManager::GetInstance().RemoveSendCallback(taskId);
        RadarParaInfo info = {"PushAsset", ReportLevel::INTERFACE, DfxBizStage::PUSH_ASSERT,
            DEFAULT_PKGNAME, assetObj->dstNetworkId_, E_EVENT_HANDLER, "eventHandler has not find"};
        RadarReportAdapter::GetInstance().ReportFileAccessAdapter(info);
        RadarReportAdapter::GetInstance().SetUserStatistics(FILE_ACCESS_FAIL_CNT);
        return E_EVENT_HANDLER;
    }
    bool isSucc = eventHandler_->SendEvent(msgEvent, 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
    if (!isSucc) {
        LOGE("Daemon event handler post push asset event fail.");
        AssetCallbackManager::GetInstance().RemoveSendCallback(taskId);
        RadarParaInfo info = {"PushAsset", ReportLevel::INTERFACE, DfxBizStage::PUSH_ASSERT,
            "AppExecFwk", assetObj->dstNetworkId_, E_EVENT_HANDLER, "event Handler fail"};
        RadarReportAdapter::GetInstance().ReportFileAccessAdapter(info);
        RadarReportAdapter::GetInstance().SetUserStatistics(FILE_ACCESS_FAIL_CNT);
        return E_EVENT_HANDLER;
    }
    return E_OK;
}

int32_t Daemon::RegisterAssetCallback(const sptr<IAssetRecvCallback> &recvCallback)
{
    LOGI("Daemon::RegisterAssetCallback begin.");
    if (recvCallback == nullptr) {
        LOGE("recvCallback is nullptr.");
        return E_NULLPTR;
    }
    AssetCallbackManager::GetInstance().AddRecvCallback(recvCallback);
    return E_OK;
}

int32_t Daemon::UnRegisterAssetCallback(const sptr<IAssetRecvCallback> &recvCallback)
{
    LOGI("Daemon::UnRegisterAssetCallback begin.");
    if (recvCallback == nullptr) {
        LOGE("recvCallback is nullptr.");
        return E_NULLPTR;
    }
    AssetCallbackManager::GetInstance().RemoveRecvCallback(recvCallback);
    return E_OK;
}

void Daemon::StartEventHandler()
{
    LOGI("StartEventHandler begin");
    if (daemonExecute_ == nullptr) {
        daemonExecute_ = std::make_shared<DaemonExecute>();
    }

    std::lock_guard<std::mutex> lock(eventHandlerMutex_);
    auto runer = AppExecFwk::EventRunner::Create(IDaemon::SERVICE_NAME.c_str());
    eventHandler_ = std::make_shared<DaemonEventHandler>(runer, daemonExecute_);
}

void Daemon::DisconnectDevice(const std::string networkId)
{
    DistributedHardware::DmDeviceInfo deviceInfo;
    auto ret = strcpy_s(deviceInfo.networkId, DM_MAX_DEVICE_ID_LEN, networkId.c_str());
    if (ret != 0) {
        LOGE("strcpy for network id failed, ret is %{public}d, errno = %{public}d", ret, errno);
        return;
    }
    ret = DeviceManagerAgent::GetInstance()->OnDeviceP2POffline(deviceInfo);
    LOGI("Daemon::DisconnectDevice result %{public}d", ret);
}

int32_t Daemon::GetDfsUrisDirFromLocal(const std::vector<std::string> &uriList,
                                       const int32_t userId,
                                       std::unordered_map<std::string,
                                       AppFileService::ModuleRemoteFileShare::HmdfsUriInfo> &uriToDfsUriMaps)
{
    LOGI("Daemon::GetDfsUrisDirFromLocal start");
    RadarParaInfo info = {"GetDfsUrisDirFromLocal", ReportLevel::DEFAULT, DfxBizStage::GENERATE_DIS_URI,
        DEFAULT_PKGNAME, "", E_OK, "GetDfsUrisDirFromLocal Begin"};
    RadarReportAdapter::GetInstance().ReportGenerateDisUriAdapter(info);
    auto callingUid = IPCSkeleton::GetCallingUid();
    if (callingUid != PASTEBOARDUSERID && callingUid != UDMFUSERID) {
        LOGE("Permission denied, caller is not pasterboard or udmf");
        info = {"GetDfsUrisDirFromLocal", ReportLevel::INTERFACE, DfxBizStage::GENERATE_DIS_URI,
            DEFAULT_PKGNAME, "", E_PERMISSION_DENIED, "Permission Denied"};
        RadarReportAdapter::GetInstance().ReportGenerateDisUriAdapter(info);
        RadarReportAdapter::GetInstance().SetUserStatistics(GENERATE_DIS_URI_FAIL_CNT);
        return E_PERMISSION_DENIED;
    }
    for (const auto &uri : uriList) {
        if (!FileSizeUtils::IsFilePathValid(FileSizeUtils::GetRealUri(uri))) {
            LOGE("path: %{public}s is forbidden", Utils::GetAnonyString(uri).c_str());
            info = {"GetDfsUrisDirFromLocal", ReportLevel::INTERFACE, DfxBizStage::GENERATE_DIS_URI,
                DEFAULT_PKGNAME, "", E_ILLEGAL_URI, "path is forbidde"};
            RadarReportAdapter::GetInstance().ReportGenerateDisUriAdapter(info);
            RadarReportAdapter::GetInstance().SetUserStatistics(GENERATE_DIS_URI_FAIL_CNT);
            return OHOS::FileManagement::E_ILLEGAL_URI;
        }
    }
    auto ret = AppFileService::ModuleRemoteFileShare::RemoteFileShare::GetDfsUrisDirFromLocal(uriList, userId,
                                                                                              uriToDfsUriMaps);
    RadarReportAdapter::GetInstance().SetUserStatistics(
        ret == E_OK ? GENERATE_DIS_URI_SUCC_CNT : GENERATE_DIS_URI_FAIL_CNT);
    if (ret != FileManagement::E_OK) {
        LOGE("GetDfsUrisDirFromLocal Failed, ret = %{public}d", ret);
        info = {"GetDfsUrisDirFromLocal", ReportLevel::INTERFACE, DfxBizStage::GENERATE_DIS_URI,
            "AFS", "", ret, "GetDfsUrisDirFromLocal Failed"};
        RadarReportAdapter::GetInstance().ReportGenerateDisUriAdapter(info);
        return ret;
    }
    return FileManagement::E_OK;
}

int32_t Daemon::GetDfsSwitchStatus(const std::string &networkId, int32_t &switchStatus)
{
    LOGI("GetDfsSwitchStatus enter.");
    if (networkId.empty() || networkId.length() >= DM_MAX_DEVICE_ID_LEN) {
        LOGE("GetDfsSwitchStatus networkId length is invalid.");
        return E_INVAL_ARG_NAPI;
    }
    bool tempStatus = false;
    int32_t ret = DeviceProfileAdapter::GetInstance().GetDeviceStatus(networkId, tempStatus);
    if (ret == E_OK) {
        switchStatus = tempStatus;
        return E_OK;
    }
    if (ret == ERR_DP_CAN_NOT_FIND) {
        LOGE("NetworkId: %{public}s not support switch", Utils::GetAnonyString(networkId).c_str());
        switchStatus = DEFAULT_DP_CANNOTFIND_VALUE;
        return ret;
    }
    LOGE("Get switch status failed, networkId: %{public}s", Utils::GetAnonyString(networkId).c_str());
    return ret;
}

int32_t Daemon::UpdateDfsSwitchStatus(int32_t switchStatus)
{
    LOGI("UpdateDfsSwitchStatus enter, switch status: %{public}d", switchStatus);
    int32_t ret = DeviceProfileAdapter::GetInstance().PutDeviceStatus(switchStatus);
    if (ret != E_OK) {
        LOGE("UpdateDfsSwitchStatus failed, err: %{public}d", ret);
        return ret;
    }
    if (switchStatus == 0) {
        std::unordered_map<std::string, MountCountInfo> allMountInfo =
            DeviceManagerAgent::GetInstance()->GetAllMountInfo();
        for (const auto &mountInfo : allMountInfo) {
            std::string networkId = mountInfo.second.networkId_;
            for (const auto &ele : mountInfo.second.callingCountMap_) {
                ConnectCount::GetInstance().RemoveConnect(ele.first, networkId);
            }
            ret = CleanUp(networkId);
            if (ret != NO_ERROR) {
                LOGE("Daemon::CloseP2PConnectionEx disconnection failed");
                return E_CONNECTION_FAILED;
            }
        }
    }
    return ret;
}

int32_t Daemon::GetConnectedDeviceList(std::vector<DfsDeviceInfo> &deviceList)
{
    LOGI("GetConnectedDeviceList enter.");
    DeviceManagerAgent::GetInstance()->GetConnectedDeviceList(deviceList);
    return E_OK;
}

int32_t Daemon::RegisterFileDfsListener(const std::string &instanceId, const sptr<IFileDfsListener> &listener)
{
    LOGI("RegisterFileDfsListener enter, instanceId: %{public}s", instanceId.c_str());
    if (instanceId.empty()) {
        LOGE("InstanceId length is invalid.");
        return E_INVAL_ARG_NAPI;
    }
    if (listener == nullptr) {
        LOGE("listener is nullptr");
        return E_INVAL_ARG_NAPI;
    }
    ConnectCount::GetInstance().AddFileConnect(instanceId, listener);
    return E_OK;
}

int32_t Daemon::UnregisterFileDfsListener(const std::string &instanceId)
{
    LOGI("UnregisterFileDfsListener enter, instanceId: %{public}s", instanceId.c_str());
    if (instanceId.empty()) {
        LOGE("InstanceId length is invalid.");
        return E_INVAL_ARG_NAPI;
    }
    if (!ConnectCount::GetInstance().RmFileConnect(instanceId)) {
        LOGE("RmFileConnect failed");
        return E_INVAL_ARG_NAPI;
    }
    return E_OK;
}

int32_t Daemon::IsSameAccountDevice(const std::string &networkId, bool &isSameAccount)
{
    LOGI("IsSameAccountDevice enter, instanceId: %{public}s", Utils::GetAnonyString(networkId).c_str());
#ifdef SUPPORT_SAME_ACCOUNT
    std::vector<DistributedHardware::DmDeviceInfo> deviceList;
    DistributedHardware::DeviceManager::GetInstance().GetTrustedDeviceList(IDaemon::SERVICE_NAME, "", deviceList);
    if (deviceList.empty()) {
        LOGE("trust device list size is invalid, size=%zu", deviceList.size());
        isSameAccount = false;
        return E_INVAL_ARG_NAPI;
    }
    for (const auto &deviceInfo : deviceList) {
        if (std::string(deviceInfo.networkId) == networkId) {
            isSameAccount = (deviceInfo.authForm == DistributedHardware::DmAuthForm::IDENTICAL_ACCOUNT);
            return E_OK;
        }
    }
    LOGE("The source and sink device is not same account, not support.");
    isSameAccount = false;
    return E_INVAL_ARG_NAPI;
#else
    isSameAccount = true;
    return E_OK;
#endif
}

void Daemon::DisconnectByRemote(const string &networkId)
{
    LOGI("start DisconnectByRemote");
    if (networkId.empty() || networkId.length() >= DM_MAX_DEVICE_ID_LEN) {
        LOGE("Daemon::DisconnectByRemote networkId length is invalid. len: %{public}zu", networkId.length());
        return;
    }
    string subnetworkId = networkId.substr(0, VALID_MOUNT_NETWORKID_LEN);
    int32_t res = DeviceManagerAgent::GetInstance()->UMountDfsDocs(networkId, subnetworkId, true);
    if (res != NO_ERROR) {
        LOGE("[UMountDfsDocs] failed");
        return;
    }
    ConnectCount::GetInstance().RemoveConnect(IPCSkeleton::GetCallingTokenID(), networkId);
    int32_t ret = CleanUp(networkId);
    if (ret != NO_ERROR) {
        LOGE("DisconnectByRemote disconnection failed.");
        return;
    }
}

int32_t Daemon::CreatControlLink(const std::string &networkId)
{
    LOGI("start CreatControlLink");
    if (ChannelManager::GetInstance().HasExistChannel(networkId)) {
        LOGI("exist channel, networkId: %{public}.6s", networkId.c_str());
        return ERR_OK;
    }

    if (ChannelManager::GetInstance().CreateClientChannel(networkId) != ERR_OK) {
        LOGE("create channel failed, networkId: %{public}.6s", networkId.c_str());
        return FileManagement::ERR_BAD_VALUE;
    }
    return ERR_OK;
}

int32_t Daemon::CancelControlLink(const std::string &networkId)
{
    if (!ChannelManager::GetInstance().HasExistChannel(networkId)) {
        LOGI("not exist channel, networkId: %{public}.6s", networkId.c_str());
        return ERR_OK;
    }
    if (ChannelManager::GetInstance().DestroyClientChannel(networkId) != ERR_OK) {
        LOGE("create channel failed, networkId: %{public}.6s", networkId.c_str());
        RadarParaInfo info = {"CancelControlLink", ReportLevel::INNER, DfxBizStage::SOFTBUS_CLOSEP2P,
            DEFAULT_PKGNAME, networkId, ERR_BAD_VALUE, "CancelControlLink failed"};
        RadarReportAdapter::GetInstance().ReportLinkConnectionAdapter(info);
        return FileManagement::ERR_BAD_VALUE;
    }
    return ERR_OK;
}

int32_t Daemon::CheckRemoteAllowConnect(const std::string &networkId)
{
    LOGI("start CheckRemoteAllowConnect");
    if (DeviceProfileAdapter::GetInstance().IsRemoteDfsVersionLowerThanGiven(networkId, FILEMANAGER_VERSION)) {
        LOGW("remote verison < 6.0.1, not support!");
        RadarParaInfo info = {"CheckRemoteAllowConnect", ReportLevel::INNER, DfxBizStage::SOFTBUS_OPENP2P,
            "DP", networkId, ERR_VERSION_NOT_SUPPORT, "verison not support"};
        RadarReportAdapter::GetInstance().ReportLinkConnectionAdapter(info);
        return FileManagement::ERR_VERSION_NOT_SUPPORT;
    }

    int32_t ret = CreatControlLink(networkId);
    if (ret != ERR_OK) {
        LOGE("CheckRemoteAllowConnect ret = %{public}d", ret);
        RadarParaInfo info = {"CheckRemoteAllowConnect", ReportLevel::INNER, DfxBizStage::SOFTBUS_OPENP2P,
            DEFAULT_PKGNAME, networkId, ret, "CreatControlLink fail"};
        RadarReportAdapter::GetInstance().ReportLinkConnectionAdapter(info);
        return ret;
    }
    ControlCmd request;
    ControlCmd response;
    request.msgType = ControlCmdType::CMD_CHECK_ALLOW_CONNECT;

    std::string srcNetId;
    ret = DistributedHardware::DeviceManager::GetInstance().GetLocalDeviceNetWorkId(IDaemon::SERVICE_NAME, srcNetId);
    if (ret != FileManagement::ERR_OK) {
        LOGE("DeviceManager GetLocalDeviceNetWorkId failed. ret is %{public}d", ret);
        RadarParaInfo info = {"CheckRemoteAllowConnect", ReportLevel::INNER, DfxBizStage::SOFTBUS_OPENP2P,
            "DM", networkId, ret, "GetLocalDeviceNetWorkId fail"};
        RadarReportAdapter::GetInstance().ReportLinkConnectionAdapter(info);
        return ret;
    }
    request.networkId = srcNetId;

    ret = ChannelManager::GetInstance().SendRequest(networkId, request, response, true);
    if (ret != ERR_OK) {
        LOGE("SendRequest ret = %{public}d", ret);
        RadarParaInfo info = {"CheckRemoteAllowConnect", ReportLevel::INNER, DfxBizStage::SOFTBUS_OPENP2P,
            DEFAULT_PKGNAME, networkId, ret, "SendRequest fail"};
        RadarReportAdapter::GetInstance().ReportLinkConnectionAdapter(info);
        return ret;
    }

    return response.msgBody == "true" ? FileManagement::ERR_BAD_VALUE : FileManagement::E_OK;
}

int32_t Daemon::NotifyRemotePublishNotification(const std::string &networkId)
{
    LOGI("start NotifyRemotePublishNotification");
    if (DeviceProfileAdapter::GetInstance().IsRemoteDfsVersionLowerThanGiven(networkId, FILEMANAGER_VERSION)) {
        LOGW("remote verison < 6.0.1, not support!");
        RadarParaInfo info = {"NotifyRemotePublishNotification", ReportLevel::INNER, DfxBizStage::SOFTBUS_OPENP2P,
            "DP", networkId, ERR_VERSION_NOT_SUPPORT, "verison not support"};
        RadarReportAdapter::GetInstance().ReportLinkConnectionAdapter(info);
        return FileManagement::ERR_VERSION_NOT_SUPPORT;
    }

    int32_t ret = CreatControlLink(networkId);
    if (ret != ERR_OK) {
        LOGE("NotifyRemotePublishNotification ret = %{public}d", ret);
        return ret;
    }
    ControlCmd request;
    request.msgType = ControlCmdType::CMD_PUBLISH_NOTIFICATION;

    std::string srcNetId;
    ret = DistributedHardware::DeviceManager::GetInstance().GetLocalDeviceNetWorkId(IDaemon::SERVICE_NAME, srcNetId);
    if (ret != ERR_OK) {
        LOGE("DeviceManager GetLocalDeviceNetWorkId failed. ret is %{public}d", ret);
        return ret;
    }
    request.networkId = srcNetId;

    ControlCmd response;
    ret = ChannelManager::GetInstance().SendRequest(networkId, request, response);
    if (ret != ERR_OK) {
        LOGE("SendRequest ret = %{public}d", ret);
        RadarParaInfo info = {"NotifyRemotePublishNotification", ReportLevel::INNER, DfxBizStage::SOFTBUS_OPENP2P,
            DEFAULT_PKGNAME, networkId, ret, "SendRequest fail"};
        RadarReportAdapter::GetInstance().ReportLinkConnectionAdapter(info);
        return ret;
    }
    return ret;
}

int32_t Daemon::NotifyRemoteCancelNotification(const std::string &networkId)
{
    LOGI("start NotifyRemoteCancelNotification");
    if (DeviceProfileAdapter::GetInstance().IsRemoteDfsVersionLowerThanGiven(networkId, FILEMANAGER_VERSION)) {
        LOGW("remote verison < 6.0.1, not support!");
        return FileManagement::ERR_VERSION_NOT_SUPPORT;
    }

    int32_t ret = CreatControlLink(networkId);
    if (ret != ERR_OK) {
        LOGE("NotifyRemoteCancelNotification ret = %{public}d", ret);
        return ret;
    }
    ControlCmd request;
    request.msgType = ControlCmdType::CMD_CANCEL_NOTIFICATION;

    std::string srcNetId;
    ret = DistributedHardware::DeviceManager::GetInstance().GetLocalDeviceNetWorkId(IDaemon::SERVICE_NAME, srcNetId);
    if (ret != ERR_OK) {
        LOGE("DeviceManager GetLocalDeviceNetWorkId failed. ret is %{public}d", ret);
        RadarParaInfo info = {"NotifyRemoteCancelNotification", ReportLevel::INNER, DfxBizStage::SOFTBUS_OPENP2P,
            "DM", networkId, ret, "GetLocalDeviceNetWorkId failed"};
        RadarReportAdapter::GetInstance().ReportLinkConnectionAdapter(info);
        return ret;
    }
    request.networkId = srcNetId;

    ControlCmd response;
    ret = ChannelManager::GetInstance().SendRequest(networkId, request, response);
    if (ret != ERR_OK) {
        LOGE("SendRequest ret = %{public}d", ret);
        RadarParaInfo info = {"NotifyRemoteCancelNotification", ReportLevel::INNER, DfxBizStage::SOFTBUS_OPENP2P,
            DEFAULT_PKGNAME, networkId, ret, "SendRequest fail"};
        RadarReportAdapter::GetInstance().ReportLinkConnectionAdapter(info);
        return ret;
    }
    return ret;
}

} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
