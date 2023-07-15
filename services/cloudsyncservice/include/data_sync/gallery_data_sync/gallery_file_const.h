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
const std::string FILE_LOGIC_ALBUM_IDS = "albumIds";
const std::string FILE_ADD_LOGIC_ALBUM_IDS = "addAlbumIds";
const std::string FILE_RM_LOGIC_ALBUM_IDS = "removeAlbumIds";
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
const std::string FILE_THUMB_SIZE = "thumb_size";
const std::string FILE_LCD_SIZE = "lcd_size";
/* properties - general */
const std::string FILE_GENERAL = "general";
/* attachments */
const std::string FILE_ATTACHMENTS = "attachments";
const std::string FILE_CONTENT = "content";
const std::string FILE_THUMBNAIL = "thumbnail";
const std::string FILE_LCD = "lcdThumbnail";

/* file type */
enum {
    FILE_TYPE_IMAGE = 1,
    FILE_TYPE_VIDEO = 4,
};

const int32_t POSITION_LOCAL = 1;
const int32_t POSITION_CLOUD = 2;
const int32_t POSITION_BOTH = 3;

const int32_t NR_LOCAL_INFO = 2;
const std::vector<std::string> GALLERY_FILE_COLUMNS = {
    Media::PhotoColumn::MEDIA_FILE_PATH,
    Media::PhotoColumn::MEDIA_SIZE,
    Media::PhotoColumn::MEDIA_DATE_MODIFIED,
    Media::PhotoColumn::PHOTO_META_DATE_MODIFIED,
    Media::PhotoColumn::MEDIA_DATE_ADDED,
    Media::PhotoColumn::MEDIA_MIME_TYPE,
    Media::PhotoColumn::MEDIA_NAME,
    Media::PhotoColumn::PHOTO_ORIENTATION,
    Media::PhotoColumn::MEDIA_TYPE,
    Media::PhotoColumn::MEDIA_IS_FAV,
    Media::PhotoColumn::PHOTO_HEIGHT,
    Media::PhotoColumn::PHOTO_WIDTH,
    Media::PhotoColumn::PHOTO_SUBTYPE,
    // Media::PhotoColumn::MEDIA_OWNER_PACKAGE,
    Media::PhotoColumn::MEDIA_DEVICE_NAME,
    Media::PhotoColumn::MEDIA_DATE_TAKEN,
    Media::PhotoColumn::MEDIA_DURATION,
    Media::PhotoColumn::MEDIA_DATE_TRASHED,
    // Media::PhotoColumn::MEDIA_DATE_DELETED
    // Media::PhotoColumn::MEDIA_HIDDEN
    Media::PhotoColumn::PHOTO_LATITUDE,
    Media::PhotoColumn::PHOTO_LONGITUDE,
    /* NR_LOCAL_INFO: keep local info in the end */
    Media::MediaColumn::MEDIA_ID,
    Media::PhotoColumn::PHOTO_CLOUD_ID
};

const std::vector<DataType> GALLERY_FILE_COLUMN_TYPES = {
    DataType::STRING,       /* data */
    DataType::LONG,         /* size */
    DataType::LONG,         /* date_modified */
    DataType::LONG,         /* meta_date_modified */
    DataType::LONG,         /* date_added */
    DataType::STRING,       /* mime_type */
    DataType::STRING,       /* display_name */
    DataType::INT,          /* orientation */
    DataType::INT,          /* media_type */
    DataType::BOOL,         /* is_favorite */
    DataType::INT,          /* height */
    DataType::INT,          /* width */
    DataType::INT,          /* subtype */
    // DataType::STRING,       /* owner_package */
    DataType::STRING,       /* device_name */
    DataType::LONG,         /* date_taken */
    DataType::INT,         /* duration */
    DataType::LONG,         /* date_trashed */
    // DataType::LONG         /* date_deleted */
    // DataType::INT         /* hidden */
    DataType::DOUBLE,       /* latitude */
    DataType::DOUBLE,       /* longitude */
    DataType::INT,        /* file_id */
    DataType::STRING        /* cloud_id */
};
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
#endif // OHOS_CLOUD_SYNC_SERVICE_GALLERY_FILE_CONST_H
