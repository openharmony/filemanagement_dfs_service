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
#include "parameters.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
const std::string QUERY_URI = "datashareproxy://";
const std::string SWITCH_STATUS_KEY = "useMobileNetworkData";
const std::string CLOUDDRIVE_KEY = "persist.kernel.bundle_name.clouddrive";
const std::string BUNDLE_NAME_KEY = "key";
const std::string BUNDLE_NAME_VALUE = "value";
auto clouddriveBundleName = system::GetParameter(CLOUDDRIVE_KEY, "");
auto queryUri = QUERY_URI + clouddriveBundleName + "/cloud_sp?key=" + SWITCH_STATUS_KEY;
std::map<std::string, bool> resultMap;
DataShare::CreateOptions options;

int32_t NetworkSetManager::QueryByDataShare(int32_t userId, const std::string &bundleName)
{
    LOGI("Query NetworkSetManager status");
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
    if (status == 1) {
        resultMap[std::to_string(userId) + "/" + bundleName] = true;
        return E_OK;
    } else {
        resultMap[std::to_string(userId) + "/" + bundleName] = false;
        return E_OK;
    }
}

void NetworkSetManager::GetNetworkSetting(int32_t userId, const std::string &bundleName)
{
    if (QueryByDataShare(userId, bundleName) != E_OK) {
        resultMap[std::to_string(userId) + "/" + bundleName] = GetConfigParams(bundleName, userId);
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

void NetworkSetManager::UnregisterObserver()
{
    options.enabled_ = true;
    auto dataShareHelper = DataShare::DataShareHelper::Creator(QUERY_URI, options);
    Uri observerUri(queryUri);
    if (dataShareHelper == nullptr) {
        LOGE("dataShareHelper = nullptr");
        return;
    }
    sptr<MobileNetworkObserver> dataObserver(new (std::nothrow) MobileNetworkObserver());
    dataShareHelper->UnregisterObserver(observerUri, dataObserver);
    LOGI("UnRegisterObserver finish");
}

void NetworkSetManager::RegisterObserver()
{
    options.enabled_ = true;
    auto dataShareHelper = DataShare::DataShareHelper::Creator(QUERY_URI, options);
    Uri observerUri(queryUri);
    if (dataShareHelper == nullptr) {
        LOGE("dataShareHelper = nullptr");
        return;
    }
    sptr<MobileNetworkObserver> dataObserver(new (std::nothrow) MobileNetworkObserver());
    dataShareHelper->RegisterObserver(observerUri, dataObserver);
    LOGI("RegisterObserver finish");
}

void MobileNetworkObserver::OnChange()
{
    LOGI("MobileNetworkObserver OnChange network status");
    if (observerCallback_ != nullptr) {
        observerCallback_();
    }
    NetworkSetManager::GetNetworkSetting(NetworkSetManager::userId_, NetworkSetManager::bundleName_);
}

bool NetworkSetManager::IsNetworkSetStatusOkay(const std::string &bundleName, const int32_t userId)
{
    LOGI("IsNetworkSetStatusOkay start");
    bool checkstatus = resultMap[std::to_string(userId) + "/" + bundleName];
    if (checkstatus) {
        LOGI("IsNetworkSetStatusOkay checkstatus = true");
        return true;
    }
    return false;
}

void NetworkSetManager::InitNetworkSetManager(const std::string &bundleName, const int32_t userId)
{
    LOGI("InitNetworkSetManager start");
    RegisterObserver();
    bundleName_ = bundleName;
    userId_ = userId;
    GetNetworkSetting(userId, bundleName);
}
}