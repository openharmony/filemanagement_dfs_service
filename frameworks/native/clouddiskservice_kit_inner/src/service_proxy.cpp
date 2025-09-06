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

sptr<ICloudDiskService> ServiceProxy::GetInstance()
{
    LOGD("getinstance");
    unique_lock<mutex> lock(instanceMutex_);
    if (serviceProxy_ != nullptr) {
        if (serviceProxy_->AsObject() != nullptr && !serviceProxy_->AsObject()->IsObjectDead()) {
            return serviceProxy_;
        }
    }

    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        LOGE("Samgr is nullptr");
        return nullptr;
    }

    auto object = samgr->CheckSystemAbility(FILEMANAGEMENT_CLOUD_DISK_SERVICE_SA_ID);
    if (object == nullptr) {
        LOGE("CloudDiskService::Connect object == nullptr");
        return nullptr;
    }

    serviceProxy_ = iface_cast<ICloudDiskService>(object);
    if (serviceProxy_ == nullptr) {
        LOGE("CloudDiskService::Connect service == nullptr");
        return nullptr;
    }
    return serviceProxy_;
}

void ServiceProxy::InvalidInstance()
{
    LOGI("invalid instance");
    unique_lock<mutex> lock(instanceMutex_);
    serviceProxy_ = nullptr;
}
} // namespace OHOS::FileManagement::CloudDiskService
