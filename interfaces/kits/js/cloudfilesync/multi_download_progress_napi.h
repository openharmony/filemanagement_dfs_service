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

#ifndef OHOS_FILEMGMT_MULTI_DOWNLOAD_PROGRESS_NAPI_H
#define OHOS_FILEMGMT_MULTI_DOWNLOAD_PROGRESS_NAPI_H

#include <memory>

#include "filemgmt_libn.h"

namespace OHOS::FileManagement::CloudSync {
class BatchProgressNapi;
class MultiDlProgressNapi final : public LibN::NExporter {
public:
    MultiDlProgressNapi(napi_env env, napi_value exports) : NExporter(env, exports) {}
    ~MultiDlProgressNapi() = default;

    bool Export() override;
    std::string GetClassName() override;
    static napi_value Constructor(napi_env env, napi_callback_info info);
    static napi_value GetStatus(napi_env env, napi_callback_info info);
    static napi_value GetTaskId(napi_env env, napi_callback_info info);
    static napi_value GetDownloadedNum(napi_env env, napi_callback_info info);
    static napi_value GetFailedNum(napi_env env, napi_callback_info info);
    static napi_value GetTotalNum(napi_env env, napi_callback_info info);
    static napi_value GetDownloadedSize(napi_env env, napi_callback_info info);
    static napi_value GetTotalSize(napi_env env, napi_callback_info info);
    static napi_value GetErrorType(napi_env env, napi_callback_info info);
    static napi_value GetFailedFileList(napi_env env, napi_callback_info info);
    static napi_value GetDownloadedFileList(napi_env env, napi_callback_info info);

    inline static const std::string className_ = "MultiDownloadProgress";
};

struct MultiDlProgressEntity {
    std::unique_ptr<BatchProgressNapi> downloadProgress{nullptr};
};
} // namespace OHOS::FileManagement::CloudSync
#endif // OHOS_FILEMGMT_MULTI_DOWNLOAD_PROGRESS_NAPI_H