/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_FILEMGMT_DFS_ERROR_H
#define OHOS_FILEMGMT_DFS_ERROR_H

#include <string_view>

#include "errors.h"

namespace OHOS::FileManagement {
enum {
    /**
     * Module type : Cloud sync service
     */
    CLOUD_SYNC_SERVICE_MODULE = 0x100
};

constexpr std::string_view SPACE_NOT_ENOUGH = "30184039";
constexpr std::string_view AT_FAILED = "31000401";
constexpr std::string_view NAME_CONFLICT = "31084932";
constexpr std::string_view INVALID_FILE = "31004002";

constexpr ErrCode CSS_ERR_OFFSET = ErrCodeOffset(SUBSYS_FILEMANAGEMENT, CLOUD_SYNC_SERVICE_MODULE);

enum CloudSyncServiceErrCode : ErrCode {
    E_OK = ERR_OK,
    E_SEVICE_DIED = CSS_ERR_OFFSET,
    E_INVAL_ARG,
    E_BROKEN_IPC,
    E_SA_LOAD_FAILED,
    E_SERVICE_DESCRIPTOR_IS_EMPTY,
    E_PERMISSION_DENIED,
    E_PERMISSION_SYSTEM,
    E_GET_TOKEN_INFO_ERROR,
    E_SYNCER_NUM_OUT_OF_RANGE,
    E_SYNC_FAILED_CLOUD_NOT_READY,
    E_SYNC_FAILED_BATTERY_LOW,
    E_SYNC_FAILED_BATTERY_TOO_LOW,
    E_SYNC_FAILED_NETWORK_NOT_AVAILABLE,
    E_GET_NETWORK_MANAGER_FAILED,

    /* data syncer */
    E_CLOUD_SDK,
    E_RDB,
    E_CONTEXT,
    E_STOP,
    E_PENDING,
    E_SCHEDULE,
    E_ASYNC_RUN,
    E_PATH
};
} // namespace OHOS::FileManagement

#endif // OHOS_FILEMGMT_DFS_ERROR_H
