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
#ifndef OHOS_FILEMGMT_DATA_SYNC_MANAGER_H
#define OHOS_FILEMGMT_DATA_SYNC_MANAGER_H

#include <set>
#include <string>

namespace OHOS::FileManagement::CloudSync {
class DataSyncConst {
public:
    // photo album table name
    static const std::string ALBUM_TABLE_NAME;
    
    // columns only in PhotoAlbumTable
    static const std::string ALBUM_ID;
    static const std::string ALBUM_TYPE;
    static const std::string ALBUM_SUBTYPE;
    static const std::string ALBUM_NAME;
    static const std::string ALBUM_COVER_URI;
    static const std::string ALBUM_COUNT;
    static const std::string ALBUM_DATE_MODIFIED;

    // For api9 compatibility
    static const std::string ALBUM_RELATIVE_PATH;

    // default fetch columns
    static const std::set<std::string> DEFAULT_FETCH_COLUMNS;

    // util constants
    static const std::string ALBUM_URI_PREFIX;
    static const std::string PHOTO_URI_PREFIX;

    static const std::string INVALID_ID;
};
}
#endif // OHOS_FILEMGMT_DATA_SYNC_MANAGER_H
