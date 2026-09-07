/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef CLOUD_DISK_SERVICE_ERROR_H
#define CLOUD_DISK_SERVICE_ERROR_H

namespace OHOS::FileManagement::CloudDiskService {

enum CloudDiskServiceErrCode : uint32_t {
    E_OK = 0,
    E_GET_TOKEN_INFO_ERROR,
    E_OSACCOUNT,
    E_PATH_NOT_EXIST,
    E_PERM = 13900001,
    E_ACCES = 13900012,

    E_INVALID_ARG = 34400001,
    E_SYNC_FOLDER_PATH_UNAUTHORIZED,
    E_IPC_FAILED,
    E_SYNC_FOLDER_LIMIT_EXCEEDED,
    E_CONFLICT_THIS_APP,
    E_CONFLICT_OTHER_APP,
    E_REGISTER_SYNC_FOLDER_FAILED,
    E_SYNC_FOLDER_NOT_REGISTERED,
    E_UNREGISTER_SYNC_FOLDER_FAILED,
    E_SYNC_FOLDER_PATH_NOT_EXIST,
    E_LISTENER_NOT_REGISTERED,
    E_LISTENER_ALREADY_REGISTERED,
    E_INVALID_CHANGE_SEQUENCE,
    E_TRY_AGAIN,
    E_NOT_ALLOWED,
    E_FILE_ALREADY_EXISTS = 34400016,
    E_NOT_A_PLACEHOLDER = 34400017,
    E_IS_A_PLACEHOLDER = 34400018,
    E_HYDRATE_IN_PROGRESS = 34400019,
    E_NO_SPACE_LEFT = 34400020,
    E_CALLBACK_NOT_REGISTERED = 34400021,
    E_CALLBACK_ALREADY_REGISTERED = 34400022,
    E_NOT_A_DIRECTORY = 34400023,
    E_FILE_NOT_EXIST = 34400024,
    E_NAME_TOO_LONG = 34400025,
    E_FILE_TOO_LARGE = 34400026,
    E_PLACEHOLDER_CUSTOM_INFO_NOT_FOUND = 34400027,
    E_PLACEHOLDER_NOT_FULLY_HYDRATED = 34400028,
    E_DEHYDRATE_DENIED = 34400029,
    E_CANCELLED = 34400030,
    E_ALREADY_HYDRATED = 34400031,
    E_NO_HYDRATION_IN_PROGRESS = 34400032,
    E_INVALID_PLACEHOLDER_STATE = 34400033,
    E_NOT_SUPPORTED = 801,
    E_PERMISSION_DENIED = 201,
    E_PERMISSION_SYSTEM = 202,
};

} // namespace OHOS::FileManagement::CloudDiskService

#endif // CLOUD_DISK_SERVICE_ERROR_H
