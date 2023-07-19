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

#ifndef OHOS_FILEMGMT_CLOUD_FILE_DOWNLOAD_NAPI_H
#define OHOS_FILEMGMT_CLOUD_FILE_DOWNLOAD_NAPI_H

#include "cloud_download_callback.h"
#include "filemgmt_libn.h"

namespace OHOS::FileManagement::CloudSync {

class CloudDownloadCallbackImpl;
class CloudFileDownloadNapi final : public LibN::NExporter {
public:
    CloudFileDownloadNapi(napi_env env, napi_value exports);
    ~CloudFileDownloadNapi() override;

    bool Export() override;
    std::string GetClassName() override;
    static napi_value Constructor(napi_env env, napi_callback_info info);
    static napi_value Start(napi_env env, napi_callback_info info);
    static napi_value On(napi_env env, napi_callback_info info);
    static napi_value Off(napi_env env, napi_callback_info info);
    static napi_value Stop(napi_env env, napi_callback_info info);

private:
    static inline std::shared_ptr<CloudDownloadCallbackImpl> callback_;
    inline static const std::string className = "Download";
};

class CloudDownloadCallbackImpl : public CloudDownloadCallback {
public:
    CloudDownloadCallbackImpl(napi_env env, napi_value fun);
    ~CloudDownloadCallbackImpl();
    void OnDownloadProcess(DownloadProgressObj &progress) override;

    class UvChangeMsg {
    public:
        UvChangeMsg(napi_env env, napi_ref ref, DownloadProgressObj downloadProgress)
            : env_(env), ref_(ref), downloadProgress_(downloadProgress) {}
        ~UvChangeMsg() {}
        napi_env env_;
        napi_ref ref_;
        DownloadProgressObj downloadProgress_;
    };

    class UvDeleteMsg {
    public:
        UvDeleteMsg(napi_env env, napi_ref ref) : env_(env), ref_(ref) {}
        ~UvDeleteMsg() {}
        napi_env env_;
        napi_ref ref_;
    };
private:
    napi_env env_;
    napi_ref cbOnRef_ = nullptr;
};
} // namespace OHOS::FileManagement::CloudSync
#endif // OHOS_FILEMGMT_CLOUD_FILE_DOWNLOAD_NAPI_H