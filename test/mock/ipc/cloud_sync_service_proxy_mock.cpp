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
#include "cloud_sync_service_proxy.h"

#include <sstream>

#include "dfs_error.h"
#include "iservice_registry.h"
#include "i_cloud_sync_service_mock.h"
#include "system_ability_definition.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
using namespace std;

int32_t CloudSyncServiceProxy::RegisterCallbackInner(const sptr<IRemoteObject> &remoteObject)
{
    return E_OK;
}

int32_t CloudSyncServiceProxy::StartSyncInner(bool forceFlag)
{
    return E_OK;
}

int32_t CloudSyncServiceProxy::StopSyncInner()
{
    return E_OK;
}

int32_t CloudSyncServiceProxy::ChangeAppSwitch(const std::string &accoutId, const std::string &bundleName, bool status)
{
    return E_OK;
}

sptr<ICloudSyncService> CloudSyncServiceProxy::GetInstance()
{
    serviceProxy_ = iface_cast<ICloudSyncService>(sptr(new CloudSyncServiceMock()));
    return serviceProxy_;
}

} // namespace OHOS::FileManagement::CloudSync