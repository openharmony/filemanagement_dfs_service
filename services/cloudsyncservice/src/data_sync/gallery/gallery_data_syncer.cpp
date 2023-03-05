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
#include "data_sync/gallery/gallery_data_syncer.h"
#include "cloud_sync_constants.h"
#include "dfs_error.h"
#include "sync_rule/battery_status.h"
#include "utils_log.h"


namespace OHOS::FileManagement::CloudSync {
using namespace std;

void GalleryDataSyncer::StartSync(bool forceFlag, SyncTriggerType triggerType)
{
    LOGI("start sync, isforceSync:%{public}d, triggerType:%{public}d", forceFlag, triggerType);
    auto syncStateManager = GetSyncStateManager();
    if (syncStateManager->IsPendingSync(forceFlag)) {
        LOGI("syncing, pending sync");
        return;
    }

    SyncStateChangedNotify(SyncType::ALL, SyncPromptState::SYNC_STATE_SYNCING);
    int32_t ret = UploadFile(forceFlag);
    OnSyncComplete(ret, SyncType::UPLOAD);
}

void GalleryDataSyncer::StopSync(SyncTriggerType triggerType)
{
    LOGI("trigger stop sync, triggerType:%{public}d", triggerType);
    auto syncStateManager = GetSyncStateManager();
    syncStateManager->SetStopSyncFlag();
}

int32_t GalleryDataSyncer::UploadFile(bool forceFlag)
{
    auto syncStateManager = GetSyncStateManager();
    if (syncStateManager->GetStopSyncFlag()) {
        LOGI("trigger stop sync");
        return E_OK;
    }
    if (!BatteryStatus::IsAllowUpload(forceFlag)) {
        return E_SYNC_FAILED_BATTERY_LOW;
    }
    return E_OK;
}
} // namespace OHOS::FileManagement::CloudSync