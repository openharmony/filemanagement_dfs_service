/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "network/softbus/softbus_handler.h"

#include <utility>

#include "device_manager.h"
#include "dfs_error.h"
#include "dm_device_info.h"
#include "network/softbus/softbus_file_receive_listener.h"
#include "network/softbus/softbus_file_send_listener.h"
#include "network/softbus/softbus_session_listener.h"
#include "trans_mananger.h"
#include "utils_directory.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
using namespace OHOS::FileManagement;
const int32_t DFS_QOS_TYPE_MIN_BW = 90 * 1024 * 1024;
const int32_t DFS_QOS_TYPE_MAX_LATENCY = 10000;
const int32_t DFS_QOS_TYPE_MIN_LATENCY = 2000;
const int32_t INVALID_SESSION_ID = -1;
const uint32_t MAX_ONLINE_DEVICE_SIZE = 10000;
std::mutex SoftBusHandler::clientSessNameMapMutex_;
std::map<int32_t, std::string> SoftBusHandler::clientSessNameMap_;
std::mutex SoftBusHandler::serverIdMapMutex_;
std::map<std::string, int32_t> SoftBusHandler::serverIdMap_;

void SoftBusHandler::OnSinkSessionOpened(int32_t sessionId, PeerSocketInfo info)
{
    std::lock_guard<std::mutex> lock(SoftBusHandler::clientSessNameMapMutex_);
    SoftBusHandler::clientSessNameMap_.insert(std::make_pair(sessionId, info.name));
}

bool SoftBusHandler::IsSameAccount(const std::string &networkId)
{
    std::vector<DistributedHardware::DmDeviceInfo> deviceList;
    DistributedHardware::DeviceManager::GetInstance().GetTrustedDeviceList(SERVICE_NAME, "", deviceList);
    if (deviceList.size() == 0 || deviceList.size() > MAX_ONLINE_DEVICE_SIZE) {
        LOGE("trust device list size is invalid, size=%zu", deviceList.size());
        return false;
    }
    for (const auto &deviceInfo : deviceList) {
        if (std::string(deviceInfo.networkId) == networkId) {
            return (deviceInfo.authForm == DistributedHardware::DmAuthForm::IDENTICAL_ACCOUNT);
        }
    }
    return false;
}

std::string SoftBusHandler::GetSessionName(int32_t sessionId)
{
    std::string sessionName = "";
    std::lock_guard<std::mutex> lock(clientSessNameMapMutex_);
    auto iter = clientSessNameMap_.find(sessionId);
    if (iter != clientSessNameMap_.end()) {
        sessionName = iter->second;
        return sessionName;
    }
    LOGE("sessionName not registered");
    return sessionName;
}

SoftBusHandler::SoftBusHandler()
{
    ISocketListener fileSendListener;
    fileSendListener.OnBind = nullptr;
    fileSendListener.OnShutdown = DistributedFile::SoftBusSessionListener::OnSessionClosed;
    fileSendListener.OnFile = DistributedFile::SoftBusFileSendListener::OnFile;
    fileSendListener.OnBytes = nullptr;
    fileSendListener.OnMessage = nullptr;
    fileSendListener.OnQos = nullptr;
    sessionListener_[DFS_CHANNLE_ROLE_SOURCE] = fileSendListener;

    ISocketListener fileReceiveListener;
    fileReceiveListener.OnBind = SoftBusHandler::OnSinkSessionOpened;
    fileReceiveListener.OnShutdown = DistributedFile::SoftBusSessionListener::OnSessionClosed;
    fileReceiveListener.OnFile = DistributedFile::SoftBusFileReceiveListener::OnFile;
    fileReceiveListener.OnBytes = nullptr;
    fileReceiveListener.OnMessage = nullptr;
    fileReceiveListener.OnQos = nullptr;
    sessionListener_[DFS_CHANNLE_ROLE_SINK] = fileReceiveListener;
}

SoftBusHandler::~SoftBusHandler() = default;

SoftBusHandler &SoftBusHandler::GetInstance()
{
    LOGD("SoftBusHandle::GetInstance");
    static SoftBusHandler handle;
    return handle;
}

int32_t SoftBusHandler::CreateSessionServer(const std::string &packageName, const std::string &sessionName,
    DFS_CHANNEL_ROLE role, const std::string physicalPath)
{
    if (packageName.empty() || sessionName.empty() || physicalPath.empty()) {
        LOGI("The parameter is empty");
        return E_SOFTBUS_SESSION_FAILED;
    }
    LOGI("CreateSessionServer Enter.");
    {
        std::lock_guard<std::mutex> lock(serverIdMapMutex_);
        if (serverIdMap_.find(sessionName) != serverIdMap_.end()) {
            LOGI("%s: Session already create.", sessionName.c_str());
            return E_OK;
        }
    }
    SocketInfo serverInfo = {
        .name = const_cast<char*>(sessionName.c_str()),
        .pkgName = const_cast<char*>(packageName.c_str()),
        .dataType = DATA_TYPE_FILE,
    };
    int32_t socketId = Socket(serverInfo);
    if (socketId < E_OK) {
        LOGE("Create Socket fail socketId, socketId = %{public}d", socketId);
        return E_SOFTBUS_SESSION_FAILED;
    }
    QosTV qos[] = {
        {.qos = QOS_TYPE_MIN_BW,        .value = DFS_QOS_TYPE_MIN_BW},
        {.qos = QOS_TYPE_MAX_LATENCY,        .value = DFS_QOS_TYPE_MAX_LATENCY},
        {.qos = QOS_TYPE_MIN_LATENCY,        .value = DFS_QOS_TYPE_MIN_LATENCY},
    };

    int32_t ret = Listen(socketId, qos, sizeof(qos) / sizeof(qos[0]), &sessionListener_[role]);
    if (ret != E_OK) {
        LOGE("Listen socket error for sessionName:%s", sessionName.c_str());
        Shutdown(socketId);
        return E_SOFTBUS_SESSION_FAILED;
    }
    {
        std::lock_guard<std::mutex> lock(serverIdMapMutex_);
        serverIdMap_.insert(std::make_pair(sessionName, socketId));
    }
    DistributedFile::SoftBusFileReceiveListener::SetRecvPath(physicalPath);
    LOGI("CreateSessionServer success.");
    return ret;
}

