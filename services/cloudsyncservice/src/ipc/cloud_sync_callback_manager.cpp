/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "ipc/cloud_sync_callback_manager.h"
#include "utils_log.h"
#include "dfs_error.h"

namespace OHOS::FileManagement::CloudSync {
using namespace std;

CloudSyncCallbackManager &CloudSyncCallbackManager::GetInstance()
{
    static CloudSyncCallbackManager instance;
    return instance;
}

void CloudSyncCallbackManager::AddCallback(const std::string &appPackageName,
                                           const int32_t userId,
                                           const sptr<ICloudSyncCallback> &callback)
{
    CallbackInfo callbackInfo;
    callbackInfo.callbackProxy_ = callback;
    callbackInfo.callerUserId_ = userId;
    SetDeathRecipient(appPackageName, callbackInfo);
    /*Delete and then insert when the key exists, ensuring that the final value is inserted.*/
    callbackListMap_.EnsureInsert(appPackageName, callbackInfo);
}

void CloudSyncCallbackManager::SetDeathRecipient(const std::string &appPackageName, CallbackInfo &cbInfo)
{
    auto remoteObject = cbInfo.callbackProxy_->AsObject();
    if (!remoteObject) {
        LOGE("Remote object can't be nullptr");
        return;
    }

    auto deathCb = [this, packageName{appPackageName}](const wptr<IRemoteObject> &obj) {
        callbackListMap_.Erase(packageName);
        LOGE("client died, Died remote obj:%{private}p, map size:%{public}d, packageName:%{private}s", obj.GetRefPtr(),
             callbackListMap_.Size(), packageName.c_str());
    };
    cbInfo.deathRecipient_ = sptr(new SvcDeathRecipient(deathCb));
    remoteObject->AddDeathRecipient(cbInfo.deathRecipient_);
}

sptr<ICloudSyncCallback> CloudSyncCallbackManager::GetCallbackProxy(const std::string &appPackageName,
                                                                    const int32_t userId)
{
    CallbackInfo cbInfo;
    if (!callbackListMap_.Find(appPackageName, cbInfo)) {
        LOGE("not found callback, appPackageName: %{private}s", appPackageName.c_str());
        return nullptr;
    }

    if (userId != cbInfo.callerUserId_) {
        LOGE("oldUserId %{public}d, currentUserId %{public}d", cbInfo.callerUserId_, userId);
        return nullptr;
    }
    return cbInfo.callbackProxy_;
}

void CloudSyncCallbackManager::NotifySyncStateChanged(const std::string &appPackageName,
                                                      const int32_t userId,
                                                      const SyncType type,
                                                      const SyncPromptState state)
{
    auto callbackProxy_ = GetCallbackProxy(appPackageName, userId);
    if (!callbackProxy_) {
        LOGE("not found object, appPackageName = %{private}s", appPackageName.c_str());
        return;
    }
    callbackProxy_->OnSyncStateChanged(type, state);
}
} // namespace OHOS::FileManagement::CloudSync