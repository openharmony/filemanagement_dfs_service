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

#include "network/softbus/softbus_file_receive_listener.h"

#include <cinttypes>

#include "dfs_error.h"
#include "network/softbus/softbus_handler.h"
#include "sandbox_helper.h"
#include "trans_mananger.h"
#include "utils_directory.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
using namespace FileManagement;
std::string SoftBusFileReceiveListener::path_ = "";
void SoftBusFileReceiveListener::OnFile(int32_t socket, FileEvent *event)
{
    if (event == nullptr) {
        LOGE("invalid parameter");
        return;
    }
    switch (event->type) {
        case FILE_EVENT_RECV_START:
            OnReceiveFileStarted(socket, event->fileCnt);
            break;
        case FILE_EVENT_RECV_PROCESS:
            OnReceiveFileProcess(socket, event->bytesProcessed, event->bytesTotal);
            break;
        case FILE_EVENT_RECV_FINISH:
            OnReceiveFileFinished(socket, event->fileCnt);
            break;
        case FILE_EVENT_RECV_ERROR:
            OnFileTransError(socket);
            break;
        case FILE_EVENT_RECV_UPDATE_PATH:
            event->UpdateRecvPath = GetRecvPath;
            break;
        default:
            LOGI("Other situations");
            break;
    }
}

const char* SoftBusFileReceiveListener::GetRecvPath()
{
    const char* path = path_.c_str();
    LOGI("path: %s", path);
    return path;
}

void SoftBusFileReceiveListener::SetRecvPath(const std::string &physicalPath)
{
    if (physicalPath.empty()) {
        LOGI("SetRecvPath physicalPath is empty.");
        return;
    }
    LOGI("SetRecvPath physicalPath: %s", physicalPath.c_str());
    if (!AppFileService::SandboxHelper::CheckValidPath(physicalPath)) {
        LOGE("invalid path.");
        return;
    }
    path_ = physicalPath;
}

std::string SoftBusFileReceiveListener::GetLocalSessionName(int32_t sessionId)
{
    std::string sessionName = "";
    sessionName = SoftBusHandler::GetSessionName(sessionId);
    return sessionName;
}

void SoftBusFileReceiveListener::OnReceiveFileStarted(int32_t sessionId, int32_t fileCnt)
{
    LOGD("OnReceiveFileStarted, sessionId = %{public}d, fileCnt = %{public}d", sessionId, fileCnt);
}

void SoftBusFileReceiveListener::OnReceiveFileProcess(int32_t sessionId, uint64_t bytesUpload, uint64_t bytesTotal)
{
    LOGD("OnReceiveFileProcess, sessionId = %{public}d, bytesUpload = %{public}" PRIu64 ","
         "bytesTotal = %{public}" PRIu64 "", sessionId, bytesUpload, bytesTotal);
    std::string sessionName = GetLocalSessionName(sessionId);
    if (sessionName.empty()) {
        LOGE("sessionName is empty");
        return;
    }
    TransManager::GetInstance().NotifyFileProgress(sessionName, bytesTotal, bytesUpload);
}

void SoftBusFileReceiveListener::OnReceiveFileFinished(int32_t sessionId, int32_t fileCnt)
{
    LOGD("OnReceiveFileFinished, sessionId = %{public}d, fileCnt = %{public}d", sessionId, fileCnt);
    std::string sessionName = GetLocalSessionName(sessionId);
    if (sessionName.empty()) {
        LOGE("sessionName is empty");
        return;
    }
    TransManager::GetInstance().NotifyFileFinished(sessionName);
    TransManager::GetInstance().DeleteTransTask(sessionName);
    SoftBusHandler::GetInstance().ChangeOwnerIfNeeded(sessionId, sessionName);
    SoftBusHandler::GetInstance().CloseSession(sessionId, sessionName);
}

void SoftBusFileReceiveListener::OnFileTransError(int32_t sessionId)
{
    LOGD("OnFileTransError, sessionId = %{public}d", sessionId);
    std::string sessionName = GetLocalSessionName(sessionId);
    if (sessionName.empty()) {
        LOGE("sessionName is empty");
        return;
    }
    TransManager::GetInstance().NotifyFileFailed(sessionName);
    TransManager::GetInstance().DeleteTransTask(sessionName);
    SoftBusHandler::GetInstance().CloseSession(sessionId, sessionName);
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS