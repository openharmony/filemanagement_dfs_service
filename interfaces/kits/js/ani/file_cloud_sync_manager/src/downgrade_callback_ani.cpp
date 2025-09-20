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

#include "downgrade_callback_ani.h"

#include "ani_utils.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
using namespace std;
using namespace arkts::ani_signature;
constexpr int32_t ANI_SCOPE_SIZE = 16;

DowngradeCallbackAniImpl::DowngradeCallbackAniImpl(ani_env *env, ani_ref fun)
{
    if (env == nullptr || env->GetVM(&vm_)) {
        LOGE("DowngradeCallbackAniImpl get vm failed.");
        return;
    }
    if (fun != nullptr) {
        cbOnRef_ = fun;
    }
}

DowngradeCallbackAniImpl::~DowngradeCallbackAniImpl()
{
    ani_env *env = nullptr;
    if (vm_ == nullptr || vm_->GetEnv(ANI_VERSION_1, &env)) {
        LOGE("DowngradeCallbackAniImpl get env failed.");
        return;
    }
    if (cbOnRef_ != nullptr) {
        env->GlobalReference_Delete(cbOnRef_);
        cbOnRef_ = nullptr;
    }
}

ani_object DowngradeCallbackAniImpl::ConvertToObject(ani_env *env, const DowngradeProgress &progress)
{
    ani_class cls;
    ani_status ret;
    string classDesc =
        Builder::BuildClass("@ohos.file.cloudSyncManager.cloudSyncManager.DownloadProgress").Descriptor();
    if ((ret = env->FindClass(classDesc.c_str(), &cls)) != ANI_OK) {
        LOGE("Cannot find class %{private}s, err: %{public}d", classDesc.c_str(), ret);
        return nullptr;
    }

    ani_method ctor;
    string ct = Builder::BuildConstructorName();
    string argSign = Builder::BuildSignatureDescriptor({
        Builder::BuildEnum("@ohos.file.cloudSyncManager.cloudSyncManager.DownloadState"),
        Builder::BuildInt(), Builder::BuildInt(), Builder::BuildInt(),
        Builder::BuildLong(), Builder::BuildLong(),
        Builder::BuildEnum("@ohos.file.cloudSyncManager.cloudSyncManager.DownloadStopReason")});
    ret = env->Class_FindMethod(cls, ct.c_str(), argSign.c_str(), &ctor);
    if (ret != ANI_OK) {
        LOGE("Find ctor method failed, ret = %{public}d", ret);
        return nullptr;
    }

    ani_enum stateEnum;
    Type stateSign = Builder::BuildEnum("@ohos.file.cloudSyncManager.cloudSyncManager.DownloadState");
    env->FindEnum(stateSign.Descriptor().c_str(), &stateEnum);
    ani_enum downloadStopReasonEnum;
    Type errorSign = Builder::BuildEnum("@ohos.file.cloudSyncManager.cloudSyncManager.DownloadStopReason");
    env->FindEnum(errorSign.Descriptor().c_str(), &downloadStopReasonEnum);

    ani_enum_item stateEnumItem;
    ani_enum_item downloadStopReasonEnumItem;
    env->Enum_GetEnumItemByIndex(downloadStopReasonEnum, progress.stopReason, &downloadStopReasonEnumItem);
    env->Enum_GetEnumItemByIndex(stateEnum, progress.state, &stateEnumItem);
    ani_object pg;
    ret = env->Object_New(cls, ctor, &pg, stateEnumItem, static_cast<double>(progress.successfulCount),
                          static_cast<double>(progress.failedCount), static_cast<double>(progress.totalCount),
                          static_cast<double>(progress.downloadedSize), static_cast<double>(progress.totalSize),
                          downloadStopReasonEnumItem);
    if (ret != ANI_OK) {
        LOGE("create new object failed, ret = %{public}d", ret);
    }
    return pg;
}

void DowngradeCallbackAniImpl::UpdateDowngradeProgress(const DowngradeProgress &progress)
{
    if (progress_ == nullptr) {
        progress_ = std::make_shared<SingleBdProgress>();
    }

    progress_->state = static_cast<int32_t>(progress.state);
    progress_->downloadedSize = progress.downloadedSize;
    progress_->totalSize = progress.totalSize;
    progress_->successfulCount = progress.successfulCount;
    progress_->failedCount = progress.failedCount;
    progress_->totalCount =  progress.totalCount;
    progress_->stopReason = static_cast<int32_t>(progress.stopReason);
}

void DowngradeCallbackAniImpl::OnDownloadProcess(const DowngradeProgress &progress)
{
    UpdateDowngradeProgress(progress);
    shared_ptr<DowngradeCallbackAniImpl> callbackImpl = shared_from_this();
    auto task = [this, progress, callbackImpl]() mutable {
        ani_env *tmpEnv = nullptr;
        if (vm_ == nullptr || vm_->GetEnv(ANI_VERSION_1, &tmpEnv)) {
            LOGE("DowngradeCallbackAniImpl get env failed.");
            return;
        }
        if (tmpEnv == nullptr || cbOnRef_ == nullptr) {
            LOGE("The env context is invalid");
            return;
        }
        ani_size nr_refs = ANI_SCOPE_SIZE;
        ani_status ret = tmpEnv->CreateLocalScope(nr_refs);
        if (ret != ANI_OK) {
            LOGE("create local scope failed, ret = %{public}d", ret);
            return;
        }
        ani_fn_object etsCb = reinterpret_cast<ani_fn_object>(cbOnRef_);
        ani_object pg = callbackImpl->ConvertToObject(tmpEnv, progress);
        ani_ref ref_;
        vector<ani_ref> vec = { pg };
        ret = tmpEnv->FunctionalObject_Call(etsCb, 1, vec.data(), &ref_);
        if (ret != ANI_OK) {
            LOGE("ani call function failed, ret = %{public}d", ret);
            return;
        }
        ret = tmpEnv->DestroyLocalScope();
        if (ret != ANI_OK) {
            LOGE("failed to DestroyLocalScope, ret = %{public}d", ret);
        }
    };
    if (!ANIUtils::SendEventToMainThread(task)) {
        LOGE("failed to send event");
    }
}
} // namespace OHOS::FileManagement::CloudSync