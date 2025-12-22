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

#include "multi_download_progress_ani.h"

#include "ani_utils.h"
#include "dfs_error.h"
#include "error_handler.h"
#include "multi_download_progress_core.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
using namespace arkts::ani_signature;
using namespace std;
const static string STATE_SETTER = Builder::BuildSetterName("state");
const static string TAKSKID_SETTER = Builder::BuildSetterName("taskId");
const static string SUCC_COUNT_SETTER = Builder::BuildSetterName("successfulCount");
const static string FAILED_COUNT_SETTER = Builder::BuildSetterName("failedCount");
const static string TOTAL_COUNT_SETTER = Builder::BuildSetterName("totalCount");
const static string DOWNLOAD_SIZE_SETTER = Builder::BuildSetterName("downloadedSize");
const static string TOTAL_SIZE_SETTER = Builder::BuildSetterName("totalSize");
const static string ERROR_SETTER = Builder::BuildSetterName("errType");

static ani_status SetEnumState(ani_env *env, const ani_class &cls, ani_object &object, const char *name, int32_t value)
{
    ani_method setter;
    ani_status ret;
    if ((ret = env->Class_FindMethod(cls, name, nullptr, &setter)) != ANI_OK) {
        LOGE("Class_FindMethod Fail %{public}s, err: %{public}d", name, ret);
        return ret;
    }
    ani_enum stateEnum;
    std::string classDesc = Builder::BuildClass("@ohos.file.cloudSync.cloudSync.State").Descriptor();
    ret = env->FindEnum(classDesc.c_str(), &stateEnum);
    if (ret != ANI_OK) {
        LOGE("FindEnum %{public}s failed, err: %{public}d", classDesc.c_str(), ret);
        return ret;
    }

    ani_enum_item enumItem;
    ret = env->Enum_GetEnumItemByIndex(stateEnum, value, &enumItem);
    if (ret != ANI_OK) {
        LOGE("Enum_GetEnumItemByIndex failed, err: %{public}d", ret);
        return ret;
    }

    return env->Object_CallMethod_Void(object, setter, enumItem);
}

static ani_status SetEnumErrType(ani_env *env, const ani_class &cls, ani_object &object, const char *name,
    int32_t value)
{
    ani_method setter;
    ani_status ret;
    if ((ret = env->Class_FindMethod(cls, name, nullptr, &setter)) != ANI_OK) {
        LOGE("Class_FindMethod Fail %{public}s, err: %{public}d", name, ret);
        return ret;
    }
    ani_enum stateEnum;
    std::string classDesc = Builder::BuildClass("@ohos.file.cloudSync.cloudSync.DownloadErrorType").Descriptor();
    ret = env->FindEnum(classDesc.c_str(), &stateEnum);
    if (ret != ANI_OK) {
        LOGE("FindEnum %{public}s failed, err: %{public}d", classDesc.c_str(), ret);
        return ret;
    }

    ani_enum_item enumItem;
    ret = env->Enum_GetEnumItemByIndex(stateEnum, value, &enumItem);
    if (ret != ANI_OK) {
        LOGE("Enum_GetEnumItemByIndex failed, err: %{public}d", ret);
        return ret;
    }

    return env->Object_CallMethod_Void(object, setter, enumItem);
}

static ani_status SetProperties(ani_env *env, const ani_class &cls, ani_object &pg, MultiDlProgressCore *progressCore)
{
    vector<pair<string_view, ani_double>> numProperties = {
        {TAKSKID_SETTER, progressCore->GetTaskId()},
        {SUCC_COUNT_SETTER, static_cast<ani_int>(progressCore->GetDownloadedNum())},
        {FAILED_COUNT_SETTER, static_cast<ani_int>(progressCore->GetFailedNum())},
        {TOTAL_COUNT_SETTER, static_cast<ani_int>(progressCore->GetTotalNum())},
        {DOWNLOAD_SIZE_SETTER, progressCore->GetDownloadedSize()},
        {TOTAL_SIZE_SETTER, progressCore->GetTotalSize()},
    };
    ani_status ret;
    for (auto iter : numProperties) {
        auto key = iter.first.data();
        ani_method setter;
        if ((ret = env->Class_FindMethod(cls, key, nullptr, &setter)) != ANI_OK) {
            LOGE("Class_FindMethod Fail %{public}s, err: %{public}d", key, ret);
            return ret;
        }
        if ((ret = env->Object_CallMethod_Void(pg, setter, iter.second)) != ANI_OK) {
            LOGE("Object_CallMethod_Void Fail %{public}s, err: %{public}d", key, ret);
            return ret;
        }
    }

    if ((ret = SetEnumState(env, cls, pg, STATE_SETTER.data(), progressCore->GetStatus())) != ANI_OK) {
        LOGE("Failed to get state enum, err: %{public}d", ret);
        return ret;
    }
    if ((ret = SetEnumErrType(env, cls, pg, ERROR_SETTER.data(), progressCore->GetErrorType())) != ANI_OK) {
        LOGE("Failed to get errType enum, err: %{public}d", ret);
        return ret;
    }
    return ANI_OK;
}

