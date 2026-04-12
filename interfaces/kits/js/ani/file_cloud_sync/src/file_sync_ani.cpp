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
#include "upload_callback_impl_ani.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {

using namespace arkts::ani_signature;

const int32_t E_IPCSS = 13600001;

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
    Type clsName = Builder::BuildClass("@ohos.file.cloudSync.cloudSync.FileSync");
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
        ErrorHandler::Throw(env, ENOMEM);
    }
}

void FileSyncAni::FileSyncConstructor1(ani_env *env, ani_object object, ani_string bundleName)
{
    std::string bnm;
    ani_status ret = ANIUtils::AniString2String(env, bundleName, bnm);
    if (ret != ANI_OK) {
        ErrorHandler::Throw(env, ENOMEM);
        return;
    }

    Type clsName = Builder::BuildClass("@ohos.file.cloudSync.cloudSync.FileSync");
    ani_class cls;
    ret = env->FindClass(clsName.Descriptor().c_str(), &cls);
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
        ErrorHandler::Throw(env, ENOMEM);
    }
}

void FileSyncAni::FileSyncOn(ani_env *env, ani_object object, ani_object fun)
{
    auto fileSync = FileSyncUnwrap(env, object);
    if (fileSync == nullptr) {
        LOGE("Cannot wrap fileSync.");
        ErrorHandler::Throw(env, E_IPCSS);
        return;
    }

    ani_ref cbOnRef;
    ani_status ret = env->GlobalReference_Create(reinterpret_cast<ani_ref>(fun), &cbOnRef);
    if (ret != ANI_OK) {
        ErrorHandler::Throw(env, E_IPCSS);
        return;
    }
    auto callback = std::make_shared<CloudSyncCallbackAniImpl>(env, cbOnRef);

    auto data = fileSync->DoOn(EVENT_TYPE, callback);
    if (!data.IsSuccess()) {
        const auto &err = data.GetError();
        LOGE("file sync do on failed, ret = %{public}d", err.GetErrNo());
        ErrorHandler::Throw(env, err);
    }
}

void FileSyncAni::FileSyncOff0(ani_env *env, ani_object object, ani_object fun)
{
    ani_ref cbOnRef;
    ani_status ret = env->GlobalReference_Create(reinterpret_cast<ani_ref>(fun), &cbOnRef);
    if (ret != ANI_OK) {
        LOGE("create callback ref failed. ret = %{public}d", ret);
        ErrorHandler::Throw(env, E_IPCSS);
        return;
    }
    auto callback = std::make_shared<CloudSyncCallbackAniImpl>(env, cbOnRef);

    auto fileSync = FileSyncUnwrap(env, object);
    if (fileSync == nullptr) {
        LOGE("Cannot wrap fileSync.");
        ErrorHandler::Throw(env, E_IPCSS);
        return;
    }
    auto data = fileSync->DoOff(EVENT_TYPE, callback);
    if (!data.IsSuccess()) {
        const auto &err = data.GetError();
        LOGE("file sync do off failed, ret = %{public}d", err.GetErrNo());
        ErrorHandler::Throw(env, err);
    }
}

void FileSyncAni::FileSyncOff1(ani_env *env, ani_object object)
{
    auto fileSync = FileSyncUnwrap(env, object);
    if (fileSync == nullptr) {
        LOGE("Cannot wrap fileSync.");
        ErrorHandler::Throw(env, E_IPCSS);
        return;
    }
    auto data = fileSync->DoOff(EVENT_TYPE);
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
        ErrorHandler::Throw(env, E_IPCSS);
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
        ErrorHandler::Throw(env, E_IPCSS);
        return;
    }
    auto data = fileSync->DoStop();
    if (!data.IsSuccess()) {
        const auto &err = data.GetError();
        LOGE("file sync do stop failed, ret = %{public}d", err.GetErrNo());
        ErrorHandler::Throw(env, err);
    }
}

