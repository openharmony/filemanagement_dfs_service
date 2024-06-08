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

#include "network/softbus/softbus_session_dispatcher.h"

#include <sstream>

#include "dm_device_info.h"
#include "network/softbus/softbus_agent.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
using namespace std;
mutex SoftbusSessionDispatcher::idMapMutex_;
map<int32_t, std::pair<std::string, std::string>> SoftbusSessionDispatcher::idMap_;
mutex SoftbusSessionDispatcher::softbusAgentMutex_;
map<string, weak_ptr<SoftbusAgent>> SoftbusSessionDispatcher::busNameToAgent_;

void SoftbusSessionDispatcher::RegisterSessionListener(const string busName, weak_ptr<SoftbusAgent> softbusAgent)
{
    if (busName == "") {
        stringstream ss;
        ss << "Failed to register session to softbus";
        LOGE("%{public}s", ss.str().c_str());
        throw runtime_error(ss.str());
    }
    lock_guard<mutex> lock(softbusAgentMutex_);
    auto agent = busNameToAgent_.find(busName);
    if (agent != busNameToAgent_.end()) {
        stringstream ss;
        ss << "this softbusAgent is exist, busName: " << busName.c_str();
        LOGE("%{public}s", ss.str().c_str());
        throw runtime_error(ss.str());
    } else {
        busNameToAgent_.insert(make_pair(busName, softbusAgent));
    }
    LOGD("RegisterSessionListener Success, busName:%{public}s", busName.c_str());
}
void SoftbusSessionDispatcher::UnregisterSessionListener(const string busName)
{
    lock_guard<mutex> lock(softbusAgentMutex_);
    auto agent = busNameToAgent_.find(busName);
    if (agent != busNameToAgent_.end()) {
        busNameToAgent_.erase(busName);
    } else {
        stringstream ss;
        ss << "this softbusAgent is not exist, busName: " << busName.c_str();
        LOGE("%{public}s", ss.str().c_str());
        throw runtime_error(ss.str());
    }
    LOGD("UnregisterSessionListener Success, busName:%{public}s", busName.c_str());
}
weak_ptr<SoftbusAgent> SoftbusSessionDispatcher::GetAgent(int32_t sessionId, std::string peerSessionName)
{
    if (peerSessionName.empty()) {
        return {};
    }
    lock_guard<mutex> lock(softbusAgentMutex_);
    auto agent = busNameToAgent_.find(string(peerSessionName));
    if (agent != busNameToAgent_.end()) {
        LOGD("Get softbus Agent Success, busName:%{public}s", peerSessionName.c_str());
        return agent->second;
    }
    LOGE("Get Session Agent fail, not exist! sessionId:%{public}d", sessionId);
    return {};
}
void SoftbusSessionDispatcher::OnSessionOpened(int32_t sessionId, PeerSocketInfo info)
{
    LOGI("OnSessionOpened Enter.");
    std::string peerSessionName(info.name);
    std::string peerDevId = info.networkId;
    if (!idMap_.empty()) {
        std::lock_guard<std::mutex> lock(idMapMutex_);
        auto it = idMap_.find(sessionId);
        if (it != idMap_.end()) {
            it->second = std::make_pair(peerDevId, peerSessionName);
        }
    }
    auto agent = GetAgent(sessionId, peerSessionName);
    if (auto spt = agent.lock()) {
        spt->OnSessionOpened(sessionId, info);
    } else {
        LOGE("session not exist!, session id is %{public}d", sessionId);
        return;
    }
}
void SoftbusSessionDispatcher::OnSessionClosed(int32_t sessionId, ShutdownReason reason)
{
    LOGI("OnSessionClosed Enter.");
    (void)reason;
    std::string peerSessionName = "";
    std::string peerDevId = "";
    {
        std::lock_guard<std::mutex> lock(idMapMutex_);
        auto it = idMap_.find(sessionId);
        if (it != idMap_.end()) {
            peerDevId = it->second.first;
            peerSessionName = it->second.second;
            idMap_.erase(it);
        }
    }
    
    auto agent = GetAgent(sessionId, peerSessionName);
    if (auto spt = agent.lock()) {
        spt->OnSessionClosed(sessionId, peerDevId);
    } else {
        LOGE("session not exist!, session id is %{public}d", sessionId);
    }
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
