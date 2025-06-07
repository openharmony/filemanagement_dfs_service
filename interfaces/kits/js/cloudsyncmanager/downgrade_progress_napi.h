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

#ifndef OHOS_FILEMGMT_DOWNGRADE_PROGRESS_NAPI_H
#define OHOS_FILEMGMT_DOWNGRADE_PROGRESS_NAPI_H

#include <memory>
#include <unordered_map>
#include <unordered_set>

#include "cloud_sync_common.h"
#include "filemgmt_libn.h"

namespace OHOS::FileManagement::CloudSync {
class DowngradeProgressNapi final : public LibN::NExporter {
public:
    DowngradeProgressNapi(napi_env env, napi_value exports) : NExporter(env, exports) {}
    ~DowngradeProgressNapi() = default;

    bool Export() override;
    std::string GetClassName() override;
    static napi_value Constructor(napi_env env, napi_callback_info info);
    static napi_value GetState(napi_env env, napi_callback_info info);
    static napi_value GetSuccessfulCount(napi_env env, napi_callback_info info);
    static napi_value GetFailedCount(napi_env env, napi_callback_info info);
    static napi_value GetTotalCount(napi_env env, napi_callback_info info);
    static napi_value GetDownloadedSize(napi_env env, napi_callback_info info);
    static napi_value GetTotalSize(napi_env env, napi_callback_info info);
    static napi_value GetStopReason(napi_env env, napi_callback_info info);

    inline static const std::string className_ = "DownloadProgress";
};

struct SingleBundleProgress {
    int32_t state{0};
    int32_t successfulCount{0};
    int32_t failedCount{0};
    int64_t downloadedSize{0};
    int64_t totalSize{0};
    int32_t totalCount{0};
    int32_t stopReason{0};
};

struct DowngradeProgressEntity {
    std::shared_ptr<SingleBundleProgress> progress;
};
} // namespace OHOS::FileManagement::CloudSync
#endif // OHOS_FILEMGMT_DOWNGRADE_PROGRESS_NAPI_H