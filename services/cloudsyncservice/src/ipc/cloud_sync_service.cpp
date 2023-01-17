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

int32_t CloudSyncService::RegisterCallbackInner(const string &appPackageName, const sptr<IRemoteObject> &callback)
{
    (void)appPackageName;
    (void)callback;
    return 0;
}
} // namespace OHOS::FileManagement::CloudSync