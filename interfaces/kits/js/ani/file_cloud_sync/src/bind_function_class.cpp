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
#include "cloud_file_cache_ani.h"
#include "cloud_sync_ani.h"
#include "file_sync_ani.h"
#include "file_version_ani.h"
#include "multi_download_progress_ani.h"
#include "utils_log.h"

using namespace OHOS;
using namespace OHOS::FileManagement::CloudSync;
using namespace arkts::ani_signature;

static ani_status BindContextOnGallery(ani_env *env)
{
    Type clsName = Builder::BuildClass("@ohos.file.cloudSync.cloudSync.GallerySync");
    ani_class cls;
    ani_status ret = env->FindClass(clsName.Descriptor().c_str(), &cls);
    if (ret != ANI_OK) {
        LOGE("find class failed. ret = %{public}d", ret);
        return ret;
    }
    std::string ct = Builder::BuildConstructorName();
    std::string vSign = Builder::BuildSignatureDescriptor({});
    std::string onOffSign = Builder::BuildSignatureDescriptor({Builder::BuildFunctionalObject(1, false)});
    std::array methods = {
        ani_native_function{ct.c_str(), vSign.c_str(),
            reinterpret_cast<void *>(CloudSyncAni::CloudSyncConstructor)},
        ani_native_function{"onProgress", onOffSign.c_str(), reinterpret_cast<void *>(CloudSyncAni::CloudSyncOn)},
        ani_native_function{"offProgress", onOffSign.c_str(), reinterpret_cast<void *>(CloudSyncAni::CloudSyncOff0)},
        ani_native_function{"offProgress", vSign.c_str(), reinterpret_cast<void *>(CloudSyncAni::CloudSyncOff1)},
        ani_native_function{"GallerySyncStart", vSign.c_str(), reinterpret_cast<void *>(CloudSyncAni::CloudSyncStart)},
        ani_native_function{"GallerySyncStop", vSign.c_str(), reinterpret_cast<void *>(CloudSyncAni::CloudSyncStop)},
    };

    ret = env->Class_BindNativeMethods(cls, methods.data(), methods.size());
    if (ret != ANI_OK) {
        LOGE("bind native method failed. ret = %{public}d", ret);
        return ret;
    };

    return ANI_OK;
}

static ani_status BindContextOnMultiDlProgress(ani_env *env)
{
    Type clsName = Builder::BuildClass("@ohos.file.cloudSync.cloudSync.MultiDownloadProgress");
    ani_class cls;
    ani_status ret = env->FindClass(clsName.Descriptor().c_str(), &cls);
    if (ret != ANI_OK) {
        LOGE("find class failed. ret = %{public}d", ret);
        return ret;
    }
    std::string ct = Builder::BuildConstructorName();
    std::string succSign = Builder::BuildSignatureDescriptor({}, Builder::BuildClass("std.core.Array"));
    std::string failSign = Builder::BuildSignatureDescriptor({}, Builder::BuildClass("std.core.Array"));
    std::array methods = {
        ani_native_function{"getFailedFiles", succSign.c_str(),
                            reinterpret_cast<void *>(MultiDlProgressAni::GetFailedFileList)},
        ani_native_function{"getSuccessfulFiles", failSign.c_str(),
                            reinterpret_cast<void *>(MultiDlProgressAni::GetDownloadedFileList)},
    };

    ret = env->Class_BindNativeMethods(cls, methods.data(), methods.size());
    if (ret != ANI_OK) {
        LOGE("bind native method failed. ret = %{public}d", ret);
        return ret;
    };

    return ANI_OK;
}

static ani_status BindContextOnCloudFileCache(ani_env *env)
{
    Type clsName = Builder::BuildClass("@ohos.file.cloudSync.cloudSync.CloudFileCache");
    ani_class cls;
    ani_status ret = env->FindClass(clsName.Descriptor().c_str(), &cls);
    if (ret != ANI_OK) {
        LOGE("find class failed. ret = %{public}d", ret);
        return ret;
    }
    std::string ct = Builder::BuildConstructorName();
    std::string vSign = Builder::BuildSignatureDescriptor({});
    std::string onOffSign = Builder::BuildSignatureDescriptor({Builder::BuildFunctionalObject(1, false)});
    std::string sSign = Builder::BuildSignatureDescriptor({Builder::BuildClass("std.core.String")});
    std::string sbSign = Builder::BuildSignatureDescriptor(
        {Builder::BuildClass("std.core.String"), Builder::BuildBoolean()});
    std::string startBatchSign = Builder::BuildSignatureDescriptor({Builder::BuildClass("std.core.Array"),
        Builder::BuildEnum("@ohos.file.cloudSync.cloudSync.DownloadFileType")}, Builder::BuildLong());
    std::string stopBatchSign = Builder::BuildSignatureDescriptor(
        {Builder::BuildLong(), Builder::BuildBoolean()});
    std::array methods = {
        ani_native_function{ct.c_str(), vSign.c_str(),
            reinterpret_cast<void *>(CloudFileCacheAni::CloudFileCacheConstructor)},
        ani_native_function{"onProgress", onOffSign.c_str(),
            reinterpret_cast<void *>(CloudFileCacheAni::CloudFileCacheOn)},
        ani_native_function{"offProgress", onOffSign.c_str(),
            reinterpret_cast<void *>(CloudFileCacheAni::CloudFileCacheOff0)},
        ani_native_function{"offProgress", vSign.c_str(),
            reinterpret_cast<void *>(CloudFileCacheAni::CloudFileCacheOff1)},
        ani_native_function{"CloudFileCacheStart", sSign.c_str(),
            reinterpret_cast<void *>(CloudFileCacheAni::CloudFileCacheStart)},
        ani_native_function{"CloudFileCacheStop", sbSign.c_str(),
            reinterpret_cast<void *>(CloudFileCacheAni::CloudFileCacheStop)},
        ani_native_function{"cleanCache", sSign.c_str(),
            reinterpret_cast<void *>(CloudFileCacheAni::CloudFileCacheCleanCache)},
        ani_native_function{"cleanFileCache", sSign.c_str(),
            reinterpret_cast<void *>(CloudFileCacheAni::CloudFileCacheCleanFileCache)},
        ani_native_function{"CloudFileCacheStartBatch", startBatchSign.c_str(),
            reinterpret_cast<void *>(CloudFileCacheAni::CloudFileCacheStartBatch)},
        ani_native_function{"CloudFileCacheStopBatch", stopBatchSign.c_str(),
            reinterpret_cast<void *>(CloudFileCacheAni::CloudFileCacheStopBatch)},
        ani_native_function{"onBatchDownload", onOffSign.c_str(),
            reinterpret_cast<void *>(CloudFileCacheAni::CloudFileCacheOnBatch)},
        ani_native_function{"offBatchDownloadInner", onOffSign.c_str(),
            reinterpret_cast<void *>(CloudFileCacheAni::CloudFileCacheOffBatch)}};
    ret = env->Class_BindNativeMethods(cls, methods.data(), methods.size());
    if (ret != ANI_OK) {
        LOGE("bind native method failed. ret = %{public}d", ret);
        return ret;
    };
    return ANI_OK;
}

static ani_status BindContextOnFileSync(ani_env *env)
{
    Type clsName = Builder::BuildClass("@ohos.file.cloudSync.cloudSync.FileSync");
    ani_class cls;
    ani_status ret = env->FindClass(clsName.Descriptor().c_str(), &cls);
    if (ret != ANI_OK) {
        LOGE("find class failed. ret = %{public}d", ret);
        return ret;
    }
    std::string ct = Builder::BuildConstructorName();
    std::string vSign = Builder::BuildSignatureDescriptor({});
    std::string onOffSign = Builder::BuildSignatureDescriptor({Builder::BuildFunctionalObject(1, false)});
    std::string sSign = Builder::BuildSignatureDescriptor({Builder::BuildClass("std.core.String")});
    std::string dSign = Builder::BuildSignatureDescriptor({}, Builder::BuildLong());
    std::array methods = {
        ani_native_function{ct.c_str(), vSign.c_str(), reinterpret_cast<void *>(FileSyncAni::FileSyncConstructor0)},
        ani_native_function{ct.c_str(), sSign.c_str(), reinterpret_cast<void *>(FileSyncAni::FileSyncConstructor1)},
        ani_native_function{"onProgress", onOffSign.c_str(), reinterpret_cast<void *>(FileSyncAni::FileSyncOn)},
        ani_native_function{"offProgress", onOffSign.c_str(), reinterpret_cast<void *>(FileSyncAni::FileSyncOff0)},
        ani_native_function{"offProgress", vSign.c_str(), reinterpret_cast<void *>(FileSyncAni::FileSyncOff1)},
        ani_native_function{"FileSyncStart", vSign.c_str(), reinterpret_cast<void *>(FileSyncAni::FileSyncStart)},
        ani_native_function{"FileSyncStop", vSign.c_str(), reinterpret_cast<void *>(FileSyncAni::FileSyncStop)},
        ani_native_function{"GallerySyncGetLastSyncTime", dSign.c_str(),
                            reinterpret_cast<void *>(FileSyncAni::FileSyncGetLastSyncTime)},
    };

    ret = env->Class_BindNativeMethods(cls, methods.data(), methods.size());
    if (ret != ANI_OK) {
        LOGE("bind native method failed. ret = %{public}d", ret);
        return ret;
    };

    return ANI_OK;
}

static ani_status BindContextOnStaticFunction(ani_env *env)
{
    Type clsName = Builder::BuildClass("@ohos.file.cloudSync.cloudSync.StaticFunction");
    ani_class cls;
    ani_status ret = env->FindClass(clsName.Descriptor().c_str(), &cls);
    if (ret != ANI_OK) {
        LOGE("find class failed. ret = %{public}d", ret);
        return ret;
    }
    std::string siSign = Builder::BuildSignatureDescriptor(
        {Builder::BuildClass("std.core.String")}, Builder::BuildInt());
    std::string sbfSign = Builder::BuildSignatureDescriptor(
        {Builder::BuildClass("std.core.String"), Builder::BuildBoolean(), Builder::BuildFunctionalObject(1, false)});
    std::string sSign = Builder::BuildSignatureDescriptor({Builder::BuildClass("std.core.String")});
    std::string vSign = Builder::BuildSignatureDescriptor({});
    std::string optfSign = Builder::BuildSignatureDescriptor({
        Builder::BuildClass("@ohos.file.cloudSync.cloudSync.OptimizeSpaceParam"),
        Builder::BuildFunctionalObject(1, false)});
    std::array methods = {
        ani_native_function{"getFileSyncStateInner", siSign.c_str(),
            reinterpret_cast<void *>(CloudSyncAni::GetFileSyncState)},
        ani_native_function{"getCoreFileSyncStateInner", siSign.c_str(),
            reinterpret_cast<void *>(CloudSyncAni::GetCoreFileSyncState)},
        ani_native_function{"registerChangeInner", sbfSign.c_str(),
            reinterpret_cast<void *>(CloudSyncAni::RegisterChange)},
        ani_native_function{"unregisterChangeInner", sSign.c_str(),
            reinterpret_cast<void *>(CloudSyncAni::UnRegisterChange)},
        ani_native_function{"optimizeStorageInner", vSign.c_str(),
            reinterpret_cast<void *>(CloudSyncAni::OptimizeStorage)},
        ani_native_function{"startOptimizeSpaceInner", optfSign.c_str(),
            reinterpret_cast<void *>(CloudSyncAni::StartOptimizeStorage)},
        ani_native_function{"stopOptimizeSpaceInner", vSign.c_str(),
            reinterpret_cast<void *>(CloudSyncAni::StopOptimizeStorage)},
    };

    ret = env->Class_BindStaticNativeMethods(cls, methods.data(), methods.size());
    if (ret != ANI_OK) {
        LOGE("bind static native method failed. ret = %{public}d", ret);
        return ret;
    };

    return ANI_OK;
}

static ani_status BindContextOnDownload(ani_env *env)
{
    Type clsName = Builder::BuildClass("@ohos.file.cloudSync.cloudSync.Download");
    ani_class cls;
    ani_status ret = env->FindClass(clsName.Descriptor().c_str(), &cls);
    if (ret != ANI_OK) {
        LOGE("find class failed. ret = %{public}d", ret);
        return ret;
    }
    std::string ct = Builder::BuildConstructorName();
    std::string vSign = Builder::BuildSignatureDescriptor({});
    std::string onOffSign = Builder::BuildSignatureDescriptor({Builder::BuildFunctionalObject(1, false)});
    std::string sSign = Builder::BuildSignatureDescriptor({Builder::BuildClass("std.core.String")});
    std::array methods = {
        ani_native_function{ct.c_str(), vSign.c_str(),
            reinterpret_cast<void *>(CloudDownloadAni::DownloadConstructor)},
        ani_native_function{"onProgress", onOffSign.c_str(), reinterpret_cast<void *>(CloudDownloadAni::DownloadOn)},
        ani_native_function{"offProgress", onOffSign.c_str(),
            reinterpret_cast<void *>(CloudDownloadAni::DownloadOff0)},
        ani_native_function{"offProgress", vSign.c_str(), reinterpret_cast<void *>(CloudDownloadAni::DownloadOff1)},
        ani_native_function{"DownloadStart", sSign.c_str(), reinterpret_cast<void *>(CloudDownloadAni::DownloadStart)},
        ani_native_function{"DownloadStop", sSign.c_str(), reinterpret_cast<void *>(CloudDownloadAni::DownloadStop)},
    };

    ret = env->Class_BindNativeMethods(cls, methods.data(), methods.size());
    if (ret != ANI_OK) {
        LOGE("bind native method failed. ret = %{public}d", ret);
        return ret;
    };

    return ANI_OK;
}

