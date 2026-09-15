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


#include "cloud_disk_progress_callback_client.h"

#include <algorithm>

namespace OHOS::FileManagement::CloudDiskService {
bool CloudDiskProgressCallbackClient::Add(const sptr<ICloudDiskProgressCallback> &callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (callback == nullptr || std::find(callbacks_.begin(), callbacks_.end(), callback) != callbacks_.end()) {
        return false;
    }
    callbacks_.push_back(callback);
    return true;
}

bool CloudDiskProgressCallbackClient::Remove(const sptr<ICloudDiskProgressCallback> &callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (callback == nullptr) {
        callbacks_.clear();
    } else {
        callbacks_.erase(std::remove(callbacks_.begin(), callbacks_.end(), callback), callbacks_.end());
    }
    return callbacks_.empty();
}

void CloudDiskProgressCallbackClient::OnProgress(const HydrateProgress &progress)
{
    std::vector<sptr<ICloudDiskProgressCallback>> callbacks;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        callbacks = callbacks_;
    }
    for (const auto &callback : callbacks) {
        callback->OnProgress(progress);
    }
}
} // namespace OHOS::FileManagement::CloudDiskService
