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
#include "photo_album_column.h"
#include "photo_map_column.h"
#include "data_sync_const.h"
#include "data_convertor.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {

enum OperationType {
    /* upload */
    FILE_CREATE,
    FILE_DELETE,
    FILE_METADATA_MODIFY,
    FILE_DATA_MODIFY,
    /* download */
    FILE_DOWNLOAD,
    /*clean*/
    FILE_CLEAN
};

/* thumb */
const std::string THUMB_SUFFIX = "THM";
const std::string LCD_SUFFIX = "LCD";

/* basic */
const std::string FILE_LOGIC_ALBUM_IDS = "albumIds";
const std::string FILE_ADD_LOGIC_ALBUM_IDS = "addAlbumIds";
const std::string FILE_RM_LOGIC_ALBUM_IDS = "removeAlbumIds";
const std::string FILE_FILE_NAME = "fileName";
const std::string FILE_HASH_ID = "hashId";
const std::string FILE_SIZE = "size";
const std::string FILE_SOURCE = "source";
const std::string FILE_FILETYPE = "fileType";
const std::string FILE_CREATED_TIME = "createdTime";
const std::string FILE_FAVORITE = "favorite";
const std::string FILE_DESCRIPTION = "description";
const std::string FILE_RECYCLE_TIME = "recycledTime";
const std::string FILE_RECYCLED = "recycled";
const std::string FILE_MIME_TYPE = "mimeType";
const std::string FILE_EDITED_TIME = "editedTime";
const std::string FILE_DATA_MODIFIED = "data_modified";
const std::string FILE_TIME_ZONE = "time_zone";
const std::string FILE_THUMB_SIZE = "thumb_size";
const std::string FILE_LCD_SIZE = "lcd_size";
const std::string FILE_EDITED_TIME_MS = "editedTime_ms";
/* properties */
const std::string FILE_PROPERTIES = "properties";
const std::string FILE_SOURCE_FILE_NAME = "sourceFileName";
const std::string FILE_FIRST_UPDATE_TIME = "first_update_time";
const std::string FILE_FILE_CREATE_TIME = "fileCreateTime";
const std::string FILE_DETAIL_TIME = "detail_time";
const std::string FILE_SOURCE_PATH = "sourcePath";
const std::string FILE_RELATIVE_BUCKET_ID = "relative_bucket_id";
const std::string FILE_POSITION = "position";
const std::string FILE_ROTATION = "rotate";
const std::string FILE_DURATION = "duration";
const std::string FILE_HEIGHT = "height";
const std::string FILE_WIDTH = "width";

/* attributes */
const std::string FILE_ATTRIBUTES = "attributes";

/* local info */
const std::string FILE_LOCAL_ID = "local_id";

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

enum MediaType {
    MEDIA_TYPE_FILE,
    MEDIA_TYPE_IMAGE,
    MEDIA_TYPE_VIDEO,
    MEDIA_TYPE_AUDIO,
};

const int32_t NR_LOCAL_INFO = 2;

const int32_t ORIENTATION_NORMAL = 1;
const int32_t ORIENTATION_ROTATE_90 = 6;
const int32_t ORIENTATION_ROTATE_180 = 3;
const int32_t ORIENTATION_ROTATE_270 = 8;

const int32_t ROTATE_ANGLE_0 = 0;
const int32_t ROTATE_ANGLE_90 = 90;
const int32_t ROTATE_ANGLE_180 = 180;
const int32_t ROTATE_ANGLE_270 = 270;

const int32_t FIRST_MATCH_PARAM = 1;
const int32_t SECOND_MATCH_PARAM = 2;

// util constants
const std::string ALBUM_URI_PREFIX = Media::PhotoAlbumColumns::ALBUM_URI_PREFIX;
const std::string PHOTO_URI_PREFIX = Media::PhotoColumn::PHOTO_URI_PREFIX;
const std::string INVALID_ASSET_ID = "0";

const std::string ASSET_UNIQUE_NUMBER_TABLE = "UniqueNumber";
const std::string ASSET_MEDIA_TYPE = "media_type";
const std::string UNIQUE_NUMBER = "unique_number";
const std::string IMAGE_ASSET_TYPE = "image";
const std::string VIDEO_ASSET_TYPE = "video";

// data calculate args
const int32_t ASSET_IN_BUCKET_NUM_MAX = 1000;
const int32_t ASSET_DIR_START_NUM = 16;
const int32_t ASSET_MAX_COMPLEMENT_ID = 999;
const std::string DEFAULT_IMAGE_NAME = "IMG_";
const std::string DEFAULT_VIDEO_NAME = "VID_";
const std::string ROOT_MEDIA_DIR = "/storage/cloud/files/";

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
    Media::PhotoColumn::PHOTO_USER_COMMENT,
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

