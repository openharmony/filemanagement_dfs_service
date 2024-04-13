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

#include "network/softbus/softbus_file_send_listener.h"

#include <cinttypes>

#include "dfs_error.h"
#include "network/softbus/softbus_handler.h"
#include "network/softbus/softbus_session_pool.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
void SoftBusFileSendListener::OnFile(int32_t socket, FileEvent *event)
{
    if (event == nullptr) {
        LOGE("invalid paramter");
        return;
    }
    switch (event->type) {
        case FILE_EVENT_SEND_PROCESS:
            OnSendFileProcess(socket, event->bytesProcessed, event->bytesTotal);
            break;
        case FILE_EVENT_SEND_FINISH:
            OnSendFileFinished(socket);
            break;
        case FILE_EVENT_SEND_ERROR:
            OnFileTransError(socket);
            break;
        default:
            LOGI("Other situations");
            break;
    }
}

std::string SoftBusFileSendListener::GetLocalSessionName(int32_t sessionId)
{
    std::string sessionName = "";
    sessionName = SoftBusHandler::GetSessionName(sessionId);
    return sessionName;
}

void SoftBusFileSendListener::OnSendFileProcess(int32_t sessionId, uint64_t bytesUpload, uint64_t bytesTotal)
{
    LOGD("OnSendFileProcess, sessionId = %{public}d bytesUpload = %{public}" PRIu64 "bytesTotal = %{public}" PRIu64 "",
         sessionId, bytesUpload, bytesTotal);
    std::string sessionName = GetLocalSessionName(sessionId);
    if (sessionName.empty()) {
        LOGE("sessionName is empty");
        return;
    }
    if (bytesUpload == bytesTotal) {
        SoftBusSessionPool::GetInstance().DeleteSessionInfo(sessionName);
    }
}

void SoftBusFileSendListener::OnSendFileFinished(int32_t sessionId)
{
    LOGD("OnSendFileFinished, sessionId = %{public}d", sessionId);
    std::string sessionName = GetLocalSessionName(sessionId);
    if (sessionName.empty()) {
        LOGE("sessionName is empty");
        return;
    }
    SoftBusHandler::GetInstance().CloseSession(sessionId, sessionName);
}

void SoftBusFileSendListener::OnFileTransError(int32_t sessionId)
{
    LOGD("SoftBusFileSendListener::OnFileTransError, sessionId = %{public}d", sessionId);
    std::string sessionName = GetLocalSessionName(sessionId);
    if (sessionName.empty()) {
        LOGE("sessionName is empty");
        return;
    }
    SoftBusHandler::GetInstance().CloseSession(sessionId, sessionName);
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS