/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "data_sync/data_sync_manager.h"

#include <vector>

#include "data_sync/gallery/gallery_data_syncer.h"
#include "dfs_error.h"
#include "ipc/cloud_sync_callback_manager.h"
#include "sync_rule/battery_status.h"
#include "sync_rule/cloud_status.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {

int32_t DataSyncManager::Init(const int32_t userId)
{
    return E_OK;
}

std::shared_ptr<DataSyncer> DataSyncManager::GetDataSyncer(const std::string appPackageName, const int32_t userId)
{
    std::lock_guard<std::mutex> lck(dataSyncMutex_);
    for (auto dataSyncer : dataSyncers_) {
        if ((dataSyncer->GetAppPackageName() == appPackageName) && (dataSyncer->GetUserId() == userId)) {
            return dataSyncer;
        }
    }

    std::shared_ptr<DataSyncer> dataSyncer_ = std::make_shared<GalleryDataSyncer>(appPackageName, userId);
    dataSyncers_.push_back(dataSyncer_);
    return dataSyncer_;
}

int32_t DataSyncManager::IsSkipSync(const std::string appPackageName, const int32_t userId) const
{
    if (!CloudStatus::IsCloudStatusOkay(appPackageName)) {
        LOGE("cloud status is not OK");
        return E_SYNC_FAILED_CLOUD_NOT_READY;
    }
    if (!BatteryStatus::IsBatteryCapcityOkay()) {
        return E_SYNC_FAILED_BATTERY_TOO_LOW;
    }
    return E_OK;
}
} // namespace OHOS::FileManagement::CloudSync