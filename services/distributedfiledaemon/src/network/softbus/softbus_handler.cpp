/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include <sys/stat.h>
#include <utility>

#include "all_connect/all_connect_manager.h"
#include "device_manager.h"
#include "dfs_daemon_event_dfx.h"
#include "dfs_error.h"
#include "dm_device_info.h"
#include "network/softbus/softbus_file_receive_listener.h"
#include "network/softbus/softbus_file_send_listener.h"
#include "network/softbus/softbus_session_listener.h"
#include "trans_mananger.h"
#include "utils_directory.h"
#include "utils_log.h"
#include "network/devsl_dispatcher.h"
#ifdef SUPPORT_SAME_ACCOUNT
#include "inner_socket.h"
#include "trans_type_enhanced.h"
#endif

namespace OHOS {
namespace Storage {
namespace DistributedFile {
using namespace OHOS::FileManagement;
const int32_t DFS_QOS_TYPE_MIN_BW = 90 * 1024 * 1024;
const int32_t DFS_QOS_TYPE_MAX_LATENCY = 10000;
const int32_t DFS_QOS_TYPE_MIN_LATENCY = 2000;
const int32_t INVALID_SESSION_ID = -1;
#ifdef SUPPORT_SAME_ACCOUNT
const uint32_t MAX_ONLINE_DEVICE_SIZE = 10000;
#endif
constexpr size_t MAX_SIZE = 500;
std::mutex SoftBusHandler::clientSessNameMapMutex_;
std::map<int32_t, std::string> SoftBusHandler::clientSessNameMap_;
std::mutex SoftBusHandler::serverIdMapMutex_;
std::map<std::string, int32_t> SoftBusHandler::serverIdMap_;
std::mutex SoftBusHandler::networkIdMapMutex_;
std::map<int32_t, std::string> SoftBusHandler::networkIdMap_;
void SoftBusHandler::OnSinkSessionOpened(int32_t sessionId, PeerSocketInfo info)
{
    if (!SoftBusHandler::IsSameAccount(info.networkId)) {
        std::lock_guard<std::mutex> lock(serverIdMapMutex_);
        auto it = serverIdMap_.find(info.name);
        if (it != serverIdMap_.end()) {
            Shutdown(it->second);
            serverIdMap_.erase(it);
            LOGI("RemoveSessionServer success.");
        }
        Shutdown(sessionId);
        return;
    }
    {
        std::lock_guard<std::mutex> lock(SoftBusHandler::clientSessNameMapMutex_);
        SoftBusHandler::clientSessNameMap_.insert(std::make_pair(sessionId, info.name));
    }
    {
        std::lock_guard<std::mutex> lock(networkIdMapMutex_);
        networkIdMap_.insert(std::make_pair(sessionId, info.networkId));
    }

    AllConnectManager::GetInstance().PublishServiceState(DfsConnectCode::COPY_FILE, info.networkId,
        ServiceCollaborationManagerBussinessStatus::SCM_CONNECTED);
}

bool SoftBusHandler::IsSameAccount(const std::string &networkId)
{
#ifdef SUPPORT_SAME_ACCOUNT
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
#else
    return true;
#endif
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
    fileSendListener.OnShutdown = DistributedFile::SoftBusFileSendListener::OnSendFileShutdown;
    fileSendListener.OnFile = DistributedFile::SoftBusFileSendListener::OnFile;
    fileSendListener.OnBytes = nullptr;
    fileSendListener.OnMessage = nullptr;
    fileSendListener.OnQos = nullptr;
    sessionListener_[DFS_CHANNLE_ROLE_SOURCE] = fileSendListener;

    ISocketListener fileReceiveListener;
    fileReceiveListener.OnBind = DistributedFile::SoftBusFileReceiveListener::OnCopyReceiveBind;
    fileReceiveListener.OnShutdown = DistributedFile::SoftBusFileReceiveListener::OnReceiveFileShutdown;
    fileReceiveListener.OnFile = DistributedFile::SoftBusFileReceiveListener::OnFile;
    fileReceiveListener.OnBytes = nullptr;
    fileReceiveListener.OnMessage = nullptr;
    fileReceiveListener.OnQos = nullptr;
    sessionListener_[DFS_CHANNLE_ROLE_SINK] = fileReceiveListener;
}

SoftBusHandler::~SoftBusHandler() = default;

SoftBusHandler &SoftBusHandler::GetInstance()
{
    LOGI("SoftBusHandle::GetInstance");
    static SoftBusHandler handle;
    return handle;
}

int32_t SoftBusHandler::CreateSessionServer(const std::string &packageName, const std::string &sessionName,
    DFS_CHANNEL_ROLE role, const std::string physicalPath)
{
    if (packageName.empty() || sessionName.empty() || physicalPath.empty()) {
        LOGI("The parameter is empty");
        return FileManagement::ERR_BAD_VALUE;
    }
    LOGI("CreateSessionServer Enter.");
    SocketInfo serverInfo = {
        .name = const_cast<char*>(sessionName.c_str()),
        .pkgName = const_cast<char*>(packageName.c_str()),
        .dataType = DATA_TYPE_FILE,
    };
    int32_t socketId = Socket(serverInfo);
    if (socketId < E_OK) {
        LOGE("Create Socket fail socketId, socketId = %{public}d", socketId);
        RADAR_REPORT(RadarReporter::DFX_SET_DFS, RadarReporter::DFX_SET_BIZ_SCENE, RadarReporter::DFX_FAILED,
            RadarReporter::BIZ_STATE, RadarReporter::DFX_END, RadarReporter::ERROR_CODE,
            RadarReporter::CREAT_SOCKET_ERROR, RadarReporter::PACKAGE_NAME,
            RadarReporter::dSoftBus + std::to_string(socketId));
        return FileManagement::ERR_BAD_VALUE;
    }
    QosTV qos[] = {
        {.qos = QOS_TYPE_MIN_BW,        .value = DFS_QOS_TYPE_MIN_BW},
        {.qos = QOS_TYPE_MAX_LATENCY,        .value = DFS_QOS_TYPE_MAX_LATENCY},
        {.qos = QOS_TYPE_MIN_LATENCY,        .value = DFS_QOS_TYPE_MIN_LATENCY},
    };

    int32_t ret = Listen(socketId, qos, sizeof(qos) / sizeof(qos[0]), &sessionListener_[role]);
    if (ret != E_OK) {
        LOGE("Listen socket error for sessionName:%{public}s", sessionName.c_str());
        Shutdown(socketId);
        return FileManagement::ERR_BAD_VALUE;
    }
    {
        std::lock_guard<std::mutex> lock(serverIdMapMutex_);
        serverIdMap_.insert(std::make_pair(sessionName, socketId));
    }
    DistributedFile::SoftBusFileReceiveListener::SetRecvPath(physicalPath);
    LOGI("CreateSessionServer success socketId = %{public}d", socketId);
    return socketId;
}

int32_t SoftBusHandler::OpenSession(const std::string &mySessionName, const std::string &peerSessionName,
    const std::string &peerDevId, int32_t &socketId)
{
    if (mySessionName.empty() || peerSessionName.empty() || peerDevId.empty()) {
        LOGI("The parameter is empty");
        return FileManagement::ERR_BAD_VALUE;
    }
    LOGI("OpenSession Enter peerDevId: %{public}s", Utils::GetAnonyString(peerDevId).c_str());
    if (!IsSameAccount(peerDevId)) {
        LOGI("The source and sink device is not same account, not support.");
        return E_OPEN_SESSION;
    }
    QosTV qos[] = {
        {.qos = QOS_TYPE_MIN_BW,        .value = DFS_QOS_TYPE_MIN_BW},
        {.qos = QOS_TYPE_MAX_LATENCY,        .value = DFS_QOS_TYPE_MAX_LATENCY},
        {.qos = QOS_TYPE_MIN_LATENCY,        .value = DFS_QOS_TYPE_MIN_LATENCY},
    };
    if (!CreatSocketId(mySessionName, peerSessionName, peerDevId, socketId)) {
        return FileManagement::ERR_BAD_VALUE;
    }
    int32_t ret = Bind(socketId, qos, sizeof(qos) / sizeof(qos[0]), &sessionListener_[DFS_CHANNLE_ROLE_SOURCE]);
    if (ret != E_OK) {
        LOGE("Bind SocketClient error");
        Shutdown(socketId);
        return ret;
    }
    {
        std::lock_guard<std::mutex> lock(clientSessNameMapMutex_);
        clientSessNameMap_.insert(std::make_pair(socketId, mySessionName));
    }
    {
        std::lock_guard<std::mutex> lock(networkIdMapMutex_);
        networkIdMap_.insert(std::make_pair(socketId, peerDevId));
    }
    LOGI("OpenSession success socketId = %{public}d", socketId);
    return E_OK;
}

bool SoftBusHandler::CreatSocketId(const std::string &mySessionName, const std::string &peerSessionName,
    const std::string &peerDevId, int32_t &socketId)
{
    SocketInfo clientInfo = {
        .name = const_cast<char*>((mySessionName.c_str())),
        .peerName = const_cast<char*>(peerSessionName.c_str()),
        .peerNetworkId = const_cast<char*>(peerDevId.c_str()),
        .pkgName = const_cast<char*>(SERVICE_NAME.c_str()),
        .dataType = DATA_TYPE_FILE,
    };
    {
        std::lock_guard<std::mutex> lock(socketMutex_);
        socketId = Socket(clientInfo);
    }
    if (socketId < E_OK) {
        LOGE("Create OpenSoftbusChannel Socket error");
        return false;
    }
    return true;
}

void SoftBusHandler::SetSocketOpt(int32_t socketId, const char **src, uint32_t srcLen)
{
#ifdef SUPPORT_SAME_ACCOUNT
    uint64_t totalSize = 0;
    for (uint32_t i = 0; i < srcLen; ++i) {
        const char *file = src[i];
        struct stat buf {};
        if (stat(file, &buf) == -1) {
            return;
        }
        totalSize += static_cast<uint64_t>(buf.st_size);
    }

    TransFlowInfo flowInfo;
    flowInfo.sessionType = SHORT_DURATION_SESSION;
    flowInfo.flowQosType = HIGH_THROUGHPUT;
    flowInfo.flowSize = totalSize;
    ::SetSocketOpt(socketId, OPT_LEVEL_SOFTBUS, (OptType)OPT_TYPE_FLOW_INFO, (void *)&flowInfo, sizeof(TransFlowInfo));
#endif
}

int32_t SoftBusHandler::CopySendFile(int32_t socketId,
                                     const std::string &peerNetworkId,
                                     const std::string &srcUri,
                                     const std::string &dstPath)
{
    LOGI("CopySendFile socketId = %{public}d", socketId);

    std::string physicalPath = SoftBusSessionListener::GetRealPath(srcUri);
    if (physicalPath.empty()) {
        LOGE("GetRealPath failed");
        return FileManagement::ERR_BAD_VALUE;
    }
    auto fileList = OHOS::Storage::DistributedFile::Utils::GetFilePath(physicalPath);
    if (fileList.empty()) {
        LOGE("GetFilePath failed or file is empty, path %{public}s", physicalPath.c_str());
        return FileManagement::ERR_BAD_VALUE;
    }
    if (!DevslDispatcher::CompareDevslWithLocal(peerNetworkId, fileList)) {
        LOGE("remote device cannot read this files");
        return FileManagement::ERR_BAD_VALUE;
    }
    const char *src[MAX_SIZE] = {};
    for (size_t i = 0; i < fileList.size() && fileList.size() < MAX_SIZE; i++) {
        src[i] = fileList.at(i).c_str();
    }

    auto fileNameList = SoftBusSessionListener::GetFileName(fileList, physicalPath, dstPath);
    if (fileNameList.empty()) {
        LOGE("GetFileName failed, path %{public}s %{public}s", physicalPath.c_str(), dstPath.c_str());
        return FileManagement::ERR_BAD_VALUE;
    }
    const char *dst[MAX_SIZE] = {};
    for (size_t i = 0; i < fileNameList.size() && fileList.size() < MAX_SIZE; i++) {
        dst[i] = fileNameList.at(i).c_str();
    }

    LOGI("Enter SendFile.");
    SetSocketOpt(socketId, src, static_cast<uint32_t>(fileList.size()));
    auto ret = ::SendFile(socketId, src, dst, static_cast<uint32_t>(fileList.size()));
    if (ret != E_OK) {
        LOGE("SendFile failed, sessionId = %{public}d", socketId);
        return ret;
    }
    return E_OK;
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
    LOGI("CloseSession Enter socketId = %{public}d", sessionId);
    if (sessionName.empty() || sessionId <= 0) {
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
    {
        std::lock_guard<std::mutex> lock(socketMutex_);
        Shutdown(sessionId);
    }
    RemoveNetworkId(sessionId);
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
    TransManager::GetInstance().NotifyFileFailed(sessionName, E_DFS_CANCEL_SUCCESS);
    TransManager::GetInstance().DeleteTransTask(sessionName);
    CloseSession(sessionId, sessionName);
}
void SoftBusHandler::RemoveNetworkId(int32_t socketId)
{
    LOGI("RemoveNetworkId begin");
    std::lock_guard<std::mutex> lock(networkIdMapMutex_);
    auto it = networkIdMap_.find(socketId);
    if (it == networkIdMap_.end()) {
        LOGE("socketId not find, socket is %{public}d", socketId);
        return;
    }
    std::string peerNetworkId = it->second;
    networkIdMap_.erase(it->first);
    for (auto &item : networkIdMap_) {
        if (item.second == peerNetworkId) {
            return;
        }
    }
    AllConnectManager::GetInstance().PublishServiceState(DfsConnectCode::COPY_FILE, peerNetworkId,
        ServiceCollaborationManagerBussinessStatus::SCM_IDLE);
}

std::vector<int32_t> SoftBusHandler::GetsocketIdFromPeerNetworkId(const std::string &peerNetworkId)
{
    if (peerNetworkId.empty()) {
        LOGE("peerNetworkId is empty");
        return {};
    }
    std::vector<int32_t> socketIdList;
    std::lock_guard<std::mutex> lock(networkIdMapMutex_);
    for (auto item : networkIdMap_) {
        if (item.second == peerNetworkId) {
            socketIdList.emplace_back(item.first);
        }
    }

    return socketIdList;
}

bool SoftBusHandler::IsService(std::string &sessionName)
{
    std::lock_guard<std::mutex> lock(serverIdMapMutex_);
    auto it = serverIdMap_.find(sessionName);
    if (it == serverIdMap_.end()) {
        return false;
    }
    return true;
}

void SoftBusHandler::CopyOnStop(const std::string &peerNetworkId)
{
    auto socketIdList = GetsocketIdFromPeerNetworkId(peerNetworkId);

    for (auto socketId : socketIdList) {
        std::string sessionName = GetSessionName(socketId);
        if (sessionName.empty()) {
            LOGE("sessionName is empty");
            continue;
        }

        if (IsService(sessionName)) {
            TransManager::GetInstance().NotifyFileFailed(sessionName, E_DFS_CANCEL_SUCCESS);
            TransManager::GetInstance().DeleteTransTask(sessionName);
        }

        CloseSession(socketId, sessionName);
    }
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS