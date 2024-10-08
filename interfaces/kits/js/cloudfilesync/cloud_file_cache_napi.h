/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "cloud_file_napi.h"

namespace OHOS::FileManagement::CloudSync {
class CloudFileCacheNapi final : public CloudFileNExporter {
public:
    CloudFileCacheNapi(napi_env env, napi_value exports) : CloudFileNExporter(env, exports) {}
    ~CloudFileCacheNapi() = default;

    bool Export() override;
    std::string GetClassName() override;
    static napi_value StartFileCache(napi_env env, napi_callback_info info);
    static napi_value StartBatchFileCache(napi_env env, napi_callback_info info);
    static napi_value StopFileCache(napi_env env, napi_callback_info info);
    static napi_value StopBatchFileCache(napi_env env, napi_callback_info info);
    static napi_value On(napi_env env, napi_callback_info info);
    static napi_value Off(napi_env env, napi_callback_info info);
    static napi_value CleanCloudFileCache(napi_env env, napi_callback_info info);

private:
    static inline std::shared_ptr<CloudDownloadCallbackImpl> callback_;
    inline static std::string className_ = "CloudFileCache";
};
} // namespace OHOS::FileManagement::CloudSync
#endif // OHOS_FILEMGMT_CLOUD_FILE_DOWNLOAD_NAPI_H