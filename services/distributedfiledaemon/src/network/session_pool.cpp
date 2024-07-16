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

#include "network/session_pool.h"
#include "dm_device_info.h"
#include "device/device_manager_agent.h"
#include "dfs_error.h"
#include "ipc/i_daemon.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
using namespace std;
namespace {
const int32_t MOUNT_DFS_COUNT_ONE = 1;
}

void SessionPool::OccupySession(int32_t sessionId, uint8_t linkType)
{
    lock_guard lock(sessionPoolLock_);
    occupySession_[sessionId] = linkType;
}

bool SessionPool::FindSession(int32_t sessionId)
{
    lock_guard lock(sessionPoolLock_);
    auto linkTypeIter = occupySession_.find(sessionId);
    if (linkTypeIter != occupySession_.end()) {
        return true;
    } else {
        return false;
    }
}

void SessionPool::HoldSession(shared_ptr<BaseSession> session, const std::string backStage)
{
    lock_guard lock(sessionPoolLock_);
    if (DeviceConnectCountOnly(session)) {
        LOGE("DeviceConnect Count Only");
        return;
    }
    talker_->SinkSessionTokernel(session, backStage);
    AddSessionToPool(session);
}

uint8_t SessionPool::ReleaseSession(const int32_t fd)
{
    uint8_t linkType = 0;
    std::string cid = "";
    lock_guard lock(sessionPoolLock_);
    LOGI("ReleaseSession start, fd=%{public}d", fd);
    if (fd < 0) {
        LOGI("NOTIFY OFFLINE, fd=%{public}d, deviceConnectCount clear", fd);
        if (deviceConnectCount_.empty()) {
            return FileManagement::ERR_BAD_VALUE;
        }
        deviceConnectCount_.clear();
        return FileManagement::ERR_BAD_VALUE;
    }
    for (auto iter = usrSpaceSessionPool_.begin(); iter != usrSpaceSessionPool_.end(); ++iter) {
        if ((*iter)->GetHandle() == fd) {
            auto linkTypeIter = occupySession_.find((*iter)->GetSessionId());
            if (linkTypeIter != occupySession_.end()) {
                linkType = linkTypeIter->second;
                cid = (*iter)->GetCid();
            }
            if (DeviceDisconnectCountOnly(cid, linkType, false)) {
                continue;
            }
        }
        if ((*iter)->GetHandle() == fd) {
            auto linkTypeIter = occupySession_.find((*iter)->GetSessionId());
            if (linkTypeIter != occupySession_.end()) {
                linkType = linkTypeIter->second;
                occupySession_.erase(linkTypeIter);
                (*iter)->Release();
                iter = usrSpaceSessionPool_.erase(iter);
                break;
            }
        }
    }
    return linkType;
}

void SessionPool::ReleaseSession(const string &cid, const uint8_t linkType)
{
    uint8_t mlinkType = 0;
    lock_guard lock(sessionPoolLock_);
    for (auto iter = usrSpaceSessionPool_.begin(); iter != usrSpaceSessionPool_.end(); ++iter) {
        if ((*iter)->GetCid() == cid) {
            auto linkTypeIter = occupySession_.find((*iter)->GetSessionId());
            if (linkTypeIter != occupySession_.end()) {
                mlinkType = (linkTypeIter->second == 0) ? linkType : linkTypeIter->second;
            }
            if (mlinkType == linkType && DeviceDisconnectCountOnly(cid, linkType, false)) {
                continue;
            }
            if (mlinkType == linkType) {
                (*iter)->Release();
                occupySession_.erase(linkTypeIter);
                iter = usrSpaceSessionPool_.erase(iter);
                mlinkType = 0;
                continue;
            }
        }
    }
}

void SessionPool::ReleaseAllSession()
{
    lock_guard lock(sessionPoolLock_);
    for (auto iter = usrSpaceSessionPool_.begin(); iter != usrSpaceSessionPool_.end();) {
        talker_->SinkOfflineCmdToKernel((*iter)->GetCid());
        /* device offline, session release by softbus */
        iter = usrSpaceSessionPool_.erase(iter);
    }
}

void SessionPool::AddSessionToPool(shared_ptr<BaseSession> session)
{
    usrSpaceSessionPool_.push_back(session);
}

bool SessionPool::DeviceDisconnectCountOnly(const string &cid, const uint8_t linkType, bool needErase)
{
    if (linkType != LINK_TYPE_P2P) {
        LOGI("DeviceDisconnectCountOnly return, linkType is %{public}d, not LINK_TYPE_P2P,", linkType);
        return false;
    }
    if (cid.empty()) {
        LOGE("fail to get networkId");
        return false;
    }
    std::string key = cid + "_" + std::to_string(linkType);
    auto itCount = deviceConnectCount_.find(key);
    if (itCount == deviceConnectCount_.end()) {
        LOGI("deviceConnectCount_ can not find %{public}s", Utils::GetAnonyString(key).c_str());
        return false;
    }
    if (needErase) {
        deviceConnectCount_.erase(itCount);
        LOGI("[DeviceDisconnectCountOnly]  %{public}s, needErase", Utils::GetAnonyString(key).c_str());
        return false;
    }
    if (itCount->second > MOUNT_DFS_COUNT_ONE) {
        LOGI("[DeviceDisconnectCountOnly] networkId_linkType %{public}s has already established \
            more than one link, count %{public}d, decrease count by one now",
            Utils::GetAnonyString(key).c_str(), itCount->second);
        deviceConnectCount_[key]--;
        return true;
    } else {
        LOGI("[DeviceDisconnectCountOnly] networkId_linkType %{public}s erase now", Utils::GetAnonyString(key).c_str());
        deviceConnectCount_.erase(itCount);
    }
    return false;
}

bool SessionPool::DeviceConnectCountOnly(std::shared_ptr<BaseSession> session)
{
    auto cid = session->GetCid();
    if (cid.empty()) {
        LOGE("fail to get networkId");
        return false;
    }
    std::string key = "";
    auto sessionId = session->GetSessionId();
    auto it = occupySession_.find(sessionId);
    if (it != occupySession_.end()) {
        uint8_t linkType = it->second;
        if (linkType != LINK_TYPE_P2P) {
            LOGI("DeviceConnectCountOnly return, linkType is %{public}d, not LINK_TYPE_P2P,", linkType);
            return false;
        }
        key = cid + "_" + std::to_string(linkType);
    } else {
        LOGE("occupySession find sessionId failed");
        return false;
    }
    auto itCount = deviceConnectCount_.find(key);
    if (itCount != deviceConnectCount_.end() && itCount->second > 0) {
        LOGI("[DeviceConnectCountOnly] networkId_linkType %{public}s has already established a link, \
            count %{public}d, increase count by one now", Utils::GetAnonyString(key).c_str(), itCount->second);
        deviceConnectCount_[key]++;
        return true;
    } else {
        LOGI("[DeviceConnectCountOnly] networkId_linkType %{public}s increase count by one now",
            Utils::GetAnonyString(key).c_str());
        deviceConnectCount_[key]++;
    }
    return false;
}

} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
