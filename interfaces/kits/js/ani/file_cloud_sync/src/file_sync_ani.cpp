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

#include "file_sync_ani.h"

#include "ani_utils.h"
#include "cloud_sync_callback_ani.h"
#include "dfs_error.h"
#include "dfsu_access_token_helper.h"
#include "error_handler.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {

using namespace arkts::ani_signature;

static FileSyncCore *FileSyncUnwrap(ani_env *env, ani_object object)
{
    ani_long nativePtr;
    auto ret = env->Object_GetFieldByName_Long(object, "nativePtr", &nativePtr);
    if (ret != ANI_OK) {
        LOGE("Unwrap FileSyncCore err: %{public}d", static_cast<int32_t>(ret));
        return nullptr;
    }
    std::uintptr_t ptrValue = static_cast<std::uintptr_t>(nativePtr);
    FileSyncCore *fileSync = reinterpret_cast<FileSyncCore *>(ptrValue);
    return fileSync;
}

void FileSyncAni::FileSyncConstructor0(ani_env *env, ani_object object)
{
    ani_namespace ns {};
    Namespace nsSign = Builder::BuildNamespace("@ohos.file.cloudSync.cloudSync");
    ani_status ret = env->FindNamespace(nsSign.Descriptor().c_str(), &ns);
    if (ret != ANI_OK) {
        LOGE("find namespace failed. ret = %{public}d", static_cast<int32_t>(ret));
        ErrorHandler::Throw(env, static_cast<int32_t>(ret));
        return;
    }
    Type clsName = Builder::BuildClass("FileSync");
    ani_class cls;
    ret = env->Namespace_FindClass(ns, clsName.Descriptor().c_str(), &cls);
    if (ret != ANI_OK) {
        LOGE("find class failed. ret = %{public}d", static_cast<int32_t>(ret));
        ErrorHandler::Throw(env, static_cast<int32_t>(ret));
        return;
    }

    ani_method bindNativePtr;
    std::string bindSign = Builder::BuildSignatureDescriptor({Builder::BuildLong()});
    ret = env->Class_FindMethod(cls, "bindNativePtr", bindSign.c_str(), &bindNativePtr);
    if (ret != ANI_OK) {
        LOGE("find class ctor. ret = %{public}d", static_cast<int32_t>(ret));
        ErrorHandler::Throw(env, static_cast<int32_t>(ret));
        return;
    }

    FsResult<FileSyncCore *> data = FileSyncCore::Constructor();
    if (!data.IsSuccess()) {
        LOGE("filesync constructor failed.");
        const auto &err = data.GetError();
        ErrorHandler::Throw(env, err);
        return;
    }

    const FileSyncCore *fileSync = data.GetData().value();
    ret = env->Object_CallMethod_Void(object, bindNativePtr, reinterpret_cast<ani_long>(fileSync));
    if (ret != ANI_OK) {
        LOGE("bindNativePtr failed.");
        delete fileSync;
        ErrorHandler::Throw(env, static_cast<int32_t>(ret));
    }
}

void FileSyncAni::FileSyncConstructor1(ani_env *env, ani_object object, ani_string bundleName)
{
    std::string bnm;
    ani_status ret = ANIUtils::AniString2String(env, bundleName, bnm);
    if (ret != ANI_OK) {
        ErrorHandler::Throw(env, static_cast<int32_t>(ret));
        return;
    }

    ani_namespace ns {};
    Namespace nsSign = Builder::BuildNamespace("@ohos.file.cloudSync.cloudSync");
    ret = env->FindNamespace(nsSign.Descriptor().c_str(), &ns);
    if (ret != ANI_OK) {
        LOGE("find namespace failed. ret = %{public}d", static_cast<int32_t>(ret));
        ErrorHandler::Throw(env, static_cast<int32_t>(ret));
        return;
    }
    Type clsName = Builder::BuildClass("FileSync");
    ani_class cls;
    ret = env->Namespace_FindClass(ns, clsName.Descriptor().c_str(), &cls);
    if (ret != ANI_OK) {
        LOGE("find class failed. ret = %{public}d", static_cast<int32_t>(ret));
        ErrorHandler::Throw(env, static_cast<int32_t>(ret));
        return;
    }

    ani_method bindNativePtr;
    std::string bindSign = Builder::BuildSignatureDescriptor({Builder::BuildLong()});
    ret = env->Class_FindMethod(cls, "bindNativePtr", bindSign.c_str(), &bindNativePtr);
    if (ret != ANI_OK) {
        LOGE("find class ctor. ret = %{public}d", static_cast<int32_t>(ret));
        ErrorHandler::Throw(env, static_cast<int32_t>(ret));
        return;
    }

    FsResult<FileSyncCore *> data = FileSyncCore::Constructor(bnm);
    if (!data.IsSuccess()) {
        LOGE("FileSyncCore constructor failed.");
        const auto &err = data.GetError();
        ErrorHandler::Throw(env, err);
        return;
    }

    const FileSyncCore *fileSync = data.GetData().value();
    ret = env->Object_CallMethod_Void(object, bindNativePtr, reinterpret_cast<ani_long>(fileSync));
    if (ret != ANI_OK) {
        LOGE("bindNativePtr failed.");
        delete fileSync;
        ErrorHandler::Throw(env, static_cast<int32_t>(ret));
    }
}

