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


#ifndef OHOS_FILEMGMT_PLACEHOLDER_PROGRESS_MANAGER_H
#define OHOS_FILEMGMT_PLACEHOLDER_PROGRESS_MANAGER_H

#include <atomic>
#include <chrono>
#include <map>
#include <memory>
#include <mutex>
#include <utility>

#include "ffrt.h"
#include "i_cloud_disk_progress_callback.h"
#include "nocopyable.h"
#include "svc_death_recipient.h"

namespace OHOS::FileManagement::CloudDiskService {
class PlaceholderProgressManager final : public NoCopyable {
public:
    using SubscriberKey = std::pair<uint64_t, int32_t>;
    static PlaceholderProgressManager &GetInstance();
    int32_t Register(const SubscriberKey &key, int32_t userId, const sptr<ICloudDiskProgressCallback> &callback);
    int32_t Unregister(const SubscriberKey &key);
    void OnRemoteDied(const SubscriberKey &key, const wptr<IRemoteObject> &remote);
    void OnTaskProgress(const std::vector<uint8_t> &reqKey, int32_t userId, const HydrateProgress &progress);
    void Drain();
    void Clear();

private:
    struct Subscriber {
        int32_t userId = -1;
        sptr<ICloudDiskProgressCallback> callback;
        sptr<SvcDeathRecipient> deathRecipient;
        std::atomic<bool> active{true};
    };
    struct LastProgress {
        int32_t state;
        std::chrono::steady_clock::time_point time;
    };

    std::mutex mutex_;
    std::map<SubscriberKey, std::shared_ptr<Subscriber>> subscribers_;
    std::map<std::vector<uint8_t>, LastProgress> lastProgress_;
    ffrt::queue queue_{"clouddisk_progress"};
};
} // namespace OHOS::FileManagement::CloudDiskService

#endif // OHOS_FILEMGMT_PLACEHOLDER_PROGRESS_MANAGER_H
