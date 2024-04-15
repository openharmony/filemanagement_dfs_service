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
#include "network/softbus/softbus_session_pool.h"
#include "utils_log.h"
#include <algorithm>

namespace OHOS {
namespace Storage {
namespace DistributedFile {
constexpr int32_t SESSION_COUNT = 10;
SoftBusSessionPool &SoftBusSessionPool::GetInstance()
{
    static SoftBusSessionPool instance;
    return instance;
}

std::string SoftBusSessionPool::GenerateSessionName()
{
    std::lock_guard<std::mutex> createSessionMutex(sessionMutex_);
    for (int32_t i = 0; i < SESSION_COUNT; i++) {
        std::string sessionName = std::string(SESSION_NAME_PREFIX) + std::to_string(i);
        auto it = sessionMap_.find(sessionName);
        if (it == sessionMap_.end()) {
            return sessionName;
        }
    }
    return "";
}

void SoftBusSessionPool::AddSessionInfo(const std::string &sessionName, const SessionInfo &sessionInfo)
{
    std::lock_guard<std::mutex> createSessionMutex(sessionMutex_);
    sessionMap_.insert(std::pair<std::string, SessionInfo>(sessionName, sessionInfo));
}

void SoftBusSessionPool::DeleteSessionInfo(const std::string &sessionName)
{
    std::lock_guard<std::mutex> createSessionMutex(sessionMutex_);
    sessionMap_.erase(sessionName);
}

bool SoftBusSessionPool::GetSessionInfo(const std::string &sessionName, SessionInfo &sessionInfo)
{
    std::lock_guard<std::mutex> createSessionMutex(sessionMutex_);
    for (const auto &it : sessionMap_) {
        if (it.first == sessionName) {
            sessionInfo = it.second;
            return true;
        }
    }
    return false;
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS