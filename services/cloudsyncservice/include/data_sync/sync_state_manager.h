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

#include "data_sync_const.h"
namespace OHOS::FileManagement::CloudSync {
enum class SyncState : int32_t {
    INIT = 0,
    SYNCING,
    CLEANING,
    SYNC_FAILED,
    SYNC_SUCCEED,
    CLEAN_SUCCEED,
    DISABLE_CLOUD,
    DISABLE_CLOUD_SUCCEED,
};

enum class Action : int32_t {
    STOP = 0,
    START,
    FORCE_START,
    CHECK,
};

class SyncStateManager {
public:
    Action UpdateSyncState(SyncState newState);
    bool CheckAndSetPending(bool forceFlag, SyncTriggerType triggerType);
    void SetCleaningFlag();
    bool GetStopSyncFlag();
    void SetStopSyncFlag();
    void SetDisableCloudFlag();
    SyncState GetSyncState() const;
    bool GetForceFlag() const;

private:
    mutable std::shared_mutex syncMutex_;
    SyncState state_{SyncState::INIT};
    Action nextAction_{Action::STOP};
    std::atomic_bool isForceSync_{false};
    std::atomic_bool stopSyncFlag_{false};
};

} // namespace OHOS::FileManagement::CloudSync

#endif // OHOS_FILEMGMT_SYNC_STATE_MANAGER_H
