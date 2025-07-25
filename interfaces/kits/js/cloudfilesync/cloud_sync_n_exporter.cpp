/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include "cloud_sync_n_exporter.h"

#include "cloud_file_cache_napi.h"
#include "cloud_file_napi.h"
#include "cloud_file_version_napi.h"
#include "cloud_sync_napi.h"
#include "file_sync_napi.h"
#include "gallery_sync_napi.h"
#include "multi_download_progress_napi.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
using namespace FileManagement::LibN;
using namespace std;

static void InitState(napi_env env, napi_value exports)
{
    char propertyName[] = "State";
    napi_value obj = nullptr;
    napi_create_object(env, &obj);
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("RUNNING",
                                     NVal::CreateInt32(env, (int32_t)State::CLOUD_FILE_DOWNLOAD_RUNNING).val_),
        DECLARE_NAPI_STATIC_PROPERTY("COMPLETED",
                                     NVal::CreateInt32(env, (int32_t)State::CLOUD_FILE_DOWNLOAD_COMPLETED).val_),
        DECLARE_NAPI_STATIC_PROPERTY("FAILED",
                                     NVal::CreateInt32(env, (int32_t)State::CLOUD_FILE_DOWNLOAD_FAILED).val_),
        DECLARE_NAPI_STATIC_PROPERTY("STOPPED",
                                     NVal::CreateInt32(env, (int32_t)State::CLOUD_FILE_DOWNLOAD_STOPPED).val_),
    };
    napi_define_properties(env, obj, sizeof(desc) / sizeof(desc[0]), desc);
    napi_set_named_property(env, exports, propertyName, obj);
}

/***********************************************
 * Module export and register
 ***********************************************/
napi_value CloudSyncExport(napi_env env, napi_value exports)
{
    InitCloudSyncState(env, exports);
    InitErrorType(env, exports);
    InitDownloadErrorType(env, exports);
    InitState(env, exports);
    InitFileSyncState(env, exports);
    InitFileState(env, exports);
    InitCloudSyncApi(env, exports);
    InitNotifyType(env, exports);
    InitCloudSyncFuncs(env, exports);
    InitOptimizeState(env, exports);
    InitDownloadFileType(env, exports);

    std::vector<std::unique_ptr<NExporter>> products;
    products.emplace_back(std::make_unique<MultiDlProgressNapi>(env, exports));
    products.emplace_back(std::make_unique<CloudFileCacheNapi>(env, exports));
    products.emplace_back(std::make_unique<CloudFileDownloadNapi>(env, exports));
    products.emplace_back(std::make_unique<GallerySyncNapi>(env, exports));
    products.emplace_back(std::make_unique<FileSyncNapi>(env, exports));
    products.emplace_back(std::make_unique<FileVersionNapi>(env, exports));
    for (auto &&product : products) {
        if (!product->Export()) {
            LOGE("INNER BUG. Failed to export class %{public}s for module cloudSyncDownload ",
                 product->GetClassName().c_str());
            return nullptr;
        } else {
            LOGI("Class %{public}s for module cloudSyncDownload has been exported", product->GetClassName().c_str());
        }
    }
    return exports;
}

void InitCloudSyncState(napi_env env, napi_value exports)
{
    char propertyName[] = "SyncState";
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("UPLOADING", NVal::CreateInt32(env, UPLOADING).val_),
        DECLARE_NAPI_STATIC_PROPERTY("UPLOAD_FAILED", NVal::CreateInt32(env, UPLOAD_FAILED).val_),
        DECLARE_NAPI_STATIC_PROPERTY("DOWNLOADING", NVal::CreateInt32(env, DOWNLOADING).val_),
        DECLARE_NAPI_STATIC_PROPERTY("DOWNLOAD_FAILED", NVal::CreateInt32(env, DOWNLOAD_FAILED).val_),
        DECLARE_NAPI_STATIC_PROPERTY("COMPLETED", NVal::CreateInt32(env, COMPLETED).val_),
        DECLARE_NAPI_STATIC_PROPERTY("STOPPED", NVal::CreateInt32(env, STOPPED).val_),
    };
    napi_value obj = nullptr;
    napi_create_object(env, &obj);
    napi_define_properties(env, obj, sizeof(desc) / sizeof(desc[0]), desc);
    napi_set_named_property(env, exports, propertyName, obj);
}

void InitOptimizeState(napi_env env, napi_value exports)
{
    char propertyName[] = "OptimizeState";
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("RUNNING", NVal::CreateInt32(env, OPTIMIZE_RUNNING).val_),
        DECLARE_NAPI_STATIC_PROPERTY("COMPLETED", NVal::CreateInt32(env, OPTIMIZE_COMPLETED).val_),
        DECLARE_NAPI_STATIC_PROPERTY("FAILED", NVal::CreateInt32(env, OPTIMIZE_FAILED).val_),
        DECLARE_NAPI_STATIC_PROPERTY("STOPPED", NVal::CreateInt32(env, OPTIMIZE_STOPPED).val_),
    };
    napi_value obj = nullptr;
    napi_create_object(env, &obj);
    napi_define_properties(env, obj, sizeof(desc) / sizeof(desc[0]), desc);
    napi_set_named_property(env, exports, propertyName, obj);
}

