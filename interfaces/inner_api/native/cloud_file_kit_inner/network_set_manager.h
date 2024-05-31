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

#ifndef OHOS_FILEMGMT_CLOUD_SYNC_DATA_SHARE_H
#define OHOS_FILEMGMT_CLOUD_SYNC_DATA_SHARE_H

#include "datashare_helper.h"
#include "data_ability_observer_stub.h"
#include "data_sync_manager.h"
#include <functional>
#include <string>

namespace OHOS::FileManagement::CloudSync {
class MobileNetworkObserver : public AAFwk::DataAbilityObserverStub {
public:
    MobileNetworkObserver() {}
    ~MobileNetworkObserver() {}
    void OnChange() override;
    using ObserverCallback = std::function<void()>;
private:
    ObserverCallback observerCallback_ = nullptr;
};

class NetworkSetManager {
public:
    static void InitNetworkSetManager(const std::string &bundleName, const int32_t userId);
    static int32_t QueryByDataShare(int32_t userId, const std::string &bundleName);
    static void RegisterObserver();
    static void UnregisterObserver();
    static void GetNetworkSetting(int32_t userId, const std::string &bundleName);
    static bool IsNetworkSetStatusOkay(const std::string &bundleName, const int32_t userId);
    static bool GetConfigParams(const std::string &bundleName, const int32_t userId);
    static inline bool networkSetStatus_;
    static inline int32_t userId_;
    static inline std::string bundleName_;
};
} // OHOS

#endif // OHOS_FILEMGMT_CLOUD_SYNC_DATA_SHARE_H