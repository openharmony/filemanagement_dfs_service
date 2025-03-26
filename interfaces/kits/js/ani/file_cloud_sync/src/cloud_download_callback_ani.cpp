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

#include "cloud_download_callback_ani.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {

CloudDownloadCallbackAniImpl::CloudDownloadCallbackAniImpl(ani_env *env, ani_ref fun, bool isBatch)
{
    env_ = env;
    if (fun != nullptr) {
        cbOnRef_ = fun;
    }
    isBatch_ = isBatch;
    ani_status ret = env_->GetVM(&vm);
    if (ret != ANI_OK) {
        LOGE("get vim failed. ret = %{public}d", ret);
    }
}

void CloudDownloadCallbackAniImpl::GetDownloadProgress(
    const DownloadProgressObj &progress, const ani_class &cls, ani_object &pg)
{
    ani_method ctor;
    std::string argPre = "DL@ohos/file/cloudSync/cloudSync/State;DD";
    std::string argBack = "Lstd/core/String;L@ohos/file/cloudSync/cloudSync/DownloadErrorType;:V";
    std::string arg = argPre + argBack;
    ani_status ret = env_->Class_FindMethod(cls, "<ctor>", arg.c_str(), &ctor);
    if (ret != ANI_OK) {
        LOGE("find ctor method failed. ret = %{public}d", ret);
        return;
    }
    ani_string uri = nullptr;
    ret = env_->String_NewUTF8(progress.path.c_str(), progress.path.size(), &uri);
    if (ret != ANI_OK) {
        LOGE("get uri failed. ret = %{public}d", ret);
        return;
    }

    // Please replace it with GetEnumItemByValue_Int if suppored in the future.
    ani_enum stateEnum;
    env_->FindEnum("L@ohos/file/cloudSync/cloudSync/State;", &stateEnum);
    ani_enum downloadErrorEnum;
    env_->FindEnum("L@ohos/file/cloudSync/cloudSync/DownloadErrorType;", &downloadErrorEnum);

    ani_enum_item stateEnumItem;
    ani_enum_item downloadErrorEnumItem;
    env_->Enum_GetEnumItemByIndex(downloadErrorEnum, progress.downloadErrorType, &downloadErrorEnumItem);

    if (!isBatch_) {
        env_->Enum_GetEnumItemByIndex(stateEnum, progress.state, &stateEnumItem);
        ret = env_->Object_New(cls, ctor, &pg, static_cast<double>(progress.downloadId), stateEnumItem,
            static_cast<double>(progress.downloadedSize), static_cast<double>(progress.totalSize), uri,
            downloadErrorEnumItem);
    } else {
        env_->Enum_GetEnumItemByIndex(stateEnum, progress.batchState, &stateEnumItem);
        ret = env_->Object_New(cls, ctor, &pg, static_cast<double>(progress.downloadId), stateEnumItem,
            static_cast<double>(progress.batchDownloadSize), static_cast<double>(progress.batchTotalSize),
            static_cast<double>(progress.batchSuccNum), static_cast<double>(progress.batchFailNum),
            static_cast<double>(progress.batchTotalNum), downloadErrorEnumItem);
    }

    if (ret != ANI_OK) {
        LOGE("create new object failed. ret = %{public}d", ret);
    }
}

void CloudDownloadCallbackAniImpl::OnDownloadProcess(const DownloadProgressObj &progress)
{
    ani_status ret = vm->GetEnv(ANI_VERSION_1, &env_);
    bool attach = false;
    if (ret != ANI_OK) {
        LOGE("vm get env failed. ret = %{public}d", ret);
        ani_options aniArgs {0, nullptr};
        ret = vm->AttachCurrentThread(&aniArgs, ANI_VERSION_1, &env_);
        if (ret != ANI_OK) {
            LOGE("AttachCurrentThread failed. ret = %{public}d", ret);
            return;
        }
        attach = true;
    }
    ani_size nr_refs = 16;
    ret = env_->CreateLocalScope(nr_refs);
    if (ret != ANI_OK) {
        LOGE("crete local scope failed. ret = %{public}d", ret);
        return;
    }
    ani_namespace ns {};
    ret = env_->FindNamespace("L@ohos/file/cloudSync/cloudSync;", &ns);
    if (ret != ANI_OK) {
        LOGE("find namespace failed. ret = %{public}d", ret);
        return;
    }
    static const char *className = "LDownloadProgressInner;";
    ani_class cls;
    ret = env_->Namespace_FindClass(ns, className, &cls);
    if (ret != ANI_OK) {
        LOGE("find class failed. ret = %{public}d", ret);
        return;
    }
    ani_object pg;
    GetDownloadProgress(progress, cls, pg);
    ani_ref ref_;
    std::vector<ani_ref> vec = { pg };
    ret = env_->FunctionalObject_Call(static_cast<ani_fn_object>(cbOnRef_), 1, vec.data(), &ref_);
    if (ret != ANI_OK) {
        LOGE("ani call function failed. ret = %{public}d", ret);
        return;
    }
    ret = env_->DestroyLocalScope();
    if (ret != ANI_OK) {
        LOGE("failed to DestroyLocalScope. ret = %{public}d", ret);
    }
    if (attach) {
        vm->DetachCurrentThread();
    }
}

void CloudDownloadCallbackAniImpl::DeleteReference()
{
    if (cbOnRef_ != nullptr) {
        env_->GlobalReference_Delete(cbOnRef_);
        cbOnRef_ = nullptr;
    }
}
} // namespace OHOS::FileManagement::CloudSync