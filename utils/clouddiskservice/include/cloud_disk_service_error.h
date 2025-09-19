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
    E_NOT_SUPPORTED = 801,
    E_PERMISSION_DENIED = 201,
};

} // namespace OHOS::FileManagement::CloudDiskService

#endif // CLOUD_DISK_SERVICE_ERROR_H
