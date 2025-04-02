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

#include "sync_rule/user_status_listener.h"

#include "common_event_manager.h"
#include "common_event_support.h"
#include "iservice_registry.h"
#include "parameters.h"
#include "system_ability_definition.h"
#include "task_state_manager.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {

UserStatusSubscriber::UserStatusSubscriber(const EventFwk::CommonEventSubscribeInfo &subscribeInfo,
    std::shared_ptr<UserStatusListener> listener) : EventFwk::CommonEventSubscriber(subscribeInfo), listener_(listener)
{
}

void UserStatusSubscriber::OnReceiveEvent(const EventFwk::CommonEventData &eventData)
{
    auto action = eventData.GetWant().GetAction();
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_USER_UNLOCKED) {
        LOGI("user unlocked");
        listener_->NotifyUserUnlocked();
        return;
    }
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_HWID_LOGOUT) {
        LOGI("account logout");
        listener_->DoCleanVideoCache();
        return;
    }
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_USER_STOPPING) {
        LOGI("account stopping");
        listener_->DoUnloadSA();
        return;
    }
}

UserStatusListener::UserStatusListener(std::shared_ptr<CloudFile::DataSyncManager> dataSyncManager)
{
    dataSyncManager_ = dataSyncManager;
}

UserStatusListener::~UserStatusListener()
{
    Stop();
}

void UserStatusListener::AddObserver(std::shared_ptr<IUserStatusObserver> observer)
{
    std::lock_guard<std::mutex> lck(obsVecMutex_);
    observers_.push_back(observer);
}

void UserStatusListener::NotifyUserUnlocked()
{
    std::lock_guard<std::mutex> lck(obsVecMutex_);
    for (auto &observer : observers_) {
        observer->OnUserUnlocked();
    }
}

void UserStatusListener::Start()
{
    /* subscribe user unlock event */
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_USER_UNLOCKED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_HWID_LOGOUT);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_USER_STOPPING);
    EventFwk::CommonEventSubscribeInfo info(matchingSkills);
    commonEventSubscriber_ = std::make_shared<UserStatusSubscriber>(info, shared_from_this());
    auto subRet = EventFwk::CommonEventManager::SubscribeCommonEvent(commonEventSubscriber_);
    LOGI("Subscriber end, SubscribeResult = %{public}d", subRet);
}

void UserStatusListener::Stop()
{
    if (commonEventSubscriber_ != nullptr) {
        EventFwk::CommonEventManager::UnSubscribeCommonEvent(commonEventSubscriber_);
        commonEventSubscriber_ = nullptr;
    }
}

void UserStatusListener::DoCleanVideoCache()
{
    dataSyncManager_->CleanVideoCache();
}

void UserStatusListener::DoUnloadSA()
{
    auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        LOGE("get samgr failed");
        return;
    }
    system::SetParameter(CLOUD_FILE_SERVICE_SA_STATUS_FLAG, CLOUD_FILE_SERVICE_SA_END);
    int32_t ret = samgrProxy->UnloadSystemAbility(FILEMANAGEMENT_CLOUD_SYNC_SERVICE_SA_ID);
    if (ret != ERR_OK) {
        LOGE("remove system ability failed");
        return;
    }
    LOGI("unload cloudfileservice end");
}
} // namespace OHOS::FileManagement::CloudSync