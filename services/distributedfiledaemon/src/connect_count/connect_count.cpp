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

#include "connect_count/connect_count.h"

#include "dfs_error.h"
#include "utils_directory.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
std::shared_ptr<ConnectCount> ConnectCount::instance_ = nullptr;
static const int32_t ON_STATUS_OFFLINE = 13900046;
std::shared_ptr<ConnectCount> ConnectCount::GetInstance()
{
    static std::once_flag once;
    std::call_once(once, [&]() {
        instance_ = std::make_shared<ConnectCount>();
    });
    return instance_;
}

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
            connect->listener->OnStatus(connect->networkId, ON_STATUS_OFFLINE);
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
            connect->listener->OnStatus(networkId, status);
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
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
