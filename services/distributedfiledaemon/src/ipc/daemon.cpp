/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#include "common_event_manager.h"
#include "common_event_support.h"
#include "connection_detector.h"
#include "device/device_manager_agent.h"
#include "dfs_daemon_event_dfx.h"
#include "dfs_error.h"
#include "dfsu_access_token_helper.h"
#include "i_file_dfs_listener.h"
#include "ipc_skeleton.h"
#include "iremote_object.h"
#include "iservice_registry.h"
#include "mountpoint/mount_manager.h"
#include "network/softbus/softbus_handler_asset.h"
#include "network/softbus/softbus_handler.h"
#include "network/softbus/softbus_session_dispatcher.h"
#include "network/softbus/softbus_session_listener.h"
#include "network/softbus/softbus_session_pool.h"
#include "sandbox_helper.h"
#include "system_ability_definition.h"
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
using HapTokenInfo = OHOS::Security::AccessToken::HapTokenInfo;
using AccessTokenKit = OHOS::Security::AccessToken::AccessTokenKit;

namespace {
const string FILE_MANAGER_AUTHORITY = "docs";
const string MEDIA_AUTHORITY = "media";
const int32_t E_PERMISSION_DENIED_NAPI = 201;
const int32_t E_INVAL_ARG_NAPI = 401;
const int32_t E_CONNECTION_FAILED = 13900045;
const int32_t E_UNMOUNT = 13600004;
constexpr int32_t CHECK_SESSION_DELAY_TIME_TWICE = 5000000;
constexpr mode_t DEFAULT_UMASK = 0002;
constexpr int32_t BLOCK_INTERVAL_SEND_FILE = 8 * 1000;
}

REGISTER_SYSTEM_ABILITY_BY_ID(Daemon, FILEMANAGEMENT_DISTRIBUTED_FILE_DAEMON_SA_ID, true);

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
    } catch (const exception &e) {
        LOGE("%{public}s", e.what());
    }

    state_ = ServiceRunningState::STATE_RUNNING;
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
    SoftBusHandlerAsset::GetInstance().DeleteAssetLocalSessionServer();
    AllConnectManager::GetInstance().UnInitAllConnectManager();
    LOGI("Stop finished successfully");
}

void Daemon::OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    if (systemAbilityId == COMMON_EVENT_SERVICE_ID) {
        (void)systemAbilityId;
        (void)deviceId;
        RegisterOsAccount();
    } else if (systemAbilityId == SOFTBUS_SERVER_SA_ID) {
        SoftBusHandlerAsset::GetInstance().CreateAssetLocalSessionServer();
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

        bool subscribeResult = EventFwk::CommonEventManager::UnSubscribeCommonEvent(subScriber_);
        LOGI("Daemon::OnRemoveSystemAbility subscribeResult = %{public}d", subscribeResult);
        subScriber_ = nullptr;
    } else if (systemAbilityId == SOFTBUS_SERVER_SA_ID) {
        SoftBusHandlerAsset::GetInstance().DeleteAssetLocalSessionServer();
    }
}

int32_t Daemon::OpenP2PConnection(const DistributedHardware::DmDeviceInfo &deviceInfo)
{
    LOGI("OpenP2PConnection networkId %{public}s", Utils::GetAnonyString(deviceInfo.networkId).c_str());
    RADAR_REPORT(RadarReporter::DFX_SET_DFS, RadarReporter::DFX_BUILD__LINK, RadarReporter::DFX_SUCCESS,
        RadarReporter::BIZ_STATE, RadarReporter::DFX_BEGIN);
    auto path = ConnectionDetector::ParseHmdfsPath();
    stringstream ss;
    ss << ConnectionDetector::MocklispHash(path);
    auto targetDir = ss.str();
    auto networkId = std::string(deviceInfo.networkId);
    int32_t ret = 0;
    if (!ConnectionDetector::GetConnectionStatus(targetDir, networkId)) {
        DeviceManagerAgent::GetInstance()->ClearCount(deviceInfo);
        LOGI("Get connection status not ok, try again.");
        ret = DeviceManagerAgent::GetInstance()->OnDeviceP2POnline(deviceInfo);
        if (ret != NO_ERROR) {
            LOGE("OpenP2PConnection failed, ret = %{public}d", ret);
        } else {
            ret = ConnectionDetector::RepeatGetConnectionStatus(targetDir, networkId);
            LOGI("RepeatGetConnectionStatus end, ret = %{public}d", ret);
        }
    }
    if (ret == FileManagement::ERR_BAD_VALUE) {
        LOGI("OpenP2PConnection check connection status failed, start to clean up");
        CloseP2PConnection(deviceInfo);
    }
    RADAR_REPORT(RadarReporter::DFX_SET_DFS, RadarReporter::DFX_BUILD__LINK, RadarReporter::DFX_SUCCESS,
        RadarReporter::BIZ_STATE, RadarReporter::DFX_END);
    return ret;
}

int32_t Daemon::CloseP2PConnection(const DistributedHardware::DmDeviceInfo &deviceInfo)
{
    LOGI("Close P2P Connection networkId %{public}s", Utils::GetAnonyString(deviceInfo.networkId).c_str());
    std::thread([=]() {
        int32_t ret = DeviceManagerAgent::GetInstance()->OnDeviceP2POffline(deviceInfo);
        LOGI("Close P2P Connection result %d", ret);
        return ret;
        }).detach();
    return 0;
}

int32_t Daemon::ConnectionCount(const DistributedHardware::DmDeviceInfo &deviceInfo)
{
    auto path = ConnectionDetector::ParseHmdfsPath();
    stringstream ss;
    ss << ConnectionDetector::MocklispHash(path);
    auto targetDir = ss.str();
    auto networkId = std::string(deviceInfo.networkId);
    int32_t ret = 0;
    ret = DeviceManagerAgent::GetInstance()->OnDeviceP2POnline(deviceInfo);
    if (ret != NO_ERROR) {
        LOGE("OpenP2PConnection failed, ret = %{public}d", ret);
    } else {
        ret = ConnectionDetector::RepeatGetConnectionStatus(targetDir, networkId);
        LOGI("RepeatGetConnectionStatus first time, ret = %{public}d", ret);
    }
    ret = ConnectionDetector::RepeatGetConnectionStatus(targetDir, networkId);
    if (ret != NO_ERROR) {
        LOGI("RepeatGetConnectionStatus third times, ret = %{public}d", ret);
        usleep(CHECK_SESSION_DELAY_TIME_TWICE);
        ret = ConnectionDetector::RepeatGetConnectionStatus(targetDir, networkId);
    }
    return ret;
}

int32_t Daemon::CleanUp(const DistributedHardware::DmDeviceInfo &deviceInfo,
                        const std::string &networkId, uint32_t callingTokenId)
{
    LOGI("CleanUp start");
    auto deviceManager = DeviceManagerAgent::GetInstance();
    if (deviceManager->RemoveRemoteReverseObj(false, callingTokenId) != E_OK) {
        LOGE("fail to RemoveRemoteReverseObj");
    }
    deviceManager->RemoveNetworkIdByOne(callingTokenId, networkId);
    int32_t ret = CloseP2PConnection(deviceInfo);
    if (ret != NO_ERROR) {
        LOGE("Daemon::CleanUp CloseP2PConnection failed");
        return E_CONNECTION_FAILED;
    }
    return 0;
}

int32_t Daemon::ConnectionAndMount(const DistributedHardware::DmDeviceInfo &deviceInfo,
                                   const std::string &networkId, uint32_t callingTokenId)
{
    LOGI("ConnectionAndMount start");
    int32_t ret = NO_ERROR;
    ret = ConnectionCount(deviceInfo);
    if (ret != NO_ERROR) {
        LOGE("connection failed");
        return ret;
    }
    auto deviceManager = DeviceManagerAgent::GetInstance();
    deviceManager->AddNetworkId(callingTokenId, networkId);
    if (!DfsuAccessTokenHelper::CheckCallerPermission(FILE_ACCESS_MANAGER_PERMISSION)) {
        LOGW("permission denied: FILE_ACCESS_MANAGER_PERMISSION");
        return ret;
    }
    std::string deviceId = deviceManager->GetDeviceIdByNetworkId(networkId);
    ret = deviceManager->MountDfsDocs(networkId, deviceId);
    if (ret != NO_ERROR) {
        LOGE("[MountDfsDocs] failed");
    }
    return ret;
}