MultiDlProgressCore *MultiDlProgressWrapper::Unwrap(ani_env *env, ani_object object)
{
    ani_long nativePtr;
    auto ret = env->Object_GetFieldByName_Long(object, "nativePtr", &nativePtr);
    if (ret != ANI_OK) {
        LOGE("Unwrap MultiDlProgressCore err: %{public}d", static_cast<int32_t>(ret));
        return nullptr;
    }
    std::uintptr_t ptrValue = static_cast<std::uintptr_t>(nativePtr);
    MultiDlProgressCore *multiDlProgress = reinterpret_cast<MultiDlProgressCore *>(ptrValue);
    return multiDlProgress;
}

ani_object MultiDlProgressWrapper::Wrap(ani_env *env, MultiDlProgressCore *multiDlProgress)
{
    if (multiDlProgress == nullptr) {
        LOGE("MultiDlProgressCore pointer is null!");
        return nullptr;
    }

    ani_status ret;
    ani_class cls;
    std::string classDesc = Builder::BuildClass("@ohos.file.cloudSync.cloudSync.MultiDownloadProgress").Descriptor();
    if ((ret = env->FindClass(classDesc.c_str(), &cls)) != ANI_OK) {
        LOGE("Cannot find class %{public}s, err: %{public}d", classDesc.c_str(), ret);
        return nullptr;
    }

    ani_method ctor;
    auto ctorDesc = Builder::BuildConstructorName();
    auto ctorSig = Builder::BuildSignatureDescriptor({Builder::BuildLong()});
    if ((ret = env->Class_FindMethod(cls, ctorDesc.c_str(), ctorSig.c_str(), &ctor)) != ANI_OK) {
        LOGE("Cannot find class %{public}s constructor method, err: %{public}d", classDesc.c_str(), ret);
        return nullptr;
    }

    ani_object progressObject;
    if ((ret = env->Object_New(cls, ctor, &progressObject, reinterpret_cast<ani_long>(multiDlProgress))) != ANI_OK) {
        LOGE("New StatInner Fail, err: %{public}d", ret);
        return nullptr;
    }

    if ((ret = SetProperties(env, cls, progressObject, multiDlProgress)) != ANI_OK) {
        LOGE("SetProperties Fail, err: %{public}d", ret);
        return nullptr;
    }

    return progressObject;
}

void MultiDlProgressAni::Constructor(ani_env *env, ani_object object)
{
    ani_status ret;
    ani_class cls;
    std::string classDesc = Builder::BuildClass("@ohos.file.cloudSync.cloudSync.MultiDownloadProgress").Descriptor();
    if ((ret = env->FindClass(classDesc.c_str(), &cls)) != ANI_OK) {
        LOGE("Cannot find class %{public}s, err: %{public}d", classDesc.c_str(), ret);
        ErrorHandler::Throw(env, JsErrCode::E_INNER_FAILED);
        return;
    }

    ani_method ctor;
    auto ctorDesc = Builder::BuildConstructorName();
    std::string bindSign = Builder::BuildSignatureDescriptor({Builder::BuildLong()});
    ret = env->Class_FindMethod(cls, ctorDesc.c_str(), bindSign.c_str(), &ctor);
    if (ret != ANI_OK) {
        LOGE("find class ctor. ret = %{public}d", static_cast<int32_t>(ret));
        ErrorHandler::Throw(env, JsErrCode::E_INNER_FAILED);
        return;
    }

    FsResult<MultiDlProgressCore *> data = MultiDlProgressCore::Constructor();
    if (!data.IsSuccess()) {
        LOGE("MultiDlProgress constructor failed.");
        const auto &err = data.GetError();
        ErrorHandler::Throw(env, err);
        return;
    }

    const MultiDlProgressCore *multiDlProgress = data.GetData().value();
    ret = env->Object_CallMethod_Void(object, ctor, reinterpret_cast<ani_long>(multiDlProgress));
    if (ret != ANI_OK) {
        LOGE("bindNativePtr failed.");
        delete multiDlProgress;
        ErrorHandler::Throw(env, JsErrCode::E_INNER_FAILED);
    }
}

