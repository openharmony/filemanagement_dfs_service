/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "ipc/cloud_download_callback_manager.h"
#include "dfs_error.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
using namespace std;

void CloudDownloadCallbackManager::StartDonwload(const std::string path, const int32_t userId)
{
    downloads_[path].state = CloudDownloadCallProgress::DownloadStatus::RUNNING;
    downloads_[path].path = path;
}

void CloudDownloadCallbackManager::StopDonwload(const std::string path, const int32_t userId)
{
    if (downloads_.find(path) != downloads_.end()) {
        downloads_[path].state = CloudDownloadCallProgress::DownloadStatus::STOPPED;
    }
}

void CloudDownloadCallbackManager::RegisterCallback(const int32_t userId,
                                                    const sptr<ICloudDownloadCallback> downloadCallback)
{
    callback_ = downloadCallback;
}

void CloudDownloadCallbackManager::UnregisterCallback(const int32_t userId)
{
    callback_ = nullptr;
}

void CloudDownloadCallbackManager::OnDownloadedResult(
    const std::string path,
    std::vector<DriveKit::DKDownloadAsset> assetsToDownload,
    std::shared_ptr<DataHandler> handler,
    std::shared_ptr<DriveKit::DKContext> context,
    std::shared_ptr<const DriveKit::DKDatabase> database,
    const std::map<DriveKit::DKDownloadAsset, DriveKit::DKDownloadResult> &results,
    const DriveKit::DKError &err)
{
    auto downloadedState = CloudDownloadCallProgress::DownloadStatus::FAILED;
    bool isDownloading = (downloads_.find(path) != downloads_.end());
    if (isDownloading) {
        auto &download = downloads_[path];
        if (err.HasError()) {
            downloadedState = CloudDownloadCallProgress::DownloadStatus::FAILED;
        } else {
            downloadedState = CloudDownloadCallProgress::DownloadStatus::COMPLETED;
        }
        download.state = downloadedState;
    }

    if (callback_ != nullptr) {
        callback_->OnDownloadedResult(path, (int32_t)downloadedState);
        if (assetsToDownload.size() == 1) {
            (void)handler->OnDownloadSuccess(assetsToDownload[0]);
        }
    }

    if (isDownloading) {
        downloads_.erase(path);
    }
}

void CloudDownloadCallbackManager::OnDownloadProcess(const std::string path,
                                                     std::shared_ptr<DriveKit::DKContext> context,
                                                     DriveKit::DKDownloadAsset asset,
                                                     DriveKit::TotalSize totalSize,
                                                     DriveKit::DownloadSize downloadSize)
{
    bool isDownloading = (downloads_.find(path) != downloads_.end());
    if (isDownloading) {
        auto &download = downloads_[path];
        download.downloadedSize = downloadSize;
        download.totalSize = totalSize;
    }

    if (callback_ != nullptr) {
        callback_->OnDownloadProcess(path, downloadSize, totalSize);
    }
}

} // namespace OHOS::FileManagement::CloudSync
