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

#include "upload_callback_impl_ani.h"

#include "ani_utils.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {

using namespace std;
using namespace arkts::ani_signature;

constexpr int32_t ANI_SCOPE_SIZE = 16;

CloudUploadCallbackImplAni::CloudUploadCallbackImplAni(ani_env *env, ani_ref fun) : vm_(nullptr), cbOnRef_(nullptr)
{
    if (env == nullptr || env->GetVM(&vm_) != ANI_OK) {
        LOGE("CloudUploadCallbackImplAni get vm failed.");
        return;
    }
    if (fun != nullptr) {
        cbOnRef_ = fun;
    }
}

CloudUploadCallbackImplAni::~CloudUploadCallbackImplAni()
{
    DeleteReference();
}

static ani_status FindEnum(ani_env *env, const std::string &enumName, ani_enum &enumObj)
{
    Type enumSign = Builder::BuildEnum(enumName);
    ani_status ret = env->FindEnum(enumSign.Descriptor().c_str(), &enumObj);
    if (ret != ANI_OK) {
        LOGE("find enum %{public}s failed. ret = %{public}d", enumName.c_str(), static_cast<int32_t>(ret));
    }
    return ret;
}

static ani_status FindClassAndConstructor(ani_env *env, const std::string &className, ani_class &cls, ani_method &ctor)
{
    Type clsName = Builder::BuildClass(className);
    ani_status ret = env->FindClass(clsName.Descriptor().c_str(), &cls);
    if (ret != ANI_OK) {
        LOGE("find class %{public}s failed. ret = %{public}d", className.c_str(), static_cast<int32_t>(ret));
        return ret;
    }

    std::string ct = Builder::BuildConstructorName();
    std::string argSign = Builder::BuildSignatureDescriptor(
        {Builder::BuildClass("std.core.String"), Builder::BuildEnum("@ohos.file.cloudSync.cloudSync.UploadState"),
         Builder::BuildLong(), Builder::BuildLong(), Builder::BuildClass("std.core.String"),
         Builder::BuildEnum("@ohos.file.cloudSync.cloudSync.ErrorType")});
    return env->Class_FindMethod(cls, ct.c_str(), argSign.c_str(), &ctor);
}

ani_status CloudUploadCallbackImplAni::GetUploadProgress(ani_env *env,
                                                         const UploadProgressObj &progress,
                                                         ani_object &pg)
{
    ani_class cls;
    ani_method ctor;
    if (FindClassAndConstructor(env, "@ohos.file.cloudSync.cloudSync.UploadProgress", cls, ctor) != ANI_OK) {
        return ANI_ERROR;
    }
    ani_string uri = nullptr;
    if (env->String_NewUTF8(progress.uri.c_str(), progress.uri.size(), &uri) != ANI_OK) {
        LOGE("get uri failed");
        return ANI_ERROR;
    }
    ani_enum stateEnum;
    ani_enum errorEnum;
    if (FindEnum(env, "@ohos.file.cloudSync.cloudSync.UploadState", stateEnum) != ANI_OK ||
        FindEnum(env, "@ohos.file.cloudSync.cloudSync.ErrorType", errorEnum) != ANI_OK) {
        return ANI_ERROR;
    }
    ani_enum_item stateItem;
    ani_enum_item errorItem;
    if (env->Enum_GetEnumItemByIndex(errorEnum, static_cast<int32_t>(progress.error), &errorItem) != ANI_OK ||
        env->Enum_GetEnumItemByIndex(stateEnum, static_cast<int32_t>(progress.state), &stateItem) != ANI_OK) {
        LOGE("get enum item failed");
        return ANI_ERROR;
    }
    ani_status ret = env->Object_New(cls, ctor, &pg, stateItem, progress.processed, progress.size, uri, errorItem);
    if (ret != ANI_OK) {
        LOGE("create object failed. ret = %{public}d", static_cast<int32_t>(ret));
    }
    return ret;
}

static ani_status CallUploadCallback(ani_env *env, ani_ref callback, ani_object progressObj)
{
    ani_fn_object etsCb = reinterpret_cast<ani_fn_object>(callback);
    std::vector<ani_ref> vec = {progressObj};
    ani_ref ref_;
    ani_status ret = env->FunctionalObject_Call(etsCb, 1, vec.data(), &ref_);
    if (ret != ANI_OK) {
        LOGE("ani call function failed. ret = %{public}d", static_cast<int32_t>(ret));
    }
    return ret;
}

void CloudUploadCallbackImplAni::OnUploadProgress(const UploadProgressObj &progress)
{
    auto task = [this, progress]() {
        if (cbOnRef_ == nullptr) {
            LOGE("callback is null");
            return;
        }
        ani_env *tmpEnv = nullptr;
        if (vm_ == nullptr || vm_->GetEnv(ANI_VERSION_1, &tmpEnv)) {
            LOGE("get env failed");
            return;
        }
        if (tmpEnv->CreateLocalScope(ANI_SCOPE_SIZE) != ANI_OK) {
            LOGE("create local scope failed");
            return;
        }
        ani_object pg = nullptr;
        if (GetUploadProgress(tmpEnv, progress, pg) != ANI_OK) {
            LOGE("get upload progress failed");
            tmpEnv->DestroyLocalScope();
            return;
        }
        CallUploadCallback(tmpEnv, cbOnRef_, pg);
        tmpEnv->DestroyLocalScope();
    };
    if (!ANIUtils::SendEventToMainThread(task)) {
        LOGE("failed to send event");
    }
}

void CloudUploadCallbackImplAni::DeleteReference()
{
    ani_env *env = nullptr;
    if (vm_ == nullptr || vm_->GetEnv(ANI_VERSION_1, &env)) {
        LOGE("CloudUploadCallbackImplAni get env failed.");
        return;
    }
    if (cbOnRef_ != nullptr) {
        env->GlobalReference_Delete(cbOnRef_);
        cbOnRef_ = nullptr;
    }
}

} // namespace OHOS::FileManagement::CloudSync
