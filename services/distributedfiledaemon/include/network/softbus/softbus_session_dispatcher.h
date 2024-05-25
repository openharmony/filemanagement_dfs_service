/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef SOFTBUS_SESSION_DISPATCHER_H
#define SOFTBUS_SESSION_DISPATCHER_H

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include "transport/socket.h"
#include "transport/trans_type.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
class SoftbusAgent;
class SoftbusSessionDispatcher final {
public:
    SoftbusSessionDispatcher() = delete;
    ~SoftbusSessionDispatcher() = delete;
    static void RegisterSessionListener(const std::string busName, std::weak_ptr<SoftbusAgent>);
    static void UnregisterSessionListener(const std::string busName);
    static std::weak_ptr<SoftbusAgent> GetAgent(int32_t sessionId, std::string peerSessionName);
    static void OnSessionOpened(int32_t sessionId, PeerSocketInfo info);
    static void OnSessionClosed(int32_t sessionId, ShutdownReason reason);

private:
    static std::mutex idMapMutex_;
    static std::map<int32_t, std::pair<std::string, std::string>> idMap_;
    static std::mutex softbusAgentMutex_;
    static std::map<std::string, std::weak_ptr<SoftbusAgent>> busNameToAgent_;
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // SOFTBUS_SESSION_DISPATCHER_H