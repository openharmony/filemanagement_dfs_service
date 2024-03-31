/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_CLOUD_DISK_SERVICE_SYNC_HELPER_H
#define OHOS_CLOUD_DISK_SERVICE_SYNC_HELPER_H

#include <mutex>

#include <timer.h>
#include <unordered_map>
#include <functional>

#include "cloud_sync_manager_lite.h"

namespace OHOS {
namespace FileManagement {
namespace CloudDisk {
constexpr int32_t SYNC_INTERVAL = 5000;
constexpr int32_t ARGS_SIZE = 2;
enum TRIGGER_SYNC_ARGS : int32_t {
    ARG_USER_ID = 0,
    ARG_BUNDLE_NAME
};
class CloudDiskSyncHelper final {
public:
    using TriggerSyncCallback = std::function<void()>;
    static CloudDiskSyncHelper& GetInstance();
    CloudDiskSyncHelper(const CloudDiskSyncHelper&) = delete;
    CloudDiskSyncHelper& operator=(const CloudDiskSyncHelper&) = delete;
    void RegisterTriggerSync(const std::string &bundleName, const int32_t &userId);

private:
    CloudDiskSyncHelper();
    ~CloudDiskSyncHelper();
    void UnregisterRepeatingTriggerSync(const std::string &bundleName, const int32_t &userId);
    void OnTriggerSyncCallback(const std::string &bundleName, const int32_t &userId);

    struct TriggerInfo {
        uint32_t timerId{0};
        TriggerSyncCallback callback;
    };
    std::unique_ptr<Utils::Timer> timer_;
    std::mutex registerMutex_;
    std::mutex unregisterMutex_;
    std::mutex callbackMutex_;
    std::unordered_map<std::string, std::shared_ptr<TriggerInfo>> triggerInfoMap_;
};
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS

#endif // OHOS_CLOUD_DISK_SERVICE_SYNC_HELPER_H