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

#ifndef OHOS_FILEMGMT_CLOUD_FILE_VERSION_NAPI_H
#define OHOS_FILEMGMT_CLOUD_FILE_VERSION_NAPI_H

#include "cloud_download_callback.h"
#include "filemgmt_libn.h"

namespace OHOS::FileManagement::CloudSync {

class FileVersionNapi : public LibN::NExporter {
public:
    FileVersionNapi(napi_env env, napi_value exports) : NExporter(env, exports) {}
    ~FileVersionNapi() = default;

    std::string GetClassName() override;
    bool Export() override;

    static napi_value Constructor(napi_env env, napi_callback_info info);
    static napi_value GetHistoryVersionList(napi_env env, napi_callback_info info);
    static napi_value DownloadHistoryVersion(napi_env env, napi_callback_info info);
    static napi_value ReplaceFileWithHistoryVersion(napi_env env, napi_callback_info info);
    static napi_value IsConflict(napi_env env, napi_callback_info info);
    static napi_value ClearFileConflict(napi_env env, napi_callback_info info);

protected:
    static inline std::string className_ = "FileVersion";
};

class VersionDownloadCallbackImpl : public CloudDownloadCallback,
                                  public std::enable_shared_from_this<VersionDownloadCallbackImpl> {
public:
    VersionDownloadCallbackImpl(napi_env env, napi_value fun);
    ~VersionDownloadCallbackImpl();
    void OnDownloadProcess(const DownloadProgressObj &progress) override;

    class UvChangeMsg {
    public:
        UvChangeMsg(std::shared_ptr<VersionDownloadCallbackImpl> downloadCallbackIn,
                    DownloadProgressObj downloadProgress)
            : downloadCallback_(downloadCallbackIn), downloadProgress_(downloadProgress)
        {
        }
        ~UvChangeMsg() {}
        std::weak_ptr<VersionDownloadCallbackImpl> downloadCallback_;
        DownloadProgressObj downloadProgress_;
    };

private:
    static void OnComplete(UvChangeMsg *msg);
    napi_env env_;
    napi_ref cbOnRef_ = nullptr;
};

struct FileVersionEntity {
    std::shared_ptr<VersionDownloadCallbackImpl> callbackImpl{nullptr};
};
} // OHOS::FileManagement::CloudSync
#endif  // OHOS_FILEMGMT_CLOUD_FILE_VERSION_NAPI_H