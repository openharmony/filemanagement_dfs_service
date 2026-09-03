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

#include "placeholder_callback_manager.h"

#include <algorithm>

#include "cloud_disk_service_error.h"
#include "placeholder_task_manager.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudDiskService {
PlaceholderCallbackManager &PlaceholderCallbackManager::GetInstance()
{
    static PlaceholderCallbackManager instance;
    return instance;
}

int32_t PlaceholderCallbackManager::RegisterCallbackTable(const std::string &bundleName,
                                                          uint32_t syncFolderIndex,
                                                          const sptr<ICloudDiskServiceCallbackTable> &callback)
{
    if (bundleName.empty() || callback == nullptr || callback->AsObject() == nullptr) {
        LOGE("Invalid callback table arguments");
        return E_INVALID_ARG;
    }

    std::lock_guard<std::mutex> lock(callbackMutex_);
    CallbackKey key{bundleName, syncFolderIndex};
    if (callbackMap_.find(key) != callbackMap_.end()) {
        LOGE("Callback table is already registered");
        return E_CALLBACK_ALREADY_REGISTERED;
    }

    AddDeathRecipientLocked(callback);
    const void *remoteKey = callback->AsObject().GetRefPtr();
    callbackMap_[key] = callback;
    remoteCallbackMap_[remoteKey].push_back(key);
    return E_OK;
}

int32_t PlaceholderCallbackManager::UnregisterCallbackTable(const std::string &bundleName, uint32_t syncFolderIndex)
{
    if (bundleName.empty()) {
        LOGE("Invalid callback table arguments");
        return E_INVALID_ARG;
    }

    std::lock_guard<std::mutex> lock(callbackMutex_);
    CallbackKey key{bundleName, syncFolderIndex};
    auto callback = callbackMap_.find(key);
    if (callback == callbackMap_.end()) {
        LOGE("Callback table is not registered");
        return E_CALLBACK_NOT_REGISTERED;
    }
    sptr<ICloudDiskServiceCallbackTable> callbackObject = callback->second;
    callbackMap_.erase(callback);
    RemoveRemoteKeyLocked(callbackObject, key);
    return E_OK;
}

sptr<ICloudDiskServiceCallbackTable> PlaceholderCallbackManager::GetCallback(const std::string &bundleName,
                                                                             uint32_t syncFolderIndex)
{
    std::lock_guard<std::mutex> lock(callbackMutex_);
    auto callback = callbackMap_.find(CallbackKey{bundleName, syncFolderIndex});
    return callback == callbackMap_.end() ? nullptr : callback->second;
}

bool PlaceholderCallbackManager::IsCallbackRegistered(const std::string &bundleName, uint32_t syncFolderIndex)
{
    return GetCallback(bundleName, syncFolderIndex) != nullptr;
}

int32_t PlaceholderCallbackManager::DispatchFetchData(const std::string &bundleName,
                                                      uint32_t syncFolderIndex,
                                                      CloudDiskCallbackReqHead &reqHead,
                                                      CloudDiskPathInfo &filePath,
                                                      CloudDiskHydratePriority priority)
{
    reqHead.callbackType = CloudDiskCallbackType::FETCH_DATA;
    CloudDiskCallbackContext reqContext{};
    CloudDiskFetchDataRequest request{filePath, priority};
    reqContext.fetchData = &request;
    return DispatchCallback(bundleName, syncFolderIndex, reqHead, reqContext);
}

int32_t PlaceholderCallbackManager::DispatchCancelFetchData(const std::string &bundleName,
                                                            uint32_t syncFolderIndex,
                                                            CloudDiskCallbackReqHead &reqHead,
                                                            CloudDiskPathInfo &filePath)
{
    reqHead.callbackType = CloudDiskCallbackType::CANCEL_FETCH_DATA;
    CloudDiskCallbackContext reqContext{};
    reqContext.cancelFetchData = &filePath;
    return DispatchCallback(bundleName, syncFolderIndex, reqHead, reqContext);
}

int32_t PlaceholderCallbackManager::DispatchFetchRangeData(const std::string &bundleName,
                                                           uint32_t syncFolderIndex,
                                                           CloudDiskCallbackReqHead &reqHead,
                                                           CloudDiskRangeInfo &rangeInfo)
{
    reqHead.callbackType = CloudDiskCallbackType::FETCH_RANGE_DATA;
    CloudDiskCallbackContext reqContext{};
    reqContext.fetchRangeData = &rangeInfo;
    return DispatchCallback(bundleName, syncFolderIndex, reqHead, reqContext);
}

