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

#include "cloud_file_cache_ani.h"

#include "ani_utils.h"
#include "dfs_error.h"
#include "error_handler.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {

using namespace arkts::ani_signature;

static CloudFileCacheCore *CloudFileCacheUnwrap(ani_env *env, ani_object object)
{
    ani_long nativePtr;
    auto ret = env->Object_GetFieldByName_Long(object, "nativePtr", &nativePtr);
    if (ret != ANI_OK) {
        LOGE("Unwrap cloudFileCacheCore err: %{public}d", static_cast<int32_t>(ret));
        return nullptr;
    }
    std::uintptr_t ptrValue = static_cast<std::uintptr_t>(nativePtr);
    CloudFileCacheCore *cloudFileCache = reinterpret_cast<CloudFileCacheCore *>(ptrValue);
    return cloudFileCache;
}

void CloudFileCacheAni::CloudFileCacheConstructor(ani_env *env, ani_object object)
{
    ani_namespace ns {};
    Namespace nsSign = Builder::BuildNamespace("@ohos.file.cloudSync.cloudSync");
    ani_status ret = env->FindNamespace(nsSign.Descriptor().c_str(), &ns);
    if (ret != ANI_OK) {
        LOGE("find namespace failed. ret = %{public}d", static_cast<int32_t>(ret));
        ErrorHandler::Throw(env, ENOMEM);
        return;
    }
    Type clsName = Builder::BuildClass("CloudFileCache");
    ani_class cls;
    ret = env->Namespace_FindClass(ns, clsName.Descriptor().c_str(), &cls);
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

    FsResult<CloudFileCacheCore *> data = CloudFileCacheCore::Constructor();
    if (!data.IsSuccess()) {
        LOGE("cloudFileCache constructor failed.");
        const auto &err = data.GetError();
        ErrorHandler::Throw(env, err);
        return;
    }

    const CloudFileCacheCore *cloudFileCache = data.GetData().value();
    ret = env->Object_CallMethod_Void(object, bindNativePtr, reinterpret_cast<ani_long>(cloudFileCache));
    if (ret != ANI_OK) {
        LOGE("bindNativePtr failed.");
        delete cloudFileCache;
        ErrorHandler::Throw(env, ENOMEM);
    }
}

void CloudFileCacheAni::CloudFileCacheOn(ani_env *env, ani_object object, ani_string evt, ani_object fun)
{
    std::string event;
    ani_status ret = ANIUtils::AniString2String(env, evt, event);
    if (ret != ANI_OK) {
        ErrorHandler::Throw(env, JsErrCode::E_IPCSS);
        return;
    }

    if (event == "multiProgress") {
        event = MULTI_PROGRESS;
    }
    if (event != PROGRESS && event != MULTI_PROGRESS) {
        LOGE("Invalid argument for event type.");
        ErrorHandler::Throw(env, JsErrCode::E_IPCSS);
        return;
    }

    auto cloudFileCache = CloudFileCacheUnwrap(env, object);
    if (cloudFileCache == nullptr) {
        LOGE("Cannot wrap cloudFileCache.");
        ErrorHandler::Throw(env, JsErrCode::E_IPCSS);
        return;
    }

    std::shared_ptr<CloudFileCacheCallbackImplAni> callbackImpl = cloudFileCache->GetCallbackImpl(event, true);
    callbackImpl->InitVm(env);
    auto status = callbackImpl->RegisterCallback(env, fun);
    if (status != ANI_OK) {
        LOGE("Failed to register callback, status: %{public}d.", status);
        ErrorHandler::Throw(env, JsErrCode::E_IPCSS);
        return;
    }
}

void CloudFileCacheAni::CloudFileCacheOff0(ani_env *env, ani_object object, ani_string evt, ani_object fun)
{
    std::string event;
    ani_status ret = ANIUtils::AniString2String(env, evt, event);
    if (ret != ANI_OK) {
        ErrorHandler::Throw(env, JsErrCode::E_IPCSS);
        return;
    }

    if (event == "multiProgress") {
        event = MULTI_PROGRESS;
    }
    if (event != PROGRESS && event != MULTI_PROGRESS) {
        LOGE("Invalid argument for event type.");
        ErrorHandler::Throw(env, JsErrCode::E_IPCSS);
        return;
    }

    auto cloudFileCache = CloudFileCacheUnwrap(env, object);
    if (cloudFileCache == nullptr) {
        LOGE("Cannot wrap cloudFileCache.");
        ErrorHandler::Throw(env, JsErrCode::E_IPCSS);
        return;
    }

    std::shared_ptr<CloudFileCacheCallbackImplAni> callbackImpl = cloudFileCache->GetCallbackImpl(event, false);
    if (callbackImpl == nullptr || callbackImpl->UnregisterCallback(env, fun) != ANI_OK) {
        LOGE("Failed to unregister callback.");
        ErrorHandler::Throw(env, JsErrCode::E_IPCSS);
        return;
    }
}

void CloudFileCacheAni::CloudFileCacheOff1(ani_env *env, ani_object object, ani_string evt)
{
    CloudFileCacheOff0(env, object, evt, nullptr);
}

