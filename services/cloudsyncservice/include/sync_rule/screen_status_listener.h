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

#ifndef OHOS_FILEMGMT_SCREEN_STATUS_LISTENER_H
#define OHOS_FILEMGMT_SCREEN_STATUS_LISTENER_H

#include "common_event_subscriber.h"
#include "data_sync_manager.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
class ScreenStatusListener : public std::enable_shared_from_this<ScreenStatusListener> {
public:
    explicit ScreenStatusListener(std::shared_ptr<CloudFile::DataSyncManager> dataSyncManager);
    ~ScreenStatusListener();
    void Start();
    void Stop();
    void ScreenOff();

private:
    std::shared_ptr<CloudFile::DataSyncManager> dataSyncManager_;
    std::shared_ptr<EventFwk::CommonEventSubscriber> commonEventSubscriber_ = nullptr;
};

class ScreenStatusSubscriber : public EventFwk::CommonEventSubscriber {
public:
    ScreenStatusSubscriber(const EventFwk::CommonEventSubscribeInfo &subscribeInfo,
                           std::shared_ptr<ScreenStatusListener> listener);
    ~ScreenStatusSubscriber() override {}
    void OnReceiveEvent(const EventFwk::CommonEventData &eventData) override;
private:
    std::shared_ptr<ScreenStatusListener> listener_;
};
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS

#endif // OHOS_FILEMGMT_SCREEN_STATUS_LISTENER_H