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

#include "softbus_adapter_mock.h"

namespace OHOS::FileManagement::CloudSync {

SoftbusAdapter &SoftbusAdapter::GetInstance()
{
    static SoftbusAdapter adapter;
    return adapter;
}

int32_t SoftbusAdapter::CreateSessionServer(const char *packageName, const char *sessionName)
{
    if (ISoftbusAdapterMock::iSoftbusAdapterMock_ == nullptr) {
        return -1;
    }
    return ISoftbusAdapterMock::iSoftbusAdapterMock_->CreateSessionServer(packageName, sessionName);
}

int32_t SoftbusAdapter::RemoveSessionServer(const char *packageName, const char *sessionName)
{
    return 0;
}

int32_t SoftbusAdapter::OpenSession(char *sessionName, char *peerDeviceId, char *groupId, TransDataType dataType)
{
    if (ISoftbusAdapterMock::iSoftbusAdapterMock_ == nullptr) {
        return -1;
    }
    return ISoftbusAdapterMock::iSoftbusAdapterMock_->OpenSession(sessionName,
        peerDeviceId, groupId, dataType);
}

int SoftbusAdapter::OpenSessionByP2P(char *sessionName, char *peerDeviceId, char *groupId, bool isFileType)
{
    TransDataType dataType;
    if (isFileType) {
        dataType = DATA_TYPE_FILE;
    } else {
        dataType = DATA_TYPE_BYTES;
    }
    return OpenSession(sessionName, peerDeviceId, groupId, dataType);
}

int SoftbusAdapter::SendBytes(int sessionId, const void *data, unsigned int dataLen)
{
    if (ISoftbusAdapterMock::iSoftbusAdapterMock_ == nullptr) {
        return -1;
    }
    return ISoftbusAdapterMock::iSoftbusAdapterMock_->SendBytes(sessionId, data, dataLen);
}

int SoftbusAdapter::SendFile(int sessionId, const std::vector<std::string> &sFileList,
    const std::vector<std::string> &dFileList)
{
    if (ISoftbusAdapterMock::iSoftbusAdapterMock_ == nullptr) {
        return -1;
    }
    return ISoftbusAdapterMock::iSoftbusAdapterMock_->SendFile(sessionId, sFileList, dFileList);
}

void SoftbusAdapter::OnBind(int socket, PeerSocketInfo info)
{
    return;
}

void SoftbusAdapter::OnShutdown(int32_t socket, ShutdownReason reason)
{
    return;
}

void SoftbusAdapter::OnFile(int32_t socket, FileEvent *event)
{
    return;
}
void SoftbusAdapter::OnBytes(int sessionId, const void *data, unsigned int dataLen)
{
    return;
}

int SoftbusAdapter::OnReceiveFileProcess(int sessionId, const char *firstFile,
    uint64_t bytesUpload, uint64_t bytesTotal)
{
    return 0;
}

void SoftbusAdapter::OnReceiveFileFinished(int sessionId, const char *files, int fileCnt)
{
    return;
}

const char* SoftbusAdapter::GetRecvPath()
{
    return "";
}

void SoftbusAdapter::RegisterSessionListener(std::string sessionName, std::shared_ptr<ISoftbusListener> listener)
{
    return;
}

void SoftbusAdapter::UnRegisterSessionListener(std::string sessionName)
{
    return;
}

void SoftbusAdapter::CloseSession(int sessionId)
{
    return;
}

bool SoftbusAdapter::IsSessionOpened(int sessionId)
{
    return false;
}
}