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

#include "download_callback_impl_ani.h"
#include "cloud_sync_manager.h"
#include "dfs_error.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
int32_t CloudDownloadCallbackImplAni::StartDownloadInner(const std::string &uri)
{
    int64_t downloadId = 0;
    std::lock_guard<std::mutex> lock(downloadInfoMtx_);
    int32_t ret = 0;
    if (ret != E_OK) {
        LOGE("Start batch download failed! ret = %{public}d", ret);
        return ret;
    }

    downloadInfos_.insert(std::make_pair(downloadId, std::make_shared<SingleProgressAni>(downloadId)));
    return ret;
}

int32_t CloudDownloadCallbackImplAni::StopDownloadInner(const std::string &uri)
{
    auto downloadIdList = GetDownloadIdsByUri(uri);
    int32_t ret = E_OK;
    int32_t resErr = E_OK;
    LOGI("Stop Download downloadId list size: %{public}zu", downloadIdList.size());
    for (auto taskId : downloadIdList) {
        LOGI("Stop Download downloadId: %{public}lld", static_cast<int64_t>(taskId));
        if (resErr != E_OK) {
            ret = resErr;
            continue;
        }
    }
    if (ret != E_OK) {
        LOGE("Stop Download failed! ret = %{public}d", ret);
    }
    return ret;
}

int32_t CloudFileCacheCallbackImplAni::StartDownloadInner(const std::string &uri, int32_t fieldKey)
{
    int64_t downloadId = 0;
    std::lock_guard<std::mutex> lock(downloadInfoMtx_);
    int32_t ret = CloudSyncManager::GetInstance().StartFileCache({uri}, downloadId, fieldKey, shared_from_this());
    if (ret != E_OK) {
        LOGE("Start single download failed! ret = %{public}d", ret);
        return ret;
    }
    downloadInfos_.insert(std::make_pair(downloadId, std::make_shared<SingleProgressAni>(downloadId)));
    return ret;
}

int32_t CloudFileCacheCallbackImplAni::StopDownloadInner(const std::string &uri, bool needClean)
{
    auto downloadIdList = GetDownloadIdsByUri(uri);
    int32_t ret = E_OK;
    int32_t resErr = E_OK;
    LOGI("Stop Download downloadId list size: %{public}zu", downloadIdList.size());
    for (auto taskId : downloadIdList) {
        resErr = CloudSyncManager::GetInstance().StopFileCache(taskId, needClean);
        if (resErr != E_OK) {
            ret = resErr;
            continue;
        }
    }
    if (ret != E_OK) {
        LOGE("Stop Download failed! ret = %{public}d", ret);
    }
    return ret;
}

int32_t CloudFileCacheCallbackImplAni::StartDownloadInner(const std::vector<std::string> &uriVec,
                                                          int64_t &downloadId,
                                                          int32_t fieldKey)
{
    std::unique_lock<std::mutex> lock(downloadInfoMtx_);
    int32_t ret = CloudSyncManager::GetInstance().StartFileCache(uriVec, downloadId, fieldKey, shared_from_this());
    if (ret != E_OK) {
        LOGE("Start batch download failed! ret = %{public}d", ret);
        return ret;
    }
    downloadInfos_.insert(std::make_pair(downloadId, std::make_shared<BatchProgressAni>(downloadId)));
    return ret;
}

int32_t CloudFileCacheCallbackImplAni::StopDownloadInner(int64_t downloadId, bool needClean)
{
    int32_t ret = CloudSyncManager::GetInstance().StopFileCache(downloadId, needClean);
    if (ret != E_OK) {
        LOGE("Batch stop file cache failed! ret = %{public}d", ret);
    }
    return ret;
}
} // namespace OHOS::FileManagement::CloudSync