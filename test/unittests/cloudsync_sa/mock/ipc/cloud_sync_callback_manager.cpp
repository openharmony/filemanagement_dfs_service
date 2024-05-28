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

#include "ipc/cloud_sync_callback_manager.h"
#include "cloud_sync_constants.h"
#include "dfs_error.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
using namespace std;
using namespace placeholders;
static const string GALLERY_BUNDLE_NAME = "com.ohos.photos";

CloudSyncCallbackManager &CloudSyncCallbackManager::GetInstance()
{
    static CloudSyncCallbackManager instance;
    return instance;
}

static void Convert2Bundle(const string &bundleName, string &bundle)
{
    string photo = ".photos";
    if (bundleName.size() > photo.size() && (bundleName.substr(bundleName.size() - photo.size()) == photo)) {
        bundle = GALLERY_BUNDLE_NAME;
    }
}

void CloudSyncCallbackManager::RemoveCallback(const std::string &bundleName)
{
    CallbackInfo cbInfo;
    string bundle = bundleName;
    Convert2Bundle(bundleName, bundle);
    if (callbackListMap_.Find(bundle, cbInfo)) {
        auto remoteObject = cbInfo.callbackProxy->AsObject();
        remoteObject->RemoveDeathRecipient(cbInfo.deathRecipient);
        callbackListMap_.Erase(bundle);
    }
}

void CloudSyncCallbackManager::AddCallback(const std::string &bundleName,
                                           const int32_t userId,
                                           const sptr<ICloudSyncCallback> &callback)
{
    CallbackInfo callbackInfo;
    string bundle = bundleName;
    Convert2Bundle(bundleName, bundle);
    if (callbackListMap_.Find(bundle, callbackInfo)) {
        auto remoteObject = callbackInfo.callbackProxy->AsObject();
        remoteObject->RemoveDeathRecipient(callbackInfo.deathRecipient);
    }
    callbackInfo.callbackProxy = callback;
    callbackInfo.callerUserId = userId;
    SetDeathRecipient(bundle, callbackInfo);
    /*Delete and then insert when the key exists, ensuring that the final value is inserted.*/
    callbackListMap_.EnsureInsert(bundle, callbackInfo);
}

void CloudSyncCallbackManager::SetDeathRecipient(const std::string &bundleName, CallbackInfo &cbInfo)
{
    auto remoteObject = cbInfo.callbackProxy->AsObject();
    if (!remoteObject) {
        LOGE("Remote object can't be nullptr");
        return;
    }

    auto deathCb = [this, bundleName{bundleName}](const wptr<IRemoteObject> &obj) {
        callbackListMap_.Erase(bundleName);
        LOGE("client died, map size:%{public}d, bundleName:%{private}s", callbackListMap_.Size(), bundleName.c_str());
    };
    cbInfo.deathRecipient = sptr(new SvcDeathRecipient(deathCb));
    remoteObject->AddDeathRecipient(cbInfo.deathRecipient);
}

void CloudSyncCallbackManager::Notify(const std::string bundleName,
                                      CallbackInfo &callbackInfo,
                                      const CloudSyncState state,
                                      const ErrorType error)
{
    auto callbackProxy = callbackInfo.callbackProxy;
    if (!callbackProxy) {
        LOGE("not found object, bundleName = %{private}s", bundleName.c_str());
        return;
    }
    callbackProxy->OnSyncStateChanged(state, error);
}

void CloudSyncCallbackManager::NotifySyncStateChanged(const std::string &bundleName,
    const CloudSyncState state, const ErrorType error)
{
    auto callback = bind(&CloudSyncCallbackManager::Notify, this, _1, _2, state, error);
    CallbackInfo callbackInfo;
    if (!callbackListMap_.Find(bundleName, callbackInfo)) {
        LOGE("not found %{private}s in callbackList", bundleName.c_str());
        return;
    }
    callback(bundleName, callbackInfo);
}
} // namespace OHOS::FileManagement::CloudSync
