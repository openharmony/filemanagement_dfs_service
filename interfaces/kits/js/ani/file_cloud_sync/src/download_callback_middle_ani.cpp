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

#include "download_callback_middle_ani.h"
#include "dfs_error.h"
#include "multi_download_progress_ani.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
constexpr int32_t ANI_SCOPE_SIZE = 16;
void CloudDlCallbackMiddleAni::RemoveDownloadInfo(int64_t downloadId)
{
    std::lock_guard<std::mutex> lock(downloadInfoMtx_);
    downloadInfos_.erase(downloadId);
}

std::shared_ptr<DlProgressAni> CloudDlCallbackMiddleAni::GetDownloadInfo(const DownloadProgressObj &progress)
{
    std::shared_ptr<DlProgressAni> originProgress = nullptr;
    {
        std::lock_guard<std::mutex> lock(downloadInfoMtx_);
        auto it = downloadInfos_.find(progress.downloadId);
        if (it == downloadInfos_.end()) {
            return nullptr;
        }
        originProgress = it->second;
    }

    // Copy a new object to return the callback to avoid affecting the download progress.
    std::shared_ptr<DlProgressAni> resProgress = originProgress->CreateNewObject();
    originProgress->Update(progress);
    resProgress->Update(progress);
    return resProgress;
}

std::vector<int64_t> CloudDlCallbackMiddleAni::GetDownloadIdsByUri(const std::string &uri)
{
    std::vector<int64_t> ids;
    std::lock_guard<std::mutex> lock(downloadInfoMtx_);
    for (const auto &[id, progress] : downloadInfos_) {
        if (progress->GetUri() == uri) {
            ids.push_back(id);
        }
    }
    return ids;
}

void CloudDlCallbackMiddleAni::OnDownloadProcess(const DownloadProgressObj &progress)
{
    auto fileCacheInfo = GetDownloadInfo(progress);
    if (fileCacheInfo == nullptr) {
        LOGE("Failed to callback, no such taskId: %{public}lld", static_cast<long long>(progress.downloadId));
        return;
    }
    std::shared_ptr<CloudDlCallbackMiddleAni> callbackImpl = shared_from_this();
    auto task = [fileCacheInfo, callbackImpl]() mutable {
        if (fileCacheInfo == nullptr || callbackImpl == nullptr) {
            LOGE("Failed to callback, is callbackImpl null: %{public}d", (callbackImpl == nullptr));
            return;
        }
        LOGI("CloudDlCallbackMiddleAni OnDownloadProcess for JS");
        bool isAttached = false;
        ani_env *tmpEnv = callbackImpl->GetEnv(isAttached);
        if (tmpEnv == nullptr) {
            LOGE("Failed to get env from vm");
            return;
        }
        ani_size nr_refs = ANI_SCOPE_SIZE;
        ani_status ret = tmpEnv->CreateLocalScope(nr_refs);
        if (ret != ANI_OK) {
            LOGE("crete local scope failed. ret = %{public}d", ret);
            return;
        }
        ani_object pg = fileCacheInfo->ConvertToObject(tmpEnv);
        callbackImpl->OnJsCallback(tmpEnv, pg, 1);
        ret = tmpEnv->DestroyLocalScope();
        if (ret != ANI_OK) {
            LOGE("failed to DestroyLocalScope. ret = %{public}d", ret);
        }
        if (fileCacheInfo->IsNeedClean()) {
            callbackImpl->RemoveDownloadInfo(fileCacheInfo->GetTaskId());
        }
        callbackImpl->DetachEnv(isAttached);
    };
    if (!ANIUtils::SendEventToMainThread(task)) {
        LOGE("failed to send event");
    }
}

void CloudDlCallbackMiddleAni::TryCleanCallback()
{
    bool noTask = false;
    {
        std::lock_guard<std::mutex> lock(downloadInfoMtx_);
        noTask = (downloadInfos_.size() == 0);
    }
    if (noTask) {
        CleanAllCallback(false);
    }
}
} // namespace OHOS::FileManagement::CloudSync