void InitFileSyncState(napi_env env, napi_value exports)
{
    char propertyName[] = "FileSyncState";
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("UPLOADING", NVal::CreateInt32(env, FILESYNCSTATE_UPLOADING).val_),
        DECLARE_NAPI_STATIC_PROPERTY("DOWNLOADING", NVal::CreateInt32(env, FILESYNCSTATE_DOWNLOADING).val_),
        DECLARE_NAPI_STATIC_PROPERTY("COMPLETED", NVal::CreateInt32(env, FILESYNCSTATE_COMPLETED).val_),
        DECLARE_NAPI_STATIC_PROPERTY("STOPPED", NVal::CreateInt32(env, FILESYNCSTATE_STOPPED).val_),
        DECLARE_NAPI_STATIC_PROPERTY("TO_BE_UPLOADED", NVal::CreateInt32(env, FILESYNCSTATE_TO_BE_UPLOADED).val_),
        DECLARE_NAPI_STATIC_PROPERTY("UPLOAD_FAILURE", NVal::CreateInt32(env, FILESYNCSTATE_UPLOAD_FAILURE).val_),
        DECLARE_NAPI_STATIC_PROPERTY("UPLOAD_SUCCESS", NVal::CreateInt32(env, FILESYNCSTATE_UPLOAD_SUCCESS).val_),
    };
    napi_value obj = nullptr;
    napi_create_object(env, &obj);
    napi_define_properties(env, obj, sizeof(desc) / sizeof(desc[0]), desc);
    napi_set_named_property(env, exports, propertyName, obj);
}

void InitFileState(napi_env env, napi_value exports)
{
    char propertyName[] = "FileState";
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("INITIAL_AFTER_DOWNLOAD",
            NVal::CreateInt32(env, FILESTATE_INITIAL_AFTER_DOWNLOAD).val_),
        DECLARE_NAPI_STATIC_PROPERTY("UPLOADING", NVal::CreateInt32(env, FILESTATE_UPLOADING).val_),
        DECLARE_NAPI_STATIC_PROPERTY("STOPPED", NVal::CreateInt32(env, FILESTATE_STOPPED).val_),
        DECLARE_NAPI_STATIC_PROPERTY("TO_BE_UPLOADED", NVal::CreateInt32(env, FILESTATE_TO_BE_UPLOADED).val_),
        DECLARE_NAPI_STATIC_PROPERTY("UPLOAD_SUCCESS", NVal::CreateInt32(env, FILESTATE_UPLOAD_SUCCESS).val_),
        DECLARE_NAPI_STATIC_PROPERTY("UPLOAD_FAILURE", NVal::CreateInt32(env, FILESTATE_UPLOAD_FAILURE).val_),
    };
    napi_value obj = nullptr;
    napi_create_object(env, &obj);
    napi_define_properties(env, obj, sizeof(desc) / sizeof(desc[0]), desc);
    napi_set_named_property(env, exports, propertyName, obj);
}

void InitErrorType(napi_env env, napi_value exports)
{
    char propertyName[] = "ErrorType";
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("NO_ERROR", NVal::CreateInt32(env, NO_ERROR).val_),
        DECLARE_NAPI_STATIC_PROPERTY("NETWORK_UNAVAILABLE", NVal::CreateInt32(env, NETWORK_UNAVAILABLE).val_),
        DECLARE_NAPI_STATIC_PROPERTY("WIFI_UNAVAILABLE", NVal::CreateInt32(env, WIFI_UNAVAILABLE).val_),
        DECLARE_NAPI_STATIC_PROPERTY("BATTERY_LEVEL_LOW", NVal::CreateInt32(env, BATTERY_LEVEL_LOW).val_),
        DECLARE_NAPI_STATIC_PROPERTY("BATTERY_LEVEL_WARNING", NVal::CreateInt32(env, BATTERY_LEVEL_WARNING).val_),
        DECLARE_NAPI_STATIC_PROPERTY("CLOUD_STORAGE_FULL", NVal::CreateInt32(env, CLOUD_STORAGE_FULL).val_),
        DECLARE_NAPI_STATIC_PROPERTY("LOCAL_STORAGE_FULL", NVal::CreateInt32(env, LOCAL_STORAGE_FULL).val_),
        DECLARE_NAPI_STATIC_PROPERTY("DEVICE_TEMPERATURE_TOO_HIGH",
            NVal::CreateInt32(env, DEVICE_TEMPERATURE_TOO_HIGH).val_),
    };
    napi_value obj = nullptr;
    napi_create_object(env, &obj);
    napi_define_properties(env, obj, sizeof(desc) / sizeof(desc[0]), desc);
    napi_set_named_property(env, exports, propertyName, obj);
}