static tuple<bool, ani_object> ToFailedFileInfo(ani_env *env, ani_class cls, ani_method ctor,
    const FailedFileInfo &files)
{
    auto [succ, fileUri] = ANIUtils::ToAniString(env, files.uri);
    if (!succ) {
        LOGE("Convert FailedFileInfo uri to ani string failed!");
        return {false, nullptr};
    }

    ani_status ret;
    ani_enum downloadErrorEnum;
    Type errorSign = Builder::BuildEnum("@ohos.file.cloudSync.cloudSync.DownloadErrorType");
    if ((ret = env->FindEnum(errorSign.Descriptor().c_str(), &downloadErrorEnum)) != ANI_OK) {
        LOGE("Find DownloadErrorType enum failed!, err: %{public}d", ret);
        return {false, nullptr};
    }
    ani_enum_item downloadErrorEnumItem;
    if ((ret = env->Enum_GetEnumItemByIndex(downloadErrorEnum, files.error, &downloadErrorEnumItem)) != ANI_OK) {
        LOGE("Find DownloadErrorType enum failed!, err: %{public}d", ret);
        return {false, nullptr};
    }

    ani_object obj;
    if ((ret = env->Object_New(cls, ctor, &obj, fileUri, downloadErrorEnumItem)) != ANI_OK) {
        LOGE("Create FailedFileInfo object failed!, err: %{public}d", ret);
        return {false, nullptr};
    }

    return {true, obj};
}

static std::tuple<bool, ani_array> ToAniObjectArray(ani_env *env, const std::vector<FailedFileInfo> &objList)
{
    ani_status ret;
    ani_class cls;
    std::string classDesc = Builder::BuildClass("@ohos.file.cloudSync.cloudSync.FailedFileInfoInner").Descriptor();
    if ((ret = env->FindClass(classDesc.c_str(), &cls)) != ANI_OK) {
        LOGE("Cannot find class %{public}s, err: %{public}d", classDesc.c_str(), ret);
        return {false, nullptr};
    }

    ani_method ctor;
    auto ctorDesc = Builder::BuildConstructorName();
    auto ctorSig =
        Builder::BuildSignatureDescriptor({Builder::BuildClass("std.core.String"),
                                           Builder::BuildEnum("@ohos.file.cloudSync.cloudSync.DownloadErrorType")});
    if ((ret = env->Class_FindMethod(cls, ctorDesc.c_str(), ctorSig.c_str(), &ctor)) != ANI_OK) {
        LOGE("Cannot find class %{public}s constructor method, err: %{public}d", classDesc.c_str(), ret);
        return {false, nullptr};
    }

    size_t length = objList.size();
    const FailedFileInfo *objArray = objList.data();
    ani_array result = nullptr;
    if (env->Array_New(length, nullptr, &result) != ANI_OK) {
        LOGE("Failed to new array");
        return {false, nullptr};
    }
    for (size_t i = 0; i < length; i++) {
        auto [succ, item] = ToFailedFileInfo(env, cls, ctor, objArray[i]);
        if (!succ) {
            LOGE("Failed to get element for array");
            return {false, nullptr};
        }
        if (env->Array_Set(result, i, item) != ANI_OK) {
            LOGE("Failed to set element for array");
            return {false, nullptr};
        }
    }
    return {true, result};
}

ani_array MultiDlProgressAni::GetFailedFileList(ani_env *env, ani_object object)
{
    auto multiDlProgress = MultiDlProgressWrapper::Unwrap(env, object);
    if (multiDlProgress == nullptr) {
        LOGE("Cannot wrap multiDlProgress.");
        ErrorHandler::Throw(env, JsErrCode::E_INNER_FAILED);
        return nullptr;
    }
    auto data = multiDlProgress->GetFailedFileList();
    if (!data.IsSuccess()) {
        const auto &err = data.GetError();
        LOGE("multiDlProgress get failed list failed, ret = %{public}d", err.GetErrNo());
        ErrorHandler::Throw(env, err);
        return nullptr;
    }

    auto [succ, res] = ToAniObjectArray(env, data.GetData().value());
    if (!succ) {
        ErrorHandler::Throw(env, JsErrCode::E_INNER_FAILED);
        return nullptr;
    }
    return res;
}

ani_array MultiDlProgressAni::GetDownloadedFileList(ani_env *env, ani_object object)
{
    auto multiDlProgress = MultiDlProgressWrapper::Unwrap(env, object);
    if (multiDlProgress == nullptr) {
        LOGE("Cannot wrap multiDlProgress.");
        ErrorHandler::Throw(env, JsErrCode::E_INNER_FAILED);
        return nullptr;
    }
    auto data = multiDlProgress->GetDownloadedFileList();
    if (!data.IsSuccess()) {
        const auto &err = data.GetError();
        LOGE("multiDlProgress get downloaded list failed, ret = %{public}d", err.GetErrNo());
        ErrorHandler::Throw(env, err);
        return nullptr;
    }

    auto [succ, res] = ANIUtils::ToAniStringArray(env, data.GetData().value());
    if (!succ) {
        ErrorHandler::Throw(env, JsErrCode::E_INNER_FAILED);
        return nullptr;
    }
    return res;
}
} // namespace OHOS::FileManagement::CloudSync