const std::vector<std::string> MEDIA_CLOUD_SYNC_COLUMNS = {
    Media::PhotoColumn::MEDIA_FILE_PATH,
    Media::PhotoColumn::MEDIA_TITLE,
    Media::PhotoColumn::MEDIA_SIZE,
    Media::PhotoColumn::MEDIA_NAME,
    Media::PhotoColumn::MEDIA_TYPE,
    Media::PhotoColumn::MEDIA_MIME_TYPE,
    Media::PhotoColumn::MEDIA_DEVICE_NAME,
    Media::PhotoColumn::MEDIA_DATE_ADDED,
    Media::PhotoColumn::MEDIA_DATE_MODIFIED,
    Media::PhotoColumn::MEDIA_DATE_TAKEN,
    Media::PhotoColumn::MEDIA_DURATION,
    Media::PhotoColumn::MEDIA_IS_FAV,
    Media::PhotoColumn::MEDIA_DATE_TRASHED,
    Media::PhotoColumn::MEDIA_HIDDEN,
    Media::PhotoColumn::MEDIA_RELATIVE_PATH,
    Media::PhotoColumn::MEDIA_VIRTURL_PATH,
    Media::PhotoColumn::PHOTO_META_DATE_MODIFIED,
    Media::PhotoColumn::PHOTO_ORIENTATION,
    Media::PhotoColumn::PHOTO_LATITUDE,
    Media::PhotoColumn::PHOTO_LONGITUDE,
    Media::PhotoColumn::PHOTO_HEIGHT,
    Media::PhotoColumn::PHOTO_WIDTH,
    Media::PhotoColumn::PHOTO_SUBTYPE,
    Media::PhotoColumn::PHOTO_DATE_YEAR,
    Media::PhotoColumn::PHOTO_DATE_MONTH,
    Media::PhotoColumn::PHOTO_DATE_DAY,
    Media::PhotoColumn::PHOTO_USER_COMMENT,
    Media::PhotoColumn::PHOTO_THUMB_STATUS,
    Media::PhotoColumn::PHOTO_SYNC_STATUS,
    Media::PhotoColumn::PHOTO_SHOOTING_MODE,
    Media::PhotoColumn::PHOTO_SHOOTING_MODE_TAG,

    /* keep cloud_id at the last, so RecordToValueBucket can skip it*/
    Media::MediaColumn::MEDIA_ID,
    Media::PhotoColumn::PHOTO_CLOUD_ID
};

const std::vector<std::string> CLOUD_SYNC_UNIQUE_COLUMNS = {
    Media::PhotoColumn::MEDIA_FILE_PATH,
    Media::PhotoColumn::MEDIA_TITLE,
    Media::PhotoColumn::MEDIA_TYPE,
    Media::PhotoColumn::MEDIA_DURATION,
    Media::PhotoColumn::MEDIA_HIDDEN,
    Media::PhotoColumn::MEDIA_RELATIVE_PATH,
    Media::PhotoColumn::MEDIA_VIRTURL_PATH,
    Media::PhotoColumn::PHOTO_META_DATE_MODIFIED,
    Media::PhotoColumn::PHOTO_SUBTYPE,
    Media::PhotoColumn::PHOTO_DATE_YEAR,
    Media::PhotoColumn::PHOTO_DATE_MONTH,
    Media::PhotoColumn::PHOTO_DATE_DAY,
    Media::PhotoColumn::PHOTO_SHOOTING_MODE,
    Media::PhotoColumn::PHOTO_SHOOTING_MODE_TAG,

    /* NR_LOCAL_INFO: keep local info in the end */
    Media::MediaColumn::MEDIA_ID,
    Media::PhotoColumn::PHOTO_CLOUD_ID
};

const std::vector<DataType> CLOUD_SYNC_UNIQUE_COLUMN_TYPES = {
    DataType::STRING,       /* data */
    DataType::STRING,       /* title */
    DataType::INT,          /* media_type */
    DataType::INT,          /* duration */
    DataType::INT,          /* hidden */
    DataType::STRING,       /* relative_path */
    DataType::STRING,       /* virtual_path */
    DataType::LONG,         /* meta_date_modified */
    DataType::INT,          /* subtype */
    DataType::STRING,       /* date_year */
    DataType::STRING,       /* date_month */
    DataType::STRING,       /* date_day */
    DataType::STRING,       /* shooting_mode */
    DataType::STRING,       /* shooting_mode_tag */
    DataType::INT,          /* file_id */
    DataType::STRING        /* cloud_id */
};

const std::vector<std::string> CLOUD_SYNC_COMMONS_COLUMNS = {
    Media::PhotoColumn::MEDIA_SIZE,
    Media::PhotoColumn::MEDIA_NAME,
    Media::PhotoColumn::MEDIA_TYPE,
    Media::PhotoColumn::MEDIA_MIME_TYPE,
    Media::PhotoColumn::MEDIA_DEVICE_NAME,
    Media::PhotoColumn::MEDIA_DATE_MODIFIED,
    Media::PhotoColumn::MEDIA_DATE_TAKEN,
    Media::PhotoColumn::MEDIA_DURATION,
    Media::PhotoColumn::MEDIA_IS_FAV,
    Media::PhotoColumn::MEDIA_DATE_TRASHED,
    Media::PhotoColumn::MEDIA_HIDDEN,
    Media::PhotoColumn::MEDIA_RELATIVE_PATH,
    Media::PhotoColumn::MEDIA_VIRTURL_PATH,
    Media::PhotoColumn::PHOTO_META_DATE_MODIFIED,
    Media::PhotoColumn::PHOTO_ORIENTATION,
    Media::PhotoColumn::PHOTO_LATITUDE,
    Media::PhotoColumn::PHOTO_LONGITUDE,
    Media::PhotoColumn::PHOTO_HEIGHT,
    Media::PhotoColumn::PHOTO_WIDTH,
    Media::PhotoColumn::PHOTO_SUBTYPE,
    /* keep cloud_id at the last, so RecordToValueBucket can skip it*/
    Media::PhotoColumn::PHOTO_CLOUD_ID
};

const std::vector<std::string> ON_UPLOAD_COLUMNS = {
    Media::PhotoColumn::MEDIA_FILE_PATH,
    Media::PhotoColumn::MEDIA_DATE_MODIFIED,
    Media::PhotoColumn::PHOTO_META_DATE_MODIFIED,
    Media::MediaColumn::MEDIA_ID,
    Media::PhotoColumn::PHOTO_CLOUD_ID,
    Media::PhotoColumn::PHOTO_DIRTY,
};
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
#endif // OHOS_CLOUD_SYNC_SERVICE_GALLERY_FILE_CONST_H