void InitDownloadErrorType(napi_env env, napi_value exports)
{
    char propertyName[] = "DownloadErrorType";
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("NO_ERROR", NVal::CreateInt32(env, NO_ERROR).val_),
        DECLARE_NAPI_STATIC_PROPERTY("UNKNOWN_ERROR",
            NVal::CreateInt32(env, DownloadProgressObj::UNKNOWN_ERROR).val_),
        DECLARE_NAPI_STATIC_PROPERTY("NETWORK_UNAVAILABLE",
            NVal::CreateInt32(env, DownloadProgressObj::NETWORK_UNAVAILABLE).val_),
        DECLARE_NAPI_STATIC_PROPERTY("LOCAL_STORAGE_FULL",
            NVal::CreateInt32(env, DownloadProgressObj::LOCAL_STORAGE_FULL).val_),
        DECLARE_NAPI_STATIC_PROPERTY("CONTENT_NOT_FOUND",
            NVal::CreateInt32(env, DownloadProgressObj::CONTENT_NOT_FOUND).val_),
        DECLARE_NAPI_STATIC_PROPERTY("FREQUENT_USER_REQUESTS",
            NVal::CreateInt32(env, DownloadProgressObj::FREQUENT_USER_REQUESTS).val_),
    };
    napi_value obj = nullptr;
    napi_create_object(env, &obj);
    napi_define_properties(env, obj, sizeof(desc) / sizeof(desc[0]), desc);
    napi_set_named_property(env, exports, propertyName, obj);
}

void InitCloudSyncApi(napi_env env, napi_value exports)
{
    static napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("getFileSyncState", CloudSyncNapi::GetFileSyncState),
        DECLARE_NAPI_FUNCTION("getCoreFileSyncState", CloudSyncNapi::GetCoreFileSyncState),
        DECLARE_NAPI_FUNCTION("optimizeStorage", CloudSyncNapi::OptimizeStorage),
        DECLARE_NAPI_FUNCTION("startOptimizeSpace", CloudSyncNapi::StartOptimizeStorage),
        DECLARE_NAPI_FUNCTION("stopOptimizeSpace", CloudSyncNapi::StopOptimizeStorage),
    };
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
}

void InitNotifyType(napi_env env, napi_value exports)
{
    const char propertyName[] = "NotifyType";
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("NOTIFY_ADDED", NVal::CreateInt32(env, NotifyType::NOTIFY_ADDED).val_),
        DECLARE_NAPI_STATIC_PROPERTY("NOTIFY_MODIFIED", NVal::CreateInt32(env, NotifyType::NOTIFY_MODIFIED).val_),
        DECLARE_NAPI_STATIC_PROPERTY("NOTIFY_DELETED", NVal::CreateInt32(env, NotifyType::NOTIFY_DELETED).val_),
        DECLARE_NAPI_STATIC_PROPERTY("NOTIFY_RENAMED", NVal::CreateInt32(env, NotifyType::NOTIFY_RENAMED).val_),
    };
    napi_value obj = nullptr;
    napi_create_object(env, &obj);
    napi_define_properties(env, obj, sizeof(desc) / sizeof(desc[0]), desc);
    napi_set_named_property(env, exports, propertyName, obj);
}

void InitCloudSyncFuncs(napi_env env, napi_value exports)
{
    static napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("registerChange", CloudSyncNapi::RegisterChange),
        DECLARE_NAPI_FUNCTION("unregisterChange", CloudSyncNapi::UnregisterChange),
    };
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
}

void InitDownloadFileType(napi_env env, napi_value exports)
{
    char propertyName[] = "DownloadFileType";
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("CONTENT", NVal::CreateInt32(env, FieldKey::FIELDKEY_CONTENT).val_),
        DECLARE_NAPI_STATIC_PROPERTY("THUMBNAIL", NVal::CreateInt32(env, FieldKey::FIELDKEY_THUMB).val_),
        DECLARE_NAPI_STATIC_PROPERTY("LCD", NVal::CreateInt32(env, FieldKey::FIELDKEY_LCD).val_),
    };
    napi_value obj = nullptr;
    napi_create_object(env, &obj);
    napi_define_properties(env, obj, sizeof(desc) / sizeof(desc[0]), desc);
    napi_set_named_property(env, exports, propertyName, obj);
}

static napi_module _module = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = CloudSyncExport,
    .nm_modname = "file.cloudSync",
    .nm_priv = ((void *)0),
    .reserved = {0}
};

extern "C" __attribute__((constructor)) void RegisterModule(void)
{
    napi_module_register(&_module);
}
} // namespace OHOS::FileManagement::CloudSync
