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

#ifndef FILEMANAGEMENT_DFS_SERVICE_SOFTBUS_ASSET_SEND_LISTENER_H
#define FILEMANAGEMENT_DFS_SERVICE_SOFTBUS_ASSET_SEND_LISTENER_H

#include <cstdint>
#include <string>
#include <map>
#include <mutex>

#include "transport/socket.h"
#include "transport/trans_type.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
class SoftBusAssetSendListener {
public:
    static void OnFile(int32_t socket, FileEvent *event);
    static void OnSendAssetFinished(int32_t socketId, const char **fileList, int32_t fileCnt);
    static void OnSendAssetError(int32_t socketId, const char **fileList, int32_t fileCnt, int32_t errorCode);
    static void OnSendShutdown(int32_t sessionId, ShutdownReason reason);

    static void AddFileMap(const std::string &taskId, bool isSingleFile);
    static bool GetIsZipFile(const std::string &taskId);
    static void RemoveFileMap(const std::string &taskId);

private:
    static std::mutex taskIsSingleFileMapMutex_;
    static std::map<std::string, bool> taskIsSingleFileMap_;
    static inline const std::string SERVICE_NAME{"ohos.storage.distributedfile.daemon"};
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // FILEMANAGEMENT_DFS_SERVICE_SOFTBUS_ASSET_SEND_LISTENER_H
