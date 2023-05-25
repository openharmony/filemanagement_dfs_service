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

#ifndef OHOS_CLOUD_SYNC_SERVICE_GALLERY_FILE_CONST_H
#define OHOS_CLOUD_SYNC_SERVICE_GALLERY_FILE_CONST_H

#include <string>

#include "medialibrary_db_const.h"
#include "medialibrary_type_const.h"

#include "data_convertor.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
/* basic */
const std::string FILE_ALBUM_ID = "albumId";
const std::string FILE_FILE_NAME = "fileName";
const std::string FILE_HASH_ID = "hashId";
const std::string FILE_SOURCE = "source";
const std::string FILE_FILETYPE = "fileType";
const std::string FILE_CREATED_TIME = "createdTime";
const std::string FILE_FAVORITE = "favorite";
const std::string FILE_DESCRIPTION = "description";
const std::string FILE_RECYCLED = "recycled";
/* properties */
const std::string FILE_PROPERTIES = "properties";
const std::string FILE_HEIGHT = "height";
const std::string FILE_ROTATION = "rotation";
const std::string FILE_WIDTH = "width";
const std::string FILE_POSITION = "position";
const std::string FILE_DATA_MODIFIED = "data_modified";
const std::string FILE_DETAIL_TIME = "detail_time";
const std::string FILE_FILE_CREATE_TIME = "fileCreateTime";
const std::string FILE_FIRST_UPDATE_TIME = "first_update_time";
const std::string FILE_RELATIVE_BUCKET_ID = "relative_bucket_id";
const std::string FILE_SOURCE_FILE_NAME = "sourceFileName";
const std::string FILE_SOURCE_PATH = "sourcePath";
const std::string FILE_TIME_ZONE = "time_zone";
/* properties - general */
const std::string FILE_GENERAL = "general";
/* attachments */
const std::string FILE_ATTACHMENTS = "attachments";
const std::string FILE_CONTENT = "content";
const std::string FILE_THUMBNAIL = "thumbnail";
const std::string FILE_LCD = "lcdThumbnail";

const int32_t POSITION_LOCAL = 1;
const int32_t POSITION_CLOUD = 2;
const int32_t POSITION_BOTH = 3;

const std::vector<std::string> GALLERY_FILE_COLUMNS = {
    Media::MEDIA_DATA_DB_FILE_PATH,
    Media::MEDIA_DATA_DB_SIZE,
    Media::MEDIA_DATA_DB_DATE_MODIFIED,
    Media::MEDIA_DATA_DB_META_DATE_MODIFIED,
    Media::MEDIA_DATA_DB_DATE_ADDED,
    Media::MEDIA_DATA_DB_MIME_TYPE,
    Media::MEDIA_DATA_DB_TITLE,
    Media::MEDIA_DATA_DB_NAME,
    Media::MEDIA_DATA_DB_ORIENTATION,
    Media::MEDIA_DATA_DB_THUMBNAIL,
    Media::MEDIA_DATA_DB_LCD,
    Media::MEDIA_DATA_DB_MEDIA_TYPE,
    Media::MEDIA_DATA_DB_IS_FAV,
    Media::MEDIA_DATA_DB_RELATIVE_PATH,
    Media::MEDIA_DATA_DB_HEIGHT,
    Media::MEDIA_DATA_DB_WIDTH,
    Media::MEDIA_DATA_DB_IS_TRASH,
    /* keep cloud_id at the last, so RecordToValueBucket can skip it*/
    Media::MEDIA_DATA_DB_CLOUD_ID
};

const std::vector<DataType> GALLERY_FILE_COLUMN_TYPES = {
    DataType::STRING,       /* data */
    DataType::LONG,         /* size */
    DataType::LONG,         /* date_modified */
    DataType::LONG,         /* meta_date_modified */
    DataType::LONG,         /* date_added */
    DataType::STRING,       /* mime_type */
    DataType::STRING,       /* title */
    DataType::STRING,       /* display_name */
    DataType::INT,          /* orientation */
    DataType::STRING,       /* thumbnail */
    DataType::STRING,       /* lcd */
    DataType::INT,          /* media_type */
    DataType::BOOL,         /* is_favorite */
    DataType::STRING,       /* relative_path */
    DataType::INT,          /* height */
    DataType::INT,          /* width */
    DataType::INT,          /* is_trash */
    DataType::STRING        /* cloud_id */
};
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
#endif // OHOS_CLOUD_SYNC_SERVICE_GALLERY_FILE_CONST_H
