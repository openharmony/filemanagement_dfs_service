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
#ifndef FILEMANAGEMENT_DFS_SERVICE_SOFTBUS_SESSION_LISTENER_H
#define FILEMANAGEMENT_DFS_SERVICE_SOFTBUS_SESSION_LISTENER_H

#include "softbus_handler.h"
#include "transport/socket.h"
#include "transport/trans_type.h"
#include <vector>

namespace OHOS {
namespace Storage {
namespace DistributedFile {
class SoftBusSessionListener {
public:
    static void OnSessionOpened(int32_t sessionId, PeerSocketInfo info);
    static void OnSessionClosed(int32_t sessionId, ShutdownReason reason);
    static std::string GetBundleName(const std::string &uri);
    static std::string GetRealPath(const std::string &srcUri);
private:
    static int32_t QueryActiveUserId();
    static std::vector<std::string> GetFileName(const std::vector<std::string> &fileList, const std::string &path);
    static std::string GetSandboxPath(const std::string &uri);
    static std::string GetLocalUri(const std::string &uri);
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // FILEMANAGEMENT_DFS_SERVICE_SOFTBUS_SESSION_LISTENER_H