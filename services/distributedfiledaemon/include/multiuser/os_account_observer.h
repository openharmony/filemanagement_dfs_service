/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#ifndef ACCOUNT_CHANGE_OBSERVER_H
#define ACCOUNT_CHANGE_OBSERVER_H

#include <memory>
#include <mutex>
#include <vector>

#include "common_event_subscribe_info.h"
#include "common_event_subscriber.h"
#include "mountpoint/mount_point.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
static constexpr int MOUNT_POINT_NUM = 2;
class OsAccountObserver final : public EventFwk::CommonEventSubscriber {
public:
    OsAccountObserver() = default;
    ~OsAccountObserver();
    explicit OsAccountObserver(const EventFwk::CommonEventSubscribeInfo &subscribeInfo);

    virtual void OnReceiveEvent(const EventFwk::CommonEventData &eventData) override;
private:
    void RemoveMPInfo(const int id);
    void AddMPInfo(const int id, const std::string &relativePath);
    int32_t GetCurrentUserId();

    std::mutex serializer_;
    std::unordered_map<int, std::vector<std::shared_ptr<MountPoint>>> mountPoints_;
    int curUsrId_ { -1 };
    std::atomic_int32_t needAddUserId_ { -1 };
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // ACCOUNT_CHANGE_OBSERVER_H