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

#include "softbus_adapter.h"

#include <cinttypes>
#include <cstddef>
#include <mutex>

#include "dfs_error.h"
#include "utils_log.h"


namespace OHOS::FileManagement::CloudSync {
using namespace std;
constexpr int MIN_BW = 1024 * 1024 * 40;
constexpr int MAX_WAIT_TIMEOUT = 10000;
constexpr int MIN_LATENCY = 1000;
const string SERVICE_NAME = "OHOS.Filemanagement.Dfs.ICloudSyncService";

SoftbusAdapter &SoftbusAdapter::GetInstance()
{
    static SoftbusAdapter instance;
    return instance;
}

int32_t SoftbusAdapter::CreateSessionServer(const char *packageName, const char *sessionName)
{
    SocketInfo info = {
        .name = const_cast<char*>(sessionName),
        .pkgName = const_cast<char*>(packageName),
    };
    int socket = ::Socket(info);
    if (socket <= 0) {
        LOGE("Failed to CreateSessionServer ret:%{public}d, sessionName:%{public}s", socket, sessionName);
        return ERR_BAD_VALUE;
    }
    std::string saveKey = std::string(sessionName) + std::string(packageName);
    auto sessionAndPackage = sessionAndPackageMap_.find(socket);
    if (sessionAndPackage == sessionAndPackageMap_.end()) {
        sessionAndPackageMap_.insert({socket, saveKey});
    }
    QosTV serverQos[] = {
        { .qos = QOS_TYPE_MIN_BW,            .value = MIN_BW},
        { .qos = QOS_TYPE_MAX_WAIT_TIMEOUT,      .value = MAX_WAIT_TIMEOUT },
        { .qos = QOS_TYPE_MIN_LATENCY,       .value = MIN_LATENCY},
    };

    ISocketListener listener = {
        .OnBind = SoftbusAdapter::OnBind,
        .OnShutdown = SoftbusAdapter::OnShutdown,
        .OnBytes = SoftbusAdapter::OnBytes,
        .OnMessage = nullptr,
        .OnStream = nullptr,
        .OnFile = SoftbusAdapter::OnFile,
    };

    int32_t ret = ::Listen(socket, serverQos, QOS_COUNT, &listener);
    if (ret != 0) {
        LOGE("Failed to CreateSessionServer sessionName:%{public}s", sessionName);
        return ret;
    }
    LOGD("Succeed to CreateSessionServer, sessionName:%{public}s", sessionName);
    return E_OK;
}

int32_t SoftbusAdapter::RemoveSessionServer(const char *packageName, const char *sessionName)
{
    std::string val = std::string(sessionName) + std::string(packageName);
    int32_t socket = SoftbusAdapter::GetInstance().GetSocketNameFromMap(val);
    string existSessionName = SoftbusAdapter::GetInstance().GetSessionNameFromMap(socket);
    if (strcmp(existSessionName.c_str(), sessionName) != 0) {
        LOGE("Failed to RemoveSessionServer sessionName:%{public}s", sessionName);
        return ERR_BAD_VALUE;
    }
    SoftbusAdapter::GetInstance().RemoveSesion(socket);
    ::Shutdown(socket);
    LOGD("Succeed to RemoveSessionServer, sessionName:%{public}s", sessionName);
    return E_OK;
}

void SoftbusAdapter::OnBind(int socket, PeerSocketInfo info)
{
    string sessionName = info.name;
    string networkId = info.networkId;
    if (sessionName.empty()) {
        LOGE("get session name failed");
        return;
    }

    auto listener = SoftbusAdapter::GetInstance().GetListener(sessionName);
    if (!listener) {
        LOGD("UnRegisterListener for session %{public}d", socket);
        return;
    }

    listener->OnSessionOpened(socket, 0);
    SoftbusAdapter::GetInstance().AcceptSesion(socket, sessionName, networkId);
}

void SoftbusAdapter::OnShutdown(int32_t socket, ShutdownReason reason)
{
    LOGD("Session OnShutdown, sessionId:%{public}d, reason:%{public}d", socket, reason);
    string sessionName = SoftbusAdapter::GetInstance().GetSessionNameFromMap(socket);
    if (sessionName.empty()) {
        LOGE("get session name failed");
        return;
    }

    auto listener = SoftbusAdapter::GetInstance().GetListener(sessionName);
    if (!listener) {
        LOGD("UnRegisterListener for session %{public}d", socket);
        return;
    }

    listener->OnSessionClosed(socket);
    SoftbusAdapter::GetInstance().RemoveSesion(socket);
}

void SoftbusAdapter::OnBytes(int socket, const void *data, unsigned int dataLen)
{
    LOGD("OnBytes invoked, dataLen:%{public}d", dataLen);
    string sessionName = SoftbusAdapter::GetInstance().GetSessionNameFromMap(socket);
    if (sessionName.empty()) {
        LOGE("get session name failed");
        return;
    }

    string peerDeviceId = SoftbusAdapter::GetInstance().GetPeerNetworkId(socket);
    if (peerDeviceId.empty()) {
        LOGE("get peerDeviceId name failed");
        return;
    }

    auto listener = SoftbusAdapter::GetInstance().GetListener(sessionName);
    if (!listener) {
        LOGD("UnRegisterListener for session %{public}d", socket);
        return;
    }

    listener->OnDataReceived(peerDeviceId, socket, data, dataLen);
}

int SoftbusAdapter::OnReceiveFileProcess(int sessionId,
                                         const char *firstFile,
                                         uint64_t bytesUpload,
                                         uint64_t bytesTotal)
{
    LOGD(
        "File receive process sessionId = %{public}d, first file:%{public}s, bytesUpload:%{public}" PRIu64 ", "
        "bytesTotal:%{public}" PRIu64 "",
        sessionId, GetAnonyString(firstFile).c_str(), bytesUpload, bytesTotal);
    return E_OK;
}

void SoftbusAdapter::OnReceiveFileFinished(int sessionId, const char *files, int fileCnt)
{
    LOGD("OnReceiveFileFinished invoked, files:%{public}s, fileCnt:%{public}d", files, fileCnt);
    string sessionName = SoftbusAdapter::GetInstance().GetSessionNameFromMap(sessionId);
    if (sessionName.empty()) {
        LOGE("get session name failed");
        return;
    }

    string peerNetworkId = SoftbusAdapter::GetInstance().GetPeerNetworkId(sessionId);
    if (peerNetworkId.empty()) {
        LOGE("get peerDeviceId name failed");
        return;
    }

    auto listener = SoftbusAdapter::GetInstance().GetListener(sessionName);
    if (!listener) {
        LOGD("UnRegisterListener for session %{public}d", sessionId);
        return;
    }

    listener->OnFileReceived(peerNetworkId, files, E_OK);
}

const char* SoftbusAdapter::GetRecvPath()
{
    return "/mnt/hmdfs/100/account/device_view/local/data/";
}

void SoftbusAdapter::OnFile(int32_t socket, FileEvent *event)
{
    if (event->type == FILE_EVENT_RECV_UPDATE_PATH) {
        event->UpdateRecvPath = GetRecvPath;
    }
}

int SoftbusAdapter::OpenSession(char *sessionName,
                                char *peerDeviceId,
                                char *groupId,
                                TransDataType dataType)
{
    SocketInfo info = {
        .name = sessionName,
        .peerName = sessionName,
        .peerNetworkId = peerDeviceId,
        .pkgName = const_cast<char*>(SERVICE_NAME.c_str()),
        .dataType = dataType,
    };
    int32_t socket = Socket(info);
    if (socket <= 0) {
        return ERR_BAD_VALUE;
    }
    std::string saveKey = std::string(sessionName) + std::string(SERVICE_NAME);
    auto sessionAndPackage = sessionAndPackageMap_.find(socket);
    if (sessionAndPackage == sessionAndPackageMap_.end()) {
        sessionAndPackageMap_.insert({socket, saveKey});
    }
    QosTV clientQos[] = {
        { .qos = QOS_TYPE_MIN_BW,            .value = MIN_BW},
        { .qos = QOS_TYPE_MAX_WAIT_TIMEOUT,      .value = MAX_WAIT_TIMEOUT },
        { .qos = QOS_TYPE_MIN_LATENCY,       .value = MIN_LATENCY},
    };

    ISocketListener listener = {
        .OnBind = SoftbusAdapter::OnBind,
        .OnShutdown = SoftbusAdapter::OnShutdown,
        .OnBytes = SoftbusAdapter::OnBytes,
        .OnFile = SoftbusAdapter::OnFile,
    };
    SoftbusAdapter::GetInstance().AcceptSesion(socket, sessionName, peerDeviceId);
    int32_t ret = ::Bind(socket, clientQos, QOS_COUNT, &listener);
    if (ret != 0) {
        ::Shutdown(socket);
    }
    return ret;
}

int SoftbusAdapter::OpenSessionByP2P(char *sessionName,
                                     char *peerDeviceId,
                                     char *groupId,
                                     bool isFileType)
{
    TransDataType dataType;
    if (isFileType) {
        dataType = DATA_TYPE_FILE;
    } else {
        dataType = DATA_TYPE_BYTES;
    }
    return OpenSession(sessionName, peerDeviceId, groupId, dataType);
}

void SoftbusAdapter::CloseSession(int sessionId)
{
    ::CloseSession(sessionId);
}

int SoftbusAdapter::SendBytes(int sessionId, const void *data, unsigned int dataLen)
{
    return ::SendBytes(sessionId, data, dataLen);
}

int SoftbusAdapter::SendFile(int sessionId,
                             const std::vector<std::string> &sFileList,
                             const std::vector<std::string> &dFileList)
{
    vector<const char *> sourceFileList;
    vector<const char *> destFileList;

    for (const string &file : sFileList) {
        sourceFileList.push_back(file.data());
    }

    for (const string &file : dFileList) {
        destFileList.push_back(file.data());
    }

    return ::SendFile(sessionId, sourceFileList.data(), destFileList.data(), sourceFileList.size());
}

/* should use this interface when session closed */
std::string SoftbusAdapter::GetSessionNameFromMap(int sessionId)
{
    lock_guard<mutex> lock(sessionMutex_);
    auto iter = sessionNameMap_.find(sessionId);
    if (iter != sessionNameMap_.end()) {
        return iter->second;
    }
    return "";
}

int32_t SoftbusAdapter::GetSocketNameFromMap(std::string sessionAndPack)
{
    lock_guard<mutex> lock(sessionMutex_);
    int32_t socket = -1;
    for (const auto& pair : sessionAndPackageMap_) {
        if (pair.second == sessionAndPack) {
            socket = pair.first;
            break;
        }
    }
    return socket;
}

std::string SoftbusAdapter::GetPeerNetworkId(int sessionId)
{
    lock_guard<mutex> lock(sessionMutex_);
    auto iter = networkIdMap_.find(sessionId);
    if (iter != networkIdMap_.end()) {
        return iter->second;
    }
    return "";
}

void SoftbusAdapter::RegisterSessionListener(std::string sessionName, std::shared_ptr<ISoftbusListener> listener)
{
    std::unique_lock<shared_mutex> lock(listenerMutex_);
    listeners_[sessionName] = listener;
}

void SoftbusAdapter::UnRegisterSessionListener(std::string sessionName)
{
    std::unique_lock<shared_mutex> lock(listenerMutex_);
    listeners_.erase(sessionName);
}

std::shared_ptr<ISoftbusListener> SoftbusAdapter::GetListener(std::string sessionName)
{
    shared_lock<shared_mutex> lock(listenerMutex_);
    if (listeners_.count(sessionName) == 0) {
        return nullptr;
    }
    return listeners_[sessionName];
}

bool SoftbusAdapter::IsSessionOpened(int sessionId)
{
    lock_guard<mutex> lock(sessionMutex_);
    auto iter = sessionOpenedMap_.find(sessionId);
    if (iter == sessionOpenedMap_.end()) {
        return false;
    }
    return iter->second;
}

void SoftbusAdapter::AcceptSesion(int sessionId, const std::string &sessionName, const std::string &networkId)
{
    lock_guard<mutex> lock(sessionMutex_);
    auto iter = sessionOpenedMap_.find(sessionId);
    if (iter == sessionOpenedMap_.end()) {
        sessionOpenedMap_.insert({sessionId, true});
    }

    auto sessionNameMap = sessionNameMap_.find(sessionId);
    if (sessionNameMap == sessionNameMap_.end()) {
        sessionNameMap_.insert({sessionId, sessionName});
    }

    auto networkIdMap = networkIdMap_.find(sessionId);
    if (networkIdMap == networkIdMap_.end()) {
        networkIdMap_.insert({sessionId, networkId});
    }
}

void SoftbusAdapter::RemoveSesion(int sessionId)
{
    lock_guard<mutex> lock(sessionMutex_);
    auto iter = sessionOpenedMap_.find(sessionId);
    if (iter != sessionOpenedMap_.end()) {
        sessionOpenedMap_.erase(iter);
    }

    auto sessionNameMap = sessionNameMap_.find(sessionId);
    if (sessionNameMap != sessionNameMap_.end()) {
        sessionNameMap_.erase(sessionNameMap);
    }

    auto networkIdMap = networkIdMap_.find(sessionId);
    if (networkIdMap != networkIdMap_.end()) {
        networkIdMap_.erase(networkIdMap);
    }
}
} // namespace OHOS::FileManagement::CloudSync