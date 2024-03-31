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

#include "clouddisk_sync_helper.h"

#include "utils_log.h"
#include "dfs_error.h"
#include "common_timer_errors.h"
namespace OHOS {
namespace FileManagement {
namespace CloudDisk {
using namespace std;
constexpr int32_t MIN_USER_ID = 100;

CloudDiskSyncHelper& CloudDiskSyncHelper::GetInstance()
{
    static CloudDiskSyncHelper instance_;
    return instance_;
}

CloudDiskSyncHelper::CloudDiskSyncHelper() : timer_(std::make_unique<Utils::Timer>("CloudDiskTriggerSync"))
{
    timer_->Setup();
}

CloudDiskSyncHelper::~CloudDiskSyncHelper()
{
    if (timer_) {
        timer_->Shutdown(true);
        timer_ = nullptr;
    }
}

void CloudDiskSyncHelper::RegisterTriggerSync(const std::string &bundleName, const int32_t &userId)
{
    if (timer_ == nullptr || bundleName.empty() || userId < MIN_USER_ID) {
        LOGE("TriggerSync parameter is invalid");
        return;
    }
    LOGD("begin trigger sync, bundleName = %{public}s, userId = %{public}d", bundleName.c_str(), userId);
    UnregisterRepeatingTriggerSync(bundleName, userId);
    string keyId = to_string(userId) + bundleName;
    function<void()> callback = [this, bundleName, userId] { OnTriggerSyncCallback(bundleName, userId); };
    uint32_t timerId = timer_->Register(callback, SYNC_INTERVAL, true);
    if (timerId == Utils::TIMER_ERR_DEAL_FAILED) {
        LOGE("Register timer failed");
        return;
    }
    shared_ptr<TriggerInfo> triggerInfoPtr = make_shared<TriggerInfo>();
    triggerInfoPtr->timerId = timerId;
    triggerInfoPtr->callback = callback;
    lock_guard<mutex> lock(registerMutex_);
    triggerInfoMap_.emplace(keyId, triggerInfoPtr);
}

void CloudDiskSyncHelper::UnregisterRepeatingTriggerSync(const std::string &bundleName, const int32_t &userId)
{
    if (timer_ == nullptr || bundleName.empty() || userId < MIN_USER_ID) {
        LOGE("UnregisterRepeatingTrigger parameter is invalid");
        return;
    }
    string keyId = to_string(userId) + bundleName;
    bool isSuccess = false;
    lock_guard<mutex> lock(unregisterMutex_);
    auto iterator = triggerInfoMap_.find(keyId);
    if (iterator != triggerInfoMap_.end()) {
        LOGD("bundleName: %{public}s, userId: %{public}d is exist", bundleName.c_str(), userId);
        auto triggerInfoPtr = iterator->second;
        timer_->Unregister(triggerInfoPtr->timerId);
        triggerInfoMap_.erase(keyId);
        isSuccess = true;
    }
    LOGD("Unregister repeating trigger result is %{public}d", isSuccess);
}

void CloudDiskSyncHelper::OnTriggerSyncCallback(const std::string &bundleName, const int32_t &userId)
{
    string keyId = to_string(userId) + bundleName;
    {
        lock_guard<mutex> lock(callbackMutex_);
        triggerInfoMap_.erase(keyId);
    }
    LOGI("cloud sync manager trigger sync, bundleName: %{public}s, userId: %{public}d", bundleName.c_str(), userId);
    int32_t ret = CloudSync::CloudSyncManagerLite::GetInstance().TriggerSync(bundleName, userId);
    if (ret != 0) {
        LOGE("cloud sync manager trigger sync err %{public}d", ret);
    }
}
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS