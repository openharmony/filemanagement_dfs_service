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

#ifndef OHOS_FILEMGMT_CLOUD_FILE_NAPI_H
#define OHOS_FILEMGMT_CLOUD_FILE_NAPI_H

#include "cloud_download_callback.h"
#include "filemgmt_libn.h"

namespace OHOS::FileManagement::CloudSync {

class CloudDownloadCallbackImpl;
class CloudFileNapi : public LibN::NExporter {
public:
    CloudFileNapi(napi_env env, napi_value exports);
    ~CloudFileNapi() = default;

    virtual bool Export() override;
    bool ToExport(std::vector<napi_property_descriptor> props);
    virtual void SetClassName(std::string classname);
    std::string GetClassName() override;
    static napi_value Constructor(napi_env env, napi_callback_info info);
    static napi_value Start(napi_env env, napi_callback_info info);
    static napi_value On(napi_env env, napi_callback_info info);
    static napi_value Off(napi_env env, napi_callback_info info);
    static napi_value Stop(napi_env env, napi_callback_info info);

private:
    static inline std::shared_ptr<CloudDownloadCallbackImpl> callback_;
    inline static std::string className_ = "CloudFileNapi";
};

class CloudDownloadCallbackImpl : public CloudDownloadCallback,
                                  public std::enable_shared_from_this<CloudDownloadCallbackImpl> {
public:
    CloudDownloadCallbackImpl(napi_env env, napi_value fun);
    ~CloudDownloadCallbackImpl() = default;
    void OnDownloadProcess(const DownloadProgressObj &progress) override;
    void DeleteReference();

    class UvChangeMsg {
    public:
        UvChangeMsg(std::shared_ptr<CloudDownloadCallbackImpl> CloudDownloadCallbackIn,
                    DownloadProgressObj downloadProgress)
            : CloudDownloadCallback_(CloudDownloadCallbackIn), downloadProgress_(downloadProgress)
        {
        }
        ~UvChangeMsg() {}
        std::weak_ptr<CloudDownloadCallbackImpl> CloudDownloadCallback_;
        DownloadProgressObj downloadProgress_;
    };

private:
    static void OnComplete(UvChangeMsg *msg);
    napi_env env_;
    napi_ref cbOnRef_ = nullptr;
};
} // namespace OHOS::FileManagement::CloudSync
#endif // OHOS_FILEMGMT_CLOUD_FILE_NAPI_H