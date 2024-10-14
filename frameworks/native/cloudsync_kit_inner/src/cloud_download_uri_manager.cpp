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

#include "cloud_download_uri_manager.h"
#include "dfs_error.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {

CloudDownloadUriManager& CloudDownloadUriManager::GetInstance()
{
    static CloudDownloadUriManager mgr;
    return mgr;
}

int32_t CloudDownloadUriManager::AddPathToUri(const std::string& path, const std::string& uri)
{
    LOGI("download_file : add path [ %{public}s ] -> uri [ %{public}s ]",
         GetAnonyString(path).c_str(), GetAnonyString(uri).c_str());
    std::lock_guard<std::mutex> lock(pathUriMutex_);
    if (pathUriMap_.find(path) == pathUriMap_.end()) {
        pathUriMap_[path] = uri;
        LOGI("download_file : add path [ %{public}s ] success, pathUriMap_ size is %{public}zu",
            GetAnonyString(path).c_str(), pathUriMap_.size());
        return E_OK;
    }
    LOGE("file is already trigger downloading, pathUriMap_ size is %{public}zu", pathUriMap_.size());
    return E_STOP;
}

int32_t CloudDownloadUriManager::AddDownloadIdToPath(int64_t &downloadId, std::vector<std::string> &pathVec)
{
    std::lock_guard<std::mutex> lock(downloadIdPathMutex_);
    if (downloadIdPathMap_.find(downloadId) == downloadIdPathMap_.end()) {
        downloadIdPathMap_[downloadId] = pathVec;
        LOGI("download_file : AddDownloadIdToPath add downloadId %{public}lld", static_cast<long long>(downloadId));
    }
    return E_OK;
}

void CloudDownloadUriManager::RemoveUri(const std::string& path)
{
    std::lock_guard<std::mutex> lock(pathUriMutex_);
    if (pathUriMap_.find(path) != pathUriMap_.end()) {
        LOGI("download_file : remove path [ %{public}s ] success, pathUriMap_ size is %{public}zu",
            GetAnonyString(path).c_str(), pathUriMap_.size());
        pathUriMap_.erase(path);
    }
}
void CloudDownloadUriManager::CheckDownloadIdPathMap(int64_t &downloadId)
{
    bool existUri = false;
    std::lock_guard<std::mutex> lock(downloadIdPathMutex_);
    if (downloadIdPathMap_.find(downloadId) != downloadIdPathMap_.end()) {
        std::vector<std::string> pathVec = downloadIdPathMap_[downloadId];

        std::lock_guard<std::mutex> lock2(pathUriMutex_);
        for (unsigned long i = 0; i < pathVec.size(); i++) {
            if (pathUriMap_.find(pathVec[i]) != pathUriMap_.end()) {
                existUri = true;
                break;
            }
        }
        if (!existUri) {
            downloadIdPathMap_.erase(downloadId);
            LOGI("download_file : remove downloadId [ %{public}lld ] success, downloadIdPathMap_ size is %{public}zu",
                 static_cast<long long>(downloadId), downloadIdPathMap_.size());
        }
    }
}

void CloudDownloadUriManager::RemoveUri(const int64_t &downloadId)
{
    std::lock_guard<std::mutex> lock(downloadIdPathMutex_);
    if (downloadIdPathMap_.find(downloadId) != downloadIdPathMap_.end()) {
        std::vector<std::string> pathVec = downloadIdPathMap_[downloadId];

        std::lock_guard<std::mutex> lock2(pathUriMutex_);
        for (unsigned long i = 0; i < pathVec.size(); i++) {
            if (pathUriMap_.find(pathVec[i]) != pathUriMap_.end()) {
                pathUriMap_.erase(pathVec[i]);
                LOGI("download_file : remove path [ %{public}s ] success, pathUriMap_ size is %{public}zu",
                    GetAnonyString(pathVec[i]).c_str(), pathUriMap_.size());
            }
        }
        downloadIdPathMap_.erase(downloadId);
        LOGI("download_file : remove downloadId [ %{public}lld ] success, downloadIdPathMap_ size is %{public}zu",
             static_cast<long long>(downloadId), downloadIdPathMap_.size());
    }
}

std::string CloudDownloadUriManager::GetUri(const std::string& path)
{
    std::lock_guard<std::mutex> lock(pathUriMutex_);
    if (pathUriMap_.find(path) != pathUriMap_.end()) {
        LOGI("download_file : get path [ %{public}s ] success, pathUriMap_ size is %{public}zu",
            GetAnonyString(path).c_str(), pathUriMap_.size());
        return pathUriMap_[path];
    }

    LOGE("download_file : get path [ %{public}s ] fail, pathUriMap_ size is %{public}zu",
        GetAnonyString(path).c_str(), pathUriMap_.size());
    return "";
}

void CloudDownloadUriManager::Reset()
{
    std::lock_guard<std::mutex> lock(pathUriMutex_);
    pathUriMap_.clear();
}
} // namespace OHOS::FileManagement::CloudSync
