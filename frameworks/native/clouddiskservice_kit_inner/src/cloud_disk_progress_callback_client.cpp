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

#include "cloud_disk_service_error.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudDiskService {
int32_t CloudDiskProgressCallbackClient::Add(uint64_t accessorId,
    const sptr<ICloudDiskProgressCallback> &callback, bool &added)
{
    added = false;
    if (callback == nullptr) {
        LOGE("Add accessor progress callback failed: callback is null");
        return E_INVALID_ARG;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    auto entry = accessorCallbacks_.find(accessorId);
    if (entry != accessorCallbacks_.end()) {
        return entry->second == callback ? E_OK : E_CALLBACK_ALREADY_REGISTERED;
    }
    accessorCallbacks_.emplace(accessorId, callback);
    added = true;
    return E_OK;
}

bool CloudDiskProgressCallbackClient::Remove(uint64_t accessorId,
    const sptr<ICloudDiskProgressCallback> &callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto entry = accessorCallbacks_.find(accessorId);
    if (entry == accessorCallbacks_.end() || entry->second != callback) {
        return false;
    }
    accessorCallbacks_.erase(entry);
    return true;
}

void CloudDiskProgressCallbackClient::OnProgress(const HydrateProgress &progress)
{
    sptr<ICloudDiskProgressCallback> callback;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto entry = accessorCallbacks_.find(progress.accessorId);
        if (entry != accessorCallbacks_.end()) {
            callback = entry->second;
        }
    }
    if (callback != nullptr) {
        callback->OnProgress(progress);
    }
}
} // namespace OHOS::FileManagement::CloudDiskService
