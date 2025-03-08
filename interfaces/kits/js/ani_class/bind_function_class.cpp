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

#include "cloud_download_ani.h"
#include "cloud_sync_ani.h"
#include "utils_log.h"

using namespace OHOS;
using namespace OHOS::FileManagement::CloudSync;

static ani_status BindContextOnGallery(ani_env *env)
{
    ani_namespace ns {};
    ani_status ret = env->FindNamespace("Lcloud_sync_ani/cloudSync;", &ns);
    if (ret != ANI_OK) {
        LOGE("find namespace failed. ret = %{public}d", ret);
        return ret;
    }

    static const char *className = "LGallerySync;";
    ani_class cls;
    ret = env->Namespace_FindClass(ns, className, &cls);
    if (ret != ANI_OK) {
        LOGE("find class failed. ret = %{public}d", ret);
        return ret;
    }

    std::array methods = {
        ani_native_function { "<ctor>", ":V", reinterpret_cast<void *>(CloudSyncAni::GallerySyncConstructor) },
        ani_native_function {
            "on", "Lstd/core/String;Lstd/core/Function1;:V", reinterpret_cast<void *>(CloudSyncAni::GallerySyncOn) },
        ani_native_function {
            "off", "Lstd/core/String;Lstd/core/Function1;:V", reinterpret_cast<void *>(CloudSyncAni::GallerySyncOff0) },
        ani_native_function { "off", "Lstd/core/String;:V", reinterpret_cast<void *>(CloudSyncAni::GallerySyncOff1) },
        ani_native_function { "GallerySyncStart", ":V", reinterpret_cast<void *>(CloudSyncAni::GallerySyncStart) },
        ani_native_function { "GallerySyncStop", ":V", reinterpret_cast<void *>(CloudSyncAni::GallerySyncStop) },
    };

    ret = env->Class_BindNativeMethods(cls, methods.data(), methods.size());
    if (ret != ANI_OK) {
        LOGE("bind native method failed. ret = %{public}d", ret);
        return ret;
    };

    return ANI_OK;
}

static ani_status BindContextOnDownload(ani_env *env)
{
    ani_namespace ns {};
    ani_status ret = env->FindNamespace("Lcloud_sync_ani/cloudSync;", &ns);
    if (ret != ANI_OK) {
        LOGE("find namespace failed. ret = %{public}d", ret);
        return ret;
    }

    static const char *className = "LDownload;";
    ani_class cls;
    ret = env->Namespace_FindClass(ns, className, &cls);
    if (ret != ANI_OK) {
        LOGE("find class failed. ret = %{public}d", ret);
        return ret;
    }

    std::array methods = {
        ani_native_function { "<ctor>", ":V", reinterpret_cast<void *>(CloudDownloadAni::DownloadConstructor) },
        ani_native_function {
            "on", "Lstd/core/String;Lstd/core/Function1;:V", reinterpret_cast<void *>(CloudDownloadAni::DownloadOn) },
        ani_native_function { "off", "Lstd/core/String;Lstd/core/Function1;:V",
            reinterpret_cast<void *>(CloudDownloadAni::DownloadOff0) },
        ani_native_function { "off", "Lstd/core/String;:V", reinterpret_cast<void *>(CloudDownloadAni::DownloadOff1) },
        ani_native_function {
            "DownloadStart", "Lstd/core/String;:V", reinterpret_cast<void *>(CloudDownloadAni::DownloadStart) },
        ani_native_function {
            "DownloadStop", "Lstd/core/String;:V", reinterpret_cast<void *>(CloudDownloadAni::DownloadStop) },
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

    auto status = BindContextOnGallery(env);
    if (status != ANI_OK) {
        return status;
    }
    status = BindContextOnDownload(env);
    if (status != ANI_OK) {
        return status;
    }
    *result = ANI_VERSION_1;
    return ANI_OK;
}