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

#ifndef OHOS_FILEMGMT_DATA_SYNCER_H
#define OHOS_FILEMGMT_DATA_SYNCER_H

#include <map>

#include "cloud_sync_constants.h"
#include "data_sync/sync_state_manager.h"

namespace OHOS::FileManagement::CloudSync {
enum class SyncTriggerType : int32_t {
    APP_TRIGGER,
    PENDING_TRIGGER,
};
class DataSyncer {
public:
    explicit DataSyncer(std::string appPackageName);
    virtual ~DataSyncer(){};

    virtual void StartSync(int32_t userId, bool forceFlag, SyncTriggerType triggerType) = 0;
    virtual void StopSync(int32_t userId, SyncTriggerType triggerType) = 0;

protected:
    void OnSyncComplete(const int32_t code, const int32_t userId, const SyncType type);
    void SyncStateChangedNotify(const int32_t userId, const SyncType type, const SyncPromptState state);
    std::shared_ptr<SyncStateManager> GetSyncStateManager(const int32_t userId);

private:
    SyncPromptState GetSyncPromptState(const int32_t code);

    std::string appPackageName_;
    std::map<const int32_t, std::shared_ptr<SyncStateManager>> SyncStateManagers_;
};
} // namespace OHOS::FileManagement::CloudSync

#endif // OHOS_FILEMGMT_DATA_SYNCER_H