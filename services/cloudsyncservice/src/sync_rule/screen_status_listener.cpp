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

#include "sync_rule/screen_status_listener.h"

#include "common_event_manager.h"
#include "common_event_support.h"
#include "sync_rule/screen_status.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
ScreenStatusSubscriber::ScreenStatusSubscriber(const EventFwk::CommonEventSubscribeInfo &subscribeInfo,
                                               std::shared_ptr<ScreenStatusListener> listener)
    : EventFwk::CommonEventSubscriber(subscribeInfo), listener_(listener)
{
}
void ScreenStatusSubscriber::OnReceiveEvent(const EventFwk::CommonEventData &eventData)
{
    auto action = eventData.GetWant().GetAction();
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON) {
        LOGI("Screen On!");
        ScreenStatus::SetScreenState(ScreenStatus::ScreenState::SCREEN_ON);
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF) {
        LOGI("Screen Off!");
        ScreenStatus::SetScreenState(ScreenStatus::ScreenState::SCREEN_OFF);
        listener_->ScreenOff();
    }
}

ScreenStatusListener::ScreenStatusListener(std::shared_ptr<DataSyncManager> dataSyncManager)
{
    dataSyncManager_ = dataSyncManager;
}

ScreenStatusListener::~ScreenStatusListener()
{
    Stop();
}

void ScreenStatusListener::ScreenOff()
{
    dataSyncManager_->DownloadThumb();
}

void ScreenStatusListener::Start()
{
    /* subscribe Battery Okay Status and Charging status */
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF);
    EventFwk::CommonEventSubscribeInfo info(matchingSkills);
    commonEventSubscriber_ = std::make_shared<ScreenStatusSubscriber>(info, shared_from_this());
    auto subRet = EventFwk::CommonEventManager::SubscribeCommonEvent(commonEventSubscriber_);
    LOGI("Subscriber end, SubscribeResult = %{public}d", subRet);
}

void ScreenStatusListener::Stop()
{
    if (commonEventSubscriber_ != nullptr) {
        EventFwk::CommonEventManager::UnSubscribeCommonEvent(commonEventSubscriber_);
        commonEventSubscriber_ = nullptr;
    }
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS