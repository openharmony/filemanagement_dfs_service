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
#include "utils_log.h"
#include <type_traits>

namespace OHOS::FileManagement::CloudSync {
using namespace std;

void SyncStateManager::GetSyncFlagAndUpdateSyncState(bool &stopFlag,
                                                   bool &pendingFlag,
                                                   bool &forceFlag,
                                                   SyncState newState)
{
    std::unique_lock<std::shared_mutex> lck(syncMutex_);

    stopFlag = stopSyncFlag_;

    if (stopSyncFlag_) {
        pendingFlag = false;
        forceFlag = false;
    } else {
        pendingFlag = isPendingSyncFlag_;
        forceFlag = forceFlag_;
    }

    state_ = newState;
    isPendingSyncFlag_ = false;
    forceFlag_ = false;
    stopSyncFlag_ = false;
}

bool SyncStateManager::IsPendingSync()
{
    std::unique_lock<std::shared_mutex> lck(syncMutex_);
    if (state_ != SyncState::SYNCING) {
        state_ = SyncState::SYNCING;
        return false;
    }

    isPendingSyncFlag_ = true;
    return true;
}

bool SyncStateManager::GetStopSyncFlag()
{
    return stopSyncFlag_;
}

bool SyncStateManager::SetStopSyncFlag()
{
    std::unique_lock<std::shared_mutex> lck(syncMutex_);
    if (state_ == SyncState::SYNCING) {
        stopSyncFlag_ = true;
        return true;
    }
    return false;
}

void SyncStateManager::SetForceSyncFlag(bool flag)
{
    if (isPendingSyncFlag_ && flag) {
        forceFlag_ = true;
    }
}
} // namespace OHOS::FileManagement::CloudSync