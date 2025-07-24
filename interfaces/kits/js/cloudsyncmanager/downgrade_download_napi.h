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

#ifndef OHOS_FILEMGMT_DOWNGRADE_DOWNLOAD_NAPI_H
#define OHOS_FILEMGMT_DOWNGRADE_DOWNLOAD_NAPI_H

#include <list>
#include <mutex>
#include <unordered_set>

#include "cloud_sync_common.h"
#include "downgrade_dl_callback.h"
#include "downgrade_progress_napi.h"
#include "filemgmt_libn.h"

namespace OHOS::FileManagement::CloudSync {
class DowngradeDownloadNapi final : public LibN::NExporter {
public:
    DowngradeDownloadNapi(napi_env env, napi_value exports) : NExporter(env, exports) {}
    ~DowngradeDownloadNapi() = default;

    bool Export() override;
    bool ToExport(std::vector<napi_property_descriptor> props);
    std::string GetClassName() override;
    static napi_value Constructor(napi_env env, napi_callback_info info);
    static napi_value GetCloudFileInfo(napi_env env, napi_callback_info info);
    static napi_value StartDownload(napi_env env, napi_callback_info info);
    static napi_value StopDownload(napi_env env, napi_callback_info info);

private:
    inline static std::string className_ = "DowngradeDownload";
};

class DowngradeDlCallbackImpl : public DowngradeDlCallback,
                                public std::enable_shared_from_this<DowngradeDlCallbackImpl> {
public:
    DowngradeDlCallbackImpl(napi_env env, napi_value func);
    ~DowngradeDlCallbackImpl();
    void OnDownloadProcess(const DowngradeProgress &progress) override;
    napi_value ConvertToValue();

public:
    napi_ref cbOnRef_;
    napi_env env_;

private:
    void UpdateDownloadProgress(const DowngradeProgress &progress);

private:
    std::shared_ptr<SingleBundleProgress> dlProgress_;
};

struct DowngradeEntity {
    explicit DowngradeEntity(std::string bundle) : bundleName(bundle) {}
    std::string bundleName;
    std::shared_ptr<DowngradeDlCallbackImpl> callbackImpl{nullptr};
};
} // namespace OHOS::FileManagement::CloudSync
#endif // OHOS_FILEMGMT_DOWNGRADE_DOWNLOAD_NAPI_H