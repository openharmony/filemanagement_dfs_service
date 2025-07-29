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

#ifndef OHOS_FILEMGMT_CLOUD_SYNC_SETTINGS_DATA_MANAGER_H
#define OHOS_FILEMGMT_CLOUD_SYNC_SETTINGS_DATA_MANAGER_H

#include <string>

#include "data_ability_observer_stub.h"
#include "safe_map.h"

namespace OHOS::FileManagement::CloudSync {
enum SwitchStatus {
    NONE = 0,
    CLOUD_SPACE,
    AI_FAMILY,
};

class SettingsDataObserver : public AAFwk::DataAbilityObserverStub {
public:
    SettingsDataObserver(const std::string &key) : key_(key) {}
    ~SettingsDataObserver() {}
    void OnChange() override;

private:
    std::string key_;
};

class SettingsDataManager {
public:
    static void InitSettingsDataManager();

    static int32_t QuerySwitchStatus();
    static int32_t QueryNetworkConnectionStatus();
    static int32_t QueryLocalSpaceFreeStatus();
    static int32_t QueryLocalSpaceFreeDays();
    static int32_t QueryMobileDataStatus();

    static SwitchStatus GetSwitchStatus();
    static SwitchStatus GetSwitchStatusByCache();
    static std::string GetNetworkConnectionStatus();
    static int32_t GetLocalSpaceFreeStatus();
    static int32_t GetLocalSpaceFreeDays();
    static std::string GetMobileDataStatus();

    static void RegisterObserver(const std::string &key);
    static void RegisterObserver(const std::string &key, sptr<AAFwk::DataAbilityObserverStub> dataObserver);

private:
    static int32_t QueryParamInSettingsData(const std::string &key, std::string &value);
    static void UnregisterObserver(const std::string &key);

    static inline SafeMap<const std::string, std::string> settingsDataMap_;
};
} // OHOS

#endif // OHOS_FILEMGMT_CLOUD_SYNC_SETTINGS_DATA_MANAGER_H