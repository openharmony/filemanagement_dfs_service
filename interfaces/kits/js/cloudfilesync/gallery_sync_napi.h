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

#ifndef OHOS_FILEMGMT_GALLERY_SYNC_NAPI_H
#define OHOS_FILEMGMT_GALLERY_SYNC_NAPI_H

#include "cloud_sync_callback.h"
#include "filemgmt_libn.h"

namespace OHOS::FileManagement::CloudSync {
const int32_t ARGS_ONE = 1;

class CloudSyncCallbackImpl;
class GallerySyncNapi final : public LibN::NExporter {
public:
    bool Export() override;
    std::string GetClassName() override;

    static napi_value Constructor(napi_env env, napi_callback_info info);

    static napi_value Start(napi_env env, napi_callback_info info);
    static napi_value Stop(napi_env env, napi_callback_info info);
    static napi_value OnCallback(napi_env env, napi_callback_info info);
    static napi_value OffCallback(napi_env env, napi_callback_info info);

    GallerySyncNapi(napi_env env, napi_value exports) : NExporter(env, exports) {};
    ~GallerySyncNapi() = default;

private:
    static inline std::shared_ptr<CloudSyncCallbackImpl> callback_;
    inline static const std::string className_ = "GallerySync";
};

class CloudSyncCallbackImpl : public CloudSyncCallback, public std::enable_shared_from_this<CloudSyncCallbackImpl> {
public:
    CloudSyncCallbackImpl(napi_env env, napi_value fun);
    ~CloudSyncCallbackImpl() = default;
    void OnSyncStateChanged(SyncType type, SyncPromptState state) override;
    void OnSyncStateChanged(CloudSyncState state, ErrorType error) override;
    void DeleteReference();

    class UvChangeMsg {
    public:
        UvChangeMsg(std::shared_ptr<CloudSyncCallbackImpl> cloudSyncCallbackIn, CloudSyncState state, ErrorType error)
            : cloudSyncCallback_(cloudSyncCallbackIn), state_(state), error_(error)
        {
        }
        ~UvChangeMsg() {}
        std::weak_ptr<CloudSyncCallbackImpl> cloudSyncCallback_;
        CloudSyncState state_;
        ErrorType error_;
    };

private:
    static void OnComplete(UvChangeMsg *msg);
    napi_env env_;
    napi_ref cbOnRef_ = nullptr;
};
} // namespace OHOS::FileManagement::CloudSync
#endif // OHOS_FILEMGMT_GALLERY_SYNC_NAPI_H
