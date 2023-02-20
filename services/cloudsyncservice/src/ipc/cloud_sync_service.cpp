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
#include "ipc/cloud_sync_service.h"

#include "dfs_error.h"
#include "dfsu_access_token_helper.h"
#include "system_ability_definition.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
using namespace std;
using namespace OHOS;

REGISTER_SYSTEM_ABILITY_BY_ID(CloudSyncService, FILEMANAGEMENT_CLOUD_SYNC_SERVICE_SA_ID, false);

void CloudSyncService::PublishSA()
{
    LOGI("Begin to init");
    bool ret = SystemAbility::Publish(this);
    if (!ret) {
        throw runtime_error(" Failed to publish the daemon");
    }
    LOGI("Init finished successfully");
}

void CloudSyncService::OnStart()
{
    LOGI("Begin to start service");
    try {
        PublishSA();
    } catch (const exception &e) {
        LOGE("%{public}s", e.what());
    }
    LOGI("Start service successfully");
}

void CloudSyncService::OnStop()
{
    LOGI("Stop finished successfully");
}

int32_t CloudSyncService::RegisterCallbackInner(const sptr<IRemoteObject> &remoteObject)
{
    if (remoteObject == nullptr) {
        LOGE("callback is nullptr");
        return E_INVAL_ARG;
    }

    string appPackageName;
    if (DfsuAccessTokenHelper::GetCallerPackageName(appPackageName)) {
        return E_INVAL_ARG;
    }

    auto callback = iface_cast<ICloudSyncCallback>(remoteObject);
    if (!remoteObject) {
        LOGE("remoteObject is nullptr");
        return E_INVAL_ARG;
    }

    callbackManager_.AddCallback(appPackageName, callback);
    return E_OK;
}

int32_t CloudSyncService::StartSyncInner(bool forceFlag)
{
    string appPackageName;
    if (DfsuAccessTokenHelper::GetCallerPackageName(appPackageName)) {
        return E_INVAL_ARG;
    }
    auto callbackProxy_ = callbackManager_.GetCallbackProxy(appPackageName);
    if (!callbackProxy_) {
        LOGE("not found object, appPackageName = %{private}s", appPackageName.c_str());
        return E_INVAL_ARG;
    }
    SyncPromptState state = SyncPromptState::SYNC_STATE_DEFAULT;
    callbackProxy_->OnSyncStateChanged(SyncType::ALL, state);
    return E_OK;
}

int32_t CloudSyncService::StopSyncInner()
{
    string appPackageName;
    if (DfsuAccessTokenHelper::GetCallerPackageName(appPackageName)) {
        return E_INVAL_ARG;
    }
    auto callbackProxy_ = callbackManager_.GetCallbackProxy(appPackageName);
    if (!callbackProxy_) {
        LOGE("not found object, appPackageName = %{private}s", appPackageName.c_str());
        return E_INVAL_ARG;
    }
    SyncPromptState state = SyncPromptState::SYNC_STATE_DEFAULT;
    callbackProxy_->OnSyncStateChanged(SyncType::ALL, state);
    return E_OK;
}
} // namespace OHOS::FileManagement::CloudSync