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
/* attachments */
const std::string FILE_ATTACHMENTS = "attachments";
const std::string FILE_CONTENT = "content";
const std::string FILE_THUMBNAIL = "thumbnail";
const std::string FILE_LCD = "lcdThumbnail";
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
#endif // OHOS_CLOUD_SYNC_SERVICE_GALLERY_FILE_CONST_H
