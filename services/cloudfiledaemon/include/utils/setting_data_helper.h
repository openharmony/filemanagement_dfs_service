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
#ifndef SETTING_DATA_HELPER_H
#define SETTING_DATA_HELPER_H
#include <map>
#include <mutex>
#include <string>

#include "data_ability_observer_stub.h"

namespace OHOS {
namespace FileManagement {
namespace CloudFile {
class SyncSwitchObserver : public AAFwk::DataAbilityObserverStub {
public:
    SyncSwitchObserver() = default;
    ~SyncSwitchObserver() = default;
    void OnChange() override;
};

class SettingDataHelper {
public:
    static SettingDataHelper &GetInstance();
    void SetUserData(int32_t userId, void *data);
    bool InitActiveBundle();
    void UpdateActiveBundle(int32_t userId = -1);

private:
    std::string GetActiveBundle();
    bool IsDataShareReady();
    void RegisterObserver();
    void SetActiveBundle(int32_t userId, std::string bundle);
    bool GetForegroundUser(int32_t &userId);

private:
    std::map<int32_t, void *> dataMap_;
    bool isDataShareReady_ = false;
    bool isBundleInited_ = false;
    std::mutex dataMtx_;
    std::mutex initMtx_;
};
} // namespace CloudFile
} // namespace FileManagement
} // namespace OHOS
#endif