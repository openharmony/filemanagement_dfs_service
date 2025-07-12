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

#include "download_callback_middle_napi.h"
#include "dfs_error.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
void CloudDlCallbackMiddleNapi::RemoveDownloadInfo(int64_t downloadId)
{
    std::lock_guard<std::mutex> lock(downloadInfoMtx_);
    downloadInfos_.erase(downloadId);
}

std::shared_ptr<DlProgressNapi> CloudDlCallbackMiddleNapi::GetDownloadInfo(int64_t downloadId)
{
    std::lock_guard<std::mutex> lock(downloadInfoMtx_);
    auto it = downloadInfos_.find(downloadId);
    if (it != downloadInfos_.end()) {
        return it->second;
    }
    return nullptr;
}

std::vector<int64_t> CloudDlCallbackMiddleNapi::GetDownloadIdsByUri(const std::string &uri)
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

void CloudDlCallbackMiddleNapi::OnDownloadProcess(const DownloadProgressObj &progress)
{
    auto fileCacheInfo = GetDownloadInfo(progress.downloadId);
    if (fileCacheInfo == nullptr) {
        LOGE("Failed to callback, no such taskId: %{public}lld", static_cast<long long>(progress.downloadId));
        return;
    }
    fileCacheInfo->Update(progress);
    std::shared_ptr<CloudDlCallbackMiddleNapi> callbackImpl = shared_from_this();
    napi_status status = napi_send_event(
        callbackImpl->env_,
        [fileCacheInfo, callbackImpl]() mutable {
            if (fileCacheInfo == nullptr || callbackImpl == nullptr) {
                LOGE("Failed to callback, is callbackImpl null: %{public}d", (callbackImpl == nullptr));
                return;
            }
            napi_env tmpEnv = callbackImpl->env_;
            napi_handle_scope scope = nullptr;
            napi_status status = napi_open_handle_scope(tmpEnv, &scope);
            if (status != napi_ok) {
                LOGE("Failed to open handle scope, status: %{public}d", status);
                napi_close_handle_scope(tmpEnv, scope);
                return;
            }
            napi_value jsProgress = fileCacheInfo->ConvertToValue(tmpEnv);
            if (jsProgress == nullptr) {
                LOGE("Failed to convert progress to nvalue");
                napi_close_handle_scope(tmpEnv, scope);
                return;
            }
            callbackImpl->OnJsCallback(&jsProgress, 1);
            napi_close_handle_scope(tmpEnv, scope);
            if (fileCacheInfo->IsNeedClean()) {
                callbackImpl->RemoveDownloadInfo(fileCacheInfo->GetTaskId());
            }
        },
        napi_eprio_immediate);
    if (status != napi_ok) {
        LOGE("Failed to execute libuv work queue, status: %{public}d", status);
    }
}
} // namespace OHOS::FileManagement::CloudSync