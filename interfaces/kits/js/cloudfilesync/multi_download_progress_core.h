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

#ifndef OHOS_FILEMGMT_MULTI_DOWNLOAD_PROGRESS_CORE_H
#define OHOS_FILEMGMT_MULTI_DOWNLOAD_PROGRESS_CORE_H

#include <memory>

#include "download_progress_ani.h"
#include "filemgmt_libfs.h"

namespace OHOS::FileManagement::CloudSync {
struct FailedFileInfo {
    FailedFileInfo(const std::string &file, int32_t err) : uri(file), error(err) {}
    std::string uri;
    int32_t error{0};
};

class MultiDlProgressCore {
public:
    MultiDlProgressCore() = default;
    ~MultiDlProgressCore() = default;

    static ModuleFileIO::FsResult<MultiDlProgressCore *> Constructor();
    int32_t GetStatus();
    int64_t GetTaskId();
    int64_t GetDownloadedNum();
    int64_t GetFailedNum();
    int64_t GetTotalNum();
    int64_t GetDownloadedSize();
    int64_t GetTotalSize();
    int32_t GetErrorType();
    ModuleFileIO::FsResult<std::vector<FailedFileInfo>> GetFailedFileList();
    ModuleFileIO::FsResult<std::vector<std::string>> GetDownloadedFileList();

    std::shared_ptr<BatchProgressAni> downloadProgress_{nullptr};
};
} // namespace OHOS::FileManagement::CloudSync
#endif // OHOS_FILEMGMT_MULTI_DOWNLOAD_PROGRESS_CORE_H