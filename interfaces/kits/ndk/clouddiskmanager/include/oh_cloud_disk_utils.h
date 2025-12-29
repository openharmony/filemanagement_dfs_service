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

#ifndef FILEMANAGEMENT_KIT_OH_CLOUD_DISK_MANAGER_UTILS_H
#define FILEMANAGEMENT_KIT_OH_CLOUD_DISK_MANAGER_UTILS_H

#include <unordered_map>

#include "cloud_disk_service_error.h"
#include "cloud_disk_error_code.h"

CloudDisk_ErrorCode ConvertToErrorCode(int32_t innerErrorCode);
char *AllocField(const char *value, size_t length);
bool IsValidPathInfo(const char *path, size_t length);

const std::unordered_map<int32_t, CloudDisk_ErrorCode> innerToNErrTable = {
    {OHOS::FileManagement::CloudDiskService::CloudDiskServiceErrCode::E_OK, CLOUD_DISK_OK},
    {OHOS::FileManagement::CloudDiskService::CloudDiskServiceErrCode::E_INVALID_ARG, CLOUD_DISK_INVALID_ARG},
    {OHOS::FileManagement::CloudDiskService::CloudDiskServiceErrCode::E_SYNC_FOLDER_PATH_UNAUTHORIZED,
        CLOUD_DISK_SYNC_FOLDER_PATH_UNAUTHORIZED},
    {OHOS::FileManagement::CloudDiskService::CloudDiskServiceErrCode::E_IPC_FAILED, CLOUD_DISK_IPC_FAILED},
    {OHOS::FileManagement::CloudDiskService::CloudDiskServiceErrCode::E_SYNC_FOLDER_LIMIT_EXCEEDED,
        CLOUD_DISK_SYNC_FOLDER_LIMIT_EXCEEDED},
    {OHOS::FileManagement::CloudDiskService::CloudDiskServiceErrCode::E_CONFLICT_THIS_APP,
        CLOUD_DISK_CONFLICT_THIS_APP},
    {OHOS::FileManagement::CloudDiskService::CloudDiskServiceErrCode::E_CONFLICT_OTHER_APP,
        CLOUD_DISK_CONFLICT_OTHER_APP},
    {OHOS::FileManagement::CloudDiskService::CloudDiskServiceErrCode::E_REGISTER_SYNC_FOLDER_FAILED,
        CLOUD_DISK_REGISTER_SYNC_FOLDER_FAILED},
    {OHOS::FileManagement::CloudDiskService::CloudDiskServiceErrCode::E_SYNC_FOLDER_NOT_REGISTERED,
        CLOUD_DISK_SYNC_FOLDER_NOT_REGISTERED},
    {OHOS::FileManagement::CloudDiskService::CloudDiskServiceErrCode::E_UNREGISTER_SYNC_FOLDER_FAILED,
        CLOUD_DISK_UNREGISTER_SYNC_FOLDER_FAILED},
    {OHOS::FileManagement::CloudDiskService::CloudDiskServiceErrCode::E_SYNC_FOLDER_PATH_NOT_EXIST,
        CLOUD_DISK_SYNC_FOLDER_PATH_NOT_EXIST},
    {OHOS::FileManagement::CloudDiskService::CloudDiskServiceErrCode::E_LISTENER_NOT_REGISTERED,
        CLOUD_DISK_LISTENER_NOT_REGISTERED},
    {OHOS::FileManagement::CloudDiskService::CloudDiskServiceErrCode::E_LISTENER_ALREADY_REGISTERED,
        CLOUD_DISK_LISTENER_ALREADY_REGISTERED},
    {OHOS::FileManagement::CloudDiskService::CloudDiskServiceErrCode::E_INVALID_CHANGE_SEQUENCE,
        CLOUD_DISK_INVALID_CHANGE_SEQUENCE},
    {OHOS::FileManagement::CloudDiskService::CloudDiskServiceErrCode::E_TRY_AGAIN, CLOUD_DISK_TRY_AGAIN},
    {OHOS::FileManagement::CloudDiskService::CloudDiskServiceErrCode::E_NOT_ALLOWED, CLOUD_DISK_NOT_ALLOWED},
    {OHOS::FileManagement::CloudDiskService::CloudDiskServiceErrCode::E_NOT_SUPPORTED, CLOUD_DISK_NOT_SUPPORTED},
    {OHOS::FileManagement::CloudDiskService::CloudDiskServiceErrCode::E_PERMISSION_DENIED,
        CLOUD_DISK_PERMISSION_DENIED},
};

#define CHECK_NULLPTR_AND_CONTINUE(ptr, ...)                  \
    if ((ptr) == nullptr) {                                   \
        LOGE("Pointer " #ptr " is nullptr, skip this item."); \
        __VA_ARGS__;                                          \
        continue;                                             \
    }
#endif // FILEMANAGEMENT_KIT_OH_CLOUD_DISK_MANAGER_UTILS_H