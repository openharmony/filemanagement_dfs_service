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

#include "safe_map.h"

#include "nocopyable.h"

#include "i_cloud_sync_callback.h"
#include "svc_death_recipient.h"

namespace OHOS::FileManagement::CloudSync {
class CloudSyncCallbackManager final : public NoCopyable {
public:
    static CloudSyncCallbackManager &GetInstance();
    struct CallbackInfo {
        sptr<ICloudSyncCallback> callbackProxy;
        sptr<SvcDeathRecipient> deathRecipient;
        int32_t callerUserId;
    };

    void RemoveCallback(const std::string &bundleName);
    void AddCallback(const std::string &bundleName, const int32_t userId, const sptr<ICloudSyncCallback> &callback);
    void NotifySyncStateChanged(const std::string &bundleName, const CloudSyncState state, const ErrorType error);

private:
    CloudSyncCallbackManager() = default;
    void SetDeathRecipient(const std::string &bundleName, CallbackInfo &cbInfo);
    void Notify(const std::string bundleName,
                CallbackInfo &callbackInfo,
                const CloudSyncState state,
                const ErrorType error);

    SafeMap<const std::string, CallbackInfo> callbackListMap_;
};
} // namespace OHOS::FileManagement::CloudSync

#endif // OHOS_FILEMGMT_CLOUD_SYNC_CALLBACK_MANAGER_H