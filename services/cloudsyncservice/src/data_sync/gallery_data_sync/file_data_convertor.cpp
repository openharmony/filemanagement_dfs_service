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

#include "file_data_convertor.h"

#include <unistd.h>

#include "medialibrary_db_const.h"
#include "thumbnail_const.h"

#include "dfs_error.h"
#include "utils_log.h"
#include "sdk_helper.h"
#include "gallery_file_const.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
using namespace std;
using namespace NativeRdb;
using namespace Media;

/* record type */
string FileDataConvertor::recordType_ = "media";

/* path */
string FileDataConvertor::prefix_ = "/data/service/el2/";
string FileDataConvertor::suffix_ = "/hmdfs/account/files";
string FileDataConvertor::sandboxPrefix_ = "/storage/cloud/files";
string FileDataConvertor::prefixLCD_ = "/mnt/hmdfs/";
string FileDataConvertor::suffixLCD_ = "/account/device_view/local/files";

FileDataConvertor::FileDataConvertor(int32_t userId, string &bundleName, OperationType type) : userId_(userId),
    bundleName_(bundleName), type_(type)
{
}

int32_t FileDataConvertor::Convert(DriveKit::DKRecord &record, NativeRdb::ResultSet &resultSet)
{
    DriveKit::DKRecordData data;
    /* properties */
    RETURN_ON_ERR(HandleProperties(FILE_PROPERTIES, data, resultSet));
    /* basic */
    RETURN_ON_ERR(HandleAlbumId(FILE_ALBUM_ID, data, resultSet));
    RETURN_ON_ERR(HandleFileName(FILE_FILE_NAME, data, resultSet));
    RETURN_ON_ERR(HandleHashId(FILE_HASH_ID, data, resultSet));
    RETURN_ON_ERR(HandleSource(FILE_SOURCE, data, resultSet));
    RETURN_ON_ERR(HandleFileType(FILE_FILETYPE, data, resultSet));
    RETURN_ON_ERR(HandleCreatedTime(FILE_CREATED_TIME, data, resultSet));
    RETURN_ON_ERR(HandleFavorite(FILE_FAVORITE, data, resultSet));
    RETURN_ON_ERR(HandleDescription(FILE_DESCRIPTION, data, resultSet));
    RETURN_ON_ERR(HandleRecycle(FILE_RECYCLED, data, resultSet));
    /* attachments */
    RETURN_ON_ERR(HandleAttachments(FILE_ATTACHMENTS, data, resultSet));

    /* set data */
    record.SetRecordData(data);
    /* control info */
    record.SetRecordType(recordType_);
    if (type_ == FILE_CREATE) {
        record.SetNewCreate(true);
    } else {
        int32_t ret = FillRecordId(record, resultSet);
        if (ret != E_OK) {
            LOGE("fill record id err %{public}d", ret);
            return ret;
        }
    }
    return E_OK;
}

/* properties */
int32_t FileDataConvertor::HandleProperties(const string &key, DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    DriveKit::DKRecordFieldMap map;
    /* general */
    RETURN_ON_ERR(HandleGeneral(FILE_ALBUM_ID, map, resultSet));
    /* basic properties */
    RETURN_ON_ERR(HandleHeight(FILE_HEIGHT, map, resultSet));
    RETURN_ON_ERR(HandleRotation(FILE_ROTATION, map, resultSet));
    RETURN_ON_ERR(HandleWidth(FILE_WIDTH, map, resultSet));
    RETURN_ON_ERR(HandlePosition(FILE_POSITION, map, resultSet));
    RETURN_ON_ERR(HandleDataModified(FILE_DATA_MODIFIED, map, resultSet));
    RETURN_ON_ERR(HandleDetailTime(FILE_DETAIL_TIME, map, resultSet));
    RETURN_ON_ERR(HandleFileCreateTime(FILE_FILE_CREATE_TIME, map, resultSet));
    RETURN_ON_ERR(HandleFirstUpdateTime(FILE_FIRST_UPDATE_TIME, map, resultSet));
    RETURN_ON_ERR(HandleRelativeBucketId(FILE_RELATIVE_BUCKET_ID, map, resultSet));
    RETURN_ON_ERR(HandleSourceFileName(FILE_SOURCE_FILE_NAME, map, resultSet));
    RETURN_ON_ERR(HandleSourcePath(FILE_SOURCE_PATH, map, resultSet));
    RETURN_ON_ERR(HandleTimeZone(FILE_TIME_ZONE, map, resultSet));

    /* set map */
    data[key] = DriveKit::DKRecordField(map);
    return E_OK;
}

/* attachments */
int32_t FileDataConvertor::HandleAttachments(const string &key, DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    if (type_ != FILE_CREATE && type_ != FILE_DATA_MODIFY) {
        return E_OK;
    }

    /* path */
    string path;
    int32_t ret = GetString(PhotoColumn::MEDIA_FILE_PATH, path, resultSet);
    if (ret != E_OK) {
        return ret;
    }

    /* content */
    ret = HandleContent(data, path);
    if (ret != E_OK) {
        LOGE("handle thumbnail err %{public}d", ret);
        return ret;
    }
    /* thumb */
    ret = HandleThumbnail(data, path);
    if (ret != E_OK) {
        LOGE("handle thumbnail err %{public}d", ret);
        return ret;
    }
    /* lcd */
    ret = HandleLcd(data, path);
    if (ret != E_OK) {
        LOGE("handle lcd err %{public}d", ret);
        return ret;
    }

    return E_OK;
}

/* record id */
inline int32_t FileDataConvertor::FillRecordId(DriveKit::DKRecord &record,
    NativeRdb::ResultSet &resultSet)
{
    string val;
    int32_t ret = GetString(PhotoColumn::PHOTO_CLOUD_ID, val, resultSet);
    if (ret != E_OK) {
        return ret;
    }
    record.SetRecordId(val);
    return E_OK;
}

/* basic */
inline int32_t FileDataConvertor::HandleAlbumId(const string &key, DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    data[key] = DriveKit::DKRecordField("default-album-1");
    return E_OK;
}

inline int32_t FileDataConvertor::HandleFileName(const string &key, DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    string val;
    int32_t ret = GetString(PhotoColumn::MEDIA_NAME, val, resultSet);
    if (ret != E_OK) {
        return ret;
    }
    data[key] = DriveKit::DKRecordField(val);
    return E_OK;
}

inline int32_t FileDataConvertor::HandleHashId(const string &key, DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    data[key] = DriveKit::DKRecordField("1");
    return E_OK;
}

inline int32_t FileDataConvertor::HandleSource(const string &key, DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    data[key] = DriveKit::DKRecordField("");
    return E_OK;
}

inline int32_t FileDataConvertor::HandleFileType(const string &key, DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    DriveKit::DKRecordFieldMap properties;
    auto err = data[FILE_PROPERTIES].GetRecordMap(properties);
    if (err != DriveKit::DKLocalErrorCode::NO_ERROR) {
        LOGE("get properties err %{public}d", err);
        return E_DATA;
    }
    int32_t mediaType = properties[MediaColumn::MEDIA_TYPE];
    data[key] = DriveKit::DKRecordField(mediaType ==
        MEDIA_TYPE_VIDEO ? FILE_TYPE_VIDEO : FILE_TYPE_IMAGE);
    return E_OK;
}

inline int32_t FileDataConvertor::HandleCreatedTime(const string &key, DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    int64_t val;
    int32_t ret = GetLong(PhotoColumn::MEDIA_DATE_ADDED, val, resultSet);
    if (ret != E_OK) {
        return ret;
    }
    /* no overflow: 64 >> 32 + 3 */
    data[key] = DriveKit::DKRecordField(to_string(val * 1000));
    return E_OK;
}

inline int32_t FileDataConvertor::HandleFavorite(const string &key, DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    int32_t val;
    int32_t ret = GetInt(PhotoColumn::MEDIA_IS_FAV, val, resultSet);
    if (ret != E_OK) {
        return ret;
    }
    data[key] = DriveKit::DKRecordField(val);
    return E_OK;
}

