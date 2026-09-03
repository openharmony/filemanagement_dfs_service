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


#include "placeholder_progress_manager.h"

#include "cloud_disk_service_error.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudDiskService {
PlaceholderProgressManager &PlaceholderProgressManager::GetInstance()
{
    static PlaceholderProgressManager instance;
    return instance;
}

int32_t PlaceholderProgressManager::Register(const SubscriberKey &key, int32_t userId,
    const sptr<ICloudDiskProgressCallback> &callback)
{
    if (callback == nullptr || callback->AsObject() == nullptr || userId < 0) {
        return E_INVALID_ARG;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    auto found = subscribers_.find(key);
    if (found != subscribers_.end()) {
        return found->second->callback->AsObject() == callback->AsObject() && found->second->userId == userId ?
            E_OK : E_CALLBACK_ALREADY_REGISTERED;
    }
    auto subscriber = std::make_shared<Subscriber>();
    subscriber->userId = userId;
    subscriber->callback = callback;
    subscriber->deathRecipient = sptr(new SvcDeathRecipient(
        [this, key](const wptr<IRemoteObject> &remote) { OnRemoteDied(key, remote); }));
    auto remote = callback->AsObject();
    if (remote->IsProxyObject() && !remote->AddDeathRecipient(subscriber->deathRecipient)) {
        LOGE("Register progress death recipient failed");
        return E_IPC_FAILED;
    }
    subscribers_.emplace(key, std::move(subscriber));
    return E_OK;
}

int32_t PlaceholderProgressManager::Unregister(const SubscriberKey &key)
{
    std::shared_ptr<Subscriber> subscriber;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto found = subscribers_.find(key);
        if (found == subscribers_.end()) {
            return E_CALLBACK_NOT_REGISTERED;
        }
        subscriber = found->second;
        subscriber->active = false;
        subscribers_.erase(found);
    }
    auto remote = subscriber->callback->AsObject();
    if (remote->IsProxyObject()) {
        remote->RemoveDeathRecipient(subscriber->deathRecipient);
    }
    return E_OK;
}

void PlaceholderProgressManager::OnRemoteDied(const SubscriberKey &key, const wptr<IRemoteObject> &remote)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto found = subscribers_.find(key);
    if (found != subscribers_.end() && wptr<IRemoteObject>(found->second->callback->AsObject()) == remote) {
        found->second->active = false;
        subscribers_.erase(found);
    }
}

void PlaceholderProgressManager::OnTaskProgress(const std::vector<uint8_t> &reqKey, int32_t userId,
    const HydrateProgress &progress)
{
    constexpr auto MIN_INTERVAL = std::chrono::milliseconds(500);
    bool terminal = progress.state == static_cast<int32_t>(HydrateProgressState::COMPLETED) ||
        progress.state == static_cast<int32_t>(HydrateProgressState::CANCELLED);
    std::vector<std::shared_ptr<Subscriber>> recipients;
    std::lock_guard<std::mutex> lock(mutex_);
    auto now = std::chrono::steady_clock::now();
    auto previous = lastProgress_.find(reqKey);
    if (!terminal && previous != lastProgress_.end() && previous->second.state == progress.state &&
        now - previous->second.time < MIN_INTERVAL) {
        return;
    }
    if (terminal) {
        lastProgress_.erase(reqKey);
    } else {
        lastProgress_.insert_or_assign(reqKey, LastProgress{progress.state, now});
    }
    for (const auto &[key, subscriber] : subscribers_) {
        (void)key;
        if (subscriber->userId == userId) {
            recipients.push_back(subscriber);
        }
    }
    // A serial queue preserves state order. No Binder call runs under task/subscriber locks.
    queue_.submit([recipients = std::move(recipients), progress] {
        for (const auto &subscriber : recipients) {
            if (subscriber->active) {
                subscriber->callback->OnProgress(progress);
            }
        }
    });
}

void PlaceholderProgressManager::Drain()
{
    auto barrier = queue_.submit_h([] {});
    queue_.wait(barrier);
}

void PlaceholderProgressManager::Clear()
{
    std::map<SubscriberKey, std::shared_ptr<Subscriber>> subscribers;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto &[key, subscriber] : subscribers_) {
            (void)key;
            subscriber->active = false;
        }
        subscribers.swap(subscribers_);
        lastProgress_.clear();
    }
    for (const auto &[key, subscriber] : subscribers) {
        (void)key;
        auto remote = subscriber->callback->AsObject();
        if (remote->IsProxyObject()) {
            remote->RemoveDeathRecipient(subscriber->deathRecipient);
        }
    }
}
} // namespace OHOS::FileManagement::CloudDiskService
