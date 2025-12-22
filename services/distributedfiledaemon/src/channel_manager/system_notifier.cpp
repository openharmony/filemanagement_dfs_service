/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include "system_notifier.h"
#include "channel_manager.h"
#include "device_manager.h"
#include "dfs_error.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
using namespace std;
using namespace OHOS::FileManagement;

namespace {
constexpr const char *SERVICE_NAME = "ohos.storage.distributedfile.daemon";
static std::mutex g_notificationMutex;
static int32_t g_notificationId = 0;
} // namespace

SystemNotifier &SystemNotifier::GetInstance()
{
    static SystemNotifier instance;
    return instance;
}

int32_t SystemNotifier::CreateNotification(const std::string &networkId)
{
    LOGI("CreateNotification enter, networkId is %{public}.6s", networkId.c_str());
    {
        std::shared_lock<std::shared_mutex> readLock(mutex);
        if (notificationMap_.find(networkId) != notificationMap_.end()) {
            LOGI("Notification %{public}.6s already exists.", networkId.c_str());
            return E_OK;
        }
    }

    // Sending system notifications later
    std::unique_lock<std::shared_mutex> write_lock(mutex_);
    notificationMap_[networkId] = GenerateNextNotificationId();
    return E_OK;
}

int32_t SystemNotifier::DestroyNotifyByNetworkId(const std::string &networkId, bool needNotifyRemote)
{
    LOGI("DestroyNotifyByNetworkId for networkId: %{public}.6s", networkId.c_str());
    {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        auto it = notificationMap_.find(networkId);
        if (it != notificationMap_.end()) {
            LOGI("Removed notification mapping for networkId: %{public}.6s", networkId.c_str());
            notificationMap_.erase(it);
        } else {
            LOGW("No notification mapping found for networkId: %{public}.6s", networkId.c_str());
            return ERR_BAD_VALUE;
        }
    }

    return needNotifyRemote ? DisconnectByNetworkId(networkId) : E_OK;
}

int32_t SystemNotifier::DisconnectByNetworkId(const std::string &networkId)
{
    LOGI("DisconnectByNetworkId enter, networkId is %{public}.6s", networkId.c_str());
    ControlCmd request;
    request.msgType = CMD_ACTIVE_DISCONNECT;
    std::string srcNetworkId;
    int32_t ret = DistributedHardware::DeviceManager::GetInstance().GetLocalDeviceNetWorkId(SERVICE_NAME, srcNetworkId);
    if (ret != E_OK || srcNetworkId.empty()) {
        LOGE("Failed to get local device network ID.");
        return ERR_BAD_VALUE;
    }
    request.networkId = srcNetworkId;

    ret = ChannelManager::GetInstance().NotifyClient(networkId, request);
    LOGI("DisconnectByNetworkId end. networkId = %{public}.6s ,ret = %{public}d", networkId.c_str(), ret);
    return ret;
}

int32_t SystemNotifier::GenerateNextNotificationId()
{
    std::lock_guard<std::mutex> lock(g_notificationMutex);
    if (g_notificationId == INT32_MAX) {
        g_notificationId = 0;
    }
    return ++g_notificationId;
}

std::string SystemNotifier::GetRemoteDeviceName(const std::string &networkId)
{
    std::string deviceName;
    int32_t ret = DistributedHardware::DeviceManager::GetInstance().GetDeviceName(SERVICE_NAME, networkId, deviceName);
    if (ret != E_OK) {
        LOGE("GetDeviceName failed. ret = %{public}d", ret);
    }
    LOGI("networkId is %{public}.6s, deviceName is %{public}s", networkId.c_str(), deviceName.c_str());
    return deviceName;
}

void SystemNotifier::ClearAllConnect()
{
    LOGI("ClearALLConnect start.");
    std::shared_lock<std::shared_mutex> readLock(mutex);
    std::vector<std::string> needClearItems;
    for (const auto &pair : notificationMap_) {
        needClearItems.push_back(pair.first);
    }
    for (const auto &networkId : needClearItems) {
        // donot disconncetbyremote
        DestroyNotifyByNetworkId(networkId, false);
    }
}

int32_t SystemNotifier::GetNotificationMapSize()
{
    std::shared_lock<std::shared_mutex> readLock(mutex);
    LOGI("GetNotificationMapSize size is %{public}zu", notificationMap_.size());
    return notificationMap_.size();
}

bool SystemNotifier::IsNetworkIdConnected(const std::string &networkId)
{
    LOGI("IsNetworkIdConnected enter, networkId is %{public}.6s", networkId.c_str());
    std::shared_lock<std::shared_mutex> readLock(mutex);
    return notificationMap_.find(networkId) != notificationMap_.end();
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS