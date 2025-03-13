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

#include "cloud_sync_callback_ani.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {

CloudSyncCallbackAniImpl::CloudSyncCallbackAniImpl(ani_env *env, ani_object fun) : env_(env)
{
    if (fun != nullptr) {
        cbOnRef_ = fun;
    }
}

void CloudSyncCallbackAniImpl::OnSyncStateChanged(CloudSyncState state, ErrorType error)
{
    ani_size nr_refs = 16;
    ani_status ret = env_->CreateLocalScope(nr_refs);
    if (ret != ANI_OK) {
        LOGE("crete local scope failed. ret = %{public}d", ret);
        return;
    }
    ani_namespace ns {};
    ret = env_->FindNamespace("Lani_cloud_sync/cloudSync;", &ns);
    if (ret != ANI_OK) {
        LOGE("find namespace failed. ret = %{public}d", ret);
        return;
    }
    static const char *className = "LSyncProgress;";
    ani_class cls;
    ret = env_->Namespace_FindClass(ns, className, &cls);
    if (ret != ANI_OK) {
        LOGE("find class failed. ret = %{public}d", ret);
        return;
    }
    ani_object pg;
    ani_method ctor;
    ret = env_->Class_FindMethod(
        cls, "<ctor>", "Lani_cloud_sync/cloudSync/SyncState;Lani_cloud_sync/cloudSync/ErrorType;:V", &ctor);
    if (ret != ANI_OK) {
        LOGE("find ctor method failed. ret = %{public}d", ret);
        return;
    }

    // Please replace it with GetEnumItemByValue_Int if suppored in the future.
    ani_enum stateEnum;
    env_->FindEnum("Lcloud_sync_ani/cloudSync/SyncState", &stateEnum);
    ani_enum errorEnum;
    env_->FindEnum("Lcloud_sync_ani/cloudSync/ErrorType", &errorEnum);

    ani_enum_item stateEnumItem;
    env_->Enum_GetEnumItemByIndex(stateEnum, state, &stateEnumItem);
    ani_enum_item errorEnumItem;
    env_->Enum_GetEnumItemByIndex(errorEnum, error, &errorEnumItem);

    ret = env_->Object_New(cls, ctor, &pg, stateEnumItem, errorEnumItem);
    if (ret != ANI_OK) {
        LOGE("create new object failed. ret = %{public}d", ret);
        return;
    }
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

void CloudSyncCallbackAniImpl::DeleteReference()
{
    if (cbOnRef_ != nullptr) {
        cbOnRef_ = nullptr;
    }
}

void CloudSyncCallbackAniImpl::OnSyncStateChanged(SyncType type, SyncPromptState state)
{
    return;
}
} // namespace OHOS::FileManagement::CloudSync