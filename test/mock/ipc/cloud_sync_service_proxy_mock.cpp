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

#include "cloud_sync_common.h"
#include "dfs_error.h"
#include "iservice_registry.h"
#include "i_cloud_sync_service_mock.h"
#include "system_ability_definition.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
using namespace std;

int32_t CloudSyncServiceProxy::RegisterCallbackInner(const sptr<IRemoteObject> &remoteObject,
                                                     const std::string &bundleName)
{
    return E_OK;
}

int32_t CloudSyncServiceProxy::StartSyncInner(bool forceFlag, const std::string &bundleName)
{
    return E_OK;
}

int32_t CloudSyncServiceProxy::StopSyncInner(const std::string &bundleName)
{
    return E_OK;
}

int32_t CloudSyncServiceProxy::ChangeAppSwitch(const std::string &accoutId, const std::string &bundleName, bool status)
{
    return E_OK;
}

int32_t CloudSyncServiceProxy::Clean(const std::string &accountId, const CleanOptions &cleanOptions)
{
    return E_OK;
}

int32_t CloudSyncServiceProxy::NotifyDataChange(const std::string &accoutId, const std::string &bundleName)
{
    return E_OK;
}

int32_t CloudSyncServiceProxy::EnableCloud(const std::string &accoutId, const SwitchDataObj &switchData)
{
    return E_OK;
}

int32_t CloudSyncServiceProxy::DisableCloud(const std::string &accoutId)
{
    return E_OK;
}

sptr<ICloudSyncService> CloudSyncServiceProxy::GetInstance()
{
    serviceProxy_ = iface_cast<ICloudSyncService>(sptr(new CloudSyncServiceMock()));
    return serviceProxy_;
}

int32_t UploadAsset(const int32_t userId, const std::string &request, std::string &result)
{
    return E_OK;
}

int32_t DownloadFile(const int32_t userId, const std::string &bundleName, AssetInfoObj &assetInfoObj)
{
    return E_OK;
}

int32_t DeleteAsset(const int32_t userId, const std::string &uri)
{
    return E_OK;
}
} // namespace OHOS::FileManagement::CloudSync
