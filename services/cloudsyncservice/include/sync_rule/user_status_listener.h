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

#ifndef USER_STATUS_LISTENER_H
#define USER_STATUS_LISTENER_H

#include <mutex>
#include <vector>

#include "common_event_subscriber.h"
#include "i_user_status_observer.h"

namespace OHOS::FileManagement::CloudSync {
class UserStatusListener : public std::enable_shared_from_this<UserStatusListener> {
public:
    UserStatusListener() = default;
    ~UserStatusListener();
    void Start();
    void Stop();
    void AddObserver(std::shared_ptr<IUserStatusObserver> observer);
    void NotifyUserUnlocked();

private:
    std::shared_ptr<EventFwk::CommonEventSubscriber> commonEventSubscriber_ = nullptr;
    std::mutex obsVecMutex_;
    std::vector<std::shared_ptr<IUserStatusObserver>> observers_;
};

class UserStatusSubscriber : public EventFwk::CommonEventSubscriber {
public:
    UserStatusSubscriber(const EventFwk::CommonEventSubscribeInfo &subscribeInfo,
                            std::shared_ptr<UserStatusListener> listener);
    ~UserStatusSubscriber() override {}
    void OnReceiveEvent(const EventFwk::CommonEventData &eventData) override;

private:
    std::shared_ptr<UserStatusListener> listener_;
};
} // namespace OHOS::FileManagement::CloudSync
#endif // USER_STATUS_LISTENER_H
