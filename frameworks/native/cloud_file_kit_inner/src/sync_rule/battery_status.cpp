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
#include "battery_status.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
constexpr int32_t PAUSE_CAPACITY_LIMIT = 15;
constexpr int32_t STOP_CAPACITY_LIMIT = 10;
constexpr int32_t FULL_BATTERY_CAPACITY = 100;

void BatteryStatus::SetChargingStatus(bool status)
{
    isCharging_.store(status);
}

void BatteryStatus::SetCapacity(int32_t capacity)
{
    capacity_.store(capacity);
}

void BatteryStatus::GetInitChargingStatus()
{
#ifdef SUPPORT_POWER
    auto &batterySrvClient = PowerMgr::BatterySrvClient::GetInstance();
    auto pluggedType = batterySrvClient.GetPluggedType();
    isCharging_.store(pluggedType != PowerMgr::BatteryPluggedType::PLUGGED_TYPE_NONE &&
                  pluggedType != PowerMgr::BatteryPluggedType::PLUGGED_TYPE_BUTT);
    capacity_.store(batterySrvClient.GetCapacity());
    LOGI("pluggedType: %{public}d, isCharging: %{public}d, capacity: %{public}d",
        pluggedType, isCharging_.load(), capacity_.load());
#endif
}

int32_t BatteryStatus::GetCapacity()
{
#ifdef SUPPORT_POWER
    auto capacity = capacity_.load();
    if (capacity < 0) {
        auto &batterySrvClient = PowerMgr::BatterySrvClient::GetInstance();
        capacity = batterySrvClient.GetCapacity();
    }
    return capacity;
#endif
    return FULL_BATTERY_CAPACITY;
}

bool BatteryStatus::IsAllowSync(bool forceFlag)
{
    if (isCharging_.load()) {
        return true;
    }

    auto capacity = GetCapacity();
    if (capacity < STOP_CAPACITY_LIMIT) {
        LOGE("power saving, stop sync");
        level_.store(BatteryStatus::LEVEL_TOO_LOW);
        return false;
    } else if (capacity < PAUSE_CAPACITY_LIMIT) {
        if (forceFlag) {
            return true;
        } else {
            LOGE("power saving, pause sync");
            level_.store(BatteryStatus::LEVEL_LOW);
            return false;
        }
    } else {
        level_.store(BatteryStatus::LEVEL_NORMAL);
        return true;
    }
}

bool BatteryStatus::IsBatteryCapcityOkay()
{
    if (isCharging_.load()) {
        return true;
    }

    auto capacity = GetCapacity();
    if (capacity < STOP_CAPACITY_LIMIT) {
        LOGE("battery capacity too low");
        level_.store(BatteryStatus::LEVEL_TOO_LOW);
        return false;
    }
    level_.store(BatteryStatus::LEVEL_NORMAL);
    return true;
}

BatteryStatus::CapacityLevel BatteryStatus::GetCapacityLevel()
{
    return level_.load();
}

bool BatteryStatus::IsCharging()
{
    return isCharging_.load();
}
} // namespace OHOS::FileManagement::CloudSync