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

#include "file_column.h"

#include <string>
#include <vector>

namespace OHOS {
namespace FileManagement {
namespace CloudDisk {
const std::string FileColumn::FILE_ID = "file_id";
const std::string FileColumn::CLOUD_ID = "cloud_id";
const std::string FileColumn::FILE_NAME = "file_name";
const std::string FileColumn::PARENT_ID = "parent_id";
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
const std::string FileColumn::SYNC_STATUS = "sync_status";
const std::string FileColumn::POSITION = "position";
const std::string FileColumn::DIRTY_TYPE = "dirty_type";
const std::string FileColumn::SRC_PATH = "src_path";
const std::string FileColumn::LCD_PATH = "lcd_path";
const std::string FileColumn::THUMBNAIL_PATH = "thumbnail_path";
const std::string FileColumn::LBR_VIDEO_PATH = "lbr_video_path";
const std::string FileColumn::MIME_TYPE = "mimetype";
const std::string FileColumn::IS_DIRECTORY = "isDirectory";
const std::string FileColumn::FILE_TYPE = "file_type";
const std::string FileColumn::FILE_CATEGORY = "file_category";

const std::string FileColumn::FILES_TABLE = "CloudDisk";

const std::string FileColumn::CREATE_FILE_TABLE = "CREATE TABLE IF NOT EXISTS " +
    FILES_TABLE + " (" +
    FILE_ID + " INTEGER PRIMARY KEY AUTOINCREMENT, " +
    CLOUD_ID + " TEXT, " +
    FILE_NAME + " TEXT, " +
    PARENT_ID + " TEXT, " +
    PARENT_CLOUD_ID + " TEXT, " +
    FILE_SIZE + " BIGINT, " +
    FILE_SHA256 + " TEXT, " +
    FILE_TIME_ADDED + " BIGINT, " +
    FILE_TIME_EDITED + " BIGINT, " +
    FILE_TIME_RECYCLED + " BIGINT DEFAULT 0, " +
    META_TIME_EDITED + " BIGINT, " +
    FILE_TIME_VISIT + " BIGINT DEFAULT 0, " +
    DIRECTLY_RECYCLED + " INT DEFAULT 0, " +
    VERSION + " BIGINT DEFAULT 0, " +
    SYNC_STATUS + " INT DEFAULT 0, " +
    POSITION + " INT DEFAULT 1, " +
    DIRTY_TYPE + " INT DEFAULT 1, " +
    SRC_PATH + " TEXT, " +
    LCD_PATH + " TEXT, " +
    THUMBNAIL_PATH + " TEXT, " +
    LBR_VIDEO_PATH + " TEXT, " +
    MIME_TYPE + " TEXT, " +
    IS_DIRECTORY + " INT, " +
    FILE_TYPE + " INTEGER, " +
    FILE_CATEGORY + " INTEGER)";

const std::set<std::string> FileColumn::FILE_COLUMNS = {
    FILE_ID, CLOUD_ID, FILE_NAME, PARENT_ID, PARENT_CLOUD_ID, FILE_SIZE, FILE_SHA256, FILE_TIME_ADDED,
    FILE_TIME_EDITED, FILE_TIME_RECYCLED, META_TIME_EDITED, FILE_TIME_VISIT, DIRECTLY_RECYCLED, VERSION,
    SYNC_STATUS, POSITION, DIRTY_TYPE, SRC_PATH, LCD_PATH, THUMBNAIL_PATH, LBR_VIDEO_PATH,
    MIME_TYPE, IS_DIRECTORY, FILE_TYPE, FILE_CATEGORY
};

const std::set<std::string> FileColumn::DEFAULT_FETCH_COLUMNS = {
    FILE_ID, CLOUD_ID, PARENT_ID, PARENT_CLOUD_ID, FILE_SIZE, FILE_TIME_ADDED,
    FILE_TIME_EDITED, FILE_TIME_RECYCLED, META_TIME_EDITED, VERSION, POSITION,
    DIRTY_TYPE
};

const std::vector<std::string> FileColumn::LOCAL_COLUMNS = {
    FILE_ID, PARENT_ID, FILE_TIME_VISIT, SYNC_STATUS, POSITION, DIRTY_TYPE, SRC_PATH,
    LCD_PATH, THUMBNAIL_PATH, LBR_VIDEO_PATH, MIME_TYPE, IS_DIRECTORY, FILE_TYPE, FILE_CATEGORY,
};

bool FileColumn::IsFileColumn(const std::string &columnName)
{
    return (FILE_COLUMNS.find(columnName) != FILE_COLUMNS.end());
}
const std::vector<std::string> FileColumn::PULL_QUERY_COLUMNS = {
    CLOUD_ID, FILE_TIME_RECYCLED, VERSION, DIRTY_TYPE, POSITION,
};
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS