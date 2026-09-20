/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "cloud_disk_fuse_mount_adapter.h"

#include <cerrno>
#include <unistd.h>

#include "iservice_registry.h"
#include "storage_manager_proxy.h"
#include "system_ability_definition.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudDiskService {
namespace {
sptr<StorageManager::IStorageManager> GetStorageManagerProxy()
{
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        LOGE("Get system ability manager failed");
        return nullptr;
    }
    auto remoteObject = samgr->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    if (remoteObject == nullptr) {
        LOGE("Get storage manager system ability failed");
        return nullptr;
    }
    auto proxy = iface_cast<StorageManager::IStorageManager>(remoteObject);
    if (proxy == nullptr) {
        LOGE("Cast storage manager proxy failed");
    }
    return proxy;
}

} // namespace

int32_t CloudDiskFuseMountAdapter::Mount(int32_t userId, const std::string &mountPoint, int &fuseFd)
{
    fuseFd = -1;
    auto proxy = GetStorageManagerProxy();
    if (proxy == nullptr) {
        return -EIO;
    }
    int32_t ret = proxy->MountCloudDiskFuse(userId, mountPoint, fuseFd);
    if (ret != 0) {
        if (fuseFd >= 0) {
            (void)close(fuseFd);
            fuseFd = -1;
        }
        LOGE("Mount cloud disk FUSE failed, userId: %{public}d, ret: %{public}d", userId, ret);
        return ret;
    }
    if (fuseFd < 0) {
        LOGE("Mount cloud disk FUSE returned an invalid fd, userId: %{public}d", userId);
        return -EBADF;
    }
    return 0;
}

int32_t CloudDiskFuseMountAdapter::Unmount(int32_t userId, const std::string &mountPoint)
{
    auto proxy = GetStorageManagerProxy();
    if (proxy == nullptr) {
        return -EIO;
    }
    int32_t ret = proxy->UMountCloudDiskFuse(userId, mountPoint);
    if (ret != 0) {
        LOGE("Unmount cloud disk FUSE failed, userId: %{public}d, ret: %{public}d", userId, ret);
    }
    return ret;
}

} // namespace OHOS::FileManagement::CloudDiskService
