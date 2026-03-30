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

#ifndef OHOS_FILEMGMT_FILE_SYNC_NAPI_H
#define OHOS_FILEMGMT_FILE_SYNC_NAPI_H

#include "cloud_sync_common.h"
#include "cloud_sync_constants.h"
#include "gallery_sync_napi.h"
#include "cloud_sync_napi.h"
#include "register_callback_manager_napi.h"
#include "cloud_upload_callback.h"

namespace OHOS::FileManagement::CloudSync {
class CloudUploadCallbackImpl;
class FileSyncNapi final : public CloudSyncNapi {
public:
    bool Export() override;
    static napi_value GetLastSyncTime(napi_env env, napi_callback_info info);
    FileSyncNapi(napi_env env, napi_value exports) : CloudSyncNapi(env, exports) {};
    ~FileSyncNapi() = default;

    static napi_value Start(napi_env env, napi_callback_info info);
    static napi_value Stop(napi_env env, napi_callback_info info);
    static napi_value OnCallback(napi_env env, napi_callback_info info);
    static napi_value OffCallback(napi_env env, napi_callback_info info);
    static napi_value RegisterUploadProgress(napi_env env, napi_callback_info info);
    static napi_value UnRegisterUploadProgress(napi_env env, napi_callback_info info);
    static napi_value GetUploadList(napi_env env, napi_callback_info info);
};

class CloudUploadCallbackImpl : public CloudUploadCallback,
                                public std::enable_shared_from_this<CloudUploadCallbackImpl> {
public:
    CloudUploadCallbackImpl(napi_env env, napi_value fun);
    ~CloudUploadCallbackImpl();
    void OnUploadProgress(const UploadProgressObj& progress) override;
    void DeleteReference() override;
    void DeleteReferenceAsync();

    class UvChangeMsg {
    public:
        UvChangeMsg(std::shared_ptr<CloudUploadCallbackImpl> cloudUploadCallbackIn, const UploadProgressObj& progress)
            : cloudUploadCallback_(cloudUploadCallbackIn), progress_(progress)
        {
        }
        ~UvChangeMsg() {}
        std::weak_ptr<CloudUploadCallbackImpl> cloudUploadCallback_;
        UploadProgressObj progress_;
    };

private:
    static void OnComplete(UvChangeMsg *msg);
    napi_env env_;
    std::shared_ptr<AutoRef> cbOnRef_{nullptr};
    static inline std::string taskName_ = "fileSync.UploadProgress";
};
} // namespace OHOS::FileManagement::CloudSync
#endif // OHOS_FILEMGMT_FILE_SYNC_NAPI_H
