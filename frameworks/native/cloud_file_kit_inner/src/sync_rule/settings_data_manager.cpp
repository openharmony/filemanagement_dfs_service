/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "settings_data_manager.h"

#include <charconv>

#include "datashare_helper.h"
#include "datashare_errno.h"
#include "datashare_result_set.h"
#include "dfs_error.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
static const std::string SETTING_DATA_QUERY_URI = "datashareproxy://";
static const std::string SETTING_DATA_COMMON_URI =
    "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true";
static const std::string SYNC_SWITCH_KEY = "photos_sync_options";                       // "0", "1", "2"
static const std::string NETWORK_CONNECTION_KEY = "photo_network_connection_status";    // "on", "off"
static const std::string LOCAL_SPACE_FREE_KEY = "photos_local_space_free";              // "1", "0"
static const std::string LOCAL_SPACE_DAYS_KEY = "photos_local_space_free_day_count";    // "30"
static const std::string MOBILE_DATA_SYNC_KEY = "photos_mobile_data_sync";              // "1", "0"
static const std::string AI_FAMILY_STATUS = "2";
static const std::string CLOUD_STATUS = "1";
static const std::string OPEN_STATUS = "on";
static const std::string ALLOW_STATUS = "1";
static const std::string LSF_ALLOW_STATUS = "0";
static const int32_t LOCAL_SPACE_DAYS_DEFAULT = 30;

void SettingsDataManager::InitSettingsDataManager()
{
    LOGI("InitSettingsDataManager");
    RegisterObserver(SYNC_SWITCH_KEY);
    RegisterObserver(NETWORK_CONNECTION_KEY);
    RegisterObserver(MOBILE_DATA_SYNC_KEY);
    RegisterObserver(LOCAL_SPACE_FREE_KEY);
    RegisterObserver(LOCAL_SPACE_DAYS_KEY);
    QuerySwitchStatus();
    QueryNetworkConnectionStatus();
    QueryLocalSpaceFreeStatus();
    QueryLocalSpaceFreeDays();
    QueryMobileDataStatus();
}

int32_t SettingsDataManager::QuerySwitchStatus()
{
    std::string value;
    int32_t ret = QueryParamInSettingsData(SYNC_SWITCH_KEY, value);
    if (ret != E_OK) {
        LOGE("query SYNC_SWITCH_KEY failed");
        settingsDataMap_.Erase(SYNC_SWITCH_KEY);
        return ret;
    }
    settingsDataMap_.EnsureInsert(SYNC_SWITCH_KEY, value);
    return E_OK;
}

int32_t SettingsDataManager::QueryNetworkConnectionStatus()
{
    std::string value;
    int32_t ret = QueryParamInSettingsData(NETWORK_CONNECTION_KEY, value);
    if (ret != E_OK) {
        LOGE("query NETWORK_CONNECTION_KEY failed");
        settingsDataMap_.Erase(NETWORK_CONNECTION_KEY);
        return ret;
    }
    settingsDataMap_.EnsureInsert(NETWORK_CONNECTION_KEY, value);
    return E_OK;
}

int32_t SettingsDataManager::QueryMobileDataStatus()
{
    std::string value;
    int32_t ret = QueryParamInSettingsData(MOBILE_DATA_SYNC_KEY, value);
    if (ret != E_OK) {
        LOGE("query MOBILE_DATA_SYNC_KEY failed");
        settingsDataMap_.Erase(MOBILE_DATA_SYNC_KEY);
        return ret;
    }
    settingsDataMap_.EnsureInsert(MOBILE_DATA_SYNC_KEY, value);
    return E_OK;
}

int32_t SettingsDataManager::QueryLocalSpaceFreeStatus()
{
    std::string value;
    int32_t ret = QueryParamInSettingsData(LOCAL_SPACE_FREE_KEY, value);
    if (ret != E_OK) {
        LOGE("query LOCAL_SPACE_FREE_KEY failed");
        settingsDataMap_.Erase(LOCAL_SPACE_FREE_KEY);
        return ret;
    }
    settingsDataMap_.EnsureInsert(LOCAL_SPACE_FREE_KEY, value);
    return E_OK;
}

int32_t SettingsDataManager::QueryLocalSpaceFreeDays()
{
    std::string value;
    int32_t ret = QueryParamInSettingsData(LOCAL_SPACE_DAYS_KEY, value);
    if (ret != E_OK) {
        LOGE("query LOCAL_SPACE_DAYS_KEY failed");
        settingsDataMap_.Erase(LOCAL_SPACE_DAYS_KEY);
        return ret;
    }
    settingsDataMap_.EnsureInsert(LOCAL_SPACE_DAYS_KEY, value);
    return E_OK;
}

SwitchStatus SettingsDataManager::GetSwitchStatus()
{
    std::string value;
    int32_t ret = QuerySwitchStatus();
    if (ret == E_INNER_RDB) {
        return SwitchStatus::CLOUD_SPACE;
    } else if (ret == E_RDB) {
        return SwitchStatus::NONE;
    }

    value = settingsDataMap_.ReadVal(SYNC_SWITCH_KEY);
    if (value == AI_FAMILY_STATUS) {
        return SwitchStatus::AI_FAMILY;
    } else if (value == CLOUD_STATUS) {
        return SwitchStatus::CLOUD_SPACE;
    } else {
        return SwitchStatus::NONE;
    }
}

SwitchStatus SettingsDataManager::GetSwitchStatusByCache()
{
    std::string value;
    if (!settingsDataMap_.Find(SYNC_SWITCH_KEY, value)) {
        return GetSwitchStatus();
    }

    value = settingsDataMap_.ReadVal(SYNC_SWITCH_KEY);
    if (value == AI_FAMILY_STATUS) {
        return SwitchStatus::AI_FAMILY;
    } else if (value == CLOUD_STATUS) {
        return SwitchStatus::CLOUD_SPACE;
    } else {
        return SwitchStatus::NONE;
    }
}

bool SettingsDataManager::GetNetworkConnectionStatus()
{
    std::string value;
    if (!settingsDataMap_.Find(NETWORK_CONNECTION_KEY, value)) {
        if (QueryNetworkConnectionStatus() != E_OK) {
            return true;
        }
    }

    value = settingsDataMap_.ReadVal(NETWORK_CONNECTION_KEY);
    return value == OPEN_STATUS;
}

bool SettingsDataManager::GetMobileDataStatus()
{
    std::string value;
    if (!settingsDataMap_.Find(MOBILE_DATA_SYNC_KEY, value)) {
        if (QueryMobileDataStatus() != E_OK) {
            return false;
        }
    }

    value = settingsDataMap_.ReadVal(MOBILE_DATA_SYNC_KEY);
    return value == ALLOW_STATUS;
}

int32_t SettingsDataManager::GetLocalSpaceFreeStatus()
{
    std::string value;
    if (!settingsDataMap_.Find(LOCAL_SPACE_FREE_KEY, value)) {
        if (QueryLocalSpaceFreeStatus() != E_OK) {
            return 1;
        }
    }
    value = settingsDataMap_.ReadVal(LOCAL_SPACE_FREE_KEY);
    if (value == LSF_ALLOW_STATUS) {
        return 0;
    } else {
        return 1;
    }
}

static bool ConvertToInt(const std::string &str, int32_t &val)
{
    auto [ptr, ec] = std::from_chars(str.c_str(), str.c_str() + str.size(), val);
    return ec == std::errc() && ptr == str.c_str() + str.size();
}

int32_t SettingsDataManager::GetLocalSpaceFreeDays()
{
    std::string value;
    if (!settingsDataMap_.Find(LOCAL_SPACE_DAYS_KEY, value)) {
        if (QueryLocalSpaceFreeDays() != E_OK) {
            return LOCAL_SPACE_DAYS_DEFAULT;
        }
    }

    value = settingsDataMap_.ReadVal(LOCAL_SPACE_DAYS_KEY);
    int32_t val = 0;
    if (ConvertToInt(value, val)) {
        return val;
    } else {
        LOGE("ConvertToInt failed, value: %{public}s", value.c_str());
        return LOCAL_SPACE_DAYS_DEFAULT;
    }
}