int32_t Daemon::OpenP2PConnectionEx(const std::string &networkId, sptr<IFileDfsListener> remoteReverseObj)
{
    LOGI("Daemon::OpenP2PConnectionEx start, networkId %{public}s", Utils::GetAnonyString(networkId).c_str());
    if (!DfsuAccessTokenHelper::CheckCallerPermission(PERM_DISTRIBUTED_DATASYNC)) {
        LOGE("[OpenP2PConnectionEx] DATASYNC permission denied");
        return E_PERMISSION_DENIED_NAPI;
    }
    if (dfsListenerDeathRecipient_ == nullptr) {
        LOGE("Daemon::OpenP2PConnectionEx, new death recipient");
        dfsListenerDeathRecipient_ = new (std::nothrow) DfsListenerDeathRecipient();
    }
    if (remoteReverseObj == nullptr) {
        LOGE("Daemon::OpenP2PConnectionEx remoteReverseObj is nullptr");
        return E_INVAL_ARG_NAPI;
    }
    remoteReverseObj->AsObject()->AddDeathRecipient(dfsListenerDeathRecipient_);
    auto deviceManager = DeviceManagerAgent::GetInstance();
    if (networkId.empty()) {
        LOGE("Daemon::OpenP2PConnectionEx networkId is null");
        return E_INVAL_ARG_NAPI;
    }
    DistributedHardware::DmDeviceInfo deviceInfo{};
    auto res = strcpy_s(deviceInfo.networkId, networkId.size() + 1, networkId.c_str());
    if (res != NO_ERROR) {
        LOGE("OpenP2PConnectionEx strcpy failed, res = %{public}d", res);
        return E_INVAL_ARG_NAPI;
    }
    auto callingTokenId = IPCSkeleton::GetCallingTokenID();
    if (deviceManager->AddRemoteReverseObj(callingTokenId, remoteReverseObj) != E_OK) {
        LOGE("Daemon::OpenP2PConnectionEx::fail to AddRemoteReverseObj");
    }
    int32_t ret = ConnectionAndMount(deviceInfo, networkId, callingTokenId);
    if (ret != NO_ERROR) {
        CleanUp(deviceInfo, networkId, callingTokenId);
        return E_CONNECTION_FAILED;
    }
    LOGI("Daemon::OpenP2PConnectionEx end");
    return ret;
}

int32_t Daemon::CloseP2PConnectionEx(const std::string &networkId)
{
    LOGI("Daemon::CloseP2PConnectionEx start, networkId: %{public}s", Utils::GetAnonyString(networkId).c_str());
    if (!DfsuAccessTokenHelper::CheckCallerPermission(PERM_DISTRIBUTED_DATASYNC)) {
        LOGE("[CloseP2PConnectionEx] DATASYNC permission denied");
        return E_PERMISSION_DENIED_NAPI;
    }
    auto deviceManager = DeviceManagerAgent::GetInstance();
    auto callingTokenId = IPCSkeleton::GetCallingTokenID();
    if (networkId.empty()) {
        LOGE("[OpenP2PConnectionEx] networkId is null");
        return E_INVAL_ARG_NAPI;
    }
    std::string deviceId = deviceManager->GetDeviceIdByNetworkId(networkId);
    if (deviceId.empty()) {
        LOGE("fail to get deviceId");
        return E_CONNECTION_FAILED;
    }
    if (DfsuAccessTokenHelper::CheckCallerPermission(FILE_ACCESS_MANAGER_PERMISSION)) {
        LOGE("[UMountDfsDocs] permission ok: FILE_ACCESS_MANAGER_PERMISSION");
        int32_t ret_umount = deviceManager->UMountDfsDocs(networkId, deviceId, false);
        if (ret_umount != E_OK) {
            LOGE("[UMountDfsDocs] failed");
            return E_UNMOUNT;
        }
    }
    DistributedHardware::DmDeviceInfo deviceInfo{};
    auto res = strcpy_s(deviceInfo.networkId, networkId.size() + 1, networkId.c_str());
    if (res != NO_ERROR) {
        LOGE("strcpy failed, res = %{public}d", res);
        return E_INVAL_ARG_NAPI;
    }
    int32_t ret = CleanUp(deviceInfo, networkId, callingTokenId);
    if (ret != NO_ERROR) {
        LOGE("Daemon::CloseP2PConnectionEx disconnection failed");
        return E_CONNECTION_FAILED;
    }
    LOGI("Daemon::CloseP2PConnectionEx end");
    return 0;
}

int32_t Daemon::RequestSendFile(const std::string &srcUri,
                                const std::string &dstPath,
                                const std::string &dstDeviceId,
                                const std::string &sessionName)
{
    LOGI("RequestSendFile begin dstDeviceId: %{public}s", Utils::GetAnonyString(dstDeviceId).c_str());
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
            return E_EVENT_HANDLER;
        }
    }

    auto ret = requestSendFileBlock->GetValue();
    LOGI("RequestSendFile end, ret is %{public}d", ret);
    return ret;
}

