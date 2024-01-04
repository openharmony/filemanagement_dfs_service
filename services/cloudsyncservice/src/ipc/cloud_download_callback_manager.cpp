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
#include "data_sync/task_state_manager.h"

namespace OHOS::FileManagement::CloudSync {
using namespace std;

CloudDownloadCallbackManager::CloudDownloadCallbackManager()
{
    callback_ = nullptr;
}

bool CloudDownloadCallbackManager::FindDownload(const std::string path)
{
    bool ret = false;
    lock_guard<mutex> lock(downloadsMtx_);
    if (downloads_.find(path) != downloads_.end() &&
        downloads_[path].state == DownloadProgressObj::Status::RUNNING) {
        ret = true;
    }
    return ret;
}

void CloudDownloadCallbackManager::StartDonwload(const std::string path,
                                                 const int32_t userId,
                                                 const int64_t downloadId)
{
    lock_guard<mutex> lock(downloadsMtx_);
    TaskStateManager::GetInstance().StartTask(bundleName_, TaskType::DOWNLOAD_TASK);
    if (downloads_.find(path) == downloads_.end()) {
        downloads_[path].refCount = 1;
    } else {
        downloads_[path].refCount++;
    }
    downloads_[path].state = DownloadProgressObj::Status::RUNNING;
    downloads_[path].path = path;
    downloads_[path].downloadId = downloadId;
    LOGI("download_file : %{public}d start download %{public}s callback success.", userId, path.c_str());
}

bool CloudDownloadCallbackManager::StopDonwload(const std::string path, const int32_t userId,
    DownloadProgressObj &download)
{
    bool ret = false;
    lock_guard<mutex> lock(downloadsMtx_);

    auto res = downloads_.find(path);
    bool isDownloading = (res != downloads_.end());
    if (isDownloading) {
        download = res->second;
        LOGI("download_file : %{public}d stop download %{public}s callback success.", userId, path.c_str());
        downloads_.erase(res);
        ret = true;
    } else {
        LOGI("download_file : %{public}d stop download %{public}s callback fail, task not exist.",
            userId, path.c_str());
        ret = false;
    }
    CheckTaskState();
    return ret;
}

std::vector<int64_t> CloudDownloadCallbackManager::StopAllDownloads(const int32_t userId)
{
    std::vector<int64_t> ret;
    lock_guard<mutex> lock(downloadsMtx_);
    for (auto &pair : downloads_) {
        pair.second.state = DownloadProgressObj::Status::FAILED;
        ret.push_back(pair.second.downloadId);
        if (callback_ != nullptr) {
            callback_->OnDownloadProcess(pair.second);
            LOGI("download_file : %{public}d stop download %{public}s callback success.", userId, pair.first.c_str());
        }
    }
    downloads_.clear();
    CheckTaskState();
    return ret;
}

void CloudDownloadCallbackManager::RegisterCallback(const int32_t userId,
                                                    const sptr<ICloudDownloadCallback> downloadCallback)
{
    LOGI("download_file : %{public}d register download callback : %{public}d", userId,
        downloadCallback != nullptr);
    unique_lock<mutex> lock(callbackMutex_);
    callback_ = downloadCallback;
}

void CloudDownloadCallbackManager::UnregisterCallback(const int32_t userId)
{
    LOGI("download_file : %{public}d unregister download callback", userId);
    unique_lock<mutex> lock(callbackMutex_);
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
    unique_lock<mutex> lock(downloadsMtx_);
    auto res = downloads_.find(path);
    bool isDownloading = (res != downloads_.end());

    LOGI("download_file : [callback downloaded] %{public}s is downloading : %{public}d .",
         path.c_str(), isDownloading);
    if (!isDownloading) {
        lock.unlock();
        return;
    }

    auto download = res->second;
    download.refCount--;
    if (download.refCount == 0) {
        downloads_.erase(res);
        CheckTaskState();
    } else {
        lock.unlock();
        return;
    }
    lock.unlock();

    LOGI("download_file : [callback downloaded] %{public}s state is %{public}s, localErr is %{public}d.",
         path.c_str(), download.to_string().c_str(), static_cast<int>(err.dkErrorCode));

    if (callback_ != nullptr && download.state == DownloadProgressObj::STOPPED) {
        LOGI("stop state");
        callback_->OnDownloadProcess(download);
        return;
    }

    auto downloadedState = err.HasError() ? DownloadProgressObj::FAILED : DownloadProgressObj::COMPLETED;
    /**
     * Avoiding the issue of cloud service not returning a total error code.
     * Currently, only single asset download is supported.
     */
    for (const auto &it : results) {
        if (!it.second.IsSuccess()) {
            LOGE("download file failed, localErr: %{public}d", it.second.GetDKError().dkErrorCode);
            downloadedState = DownloadProgressObj::FAILED;
            break;
        }
    }

    if (download.state == DownloadProgressObj::RUNNING) {
        download.state = downloadedState;
        if (callback_ != nullptr) {
            callback_->OnDownloadProcess(download);
        }
        if ((assetsToDownload.size() == 1) && (download.state == DownloadProgressObj::COMPLETED)) {
            (void)handler->OnDownloadSuccess(assetsToDownload[0]);
        }
    }
}

void CloudDownloadCallbackManager::OnDownloadProcess(const std::string path,
                                                     std::shared_ptr<DriveKit::DKContext> context,
                                                     DriveKit::DKDownloadAsset asset,
                                                     DriveKit::TotalSize totalSize,
                                                     DriveKit::DownloadSize downloadSize)
{
    unique_lock<mutex> lock(downloadsMtx_);
    auto res = downloads_.find(path);
    bool isDownloading = (res != downloads_.end());

    LOGI("download_file : [callback downloading] %{public}s is downloading : %{public}d .",
        path.c_str(), isDownloading);
    if (!isDownloading) {
        lock.unlock();
        return;
    }

    auto download = res->second;
    lock.unlock();

    download.downloadedSize = downloadSize;
    download.totalSize = totalSize;
    LOGI("download_file : [callback downloading] %{public}s state is %{public}s.", path.c_str(),
         download.to_string().c_str());
    if (callback_ != nullptr && download.state == DownloadProgressObj::RUNNING) {
        callback_->OnDownloadProcess(download);
    }
}

void CloudDownloadCallbackManager::SetBundleName(const std::string &bundleName)
{
    bundleName_ = bundleName;
}

void CloudDownloadCallbackManager::CheckTaskState()
{
    if (downloads_.empty()) {
        TaskStateManager::GetInstance().CompleteTask(bundleName_, TaskType::DOWNLOAD_TASK);
    }
}

void CloudDownloadCallbackManager::NotifyProcessStop(DownloadProgressObj &download)
{
    download.state = DownloadProgressObj::Status::STOPPED;
    if (callback_ != nullptr) {
        callback_->OnDownloadProcess(download);
    }
}
} // namespace OHOS::FileManagement::CloudSync
