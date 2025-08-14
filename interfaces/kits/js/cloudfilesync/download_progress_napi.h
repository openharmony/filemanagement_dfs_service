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

#ifndef OHOS_FILEMGMT_DOWNLOAD_PROGRESS_NAPI_H
#define OHOS_FILEMGMT_DOWNLOAD_PROGRESS_NAPI_H

#include <string>
#include <unordered_map>
#include <unordered_set>

#include "cloud_sync_common.h"
#include "n_napi.h"

namespace OHOS::FileManagement::CloudSync {
class DlProgressNapi {
public:
    explicit DlProgressNapi(int64_t downloadId) : taskId_(downloadId) {}
    virtual ~DlProgressNapi() = default;
    virtual void Update(const DownloadProgressObj &progress) = 0;
    virtual napi_value ConvertToValue(napi_env env) = 0;
    virtual std::shared_ptr<DlProgressNapi> CreateNewObject() = 0;

    // Use default inline
    int64_t GetTaskId() const
    {
        return taskId_;
    }
    int64_t GetTotalSize() const
    {
        return totalSize_;
    }
    int64_t GetDownloadedSize() const
    {
        return downloadedSize_;
    }
    int32_t GetState() const
    {
        return state_;
    }
    int32_t GetErrorType() const
    {
        return errorType_;
    }
    bool IsNeedClean() const
    {
        return needClean_;
    }
    std::string GetUri() const
    {
        return uri_;
    }

protected:
    std::string uri_;
    // taskId_ is used to identify the download task, it is unique for each download task.
    int64_t taskId_{0};
    int64_t totalSize_{0};
    int64_t downloadedSize_{0};
    int32_t state_{0};
    int32_t errorType_{0};
    bool needClean_{false};
    std::mutex mtx_;
};

class SingleProgressNapi : public DlProgressNapi {
public:
    explicit SingleProgressNapi(int64_t downloadId) : DlProgressNapi(downloadId) {}
    void Update(const DownloadProgressObj &progress) override;
    napi_value ConvertToValue(napi_env env) override;
    std::shared_ptr<DlProgressNapi> CreateNewObject() override;
};

class BatchProgressNapi : public DlProgressNapi,
                          public std::enable_shared_from_this<BatchProgressNapi> {
public:
    explicit BatchProgressNapi(int64_t downloadId) : DlProgressNapi(downloadId) {}
    void Update(const DownloadProgressObj &progress) override;
    napi_value ConvertToValue(napi_env env) override;
    std::shared_ptr<DlProgressNapi> CreateNewObject() override;

    // Getters for batch download progress
    int64_t GetTotalNum() const
    {
        return totalNum_;
    }
    std::size_t GetSuccNum() const
    {
        return downloadedFiles_.size();
    }
    std::size_t GetFailedNum() const
    {
        return failedFiles_.size();
    }
    const std::unordered_set<std::string>& GetDownloadedFiles() const
    {
        return downloadedFiles_;
    }
    const std::unordered_map<std::string, int32_t>& GetFailedFiles() const
    {
        return failedFiles_;
    }

    // The maximum size of files list is 400, which may cause performance problems.
    void SetDownloadedFiles(const std::unordered_set<std::string> &fileList);
    void SetFailedFiles(const std::unordered_map<std::string, int32_t> &fileList);

private:
    int64_t totalNum_{0};
    std::unordered_set<std::string> downloadedFiles_;
    std::unordered_map<std::string, int32_t> failedFiles_;
};
} // namespace OHOS::FileManagement::CloudSync
#endif // OHOS_FILEMGMT_DOWNLOAD_PROGRESS_NAPI_H
