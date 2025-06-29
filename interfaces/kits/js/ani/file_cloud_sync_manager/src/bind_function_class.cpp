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

#include <iostream>
#include <vector>

#include "cloud_sync_manager_ani.h"
#include "utils_log.h"

using namespace OHOS;
using namespace OHOS::FileManagement::CloudSync;
using namespace arkts::ani_signature;

static ani_status BindContextOnCloudSyncManager(ani_env *env)
{
    ani_namespace ns {};
    Namespace nsSign = Builder::BuildNamespace("@ohos.file.cloudSyncManager.cloudSyncManager");
    ani_status ret = env->FindNamespace(nsSign.Descriptor().c_str(), &ns);
    if (ret != ANI_OK) {
        LOGE("find namespace failed. ret = %{public}d", ret);
        return ret;
    }

    Type clsName = Builder::BuildClass("StaticFunction");
    ani_class cls;
    ret = env->Namespace_FindClass(ns, clsName.Descriptor().c_str(), &cls);
    if (ret != ANI_OK) {
        LOGE("find class failed. ret = %{public}d", ret);
        return ret;
    }
    std::string ssbSign = Builder::BuildSignatureDescriptor(
        {Builder::BuildClass("std.core.String"), Builder::BuildClass("std.core.String"), Builder::BuildBoolean()});
    std::string ssSign = Builder::BuildSignatureDescriptor(
        {Builder::BuildClass("std.core.String"), Builder::BuildClass("std.core.String")});
    std::string srSign = Builder::BuildSignatureDescriptor(
        {Builder::BuildClass("std.core.String"), Builder::BuildClass("escompat.Record")});
    std::string sSign = Builder::BuildSignatureDescriptor({Builder::BuildClass("std.core.String")});
    std::string deSign = Builder::BuildSignatureDescriptor({
        Builder::BuildDouble(),
        Builder::BuildClass("@ohos.file.cloudSyncManager.cloudSyncManager.ExtraData")});
    std::array methods = {
        ani_native_function { "changeAppCloudSwitchInner", ssbSign.c_str(),
            reinterpret_cast<void *>(CloudSyncManagerAni::ChangeAppCloudSwitch) },
        ani_native_function { "notifyDataChangeInner", ssSign.c_str(),
            reinterpret_cast<void *>(CloudSyncManagerAni::NotifyDataChange) },
        ani_native_function { "enableCloudInner", srSign.c_str(),
            reinterpret_cast<void *>(CloudSyncManagerAni::EnableCloud) },
        ani_native_function {
            "disableCloudInner", sSign.c_str(), reinterpret_cast<void *>(CloudSyncManagerAni::DisableCloud) },
        ani_native_function { "cleanInner", srSign.c_str(), reinterpret_cast<void *>(CloudSyncManagerAni::Clean) },
        ani_native_function { "notifyEventChangeInner", deSign.c_str(),
            reinterpret_cast<void *>(CloudSyncManagerAni::NotifyEventChange) },
    };

    ret = env->Class_BindNativeMethods(cls, methods.data(), methods.size());
    if (ret != ANI_OK) {
        LOGE("bind native method failed. ret = %{public}d", ret);
        return ret;
    };

    return ANI_OK;
}

ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    ani_env *env;
    if (ANI_OK != vm->GetEnv(ANI_VERSION_1, &env)) {
        std::cerr << "Unsupported ANI_VERSION_1" << std::endl;
        return (ani_status)ANI_ERROR;
    }

    auto status = BindContextOnCloudSyncManager(env);
    if (status != ANI_OK) {
        return status;
    }
    *result = ANI_VERSION_1;
    return ANI_OK;
}