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

#ifndef FILEMANAGEMENT_DFS_SERVICE_SOFTBUS_FILE_RECEIVE_LISTENER_H
#define FILEMANAGEMENT_DFS_SERVICE_SOFTBUS_FILE_RECEIVE_LISTENER_H

#include "network/softbus/softbus_session_pool.h"
#include <cstdint>
#include "transport/socket.h"
#include "transport/trans_type.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
class SoftBusFileReceiveListener {
public:
    static void OnFile(int32_t socket, FileEvent *event);
    static void OnReceiveFileStarted(int32_t sessionId, int32_t fileCnt);
    static void OnReceiveFileProcess(int32_t sessionId, uint64_t bytesUpload, uint64_t bytesTotal);
    static void OnReceiveFileFinished(int32_t sessionId, int32_t fileCnt);
    static void OnFileTransError(int32_t sessionId);
    static std::string GetLocalSessionName(int32_t socket);
    static void SetRecvPath(const std::string &physicalPath);
    static const char* GetRecvPath();

private:
    static inline const std::string SERVICE_NAME{"ohos.storage.distributedfile.daemon"};
    static std::string path_;
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // FILEMANAGEMENT_DFS_SERVICE_SOFTBUS_FILE_RECEIVE_LISTENER_H