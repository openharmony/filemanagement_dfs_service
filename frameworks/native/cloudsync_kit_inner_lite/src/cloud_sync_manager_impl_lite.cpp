/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "cloud_sync_manager_impl_lite.h"
#include "cloud_sync_service_proxy_lite.h"
#include "dfs_error.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
constexpr int32_t MIN_USER_ID = 100;
CloudSyncManagerImplLite &CloudSyncManagerImplLite::GetInstance()
{
}

int32_t CloudSyncManagerImplLite::TriggerSync(const std::string &bundleName, const int32_t &userId)
{
    return 0;
}

void CloudSyncManagerImplLite::SetDeathRecipient(const sptr<IRemoteObject> &remoteObject)
{
}
} // namespace OHOS::FileManagement::CloudSync
