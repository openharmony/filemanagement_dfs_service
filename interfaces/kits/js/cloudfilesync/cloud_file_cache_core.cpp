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

#include <memory>

#include "cloud_sync_manager.h"
#include "dfs_error.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
using namespace ModuleFileIO;
FsResult<CloudFileCacheCore *> CloudFileCacheCore::Constructor(const std::optional<std::string> &bundleName)
{
    std::unique_ptr<CloudFileCacheCore> cloudFileCachePtr = nullptr;
    if (bundleName.has_value()) {
        const string &name = *bundleName;
        cloudFileCachePtr = std::make_unique<CloudFileCacheCore>(name);
    } else {
        cloudFileCachePtr = std::make_unique<CloudFileCacheCore>();
    }
    return FsResult<CloudFileCacheCore *>::Success(cloudFileCachePtr.release());
}

FsResult<void> CloudFileCacheCore::DoOn(const string &event, const shared_ptr<CloudFileCacheCallbackImplAni> callback)
{
    return FsResult<void>::Success();
}

FsResult<void> CloudFileCacheCore::DoOff(const string &event,
                                         const optional<shared_ptr<CloudFileCacheCallbackImplAni>> &callback)
{
    return FsResult<void>::Success();
}

std::shared_ptr<CloudFileCacheCallbackImplAni> CloudFileCacheCore::GetCallbackImpl(const std::string &eventType,
                                                                                   bool isInit)
{
    std::shared_ptr<CloudFileCacheCallbackImplAni> callbackImpl = nullptr;
    std::lock_guard<std::mutex> lock(registerMutex_);
    auto iter = registerMap_.find(eventType);
    if (iter == registerMap_.end() || iter->second == nullptr) {
        if (isInit) {
            callbackImpl = std::make_shared<CloudFileCacheCallbackImplAni>();
            registerMap_.insert(make_pair(eventType, callbackImpl));
        }
    } else {
        callbackImpl = iter->second;
    }
    return callbackImpl;
}

FsResult<void> CloudFileCacheCore::DoStart(const string &uri)
{
    auto callbackImpl = GetCallbackImpl(PROGRESS, true);
    int32_t ret = callbackImpl->StartDownloadInner(uri, FieldKey::FIELDKEY_CONTENT);
    if (ret != E_OK) {
        LOGE("Start Download failed! ret = %{public}d", ret);
        return FsResult<void>::Error(Convert2ErrNum(ret));
    }

    return FsResult<void>::Success();
}

FsResult<int64_t> CloudFileCacheCore::DoStart(const std::vector<std::string> &uriList, int32_t fieldKey)
{
    auto callbackImpl = GetCallbackImpl(MULTI_PROGRESS, true);
    int64_t downloadId = 0;
    int32_t ret = callbackImpl->StartDownloadInner(uriList, downloadId, fieldKey);
    if (ret != E_OK) {
        ret = (ret == E_CLOUD_SDK) ? E_SERVICE_INNER_ERROR : ret;
        LOGE("Start Download failed! ret = %{public}d", ret);
        return FsResult<int64_t>::Error(Convert2ErrNum(ret));
    }

    return FsResult<int64_t>::Success(downloadId);
}

FsResult<void> CloudFileCacheCore::DoStop(const string &uri, bool needClean)
{
    auto callbackImpl = GetCallbackImpl(PROGRESS, false);
    if (callbackImpl == nullptr) {
        LOGE("Failed to stop download, callback is null!");
        return FsResult<void>::Error(Convert2ErrNum(E_BROKEN_IPC));
    }
    int32_t ret = callbackImpl->StopDownloadInner(uri, needClean);
    if (ret != E_OK) {
        LOGE("Stop Download failed! ret = %{public}d", ret);
        if (ret != E_INVAL_ARG) {
            ret = E_BROKEN_IPC;
        }
        return FsResult<void>::Error(Convert2ErrNum(ret));
    }

    return FsResult<void>::Success();
}

FsResult<void> CloudFileCacheCore::DoStop(int64_t downloadId, bool needClean)
{
    auto callbackImpl = GetCallbackImpl(MULTI_PROGRESS, false);
    if (callbackImpl == nullptr) {
        LOGE("Failed to stop batch download, callback is null!");
        return FsResult<void>::Error(EINVAL);
    }
    int32_t ret = callbackImpl->StopDownloadInner(downloadId, needClean);
    if (ret != E_OK) {
        LOGE("Stop batch download failed! ret = %{public}d", ret);
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

FsResult<void> CloudFileCacheCore::CleanFileCache(const optional<const string> &uri)
{
    LOGI("CleanFileCache start");
    int32_t ret;

    if (uri.has_value()) {
        const string &fileUri = *uri;
        ret = CloudSyncManager::GetInstance().CleanFileCache(fileUri);
        if (ret != E_OK) {
            LOGE("Clean File Cache failed! ret = %{public}d", ret);
            return FsResult<void>::Error(Convert2ErrNum(ret));
        }
    } else {
        std::string bundleName = bundleName_;

        // 如果bundleName_为空，自动获取
        if (bundleName.empty()) {
            ret = CloudSyncManager::GetInstance().CleanAllFileCache();
        } else {
            ret = CloudSyncManager::GetInstance().CleanAllFileCache(bundleName);
        }
    
        if (ret != E_OK) {
            LOGE("Clean File Cache failed! ret = %{public}d", ret);
            return FsResult<void>::Error(Convert2ErrNum(ret));
        }
    }

    return FsResult<void>::Success();
}

FsResult<void> CloudFileCacheCore::GetDownloadList(const std::vector<std::string> &uriVec,
                                                   std::vector<CloudSync::DownloadProgressObj> &downloadList)
{
    int32_t ret =  CloudSyncManager::GetInstance().GetDownloadList(uriVec, downloadList);
    if (ret != E_OK) {
        LOGE("GetDownloadList failed! ret = %{public}d", ret);
        return FsResult<void>::Error(Convert2ErrNum(ret));
    }
    return FsResult<void>::Success();
}

FsResult<int64_t> CloudFileCacheCore::GetCachedTotalSize()
{
    LOGI("GetCachedTotalSize start");
    
    std::string bundleName = bundleName_;
    int64_t totalSize = 0;
    int32_t ret;
    
    // 如果bundleName_为空，自动获取
    if (bundleName.empty()) {
        ret = CloudSyncManager::GetInstance().GetCachedTotalSize(totalSize);
    } else {
        ret = CloudSyncManager::GetInstance().GetCachedTotalSize(bundleName, totalSize);
    }
    
    if (ret != E_OK) {
        LOGE("Get Cached Total Size failed! ret = %{public}d", ret);
        return FsResult<int64_t>::Error(Convert2ErrNum(ret));
    }
    
    return FsResult<int64_t>::Success(totalSize);
}

} // namespace OHOS::FileManagement::CloudSync