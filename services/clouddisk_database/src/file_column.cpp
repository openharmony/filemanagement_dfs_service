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

#include "file_column.h"

namespace OHOS {
namespace FileManagement {
namespace CloudDisk {
const std::string FileColumn::CLOUD_ID = "cloud_id";
const std::string FileColumn::IS_DIRECTORY = "isDirectory";
const std::string FileColumn::FILE_NAME = "file_name";
const std::string FileColumn::PARENT_CLOUD_ID = "parent_cloud_id";
const std::string FileColumn::FILE_SIZE = "file_size";
const std::string FileColumn::FILE_SHA256 = "sha256";
const std::string FileColumn::FILE_TIME_ADDED = "time_added";
const std::string FileColumn::FILE_TIME_EDITED = "time_edited";
const std::string FileColumn::FILE_TIME_RECYCLED = "time_recycled";
const std::string FileColumn::META_TIME_EDITED = "meta_time_edited";
const std::string FileColumn::FILE_TIME_VISIT = "time_visit";
const std::string FileColumn::DIRECTLY_RECYCLED = "directly_recycled";
const std::string FileColumn::VERSION = "version";
const std::string FileColumn::OPERATE_TYPE = "operateType";
const std::string FileColumn::SYNC_STATUS = "sync_status";
const std::string FileColumn::POSITION = "position";
const std::string FileColumn::DIRTY_TYPE = "dirty_type";
const std::string FileColumn::MIME_TYPE = "mimetype";
const std::string FileColumn::FILE_TYPE = "file_type";
const std::string FileColumn::FILE_CATEGORY = "file_category";
const std::string FileColumn::IS_FAVORITE = "isFavorite";
const std::string FileColumn::FILE_STATUS = "file_status";
const std::string FileColumn::ROW_ID = "rowid";

const std::string FileColumn::FILES_TABLE = "CloudDisk";
const std::string FileColumn::PARENT_CLOUD_ID_INDEX = "parentCloudId_index";

const std::string FileColumn::CREATE_FILE_TABLE = "CREATE TABLE IF NOT EXISTS " +
    FILES_TABLE + " (" +
    CLOUD_ID + " TEXT PRIMARY KEY NOT NULL, " +
    IS_DIRECTORY + " INT, " +
    FILE_NAME + " TEXT NOT NULL, " +
    PARENT_CLOUD_ID + " TEXT NOT NULL, " +
    FILE_SIZE + " BIGINT, " +
    FILE_SHA256 + " TEXT, " +
    FILE_TIME_ADDED + " BIGINT, " +
    FILE_TIME_EDITED + " BIGINT, " +
    FILE_TIME_RECYCLED + " BIGINT DEFAULT 0, " +
    META_TIME_EDITED + " BIGINT, " +
    FILE_TIME_VISIT + " BIGINT DEFAULT 0, " +
    DIRECTLY_RECYCLED + " INT DEFAULT 0, " +
    VERSION + " BIGINT DEFAULT 0, " +
    OPERATE_TYPE + " BIGINT DEFAULT 0, " +
    SYNC_STATUS + " INT DEFAULT 0, " +
    POSITION + " INT DEFAULT 1, " +
    DIRTY_TYPE + " INT DEFAULT 1, " +
    MIME_TYPE + " TEXT, " +
    FILE_TYPE + " INT, " +
    FILE_CATEGORY + " TEXT, " +
    IS_FAVORITE + " INT DEFAULT 0, " +
    FILE_STATUS + " INT DEFAULT 4)";

const std::string FileColumn::CREATE_PARENT_CLOUD_ID_INDEX = "CREATE INDEX IF NOT EXISTS " +
    PARENT_CLOUD_ID_INDEX + " ON " + FILES_TABLE +
    " (" + PARENT_CLOUD_ID + ")";

const std::string FileColumn::ADD_IS_FAVORITE = "ALTER Table " + FILES_TABLE +
    " ADD COLUMN " + IS_FAVORITE + " INT DEFAULT 0";

const std::string FileColumn::ADD_FILE_STATUS = "ALTER Table " + FILES_TABLE +
    " ADD COLUMN " + FILE_STATUS + " INT";

const std::string FileColumn::SET_FILE_STATUS_DEFAULT = "UPDATE " + FILES_TABLE +
    " SET " + FILE_STATUS + " = 4 WHERE " + FILE_STATUS + " IS NULL";

const std::vector<std::string> FileColumn::FILE_SYSTEM_QUERY_COLUMNS = {
    FILE_NAME,
    CLOUD_ID,
    PARENT_CLOUD_ID,
    POSITION,
    FILE_SIZE,
    META_TIME_EDITED,
    FILE_TIME_ADDED,
    FILE_TIME_EDITED,
    IS_DIRECTORY,
    ROW_ID
};

const std::vector<std::string> FileColumn::DISK_CLOUD_SYNC_COLUMNS = {
    CLOUD_ID,
    IS_DIRECTORY,
    FILE_NAME,
    PARENT_CLOUD_ID,
    FILE_SIZE,
    FILE_SHA256,
    FILE_TIME_ADDED,
    FILE_TIME_EDITED,
    FILE_TIME_RECYCLED,
    META_TIME_EDITED,
    DIRECTLY_RECYCLED,
    VERSION,
    OPERATE_TYPE
};

const std::vector<std::string> FileColumn::LOCAL_COLUMNS = {
    FILE_TIME_VISIT, SYNC_STATUS, POSITION, DIRTY_TYPE,
    MIME_TYPE, IS_DIRECTORY, FILE_TYPE, FILE_CATEGORY,
};

const std::vector<std::string> FileColumn::PULL_QUERY_COLUMNS = {
    CLOUD_ID, FILE_TIME_RECYCLED, VERSION, DIRTY_TYPE, POSITION,
    FILE_TIME_EDITED, FILE_SHA256, FILE_SIZE, FILE_NAME, PARENT_CLOUD_ID, ROW_ID,
};

const std::vector<std::string> FileColumn::DISK_ON_UPLOAD_COLUMNS = {
    CLOUD_ID,
    FILE_TIME_EDITED,
    META_TIME_EDITED
};
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS