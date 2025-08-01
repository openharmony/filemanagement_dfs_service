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
    static const std::string CHECK_FLAG;
    static const std::string ROOT_DIRECTORY;
    static const std::string ATTRIBUTE;

    // Only Local
    static const std::string FILE_TIME_VISIT;
    static const std::string SYNC_STATUS;
    static const std::string POSITION;
    static const std::string DIRTY_TYPE;
    static const std::string THM_FLAG;
    static const std::string LCD_FLAG;
    static const std::string MIME_TYPE;
    static const std::string FILE_TYPE;
    static const std::string FILE_CATEGORY;
    static const std::string IS_FAVORITE;
    static const std::string NO_NEED_UPLOAD;
    static const std::string SRC_CLOUD_ID;
    static const std::string THM_SIZE;
    static const std::string LCD_SIZE;
    static const std::string SOURCE_CLOUD_ID;
    static const std::string LOCAL_FLAG;

    /*  description for LOCAL_FLAG shift
     *   |-----------------------bit 63 ~ 1 -------------|---bit 0----|
     *   |--------------------------reserved-------------|-isConflict-|
    */
    static const uint64_t LOCAL_FLAG_MASK_ISCONFLICT;

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

    //set CheckFlag
    static const std::string ADD_CHECK_FLAG;

    // set attribute
    static const std::string ADD_ATTRIBUTE;

    // set thm
    static const std::string ADD_THM_FLAG;

    static const std::string ADD_LCD_FLAG;

    static const std::string ADD_UPLOAD_FLAG;

    static const std::string ADD_THM_SIZE;

    static const std::string ADD_LCD_SIZE;

    static const std::string ADD_SOURCE_CLOUD_ID;

    // Add copy field
    static const std::string ADD_SRC_CLOUD_ID;

    static const std::string ADD_LOCAL_FLAG;

    // Query FILES_TABLE for FileSystem
    static const std::vector<std::string> FILE_SYSTEM_QUERY_COLUMNS;

    // Query FILES_TABLE for Disk Push
    static const std::vector<std::string> DISK_CLOUD_SYNC_COLUMNS;

    static const std::vector<std::string> LOCAL_COLUMNS;

    // Query pull info columns
    static const std::vector<std::string> PULL_QUERY_COLUMNS;

    // Query local info columns
    static const std::vector<std::string> DISK_ON_UPLOAD_COLUMNS;

    static const std::vector<std::string> EXT_ATTR_QUERY_COLUMNS;

    // Query FILES_TABLE for CopyFile
    static const std::vector<std::string> DISK_CLOUD_FOR_COPY;

    // Query FILES_TABLE for ThmInsert
    static const std::vector<std::string> DISK_CLOUD_FOR_THM_INSERT;
};
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS
#endif // OHOS_CLOUD_DISK_SERVICE_FILE_COLUMN_H