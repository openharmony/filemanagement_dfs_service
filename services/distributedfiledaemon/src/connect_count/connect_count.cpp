/*
* Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "connect_count/connect_count.h"

#include "dfs_error.h"
#include "utils_directory.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
static const int32_t ON_STATUS_OFFLINE = 13900046;
IMPLEMENT_SINGLE_INSTANCE(ConnectCount);

void ConnectCount::AddConnect(uint32_t callingTokenId, const std::string &networkId, sptr<IFileDfsListener> &listener)
{
    std::lock_guard lock(connectMutex_);
    for (auto &connect : connectList_) {
        if (connect->callingTokenId == callingTokenId && connect->networkId == networkId) {
            connect->num++;
            return;
        }
    }
    auto connect = std::make_shared<Connect>(callingTokenId, networkId, 1, listener);
    connectList_.insert(connect);
}

bool ConnectCount::CheckCount(const std::string &networkId)
{
    std::lock_guard lock(connectMutex_);
    for (const auto &connect : connectList_) {
        if (connect->networkId == networkId) {
            return true;
        }
    }
    return false;
}

void ConnectCount::RemoveAllConnect()
{
    std::lock_guard lock(connectMutex_);
    for (const auto &connect : connectList_) {
        if (connect->listener != nullptr) {
            connect->listener->OnStatus(connect->networkId, ON_STATUS_OFFLINE, "", 0);
        }
    }
    connectList_.clear();
}

std::vector<std::string> ConnectCount::RemoveConnect(uint32_t callingTokenId)
{
    std::lock_guard lock(connectMutex_);
    auto networkIds = GetNetworkIds(callingTokenId);
    for (auto iter = connectList_.begin(); iter != connectList_.end();) {
        if ((*iter)->callingTokenId == callingTokenId) {
            iter = connectList_.erase(iter);
            continue;
        }
        iter++;
    }
    return networkIds;
}

void ConnectCount::RemoveConnect(const std::string &networkId)
{
    std::lock_guard lock(connectMutex_);
    for (auto iter = connectList_.begin(); iter != connectList_.end();) {
        if ((*iter)->networkId == networkId) {
            iter = connectList_.erase(iter);
            continue;
        }
        iter++;
    }
}

void ConnectCount::RemoveConnect(uint32_t callingTokenId, const std::string &networkId)
{
    std::lock_guard lock(connectMutex_);
    for (auto iter = connectList_.begin(); iter != connectList_.end();) {
        if ((*iter)->callingTokenId == callingTokenId && (*iter)->networkId == networkId) {
            iter = connectList_.erase(iter);
            continue;
        }
        iter++;
    }
}

std::vector<std::string> ConnectCount::GetNetworkIds(uint32_t callingTokenId)
{
    std::lock_guard lock(connectMutex_);
    std::vector<std::string> networkIds;
    for (const auto &connect : connectList_) {
        if (connect->callingTokenId == callingTokenId) {
            networkIds.push_back(connect->networkId);
        }
    }
    return networkIds;
}

void ConnectCount::NotifyRemoteReverseObj(const std::string &networkId, int32_t status)
{
    std::lock_guard lock(connectMutex_);
    for (const auto &connect : connectList_) {
        if (connect->networkId == networkId && connect->listener != nullptr) {
            connect->listener->OnStatus(networkId, status, "", 0);
            LOGI("NotifyRemoteReverseObj, networkId: %{public}s, callingTokenId: %{public}u",
                 Utils::GetAnonyString(networkId).c_str(), connect->callingTokenId);
        }
    }
}

int32_t ConnectCount::FindCallingTokenIdForListerner(const sptr<IRemoteObject> &listener, uint32_t &callingTokenId)
{
    std::lock_guard lock(connectMutex_);
    for (const auto &connect : connectList_) {
        if (connect->listener != nullptr && (connect->listener)->AsObject() == listener) {
            callingTokenId =  connect->callingTokenId;
            return FileManagement::E_OK;
        }
    }
    return FileManagement::ERR_BAD_VALUE;
}

void ConnectCount::AddFileConnect(const std::string &instanceId, const sptr<IFileDfsListener> &listener)
{
    LOGI("AddFileConnect instanceId: %{public}s", instanceId.c_str());
    std::lock_guard lock(fileConnectMutex_);
    auto iter = fileConnectMap_.find(instanceId);
    if (iter != fileConnectMap_.end()) {
        LOGW("InstanceId: %{public}s has already exists, replace the listener with a new value", instanceId.c_str());
        iter->second = listener;
    } else {
        fileConnectMap_.emplace(instanceId, listener);
    }
}

bool ConnectCount::RmFileConnect(const std::string &instanceId)
{
    LOGI("RmFileConnect instanceId: %{public}s", instanceId.c_str());
    std::lock_guard lock(fileConnectMutex_);
    auto iter = fileConnectMap_.find(instanceId);
    if (iter == fileConnectMap_.end()) {
        LOGE("RmFileConnect failed, instanceId: %{public}s not exists", instanceId.c_str());
        return false;
    }
    fileConnectMap_.erase(iter);
    return true;
}

void ConnectCount::NotifyFileStatusChange(const std::string &networkId,
                                          const int32_t status,
                                          const std::string &path,
                                          StatusType type)
{
    std::lock_guard lock(fileConnectMutex_);
    for (const auto &connect : fileConnectMap_) {
        if (connect.second != nullptr) {
            int32_t tmpType = static_cast<int32_t>(type);
            connect.second->OnStatus(networkId, status, path, tmpType);
            LOGI("StatusChange, networkId: %{public}s, status: %{public}d, path: %{public}s, type: %{public}d",
                 Utils::GetAnonyString(networkId).c_str(), status, Utils::GetAnonyString(path).c_str(), tmpType);
        }
    }
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