int32_t Daemon::PrepareSession(const std::string &srcUri,
                               const std::string &dstUri,
                               const std::string &srcDeviceId,
                               const sptr<IRemoteObject> &listener,
                               HmdfsInfo &info)
{
    LOGI("PrepareSession begin srcDeviceId: %{public}s", Utils::GetAnonyString(srcDeviceId).c_str());
    auto listenerCallback = iface_cast<IFileTransListener>(listener);
    if (listenerCallback == nullptr) {
        LOGE("ListenerCallback is nullptr");
        return E_NULLPTR;
    }

    auto daemon = GetRemoteSA(srcDeviceId);
    if (daemon == nullptr) {
        LOGE("Daemon is nullptr");
        return E_SA_LOAD_FAILED;
    }

    std::string physicalPath;
    auto ret = GetRealPath(srcUri, dstUri, physicalPath, info, daemon);
    if (ret != E_OK) {
        LOGE("GetRealPath failed, ret = %{public}d", ret);
        return ret;
    }

    SoftBusSessionPool::SessionInfo sessionInfo{.dstPath = physicalPath, .uid = IPCSkeleton::GetCallingUid()};
    auto sessionName = SoftBusSessionPool::GetInstance().GenerateSessionName(sessionInfo);
    if (sessionName.empty()) {
        LOGE("SessionServer exceed max");
        return E_SOFTBUS_SESSION_FAILED;
    }
    info.sessionName = sessionName;
    StoreSessionAndListener(physicalPath, sessionName, listenerCallback);

    auto prepareSessionBlock = std::make_shared<BlockObject<int32_t>>(BLOCK_INTERVAL_SEND_FILE, ERR_BAD_VALUE);
    auto prepareSessionData = std::make_shared<PrepareSessionData>(
        srcUri, physicalPath, sessionName, daemon, info, prepareSessionBlock);
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
    LOGI("PrepareSession end, ret is %{public}d", ret);
    return ret;
}

void Daemon::StoreSessionAndListener(const std::string &physicalPath,
                                     const std::string &sessionName,
                                     const sptr<IFileTransListener> &listener)
{
    TransManager::GetInstance().AddTransTask(sessionName, listener);
}

int32_t Daemon::GetRealPath(const std::string &srcUri,
                            const std::string &dstUri,
                            std::string &physicalPath,
                            HmdfsInfo &info,
                            const sptr<IDaemon> &daemon)
{
    auto start = std::chrono::high_resolution_clock::now();
    bool isSrcFile = false;
    bool isSrcDir = false;
    if (daemon == nullptr) {
        LOGE("Daemon::GetRealPath daemon is nullptr");
        return E_INVAL_ARG_NAPI;
    }
    auto ret = daemon->GetRemoteCopyInfo(srcUri, isSrcFile, isSrcDir);
    if (ret != E_OK) {
        LOGE("GetRemoteCopyInfo failed, ret = %{public}d", ret);
        RADAR_REPORT(RadarReporter::DFX_SET_DFS, RadarReporter::DFX_SET_BIZ_SCENE, RadarReporter::DFX_FAILED,
            RadarReporter::BIZ_STATE, RadarReporter::DFX_END, RadarReporter::ERROR_CODE,
            RadarReporter::GET_REMOTE_COPY_INFO_ERROR);
        return E_SOFTBUS_SESSION_FAILED;
    }

    HapTokenInfo hapTokenInfo;
    int result = AccessTokenKit::GetHapTokenInfo(IPCSkeleton::GetCallingTokenID(), hapTokenInfo);
    if (result != Security::AccessToken::AccessTokenKitRet::RET_SUCCESS) {
        LOGE("GetHapTokenInfo failed, errCode = %{public}d", result);
        RADAR_REPORT(RadarReporter::DFX_SET_DFS, RadarReporter::DFX_SET_BIZ_SCENE, RadarReporter::DFX_FAILED,
            RadarReporter::BIZ_STATE, RadarReporter::DFX_END, RadarReporter::ERROR_CODE,
            RadarReporter::GET_HAP_TOKEN_INFO_ERROR, RadarReporter::PACKAGE_NAME,
            RadarReporter::accessTokenKit + to_string(result));
        return E_GET_USER_ID;
    }
    ret = SandboxHelper::GetPhysicalPath(dstUri, std::to_string(hapTokenInfo.userID), physicalPath);
    if (ret != E_OK) {
        LOGE("invalid uri, ret = %{public}d", ret);
        return E_GET_PHYSICAL_PATH_FAILED;
    }

    LOGI("physicalPath %{public}s, userId %{public}s", GetAnonyString(physicalPath).c_str(),
         std::to_string(hapTokenInfo.userID).c_str());
    info.dstPhysicalPath = physicalPath;
    ret = CheckCopyRule(physicalPath, dstUri, hapTokenInfo, isSrcFile, info);
    if (ret != E_OK) {
        LOGE("CheckCopyRule failed, ret = %{public}d", ret);
        return E_GET_PHYSICAL_PATH_FAILED;
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    Utils::SysEventFileParse(duration.count());
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
        LOGI("Not CheckValidPath, physicalPath %{public}s", GetAnonyString(physicalPath).c_str());
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
        std::regex pathRegex("^[a-zA-Z0-9_\\-/\\\\]*$");
        if (!std::filesystem::exists(physicalPath, errCode) && std::regex_match(physicalPath.c_str(), pathRegex)) {
            std::filesystem::create_directory(physicalPath, errCode);
            if (errCode.value() != 0) {
                LOGE("Create directory failed, physicalPath %{public}s", GetAnonyString(physicalPath).c_str());
                return E_GET_PHYSICAL_PATH_FAILED;
            }
        }
    }
    return E_OK;
}

