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

#include "ani_utils.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {

using namespace arkts::ani_signature;
constexpr int32_t ANI_SCOPE_SIZE = 16;

CloudDownloadCallbackAniImpl::CloudDownloadCallbackAniImpl(ani_env *env, ani_ref fun, bool isBatch)
{
    env_ = env;
    if (fun != nullptr) {
        cbOnRef_ = fun;
    }
    isBatch_ = isBatch;
}

void CloudDownloadCallbackAniImpl::GetDownloadProgress(
    const DownloadProgressObj &progress, const ani_class &cls, ani_object &pg)
{
    ani_method ctor;
    std::string ct = Builder::BuildConstructorName();
    std::string argSign = Builder::BuildSignatureDescriptor({
        Builder::BuildDouble(), Builder::BuildEnum("@ohos.file.cloudSync.cloudSync.State"),
        Builder::BuildDouble(), Builder::BuildDouble(), Builder::BuildClass("std.core.String"),
        Builder::BuildEnum("@ohos.file.cloudSync.cloudSync.DownloadErrorType")
    });
    ani_status ret = env_->Class_FindMethod(cls, ct.c_str(), argSign.c_str(), &ctor);
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

    ani_enum stateEnum;
    Type stateSign = Builder::BuildEnum("@ohos.file.cloudSync.cloudSync.State");
    env_->FindEnum(stateSign.Descriptor().c_str(), &stateEnum);
    ani_enum downloadErrorEnum;
    Type errorSign = Builder::BuildEnum("@ohos.file.cloudSync.cloudSync.DownloadErrorType");
    env_->FindEnum(errorSign.Descriptor().c_str(), &downloadErrorEnum);

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
    auto task = [this, progress]() {
        LOGI("CloudDownloadCallbackAniImpl OnDownloadProcess");
        ani_env *tmpEnv = env_;
        ani_size nr_refs = ANI_SCOPE_SIZE;
        ani_status ret = tmpEnv->CreateLocalScope(nr_refs);
        if (ret != ANI_OK) {
            LOGE("crete local scope failed. ret = %{public}d", ret);
            return;
        }
        ani_namespace ns {};
        Namespace nsSign = Builder::BuildNamespace("@ohos.file.cloudSync.cloudSync");
        ret = tmpEnv->FindNamespace(nsSign.Descriptor().c_str(), &ns);
        if (ret != ANI_OK) {
            LOGE("find namespace failed. ret = %{public}d", ret);
            return;
        }
        Type clsName = Builder::BuildClass("DownloadProgressInner");
        ani_class cls;
        ret = tmpEnv->Namespace_FindClass(ns, clsName.Descriptor().c_str(), &cls);
        if (ret != ANI_OK) {
            LOGE("find class failed. ret = %{public}d", ret);
            return;
        }
        ani_object pg;
        GetDownloadProgress(progress, cls, pg);
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

void CloudDownloadCallbackAniImpl::DeleteReference()
{
    if (cbOnRef_ != nullptr) {
        env_->GlobalReference_Delete(cbOnRef_);
        cbOnRef_ = nullptr;
    }
}
} // namespace OHOS::FileManagement::CloudSync