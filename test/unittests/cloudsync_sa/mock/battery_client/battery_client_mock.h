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

#ifndef OHOS_CLOUD_SYNC_BATTERY_CLIENT_MOCK_H
#define OHOS_CLOUD_SYNC_BATTERY_CLIENT_MOCK_H

#include <gmock/gmock.h>
#include <memory>
#include "battery_info.h"
#include "battery_srv_client.h"

namespace OHOS {
namespace PowerMgr {

class BatteryClientMethod {
public:
    virtual ~BatteryClientMethod() = default;
public:
    virtual int32_t GetCapacity() = 0;
    virtual BatteryChargeState GetChargingStatus() = 0;
    virtual BatteryPluggedType GetPluggedType() = 0;
public:
    static inline std::shared_ptr<BatteryClientMethod> batteryClientMethod_ = nullptr;
};

class BatteryClientMethodMock : public BatteryClientMethod {
public:
    MOCK_METHOD0(GetCapacity, int32_t());
    MOCK_METHOD0(GetChargingStatus, BatteryChargeState());
    MOCK_METHOD0(GetPluggedType, BatteryPluggedType());
};

} // namespace PowerMgr
} // namespace OHOS

#endif // OHOS_CLOUD_SYNC_BATTERY_CLIENT_MOCK_H