void CloudFileCacheAni::CloudFileCacheStart(ani_env *env, ani_object object, ani_string uri)
{
    std::string uriInput;
    ani_status ret = ANIUtils::AniString2String(env, uri, uriInput);
    if (ret != ANI_OK) {
        ErrorHandler::Throw(env, JsErrCode::E_IPCSS);
        return;
    }

    auto cloudFileCache = CloudFileCacheUnwrap(env, object);
    if (cloudFileCache == nullptr) {
        LOGE("Cannot wrap cloudFileCache.");
        ErrorHandler::Throw(env, JsErrCode::E_IPCSS);
        return;
    }
    auto data = cloudFileCache->DoStart(uriInput);
    if (!data.IsSuccess()) {
        const auto &err = data.GetError();
        LOGE("cloudFileCache do start failed, ret = %{public}d", err.GetErrNo());
        ErrorHandler::Throw(env, err);
    }
}

ani_double CloudFileCacheAni::CloudFileCacheStartBatch(ani_env *env,
                                                       ani_object object,
                                                       ani_array_ref uriList,
                                                       ani_enum_item fileType)
{
    ani_double errResult = 0;
    auto [ret, urisInput] = ANIUtils::AniToStringArray(env, uriList);
    if (!ret) {
        ErrorHandler::Throw(env, JsErrCode::E_IPCSS);
        return errResult;
    }

    int32_t fieldKey = static_cast<int32_t>(FieldKey::FIELDKEY_CONTENT);
    tie(ret, fieldKey) = ANIUtils::EnumToInt32(env, fileType);
    if (!ret) {
        LOGE("cloudFileCache get fileType failed");
        ErrorHandler::Throw(env, JsErrCode::E_IPCSS);
        return errResult;
    }

    auto cloudFileCache = CloudFileCacheUnwrap(env, object);
    if (cloudFileCache == nullptr) {
        LOGE("Cannot wrap cloudFileCache.");
        ErrorHandler::Throw(env, JsErrCode::E_IPCSS);
        return errResult;
    }
    auto data = cloudFileCache->DoStart(urisInput, fieldKey);
    if (!data.IsSuccess()) {
        const auto &err = data.GetError();
        LOGE("cloudFileCache do start failed, ret = %{public}d", err.GetErrNo());
        ErrorHandler::Throw(env, err);
        return errResult;
    }

    return static_cast<ani_double>(data.GetData().value());
}

void CloudFileCacheAni::CloudFileCacheStop(ani_env *env, ani_object object, ani_string uri, ani_boolean needClean)
{
    std::string uriInput;
    ani_status ret = ANIUtils::AniString2String(env, uri, uriInput);
    if (ret != ANI_OK) {
        ErrorHandler::Throw(env, JsErrCode::E_IPCSS);
        return;
    }

    auto cloudFileCache = CloudFileCacheUnwrap(env, object);
    if (cloudFileCache == nullptr) {
        LOGE("Cannot wrap cloudFileCache.");
        ErrorHandler::Throw(env, JsErrCode::E_IPCSS);
        return;
    }

    bool needCleanInput = needClean;

    auto data = cloudFileCache->DoStop(uriInput, needCleanInput);
    if (!data.IsSuccess()) {
        const auto &err = data.GetError();
        LOGE("cloudFileCache do stop failed, ret = %{public}d", err.GetErrNo());
        ErrorHandler::Throw(env, err);
    }
}

void CloudFileCacheAni::CloudFileCacheStopBatch(ani_env *env,
                                                ani_object object,
                                                ani_double taskId,
                                                ani_boolean needClean)
{
    auto cloudFileCache = CloudFileCacheUnwrap(env, object);
    if (cloudFileCache == nullptr) {
        LOGE("Cannot wrap cloudFileCache.");
        ErrorHandler::Throw(env, JsErrCode::E_IPCSS);
        return;
    }

    bool needCleanInput = needClean;

    auto data = cloudFileCache->DoStop(static_cast<int64_t>(taskId), needCleanInput);
    if (!data.IsSuccess()) {
        const auto &err = data.GetError();
        LOGE("cloudFileCache do stop failed, ret = %{public}d", err.GetErrNo());
        ErrorHandler::Throw(env, err);
    }
}

void CloudFileCacheAni::CloudFileCacheCleanCache(ani_env *env, ani_object object, ani_string uri)
{
    std::string uriInput;
    ani_status ret = ANIUtils::AniString2String(env, uri, uriInput);
    if (ret != ANI_OK) {
        ErrorHandler::Throw(env, JsErrCode::E_IPCSS);
        return;
    }

    auto cloudFileCache = CloudFileCacheUnwrap(env, object);
    if (cloudFileCache == nullptr) {
        LOGE("Cannot wrap cloudFileCache.");
        ErrorHandler::Throw(env, JsErrCode::E_IPCSS);
        return;
    }
    auto data = cloudFileCache->CleanCache(uriInput);
    if (!data.IsSuccess()) {
        const auto &err = data.GetError();
        LOGE("cloudFileCache do cleanCache failed, ret = %{public}d", err.GetErrNo());
        ErrorHandler::Throw(env, err);
    }
}
} // namespace OHOS::FileManagement::CloudSync