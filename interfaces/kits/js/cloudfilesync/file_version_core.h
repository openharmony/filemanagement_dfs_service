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

#ifndef OHOS_FILEMGMT_FILE_VERSION_CORE_H
#define OHOS_FILEMGMT_FILE_VERSION_CORE_H

#include "file_version_callback_ani.h"
#include "filemgmt_libfs.h"

namespace OHOS::FileManagement::CloudSync {
using namespace ModuleFileIO;

class FileVersionCore {
public:
    static FsResult<FileVersionCore *> Constructor();
    FsResult<std::vector<HistoryVersion>> GetHistoryVersionList(const std::string &uri, int32_t numLimit);
    FsResult<std::string> DownloadHistoryVersion(const std::string &uri, uint64_t versionId,
        std::shared_ptr<CloudDownloadCallback> callback);
    FsResult<void> ReplaceFileWithHistoryVersion(const std::string &oriUri, const std::string &verUri);
    FsResult<bool> IsConflict(const std::string &uri);
    FsResult<void> ClearFileConflict(const std::string &uri);
    FileVersionCore() {};
    ~FileVersionCore() = default;

private:
    // std::shared_ptr<CloudSyncCallbackMiddle> callback_;
    // std::unique_ptr<BundleEntity> bundleEntity;
};
} // namespace OHOS::FileManagement::CloudSync
#endif // OHOS_FILEMGMT_FILE_VERSION_CORE_H
