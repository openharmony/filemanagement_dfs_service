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
    return E_OK;
}

int32_t SoftbusAdapter::RemoveSessionServer(const char *packageName, const char *sessionName)
{
    return E_OK;
}

int SoftbusAdapter::OnSessionOpened(int sessionId, int result)
{
    return E_OK;
}

void SoftbusAdapter::OnSessionClosed(int sessionId)
{
}

void SoftbusAdapter::OnBytesReceived(int sessionId, const void *data, unsigned int dataLen)
{
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
}

void SoftbusAdapter::OnFileTransError(int sessionId)
{
    LOGD("OnFileTransError sessionId=%{public}d", sessionId);
}

void SoftbusAdapter::SetFileReceiveListener(const char *packageName, const char *sessionName)
{
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
}

std::string SoftbusAdapter::GetPeerNetworkId(int sessionId)
{
}

bool SoftbusAdapter::IsFromServer(int sessionId)
{
}

void SoftbusAdapter::RegisterSessionListener(std::string sessionName, std::shared_ptr<ISoftbusListener> listener)
{
}

void SoftbusAdapter::UnRegisterSessionListener(std::string sessionName)
{
}

std::shared_ptr<ISoftbusListener> SoftbusAdapter::GetListener(std::string sessionName)
{
}

bool SoftbusAdapter::IsSessionOpened(int sessionId)
{
    return true;
}

void SoftbusAdapter::AcceptSesion(int sessionId)
{
}

void SoftbusAdapter::RemoveSesion(int sessionId)
{
}
} // namespace OHOS::FileManagement::CloudSync