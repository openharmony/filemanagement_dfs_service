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
DataSyncer::DataSyncer(std::string appPackageName) : appPackageName_(appPackageName)
{
}

std::shared_ptr<SyncStateManager> DataSyncer::GetSyncStateManager(const int32_t userId)
{
    auto it = SyncStateManagers_.find(userId);
    if (it != SyncStateManagers_.end()) {
        return it->second;
    }
    std::shared_ptr<SyncStateManager> syncStateManager_ = std::make_shared<SyncStateManager>();
    SyncStateManagers_[userId] = syncStateManager_;
    return syncStateManager_;
}

void DataSyncer::OnSyncComplete(const int32_t code, int32_t userId, SyncType type)
{
    SyncState syncState;
    if (code == E_OK) {
        syncState = SyncState::SYNC_SUCCEED;
    } else {
        syncState = SyncState::SYNC_FAILED;
    }
    bool stopFlag;
    bool isExistPendingSyncFlag;
    bool forceSyncFlag;
    auto syncStateManager = GetSyncStateManager(userId);
    syncStateManager->GetSyncFlagAndUpdateSyncState(stopFlag, isExistPendingSyncFlag, forceSyncFlag, syncState);
    if (stopFlag) {
        CloudSyncCallbackManager::GetInstance().NotifySyncStateChanged(appPackageName_, userId, SyncType::ALL,
                                                                       SyncPromptState::SYNC_STATE_DEFAULT);
        return;
    }

    if (isExistPendingSyncFlag) {
        StartSync(userId, forceSyncFlag, SyncTriggerType::PENDING_TRIGGER);
        return;
    }

    auto state = GetSyncPromptState(code);
    if (code == E_OK) {
        CloudSyncCallbackManager::GetInstance().NotifySyncStateChanged(appPackageName_, userId, SyncType::ALL, state);
    } else {
        CloudSyncCallbackManager::GetInstance().NotifySyncStateChanged(appPackageName_, userId, type, state);
    }
}

void DataSyncer::SyncStateChangedNotify(const int32_t userId, const SyncType type, const SyncPromptState state)
{
    CloudSyncCallbackManager::GetInstance().NotifySyncStateChanged(appPackageName_, userId, SyncType::ALL, state);
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



