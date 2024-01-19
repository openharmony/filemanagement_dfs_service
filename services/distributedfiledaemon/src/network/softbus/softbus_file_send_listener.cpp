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

#include "network/softbus/softbus_file_send_listener.h"

#include <cinttypes>

#include "dfs_error.h"
#include "network/softbus/softbus_handler.h"
#include "network/softbus/softbus_session_pool.h"
#include "session.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
constexpr uint32_t MAX_SIZE = 128;
int32_t SoftBusFileSendListener::OnSendFileProcess(int sessionId, uint64_t bytesUpload, uint64_t bytesTotal)
{
    LOGD("OnSendFileProcess, sessionId = %{public}d bytesUpload = %{public}" PRIu64 "bytesTotal = %{public}" PRIu64 "",
         sessionId, bytesUpload, bytesTotal);
    if (bytesUpload == bytesTotal) {
        char sessionName[MAX_SIZE] = {};
        auto ret = ::GetMySessionName(sessionId, sessionName, MAX_SIZE);
        if (ret != FileManagement::E_OK) {
            LOGE("GetMySessionName failed, ret = %{public}d", ret);
            return FileManagement::E_OK;
        }
        SoftBusSessionPool::GetInstance().DeleteSessionInfo(sessionName);
    }
    return 0;
}

int32_t SoftBusFileSendListener::OnSendFileFinished(int sessionId, const char *firstFile)
{
    LOGD("OnSendFileFinished, sessionId = %{public}d, firstFile = %{public}s", sessionId, firstFile);
    SoftBusHandler::GetInstance().RemoveSession(sessionId);
    return 0;
}

void SoftBusFileSendListener::OnFileTransError(int sessionId)
{
    LOGD("SoftBusFileSendListener::OnFileTransError, sessionId = %{public}d", sessionId);
    SoftBusHandler::GetInstance().RemoveSession(sessionId);
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS