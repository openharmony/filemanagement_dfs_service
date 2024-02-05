/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
#include <stdexcept>

#include "accesstoken_kit.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "connection_detector.h"
#include "device/device_manager_agent.h"
#include "dfs_error.h"
#include "ipc_skeleton.h"
#include "iremote_object.h"
#include "iservice_registry.h"
#include "mountpoint/mount_manager.h"
#include "network/softbus/softbus_handler.h"
#include "network/softbus/softbus_session_dispatcher.h"
#include "network/softbus/softbus_session_pool.h"
#include "network/softbus/softbus_session_listener.h"
#include "sandbox_helper.h"
#include "system_ability_definition.h"
#include "trans_mananger.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
using namespace std;
using namespace OHOS::AppFileService;
using namespace OHOS::FileManagement;
using HapTokenInfo = OHOS::Security::AccessToken::HapTokenInfo;
using AccessTokenKit = OHOS::Security::AccessToken::AccessTokenKit;

constexpr int32_t LINK_TYPE_NUM = 4;
const string FILE_MANAGER_AUTHORITY = "docs";
const string MEDIA_AUTHORITY = "media";
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
        AddSystemAbilityListener(COMMON_EVENT_SERVICE_ID);
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
    LOGI("Stop finished successfully");
}

void Daemon::OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    (void)systemAbilityId;
    (void)deviceId;
    RegisterOsAccount();
}

void Daemon::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    (void)deviceId;
    if (systemAbilityId != COMMON_EVENT_SERVICE_ID) {
        LOGE("systemAbilityId is not COMMON_EVENT_SERVICE_ID");
        return;
    }

    if (subScriber_ == nullptr) {
        LOGE("Daemon::OnRemoveSystemAbility subscriberPtr is nullptr");
        return;
    }

    bool subscribeResult = EventFwk::CommonEventManager::UnSubscribeCommonEvent(subScriber_);
    LOGI("Daemon::OnRemoveSystemAbility subscribeResult = %{public}d", subscribeResult);
    subScriber_ = nullptr;
}

int32_t Daemon::OpenP2PConnection(const DistributedHardware::DmDeviceInfo &deviceInfo)
{
    auto path = ConnectionDetector::ParseHmdfsPath();
    stringstream ss;
    ss << ConnectionDetector::MocklispHash(path);
    auto targetDir = ss.str();
    auto networkId = std::string(deviceInfo.networkId);
    int32_t ret = 0;
    if (!ConnectionDetector::GetConnectionStatus(targetDir, networkId)) {
        LOGI("Get connection status not ok, try again.");
        ret = DeviceManagerAgent::GetInstance()->OnDeviceP2POnline(deviceInfo);
        if (ret != NO_ERROR) {
            LOGE("OpenP2PConnection failed, ret = %{public}d", ret);
        } else {
            ret = ConnectionDetector::RepeatGetConnectionStatus(targetDir, networkId);
            LOGI("RepeatGetConnectionStatus end, ret = %{public}d", ret);
        }
    }
    return ret;
}

int32_t Daemon::CloseP2PConnection(const DistributedHardware::DmDeviceInfo &deviceInfo)
{
    LOGI("Close P2P Connection");
    std::thread([=]() {
        int32_t ret = DeviceManagerAgent::GetInstance()->OnDeviceP2POffline(deviceInfo);
        LOGI("Close P2P Connection result %d", ret);
        return ret;
        }).detach();
    return 0;
}

int32_t Daemon::RequestSendFile(const std::string &srcUri,
                                const std::string &dstPath,
                                const std::string &dstDeviceId,
                                const std::string &sessionName)
{
    auto ret = SoftBusHandler::GetInstance().CreateSessionServer(IDaemon::SERVICE_NAME.c_str(), sessionName.c_str());
    if (ret != E_OK) {
        LOGE("CreateSessionServer failed, ret = %{public}d", ret);
        return E_SOFTBUS_SESSION_FAILED;
    }

    ret = SoftBusHandler::GetInstance().SetFileSendListener(IDaemon::SERVICE_NAME.c_str(), sessionName.c_str());
    if (ret != E_OK) {
        LOGE("SetFileSendListener failed, ret = %{public}d", ret);
        RemoveSessionServer(IDaemon::SERVICE_NAME.c_str(), sessionName.c_str());
        return E_SOFTBUS_SESSION_FAILED;
    }

    SessionAttribute attr{
        .dataType = TYPE_FILE,
        .linkTypeNum = LINK_TYPE_NUM,
        .linkType{LINK_TYPE_WIFI_P2P, LINK_TYPE_WIFI_WLAN_5G, LINK_TYPE_WIFI_WLAN_2G, LINK_TYPE_BR},
        .fastTransData = nullptr,
        .fastTransDataSize = 0,
    };
    auto sessionId = ::OpenSession(sessionName.c_str(), sessionName.c_str(), dstDeviceId.c_str(), "groupId", &attr);
    if (sessionId <= 0) {
        LOGE("OpenSession failed");
        return E_SOFTBUS_SESSION_FAILED;
    }

    SoftBusSessionPool::SessionInfo sessionInfo{.sessionId = sessionId, .srcUri = srcUri, .dstPath = dstPath};
    SoftBusSessionPool::GetInstance().AddSessionInfo(sessionName, sessionInfo);
    return E_OK;
}

int32_t Daemon::PrepareSession(const std::string &srcUri,
                               const std::string &dstUri,
                               const std::string &srcDeviceId,
                               const sptr<IRemoteObject> &listener)
{
    auto listenerCallback = iface_cast<IFileTransListener>(listener);
    auto sessionName = SoftBusSessionPool::GetInstance().GenerateSessionName();
    if (sessionName.empty() || listenerCallback == nullptr) {
        LOGI("SessionServer exceed max or ListenerCallback is nullptr");
        return CancelWait(sessionName, listenerCallback);
    }

    auto &deviceManager = DistributedHardware::DeviceManager::GetInstance();
    DistributedHardware::DmDeviceInfo localDeviceInfo{};
    int errCode = deviceManager.GetLocalDeviceInfo(IDaemon::SERVICE_NAME, localDeviceInfo);
    if (errCode != E_OK) {
        LOGI("GetLocalDeviceInfo failed, errCode = %{public}d", errCode);
        return CancelWait(sessionName, listenerCallback);
    }

    HapTokenInfo hapTokenInfo;
    int result = AccessTokenKit::GetHapTokenInfo(IPCSkeleton::GetCallingTokenID(), hapTokenInfo);
    if (result != Security::AccessToken::AccessTokenKitRet::RET_SUCCESS) {
        LOGE("GetHapTokenInfo failed, errCode = %{public}d", result);
        return CancelWait(sessionName, listenerCallback);
    }

    std::string physicalPath;
    auto ret = GetRealPath(dstUri, hapTokenInfo, sessionName, physicalPath);
    if (ret != E_OK) {
        LOGE("GetRealPath failed, ret = %{public}d", ret);
        return CancelWait(sessionName, listenerCallback);
    }
    SoftBusSessionPool::SessionInfo sessionInfo{.dstPath = physicalPath, .uid = IPCSkeleton::GetCallingUid()};
    SoftBusSessionPool::GetInstance().AddSessionInfo(sessionName, sessionInfo);
    ret = SoftBusHandler::GetInstance().CreateSessionServer(IDaemon::SERVICE_NAME, sessionName);
    if (ret != E_OK) {
        LOGE("CreateSessionServer failed, ret = %{public}d", ret);
        RemoveSession(sessionName);
        return CancelWait(sessionName, listenerCallback);
    }

    ret = SoftBusHandler::GetInstance().SetFileReceiveListener(IDaemon::SERVICE_NAME, sessionName, physicalPath);
    if (ret != E_OK) {
        LOGE("SetFileReceiveListener failed, ret = %{public}d", ret);
        RemoveSession(sessionName);
        return CancelWait(sessionName, listenerCallback);
    }
    TransManager::GetInstance().AddTransTask(sessionName, listenerCallback);
    return LoadRemoteSA(srcUri, physicalPath, localDeviceInfo.networkId, srcDeviceId, sessionName);
}

int32_t Daemon::LoadRemoteSA(const std::string &srcUri,
                             const std::string &dstPath,
                             const std::string &localDeviceId,
                             const std::string &remoteDeviceId,
                             const std::string &sessionName)
{
    auto sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        LOGE("Samgr is nullptr");
        RemoveSession(sessionName);
        return E_SA_LOAD_FAILED;
    }

    auto object = sam->GetSystemAbility(FILEMANAGEMENT_DISTRIBUTED_FILE_DAEMON_SA_ID, remoteDeviceId);
    if (object == nullptr) {
        LOGE("GetSystemAbility failed");
        RemoveSession(sessionName);
        return E_SA_LOAD_FAILED;
    }
    auto daemon = iface_cast<OHOS::Storage::DistributedFile::IDaemon>(object);
    if (daemon == nullptr) {
        LOGE("Connect service nullptr");
        RemoveSession(sessionName);
        return E_SA_LOAD_FAILED;
    }

    auto ret = daemon->RequestSendFile(srcUri, dstPath, localDeviceId, sessionName);
    if (ret != E_OK) {
        LOGE("RequestSendFile failed, ret = %{public}d", ret);
        RemoveSession(sessionName);
        return E_SA_LOAD_FAILED;
    }
    return ret;
}

void Daemon::RemoveSession(const std::string &sessionName)
{
    TransManager::GetInstance().DeleteTransTask(sessionName);
    SoftBusSessionPool::GetInstance().DeleteSessionInfo(sessionName);
    RemoveSessionServer(IDaemon::SERVICE_NAME.c_str(), sessionName.c_str());
}

int32_t Daemon::GetRealPath(const std::string &dstUri,
                            const HapTokenInfo &hapTokenInfo,
                            const std::string &sessionName,
                            std::string &physicalPath)
{
    Uri uri(dstUri);
    auto authority = uri.GetAuthority();
    if (authority == FILE_MANAGER_AUTHORITY || authority == MEDIA_AUTHORITY) {
        auto ret = SandboxHelper::GetPhysicalPath(dstUri, std::to_string(hapTokenInfo.userID), physicalPath);
        if (ret != E_OK) {
            LOGE("invalid uri, ret = %{public}d", ret);
            RemoveSession(sessionName);
            return E_GET_PHYSICAL_PATH_FAILED;
        }
    } else {
        auto bundleName = SoftBusSessionListener::GetBundleName(dstUri);
        if (bundleName.empty()) {
            LOGE("not find bundle name");
            RemoveSession(sessionName);
            return E_GET_PHYSICAL_PATH_FAILED;
        }
        physicalPath = "/data/service/el2/" + to_string(hapTokenInfo.userID) + "/hmdfs/account/data/" + bundleName;
    }
    LOGI("physicalPath %{public}s", physicalPath.c_str());
    if (!SandboxHelper::CheckValidPath(physicalPath)) {
        LOGE("invalid path.");
        RemoveSession(sessionName);
        return E_GET_PHYSICAL_PATH_FAILED;
    }
    return E_OK;
}

int32_t Daemon::CancelWait(const std::string &sessionName, const sptr<IFileTransListener> &listenerCallback)
{
    listenerCallback->OnFailed(sessionName);
    return E_SOFTBUS_SESSION_FAILED;
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS