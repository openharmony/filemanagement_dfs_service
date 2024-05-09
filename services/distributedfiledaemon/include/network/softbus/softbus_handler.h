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

#ifndef FILEMANAGEMENT_DFS_SERVICE_SOFTBUS_HANDLER_H
#define FILEMANAGEMENT_DFS_SERVICE_SOFTBUS_HANDLER_H

#include "transport/socket.h"
#include "transport/trans_type.h"
#include <map>
#include <string>
#include <mutex>

namespace OHOS {
namespace Storage {
namespace DistributedFile {
typedef enum {
    DFS_CHANNLE_ROLE_SOURCE = 0,
    DFS_CHANNLE_ROLE_SINK = 1,
} DFS_CHANNEL_ROLE;
class SoftBusHandler {
public:
    SoftBusHandler();
    ~SoftBusHandler();
    static SoftBusHandler &GetInstance();
    int32_t CreateSessionServer(const std::string &packageName, const std::string &sessionName,
        DFS_CHANNEL_ROLE role, const std::string physicalPath);
    int32_t OpenSession(const std::string &mySessionName, const std::string &peerSessionName,
        const std::string &peerDevId, DFS_CHANNEL_ROLE role);
    void ChangeOwnerIfNeeded(int32_t sessionId, const std::string sessionName);
    void CloseSession(int32_t sessionId, const std::string sessionName);
    void CloseSessionWithSessionName(const std::string sessionName);
    static std::string GetSessionName(int32_t sessionId);
    static void OnSinkSessionOpened(int32_t sessionId, PeerSocketInfo info);
    static bool IsSameAccount(const std::string &networkId);

private:
    static std::mutex clientSessNameMapMutex_;
    static std::map<int32_t, std::string> clientSessNameMap_;
    static std::mutex serverIdMapMutex_;
    static std::map<std::string, int32_t> serverIdMap_;
    static inline const std::string SERVICE_NAME{"ohos.storage.distributedfile.daemon"};
    std::map<DFS_CHANNEL_ROLE, ISocketListener> sessionListener_;
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS

#endif // FILEMANAGEMENT_DFS_SERVICE_SOFTBUS_HANDLER_H