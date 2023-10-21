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

#ifndef OHOS_CLOUD_DISK_SERVICE_FILE_COLUMN_H
#define OHOS_CLOUD_DISK_SERVICE_FILE_COLUMN_H

#include <set>
#include <string>

namespace OHOS {
namespace FileManagement {
namespace CloudDisk {
class FileColumn {
public:
    // Local And Cloud
    static const std::string CLOUD_ID;
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

    // Only Local
    static const std::string FILE_ID;
    static const std::string PARENT_ID;
    static const std::string FILE_TIME_VISIT;
    static const std::string SYNC_STATUS;
    static const std::string POSITION;
    static const std::string DIRTY_TYPE;
    static const std::string SRC_PATH;
    static const std::string LCD_PATH;
    static const std::string THUMBNAIL_PATH;
    static const std::string LBR_VIDEO_PATH;
    static const std::string MIME_TYPE;
    static const std::string IS_DIRECTORY;
    static const std::string FILE_TYPE;
    static const std::string FILE_CATEGORY;

    // table name
    static const std::string FILES_TABLE;

    // create FileTable sql
    static const std::string CREATE_FILE_TABLE;

    // All Columns
    static const std::set<std::string> FILE_COLUMNS;

    static const std::set<std::string> DEFAULT_FETCH_COLUMNS;

    static bool IsFileColumn(const std::string &columnName);
};
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS
#endif // OHOS_CLOUD_DISK_SERVICE_FILE_COLUMN_H