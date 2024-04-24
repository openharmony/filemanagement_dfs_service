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
constexpr int SESSION_NAME_SIZE_MAX = 256;
constexpr int NETWORK_ID_SIZE_MAX = 65;

SoftbusAdapter &SoftbusAdapter::GetInstance()
{
    static SoftbusAdapter instance;
    return instance;
}

int32_t SoftbusAdapter::CreateSessionServer(const char *packageName, const char *sessionName)
{
    ISessionListener sessionListener = {
        .OnSessionOpened = SoftbusAdapter::OnSessionOpened,
        .OnSessionClosed = SoftbusAdapter::OnSessionClosed,
        .OnBytesReceived = SoftbusAdapter::OnBytesReceived,
        .OnMessageReceived = nullptr,
        .OnStreamReceived = nullptr,
    };

    int ret = ::CreateSessionServer(packageName, sessionName, &sessionListener);
    if (ret != 0) {
        LOGE("Failed to CreateSessionServer ret:%{public}d, sessionName:%{public}s", ret, sessionName);
        return ret;
    }
    LOGD("Succeed to CreateSessionServer, sessionName:%{public}s", sessionName);
    return E_OK;
}

int32_t SoftbusAdapter::RemoveSessionServer(const char *packageName, const char *sessionName)
{
    int ret = ::RemoveSessionServer(packageName, sessionName);
    if (ret != 0) {
        LOGE("Failed to RemoveSessionServer ret:%{public}d, sessionName:%{public}s", ret, sessionName);
        return ret;
    }
    LOGD("Succeed to RemoveSessionServer, sessionName:%{public}s", sessionName);
    return E_OK;
}

int SoftbusAdapter::OnSessionOpened(int sessionId, int result)
{
    LOGD("Session opened, sessionId:%{public}d, result:%{public}d, Is %{public}s side", sessionId, result,
         SoftbusAdapter::GetInstance().IsFromServer(sessionId) == true ? "Server" : "Client");
    string sessionName = SoftbusAdapter::GetInstance().GetSessionNameById(sessionId);
    if (sessionName.empty()) {
        LOGE("get session name failed");
        return E_INVAL_ARG;
    }

    auto listener = SoftbusAdapter::GetInstance().GetListener(sessionName);
    if (!listener) {
        LOGD("UnRegisterListener for session %{public}d", sessionId);
        return E_OK;
    }

    auto ret = listener->OnSessionOpened(sessionId, result);
    if (ret != E_OK) {
        LOGE("session opened failed");
        return ret;
    }

    SoftbusAdapter::GetInstance().AcceptSesion(sessionId, sessionName);

    return E_OK;
}

void SoftbusAdapter::OnSessionClosed(int sessionId)
{
    LOGD("Session OnSessionClosed, sessionId:%{public}d", sessionId);
    string sessionName = SoftbusAdapter::GetInstance().GetSessionNameFromMap(sessionId);
    if (sessionName.empty()) {
        LOGE("get session name failed");
        return;
    }

    auto listener = SoftbusAdapter::GetInstance().GetListener(sessionName);
    if (!listener) {
        LOGD("UnRegisterListener for session %{public}d", sessionId);
        return;
    }

    listener->OnSessionClosed(sessionId);
    SoftbusAdapter::GetInstance().RemoveSesion(sessionId);
}

void SoftbusAdapter::OnBytesReceived(int sessionId, const void *data, unsigned int dataLen)
{
    LOGD("OnBytesReceived invoked, dataLen:%{public}d", dataLen);
    string sessionName = SoftbusAdapter::GetInstance().GetSessionNameById(sessionId);
    if (sessionName.empty()) {
        LOGE("get session name failed");
        return;
    }

    string peerDeviceId = SoftbusAdapter::GetInstance().GetPeerNetworkId(sessionId);
    if (peerDeviceId.empty()) {
        LOGE("get peerDeviceId name failed");
        return;
    }

    auto listener = SoftbusAdapter::GetInstance().GetListener(sessionName);
    if (!listener) {
        LOGD("UnRegisterListener for session %{public}d", sessionId);
        return;
    }

    listener->OnDataReceived(peerDeviceId, sessionId, data, dataLen);
}

int SoftbusAdapter::OnReceiveFileStarted(int sessionId, const char *files, int fileCnt)
{
    LOGD("File receive start sessionId = %{public}d, first file:%{public}s, fileCnt:%{public}d", sessionId, files,
         fileCnt);
    return E_OK;
}

int SoftbusAdapter::OnReceiveFileProcess(int sessionId,
                                         const char *firstFile,
                                         uint64_t bytesUpload,
                                         uint64_t bytesTotal)
{
    LOGD(
        "File receive process sessionId = %{public}d, first file:%{public}s, bytesUpload:%{public}" PRIu64 ", "
        "bytesTotal:%{public}" PRIu64 "",
        sessionId, firstFile, bytesUpload, bytesTotal);
    return E_OK;
}

void SoftbusAdapter::OnReceiveFileFinished(int sessionId, const char *files, int fileCnt)
{
    LOGD("OnReceiveFileFinished invoked, files:%{public}s, fileCnt:%{public}d", files, fileCnt);
    string sessionName = SoftbusAdapter::GetInstance().GetSessionNameById(sessionId);
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

void SoftbusAdapter::OnFileTransError(int sessionId)
{
    LOGD("OnFileTransError sessionId=%{public}d", sessionId);
}

int SoftbusAdapter::SetFileReceiveListener(const char *packageName, const char *sessionName)
{
    const char *rootDir = "/mnt/hmdfs/100/account/device_view/local/data";
    IFileReceiveListener fileReceiveListener = {
        .OnReceiveFileStarted = SoftbusAdapter::OnReceiveFileStarted,
        .OnReceiveFileProcess = SoftbusAdapter::OnReceiveFileProcess,
        .OnReceiveFileFinished = SoftbusAdapter::OnReceiveFileFinished,
        .OnFileTransError = nullptr,
    };

    int ret = ::SetFileReceiveListener(packageName, sessionName, &fileReceiveListener, rootDir);
    if (ret != 0) {
        LOGE("Failed to SetFileReceiveListener ret:%{public}d, sessionName:%{public}s", ret, sessionName);
    }
    LOGD("Succeed to SetFileReceiveListener, sessionName:%{public}s", sessionName);
    return ret;
}

int SoftbusAdapter::OpenSession(const char *sessionName,
                                const char *peerDeviceId,
                                const char *groupId,
                                const SessionAttribute *attr)
{
    return ::OpenSession(sessionName, sessionName, peerDeviceId, groupId, attr);
}

int SoftbusAdapter::OpenSessionByP2P(const char *sessionName,
                                     const char *peerDeviceId,
                                     const char *groupId,
                                     bool isFileType)
{
    SessionAttribute attr{};
    if (isFileType) {
        attr.dataType = TYPE_FILE;
    } else {
        attr.dataType = TYPE_BYTES;
    }

    int index = 0;
    attr.linkType[index++] = LINK_TYPE_WIFI_P2P;
    attr.linkType[index++] = LINK_TYPE_WIFI_WLAN_5G;
    attr.linkType[index++] = LINK_TYPE_WIFI_WLAN_2G;
    attr.linkTypeNum = index;

    return OpenSession(sessionName, peerDeviceId, groupId, &attr);
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

std::string SoftbusAdapter::GetSessionNameById(int sessionId)
{
    char buff[SESSION_NAME_SIZE_MAX] = "";
    int ret = ::GetMySessionName(sessionId, buff, sizeof(buff));
    if (ret != E_OK) {
        return "";
    }
    return string(buff);
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

std::string SoftbusAdapter::GetPeerNetworkId(int sessionId)
{
    char buff[NETWORK_ID_SIZE_MAX] = "";
    int ret = ::GetPeerDeviceId(sessionId, buff, sizeof(buff));
    if (ret != E_OK) {
        return "";
    }
    return string(buff);
}

bool SoftbusAdapter::IsFromServer(int sessionId)
{
    auto isClientSide = ::GetSessionSide(sessionId);
    if (isClientSide) {
        return false;
    }
    return true;
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

void SoftbusAdapter::AcceptSesion(int sessionId, const std::string &sessionName)
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
}
} // namespace OHOS::FileManagement::CloudSync
