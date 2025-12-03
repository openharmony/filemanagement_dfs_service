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
#include "cloud_disk_service_callback_manager.h"

#include <cinttypes>

#include "cloud_disk_service_syncfolder.h"
#include "cloud_disk_sync_folder.h"
#include "cloud_disk_service_access_token.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudDiskService {
using namespace std;

constexpr const char *FILE_SYNC_STATE = "user.clouddisk.filesyncstate";

CloudDiskServiceCallbackManager &CloudDiskServiceCallbackManager::GetInstance()
{
    static CloudDiskServiceCallbackManager instance;
    return instance;
}

void CloudDiskServiceCallbackManager::AddCallback(const std::string &bundleName,
                                                  const sptr<ICloudDiskServiceCallback> &callback)
{
    unique_lock<mutex> lock(callbackMutex_);
    if (callbackAppMap_[bundleName].callback != nullptr) {
        return;
    }
    if (callback == nullptr) {
        LOGE("callback is nullptr");
        return;
    }
    callbackAppMap_[bundleName].callback = callback;
    auto remoteObject = callback->AsObject();
    auto deathCb = [this, bundleName](const wptr<IRemoteObject> &obj) {
        LOGE("client died");
        unique_lock<mutex> lock(callbackMutex_, defer_lock);
        std::vector<uint32_t> syncFolderIndex;
        lock.lock();
        auto it = callbackAppMap_.find(bundleName);
        if (it != callbackAppMap_.end()) {
            syncFolderIndex = it->second.syncFolderIndexs;
            callbackAppMap_.erase(it);
        }
        lock.unlock();
        int32_t userId = CloudDiskServiceAccessToken::GetUserId();
        if (userId == 0) {
            CloudDiskServiceAccessToken::GetAccountId(userId);
        }
        for (auto item : syncFolderIndex) {
            lock.lock();
            callbackIndexMap_.erase(item);
            lock.unlock();
            CloudDiskServiceSyncFolder::UnRegisterSyncFolderChanges(userId, item);
        }
    };
    auto death = sptr(new SvcDeathRecipient(deathCb));
    remoteObject->AddDeathRecipient(death);
}

void CloudDiskServiceCallbackManager::OnChangeData(const uint32_t syncFolderIndex,
                                                   const std::vector<ChangeData> &changeData)
{
    unique_lock<mutex> lock(callbackMutex_);
    auto item = callbackIndexMap_.find(syncFolderIndex);
    if (item == callbackIndexMap_.end() || !item->second) {
        LOGE("callback is nullptr");
        return;
    }
    std::string syncFolderSync;
    if (!CloudDiskSyncFolder::GetInstance().GetSyncFolderByIndex(item->first, syncFolderSync)) {
        LOGE("Get syncFolder failed");
        return;
    }
    int32_t userId = CloudDiskServiceAccessToken::GetUserId();
    if (userId == 0) {
        CloudDiskServiceAccessToken::GetAccountId(userId);
    }
    std::string sandboxPath;
    if (!CloudDiskSyncFolder::GetInstance().PathToSandboxPathByPhysicalPath(syncFolderSync, std::to_string(userId),
                                                                            sandboxPath)) {
        LOGE("Get path failed");
        return;
    }
    item->second->OnChangeData(sandboxPath, changeData);
}

bool CloudDiskServiceCallbackManager::RegisterSyncFolderMap(std::string &bundleName,
                                                            uint32_t syncFolderIndex,
                                                            const sptr<ICloudDiskServiceCallback> &callback)
{
    unique_lock<mutex> lock(callbackMutex_);
    auto item = callbackIndexMap_.find(syncFolderIndex);
    if (item != callbackIndexMap_.end()) {
        LOGE("Register for this syncFolder is exist");
        return false;
    }
    callbackIndexMap_[syncFolderIndex] = callback;

    auto it = callbackAppMap_.find(bundleName);
    if (it == callbackAppMap_.end()) {
        CallbackValue callbackValue;
        callbackValue.syncFolderIndexs.push_back(syncFolderIndex);
        callbackAppMap_[bundleName] = callbackValue;
    } else {
        it->second.syncFolderIndexs.push_back(syncFolderIndex);
    }
    return true;
}

void CloudDiskServiceCallbackManager::UnregisterSyncFolderMap(const std::string &bundleName, uint32_t syncFolderIndex)
{
    unique_lock<mutex> lock(callbackMutex_);
    auto it = callbackIndexMap_.find(syncFolderIndex);
    if (it != callbackIndexMap_.end()) {
        callbackIndexMap_.erase(it);
    }

    auto item = callbackAppMap_.find(bundleName);
    if (item != callbackAppMap_.end()) {
        auto &vec = item->second.syncFolderIndexs;
        auto pos = std::find(vec.begin(), vec.end(), syncFolderIndex);
        if (pos != vec.end()) {
            vec.erase(pos);
        }
    }
}

bool CloudDiskServiceCallbackManager::UnregisterSyncFolderForChangesMap(std::string &bundleName,
                                                                        uint32_t syncFolderIndex)
{
    unique_lock<mutex> lock(callbackMutex_);
    auto it = callbackIndexMap_.find(syncFolderIndex);
    if (it == callbackIndexMap_.end()) {
        LOGE("No register for this syncFolder");
        return false;
    }
    callbackIndexMap_.erase(it);

    auto item = callbackAppMap_.find(bundleName);
    if (item == callbackAppMap_.end()) {
        LOGE("No such app in callback");
        return false;
    }
    auto &vec = item->second.syncFolderIndexs;
    auto pos = std::find(vec.begin(), vec.end(), syncFolderIndex);
    if (pos != vec.end()) {
        vec.erase(pos);
    }
    return true;
}

void CloudDiskServiceCallbackManager::ClearMap()
{
    unique_lock<mutex> lock(callbackMutex_);
    callbackAppMap_.clear();
    callbackIndexMap_.clear();
}

} // namespace OHOS::FileManagement::CloudDiskService