/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "register_callback_manager_napi.h"

#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
using namespace std;
napi_status RegisterCallbackManagerNapi::RegisterCallback(napi_value callback)
{
    if (callback == nullptr) {
        return napi_invalid_arg;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackMtx_);
    if (IsExisted(callback)) {
        return napi_ok;
    }
    napi_ref ref;
    napi_status status = napi_create_reference(env_, callback, 1, &ref);
    if (status != napi_ok) {
        LOGE("Failed to create reference: %{public}d", status);
        return status;
    }

    callbackList_.push_back(std::make_pair(true, ref));
    validRefNum_++;
    LOGI("After register, callback list size: %{public}zu, validRefNum_=%{public}d", callbackList_.size(),
         validRefNum_.load());
    return napi_ok;
}

napi_status RegisterCallbackManagerNapi::UnregisterCallback(napi_value callback)
{
    if (validRefNum_.load() == 0) {
        return napi_ok;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackMtx_);
    if (callback == nullptr) {
        for (auto &iter : callbackList_) {
            iter.first = false;
        }
        validRefNum_ = 0;
        LOGI("After unregister all, callback list size: %{public}zu, validRefNum_=%{public}d", callbackList_.size(),
             validRefNum_.load());
        return napi_ok;
    }
    bool isFound = false;
    napi_value val = nullptr;
    for (auto &iter : callbackList_) {
        napi_status status = napi_get_reference_value(env_, iter.second, &val);
        if (status != napi_ok) {
            LOGE("Failed to get reference: %{public}d", status);
            continue;
        }
        status = napi_strict_equals(env_, callback, val, &isFound);
        if (status != napi_ok) {
            LOGE("Failed to strict compare: %{public}d", status);
            continue;
        }
        if (isFound) {
            if (iter.first) {
                iter.first = false;
                validRefNum_--;
            }
            break;
        }
    }
    LOGI("After unregister, callback list size: %{public}zu, validRefNum_=%{public}d", callbackList_.size(),
         validRefNum_.load());
    return napi_ok;
}

// No need to lock
bool RegisterCallbackManagerNapi::IsExisted(napi_value callback)
{
    bool isExisted = false;
    napi_value val = nullptr;
    for (auto &iter : callbackList_) {
        napi_status status = napi_get_reference_value(env_, iter.second, &val);
        if (status != napi_ok) {
            LOGE("Failed to get reference: %{public}d", status);
            continue;
        }
        status = napi_strict_equals(env_, callback, val, &isExisted);
        if (status != napi_ok) {
            LOGE("Failed to strict compare: %{public}d", status);
            continue;
        }
        if (isExisted) {
            return iter.first;
        }
    }

    return false;
}

// Running in JS thread
void RegisterCallbackManagerNapi::OnJsCallback(napi_value *value, uint32_t argc)
{
    std::lock_guard<std::recursive_mutex> lock(callbackMtx_);
    for (auto iter = callbackList_.begin(); iter != callbackList_.end();) {
        if (!iter->first) {
            napi_delete_reference(env_, iter->second);
            iter = callbackList_.erase(iter);
            continue;
        }
        napi_value jsCallback = nullptr;
        napi_status status = napi_get_reference_value(env_, iter->second, &jsCallback);
        if (status != napi_ok) {
            LOGE("Failed to get reference: %{public}d", status);
            continue;
        }
        napi_value jsResult = nullptr;
        status = napi_call_function(env_, nullptr, jsCallback, argc, value, &jsResult);
        if (status != napi_ok) {
            LOGE("Failed to call function: %{public}d", status);
        }
        iter++;
    }
}

RegisterCallbackManagerNapi::~RegisterCallbackManagerNapi()
{
    CleanAllCallback(true);
}

void RegisterCallbackManagerNapi::CleanAllCallback(bool force)
{
    std::lock_guard<std::recursive_mutex> lock(callbackMtx_);
    if (!force) {
        if (validRefNum_.load() != 0) {
            return;
        }
    }

    for (auto &iter : callbackList_) {
        napi_delete_reference(env_, iter.second);
    }
    callbackList_.clear();
    validRefNum_ = 0;
    LOGI("Clean all callback, valid callback count is 0");
}
} // namespace OHOS::FileManagement::CloudSync