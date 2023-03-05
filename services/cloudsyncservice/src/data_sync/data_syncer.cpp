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
#include "data_sync/data_syncer.h"

#include <thread>

#include "dfs_error.h"
#include "ipc/cloud_sync_callback_manager.h"
#include "sync_rule/battery_status.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
using namespace std;
DataSyncer::DataSyncer(const std::string appPackageName, const int32_t userId)
    : appPackageName_(appPackageName), userId_(userId)
{
    syncStateManager_ = std::make_shared<SyncStateManager>();
}

std::string DataSyncer::GetAppPackageName() const
{
    return appPackageName_;
}

int32_t DataSyncer::GetUserId() const
{
    return userId_;
}

void DataSyncer::OnSyncComplete(const int32_t code, SyncType type)
{
    SyncState syncState;
    if (code == E_OK) {
        syncState = SyncState::SYNC_SUCCEED;
    } else {
        syncState = SyncState::SYNC_FAILED;
    }

    auto nextAction = syncStateManager_->UpdateSyncState(syncState);
    if (nextAction == Action::START) {
        StartSync(false, SyncTriggerType::PENDING_TRIGGER);
        return;
    }
    if (nextAction == Action::FORCE_START) {
        StartSync(true, SyncTriggerType::PENDING_TRIGGER);
        return;
    }

    auto state = GetSyncPromptState(code);
    if (code == E_OK) {
        CloudSyncCallbackManager::GetInstance().NotifySyncStateChanged(appPackageName_, userId_, SyncType::ALL, state);
    } else {
        CloudSyncCallbackManager::GetInstance().NotifySyncStateChanged(appPackageName_, userId_, type, state);
    }
}

std::shared_ptr<SyncStateManager> DataSyncer::GetSyncStateManager()
{
    return syncStateManager_;
}

void DataSyncer::SyncStateChangedNotify(const SyncType type, const SyncPromptState state)
{
    CloudSyncCallbackManager::GetInstance().NotifySyncStateChanged(appPackageName_, userId_, SyncType::ALL, state);
}

SyncPromptState DataSyncer::GetSyncPromptState(const int32_t code)
{
    if (code == E_OK) {
        return SyncPromptState::SYNC_STATE_DEFAULT;
    }
    SyncPromptState state(SyncPromptState::SYNC_STATE_DEFAULT);
    auto capacityLevel = BatteryStatus::GetCapacityLevel();
    if (capacityLevel == BatteryStatus::LEVEL_LOW) {
        state = SyncPromptState::SYNC_STATE_PAUSED_FOR_BATTERY;
    } else if (capacityLevel == BatteryStatus::LEVEL_TOO_LOW) {
        state = SyncPromptState::SYNC_STATE_BATTERY_TOO_LOW;
    }

    return state;
}
} // namespace OHOS::FileManagement::CloudSync
