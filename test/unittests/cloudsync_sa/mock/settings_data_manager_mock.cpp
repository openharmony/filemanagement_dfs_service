/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "settings_data_manager_mock.h"

namespace OHOS::FileManagement::CloudSync {
std::string SettingsDataManager::GetQueryKey(const std::string &key)
{
    return "";
}

std::string SettingsDataManager::GetSettingsDataCommonUri()
{
    return "";
}

std::string SettingsDataManager::GetSettingsDataUri(const std::string &key)
{
    return "";
}

std::string SettingsDataManager::GetUserSettingsDataUri(const std::string &key)
{
    return "";
}

void SettingsDataManager::UpdateIsSupportUserSettingsData()
{
}

bool SettingsDataManager::UpdateCurrentUserId()
{
    if (SettingsDataManagerMock::proxy_ != nullptr) {
        return SettingsDataManagerMock::proxy_->UpdateCurrentUserId();
    }
    return true;
}

void SettingsDataManager::OnUserSwitched(int32_t userId)
{
}

void SettingsDataManager::ReregisterAllObservers(int32_t userId)
{
}

void SettingsDataManager::PreInit()
{
}

void SettingsDataManager::InitSettingsDataManager()
{
}

int32_t SettingsDataManager::QuerySwitchStatus(std::string &value)
{
    if (SettingsDataManagerMock::proxy_ != nullptr) {
        return SettingsDataManagerMock::proxy_->QuerySwitchStatus(value);
    }
    return 0;
}

int32_t SettingsDataManager::QueryNetworkConnectionStatus(std::string &value)
{
    if (SettingsDataManagerMock::proxy_ != nullptr) {
        return SettingsDataManagerMock::proxy_->QueryNetworkConnectionStatus(value);
    }
    return 0;
}

int32_t SettingsDataManager::QueryMobileDataStatus(std::string &value)
{
    if (SettingsDataManagerMock::proxy_ != nullptr) {
        return SettingsDataManagerMock::proxy_->QueryMobileDataStatus(value);
    }
    return 0;
}

int32_t SettingsDataManager::QueryLocalSpaceFreeStatus(std::string &value)
{
    if (SettingsDataManagerMock::proxy_ != nullptr) {
        return SettingsDataManagerMock::proxy_->QueryLocalSpaceFreeStatus(value);
    }
    return 0;
}

int32_t SettingsDataManager::QueryLocalSpaceFreeDays(std::string &value)
{
    if (SettingsDataManagerMock::proxy_ != nullptr) {
        return SettingsDataManagerMock::proxy_->QueryLocalSpaceFreeDays(value);
    }
    return 0;
}

SwitchStatus SettingsDataManager::GetSwitchStatus()
{
    if (SettingsDataManagerMock::proxy_ != nullptr) {
        return SettingsDataManagerMock::proxy_->GetSwitchStatus();
    }
    return SwitchStatus::NONE;
}

SwitchStatus SettingsDataManager::GetSwitchStatusByCache()
{
    if (SettingsDataManagerMock::proxy_ != nullptr) {
        return SettingsDataManagerMock::proxy_->GetSwitchStatusByCache();
    }
    return SwitchStatus::NONE;
}

bool SettingsDataManager::GetNetworkConnectionStatus()
{
    if (SettingsDataManagerMock::proxy_ != nullptr) {
        return SettingsDataManagerMock::proxy_->GetNetworkConnectionStatus();
    }
    return true;
}

bool SettingsDataManager::GetMobileDataStatus()
{
    if (SettingsDataManagerMock::proxy_ != nullptr) {
        return SettingsDataManagerMock::proxy_->GetMobileDataStatus();
    }
    return true;
}

int32_t SettingsDataManager::GetLocalSpaceFreeStatus()
{
    if (SettingsDataManagerMock::proxy_ != nullptr) {
        return SettingsDataManagerMock::proxy_->GetLocalSpaceFreeStatus();
    }
    return 0;
}

int32_t SettingsDataManager::GetLocalSpaceFreeDays()
{
    if (SettingsDataManagerMock::proxy_ != nullptr) {
        return SettingsDataManagerMock::proxy_->GetLocalSpaceFreeDays();
    }
    return 0;
}

int32_t SettingsDataManager::InitUserSettings()
{
    if (SettingsDataManagerMock::proxy_ != nullptr) {
        return SettingsDataManagerMock::proxy_->InitUserSettings();
    }
    return 0;
}

int32_t SettingsDataManager::InitAndQuerySettingsData(const std::string &key, std::string &value, bool isFirst)
{
    if (SettingsDataManagerMock::proxy_ != nullptr) {
        return SettingsDataManagerMock::proxy_->InitAndQuerySettingsData(key, value, isFirst);
    }
    return 0;
}

int32_t SettingsDataManager::QueryParamInUserSettingsData(const std::string &key, std::string &value)
{
    if (SettingsDataManagerMock::proxy_ != nullptr) {
        return SettingsDataManagerMock::proxy_->QueryParamInUserSettingsData(key, value);
    }
    return 0;
}

int32_t SettingsDataManager::QueryParamInSettingsData(const std::string &key, std::string &value)
{
    if (SettingsDataManagerMock::proxy_ != nullptr) {
        return SettingsDataManagerMock::proxy_->QueryParamInSettingsData(key, value);
    }
    return 0;
}

void SettingsDataManager::RegisterObserver(const std::string &key)
{
}

void SettingsDataManager::RegisterObserver(const std::string &key, sptr<AAFwk::DataAbilityObserverStub> dataObserver)
{
}

void SettingsDataManager::UnregisterDemonObserver(const std::string &key,
    sptr<AAFwk::DataAbilityObserverStub> dataObserver)
{
}

void SettingsDataManager::UnregisterObserver(const std::string &key)
{
}
}