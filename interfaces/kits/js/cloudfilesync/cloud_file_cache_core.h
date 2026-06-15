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

#ifndef OHOS_FILEMGMT_CLOUD_FILE_CACHE_CORE_H
#define OHOS_FILEMGMT_CLOUD_FILE_CACHE_CORE_H

#include <mutex>
#include <optional>
#include <unordered_set>

#include "download_callback_impl_ani.h"
#include "filemgmt_libfs.h"

namespace OHOS::FileManagement::CloudSync {
const std::string PROGRESS = "progress";
const std::string MULTI_PROGRESS = "batchDownload";
class CloudFileCacheCore {
public:
    CloudFileCacheCore() = default;
    explicit CloudFileCacheCore(const std::string &bundleName) : bundleName_(bundleName) {}
    ~CloudFileCacheCore() = default;

    static ModuleFileIO::FsResult<CloudFileCacheCore *> Constructor(
        const std::optional<std::string> &bundleName = std::nullopt);
    ModuleFileIO::FsResult<void> DoOn(const std::string &event,
                                      const std::shared_ptr<CloudFileCacheCallbackImplAni> callback);
    ModuleFileIO::FsResult<void> DoOff(const std::string &event,
        const std::optional<std::shared_ptr<CloudFileCacheCallbackImplAni>> &callback = std::nullopt);
    ModuleFileIO::FsResult<void> DoStart(const std::string &uri);
    ModuleFileIO::FsResult<int64_t> DoStart(const std::vector<std::string> &uriList, int32_t fieldKey);
    ModuleFileIO::FsResult<void> DoStop(const std::string &uri, bool needClean = false);
    ModuleFileIO::FsResult<void> DoStop(int64_t downloadId, bool needClean = false);
    ModuleFileIO::FsResult<void> CleanCache(const std::string &uri);
    ModuleFileIO::FsResult<void> CleanFileCache(const std::string &uri);
    ModuleFileIO::FsResult<void> GetDownloadList(const std::vector<std::string> &uriVec,
            std::vector<CloudSync::DownloadProgressObj> &downloadList);
    ModuleFileIO::FsResult<int64_t> GetCachedTotalSize();
    std::shared_ptr<CloudFileCacheCallbackImplAni> GetCallbackImpl(const std::string &eventType, bool isInit);
    ModuleFileIO::FsResult<void> CleanAllFileCache();

private:
    std::string bundleName_;
    std::mutex registerMutex_;
    std::unordered_map<std::string, std::shared_ptr<CloudFileCacheCallbackImplAni>> registerMap_;
};

} // namespace OHOS::FileManagement::CloudSync
#endif // OHOS_FILEMGMT_CLOUD_FILE_CACHE_CORE_H