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

#ifndef OHOS_FILEMGMT_BATTERY_STATUS_H
#define OHOS_FILEMGMT_BATTERY_STATUS_H

#include <cstdint>
#include <atomic>

#ifdef SUPPORT_POWER
#include "battery_info.h"
#include "battery_srv_client.h"
#endif

namespace OHOS::FileManagement::CloudSync {
class BatteryStatus {
public:
    enum CapacityLevel {
        LEVEL_NORMAL,
        LEVEL_LOW,
        LEVEL_TOO_LOW,
    };
    static bool IsAllowUpload(bool forceFlag);
    static bool IsBatteryCapcityOkay();
    static CapacityLevel GetCapacityLevel();
    static void GetInitChargingStatus();
    static void SetChargingStatus(bool status);
    static bool IsCharging();
private:
    static int32_t GetCapacity();
    static inline CapacityLevel level_{LEVEL_NORMAL};
    static inline std::atomic_bool isCharging_{false};
};
} // namespace OHOS::FileManagement::CloudSync

#endif // OHOS_FILEMGMT_BATTERY_STATUS_H