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
using namespace FileManagement::LibN;

const int32_t ARGS_ONE = 1;

class GallerySyncNapi final : public NExporter {
public:
    inline static const std::string className_ = "GallerySync";

    bool Export() override;
    std::string GetClassName() override;

    static napi_value Constructor(napi_env env, napi_callback_info info);

    static napi_value Start(napi_env env, napi_callback_info info);
    static napi_value Stop(napi_env env, napi_callback_info info);
    static napi_value OnCallback(napi_env env, napi_callback_info info);
    static napi_value OffCallback(napi_env env, napi_callback_info info);

    GallerySyncNapi(napi_env env, napi_value exports) : NExporter(env, exports) {};
    ~GallerySyncNapi() = default;
};

class CloudSyncCallbackImpl : public CloudSyncCallback {
public:
    CloudSyncCallbackImpl(napi_env env, napi_value fun);
    ~CloudSyncCallbackImpl();
    void OnSyncStateChanged(SyncType type, SyncPromptState state) override;
    void OnCloudSyncStateChanged(CloudSyncState state, ErrorType type) override;

    class UvChangeMsg {
    public:
        UvChangeMsg(napi_env env, napi_ref ref, CloudSyncState state, ErrorType type)
            : env_(env), ref_(ref), state_(state), type_(type) {}
        ~UvChangeMsg() {}
        napi_env env_;
        napi_ref ref_;
        CloudSyncState state_;
        ErrorType type_;
    };
private:
    napi_env env_;
    napi_ref cbOnRef_ = nullptr;
};
} // namespace OHOS::FileManagement::CloudSync
#endif // OHOS_FILEMGMT_GALLERY_SYNC_NAPI_H
