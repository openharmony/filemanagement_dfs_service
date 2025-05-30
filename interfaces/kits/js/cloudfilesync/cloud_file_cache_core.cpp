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

#include "cloud_file_cache_core.h"
#include "cloud_sync_manager.h"
#include "dfs_error.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
const int32_t E_PARAMS = 401;

bool RegisterManager::HasEvent(const string &eventType)
{
    unique_lock<mutex> registerMutex_;
    bool hasEvent = false;
    for (auto &iter : registerInfo_) {
        if (iter->eventType == eventType) {
            hasEvent = true;
            break;
        }
    }
    return hasEvent;
}

bool RegisterManager::AddRegisterInfo(shared_ptr<RegisterInfoArg> info)
{
    if (HasEvent(info->eventType)) {
        return false;
    }
    {
        unique_lock<mutex> registerMutex_;
        registerInfo_.insert(info);
    }
    return true;
}

bool RegisterManager::RemoveRegisterInfo(const string &eventType)
{
    unique_lock<mutex> registerMutex_;
    bool isFound = false;
    for (auto iter = registerInfo_.begin(); iter != registerInfo_.end();) {
        if ((*iter)->eventType == eventType) {
            isFound = true;
            iter = registerInfo_.erase(iter);
        } else {
            iter++;
        }
    }
    return isFound;
}

FsResult<CloudFileCacheCore *> CloudFileCacheCore::Constructor()
{
    CloudFileCacheCore *cloudFileCachePtr = new CloudFileCacheCore();
    if (cloudFileCachePtr == nullptr) {
        LOGE("Failed to create CloudFileCacheCore object on heap.");
        return FsResult<CloudFileCacheCore *>::Error(ENOMEM);
    }

    return FsResult<CloudFileCacheCore *>::Success(move(cloudFileCachePtr));
}

CloudFileCacheCore::CloudFileCacheCore()
{
    LOGI("Create fileCacheEntity");
    fileCacheEntity = make_unique<FileCacheEntity>();
}

FsResult<void> CloudFileCacheCore::DoOn(const string &event, const shared_ptr<CloudDownloadCallbackMiddle> callback)
{
    LOGI("On begin");
    if (event != PROGRESS && event != MULTI_PROGRESS) {
        LOGE("On get progress failed!");
        return FsResult<void>::Error(E_PARAMS);
    }

    if (!fileCacheEntity) {
        LOGE("Failed to get file cache entity.");
        return FsResult<void>::Error(E_PARAMS);
    }

    auto arg = make_shared<RegisterInfoArg>();
    arg->eventType = event;
    arg->callback = callback;
    if (!fileCacheEntity->registerMgr.AddRegisterInfo(arg)) {
        LOGE("Batch-On register callback fail, callback already exist");
        return FsResult<void>::Error(E_PARAMS);
    }

    int32_t ret = CloudSyncManager::GetInstance().RegisterFileCacheCallback(arg->callback);
    if (ret != E_OK) {
        LOGE("RegisterDownloadFileCallback error, ret: %{public}d", ret);
        (void)fileCacheEntity->registerMgr.RemoveRegisterInfo(event);
        return FsResult<void>::Error(Convert2ErrNum(ret));
    }

    return FsResult<void>::Success();
}

FsResult<void> CloudFileCacheCore::DoOff(
    const string &event, const optional<shared_ptr<CloudDownloadCallbackMiddle>> &callback)
{
    LOGI("Off begin");
    if (event != PROGRESS && event != MULTI_PROGRESS) {
        LOGE("Off get progress failed!");
        return FsResult<void>::Error(E_PARAMS);
    }

    if (!fileCacheEntity) {
        LOGE("Failed to get file cache entity.");
        return FsResult<void>::Error(E_PARAMS);
    }

    if (!fileCacheEntity->registerMgr.HasEvent(event)) {
        LOGE("Batch-Off no callback is registered for this event type: %{public}s.", event.c_str());
        return FsResult<void>::Error(E_PARAMS);
    }

    int32_t ret = CloudSyncManager::GetInstance().UnregisterFileCacheCallback();
    if (ret != E_OK) {
        LOGE("Failed to unregister callback, error: %{public}d", ret);
        return FsResult<void>::Error(Convert2ErrNum(ret));
    }

    if (!fileCacheEntity->registerMgr.RemoveRegisterInfo(event)) {
        LOGE("Batch-Off remove callback is failed, event type: %{public}s.", event.c_str());
        return FsResult<void>::Error(E_PARAMS);
    }

    return FsResult<void>::Success();
}

FsResult<void> CloudFileCacheCore::DoStart(const string &uri)
{
    int32_t ret = CloudSyncManager::GetInstance().StartFileCache(uri);
    if (ret != E_OK) {
        LOGE("Start Download failed with errormessage: ret = %{public}d", ret);
        return FsResult<void>::Error(Convert2ErrNum(ret));
    }
    LOGI("Start Download successfully!");

    return FsResult<void>::Success();
}

FsResult<void> CloudFileCacheCore::DoStop(const string &uri, const bool needClean)
{
    int32_t ret = CloudSyncManager::GetInstance().StopDownloadFile(uri, needClean);
    if (ret != E_OK) {
        LOGE("Stop Download failed! ret = %{public}d", ret);
        return FsResult<void>::Error(Convert2ErrNum(ret));
    }

    return FsResult<void>::Success();
}

FsResult<void> CloudFileCacheCore::CleanCache(const string &uri)
{
    LOGI("CleanCache start");

    int32_t ret = CloudSyncManager::GetInstance().CleanCache(uri);
    if (ret != E_OK) {
        LOGE("Clean Cache failed! ret = %{public}d", ret);
        return FsResult<void>::Error(Convert2ErrNum(ret));
    }

    return FsResult<void>::Success();
}
} // namespace OHOS::FileManagement::CloudSync