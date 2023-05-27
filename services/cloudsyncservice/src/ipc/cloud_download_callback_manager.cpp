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

#include "utils_log.h"
#include "ipc/cloud_download_callback_manager.h"

namespace OHOS::FileManagement::CloudSync {
using namespace std;

CloudDownloadCallbackManager::CloudDownloadCallbackManager()
{
    callback_ = nullptr;
}

void CloudDownloadCallbackManager::StartDonwload(const std::string path, const int32_t userId)
{
    downloads_[path].state = DownloadProgressObj::Status::RUNNING;
    downloads_[path].path = path;
    LOGI("download_file : %{public}d start download %{public}s callback success.", userId, path.c_str());
}

void CloudDownloadCallbackManager::StopDonwload(const std::string path, const int32_t userId)
{
    if (downloads_.find(path) != downloads_.end()) {
        downloads_[path].state = DownloadProgressObj::Status::STOPPED;
        LOGI("download_file : %{public}d stop download %{public}s callback success.", userId, path.c_str());
    } else {
        LOGI("download_file : %{public}d stop download %{public}s callback fail, task not exist.", userId, path.c_str());
    }
}

void CloudDownloadCallbackManager::RegisterCallback(const int32_t userId,
                                                    const sptr<ICloudDownloadCallback> downloadCallback)
{
    LOGI("download_file : %{public}d register download callback : %{public}d", userId, downloadCallback != nullptr);
    callback_ = downloadCallback;
}

void CloudDownloadCallbackManager::UnregisterCallback(const int32_t userId)
{
    LOGI("download_file : %{public}d unregister download callback", userId);
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
    auto downloadedState = DownloadProgressObj::FAILED;
    bool isDownloading = (downloads_.find(path) != downloads_.end());
    LOGI("download_file : [callback downloaded] %{public}s is downloading : %{public}d .", path.c_str(), isDownloading);
    if (isDownloading) {
        auto &download = downloads_[path];
        if (err.HasError()) {
            downloadedState = DownloadProgressObj::FAILED;
        } else {
            downloadedState = DownloadProgressObj::COMPLETED;
        }
        LOGI("download_file : [callback downloaded] %{public}s state is %{public}s.", path.c_str(), download.to_string().c_str());
        if (callback_ != nullptr && download.state == DownloadProgressObj::RUNNING) {
            download.state = downloadedState;
            callback_->OnDownloadProcess(download);
            if (assetsToDownload.size() == 1) {
                (void)handler->OnDownloadSuccess(assetsToDownload[0]);
            }
        }

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
    LOGI("download_file : [callback downloading] %{public}s is downloading : %{public}d .", path.c_str(), isDownloading);
    if (isDownloading) {
        auto &download = downloads_[path];
        download.downloadedSize = downloadSize;
        download.totalSize = totalSize;
        LOGI("download_file : [callback downloading] %{public}s state is %{public}s.", path.c_str(), download.to_string().c_str());
        if (callback_ != nullptr && download.state == DownloadProgressObj::RUNNING) {
            callback_->OnDownloadProcess(download);
        }
    }
}

} // namespace OHOS::FileManagement::CloudSync
