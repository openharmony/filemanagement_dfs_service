/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_FILEMGMT_CLOUD_FILE_CACHE_NAPI_H
#define OHOS_FILEMGMT_CLOUD_FILE_CACHE_NAPI_H

#include <mutex>
#include <string>
#include <unordered_map>

#include "download_callback_middle_napi.h"
#include "download_progress_napi.h"
#include "filemgmt_libn.h"

namespace OHOS::FileManagement::CloudSync {
const std::string PROGRESS = "progress";
const std::string MULTI_PROGRESS = "batchDownload";
class CloudFileCacheNapi final : public LibN::NExporter {
public:
    CloudFileCacheNapi(napi_env env, napi_value exports) : NExporter(env, exports) {}
    ~CloudFileCacheNapi() = default;

    bool Export() override;
    bool ToExport(std::vector<napi_property_descriptor> props);
    std::string GetClassName() override;
    static napi_value Constructor(napi_env env, napi_callback_info info);
    static napi_value StartFileCache(napi_env env, napi_callback_info info);
    static napi_value StartBatchFileCache(napi_env env, napi_callback_info info);
    static napi_value StopFileCache(napi_env env, napi_callback_info info);
    static napi_value StopBatchFileCache(napi_env env, napi_callback_info info);
    static napi_value On(napi_env env, napi_callback_info info);
    static napi_value Off(napi_env env, napi_callback_info info);
    static napi_value CleanCloudFileCache(napi_env env, napi_callback_info info);
    static napi_value CleanFileCache(napi_env env, napi_callback_info info);

private:
    inline static std::string className_ = "CloudFileCache";
};

class CloudFileCacheCallbackImplNapi : public CloudDlCallbackMiddleNapi {
public:
    explicit CloudFileCacheCallbackImplNapi(napi_env env) : CloudDlCallbackMiddleNapi(env) {}
    int32_t StartDownloadInner(const std::string &uri, int32_t fieldKey);
    int32_t StartDownloadInner(const std::vector<std::string> &uriList, int64_t &downloadId, int32_t fieldKey);
    int32_t StopDownloadInner(const std::string &uri, bool needClean);
    int32_t StopDownloadInner(int64_t downloadId, bool needClean);
};

struct FileCacheEntity {
    std::unordered_map<std::string, std::shared_ptr<CloudFileCacheCallbackImplNapi>> registerMap;
};
} // namespace OHOS::FileManagement::CloudSync
#endif // OHOS_FILEMGMT_CLOUD_FILE_CACHE_NAPI_H