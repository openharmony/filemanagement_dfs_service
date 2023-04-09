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

#include "sync_rule/battery_status_listener.h"

#include "common_event_manager.h"
#include "common_event_support.h"
#include "sync_rule/battery_status.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {

BatteryStatusSubscriber::BatteryStatusSubscriber(const EventFwk::CommonEventSubscribeInfo &subscribeInfo,
                                                 std::shared_ptr<BatteryStatusListener> listener)
    : EventFwk::CommonEventSubscriber(subscribeInfo), listener_(listener)
{
}

void BatteryStatusSubscriber::OnReceiveEvent(const EventFwk::CommonEventData &eventData)
{
    auto action = eventData.GetWant().GetAction();
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_OKAY) {
        LOGI("Battery status changed: BATTERY_STATUS_OKAY");
        listener_->OnStatusNormal();
        return;
    }
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_CHARGING) {
        LOGI("Charging status changed: charging");
        BatteryStatus::SetChargingStatus(true);
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_DISCHARGING) {
        BatteryStatus::SetChargingStatus(false);
        LOGI("Charging status changed: discharging");
    } else {
        LOGI("OnReceiveEvent action is invalid");
    }
}

BatteryStatusListener::BatteryStatusListener(std::shared_ptr<DataSyncManager> dataSyncManager)
{
    dataSyncManager_ = dataSyncManager;
}

BatteryStatusListener::~BatteryStatusListener()
{
    Stop();
}

void BatteryStatusListener::Start()
{
    /* subscribe Battery Okay Status and Charging status */
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_OKAY);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CHARGING);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_DISCHARGING);
    EventFwk::CommonEventSubscribeInfo info(matchingSkills);
    commonEventSubscriber_ = std::make_shared<BatteryStatusSubscriber>(info, shared_from_this());
    auto subRet = EventFwk::CommonEventManager::SubscribeCommonEvent(commonEventSubscriber_);
    LOGI("Subscriber end, SubscribeResult = %{public}d", subRet);
}

void BatteryStatusListener::Stop()
{
    if (commonEventSubscriber_ != nullptr) {
        EventFwk::CommonEventManager::UnSubscribeCommonEvent(commonEventSubscriber_);
        commonEventSubscriber_ = nullptr;
    }
}

void BatteryStatusListener::OnStatusNormal()
{
    dataSyncManager_->TriggerRecoverySync(triggerType_);
    LOGI("battery status OK");
}

void BatteryStatusListener::OnStatusAbnormal() {}
} // namespace OHOS::FileManagement::CloudSync