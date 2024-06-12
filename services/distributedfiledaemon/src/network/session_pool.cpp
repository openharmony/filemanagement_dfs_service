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
#include "ipc/i_daemon.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
using namespace std;
namespace {
const int32_t MOUNT_DFS_COUNT_ONE = 1;
const uint32_t MAX_ONLINE_DEVICE_SIZE = 10000;
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
    if (fd < 0) {
        LOGI("fd=%{public}d, deviceConnectCount clear", fd);
        deviceConnectCount_.clear();
    }
    for (auto iter = usrSpaceSessionPool_.begin(); iter != usrSpaceSessionPool_.end();) {
        if ((*iter)->GetHandle() == fd) {
            auto linkTypeIter = occupySession_.find((*iter)->GetSessionId());
            if (linkTypeIter != occupySession_.end()) {
                linkType = linkTypeIter->second;
                cid = (*iter)->GetCid();
            }
        }
        if (DeviceDisconnectCountOnly(cid, linkType, false)) {
            continue;
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
        ++iter;
    }
    return linkType;
}

void SessionPool::ReleaseSession(const string &cid, const uint8_t linkType)
{
    uint8_t mlinkType = 0;
    lock_guard lock(sessionPoolLock_);
    for (auto iter = usrSpaceSessionPool_.begin(); iter != usrSpaceSessionPool_.end();) {
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
        ++iter;
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

std::string SessionPool::GetDeviceIdByCid(const std::string &cid)
{
    std::string deviceId = "";
    auto it = deviceIdByCid_.find(cid);
    if (!cid.empty() && it != deviceIdByCid_.end()) {
        deviceId = it->second;
    }
    if (!deviceId.empty()) {
        return deviceId;
    }
    std::vector<DistributedHardware::DmDeviceInfo> deviceList;
    DistributedHardware::DeviceManager::GetInstance().GetTrustedDeviceList(IDaemon::SERVICE_NAME, "", deviceList);
    if (deviceList.size() == 0 || deviceList.size() > MAX_ONLINE_DEVICE_SIZE) {
        LOGE("the size of trust device list is invalid, size=%zu", deviceList.size());
        return deviceId;
    }
    for (const auto &deviceInfo : deviceList) {
        if (!cid.empty() && std::string(deviceInfo.networkId) == cid) {
            deviceId = std::string(deviceInfo.deviceId);
        }
    }
    return deviceId;
}

bool SessionPool::DeviceDisconnectCountOnly(const string &cid, const uint8_t linkType, bool needErase)
{
    if (linkType != LINK_TYPE_P2P) {
        LOGI("DeviceDisconnectCountOnly return, linkType is %{public}d, not LINK_TYPE_P2P,", linkType);
        return false;
    }
    std::string deviceId = GetDeviceIdByCid(cid);
    if (deviceId.empty()) {
        LOGE("fail to get deviceId");
        return false;
    }
    std::string key = deviceId + "_" + std::to_string(linkType);
    
    auto itCount = deviceConnectCount_.find(key);
    if (itCount == deviceConnectCount_.end()) {
        LOGI("deviceConnectCount_ can not find %{public}s", Utils::GetAnonyString(key).c_str());
        deviceIdByCid_.erase(cid);
        return false;
    }
    if (needErase) {
        deviceConnectCount_.erase(itCount);
        LOGI("[DeviceDisconnectCountOnly]  %{public}s, needErase", Utils::GetAnonyString(key).c_str());
        deviceIdByCid_.erase(cid);
        return false;
    }
    if (itCount->second > MOUNT_DFS_COUNT_ONE) {
        LOGI("[DeviceDisconnectCountOnly] deviceId_linkType %{public}s has already established \
            more than one link, count %{public}d, decrease count by one now",
            Utils::GetAnonyString(key).c_str(), itCount->second);
        deviceConnectCount_[key]--;
        return true;
    } else {
        LOGI("[DeviceDisconnectCountOnly] deviceId_linkType %{public}s erase now", Utils::GetAnonyString(key).c_str());
        deviceConnectCount_.erase(itCount);
    }
    deviceIdByCid_.erase(cid);
    return false;
}

bool SessionPool::DeviceConnectCountOnly(std::shared_ptr<BaseSession> session)
{
    std::vector<DistributedHardware::DmDeviceInfo> deviceList;
    DistributedHardware::DeviceManager::GetInstance().GetTrustedDeviceList(IDaemon::SERVICE_NAME, "", deviceList);
    if (deviceList.size() == 0 || deviceList.size() > MAX_ONLINE_DEVICE_SIZE) {
        LOGE("the size of trust device list is invalid, size=%zu", deviceList.size());
        return false;
    }
    auto cid = session->GetCid();
    std::string deviceId = "";
    for (const auto &deviceInfo : deviceList) {
        if (!cid.empty() && std::string(deviceInfo.networkId) == cid) {
            deviceId = std::string(deviceInfo.deviceId);
        }
    }
    if (deviceId.empty()) {
        LOGE("fail to get deviceId");
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
        deviceIdByCid_.insert({cid, deviceId});
        key = deviceId + "_" + std::to_string(linkType);
    } else {
        LOGE("occupySession find sessionId failed");
        return false;
    }

    auto itCount = deviceConnectCount_.find(key);
    if (itCount != deviceConnectCount_.end() && itCount->second > 0) {
        LOGI("[DeviceConnectCountOnly] deviceId_linkType %{public}s has already established a link, \
            count %{public}d, increase count by one now", Utils::GetAnonyString(key).c_str(), itCount->second);
        deviceConnectCount_[key]++;
        return true;
    } else {
        LOGI("[DeviceConnectCountOnly] deviceId_linkType %{public}s increase count by one now",
            Utils::GetAnonyString(key).c_str());
        deviceConnectCount_[key]++;
    }
    return false;
}

} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
