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

#include "data_sync/sync_state_manager.h"

#include <type_traits>

#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
using namespace std;

Action SyncStateManager::UpdateSyncState(SyncState newState)
{
    std::unique_lock<std::shared_mutex> lck(syncMutex_);
    state_ = newState;
    stopSyncFlag_ = false;
    return nextAction_;
}

bool SyncStateManager::CheckAndSetPending(bool forceFlag, SyncTriggerType triggerType)
{
    std::unique_lock<std::shared_mutex> lck(syncMutex_);
    if (state_ != SyncState::SYNCING &&
        state_ != SyncState::CLEANING &&
        state_ != SyncState::DISABLE_CLOUD) {
        state_ = SyncState::SYNCING;
        nextAction_ = Action::STOP;
        isForceSync_ = forceFlag;
        return false;
    }

    if (nextAction_ == Action::CHECK) {
        return true;
    }
    if (forceFlag) {
        nextAction_ = Action::FORCE_START;
    } else if (triggerType == SyncTriggerType::TASK_TRIGGER) {
        nextAction_ = Action::CHECK;
    } else {
        nextAction_ = Action::START;
    }
    return true;
}

void SyncStateManager::SetDisableCloudFlag()
{
    std::unique_lock<std::shared_mutex> lck(syncMutex_);
    state_ = SyncState::DISABLE_CLOUD;
    nextAction_ = Action::STOP;
}

void SyncStateManager::SetCleaningFlag()
{
    std::unique_lock<std::shared_mutex> lck(syncMutex_);
    state_ = SyncState::CLEANING;
    nextAction_ = Action::STOP;
}

bool SyncStateManager::GetStopSyncFlag()
{
    return stopSyncFlag_;
}

void SyncStateManager::SetStopSyncFlag()
{
    std::unique_lock<std::shared_mutex> lck(syncMutex_);
    nextAction_ = Action::STOP;
    stopSyncFlag_ = true;
}

SyncState SyncStateManager::GetSyncState() const
{
    return state_;
}

bool SyncStateManager::GetForceFlag() const
{
    return isForceSync_;
}
} // namespace OHOS::FileManagement::CloudSync