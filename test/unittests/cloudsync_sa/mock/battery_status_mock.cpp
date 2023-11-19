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
#include "sync_rule/battery_status.h"

namespace OHOS::FileManagement::CloudSync {
constexpr int32_t DEFAULT_BATTERY_CAPCITY = 100;

void BatteryStatus::SetChargingStatus(bool status)
{
}

void BatteryStatus::GetInitChargingStatus()
{
}

int32_t BatteryStatus::GetCapacity()
{
    int32_t capacity = DEFAULT_BATTERY_CAPCITY;
    return capacity;
}

bool BatteryStatus::IsAllowUpload(bool forceFlag)
{
    return true;
}

bool BatteryStatus::IsBatteryCapcityOkay()
{
    if (level_ == LEVEL_TOO_LOW) {
        return false;
    }
    return true;
}

BatteryStatus::CapacityLevel BatteryStatus::GetCapacityLevel()
{
    return level_;
}

bool BatteryStatus::IsCharging()
{
    return true;
}
} // namespace OHOS::FileManagement::CloudSync