ani_long FileSyncAni::FileSyncGetLastSyncTime(ani_env *env, ani_object object)
{
    auto fileSync = FileSyncUnwrap(env, object);
    if (fileSync == nullptr) {
        LOGE("Cannot wrap fileSync.");
        ErrorHandler::Throw(env, E_IPCSS);
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
    return lastSyncTime;
}

void FileSyncAni::FileSyncRegisterUploadProgress(ani_env *env, ani_object object, ani_object fun)
{
    auto fileSync = FileSyncUnwrap(env, object);
    if (fileSync == nullptr) {
        LOGE("Cannot wrap fileSync.");
        ErrorHandler::Throw(env, E_IPCSS);
        return;
    }

    ani_ref cbOnRef;
    ani_status ret = env->GlobalReference_Create(reinterpret_cast<ani_ref>(fun), &cbOnRef);
    if (ret != ANI_OK) {
        ErrorHandler::Throw(env, E_IPCSS);
        return;
    }
    auto callback = std::make_shared<CloudUploadCallbackImplAni>(env, cbOnRef);

    auto data = fileSync->DoRegisterUploadProgress(callback);
    if (!data.IsSuccess()) {
        const auto &err = data.GetError();
        LOGE("file sync register upload progress failed, ret: %{public}d", err.GetErrNo());
        ErrorHandler::Throw(env, err);
    }
}

void FileSyncAni::FileSyncUnRegisterUploadProgress(ani_env *env, ani_object object)
{
    auto fileSync = FileSyncUnwrap(env, object);
    if (fileSync == nullptr) {
        LOGE("Cannot wrap fileSync.");
        ErrorHandler::Throw(env, E_IPCSS);
        return;
    }

    auto data = fileSync->DoUnRegisterUploadProgress();
    if (!data.IsSuccess()) {
        const auto &err = data.GetError();
        LOGE("file sync unregister upload failed, ret: %{public}d", err.GetErrNo());
        ErrorHandler::Throw(env, err);
    }
    return;
}

static bool FindUploadProgressClass(ani_env *env, ani_class &cls, ani_method &ctor)
{
    std::string classDesc = Builder::BuildClass("@ohos.file.cloudSync.cloudSync.UploadProgressInner").Descriptor();
    ani_status ret = env->FindClass(classDesc.c_str(), &cls);
    if (ret != ANI_OK) {
        LOGE("Cannot find class %{public}s, err: %{public}d", classDesc.c_str(), static_cast<int32_t>(ret));
        return false;
    }

    std::string ct = Builder::BuildConstructorName();
    std::string argSign = Builder::BuildSignatureDescriptor(
        {Builder::BuildClass("std.core.String"), Builder::BuildEnum("@ohos.file.cloudSync.cloudSync.UploadState"),
         Builder::BuildLong(), Builder::BuildLong(), Builder::BuildClass("std.core.String"),
         Builder::BuildEnum("@ohos.file.cloudSync.cloudSync.ErrorType")});
    ret = env->Class_FindMethod(cls, ct.c_str(), argSign.c_str(), &ctor);
    if (ret != ANI_OK) {
        LOGE("find ctor method failed. ret = %{public}d", static_cast<int32_t>(ret));
        return false;
    }
    return true;
}

struct EnumItemInfo {
    ani_enum enums;
    int32_t value;
    ani_enum_item &item;
    EnumItemInfo(ani_enum e, int32_t v, ani_enum_item &i) : enums(e), value(v), item(i) {}
};

struct EnumItemContext {
    ani_enum stateEnum;
    ani_enum errorEnum;
    int32_t state;
    int32_t error;
    ani_enum_item &stateItem;
    ani_enum_item &errorItem;
    EnumItemContext(const EnumItemInfo &stateInfo, const EnumItemInfo &errorInfo)
        : stateEnum(stateInfo.enums),
          errorEnum(errorInfo.enums),
          state(stateInfo.value),
          error(errorInfo.value),
          stateItem(stateInfo.item),
          errorItem(errorInfo.item)
    {
    }
};

static bool GetUploadEnums(ani_env *env, ani_enum &stateEnum, ani_enum &errorEnum)
{
    Type stateSign = Builder::BuildEnum("@ohos.file.cloudSync.cloudSync.UploadState");
    ani_status ret = env->FindEnum(stateSign.Descriptor().c_str(), &stateEnum);
    if (ret != ANI_OK) {
        LOGE("find state enum failed. ret = %{public}d", static_cast<int32_t>(ret));
        return false;
    }
    Type errorSign = Builder::BuildEnum("@ohos.file.cloudSync.cloudSync.ErrorType");
    ret = env->FindEnum(errorSign.Descriptor().c_str(), &errorEnum);
    if (ret != ANI_OK) {
        LOGE("find error enum failed. ret = %{public}d", static_cast<int32_t>(ret));
        return false;
    }
    return true;
}

static bool GetUploadEnumItems(ani_env *env, const EnumItemContext &ctx)
{
    ani_status ret = env->Enum_GetEnumItemByIndex(ctx.errorEnum, ctx.error, &ctx.errorItem);
    if (ret != ANI_OK) {
        LOGE("get error enum item failed. ret = %{public}d", static_cast<int32_t>(ret));
        return false;
    }
    ret = env->Enum_GetEnumItemByIndex(ctx.stateEnum, ctx.state, &ctx.stateItem);
    if (ret != ANI_OK) {
        LOGE("get state enum item failed. ret = %{public}d", static_cast<int32_t>(ret));
        return false;
    }
    return true;
}

ani_object ConvertUploadProgressObjToInner(ani_env *env, const CloudSync::UploadProgressObj &progress)
{
    ani_class cls;
    ani_method ctor;
    if (!FindUploadProgressClass(env, cls, ctor)) {
        return nullptr;
    }
    ani_string uri = nullptr;
    ani_status ret = env->String_NewUTF8(progress.uri.c_str(), progress.uri.size(), &uri);
    if (ret != ANI_OK) {
        LOGE("get uri failed. ret = %{public}d", static_cast<int32_t>(ret));
        return nullptr;
    }
    ani_enum stateEnum;
    ani_enum errorEnum;
    if (!GetUploadEnums(env, stateEnum, errorEnum)) {
        return nullptr;
    }
    ani_enum_item stateEnumItem;
    ani_enum_item errorEnumItem;
    EnumItemInfo stateInfo(stateEnum, static_cast<int32_t>(progress.state), stateEnumItem);
    EnumItemInfo errorInfo(errorEnum, static_cast<int32_t>(progress.error), errorEnumItem);
    EnumItemContext ctx(stateInfo, errorInfo);
    if (!GetUploadEnumItems(env, ctx)) {
        return nullptr;
    }
    ani_object pg;
    ret = env->Object_New(cls, ctor, &pg, stateEnumItem, progress.processed, progress.size, uri, errorEnumItem);
    if (ret != ANI_OK) {
        LOGE("create new object failed. ret = %{public}d", static_cast<int32_t>(ret));
        return nullptr;
    }
    return pg;
}

static ani_array CreateProgressArray(ani_env *env, size_t length)
{
    ani_array result = nullptr;
    ani_status status = env->Array_New(length, nullptr, &result);
    if (status != ANI_OK) {
        LOGE("Failed to create array");
        return nullptr;
    }
    return result;
}

ani_array FileSyncAni::FileSyncGetUploadList(ani_env *env, ani_object object, ani_array uriVec)
{
    auto fileSync = FileSyncUnwrap(env, object);
    if (fileSync == nullptr) {
        LOGE("Cannot wrap fileSync.");
        ErrorHandler::Throw(env, E_IPCSS);
        return nullptr;
    }
    auto [ret, uris] = ANIUtils::AniToStringArray(env, uriVec);
    if (!ret) {
        ErrorHandler::Throw(env, E_IPCSS);
        return nullptr;
    }
    struct UploadListArg {
        std::vector<CloudSync::UploadProgressObj> uploadList;
    };
    auto arg = std::make_shared<UploadListArg>();
    auto data = fileSync->DoGetUploadList(uris, arg->uploadList);
    if (!data.IsSuccess()) {
        const auto &err = data.GetError();
        LOGE("file sync get upload list failed, ret: %{public}d", err.GetErrNo());
        ErrorHandler::Throw(env, err);
        return nullptr;
    }
    auto result = CreateProgressArray(env, arg->uploadList.size());
    if (result == nullptr) {
        return nullptr;
    }
    for (size_t i = 0; i < arg->uploadList.size(); i++) {
        auto item = ConvertUploadProgressObjToInner(env, arg->uploadList[i]);
        if (item == nullptr || env->Array_Set(result, i, item) != ANI_OK) {
            LOGE("Failed to convert or set UploadProgressObj");
            return nullptr;
        }
    }
    return result;
}
} // namespace OHOS::FileManagement::CloudSync