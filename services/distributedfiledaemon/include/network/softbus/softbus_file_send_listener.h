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

#ifndef FILEMANAGEMENT_DFS_SERVICE_SOFTBUS_FILE_SEND_LISTENER_H
#define FILEMANAGEMENT_DFS_SERVICE_SOFTBUS_FILE_SEND_LISTENER_H

#include <stdint.h>
#include <string>

namespace OHOS {
namespace Storage {
namespace DistributedFile {
class SoftBusFileSendListener {
public:
    static int32_t OnSendFileProcess(int sessionId, uint64_t bytesUpload, uint64_t bytesTotal);
    static int32_t OnSendFileFinished(int sessionId, const char *firstFile);
    static void OnFileTransError(int sessionId);
    static void RemoveSession(int sessionId);

private:
    static inline const std::string SERVICE_NAME{"ohos.storage.distributedfile.daemon"};
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // FILEMANAGEMENT_DFS_SERVICE_SOFTBUS_FILE_SEND_LISTENER_H