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

#ifndef OHOS_CLOUD_SYNC_SERVICE_GALLERY_ALBUM_CONST_H
#define OHOS_CLOUD_SYNC_SERVICE_GALLERY_ALBUM_CONST_H

#include <string>

#include "medialibrary_db_const.h"
#include "medialibrary_type_const.h"
#include "photo_album_column.h"
#include "photo_map_column.h"

#include "data_convertor.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {

/* basic */
const std::string ALBUM_NAME = "albumName";
const std::string ALBUM_ID = "albumId";
const std::string ALBUM_TYPE = "type";
const std::string ALBUM_LOGIC_TYPE = "logicType";
const std::string ALBUM_PATH = "localPath";
/* properties */
const std::string ALBUM_PROPERTIES = "properties";

enum AlbumType : int32_t {
    NORMAL,
    SHARE,
};

enum LogicType : int32_t {
    PHYSICAL,
    LOGICAL,
};

const std::string ALBUM_LOCAL_PATH_PREFIX = "/OH/";

const std::vector<std::string> ALBUM_LOCAL_QUERY_COLUMNS = {
    Media::PhotoAlbumColumns::ALBUM_ID,
    Media::PhotoAlbumColumns::ALBUM_NAME,
    Media::PhotoAlbumColumns::ALBUM_DIRTY,
    Media::PhotoAlbumColumns::ALBUM_CLOUD_ID
};

const std::vector<std::string> GALLERY_ALBUM_COLUMNS = {
    Media::PhotoAlbumColumns::ALBUM_TYPE,
    Media::PhotoAlbumColumns::ALBUM_SUBTYPE,
    Media::PhotoAlbumColumns::ALBUM_NAME,
    Media::PhotoAlbumColumns::ALBUM_DATE_MODIFIED,
    /* put cloud id in the last, and skip it in the loop (size - 1) */
    Media::PhotoAlbumColumns::ALBUM_CLOUD_ID
};

const std::vector<DataType> GALLERY_ALBUM_COLUMN_TYPES = {
    DataType::INT,        /* album_type */
    DataType::INT,        /* album_subtype */
    DataType::STRING,        /* album_name */
    DataType::LONG,        /* date_modified */
    DataType::STRING        /* cloud_id */
};
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
#endif // OHOS_CLOUD_SYNC_SERVICE_GALLERY_ALBUM_CONST_H
