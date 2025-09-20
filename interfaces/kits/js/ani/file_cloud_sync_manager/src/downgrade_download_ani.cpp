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

#include "downgrade_download_ani.h"

#include "ani_utils.h"
#include "dfs_error.h"
#include "utils_log.h"
#include "error_handler.h"

namespace OHOS::FileManagement::CloudSync {
using namespace std;
using namespace arkts::ani_signature;

static DowngradeDownloadCore *DowngradeDownloadUnwrap(ani_env *env, ani_object object)
{
    ani_long nativePtr;
    auto ret = env->Object_GetFieldByName_Long(object, "nativePtr", &nativePtr);
    if (ret != ANI_OK) {
        LOGE("Unwrap DowngradeDownloadCore err: %{public}d", static_cast<int32_t>(ret));
        return nullptr;
    }
    uintptr_t ptrValue = static_cast<uintptr_t>(nativePtr);
    DowngradeDownloadCore *downgradeDlCore = reinterpret_cast<DowngradeDownloadCore *>(ptrValue);
    return downgradeDlCore;
}

void DowngradeDownloadAni::DowngradeDownloadConstructor(ani_env *env, ani_object object, ani_string bundleName)
{
    string bnm;
    ani_status ret = ANIUtils::AniString2String(env, bundleName, bnm);
    if (ret != ANI_OK) {
        ErrorHandler::Throw(env, JsErrCode::E_INNER_FAILED);
        return;
    }

    Type clsName = Builder::BuildClass("@ohos.file.cloudSyncManager.cloudSyncManager.DowngradeDownload");
    ani_class cls;
    ret = env->FindClass(clsName.Descriptor().c_str(), &cls);
    if (ret != ANI_OK) {
        LOGE("find class failed, ret = %{public}d", static_cast<int32_t>(ret));
        ErrorHandler::Throw(env, JsErrCode::E_INNER_FAILED);
        return;
    }

    ani_method bindNativePtr;
    string bindSign = Builder::BuildSignatureDescriptor({Builder::BuildLong()});
    ret = env->Class_FindMethod(cls, "bindNativePtr", bindSign.c_str(), &bindNativePtr);
    if (ret != ANI_OK) {
        LOGE("find class ctor failed, ret = %{public}d", static_cast<int32_t>(ret));
        ErrorHandler::Throw(env, JsErrCode::E_INNER_FAILED);
        return;
    }

    FsResult<DowngradeDownloadCore *> data = DowngradeDownloadCore::Constructor(bnm);
    if (!data.IsSuccess()) {
        LOGE("DowngradeDownloadCore constructor failed.");
        const auto &err = data.GetError();
        ErrorHandler::Throw(env, err);
        return;
    }

    const DowngradeDownloadCore *downgradeDlSync = data.GetData().value();
    ret = env->Object_CallMethod_Void(object, bindNativePtr, reinterpret_cast<ani_long>(downgradeDlSync));
    if (ret != ANI_OK) {
        LOGE("bindNativePtr failed.");
        delete downgradeDlSync;
        ErrorHandler::Throw(env, JsErrCode::E_INNER_FAILED);
    }
}

ani_ref DowngradeDownloadAni::DowngradeDownloadGetCloudFileInfo(ani_env *env, ani_object object)
{
    auto downgradeDlSync = DowngradeDownloadUnwrap(env, object);
    if (downgradeDlSync == nullptr) {
        LOGE("Cannot wrap downgradeDlSync.");
        ErrorHandler::Throw(env, JsErrCode::E_INNER_FAILED);
        return nullptr;
    }

    auto data = downgradeDlSync->DoDowngradeDlGetCloudFileInfo();
    if (!data.IsSuccess()) {
        const auto &err = data.GetError();
        LOGE("downgrade download do getCloudFileInfo failed, ret = %{public}d", err.GetErrNo());
        ErrorHandler::Throw(env, err);
        return nullptr;
    }

    Type clsName = Builder::BuildClass("@ohos.file.cloudSyncManager.cloudSyncManager.CloudFileInfoInner");
    ani_class cls;
    ani_status ret = env->FindClass(clsName.Descriptor().c_str(), &cls);
    if (ret != ANI_OK) {
        LOGE("find class failed, ret = %{public}d", static_cast<int32_t>(ret));
        ErrorHandler::Throw(env, JsErrCode::E_INNER_FAILED);
        return nullptr;
    }
    ani_method ctor;
    string ct = Builder::BuildConstructorName();
    string argSign = Builder::BuildSignatureDescriptor({
        Builder::BuildInt(), Builder::BuildLong(), Builder::BuildInt(),
        Builder::BuildLong(), Builder::BuildInt(), Builder::BuildLong()
    });
    ret = env->Class_FindMethod(cls, ct.c_str(), argSign.c_str(), &ctor);
    if (ret != ANI_OK) {
        LOGE("Find ctor method failed, ret = %{public}d", ret);
        return nullptr;
    }
    
    CloudFileInfo cloudFile = data.GetData().value();
    ani_object obj;
    ret = env->Object_New(cls, ctor, &obj, static_cast<double>(cloudFile.cloudfileCount),
        static_cast<double>(cloudFile.cloudFileTotalSize), static_cast<double>(cloudFile.localFileCount),
        static_cast<double>(cloudFile.localFileTotalSize), static_cast<double>(cloudFile.bothFileCount),
        static_cast<double>(cloudFile.bothFileTotalSize));
    if (ret != ANI_OK) {
        LOGE("Create new object failed. ret = %{public}d", ret);
        return nullptr;
    }
    return static_cast<ani_ref>(obj);
}

void DowngradeDownloadAni::DowngradeDownloadStartDownload(ani_env *env, ani_object object, ani_object fun)
{
    auto downgradeDlSync = DowngradeDownloadUnwrap(env, object);
    if (downgradeDlSync == nullptr) {
        LOGE("Cannot wrap downgradeDlSync.");
        ErrorHandler::Throw(env, JsErrCode::E_INNER_FAILED);
        return;
    }

    ani_ref cbOnRef;
    ani_status ret = env->GlobalReference_Create(reinterpret_cast<ani_ref>(fun), &cbOnRef);
    if (ret != ANI_OK) {
        ErrorHandler::Throw(env, JsErrCode::E_INNER_FAILED);
        return;
    }

    auto callback = make_shared<DowngradeCallbackAniImpl>(env, cbOnRef);
    auto data = downgradeDlSync->DoDowngradeDlStartDownload(callback);
    if (!data.IsSuccess()) {
        const auto &err = data.GetError();
        LOGE("downgrade download do downgradeDlStartDownload failed, ret = %{public}d", err.GetErrNo());
        ErrorHandler::Throw(env, err);
    }
}

void DowngradeDownloadAni::DowngradeDownloadStopDownload(ani_env *env, ani_object object)
{
    auto downgradeDlSync = DowngradeDownloadUnwrap(env, object);
    if (downgradeDlSync == nullptr) {
        LOGE("Cannot wrap downgradeDlSync.");
        ErrorHandler::Throw(env, JsErrCode::E_INNER_FAILED);
        return;
    }
    auto data = downgradeDlSync->DoDowngradeDlStopDownload();
    if (!data.IsSuccess()) {
        const auto &err = data.GetError();
        LOGE("downgrade download do downgradeDlStopDownload failed, ret = %{public}d", err.GetErrNo());
        ErrorHandler::Throw(env, err);
    }
}
} // namespace OHOS::FileManagement::CloudSync