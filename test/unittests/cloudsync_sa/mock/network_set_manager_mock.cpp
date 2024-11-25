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
#include "network_set_manager.h"

#include "accesstoken_kit.h"
#include "cloud_file_kit.h"
#include "datashare_errno.h"
#include "datashare_result_set.h"
#include "dfs_error.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "parameters.h"
#include "system_ability_definition.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
const std::string QUERY_URI = "datashareproxy://";
const std::string PHOTO_NET_CONT = "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true";
const std::string SWITCH_STATUS_KEY = "useMobileNetworkData";
const std::string CLOUDDRIVE_KEY = "persist.kernel.bundle_name.clouddrive";
const std::string PHOTOS_BUNDLE_NAME = "com.ohos.photos";
const std::string BUNDLE_NAME_KEY = "key";
const std::string BUNDLE_NAME_VALUE = "value";
const std::int32_t MOCK_USER_ID = 1;
auto clouddriveBundleName = system::GetParameter(CLOUDDRIVE_KEY, "");
auto photoQueryUri = PHOTO_NET_CONT + "&key=photo_network_connection_status";
auto queryUri = QUERY_URI + clouddriveBundleName + "/cloud_sp?key=" + SWITCH_STATUS_KEY;
DataShare::CreateOptions options;

int32_t NetworkSetManager::QueryCellularConnect(int32_t userId, const std::string &bundleName)
{
    return E_OK;
}

int32_t NetworkSetManager::QueryNetConnect(int32_t userId, const std::string &bundleName)
{
    return E_OK;
}

void NetworkSetManager::GetCellularConnect(const std::string &bundleName, const int32_t userId)
{
}

void NetworkSetManager::GetNetConnect(const std::string &bundleName, const int32_t userId)
{
}

bool NetworkSetManager::GetConfigParams(const std::string &bundleName, int32_t userId)
{
    return true;
}

void NetworkSetManager::UnregisterObserver(const std::string &bundleName, const int32_t userId, const int32_t type)
{
}

void NetworkSetManager::RegisterObserver(const std::string &bundleName, const int32_t userId, const int32_t type)
{
}

void MobileNetworkObserver::OnChange()
{
}

bool NetworkSetManager::IsAllowCellularConnect(const std::string &bundleName, const int32_t userId)
{
    if (userId == MOCK_USER_ID) {
        return true;
    }
    return false;
}

bool NetworkSetManager::IsAllowNetConnect(const std::string &bundleName, const int32_t userId)
{
    if (userId == MOCK_USER_ID) {
        return true;
    }
    return false;
}

void NetworkSetManager::InitNetworkSetManager(const std::string &bundleName, const int32_t userId)
{
}

void NetworkSetManager::InitDataSyncManager(std::shared_ptr<CloudFile::DataSyncManager> dataSyncManager)
{
}

void NetworkSetManager::NetWorkChangeStopUploadTask()
{
}
}