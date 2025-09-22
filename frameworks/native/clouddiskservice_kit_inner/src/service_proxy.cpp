/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "service_proxy.h"

#include <sstream>

#include "cloud_disk_service_proxy.h"
#include "dfs_error.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudDiskService {
using namespace std;
constexpr int LOAD_SA_TIMEOUT_MS = 4000;

sptr<ICloudDiskService> ServiceProxy::GetInstance()
{
    LOGD("getinstance");
    unique_lock<mutex> lock(instanceMutex_);

    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        LOGE("Samgr is nullptr");
        return nullptr;
    }

    auto object = samgr->CheckSystemAbility(FILEMANAGEMENT_CLOUD_DISK_SERVICE_SA_ID);
    if (object != nullptr) {
        LOGI("SA check successfully");
        serviceProxy_ = iface_cast<ICloudDiskService>(object);
        return serviceProxy_;
    }

    sptr<ServiceProxyLoadCallback> cloudDiskLoadCallback = new ServiceProxyLoadCallback();
    if (cloudDiskLoadCallback == nullptr) {
        LOGE("cloudDiskLoadCallback is nullptr");
        return nullptr;
    }
    int32_t ret = samgr->LoadSystemAbility(FILEMANAGEMENT_CLOUD_DISK_SERVICE_SA_ID, cloudDiskLoadCallback);
    if (ret != E_OK) {
        LOGE("Failed to Load systemAbility, systemAbilityId:%{public}d, ret code:%{public}d",
             FILEMANAGEMENT_CLOUD_DISK_SERVICE_SA_ID, ret);
        return nullptr;
    }
    unique_lock<mutex> proxyLock(proxyMutex_);
    auto waitStatus = cloudDiskLoadCallback->proxyConVar_.wait_for(
        proxyLock, std::chrono::milliseconds(LOAD_SA_TIMEOUT_MS),
        [cloudDiskLoadCallback]() { return cloudDiskLoadCallback->isLoadSuccess_.load(); });
    if (!waitStatus) {
        LOGE("Load CloudSync SA timeout");
        return nullptr;
    }
    return serviceProxy_;
}

void ServiceProxy::ServiceProxyLoadCallback::OnLoadSystemAbilitySuccess(
    int32_t systemAbilityId,
    const sptr<IRemoteObject> &remoteObject)
{
    LOGI("Load CloudDiskService SA success,systemAbilityId:%{public}d, remoteObj result:%{private}s", systemAbilityId,
         (remoteObject == nullptr ? "false" : "true"));
    unique_lock<mutex> lock(proxyMutex_);
    serviceProxy_ = iface_cast<ICloudDiskService>(remoteObject);

    isLoadSuccess_.store(true);
    proxyConVar_.notify_one();
}

void ServiceProxy::ServiceProxyLoadCallback::OnLoadSystemAbilityFail(int32_t systemAbilityId)
{
    LOGI("Load CloudDiskService SA failed,systemAbilityId:%{public}d", systemAbilityId);
    unique_lock<mutex> lock(proxyMutex_);
    serviceProxy_ = nullptr;
    isLoadSuccess_.store(false);
    proxyConVar_.notify_one();
}

void ServiceProxy::InvalidInstance()
{
    LOGI("invalid instance");
    unique_lock<mutex> lock(instanceMutex_);
    serviceProxy_ = nullptr;
}
} // namespace OHOS::FileManagement::CloudDiskService