int32_t PlaceholderCallbackManager::DispatchDehydrate(const std::string &bundleName,
                                                      uint32_t syncFolderIndex,
                                                      CloudDiskCallbackReqHead &reqHead,
                                                      CloudDiskPathInfo &filePath)
{
    reqHead.callbackType = CloudDiskCallbackType::DEHYDRATE;
    CloudDiskDehydrateInfo dehydrateInfo{filePath, false};
    CloudDiskCallbackContext reqContext{};
    reqContext.dehydrateData = &dehydrateInfo;
    int32_t ret = DispatchCallback(bundleName, syncFolderIndex, reqHead, reqContext);
    if (ret != E_OK) {
        return ret;
    }
    return dehydrateInfo.allow ? E_OK : E_DEHYDRATE_DENIED;
}

int32_t PlaceholderCallbackManager::DispatchCallback(const std::string &bundleName,
                                                     uint32_t syncFolderIndex,
                                                     const CloudDiskCallbackReqHead &reqHead,
                                                     CloudDiskCallbackContext &reqContext)
{
    if (bundleName.empty()) {
        LOGE("Invalid dispatch callback arguments");
        return E_INVALID_ARG;
    }
    auto callback = GetCallback(bundleName, syncFolderIndex);
    if (callback == nullptr) {
        LOGE("Callback table is not registered");
        return E_CALLBACK_NOT_REGISTERED;
    }
    callback->OnCallback(reqHead, reqContext);
    return E_OK;
}

void PlaceholderCallbackManager::ClearBySyncFolder(const std::string &bundleName, uint32_t syncFolderIndex)
{
    std::lock_guard<std::mutex> lock(callbackMutex_);
    CallbackKey key{bundleName, syncFolderIndex};
    auto callback = callbackMap_.find(key);
    if (callback == callbackMap_.end()) {
        return;
    }
    sptr<ICloudDiskServiceCallbackTable> callbackObject = callback->second;
    callbackMap_.erase(callback);
    RemoveRemoteKeyLocked(callbackObject, key);
}

void PlaceholderCallbackManager::AddDeathRecipientLocked(const sptr<ICloudDiskServiceCallbackTable> &callback)
{
    auto remoteObject = callback->AsObject();
    const void *remoteKey = remoteObject.GetRefPtr();
    if (deathRecipientMap_.find(remoteKey) != deathRecipientMap_.end()) {
        return;
    }
    auto deathCallback = [this, remoteKey](const wptr<IRemoteObject> &object) {
        LOGE("Callback table client died");
        OnRemoteDied(remoteKey);
    };
    auto deathRecipient = sptr(new SvcDeathRecipient(deathCallback));
    if (!remoteObject->AddDeathRecipient(deathRecipient)) {
        LOGW("Failed to add callback table death recipient");
    }
    deathRecipientMap_[remoteKey] = deathRecipient;
}

void PlaceholderCallbackManager::RemoveRemoteKeyLocked(const sptr<ICloudDiskServiceCallbackTable> &callback,
                                                       const CallbackKey &key)
{
    if (callback == nullptr || callback->AsObject() == nullptr) {
        return;
    }
    auto remoteObject = callback->AsObject();
    const void *remoteKey = remoteObject.GetRefPtr();
    auto remoteCallbacks = remoteCallbackMap_.find(remoteKey);
    if (remoteCallbacks == remoteCallbackMap_.end()) {
        return;
    }
    auto &keys = remoteCallbacks->second;
    keys.erase(
        std::remove_if(keys.begin(), keys.end(), [&key](const CallbackKey &current) {
            return current.bundleName == key.bundleName &&
                current.syncFolderIndex == key.syncFolderIndex;
        }),
        keys.end());
    if (!keys.empty()) {
        return;
    }

    remoteCallbackMap_.erase(remoteCallbacks);
    auto deathRecipient = deathRecipientMap_.find(remoteKey);
    if (deathRecipient != deathRecipientMap_.end()) {
        remoteObject->RemoveDeathRecipient(deathRecipient->second);
        deathRecipientMap_.erase(deathRecipient);
    }
}

void PlaceholderCallbackManager::OnRemoteDied(const void *remoteKey)
{
    std::vector<CallbackKey> keys;
    {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        auto remoteCallbacks = remoteCallbackMap_.find(remoteKey);
        if (remoteCallbacks == remoteCallbackMap_.end()) {
            return;
        }
        keys = std::move(remoteCallbacks->second);
        for (const auto &key : keys) {
            callbackMap_.erase(key);
        }
        remoteCallbackMap_.erase(remoteCallbacks);
        deathRecipientMap_.erase(remoteKey);
    }
    // Do not hold the callback registry lock while waiting for an in-flight Execute to finish.
    for (const auto &key : keys) {
        PlaceholderTaskManager::GetInstance().CancelTasksBySyncFolder(key.bundleName, key.syncFolderIndex);
    }
}
} // namespace OHOS::FileManagement::CloudDiskService
