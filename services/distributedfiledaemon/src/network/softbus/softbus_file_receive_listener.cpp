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

#include "network/softbus/softbus_file_receive_listener.h"

#include "dfs_error.h"
#include "network/softbus/softbus_handler.h"
#include "session.h"
#include "utils_directory.h"
#include "utils_log.h"
#include <cinttypes>

namespace OHOS {
namespace Storage {
namespace DistributedFile {
int32_t SoftBusFileReceiveListener::OnReceiveFileStarted(int sessionId, const char *files, int fileCnt)
{
    LOGD("OnReceiveFileStarted, sessionId = %{public}d, files = %{public}s, fileCnt = %{public}d", sessionId, files,
         fileCnt);
    return 0;
}

int32_t SoftBusFileReceiveListener::OnReceiveFileProcess(int sessionId,
                                                         const char *firstFile,
                                                         uint64_t bytesUpload,
                                                         uint64_t bytesTotal)
{
    LOGD("OnReceiveFileProcess, sessionId = %{public}d, firstFile = %{public}s, bytesUpload = %{public}" PRIu64 ","
         "bytesTotal = %{public}" PRIu64 "", sessionId, firstFile, bytesUpload, bytesTotal);
    return 0;
}

void SoftBusFileReceiveListener::OnReceiveFileFinished(int sessionId, const char *files, int fileCnt)
{
    LOGD("OnReceiveFileFinished, sessionId = %{public}d, files = %{public}s, fileCnt = %{public}d", sessionId, files,
         fileCnt);
    SoftBusHandler::GetInstance().RemoveSession(sessionId, true);
}

void SoftBusFileReceiveListener::OnFileTransError(int sessionId)
{
    LOGD("OnFileTransError, sessionId = %{public}d", sessionId);
    SoftBusHandler::GetInstance().RemoveSession(sessionId);
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS