/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "dfs_error.h"
#include "utils_log.h"
#include "cloud_sync_callback_client_manager.h"
#include "cloud_sync_common.h"

namespace OHOS::FileManagement::CloudSync {
CloudSyncCallbackClientManager &CloudSyncCallbackClientManager::GetInstance()
{
    static CloudSyncCallbackClientManager instance;
    return instance;
}

int32_t CloudSyncCallbackClientManager::AddCallback(const CallbackInfo &callbackInfo)
{
    if ((callbackInfo.callbackId.empty()) || (callbackInfo.callback == nullptr)) {
        LOGE("Id or callback empty, callbackId:%{public}s, bundleName:%{public}s.",
            callbackInfo.callbackId.c_str(), callbackInfo.bundleName.c_str());
        return E_INVAL_ARG;
    }

    std::lock_guard<std::mutex> lock(callbackListMutex_);
    auto it = std::find_if(callbackList_.begin(), callbackList_.end(),
        [&callbackInfo](const CallbackInfo &info) {
            return info.callbackId == callbackInfo.callbackId;
        });
    if (it != callbackList_.end()) {
        LOGE("Id same, callbackId:%{public}s, bundleName:%{public}s.",
            callbackInfo.callbackId.c_str(), callbackInfo.bundleName.c_str());
        return E_INVAL_ARG;
    }

    LOGI("Add callback, callbackId:%{public}s, bundleName:%{public}s.",
        callbackInfo.callbackId.c_str(), callbackInfo.bundleName.c_str());
    callbackList_.push_back(callbackInfo);
    return E_OK;
}

int32_t CloudSyncCallbackClientManager::RemoveCallback(const CallbackInfo &callbackInfo)
{
    if (callbackInfo.callbackId.empty()) {
        LOGE("Id empty, callbackId:%{public}s, bundleName:%{public}s.",
            callbackInfo.callbackId.c_str(), callbackInfo.bundleName.c_str());
        return E_INVAL_ARG;
    }

    std::lock_guard<std::mutex> lock(callbackListMutex_);
    auto it = std::find_if(callbackList_.begin(), callbackList_.end(),
        [&callbackInfo](const CallbackInfo &info) {
            return info.callbackId == callbackInfo.callbackId;
        });
    if (it == callbackList_.end()) {
        LOGE("Not find callbackId:%{public}s, bundleName:%{public}s.",
            callbackInfo.callbackId.c_str(), callbackInfo.bundleName.c_str());
        return E_INVAL_ARG;
    }

    LOGI("Remove callback, callbackId:%{public}s, bundleName:%{public}s.",
        it->callbackId.c_str(), it->bundleName.c_str());
    callbackList_.erase(it);
    return E_OK;
}

int32_t CloudSyncCallbackClientManager::GetAllCallback(std::vector<CallbackInfo> &callbackInfo)
{
    std::lock_guard<std::mutex> lock(callbackListMutex_);
    callbackInfo = callbackList_;
    return E_OK;
}

bool CloudSyncCallbackClientManager::IsEmpty()
{
    std::lock_guard<std::mutex> lock(callbackListMutex_);
    return callbackList_.empty();
}
} // namespace OHOS::FileManagement::CloudSync