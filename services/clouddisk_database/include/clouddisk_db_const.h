/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_CLOUD_DISK_SERVICE_DB_CONST_H
#define OHOS_CLOUD_DISK_SERVICE_DB_CONST_H

#include <string>
#include "clouddisk_type_const.h"

namespace OHOS {
namespace FileManagement {
namespace CloudDisk {
//rdb
const std::string DATA_SERVICE_EL1_PUBLIC_CLOUDFILE = "/data/service/el1/public/cloudfile/";
const std::string CLOUD_DISK_DATABASE_NAME = "clouddisk.db";
//rdb version
const int32_t CLOUD_DISK_RDB_VERSION = 5;
enum {
    VERSION_ADD_PARENT_CLOUD_ID_INDEX = 2,
    VERSION_FIX_FILE_TRIGGER = 3,
    VERSION_FIX_CREATE_AND_LOCAL_TRIGGER = 4,
    VERSION_ADD_STATUS_ERROR_FAVORITE = 5,
};
} // namespace CloudDisk
} // namespace FileManagement
} // namaspace OHOS

#endif // OHOS_CLOUD_DISK_SERVICE_DB_CONST_H