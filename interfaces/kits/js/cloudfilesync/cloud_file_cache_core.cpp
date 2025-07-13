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
FsResult<CloudFileCacheCore *> CloudFileCacheCore::Constructor()
{
    std::unique_ptr<CloudFileCacheCore> cloudFileCachePtr = std::make_unique<CloudFileCacheCore>();
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
        LOGE("Stop Download failed! ret = %{public}d", ret);
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
        LOGE("Stop Download failed! ret = %{public}d", ret);
        return FsResult<int64_t>::Error(Convert2ErrNum(ret));
    }

    return FsResult<int64_t>::Success(downloadId);
}

FsResult<void> CloudFileCacheCore::DoStop(const string &uri, bool needClean)
{
    auto callbackImpl = GetCallbackImpl(PROGRESS, false);
    if (callbackImpl == nullptr) {
        LOGE("Failed to stop download, callback is null!");
        return FsResult<void>::Error(E_INVAL_ARG);
    }
    int32_t ret = callbackImpl->StopDownloadInner(uri, needClean);
    if (ret != E_OK) {
        LOGE("Stop Download failed! ret = %{public}d", ret);
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
} // namespace OHOS::FileManagement::CloudSync