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

#ifndef OHOS_FILEMGMT_SYNC_STATE_MANAGER_H
#define OHOS_FILEMGMT_SYNC_STATE_MANAGER_H

#include <atomic>
#include <shared_mutex>

namespace OHOS::FileManagement::CloudSync {
enum class SyncState : int32_t {
    NOT_SYNC = 0,
    SYNCING,
    SYNC_FAILED,
    SYNC_SUCCEED,
};

class SyncStateManager {
public:
    void GetSyncFlagAndUpdateSyncState(bool &stopFlag, bool &pendingFlag, bool &forceFlag, SyncState newState);
    bool IsPendingSync();
    bool GetStopSyncFlag();
    bool SetStopSyncFlag();
    void SetForceSyncFlag(bool flag);

private:
    mutable std::shared_mutex syncMutex_;
    SyncState state_{SyncState::NOT_SYNC};
    std::atomic_bool isPendingSyncFlag_{false};
    std::atomic_bool stopSyncFlag_{false};
    std::atomic_bool forceFlag_{false};
};

} // namespace OHOS::FileManagement::CloudSync

#endif // OHOS_FILEMGMT_SYNC_STATE_MANAGER_H