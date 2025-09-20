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

#include "file_version_callback_ani.h"

#include "ani_utils.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {

using namespace std;
using namespace arkts::ani_signature;

static const int64_t PERCENT = 100;
constexpr int32_t ANI_SCOPE_SIZE = 16;

VersionDownloadCallbackAniImpl::VersionDownloadCallbackAniImpl(ani_env *env, ani_ref fun)
{
    if (env == nullptr || env->GetVM(&vm_)) {
        LOGE("CloudSyncCallbackAniImpl get vm failed.");
        return;
    }
    if (fun != nullptr) {
        cbOnRef_ = fun;
    }
}

VersionDownloadCallbackAniImpl::~VersionDownloadCallbackAniImpl()
{
    ani_env *env = nullptr;
    if (vm_ == nullptr || vm_->GetEnv(ANI_VERSION_1, &env)) {
        LOGE("CloudSyncCallbackAniImpl get env failed.");
        return;
    }
    if (cbOnRef_ != nullptr) {
        env->GlobalReference_Delete(cbOnRef_);
        cbOnRef_ = nullptr;
    }
}

ani_status VersionDownloadCallbackAniImpl::GetDownloadProgress(ani_env *env,
    const DownloadProgressObj &progress, const ani_class &cls, ani_object &pg)
{
    ani_method ctor;
    std::string ct = Builder::BuildConstructorName();
    std::string argSign = Builder::BuildSignatureDescriptor({
        Builder::BuildEnum("@ohos.file.cloudSync.cloudSync.State"), Builder::BuildInt(),
        Builder::BuildEnum("@ohos.file.cloudSync.cloudSync.DownloadErrorType")
    });
    ani_status ret = env->Class_FindMethod(cls, ct.c_str(), argSign.c_str(), &ctor);
    if (ret != ANI_OK) {
        LOGE("find ctor method failed. ret = %{public}d", ret);
        return ret;
    }

    ani_enum stateEnum;
    Type stateSign = Builder::BuildEnum("@ohos.file.cloudSync.cloudSync.State");
    if (env->FindEnum(stateSign.Descriptor().c_str(), &stateEnum) != ANI_OK) {
        LOGE("find enum State fail.");
        return ret;
    }
    ani_enum ErrorEnum;
    Type errorSign = Builder::BuildEnum("@ohos.file.cloudSync.cloudSync.DownloadErrorType");
    if (env->FindEnum(errorSign.Descriptor().c_str(), &ErrorEnum) != ANI_OK) {
        LOGE("find enum DownloadErrorType fail.");
        return ret;
    }
    ani_enum_item stateEnumItem;
    ani_enum_item ErrorEnumItem;
    if (env->Enum_GetEnumItemByIndex(stateEnum, progress.state, &stateEnumItem) != ANI_OK ||
        env->Enum_GetEnumItemByIndex(ErrorEnum, progress.downloadErrorType, &ErrorEnumItem) != ANI_OK) {
        LOGE("get ani enum fail.");
        return ret;
    }
    ani_long process = (static_cast<ani_long>(progress.downloadedSize) / progress.totalSize) * PERCENT;
    ret = env->Object_New(cls, ctor, &pg, stateEnumItem, static_cast<ani_int>(process), ErrorEnumItem);
    if (ret != ANI_OK) {
        LOGE("create new object failed. ret = %{public}d", ret);
        return ret;
    }
    return ANI_OK;
}

void VersionDownloadCallbackAniImpl::OnDownloadProcess(const DownloadProgressObj &progress)
{
    auto task = [this, progress]() {
        LOGI("CloudDownloadCallbackAniImpl OnDownloadProcess");
        ani_env *tmpEnv = nullptr;
        if (vm_ == nullptr || vm_->GetEnv(ANI_VERSION_1, &tmpEnv)) {
            LOGE("CloudSyncCallbackAniImpl get env failed.");
            return;
        }
        ani_size nr_refs = ANI_SCOPE_SIZE;
        ani_status ret = tmpEnv->CreateLocalScope(nr_refs);
        if (ret != ANI_OK) {
            LOGE("crete local scope failed. ret = %{public}d", ret);
            return;
        }
        Type clsName = Builder::BuildClass("@ohos.file.cloudSync.cloudSync.VersionDownloadProgressInner");
        ani_class cls;
        ret = tmpEnv->FindClass(clsName.Descriptor().c_str(), &cls);
        if (ret != ANI_OK) {
            LOGE("find class failed. ret = %{public}d", ret);
            return;
        }
        ani_object pg;
        ret = GetDownloadProgress(tmpEnv, progress, cls, pg);
        if (ret != ANI_OK) {
            LOGE("GetDownloadProgress failed. ret = %{public}d", ret);
            return;
        }
        ani_ref ref_;
        ani_fn_object etsCb = reinterpret_cast<ani_fn_object>(cbOnRef_);
        std::vector<ani_ref> vec = { pg };
        ret = tmpEnv->FunctionalObject_Call(etsCb, 1, vec.data(), &ref_);
        if (ret != ANI_OK) {
            LOGE("ani call function failed. ret = %{public}d", ret);
            return;
        }
        ret = tmpEnv->DestroyLocalScope();
        if (ret != ANI_OK) {
            LOGE("failed to DestroyLocalScope. ret = %{public}d", ret);
        }
    };
    if (!ANIUtils::SendEventToMainThread(task)) {
        LOGE("failed to send event");
    }
}
} // namespace OHOS::FileManagement::CloudSync