int32_t SoftBusHandler::OpenSession(const std::string &mySessionName, const std::string &peerSessionName,
    const std::string &peerDevId, DFS_CHANNEL_ROLE role)
{
    if (mySessionName.empty() || peerSessionName.empty() || peerDevId.empty()) {
        LOGI("The parameter is empty");
        return E_OPEN_SESSION;
    }
    LOGI("OpenSession Enter.");
    QosTV qos[] = {
        {.qos = QOS_TYPE_MIN_BW,        .value = DFS_QOS_TYPE_MIN_BW},
        {.qos = QOS_TYPE_MAX_LATENCY,        .value = DFS_QOS_TYPE_MAX_LATENCY},
        {.qos = QOS_TYPE_MIN_LATENCY,        .value = DFS_QOS_TYPE_MIN_LATENCY},
    };
    SocketInfo clientInfo = {
        .name = const_cast<char*>((mySessionName.c_str())),
        .peerName = const_cast<char*>(peerSessionName.c_str()),
        .peerNetworkId = const_cast<char*>(peerDevId.c_str()),
        .pkgName = const_cast<char*>(SERVICE_NAME.c_str()),
        .dataType = DATA_TYPE_FILE,
    };
    int32_t socketId = Socket(clientInfo);
    if (socketId < E_OK) {
        LOGE("Create OpenSoftbusChannel Socket error");
        return E_OPEN_SESSION;
    }
    int32_t ret = Bind(socketId, qos, sizeof(qos) / sizeof(qos[0]), &sessionListener_[role]);
    if (ret != E_OK) {
        LOGE("Bind SocketClient error");
        Shutdown(socketId);
        return E_OPEN_SESSION;
    }
    {
        std::lock_guard<std::mutex> lock(clientSessNameMapMutex_);
        clientSessNameMap_.insert(std::make_pair(socketId, mySessionName));
    }
    LOGI("OpenSession success.");
    return socketId;
}

void SoftBusHandler::ChangeOwnerIfNeeded(int32_t sessionId, const std::string sessionName)
{
    if (sessionName.empty()) {
        LOGI("sessionName is empty");
        return;
    }
    SoftBusSessionPool::SessionInfo sessionInfo {};
    int32_t ret = SoftBusSessionPool::GetInstance().GetSessionInfo(sessionName, sessionInfo);
    if (!ret) {
        LOGE("GetSessionInfo failed");
        return;
    }
    if (DistributedFile::Utils::ChangeOwnerRecursive(sessionInfo.dstPath, sessionInfo.uid, sessionInfo.uid) != 0) {
        LOGE("ChangeOwnerRecursive failed");
    }
}

void SoftBusHandler::CloseSession(int32_t sessionId, const std::string sessionName)
{
    LOGI("CloseSession Enter.");
    if (sessionName.empty()) {
        LOGI("sessionName is empty");
        return;
    }
    if (!serverIdMap_.empty()) {
        std::lock_guard<std::mutex> lock(serverIdMapMutex_);
        auto it = serverIdMap_.find(sessionName);
        if (it != serverIdMap_.end()) {
            int32_t serverId = it->second;
            serverIdMap_.erase(it);
            Shutdown(serverId);
            LOGI("RemoveSessionServer success.");
        }
    }
    if (!clientSessNameMap_.empty()) {
        std::lock_guard<std::mutex> lock(clientSessNameMapMutex_);
        auto it = clientSessNameMap_.find(sessionId);
        if (it != clientSessNameMap_.end()) {
            clientSessNameMap_.erase(it->first);
        }
    }
    Shutdown(sessionId);
    SoftBusSessionPool::GetInstance().DeleteSessionInfo(sessionName);
}

void SoftBusHandler::CloseSessionWithSessionName(const std::string sessionName)
{
    LOGI("CloseSessionWithSessionName Enter.");
    if (sessionName.empty()) {
        LOGI("sessionName is empty");
        return;
    }
    int32_t sessionId = INVALID_SESSION_ID;
    if (!clientSessNameMap_.empty()) {
        std::lock_guard<std::mutex> lock(clientSessNameMapMutex_);
        for (auto it : SoftBusHandler::clientSessNameMap_) {
            if (it.second == sessionName) {
                sessionId = it.first;
                clientSessNameMap_.erase(it.first);
                break;
            }
        }
    }
    TransManager::GetInstance().NotifyFileFailed(sessionName);
    TransManager::GetInstance().DeleteTransTask(sessionName);
    CloseSession(sessionId, sessionName);
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS