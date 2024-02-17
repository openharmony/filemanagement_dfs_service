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

#include "network/softbus/softbus_handler.h"

#include "dfs_error.h"
#include "network/softbus/softbus_file_receive_listener.h"
#include "network/softbus/softbus_file_send_listener.h"
#include "network/softbus/softbus_session_listener.h"
#include "utils_log.h"
#include <utility>

#include "session.h"
#include "utils_directory.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
constexpr uint32_t MAX_SIZE = 128;
using namespace OHOS::FileManagement;
SoftBusHandler::SoftBusHandler()
{
    sessionListener_.OnSessionOpened = DistributedFile::SoftBusSessionListener::OnSessionOpened;
    sessionListener_.OnSessionClosed = DistributedFile::SoftBusSessionListener::OnSessionClosed;
    sessionListener_.OnBytesReceived = nullptr;
    sessionListener_.OnMessageReceived = nullptr;
    sessionListener_.OnQosEvent = nullptr;

    fileReceiveListener_.OnReceiveFileStarted = DistributedFile::SoftBusFileReceiveListener::OnReceiveFileStarted;
    fileReceiveListener_.OnReceiveFileProcess = DistributedFile::SoftBusFileReceiveListener::OnReceiveFileProcess;
    fileReceiveListener_.OnReceiveFileFinished = DistributedFile::SoftBusFileReceiveListener::OnReceiveFileFinished;
    fileReceiveListener_.OnFileTransError = DistributedFile::SoftBusFileReceiveListener::OnFileTransError;

    fileSendListener_.OnSendFileProcess = DistributedFile::SoftBusFileSendListener::OnSendFileProcess;
    fileSendListener_.OnSendFileFinished = DistributedFile::SoftBusFileSendListener::OnSendFileFinished;
    fileSendListener_.OnFileTransError = DistributedFile::SoftBusFileSendListener::OnFileTransError;
}

SoftBusHandler::~SoftBusHandler() = default;

SoftBusHandler &SoftBusHandler::GetInstance()
{
    LOGD("SoftBusHandle::GetInstance");
    static SoftBusHandler handle;
    return handle;
}

int32_t SoftBusHandler::CreateSessionServer(const std::string &packageName, const std::string &sessionName)
{
    return ::CreateSessionServer(packageName.c_str(), sessionName.c_str(), &sessionListener_);
}

int32_t SoftBusHandler::SetFileReceiveListener(const std::string &packageName,
                                               const std::string &sessionName,
                                               const std::string &dstPath)
{
    return ::SetFileReceiveListener(packageName.c_str(), sessionName.c_str(), &fileReceiveListener_, dstPath.c_str());
}

int32_t SoftBusHandler::SetFileSendListener(const std::string &packageName, const std::string &sessionName)
{
    return ::SetFileSendListener(packageName.c_str(), sessionName.c_str(), &fileSendListener_);
}

void SoftBusHandler::ChangeOwnerIfNeeded(int sessionId)
{
    char sessionName[MAX_SIZE] = {};
    auto ret = ::GetMySessionName(sessionId, sessionName, MAX_SIZE);
    if (ret != E_OK) {
        LOGE("GetMySessionName failed, ret = %{public}d", ret);
        return;
    }
    SoftBusSessionPool::SessionInfo sessionInfo {};
    ret = SoftBusSessionPool::GetInstance().GetSessionInfo(sessionName, sessionInfo);
    if (!ret) {
        LOGE("GetSessionInfo failed");
        return;
    }
    if (DistributedFile::Utils::ChangeOwnerRecursive(sessionInfo.dstPath, sessionInfo.uid, sessionInfo.uid) != 0) {
        LOGE("ChangeOwnerRecursive failed");
    }
}

void SoftBusHandler::CloseSession(int sessionId)
{
    char sessionName[MAX_SIZE] = {};
    auto ret = ::GetMySessionName(sessionId, sessionName, MAX_SIZE);
    if (ret != E_OK) {
        LOGE("GetMySessionName failed, ret = %{public}d", ret);
        return;
    }
    ::CloseSession(sessionId);
    RemoveSessionServer(SERVICE_NAME.c_str(), sessionName);
    SoftBusSessionPool::GetInstance().DeleteSessionInfo(sessionName);
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS