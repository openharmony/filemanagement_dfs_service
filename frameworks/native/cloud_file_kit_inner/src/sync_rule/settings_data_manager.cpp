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
#include "iservice_registry.h"
#include "os_account_manager.h"
#include "parameters.h"
#include "system_ability_definition.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
static const std::string SETTING_DATA_QUERY_URI = "datashareproxy://";
static const std::string SETTING_DATA_COMMON_URI = "datashare:///com.ohos.settingsdata/entry/settingsdata";
static const std::string PHOTOS_KEY = "persist.kernel.bundle_name.photos";
static const std::string SYNC_SWITCH_KEY = "photos_sync_options";                       // "0", "1", "2"
static const std::string NETWORK_CONNECTION_KEY = "photo_network_connection_status";    // "on", "off"
static const std::string LOCAL_SPACE_FREE_KEY = "photos_local_space_free";              // "1", "0"
static const std::string LOCAL_SPACE_DAYS_KEY = "photos_local_space_free_day_count";    // "30"
static const std::string MOBILE_DATA_SYNC_KEY = "photos_mobile_data_sync";              // "1", "0"
static const std::string USER_SUFFIX = "_user";
static const std::string AI_FAMILY_STATUS = "2";
static const std::string CLOUD_STATUS = "1";
static const std::string OPEN_STATUS = "on";
static const std::string ALLOW_STATUS = "1";
static const std::string LSF_ALLOW_STATUS = "0";
static const int32_t LOCAL_SPACE_DAYS_DEFAULT = 30;

std::string SettingsDataManager::GetQueryKey(const std::string &key)
{
    if (supportUserSettingsData_) {
        return key + USER_SUFFIX;
    } else {
        return key;
    }
}

std::string SettingsDataManager::GetSettingsDataCommonUri()
{
    if (supportUserSettingsData_) {
        return SETTING_DATA_COMMON_URI + "/USER_SETTINGSDATA_" + std::to_string(currentUserId_) + "?Proxy=true";
    } else {
        return SETTING_DATA_COMMON_URI + "/SETTINGSDATA" + "?Proxy=true";
    }
}

std::string SettingsDataManager::GetSettingsDataUri(const std::string &key)
{
    return GetSettingsDataCommonUri() + "&key=" + GetQueryKey(key);
}

std::string SettingsDataManager::GetUserSettingsDataUri(const std::string &key)
{
    return SETTING_DATA_COMMON_URI + "/USER_SETTINGSDATA_" + std::to_string(currentUserId_) + "?Proxy=true&key=" +
        key + USER_SUFFIX;
}

void SettingsDataManager::UpdateIsSupportUserSettingsData(bool isDemon)
{
    std::string value;
    int32_t ret = E_OK;
    if (isDemon) {
        ret = QueryParamInUserSettingsData(SYNC_SWITCH_KEY, value);
    } else {
        ret = InitAndQuerySettingsData(SYNC_SWITCH_KEY, value, true);
    }
    supportUserSettingsData_ = ret == E_OK;
    LOGI("supportUserSettingsData_: %{public}d", supportUserSettingsData_);
}

void SettingsDataManager::UpdateCurrentUserId()
{
    int32_t foregroundLocalId = -1;
    int32_t ret = AccountSA::OsAccountManager::GetForegroundOsAccountLocalId(foregroundLocalId);
    if (ret == E_OK) {
        currentUserId_ = foregroundLocalId;
        LOGI("Update current user id to: %{public}d", currentUserId_);
    } else {
        LOGE("GetForegroundOsAccountLocalId failed, ret: %{public}d, currentUserId_: %{public}d", ret, currentUserId_);
    }
}

void SettingsDataManager::OnUserSwitched(int32_t userId)
{
    LOGI("User switched to: %{public}d, supportUserSettingsData_: %{public}d", userId, supportUserSettingsData_);
    if (supportUserSettingsData_) {
        std::string value;
        ReregisterAllObservers(userId);
        QuerySwitchStatus(value);
        QueryNetworkConnectionStatus(value);
        QueryLocalSpaceFreeStatus(value);
        QueryLocalSpaceFreeDays(value);
        QueryMobileDataStatus(value);
    }
}

void SettingsDataManager::ReregisterAllObservers(int32_t userId)
{
    UnregisterObserver(SYNC_SWITCH_KEY);
    UnregisterObserver(NETWORK_CONNECTION_KEY);
    UnregisterObserver(MOBILE_DATA_SYNC_KEY);
    UnregisterObserver(LOCAL_SPACE_FREE_KEY);
    UnregisterObserver(LOCAL_SPACE_DAYS_KEY);
    UnregisterDemonObserver(SYNC_SWITCH_KEY);

    currentUserId_ = userId;
    settingsDataMap_.Clear();
    observerMap_.Clear();

    RegisterObserver(SYNC_SWITCH_KEY);
    RegisterObserver(NETWORK_CONNECTION_KEY);
    RegisterObserver(MOBILE_DATA_SYNC_KEY);
    RegisterObserver(LOCAL_SPACE_FREE_KEY);
    RegisterObserver(LOCAL_SPACE_DAYS_KEY);
    ReregisterDemonObserver(SYNC_SWITCH_KEY);
}

void SettingsDataManager::InitSettingsDataManager()
{
    LOGI("InitSettingsDataManager");
    UpdateCurrentUserId();
    UpdateIsSupportUserSettingsData();

    RegisterObserver(SYNC_SWITCH_KEY);
    RegisterObserver(NETWORK_CONNECTION_KEY);
    RegisterObserver(MOBILE_DATA_SYNC_KEY);
    RegisterObserver(LOCAL_SPACE_FREE_KEY);
    RegisterObserver(LOCAL_SPACE_DAYS_KEY);

    std::string value;
    QuerySwitchStatus(value);
    QueryNetworkConnectionStatus(value);
    QueryLocalSpaceFreeStatus(value);
    QueryLocalSpaceFreeDays(value);
    QueryMobileDataStatus(value);
}

int32_t SettingsDataManager::QuerySwitchStatus(std::string &value)
{
    int32_t ret = QueryParamInSettingsData(SYNC_SWITCH_KEY, value);
    if (ret != E_OK && supportUserSettingsData_) {
        ret = InitAndQuerySettingsData(SYNC_SWITCH_KEY, value);
    }
    if (ret != E_OK) {
        LOGE("query SYNC_SWITCH_KEY failed");
        settingsDataMap_.Erase(SYNC_SWITCH_KEY);
        return ret;
    }
    settingsDataMap_.EnsureInsert(SYNC_SWITCH_KEY, value);
    return E_OK;
}

int32_t SettingsDataManager::QueryNetworkConnectionStatus(std::string &value)
{
    int32_t ret = QueryParamInSettingsData(NETWORK_CONNECTION_KEY, value);
    if (ret != E_OK) {
        LOGE("query NETWORK_CONNECTION_KEY failed");
        settingsDataMap_.Erase(NETWORK_CONNECTION_KEY);
        return ret;
    }
    settingsDataMap_.EnsureInsert(NETWORK_CONNECTION_KEY, value);
    return E_OK;
}

int32_t SettingsDataManager::QueryMobileDataStatus(std::string &value)
{
    int32_t ret = QueryParamInSettingsData(MOBILE_DATA_SYNC_KEY, value);
    if (ret != E_OK) {
        LOGE("query MOBILE_DATA_SYNC_KEY failed");
        settingsDataMap_.Erase(MOBILE_DATA_SYNC_KEY);
        return ret;
    }
    settingsDataMap_.EnsureInsert(MOBILE_DATA_SYNC_KEY, value);
    return E_OK;
}

int32_t SettingsDataManager::QueryLocalSpaceFreeStatus(std::string &value)
{
    int32_t ret = QueryParamInSettingsData(LOCAL_SPACE_FREE_KEY, value);
    if (ret != E_OK) {
        LOGE("query LOCAL_SPACE_FREE_KEY failed");
        settingsDataMap_.Erase(LOCAL_SPACE_FREE_KEY);
        return ret;
    }
    settingsDataMap_.EnsureInsert(LOCAL_SPACE_FREE_KEY, value);
    return E_OK;
}

int32_t SettingsDataManager::QueryLocalSpaceFreeDays(std::string &value)
{
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
    int32_t ret = QuerySwitchStatus(value);
    if (ret == E_INNER_RDB) {
        return SwitchStatus::CLOUD_SPACE;
    } else if (ret == E_RDB) {
        return SwitchStatus::NONE;
    }

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
        if (QueryNetworkConnectionStatus(value) != E_OK) {
            return true;
        }
    }
    return value == OPEN_STATUS;
}

bool SettingsDataManager::GetMobileDataStatus()
{
    std::string value;
    if (!settingsDataMap_.Find(MOBILE_DATA_SYNC_KEY, value)) {
        if (QueryMobileDataStatus(value) != E_OK) {
            return false;
        }
    }
    return value == ALLOW_STATUS;
}

int32_t SettingsDataManager::GetLocalSpaceFreeStatus()
{
    std::string value;
    if (!settingsDataMap_.Find(LOCAL_SPACE_FREE_KEY, value)) {
        if (QueryLocalSpaceFreeStatus(value) != E_OK) {
            return 1;
        }
    }
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
        if (QueryLocalSpaceFreeDays(value) != E_OK) {
            return LOCAL_SPACE_DAYS_DEFAULT;
        }
    }

    int32_t val = 0;
    if (ConvertToInt(value, val)) {
        return val;
    } else {
        LOGE("ConvertToInt failed, value: %{public}s", value.c_str());
        return LOCAL_SPACE_DAYS_DEFAULT;
    }
}

static std::string GetSettingsDataInitUri()
{
    std::string photoBundleName = system::GetParameter(PHOTOS_KEY, "");
    return "datashare:///" + photoBundleName + ".provider.userPropertySettings";
}

static std::shared_ptr<DataShare::DataShareHelper> CreateDataShareHelper()
{
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        LOGE("Samgr is nullptr");
        return nullptr;
    }
    auto object = samgr->CheckSystemAbility(FILEMANAGEMENT_CLOUD_SYNC_SERVICE_SA_ID);
    if (object == nullptr) {
        LOGE("object == nullptr");
        return nullptr;
    }
    return DataShare::DataShareHelper::Creator(object, GetSettingsDataInitUri());
}

int32_t SettingsDataManager::InitUserSettings()
{
    LOGI("call photos datashare interface to init user settings");
    auto dataShareHelper = CreateDataShareHelper();
    if (dataShareHelper == nullptr) {
        LOGE("dataShareHelper == nullptr");
        return E_RDB;
    }

    DataShare::DataSharePredicates predicates;
    DataShare::DataShareValuesBucket valuesBucket;
    DataShare::DataShareValueObject valueObject(1);
    valuesBucket.Put("init", valueObject);
    std::string initUri = GetSettingsDataInitUri() + "?init";
    LOGI("initUri: %{public}s", initUri.c_str());
    Uri uri(initUri);
    auto ret = dataShareHelper->UpdateEx(uri, predicates, valuesBucket);
    dataShareHelper->Release();

    LOGI("InitUserSettings end, ret: %{public}d", ret.first);
    return ret.first;
}

int32_t SettingsDataManager::InitAndQuerySettingsData(const std::string &key, std::string &value, bool isFirst)
{
    int32_t ret = E_OK;
    if (isFirst) {
        ret = QueryParamInUserSettingsData(key, value);
        if (ret == E_OK) {
            return E_OK;
        }
    }

    ret = InitUserSettings();
    if (ret != E_OK) {
        LOGE("InitUserSettings failed");
        return E_RDB;
    }

    ret = QueryParamInUserSettingsData(key, value);
    if (ret != E_OK) {
        LOGE("QueryParamInUserSettingsData failed");
        return E_RDB;
    }
    return E_OK;
}

int32_t SettingsDataManager::QueryParamInUserSettingsData(const std::string &key, std::string &value)
{
    std::string userKey = key + USER_SUFFIX;
    LOGI("Query userKey: %{public}s, currentUserId: %{public}d", userKey.c_str(), currentUserId_);
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo("KEYWORD", userKey);
    std::vector<std::string> columns = {"value"};
    DataShare::CreateOptions options;
    options.enabled_ = true;
    auto dataShareHelper = DataShare::DataShareHelper::Creator(SETTING_DATA_QUERY_URI, options);
    if (dataShareHelper == nullptr) {
        LOGE("dataShareHelper == nullptr");
        return E_RDB;
    }

    std::string queryUri = GetUserSettingsDataUri(key);
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

    LOGI("Query success, userKey: %{public}s, value: %{public}s", userKey.c_str(), value.c_str());
    return E_OK;
}

int32_t SettingsDataManager::QueryParamInSettingsData(const std::string &key, std::string &value)
{
    std::string queryKey = GetQueryKey(key);
    LOGI("Query key: %{public}s, currentUserId: %{public}d, supportUserSettingsData_: %{public}d",
        queryKey.c_str(), currentUserId_, supportUserSettingsData_);
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo("KEYWORD", queryKey);
    std::vector<std::string> columns = {"value"};
    DataShare::CreateOptions options;
    options.enabled_ = true;
    auto dataShareHelper = DataShare::DataShareHelper::Creator(SETTING_DATA_QUERY_URI, options);
    if (dataShareHelper == nullptr) {
        LOGE("dataShareHelper == nullptr");
        return E_RDB;
    }

    std::string queryUri = GetSettingsDataUri(key);
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

    LOGI("Query success, key: %{public}s, value: %{public}s", queryKey.c_str(), value.c_str());
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
    std::string uri = GetSettingsDataUri(key);
    Uri observerUri(uri);
    dataShareHelper->RegisterObserver(observerUri, dataObserver);
    dataShareHelper->Release();

    observerMap_.EnsureInsert(key, dataObserver);
    LOGI("Register SettingsDataObserver uri: %{public}s finish", uri.c_str());
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

    std::string uri = GetSettingsDataUri(key);
    Uri observerUri(uri);
    dataShareHelper->RegisterObserver(observerUri, dataObserver);
    dataShareHelper->Release();
    observerDemon_.emplace_back(dataObserver);
    LOGI("Register SettingsDataObserver uri: %{public}s finish", uri.c_str());
}

void SettingsDataManager::ReregisterDemonObserver(const std::string &key)
{
    if (observerDemon_.empty()) {
        return;
    }

    DataShare::CreateOptions options;
    options.enabled_ = true;
    auto dataShareHelper = DataShare::DataShareHelper::Creator(SETTING_DATA_QUERY_URI, options);
    if (dataShareHelper == nullptr) {
        LOGE("dataShareHelper == nullptr");
        return;
    }

    Uri observerUri(GetSettingsDataUri(key));
    for (sptr<AAFwk::DataAbilityObserverStub> dataObserver : observerDemon_) {
        dataShareHelper->RegisterObserver(observerUri, dataObserver);
    }
    dataShareHelper->Release();
}

void SettingsDataManager::UnregisterDemonObserver(const std::string &key)
{
    if (observerDemon_.empty()) {
        return;
    }

    DataShare::CreateOptions options;
    options.enabled_ = true;
    auto dataShareHelper = DataShare::DataShareHelper::Creator(SETTING_DATA_QUERY_URI, options);
    if (dataShareHelper == nullptr) {
        LOGE("dataShareHelper == nullptr");
        return;
    }

    Uri observerUri(GetSettingsDataUri(key));
    for (sptr<AAFwk::DataAbilityObserverStub> dataObserver : observerDemon_) {
        dataShareHelper->UnregisterObserver(observerUri, dataObserver);
    }
    dataShareHelper->Release();
}

void SettingsDataManager::UnregisterObserver(const std::string &key)
{
    LOGD("unregister key: %{public}s", key.c_str());
    sptr<AAFwk::DataAbilityObserverStub> dataObserver;
    if (!observerMap_.Find(key, dataObserver)) {
        LOGW("Observer for key %{public}s not found in cache", key.c_str());
        return;
    }

    DataShare::CreateOptions options;
    options.enabled_ = true;
    auto dataShareHelper = DataShare::DataShareHelper::Creator(SETTING_DATA_QUERY_URI, options);
    if (dataShareHelper == nullptr) {
        LOGE("dataShareHelper == nullptr");
        return;
    }

    std::string uri = GetSettingsDataUri(key);
    Uri observerUri(uri);
    dataShareHelper->UnregisterObserver(observerUri, dataObserver);
    dataShareHelper->Release();

    observerMap_.Erase(key);
    LOGI("Unregister SettingsDataObserver uri: %{public}s finish", uri.c_str());
}

void SettingsDataObserver::OnChange()
{
    LOGD("change key: %{public}s", key_.c_str());
    std::string value;
    if (key_ == SYNC_SWITCH_KEY) {
        SettingsDataManager::QuerySwitchStatus(value);
    } else if (key_ == NETWORK_CONNECTION_KEY) {
        SettingsDataManager::QueryNetworkConnectionStatus(value);
    } else if (key_ == LOCAL_SPACE_FREE_KEY) {
        SettingsDataManager::QueryLocalSpaceFreeStatus(value);
    } else if (key_ == LOCAL_SPACE_DAYS_KEY) {
        SettingsDataManager::QueryLocalSpaceFreeDays(value);
    } else if (key_ == MOBILE_DATA_SYNC_KEY) {
        SettingsDataManager::QueryMobileDataStatus(value);
    }
}
}