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
    Type clsName = Builder::BuildClass("@ohos.file.cloudSync.cloudSync.CloudFileCache");
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

void CloudFileCacheAni::CloudFileCacheConstructorWithBundleName(ani_env *env, ani_object object, ani_string bundleName)
{
    std::string bundleNameInput;
    ani_status ret = ANIUtils::AniString2String(env, bundleName, bundleNameInput);
    if (ret != ANI_OK) {
        ErrorHandler::Throw(env, JsErrCode::E_IPCSS);
        return;
    }

    Type clsName = Builder::BuildClass("@ohos.file.cloudSync.cloudSync.CloudFileCache");
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

    FsResult<CloudFileCacheCore *> data = CloudFileCacheCore::Constructor(bundleNameInput);
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

void CloudFileCacheAni::CloudFileCacheOn(ani_env *env, ani_object object, ani_object fun)
{
    auto cloudFileCache = CloudFileCacheUnwrap(env, object);
    if (cloudFileCache == nullptr) {
        LOGE("Cannot wrap cloudFileCache.");
        ErrorHandler::Throw(env, JsErrCode::E_IPCSS);
        return;
    }

    std::shared_ptr<CloudFileCacheCallbackImplAni> callbackImpl = cloudFileCache->GetCallbackImpl(EVENT_TYPE, true);
    callbackImpl->InitVm(env);
    auto status = callbackImpl->RegisterCallback(env, fun);
    if (status != ANI_OK) {
        LOGE("Failed to register callback, status: %{public}d.", status);
        ErrorHandler::Throw(env, JsErrCode::E_IPCSS);
        return;
    }
}

void CloudFileCacheAni::CloudFileCacheOff0(ani_env *env, ani_object object, ani_object fun)
{
    auto cloudFileCache = CloudFileCacheUnwrap(env, object);
    if (cloudFileCache == nullptr) {
        LOGE("Cannot wrap cloudFileCache.");
        ErrorHandler::Throw(env, JsErrCode::E_IPCSS);
        return;
    }

    std::shared_ptr<CloudFileCacheCallbackImplAni> callbackImpl = cloudFileCache->GetCallbackImpl(EVENT_TYPE, false);
    if (callbackImpl == nullptr || callbackImpl->UnregisterCallback(env, fun) != ANI_OK) {
        LOGE("Failed to unregister callback.");
        ErrorHandler::Throw(env, JsErrCode::E_IPCSS);
        return;
    }
    callbackImpl->TryCleanCallback();
}

void CloudFileCacheAni::CloudFileCacheOff1(ani_env *env, ani_object object)
{
    CloudFileCacheOff0(env, object, nullptr);
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

ani_long CloudFileCacheAni::CloudFileCacheStartBatch(ani_env *env, ani_object object,
    ani_array uriList, ani_enum_item fileType)
{
    ani_long errResult = 0;
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

    return data.GetData().value();
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

void CloudFileCacheAni::CloudFileCacheStopBatch(ani_env *env, ani_object object, ani_long taskId, ani_boolean needClean)
{
    auto cloudFileCache = CloudFileCacheUnwrap(env, object);
    if (cloudFileCache == nullptr) {
        LOGE("Cannot wrap cloudFileCache.");
        ErrorHandler::Throw(env, JsErrCode::E_IPCSS);
        return;
    }

    bool needCleanInput = needClean;

    auto data = cloudFileCache->DoStop(taskId, needCleanInput);
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

void CloudFileCacheAni::CloudFileCacheCleanFileCache(ani_env *env, ani_object object, ani_string uri)
{
    std::string uriInput;
    ani_status ret = ANIUtils::AniString2String(env, uri, uriInput);
    if (ret != ANI_OK) {
        ErrorHandler::Throw(env, JsErrCode::E_INNER_FAILED);
        return;
    }

    auto cloudFileCache = CloudFileCacheUnwrap(env, object);
    if (cloudFileCache == nullptr) {
        LOGE("Cannot wrap cloudFileCache.");
        ErrorHandler::Throw(env, JsErrCode::E_INNER_FAILED);
        return;
    }
    auto data = cloudFileCache->CleanFileCache(uriInput);
    if (!data.IsSuccess()) {
        const auto &err = data.GetError();
        LOGE("cloudFileCache do cleanFileCache failed, ret = %{public}d", err.GetErrNo());
        ErrorHandler::Throw(env, err);
    }
}

void CloudFileCacheAni::CloudFileCacheCleanFileCache1(ani_env *env, ani_object object)
{
    auto cloudFileCache = CloudFileCacheUnwrap(env, object);
    if (cloudFileCache == nullptr) {
        LOGE("Cannot wrap cloudFileCache.");
        ErrorHandler::Throw(env, JsErrCode::E_IPCSS);
        return;
    }

    auto data = cloudFileCache->CleanFileCache();
    if (!data.IsSuccess()) {
        const auto &err = data.GetError();
        LOGE("cloudFileCache clean file cache failed, ret = %{public}d", err.GetErrNo());
        ErrorHandler::Throw(env, err);
    }
}

void CloudFileCacheAni::CloudFileCacheOnBatch(ani_env *env, ani_object object, ani_object fun)
{
    auto cloudFileCache = CloudFileCacheUnwrap(env, object);
    if (cloudFileCache == nullptr) {
        LOGE("Cannot wrap cloudFileCache.");
        ErrorHandler::Throw(env, JsErrCode::E_IPCSS);
        return;
    }

    std::shared_ptr<CloudFileCacheCallbackImplAni> callbackImpl =
        cloudFileCache->GetCallbackImpl(MULT_EVENT_TYPE, true);
    callbackImpl->InitVm(env);
    auto status = callbackImpl->RegisterCallback(env, fun);
    if (status != ANI_OK) {
        LOGE("Failed to register callback, status: %{public}d.", status);
        ErrorHandler::Throw(env, JsErrCode::E_IPCSS);
        return;
    }
}

void CloudFileCacheAni::CloudFileCacheOffBatch(ani_env *env, ani_object object, ani_object fun)
{
    auto cloudFileCache = CloudFileCacheUnwrap(env, object);
    if (cloudFileCache == nullptr) {
        LOGE("Cannot wrap cloudFileCache.");
        ErrorHandler::Throw(env, JsErrCode::E_IPCSS);
        return;
    }

    std::shared_ptr<CloudFileCacheCallbackImplAni> callbackImpl =
        cloudFileCache->GetCallbackImpl(MULT_EVENT_TYPE, false);
    if (callbackImpl == nullptr || callbackImpl->UnregisterCallback(env, fun) != ANI_OK) {
        LOGE("Failed to unregister callback.");
        ErrorHandler::Throw(env, JsErrCode::E_IPCSS);
        return;
    }
    callbackImpl->TryCleanCallback();
}

static bool FindDownloadProgressClass(ani_env *env, ani_class &cls, ani_method &ctor)
{
    std::string classDesc = Builder::BuildClass(
        "@ohos.file.cloudSync.cloudSync.DownloadProgressInner").Descriptor();
    ani_status ret = env->FindClass(classDesc.c_str(), &cls);
    if (ret != ANI_OK) {
        LOGE("Cannot find class %{private}s, err: %{public}d", classDesc.c_str(), ret);
        return false;
    }

    std::string ct = Builder::BuildConstructorName();
    std::string argSign = Builder::BuildSignatureDescriptor(
        {Builder::BuildLong(), Builder::BuildEnum("@ohos.file.cloudSync.cloudSync.State"),
         Builder::BuildLong(), Builder::BuildLong(), Builder::BuildClass("std.core.String"),
         Builder::BuildEnum("@ohos.file.cloudSync.cloudSync.DownloadErrorType")});
    ret = env->Class_FindMethod(cls, ct.c_str(), argSign.c_str(), &ctor);
    if (ret != ANI_OK) {
        LOGE("find ctor method: failed. ret = %{public}d", ret);
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

static bool GetDownloadEnums(ani_env *env, ani_enum &stateEnum, ani_enum &errorEnum)
{
    Type stateSign = Builder::BuildEnum("@ohos.file.cloudSync.cloudSync.State");
    ani_status ret = env->FindEnum(stateSign.Descriptor().c_str(), &stateEnum);
    if (ret != ANI_OK) {
        LOGE("find state enum: failed. ret = %{public}d", ret);
        return false;
    }

    Type errorSign = Builder::BuildEnum("@ohos.file.cloudSync.cloudSync.DownloadErrorType");
    ret = env->FindEnum(errorSign.Descriptor().c_str(), &errorEnum);
    if (ret != ANI_OK) {
        LOGE("find error enum: failed. ret = %{public}d", ret);
        return false;
    }
    return true;
}

static bool GetDownloadEnumItems(ani_env *env, const EnumItemContext &ctx)
{
    ani_status ret = env->Enum_GetEnumItemByIndex(ctx.errorEnum, ctx.error, &ctx.errorItem);
    if (ret != ANI_OK) {
        LOGE("get error enum item: failed. ret = %{public}d", ret);
        return false;
    }
    ret = env->Enum_GetEnumItemByIndex(ctx.stateEnum, ctx.state, &ctx.stateItem);
    if (ret != ANI_OK) {
        LOGE("get state: enum item: failed. ret = %{public}d", ret);
        return false;
    }
    return true;
}

ani_object ConvertDownloadProgressObjToInner(ani_env *env, const CloudSync::DownloadProgressObj &progress)
{
    ani_class cls;
    ani_method ctor;
    if (!FindDownloadProgressClass(env, cls, ctor)) {
        return nullptr;
    }
    ani_string uri = nullptr;
    ani_status ret = env->String_NewUTF8(progress.path.c_str(), progress.path.size(), &uri);
    if (ret != ANI_OK) {
        LOGE("get uri: failed. ret = %{public}d", ret);
        return nullptr;
    }
    ani_enum stateEnum;
    ani_enum errorEnum;
    if (!GetDownloadEnums(env, stateEnum, errorEnum)) {
        return nullptr;
    }
    ani_enum_item stateEnumItem;
    ani_enum_item errorEnumItem;
    EnumItemInfo stateInfo(stateEnum, progress.state, stateEnumItem);
    EnumItemInfo errorInfo(errorEnum, progress.downloadErrorType, errorEnumItem);
    EnumItemContext ctx(stateInfo, errorInfo);
    if (!GetDownloadEnumItems(env, ctx)) {
        return nullptr;
    }
    ani_object pg;
    ret = env->Object_New(cls, ctor, &pg, progress.downloadId, stateEnumItem, progress.downloadedSize,
                          progress.totalSize, uri, errorEnumItem);
    if (ret != ANI_OK) {
        LOGE("create new object: failed. ret = %{public}d", ret);
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

ani_array CloudFileCacheAni::CloudFileCacheGetDownloadList(ani_env *env, ani_object object, ani_array uriVec)
{
    auto cloudFileCache = CloudFileCacheUnwrap(env, object);
    if (cloudFileCache == nullptr) {
        LOGE("Cannot wrap cloudFileCache.");
        ErrorHandler::Throw(env, JsErrCode::E_IPCSS);
        return nullptr;
    }
    auto [ret, uris] = ANIUtils::AniToStringArray(env, uriVec);
    if (!ret) {
        ErrorHandler::Throw(env, JsErrCode::E_IPCSS);
        return nullptr;
    }
    struct DownloadListArg {
        std::vector<CloudSync::DownloadProgressObj> downloadList;
    };
    auto arg = make_shared<DownloadListArg>();
    auto data = cloudFileCache->GetDownloadList(uris, arg->downloadList);
    if (!data.IsSuccess()) {
        const auto &err = data.GetError();
        LOGE("cloudFileCache get download list failed, ret = %{public}d", err.GetErrNo());
        ErrorHandler::Throw(env, err);
        return nullptr;
    }
    auto result = CreateProgressArray(env, arg->downloadList.size());
    if (result == nullptr) {
        return nullptr;
    }
    for (size_t i = 0; i < arg->downloadList.size(); i++) {
        auto item = ConvertDownloadProgressObjToInner(env, arg->downloadList[i]);
        if (item == nullptr || env->Array_Set(result, i, item) != ANI_OK) {
            LOGE("Failed to convert or set DownloadProgressObj");
            return nullptr;
        }
    }
    return result;
}

ani_long CloudFileCacheAni::CloudFileCacheGetCachedTotalSize(ani_env *env, ani_object object)
{
    auto cloudFileCache = CloudFileCacheUnwrap(env, object);
    if (cloudFileCache == nullptr) {
        LOGE("Cannot wrap cloudFileCache.");
        ErrorHandler::Throw(env, JsErrCode::E_IPCSS);
        return 0;
    }

    auto data = cloudFileCache->GetCachedTotalSize();
    if (!data.IsSuccess()) {
        const auto &err = data.GetError();
        LOGE("cloudFileCache get cached total size failed, ret = %{public}d", err.GetErrNo());
        ErrorHandler::Throw(env, err);
        return 0;
    }

    return data.GetData().value();
}

void CloudFileCacheAni::CloudFileCacheConstructorForWatch(ani_env *env, ani_object object)
{
    LOGD("Start CloudFileCacheConstructorForWatch in ani");
    return;
}

void CloudFileCacheAni::CloudFileCacheOnForWatch(ani_env *env, ani_object object, ani_object fun)
{
    LOGD("Start CloudFileCacheOnForWatch in ani");
    return;
}

void CloudFileCacheAni::CloudFileCacheOff0ForWatch(ani_env *env, ani_object object, ani_object fun)
{
    LOGD("Start CloudFileCacheOff0ForWatch in ani");
    return;
}

void CloudFileCacheAni::CloudFileCacheOff1ForWatch(ani_env *env, ani_object object)
{
    LOGD("Start CloudFileCacheOff1ForWatch in ani");
    return;
}

void CloudFileCacheAni::CloudFileCacheStartForWatch(ani_env *env, ani_object object, ani_string uri)
{
    LOGD("Start CloudFileCacheStartForWatch in ani");
    return;
}

ani_long CloudFileCacheAni::CloudFileCacheStartBatchForWatch(ani_env *env, ani_object object,
    ani_array uriList, ani_enum_item fileType)
{
    LOGD("Start CloudFileCacheStartBatchForWatch in ani");
    return 0;
}

void CloudFileCacheAni::CloudFileCacheStopForWatch(ani_env *env, ani_object object, ani_string uri,
    ani_boolean needClean)
{
    LOGD("Start CloudFileCacheStopForWatch in ani");
    return;
}

void CloudFileCacheAni::CloudFileCacheStopBatchForWatch(ani_env *env, ani_object object, ani_long taskId,
    ani_boolean needClean)
{
    LOGD("Start CloudFileCacheStopBatchForWatch in ani");
    return;
}

void CloudFileCacheAni::CloudFileCacheCleanCacheForWatch(ani_env *env, ani_object object, ani_string uri)
{
    LOGD("Start CloudFileCacheCleanCacheForWatch in ani");
    return;
}

void CloudFileCacheAni::CloudFileCacheCleanFileCacheForWatch(ani_env *env, ani_object object, ani_string uri)
{
    LOGD("Start CloudFileCacheCleanFileCacheForWatch in ani");
    return;
}

void CloudFileCacheAni::CloudFileCacheOnBatchForWatch(ani_env *env, ani_object object, ani_object fun)
{
    LOGD("Start CloudFileCacheOnBatchForWatch in ani");
    return;
}

void CloudFileCacheAni::CloudFileCacheOffBatchForWatch(ani_env *env, ani_object object, ani_object fun)
{
    LOGD("Start CloudFileCacheOffBatchForWatch in ani");
    return;
}
} // namespace OHOS::FileManagement::CloudSync