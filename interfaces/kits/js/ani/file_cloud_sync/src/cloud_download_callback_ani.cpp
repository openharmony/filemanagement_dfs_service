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

CloudDownloadCallbackAniImpl::CloudDownloadCallbackAniImpl(ani_env *env, ani_object fun, bool isBatch) : env_(env)
{
    if (fun != nullptr) {
        cbOnRef_ = fun;
    }
    isBatch_ = isBatch;
}

void CloudDownloadCallbackAniImpl::GetDownloadProgress(
    const DownloadProgressObj &progress, const ani_class &cls, ani_object &pg)
{
    ani_method ctor;
    std::string argPre = "Lstd/core/Double;L@ohos/file/cloudSync/cloudSync/#State;Lstd/core/Double;Lstd/core/Double;";
    std::string argBack = "Lstd/core/String;L@ohos/file/cloudSync/cloudSync/#DownloadErrorType;:V";
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
    env_->FindEnum("L@ohos/file/cloudSync/cloudSync/#State", &stateEnum);
    ani_enum downloadErrorEnum;
    env_->FindEnum("L@ohos/file/cloudSync/cloudSync/#DownloadErrorType", &downloadErrorEnum);

    ani_enum_item stateEnumItem;
    ani_enum_item downloadErrorEnumItem;
    env_->Enum_GetEnumItemByIndex(downloadErrorEnum, progress.downloadErrorType, &downloadErrorEnumItem);

    if (!isBatch_) {
        env_->Enum_GetEnumItemByIndex(stateEnum, progress.state, &stateEnumItem);
        ret = env_->Object_New(cls, ctor, &pg, progress.downloadId, stateEnumItem, progress.downloadedSize,
            progress.totalSize, uri, downloadErrorEnumItem);
    } else {
        env_->Enum_GetEnumItemByIndex(stateEnum, progress.batchState, &stateEnumItem);
        ret = env_->Object_New(cls, ctor, &pg, progress.downloadId, stateEnumItem, progress.batchDownloadSize,
            progress.batchTotalSize, progress.batchSuccNum, progress.batchFailNum, progress.batchTotalNum,
            downloadErrorEnumItem);
    }

    if (ret != ANI_OK) {
        LOGE("create new object failed. ret = %{public}d", ret);
    }
}

void CloudDownloadCallbackAniImpl::OnDownloadProcess(const DownloadProgressObj &progress)
{
    ani_size nr_refs = 16;
    ani_status ret = env_->CreateLocalScope(nr_refs);
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
    ret = env_->Object_CallMethodByName_Ref(cbOnRef_, "invoke0", "Lstd/core/Object;:Lstd/core/Object;", &ref_, pg);
    if (ret != ANI_OK) {
        LOGE("ani call function failed. ret = %{public}d", ret);
        return;
    }
    ret = env_->DestroyLocalScope();
    if (ret != ANI_OK) {
        LOGE("failed to DestroyLocalScope. ret = %{public}d", ret);
    }
}

void CloudDownloadCallbackAniImpl::DeleteReference()
{
    if (cbOnRef_ != nullptr) {
        cbOnRef_ = nullptr;
    }
}
} // namespace OHOS::FileManagement::CloudSync