static ani_status BindContextOnFileVersion(ani_env *env)
{
    Type clsName = Builder::BuildClass("@ohos.file.cloudSync.cloudSync.FileVersion");
    ani_class cls;
    ani_status ret = env->FindClass(clsName.Descriptor().c_str(), &cls);
    if (ret != ANI_OK) {
        LOGE("find class failed. ret = %{public}d", ret);
        return ret;
    }
    std::string ct = Builder::BuildConstructorName();
    std::string vSign = Builder::BuildSignatureDescriptor({});
    std::string getHisListSign = Builder::BuildSignatureDescriptor({Builder::BuildClass("std.core.String"),
        Builder::BuildInt()}, Builder::BuildClass("std.core.Array"));
    std::string downloadHisVerSign = Builder::BuildSignatureDescriptor({
        Builder::BuildClass("std.core.String"), Builder::BuildClass("std.core.String"),
        Builder::BuildFunctionalObject(1, false)}, Builder::BuildClass("std.core.String"));
    std::string replaceSign = Builder::BuildSignatureDescriptor({
        Builder::BuildClass("std.core.String"), Builder::BuildClass("std.core.String")});
    std::string isConflictSign = Builder::BuildSignatureDescriptor({
        Builder::BuildClass("std.core.String")}, Builder::BuildBoolean());
    std::string clearConflictSign = Builder::BuildSignatureDescriptor({Builder::BuildClass("std.core.String")});
    std::array methods = {
        ani_native_function{ct.c_str(), vSign.c_str(),
            reinterpret_cast<void *>(FileVersionAni::FileVersionConstructor)},
        ani_native_function{"getHistoryVersionListInner", getHisListSign.c_str(),
            reinterpret_cast<void *>(FileVersionAni::FileVersionGetHistoryVersionList)},
        ani_native_function{"downloadHistoryVersionInner", downloadHisVerSign.c_str(),
            reinterpret_cast<void *>(FileVersionAni::FileVersionDownloadHistoryVersion)},
        ani_native_function{"replaceFileWithHistoryVersionInner", replaceSign.c_str(),
            reinterpret_cast<void *>(FileVersionAni::FileVersionReplaceFileWithHistoryVersion)},
        ani_native_function{"isFileConflictInner", isConflictSign.c_str(),
            reinterpret_cast<void *>(FileVersionAni::FileVersionIsFileConflict)},
        ani_native_function{"clearFileConflictInner", clearConflictSign.c_str(),
            reinterpret_cast<void *>(FileVersionAni::FileVersionClearFileConflict)},
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
        LOGE("Unsupported ANI_VERSION_1");
        return (ani_status)ANI_ERROR;
    }

    auto status = BindContextOnGallery(env);
    if (status != ANI_OK) {
        return status;
    }
    status = BindContextOnMultiDlProgress(env);
    if (status != ANI_OK) {
        return status;
    }
    status = BindContextOnCloudFileCache(env);
    if (status != ANI_OK) {
        return status;
    }
    status = BindContextOnFileSync(env);
    if (status != ANI_OK) {
        return status;
    }
    status = BindContextOnDownload(env);
    if (status != ANI_OK) {
        return status;
    }
    status = BindContextOnStaticFunction(env);
    if (status != ANI_OK) {
        return status;
    }
    status = BindContextOnFileVersion(env);
    if (status != ANI_OK) {
        return status;
    }
    *result = ANI_VERSION_1;
    return ANI_OK;
}