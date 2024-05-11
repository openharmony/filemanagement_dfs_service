/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_CLOUD_DISK_SERVICE_FILE_COLUMN_H
#define OHOS_CLOUD_DISK_SERVICE_FILE_COLUMN_H

#include <vector>

#include "clouddisk_db_const.h"

namespace OHOS {
namespace FileManagement {
namespace CloudDisk {
class FileColumn {
public:
    // Local And Cloud
    static const std::string CLOUD_ID;
    static const std::string IS_DIRECTORY;
    static const std::string FILE_NAME;
    static const std::string PARENT_CLOUD_ID;
    static const std::string FILE_SIZE;
    static const std::string FILE_SHA256;
    static const std::string FILE_TIME_ADDED;
    static const std::string FILE_TIME_EDITED;
    static const std::string FILE_TIME_RECYCLED;
    static const std::string META_TIME_EDITED;
    static const std::string DIRECTLY_RECYCLED;
    static const std::string VERSION;
    static const std::string OPERATE_TYPE;
    static const std::string FILE_STATUS;
    static const std::string ROW_ID;

    // Only Local
    static const std::string FILE_TIME_VISIT;
    static const std::string SYNC_STATUS;
    static const std::string POSITION;
    static const std::string DIRTY_TYPE;
    static const std::string MIME_TYPE;
    static const std::string FILE_TYPE;
    static const std::string FILE_CATEGORY;
    static const std::string IS_FAVORITE;

    // table name
    static const std::string FILES_TABLE;

    // parentCloudId index
    static const std::string PARENT_CLOUD_ID_INDEX;

    // create FileTable sql
    static const std::string CREATE_FILE_TABLE;

    // create parentCloudId index
    static const std::string CREATE_PARENT_CLOUD_ID_INDEX;

    //create IsFavorite
    static const std::string ADD_IS_FAVORITE;

    //create FileStatus
    static const std::string ADD_FILE_STATUS;

    //set FileStatus
    static const std::string SET_FILE_STATUS_DEFAULT;

    // Query FILES_TABLE for FileSystem
    static const std::vector<std::string> FILE_SYSTEM_QUERY_COLUMNS;

    // Query FILES_TABLE for Disk Push
    static const std::vector<std::string> DISK_CLOUD_SYNC_COLUMNS;

    static const std::vector<std::string> LOCAL_COLUMNS;

    // Query pull info columns
    static const std::vector<std::string> PULL_QUERY_COLUMNS;

    // Query local info columns
    static const std::vector<std::string> DISK_ON_UPLOAD_COLUMNS;
};
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS
#endif // OHOS_CLOUD_DISK_SERVICE_FILE_COLUMN_H