int32_t SettingsDataManager::QueryParamInSettingsData(const std::string &key, std::string &value)
{
    LOGI("Query key: %{public}s", key.c_str());
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo("KEYWORD", key);
    std::vector<std::string> columns = {"value"};
    DataShare::CreateOptions options;
    options.enabled_ = true;
    auto dataShareHelper = DataShare::DataShareHelper::Creator(SETTING_DATA_QUERY_URI, options);
    if (dataShareHelper == nullptr) {
        LOGE("dataShareHelper == nullptr");
        return E_RDB;
    }

    std::string queryUri = SETTING_DATA_COMMON_URI + "&key=" + key;
    Uri uri(queryUri);
    std::shared_ptr<DataShare::DataShareResultSet> resultSet = dataShareHelper->Query(uri, predicates, columns);
    dataShareHelper->Release();
    if (resultSet == nullptr) {
        LOGE("resultSet == nullptr");
        return E_RDB;
    }
    if (resultSet->GoToFirstRow() != E_OK) {
        LOGW("not found key: %{public}s", key.c_str());
        return E_INNER_RDB;
    }
    int32_t columnIndex = 0;
    if (resultSet->GetColumnIndex("value", columnIndex) != E_OK) {
        LOGE("GetColumnIndex failed");
        return E_RDB;
    }
    if (resultSet->GetString(columnIndex, value) != E_OK) {
        LOGE("GetString failed");
        return E_RDB;
    }

    LOGI("Query success, value: %{public}s", value.c_str());
    return E_OK;
}

void SettingsDataManager::RegisterObserver(const std::string &key)
{
    LOGD("register key: %{public}s", key.c_str());
    DataShare::CreateOptions options;
    options.enabled_ = true;
    auto dataShareHelper = DataShare::DataShareHelper::Creator(SETTING_DATA_QUERY_URI, options);
    if (dataShareHelper == nullptr) {
        LOGE("dataShareHelper == nullptr");
        return;
    }
    sptr<SettingsDataObserver> dataObserver(new (std::nothrow) SettingsDataObserver(key));
    Uri observerUri(SETTING_DATA_COMMON_URI + "&key=" + key);
    dataShareHelper->RegisterObserver(observerUri, dataObserver);
    dataShareHelper->Release();
    LOGI("Register SettingsDataObserver key: %{public}s finish", key.c_str());
}

void SettingsDataManager::RegisterObserver(const std::string &key, sptr<AAFwk::DataAbilityObserverStub> dataObserver)
{
    LOGD("register key: %{public}s", key.c_str());
    DataShare::CreateOptions options;
    options.enabled_ = true;
    auto dataShareHelper = DataShare::DataShareHelper::Creator(SETTING_DATA_QUERY_URI, options);
    if (dataShareHelper == nullptr) {
        LOGE("dataShareHelper == nullptr");
        return;
    }

    Uri observerUri(SETTING_DATA_COMMON_URI + "&key=" + key);
    dataShareHelper->RegisterObserver(observerUri, dataObserver);
    dataShareHelper->Release();
    LOGI("Register SettingsDataObserver key: %{public}s finish", key.c_str());
}

void SettingsDataManager::UnregisterObserver(const std::string &key)
{
    DataShare::CreateOptions options;
    options.enabled_ = true;
    auto dataShareHelper = DataShare::DataShareHelper::Creator(SETTING_DATA_QUERY_URI, options);
    if (dataShareHelper == nullptr) {
        LOGE("dataShareHelper == nullptr");
        return;
    }
    sptr<SettingsDataObserver> dataObserver(new (std::nothrow) SettingsDataObserver(key));
    Uri observerUri(SETTING_DATA_COMMON_URI + "&key=" + key);
    dataShareHelper->UnregisterObserver(observerUri, dataObserver);
    dataShareHelper->Release();
    LOGI("Unregister SettingsDataObserver key: %{public}s finish", key.c_str());
}

void SettingsDataObserver::OnChange()
{
    LOGD("change key: %{public}s", key_.c_str());
    if (key_ == SYNC_SWITCH_KEY) {
        SettingsDataManager::QuerySwitchStatus();
    } else if (key_ == NETWORK_CONNECTION_KEY) {
        SettingsDataManager::QueryNetworkConnectionStatus();
    } else if (key_ == LOCAL_SPACE_FREE_KEY) {
        SettingsDataManager::QueryLocalSpaceFreeStatus();
    } else if (key_ == LOCAL_SPACE_DAYS_KEY) {
        SettingsDataManager::QueryLocalSpaceFreeDays();
    } else if (key_ == MOBILE_DATA_SYNC_KEY) {
        SettingsDataManager::QueryMobileDataStatus();
    }
}
}