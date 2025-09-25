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

#include "cloud_download_ani.h"

#include "ani_utils.h"
#include "error_handler.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {

using namespace arkts::ani_signature;

const int32_t E_IPCSS = 13600001;

static CloudFileCore *CloudDownloadUnwrap(ani_env *env, ani_object object)
{
    ani_long nativePtr;
    auto ret = env->Object_GetFieldByName_Long(object, "nativePtr", &nativePtr);
    if (ret != ANI_OK) {
        LOGE("Unwrap cloudsyncCore err: %{public}d", static_cast<int32_t>(ret));
        return nullptr;
    }
    std::uintptr_t ptrValue = static_cast<std::uintptr_t>(nativePtr);
    CloudFileCore *cloudDownload = reinterpret_cast<CloudFileCore *>(ptrValue);
    return cloudDownload;
}

void CloudDownloadAni::DownloadConstructor(ani_env *env, ani_object object)
{
    Type clsName = Builder::BuildClass("@ohos.file.cloudSync.cloudSync.Download");
    ani_class cls;
    ani_status ret = env->FindClass(clsName.Descriptor().c_str(), &cls);
    if (ret != ANI_OK) {
        LOGE("find class failed. ret = %{public}d", static_cast<int32_t>(ret));
        ErrorHandler::Throw(env, ENOMEM);
        return;
    }

    ani_method bindNativePtr;
    std::string bindSign = Builder::BuildSignatureDescriptor({Builder::BuildLong()});
    ret = env->Class_FindMethod(cls, "bindNativePtr", bindSign.c_str(), &bindNativePtr);
    if (ret != ANI_OK) {
        LOGE("find class ctor. ret = %{public}d", static_cast<int32_t>(ret));
        ErrorHandler::Throw(env, ENOMEM);
        return;
    }

    FsResult<CloudFileCore *> data = CloudFileCore::Constructor();
    if (!data.IsSuccess()) {
        LOGE("cloudfile constructor failed.");
        const auto &err = data.GetError();
        ErrorHandler::Throw(env, err);
        return;
    }

    auto cloudFile = data.GetData().value();
    ret = env->Object_CallMethod_Void(object, bindNativePtr, reinterpret_cast<ani_long>(cloudFile));
    if (ret != ANI_OK) {
        LOGE("bindNativePtr failed.");
        ErrorHandler::Throw(env, ENOMEM);
        delete cloudFile;
    }
}

void CloudDownloadAni::DownloadOn(ani_env *env, ani_object object, ani_object fun)
{
    ani_ref cbOnRef;
    ani_status ret = env->GlobalReference_Create(reinterpret_cast<ani_ref>(fun), &cbOnRef);
    if (ret != ANI_OK) {
        ErrorHandler::Throw(env, E_IPCSS);
        return;
    }
    auto callback = std::make_shared<CloudDownloadCallbackAniImpl>(env, cbOnRef);

    auto cloudDownload = CloudDownloadUnwrap(env, object);
    if (cloudDownload == nullptr) {
        LOGE("Cannot wrap cloudDownload.");
        ErrorHandler::Throw(env, E_IPCSS);
        return;
    }
    auto data = cloudDownload->DoOn(EVENT_TYPE, callback);
    if (!data.IsSuccess()) {
        const auto &err = data.GetError();
        LOGE("cloud download do on failed, ret = %{public}d", err.GetErrNo());
        ErrorHandler::Throw(env, err);
    }
}

void CloudDownloadAni::DownloadOff0(ani_env *env, ani_object object, ani_object fun)
{
    ani_ref cbOnRef;
    ani_status ret = env->GlobalReference_Create(reinterpret_cast<ani_ref>(fun), &cbOnRef);
    if (ret != ANI_OK) {
        ErrorHandler::Throw(env, E_IPCSS);
        return;
    }
    auto callback = std::make_shared<CloudDownloadCallbackAniImpl>(env, cbOnRef);

    auto cloudDownload = CloudDownloadUnwrap(env, object);
    if (cloudDownload == nullptr) {
        LOGE("Cannot wrap cloudDownload.");
        ErrorHandler::Throw(env, E_IPCSS);
        return;
    }
    auto data = cloudDownload->DoOff(EVENT_TYPE, callback);
    if (!data.IsSuccess()) {
        const auto &err = data.GetError();
        LOGE("cloud download do off failed, ret = %{public}d", err.GetErrNo());
        ErrorHandler::Throw(env, err);
    }
}

void CloudDownloadAni::DownloadOff1(ani_env *env, ani_object object)
{
    auto cloudDownload = CloudDownloadUnwrap(env, object);
    if (cloudDownload == nullptr) {
        LOGE("Cannot wrap cloudDownload.");
        ErrorHandler::Throw(env, E_IPCSS);
        return;
    }
    auto data = cloudDownload->DoOff(EVENT_TYPE);
    if (!data.IsSuccess()) {
        const auto &err = data.GetError();
        LOGE("cloud download do off failed, ret = %{public}d", err.GetErrNo());
        ErrorHandler::Throw(env, err);
    }
}

void CloudDownloadAni::DownloadStart(ani_env *env, ani_object object, ani_string uri)
{
    std::string uriInput;
    ani_status ret = ANIUtils::AniString2String(env, uri, uriInput);
    if (ret != ANI_OK) {
        ErrorHandler::Throw(env, E_PERMISSION);
        return;
    }

    auto cloudDownload = CloudDownloadUnwrap(env, object);
    if (cloudDownload == nullptr) {
        LOGE("Cannot wrap cloudDownload.");
        ErrorHandler::Throw(env, E_PERMISSION);
        return;
    }
    auto data = cloudDownload->DoStart(uriInput);
    if (!data.IsSuccess()) {
        const auto &err = data.GetError();
        LOGE("cloud download do start failed, ret = %{public}d", err.GetErrNo());
        ErrorHandler::Throw(env, err);
    }
}

void CloudDownloadAni::DownloadStop(ani_env *env, ani_object object, ani_string uri)
{
    std::string uriInput;
    ani_status ret = ANIUtils::AniString2String(env, uri, uriInput);
    if (ret != ANI_OK) {
        ErrorHandler::Throw(env, E_PERMISSION);
        return;
    }

    auto cloudDownload = CloudDownloadUnwrap(env, object);
    if (cloudDownload == nullptr) {
        LOGE("Cannot wrap cloudDownload.");
        ErrorHandler::Throw(env, E_PERMISSION);
        return;
    }
    auto data = cloudDownload->DoStop(uriInput);
    if (!data.IsSuccess()) {
        const auto &err = data.GetError();
        LOGE("cloud download do stop failed, ret = %{public}d", err.GetErrNo());
        ErrorHandler::Throw(env, err);
    }
}
}