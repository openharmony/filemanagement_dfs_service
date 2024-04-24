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
    LOGI("download_file : add path [ %{public}s ] -> uri [ %{public}s ]", path.c_str(), uri.c_str());
    std::lock_guard<std::mutex> lock(mutex_);
    if (pathMap_.find(path) == pathMap_.end()) {
        pathMap_[path] = uri;
        LOGI("download_file : add path [ %{public}s ] success, pathMap size is %{public}zu",
            path.c_str(), pathMap_.size());
        return E_OK;
    }
    LOGE("file is already trigger downloading, pathMap size is %{public}zu", pathMap_.size());
    return E_STOP;
}

void CloudDownloadUriManager::RemoveUri(const std::string& path)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (pathMap_.find(path) != pathMap_.end()) {
        LOGI("download_file : remove path [ %{public}s ] success, pathMap size is %{public}zu",
            path.c_str(), pathMap_.size());
        pathMap_.erase(path);
    }
}

std::string CloudDownloadUriManager::GetUri(const std::string& path)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (pathMap_.find(path) != pathMap_.end()) {
        LOGI("download_file : get path [ %{public}s ] success, pathMap size is %{public}zu",
            path.c_str(), pathMap_.size());
        return pathMap_[path];
    }

    LOGE("download_file : get path [ %{public}s ] fail, pathMap size is %{public}zu",
        path.c_str(), pathMap_.size());
    return "";
}

} // namespace OHOS::FileManagement::CloudSync
