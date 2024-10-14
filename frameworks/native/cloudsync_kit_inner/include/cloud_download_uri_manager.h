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

#ifndef OHOS_FILEMGMT_CLOUD_DOWNLOAD_URI_MANAGER_H
#define OHOS_FILEMGMT_CLOUD_DOWNLOAD_URI_MANAGER_H

#include "nocopyable.h"

#include <map>
#include <mutex>

namespace OHOS::FileManagement::CloudSync {
class CloudDownloadUriManager : public NoCopyable {
public:
    static CloudDownloadUriManager &GetInstance();

    int32_t AddPathToUri(const std::string& path, const std::string& uri);
    int32_t AddDownloadIdToPath(int64_t &downloadId, std::vector<std::string> &pathVec);
    std::string GetUri(const std::string& path);
    void RemoveUri(const std::string& path);
    void CheckDownloadIdPathMap(int64_t &downloadId);
    void RemoveUri(const int64_t &downloadId);
    void Reset();

private:
    CloudDownloadUriManager() = default;
    std::mutex pathUriMutex_;
    std::mutex downloadIdPathMutex_;
    std::map<std::string, std::string> pathUriMap_;
    std::map<uint64_t, std::vector<std::string>> downloadIdPathMap_;
};
} // namespace OHOS::FileManagement::CloudSync

#endif // OHOS_FILEMGMT_CLOUD_DOWNLOAD_URI_MANAGER_H
