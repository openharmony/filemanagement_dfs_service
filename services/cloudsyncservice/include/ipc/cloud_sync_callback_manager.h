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

#ifndef OHOS_FILEMGMT_CLOUD_SYNC_CALLBACK_MANAGER_H
#define OHOS_FILEMGMT_CLOUD_SYNC_CALLBACK_MANAGER_H

#include "i_cloud_sync_callback.h"
#include "safe_map.h"
#include "svc_death_recipient.h"

namespace OHOS::FileManagement::CloudSync {
class CloudSyncCallbackManager : public RefBase {
public:
    static CloudSyncCallbackManager &GetInstance();
    struct CallbackInfo {
        sptr<ICloudSyncCallback> callbackProxy_;
        sptr<SvcDeathRecipient> deathRecipient_;
        int32_t callerUserId_;
    };

    void AddCallback(const std::string &bundleName, const int32_t userId, const sptr<ICloudSyncCallback> &callback);
    void NotifySyncStateChanged(const SyncType type, const SyncPromptState state);

private:
    void SetDeathRecipient(const std::string &bundleName, CallbackInfo &cbInfo);
    sptr<ICloudSyncCallback> GetCallbackProxy(const std::string &bundleName, const int32_t userId);
    void Notify(const std::string bundleName,
                CallbackInfo &callbackInfo,
                const SyncType type,
                const SyncPromptState state);

    SafeMap<const std::string, CallbackInfo> callbackListMap_;
};
} // namespace OHOS::FileManagement::CloudSync

#endif // OHOS_FILEMGMT_CLOUD_SYNC_CALLBACK_MANAGER_H