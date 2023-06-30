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
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
using namespace std;
CloudDownloadUriManager& CloudDownloadUriManager::GetInstance()
{
    static CloudDownloadUriManager mgr;
    return mgr;
}

void CloudDownloadUriManager::RemoveUri(const std::string& path)
{
    if (pathMap_.find(path) != pathMap_.end()) {
        LOGI("download_file : remove path [ %{public}s ] success", path.c_str());
        pathMap_.erase(path);
    }
}

std::string CloudDownloadUriManager::GetUri(const std::string& path)
{
    if (path.empty()) {
        LOGI("path is empty");
        return "";
    }
    return path;
}

} // namespace OHOS::FileManagement::CloudSync