void FileSyncAni::FileSyncOn(ani_env *env, ani_object object, ani_string evt, ani_object fun)
{
    std::string event;
    ani_status ret = ANIUtils::AniString2String(env, evt, event);
    if (ret != ANI_OK) {
        ErrorHandler::Throw(env, static_cast<int32_t>(ret));
        return;
    }

    auto fileSync = FileSyncUnwrap(env, object);
    if (fileSync == nullptr) {
        LOGE("Cannot wrap fileSync.");
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return;
    }

    ani_ref cbOnRef;
    ret = env->GlobalReference_Create(reinterpret_cast<ani_ref>(fun), &cbOnRef);
    if (ret != ANI_OK) {
        ErrorHandler::Throw(env, static_cast<int32_t>(ret));
        return;
    }
    auto callback = std::make_shared<CloudSyncCallbackAniImpl>(env, cbOnRef);

    auto data = fileSync->DoOn(event, callback);
    if (!data.IsSuccess()) {
        const auto &err = data.GetError();
        LOGE("file sync do on failed, ret = %{public}d", err.GetErrNo());
        ErrorHandler::Throw(env, err);
    }
}

void FileSyncAni::FileSyncOff0(ani_env *env, ani_object object, ani_string evt, ani_object fun)
{
    ani_ref cbOnRef;
    ani_status ret = env->GlobalReference_Create(reinterpret_cast<ani_ref>(fun), &cbOnRef);
    if (ret != ANI_OK) {
        LOGE("create callback ref failed. ret = %{public}d", ret);
        ErrorHandler::Throw(env, static_cast<int32_t>(ret));
        return;
    }
    auto callback = std::make_shared<CloudSyncCallbackAniImpl>(env, cbOnRef);

    std::string event;
    ret = ANIUtils::AniString2String(env, evt, event);
    if (ret != ANI_OK) {
        ErrorHandler::Throw(env, static_cast<int32_t>(ret));
        return;
    }

    auto fileSync = FileSyncUnwrap(env, object);
    if (fileSync == nullptr) {
        LOGE("Cannot wrap fileSync.");
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return;
    }
    auto data = fileSync->DoOff(event, callback);
    if (!data.IsSuccess()) {
        const auto &err = data.GetError();
        LOGE("file sync do off failed, ret = %{public}d", err.GetErrNo());
        ErrorHandler::Throw(env, err);
    }
}

void FileSyncAni::FileSyncOff1(ani_env *env, ani_object object, ani_string evt)
{
    std::string event;
    ani_status ret = ANIUtils::AniString2String(env, evt, event);
    if (ret != ANI_OK) {
        ErrorHandler::Throw(env, static_cast<int32_t>(ret));
        return;
    }

    auto fileSync = FileSyncUnwrap(env, object);
    if (fileSync == nullptr) {
        LOGE("Cannot wrap fileSync.");
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return;
    }
    auto data = fileSync->DoOff(event);
    if (!data.IsSuccess()) {
        const auto &err = data.GetError();
        LOGE("file sync do off failed, ret = %{public}d", err.GetErrNo());
        ErrorHandler::Throw(env, err);
    }
}

void FileSyncAni::FileSyncStart(ani_env *env, ani_object object)
{
    auto fileSync = FileSyncUnwrap(env, object);
    if (fileSync == nullptr) {
        LOGE("Cannot wrap fileSync.");
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return;
    }
    auto data = fileSync->DoStart();
    if (!data.IsSuccess()) {
        const auto &err = data.GetError();
        LOGE("file sync do start failed, ret = %{public}d", err.GetErrNo());
        ErrorHandler::Throw(env, err);
    }
}

void FileSyncAni::FileSyncStop(ani_env *env, ani_object object)
{
    auto fileSync = FileSyncUnwrap(env, object);
    if (fileSync == nullptr) {
        LOGE("Cannot wrap fileSync.");
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return;
    }
    auto data = fileSync->DoStop();
    if (!data.IsSuccess()) {
        const auto &err = data.GetError();
        LOGE("file sync do stop failed, ret = %{public}d", err.GetErrNo());
        ErrorHandler::Throw(env, err);
    }
}

ani_double FileSyncAni::FileSyncGetLastSyncTime(ani_env *env, ani_object object)
{
    auto fileSync = FileSyncUnwrap(env, object);
    if (fileSync == nullptr) {
        LOGE("Cannot wrap fileSync.");
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return 0;
    }

    auto data = fileSync->DoGetLastSyncTime();
    if (!data.IsSuccess()) {
        const auto &err = data.GetError();
        LOGE("file sync do GetLastSyncTime failed, ret = %{public}d", err.GetErrNo());
        ErrorHandler::Throw(env, err);
        return 0;
    }

    const int64_t lastSyncTime = data.GetData().value();
    return static_cast<ani_double>(lastSyncTime);
}
} // namespace OHOS::FileManagement::CloudSync