inline int32_t FileDataConvertor::HandleDescription(const string &key, DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    data[key] = DriveKit::DKRecordField("");
    return E_OK;
}

inline int32_t FileDataConvertor::HandleRecycle(const string &key, DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    int64_t val;
    int32_t ret = GetLong(PhotoColumn::MEDIA_DATE_TRASHED, val, resultSet);
    if (ret != E_OK) {
        return ret;
    }
    data[key] = DriveKit::DKRecordField(!!val);
    return E_OK;
}

/* properties */
inline int32_t FileDataConvertor::HandleHeight(const string &key, DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    int32_t val;
    int32_t ret = GetInt(PhotoColumn::PHOTO_HEIGHT, val, resultSet);
    if (ret != E_OK) {
        return ret;
    }
    map[key] = DriveKit::DKRecordField(val);
    return E_OK;
}

inline int32_t FileDataConvertor::HandleRotation(const string &key, DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    int32_t val;
    int32_t ret = GetInt(PhotoColumn::PHOTO_ORIENTATION, val, resultSet);
    if (ret != E_OK) {
        return ret;
    }
    map[key] = DriveKit::DKRecordField(val);
    return E_OK;
}

inline int32_t FileDataConvertor::HandleWidth(const string &key, DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    int32_t val;
    int32_t ret = GetInt(PhotoColumn::PHOTO_WIDTH, val, resultSet);
    if (ret != E_OK) {
        return ret;
    }
    map[key] = DriveKit::DKRecordField(val);
    return E_OK;
}

inline int32_t FileDataConvertor::HandlePosition(const string &key, DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    map[key] = DriveKit::DKRecordField("");
    return E_OK;
}

inline int32_t FileDataConvertor::HandleDataModified(const string &key, DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    int64_t val;
    int32_t ret = GetLong(PhotoColumn::MEDIA_DATE_MODIFIED, val, resultSet);
    if (ret != E_OK) {
        return ret;
    }
    map[key] = DriveKit::DKRecordField(to_string(val));
    return E_OK;
}

inline int32_t FileDataConvertor::HandleDetailTime(const string &key, DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    map[key] = DriveKit::DKRecordField("");
    return E_OK;
}

inline int32_t FileDataConvertor::HandleFileCreateTime(const string &key, DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    int64_t val;
    int32_t ret = GetLong(PhotoColumn::MEDIA_DATE_ADDED, val, resultSet);
    if (ret != E_OK) {
        return ret;
    }
    map[key] = DriveKit::DKRecordField(to_string(val));
    return E_OK;
}

inline int32_t FileDataConvertor::HandleFirstUpdateTime(const string &key, DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    map[key] = DriveKit::DKRecordField("");
    return E_OK;
}

inline int32_t FileDataConvertor::HandleRelativeBucketId(const string &key, DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    map[key] = DriveKit::DKRecordField("1");
    return E_OK;
}

inline int32_t FileDataConvertor::HandleSourceFileName(const string &key, DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    string val;
    int32_t ret = GetString(PhotoColumn::MEDIA_NAME, val, resultSet);
    if (ret != E_OK) {
        return ret;
    }
    map[key] = DriveKit::DKRecordField(val);
    return E_OK;
}

inline int32_t FileDataConvertor::HandleSourcePath(const string &key, DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    string val;
    int32_t ret = GetString(PhotoColumn::MEDIA_FILE_PATH, val, resultSet);
    if (ret != E_OK) {
        return ret;
    }
    map[key] = DriveKit::DKRecordField(val);
    return E_OK;
}

inline int32_t FileDataConvertor::HandleTimeZone(const string &key, DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    map[key] = DriveKit::DKRecordField("");
    return E_OK;
}

