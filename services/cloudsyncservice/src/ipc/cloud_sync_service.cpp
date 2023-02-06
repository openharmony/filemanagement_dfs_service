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

// #include "iremote_object.h"
#include "dfs_error.h"
#include "system_ability_definition.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
using namespace std;
using namespace OHOS;

REGISTER_SYSTEM_ABILITY_BY_ID(CloudSyncService, FILEMANAGEMENT_CLOUD_SYNC_SERVICE_SA_ID, false);

void CloudSyncService::OnStart()
{
    (void)SystemAbility::Publish(this); // TODO
    LOGI("Start service successfully");
}

void CloudSyncService::OnStop()
{
    LOGI("Stop finished successfully");
}

int32_t CloudSyncService::RegisterCallbackInner(const string &appPackageName, const sptr<IRemoteObject> &remoteObject)
{
    if (remoteObject == nullptr) {
        LOGE("callback is nullptr");
        return E_INVAL_ARG;
    }

    if (appPackageName.empty()) {
        LOGE("appPackageName is invalid");
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

int32_t CloudSyncService::StartSyncInner(const std::string &appPackageName, int type, bool forceFlag)
{
    return E_OK;
}

int32_t CloudSyncService::StopSyncInner(const std::string &appPackageName)
{
    return E_OK;
}
} // namespace OHOS::FileManagement::CloudSync