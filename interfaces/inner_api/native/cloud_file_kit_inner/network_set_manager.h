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

#include <functional>
#include <iostream>
#include <map>
#include <string>

#include "datashare_helper.h"
#include "data_ability_observer_stub.h"
#include "data_sync_manager.h"
#include "safe_map.h"

namespace OHOS::FileManagement::CloudSync {
class MobileNetworkObserver : public AAFwk::DataAbilityObserverStub {
public:
    MobileNetworkObserver(const std::string &bundleName, const int32_t userId, const int32_t type)
        : bundleName_(bundleName), userId_(userId), type_(type) {}
    ~MobileNetworkObserver() {}
    void OnChange() override;
    using ObserverCallback = std::function<void()>;
private:
    std::string bundleName_;
    int32_t userId_ = -1;
    int32_t type_ = -1;
    ObserverCallback observerCallback_ = nullptr;
};

class NetworkSetManager {
public:
    enum PhotoType {
        CELLULARCONNECT,
        NETCONNECT,
    };
    static void InitNetworkSetManager(const std::string &bundleName, const int32_t userId);
    static int32_t QueryCellularConnect(int32_t userId, const std::string &bundleName);
    static int32_t QueryNetConnect(int32_t userId, const std::string &bundleName);
    static void RegisterObserver(const std::string &bundleName, const int32_t userId, const int32_t type);
    static void UnregisterObserver(const std::string &bundleName, const int32_t userId, const int32_t type);
    static void GetCellularConnect(const std::string &bundleName, const int32_t userId);
    static void GetNetConnect(const std::string &bundleName, const int32_t userId);
    static bool IsAllowCellularConnect(const std::string &bundleName, const int32_t userId);
    static bool IsAllowNetConnect(const std::string &bundleName, const int32_t userId);
    static bool GetConfigParams(const std::string &bundleName, const int32_t userId);
    static inline SafeMap<const std::string, bool> cellularNetMap_;
    static inline SafeMap<const std::string, bool> netMap_;
};
} // OHOS

#endif // OHOS_FILEMGMT_CLOUD_SYNC_DATA_SHARE_H