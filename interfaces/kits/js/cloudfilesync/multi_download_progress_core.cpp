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

#include "multi_download_progress_core.h"

#include <memory>

#include "cloud_sync_common.h"
#include "dfs_error.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
using namespace ModuleFileIO;
using namespace std;
FsResult<MultiDlProgressCore *> MultiDlProgressCore::Constructor()
{
    std::unique_ptr<MultiDlProgressCore> dlProgress = std::make_unique<MultiDlProgressCore>();
    return FsResult<MultiDlProgressCore *>::Success(dlProgress.release());
}

int32_t MultiDlProgressCore::GetStatus()
{
    int32_t state = static_cast<int32_t>(DownloadProgressObj::Status::FAILED);
    if (downloadProgress_ == nullptr) {
        LOGE("Failed to get multiDlProgress.");
    } else {
        state = downloadProgress_->GetState();
    }

    return state;
}

int64_t MultiDlProgressCore::GetTaskId()
{
    int64_t taskId = 0;
    if (downloadProgress_ == nullptr) {
        LOGE("Failed to get MultiDlProgressEntity.");
    } else {
        taskId = downloadProgress_->GetTaskId();
    }

    return taskId;
}

int64_t MultiDlProgressCore::GetDownloadedNum()
{
    int64_t succNum = -1;
    if (downloadProgress_ == nullptr) {
        LOGE("Failed to get MultiDlProgressEntity.");
    } else {
        succNum = static_cast<int64_t>(downloadProgress_->GetSuccNum());
    }

    return succNum;
}

int64_t MultiDlProgressCore::GetFailedNum()
{
    int64_t failedNum = -1;
    if (downloadProgress_ == nullptr) {
        LOGE("Failed to get MultiDlProgressEntity.");
    } else {
        failedNum = static_cast<int64_t>(downloadProgress_->GetFailedNum());
    }

    return failedNum;
}

int64_t MultiDlProgressCore::GetTotalNum()
{
    int64_t totalNum = -1;
    if (downloadProgress_ == nullptr) {
        LOGE("Failed to get MultiDlProgressEntity.");
    } else {
        totalNum = downloadProgress_->GetTotalNum();
    }

    return totalNum;
}

int64_t MultiDlProgressCore::GetDownloadedSize()
{
    int64_t downloadSize = INT64_MAX;
    if (downloadProgress_ == nullptr) {
        LOGE("Failed to get MultiDlProgressEntity.");
    } else {
        downloadSize = downloadProgress_->GetDownloadedSize();
    }

    return downloadSize;
}

int64_t MultiDlProgressCore::GetTotalSize()
{
    int64_t totalSize = INT64_MAX;
    if (downloadProgress_ == nullptr) {
        LOGE("Failed to get MultiDlProgressEntity.");
    } else {
        totalSize = downloadProgress_->GetTotalSize();
    }

    return totalSize;
}

int32_t MultiDlProgressCore::GetErrorType()
{
    int32_t errorType = static_cast<int32_t>(DownloadProgressObj::DownloadErrorType::UNKNOWN_ERROR);
    if (downloadProgress_ == nullptr) {
        LOGE("Failed to get MultiDlProgressEntity.");
    } else {
        errorType = downloadProgress_->GetErrorType();
    }

    return errorType;
}

FsResult<std::vector<FailedFileInfo>> MultiDlProgressCore::GetFailedFileList()
{
    if (downloadProgress_ == nullptr) {
        LOGE("Failed to get MultiDlProgressEntity.");
        return FsResult<std::vector<FailedFileInfo>>::Error(Convert2ErrNum(E_SERVICE_INNER_ERROR));
    }
    auto failedFiles = downloadProgress_->GetFailedFiles();
    std::vector<FailedFileInfo> res;
    for (auto &iter : failedFiles) {
        res.emplace_back(iter.first, iter.second);
    }

    return FsResult<std::vector<FailedFileInfo>>::Success(res);
}

FsResult<std::vector<std::string>> MultiDlProgressCore::GetDownloadedFileList()
{
    if (downloadProgress_ == nullptr) {
        LOGE("Failed to get MultiDlProgressEntity.");
        return FsResult<std::vector<std::string>>::Error(Convert2ErrNum(E_SERVICE_INNER_ERROR));
    }

    auto downloadedFiles = downloadProgress_->GetDownloadedFiles();
    std::vector<string> res(downloadedFiles.begin(), downloadedFiles.end());
    return FsResult<std::vector<std::string>>::Success(res);
}

void MultiDlProgressCore::SetProgress(std::unique_ptr<BatchProgressAni> progress)
{
    downloadProgress_ = std::move(progress);
}
} // namespace OHOS::FileManagement::CloudSync