int32_t Daemon::GetRemoteCopyInfo(const std::string &srcUri, bool &isSrcFile, bool &srcIsDir)
{
    LOGI("GetRemoteCopyInfo begin.");
    auto physicalPath = SoftBusSessionListener::GetRealPath(srcUri);
    if (physicalPath.empty()) {
        LOGE("GetRemoteCopyInfo GetRealPath failed.");
        return E_SOFTBUS_SESSION_FAILED;
    }
    isSrcFile = Utils::IsFile(physicalPath);
    srcIsDir = Utils::IsFolder(physicalPath);
    return E_OK;
}

sptr<IDaemon> Daemon::GetRemoteSA(const std::string &remoteDeviceId)
{
    LOGI("GetRemoteSA begin, DeviceId: %{public}s", Utils::GetAnonyString(remoteDeviceId).c_str());
    auto sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        LOGE("Sam is nullptr");
        return nullptr;
    }

    auto object = sam->GetSystemAbility(FILEMANAGEMENT_DISTRIBUTED_FILE_DAEMON_SA_ID, remoteDeviceId);
    if (object == nullptr) {
        LOGE("GetSystemAbility failed");
        RADAR_REPORT(RadarReporter::DFX_SET_DFS, RadarReporter::DFX_SET_BIZ_SCENE, RadarReporter::DFX_FAILED,
            RadarReporter::BIZ_STATE, RadarReporter::DFX_END, RadarReporter::ERROR_CODE,
            RadarReporter::GET_SYSTEM_ABILITY_ERROR, RadarReporter::PACKAGE_NAME, RadarReporter::saMgr);
        return nullptr;
    }
    auto daemon = iface_cast<IDaemon>(object);
    if (daemon == nullptr) {
        LOGE("Connect service nullptr");
        return nullptr;
    }
    LOGI("GetRemoteSA success, DeviceId: %{public}s", Utils::GetAnonyString(remoteDeviceId).c_str());
    return daemon;
}

int32_t Daemon::Copy(const std::string &srcUri,
                     const std::string &dstPath,
                     const sptr<IDaemon> &daemon,
                     const std::string &sessionName)
{
    auto &deviceManager = DistributedHardware::DeviceManager::GetInstance();
    DistributedHardware::DmDeviceInfo localDeviceInfo{};
    int errCode = deviceManager.GetLocalDeviceInfo(IDaemon::SERVICE_NAME, localDeviceInfo);
    if (errCode != E_OK) {
        LOGE("GetLocalDeviceInfo failed, errCode = %{public}d", errCode);
        return E_GET_DEVICE_ID;
    }
    if (daemon == nullptr) {
        LOGE("Daemon::Copy daemon is nullptr");
        return E_INVAL_ARG_NAPI;
    }
    LOGI("Copy localDeviceInfo.networkId: %{public}s", Utils::GetAnonyString(localDeviceInfo.networkId).c_str());
    auto ret = daemon->RequestSendFile(srcUri, dstPath, localDeviceInfo.networkId, sessionName);
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
        return E_INVAL_ARG;
    }
    auto callingUid = IPCSkeleton::GetCallingUid();
    if (callingUid != sessionInfo.uid) {
        LOGE("CancelCopyTask failed, calling uid=%{public}d has no permission to cancel copy for uid=%{public}d.",
             callingUid, sessionInfo.uid);
        return E_PERMISSION_DENIED;
    }
    SoftBusHandler::GetInstance().CloseSessionWithSessionName(sessionName);
    return E_OK;
}