/* properties - general */
int32_t FileDataConvertor::HandleGeneral(const string &key, DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    auto size = GALLERY_FILE_COLUMNS.size();
    for (decltype(size) i = 0; i < size - 1; i++) {
        const string &key = GALLERY_FILE_COLUMNS[i];
        DataType type = GALLERY_FILE_COLUMN_TYPES[i];
        switch (type) {
            case DataType::INT: {
                SET_RECORD_INT(key, resultSet, map);
                break;
            }
            case DataType::LONG: {
                SET_RECORD_LONG(key, resultSet, map);
                break;
            }
            case DataType::DOUBLE: {
                SET_RECORD_DOUBLE(key, resultSet, map);
                break;
            }
            case DataType::STRING: {
                SET_RECORD_STRING(key, resultSet, map);
                break;
            }
            case DataType::BOOL: {
                SET_RECORD_BOOL(key, resultSet, map);
                break;
            }
        }
    }
    return E_OK;
}

/* attachments */
int32_t FileDataConvertor::HandleContent(DriveKit::DKRecordData &data,
    string &path)
{
    string lowerPath = GetLowerPath(path);
    if (access(lowerPath.c_str(), F_OK)) {
        LOGE("content %{private}s doesn't exist", lowerPath.c_str());
        return E_PATH;
    }

    /* asset */
    DriveKit::DKAsset content;
    content.uri = move(lowerPath);
    content.assetName = FILE_CONTENT;
    content.operationType = DriveKit::DKAssetOperType::DK_ASSET_ADD;
    data[FILE_CONTENT] = DriveKit::DKRecordField(content);
    return E_OK;
}

int32_t FileDataConvertor::HandleThumbnail(DriveKit::DKRecordData &data,
    string &path)
{
    string thumbnailPath = GetThumbPath(path, THUMB_SUFFIX);
    if (access(thumbnailPath.c_str(), F_OK)) {
        LOGE("thumbnail %{private}s doesn't exist", thumbnailPath.c_str());
        return E_PATH;
    }

    /* asset */
    DriveKit::DKAsset content;
    content.uri = move(thumbnailPath);
    content.assetName = FILE_THUMBNAIL;
    content.operationType = DriveKit::DKAssetOperType::DK_ASSET_ADD;
    data[FILE_THUMBNAIL] = DriveKit::DKRecordField(content);
    return E_OK;
}

int32_t FileDataConvertor::HandleLcd(DriveKit::DKRecordData &data,
    string &path)
{
    string lcdPath = GetThumbPath(path, LCD_SUFFIX);
    if (access(lcdPath.c_str(), F_OK)) {
        LOGE("lcd %{private}s doesn't exist", lcdPath.c_str());
        return E_PATH;
    }

    /* asset */
    DriveKit::DKAsset content;
    content.uri = move(lcdPath);
    content.assetName = FILE_LCD;
    content.operationType = DriveKit::DKAssetOperType::DK_ASSET_ADD;
    data[FILE_LCD] = DriveKit::DKRecordField(content);
    return E_OK;
}

string FileDataConvertor::GetLowerPath(const string &path)
{
    size_t pos = path.find_first_of(sandboxPrefix_);
    if (pos == string::npos) {
        LOGE("invalid path %{private}s", path.c_str());
        return "";
    }
    return prefix_ + to_string(userId_) + suffix_ + path.substr(pos +
        sandboxPrefix_.size());
}

string FileDataConvertor::GetLowerTmpPath(const string &path)
{
    size_t pos = path.find_first_of(sandboxPrefix_);
    if (pos == string::npos) {
        LOGE("invalid path %{private}s", path.c_str());
        return "";
    }
    return prefix_ + to_string(userId_) + suffix_ + path.substr(pos +
        sandboxPrefix_.size());
}

string FileDataConvertor::GetSandboxPath(const string &path)
{
    string localPrefix =  prefix_ + to_string(userId_) + suffix_;
    size_t pos = path.find_first_of(localPrefix);
    if (pos == string::npos) {
        LOGE("invalid path %{private}s", path.c_str());
        return "";
    }
    return sandboxPrefix_ + path.substr(pos + localPrefix.size());
}

string FileDataConvertor::GetThumbPath(const string &path, const string &key)
{
    if (path.length() < ROOT_MEDIA_DIR.length()) {
        return "";
    }
    /* transform sandbox path */
    return prefixLCD_ + to_string(userId_) + suffixLCD_ + "/" + Media::GetThumbnailPath(path,
        key).substr(ROOT_MEDIA_DIR.length());
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
