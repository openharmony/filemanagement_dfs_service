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

#ifndef OHOS_FILEMGMT_CLOUD_SYNC_SETTINGS_DATA_MANAGER_MOCK_H
#define OHOS_FILEMGMT_CLOUD_SYNC_SETTINGS_DATA_MANAGER_MOCK_H

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "settings_data_manager.h"

namespace OHOS::FileManagement::CloudSync {
class ISettingsDataManager {
public:
    virtual bool UpdateCurrentUserId() = 0;
    virtual int32_t QueryParamInSettingsData(const std::string &key, std::string &value) = 0;
    virtual int32_t QueryParamInUserSettingsData(const std::string &key, std::string &value) = 0;
    virtual int32_t QuerySwitchStatus(std::string &value) = 0;
    virtual int32_t QueryNetworkConnectionStatus(std::string &value) = 0;
    virtual int32_t QueryMobileDataStatus(std::string &value) = 0;
    virtual int32_t QueryLocalSpaceFreeStatus(std::string &value) = 0;
    virtual int32_t QueryLocalSpaceFreeDays(std::string &value) = 0;
    virtual SwitchStatus GetSwitchStatus() = 0;
    virtual SwitchStatus GetSwitchStatusByCache() = 0;
    virtual bool GetNetworkConnectionStatus() = 0;
    virtual bool GetMobileDataStatus() = 0;
    virtual int32_t GetLocalSpaceFreeStatus() = 0;
    virtual int32_t GetLocalSpaceFreeDays() = 0;
    virtual int32_t InitUserSettings() = 0;
    virtual int32_t InitAndQuerySettingsData(const std::string &key, std::string &value, bool isFirst) = 0;

    virtual ~ISettingsDataManager() = default;
};

class SettingsDataManagerMock : public ISettingsDataManager {
public:
    MOCK_METHOD0(UpdateCurrentUserId, bool());
    MOCK_METHOD2(QueryParamInSettingsData, int32_t(const std::string &key, std::string &value));
    MOCK_METHOD2(QueryParamInUserSettingsData, int32_t(const std::string &key, std::string &value));
    MOCK_METHOD1(QuerySwitchStatus, int32_t(std::string &value));
    MOCK_METHOD1(QueryNetworkConnectionStatus, int32_t(std::string &value));
    MOCK_METHOD1(QueryMobileDataStatus, int32_t(std::string &value));
    MOCK_METHOD1(QueryLocalSpaceFreeStatus, int32_t(std::string &value));
    MOCK_METHOD1(QueryLocalSpaceFreeDays, int32_t(std::string &value));
    MOCK_METHOD0(GetSwitchStatus, SwitchStatus());
    MOCK_METHOD0(GetSwitchStatusByCache, SwitchStatus());
    MOCK_METHOD0(GetNetworkConnectionStatus, bool());
    MOCK_METHOD0(GetMobileDataStatus, bool());
    MOCK_METHOD0(GetLocalSpaceFreeStatus, int32_t());
    MOCK_METHOD0(GetLocalSpaceFreeDays, int32_t());
    MOCK_METHOD0(InitUserSettings, int32_t());
    MOCK_METHOD3(InitAndQuerySettingsData, int32_t(const std::string &key, std::string &value, bool isFirst));

    static inline std::shared_ptr<SettingsDataManagerMock> proxy_ = nullptr;
};
} // OHOS
#endif // OHOS_FILEMGMT_CLOUD_SYNC_SETTINGS_DATA_MANAGER_MOCK_H