void Daemon::DeleteSessionAndListener(const std::string &sessionName, const int32_t socketId)
{
    SoftBusSessionPool::GetInstance().DeleteSessionInfo(sessionName);
    TransManager::GetInstance().DeleteTransTask(sessionName);
    SoftBusHandler::GetInstance().CloseSession(socketId, sessionName);
}

void Daemon::DfsListenerDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    sptr<IRemoteObject> diedRemote = remote.promote();
    if (diedRemote == nullptr) {
        LOGE("Daemon::DfsListenerDeathRecipient OnremoteDied received died notify nullptr");
        return;
    }
    auto deviceManager = DeviceManagerAgent::GetInstance();
    uint32_t callingTokenId = 0;
    sptr<IFileDfsListener> listener = nullptr;
    deviceManager->FindListenerByObject(remote, callingTokenId, listener);
    if (callingTokenId==0 || listener == nullptr) {
        LOGE("fail to FindListenerByObject");
        return;
    }
    if (deviceManager->RemoveRemoteReverseObj(false, callingTokenId) != E_OK) {
        LOGE("fail to RemoveRemoteReverseObj");
    }
    auto networkIdSet = deviceManager->GetNetworkIds(callingTokenId);
    if (networkIdSet.empty()) {
        LOGE("fail to get networkIdSet");
        return;
    }
    for (auto it = networkIdSet.begin(); it != networkIdSet.end(); ++it) {
        if (it->empty()) {
            LOGE("[DfsListenerDeathRecipient] networkId is null");
            continue;
        }
        std::string deviceId = deviceManager->GetDeviceIdByNetworkId(*it);
        if (deviceId.empty()) {
            LOGE("fail to get deviceId, networkId: %{public}s", Utils::GetAnonyString(*it).c_str());
            continue;
        }
        deviceManager->UMountDfsDocs(*it, deviceId, false);
        DistributedHardware::DmDeviceInfo deviceInfo{};
        auto res = strcpy_s(deviceInfo.networkId, it->size() + 1, it->c_str());
        if (res != 0) {
            LOGE("strcpy failed, res = %{public}d", res);
            return;
        }
        std::thread([=]() {
            int32_t ret = deviceManager->OnDeviceP2POffline(deviceInfo);
            LOGI("Close P2P Connection result %d", ret);
            }).detach();
    }
    deviceManager->RemoveNetworkId(callingTokenId);
    LOGI("Daemon::DfsListenerDeathRecipient OnremoteDied end");
    return;
}

int32_t Daemon::PushAsset(int32_t userId,
                          const sptr<AssetObj> &assetObj,
                          const sptr<IAssetSendCallback> &sendCallback)
{
    LOGI("Daemon::PushAsset begin.");
    if (assetObj == nullptr || sendCallback == nullptr) {
        LOGE("param is nullptr.");
        return E_NULLPTR;
    }
    auto taskId = assetObj->srcBundleName_ + assetObj->sessionId_;
    if (taskId.empty()) {
        LOGE("assetObj info is null.");
        return E_NULLPTR;
    }
    AssetCallbackManager::GetInstance().AddSendCallback(taskId, sendCallback);
    auto pushData = std::make_shared<PushAssetData>(userId, assetObj);
    auto msgEvent = AppExecFwk::InnerEvent::Get(DEAMON_EXECUTE_PUSH_ASSET, pushData, 0);

    std::lock_guard<std::mutex> lock(eventHandlerMutex_);
    if (eventHandler_ == nullptr) {
        LOGE("eventHandler has not find");
        AssetCallbackManager::GetInstance().RemoveSendCallback(taskId);
        return E_EVENT_HANDLER;
    }
    bool isSucc = eventHandler_->SendEvent(msgEvent, 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
    if (!isSucc) {
        LOGE("Daemon event handler post push asset event fail.");
        AssetCallbackManager::GetInstance().RemoveSendCallback(taskId);
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
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS