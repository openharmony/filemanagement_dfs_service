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
auto clouddriveBundleName = system::GetParameter(CLOUDDRIVE_KEY, "");
auto photoQueryUri = PHOTO_NET_CONT + "&key=photo_network_connection_status";
auto queryUri = QUERY_URI + clouddriveBundleName + "/cloud_sp?key=" + SWITCH_STATUS_KEY;
DataShare::CreateOptions options;

int32_t NetworkSetManager::QueryCellularConnect(int32_t userId, const std::string &bundleName)
{
    LOGI("QueryCellularConnect bundleName: %{public}s", bundleName.c_str());
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(BUNDLE_NAME_KEY, SWITCH_STATUS_KEY);
    std::shared_ptr<DataShare::DataShareResultSet> resultSet;
    std::vector<std::string> columns;
    DataShare::CreateOptions options;
    options.enabled_ = true;
    auto clouddriveBundleName = system::GetParameter(CLOUDDRIVE_KEY, "");
    auto queryUri = QUERY_URI + clouddriveBundleName + "/cloud_sp?user=" + std::to_string(userId);
    Uri uri(queryUri);
    auto dataShareHelper = DataShare::DataShareHelper::Creator(QUERY_URI, options);
    if (dataShareHelper == nullptr) {
        LOGE("dataShareHelper = nullptr");
        return E_RDB;
    }
    resultSet = dataShareHelper->Query(uri, predicates, columns);
    if (resultSet == nullptr) {
        return E_RDB;
    }
    if (resultSet->GoToFirstRow() != E_OK) {
        return E_RDB;
    }
    int32_t columnIndex = 0;
    if (resultSet->GetColumnIndex(BUNDLE_NAME_VALUE, columnIndex) != E_OK) {
        return E_RDB;
    }
    int64_t status = -1;
    if (resultSet->GetLong(columnIndex, status) != E_OK) {
        return E_RDB;
    }
    std::string queryKey = std::to_string(userId) + "/" + bundleName;
    if (status == 1) {
        cellularNetMap_.EnsureInsert(queryKey, true);
        LOGI("QueryCellularConnect on");
    } else {
        cellularNetMap_.EnsureInsert(queryKey, false);
        LOGI("QueryCellularConnect off");
    }
    return E_OK;
}

int32_t NetworkSetManager::QueryNetConnect(int32_t userId, const std::string &bundleName)
{
    LOGI("QueryNetConnect bundleName: %{public}s", bundleName.c_str());
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo("KEYWORD", "photo_network_connection_status");
    std::vector<std::string> columns = {"VALUE"};
    std::shared_ptr<DataShare::DataShareResultSet> resultSet;
    DataShare::CreateOptions options;
    options.enabled_ = true;
    auto queryUri = PHOTO_NET_CONT + "&key=photo_network_connection_status";
    auto dataShareHelper = DataShare::DataShareHelper::Creator(QUERY_URI, options);
    Uri uri(queryUri);
    if (dataShareHelper == nullptr) {
        LOGE("dataShareHelper = nullptr");
        return E_RDB;
    }
    resultSet = dataShareHelper->Query(uri, predicates, columns);
    if (resultSet == nullptr) {
        return E_RDB;
    }
    if (resultSet->GoToFirstRow() != E_OK) {
        return E_RDB;
    }
    int32_t columnIndex = 0;
    if (resultSet->GetColumnIndex(BUNDLE_NAME_VALUE, columnIndex) != E_OK) {
        return E_RDB;
    }
    std::string val;
    if (resultSet->GetString(columnIndex, val) != E_OK) {
        return E_RDB;
    }
    std::string queryKey = std::to_string(userId) + "/" + bundleName;
    if (val == "on") {
        netMap_.EnsureInsert(queryKey, true);
        LOGI("QueryNetConnect on");
    } else {
        netMap_.EnsureInsert(queryKey, false);
        LOGI("QueryNetConnect off");
    }
    return E_OK;
}

void NetworkSetManager::GetCellularConnect(const std::string &bundleName, const int32_t userId)
{
    if (QueryCellularConnect(userId, bundleName) != E_OK) {
        cellularNetMap_.EnsureInsert(std::to_string(userId) + "/" +
                                     bundleName, GetConfigParams(bundleName, userId));
    }
}


void NetworkSetManager::GetNetConnect(const std::string &bundleName, const int32_t userId)
{
    if (QueryNetConnect(userId, bundleName) != E_OK) {
        netMap_.EnsureInsert(std::to_string(userId) + "/" + bundleName, true);
    }
}

bool NetworkSetManager::GetConfigParams(const std::string &bundleName, int32_t userId)
{
    auto driveKit = CloudFile::CloudFileKit::GetInstance();
    std::map<std::string, std::string> param;
    auto err = driveKit->GetAppConfigParams(userId, bundleName, param);
    if (err != E_OK || param.empty()) {
        LOGE("GetAppConfigParams failed");
        return false;
    }
    int32_t networkData = std::stoi(param["useMobileNetworkData"]);
    if (networkData == 0) {
        return false;
    }
    return true;
}

void NetworkSetManager::UnregisterObserver(const std::string &bundleName, const int32_t userId, const int32_t type)
{
    LOGI("UnregisterObserver start");
    options.enabled_ = true;
    auto dataShareHelper = DataShare::DataShareHelper::Creator(QUERY_URI, options);
    if (dataShareHelper == nullptr) {
        LOGE("dataShareHelper = nullptr");
        return;
    }
    sptr<MobileNetworkObserver> dataObserver(new (std::nothrow) MobileNetworkObserver(bundleName,
    userId, type));
    if (type == CELLULARCONNECT) {
        Uri observerUri(queryUri);
        dataShareHelper->UnregisterObserver(observerUri, dataObserver);
    } else {
        Uri observerUri(photoQueryUri);
        dataShareHelper->UnregisterObserver(observerUri, dataObserver);
    }
    LOGI("UnregisterObserver type:%{public}d, finish", type);
}

void NetworkSetManager::RegisterObserver(const std::string &bundleName, const int32_t userId, const int32_t type)
{
    options.enabled_ = true;
    auto dataShareHelper = DataShare::DataShareHelper::Creator(QUERY_URI, options);
    if (dataShareHelper == nullptr) {
        LOGE("dataShareHelper = nullptr");
        return;
    }
    sptr<MobileNetworkObserver> dataObserver(new (std::nothrow) MobileNetworkObserver(bundleName, userId, type));
    if (type == CELLULARCONNECT) {
        Uri observerUri(queryUri);
        dataShareHelper->RegisterObserver(observerUri, dataObserver);
    } else {
        Uri observerUri(photoQueryUri);
        dataShareHelper->RegisterObserver(observerUri, dataObserver);
    }
    LOGI("RegisterObserver type:%{public}d, finish", type);
}

void MobileNetworkObserver::OnChange()
{
    LOGI("MobileNetworkObserver OnChange network status");
    if (observerCallback_ != nullptr) {
        observerCallback_();
    }
    if (type_ == NetworkSetManager::CELLULARCONNECT) {
        NetworkSetManager::GetCellularConnect(bundleName_, userId_);
    } else {
        NetworkSetManager::GetNetConnect(bundleName_, userId_);
    }
}

bool NetworkSetManager::IsAllowCellularConnect(const std::string &bundleName, const int32_t userId)
{
    LOGI("IsAllowCellularConnect bundleName: %{public}s", bundleName.c_str());
    bool checkSwitch = false;
    bool getCellularConnect = cellularNetMap_.Find(std::to_string(userId) + "/" + bundleName, checkSwitch);
    if (getCellularConnect == false) {
        GetCellularConnect(bundleName, userId);
        return cellularNetMap_.ReadVal(std::to_string(userId) + "/" + bundleName);
    }
    return checkSwitch;
}

bool NetworkSetManager::IsAllowNetConnect(const std::string &bundleName, const int32_t userId)
{
    LOGI("IsAllowNetConnect bundleName: %{public}s", bundleName.c_str());
    bool checkSwitch = true;
    bool getNetConnect = netMap_.Find(std::to_string(userId) + "/" + bundleName, checkSwitch);
    if (getNetConnect == false) {
        GetNetConnect(bundleName, userId);
        return netMap_.ReadVal(std::to_string(userId) + "/" + bundleName);
    }
    return checkSwitch;
}

void NetworkSetManager::InitNetworkSetManager(const std::string &bundleName, const int32_t userId)
{
    LOGI("InitNetworkSetManager bundleName: %{public}s", bundleName.c_str());
    if (bundleName != PHOTOS_BUNDLE_NAME) {
        LOGE("InitNetworkSetManager bundleName is illegals");
        return;
    }
    RegisterObserver(bundleName, userId, CELLULARCONNECT);
    RegisterObserver(bundleName, userId, NETCONNECT);
    GetCellularConnect(bundleName, userId);
    GetNetConnect(bundleName, userId);
}
}