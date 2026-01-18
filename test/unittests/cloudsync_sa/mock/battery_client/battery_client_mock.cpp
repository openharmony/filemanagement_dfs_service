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

#include "battery_client_mock.h"
#include "utils_log.h"

namespace OHOS {
namespace PowerMgr {

constexpr int32_t DEFAULT_BATTERY_CAPACITY = 100;

int32_t BatterySrvClient::GetCapacity()
{
    if (BatteryClientMethod::batteryClientMethod_ == nullptr) {
        LOGW("BatteryClientMethod::batteryClientMethod_ is nullptr, return default capacity");
        return DEFAULT_BATTERY_CAPACITY;
    }
    return BatteryClientMethod::batteryClientMethod_->GetCapacity();
}

BatteryChargeState BatterySrvClient::GetChargingStatus()
{
    if (BatteryClientMethod::batteryClientMethod_ == nullptr) {
        LOGW("BatteryClientMethod::batteryClientMethod_ is nullptr, return default charging status");
        return BatteryChargeState::CHARGE_STATE_NONE;
    }
    return BatteryClientMethod::batteryClientMethod_->GetChargingStatus();
}

BatteryPluggedType BatterySrvClient::GetPluggedType()
{
    if (BatteryClientMethod::batteryClientMethod_ == nullptr) {
        LOGW("BatteryClientMethod::batteryClientMethod_ is nullptr, return default plugged type");
        return BatteryPluggedType::PLUGGED_TYPE_NONE;
    }
    return BatteryClientMethod::batteryClientMethod_->GetPluggedType();
}

} // namespace PowerMgr
} // namespace OHOS
