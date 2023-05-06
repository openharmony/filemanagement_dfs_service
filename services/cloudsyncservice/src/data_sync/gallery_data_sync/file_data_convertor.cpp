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

#include "medialibrary_db_const.h"

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
string FileDataConvertor::sandboxPrefix_ = "/storage/media/local/files";
string FileDataConvertor::prefixLCD_ = "/mnt/hmdfs/";
string FileDataConvertor::suffixLCD_ = "/account/device_view/local/files";

/* basic map */
unordered_map<string, int32_t (FileDataConvertor::*)(string &key, DriveKit::DKRecordData &data,
        NativeRdb::ResultSet &resultSet)> FileDataConvertor::map_ = {
    /* basic */
    { FILE_ALBUM_ID, &FileDataConvertor::HandleAlbumId },
    { FILE_FILE_NAME, &FileDataConvertor::HandleFileName },
    { FILE_HASH_ID, &FileDataConvertor::HandleHashId },
    { FILE_SOURCE, &FileDataConvertor::HandleSource },
    { FILE_FILETYPE, &FileDataConvertor::HandleFileType },
    { FILE_CREATED_TIME, &FileDataConvertor::HandleCreatedTime },
    { FILE_FAVORITE, &FileDataConvertor::HandleFavorite },
    { FILE_DESCRIPTION, &FileDataConvertor::HandleDescription },
    /* properties */
    { FILE_PROPERTIES, &FileDataConvertor::HandleProperties },
    /* attachments */
    { FILE_ATTACHMENTS, &FileDataConvertor::HandleAttachments },
};

/* properties map */
unordered_map<string, int32_t (FileDataConvertor::*)(string &key, DriveKit::DKRecordFieldMap &map,
        NativeRdb::ResultSet &resultSet)> FileDataConvertor::pMap_ = {
    { FILE_HEIGHT, &FileDataConvertor::HandleHeight },
    { FILE_ROTATION, &FileDataConvertor::HandleRotation },
    { FILE_WIDTH, &FileDataConvertor::HandleWidth },
    { FILE_POSITION, &FileDataConvertor::HandlePosition },
    { FILE_DATA_MODIFIED, &FileDataConvertor::HandleDataModified },
    { FILE_DETAIL_TIME, &FileDataConvertor::HandleDetailTime },
    { FILE_FILE_CREATE_TIME, &FileDataConvertor::HandleFileCreateTime },
    { FILE_FIRST_UPDATE_TIME, &FileDataConvertor::HandleFirstUpdateTime },
    { FILE_RELATIVE_BUCKET_ID, &FileDataConvertor::HandleRelativeBucketId },
    { FILE_SOURCE_FILE_NAME, &FileDataConvertor::HandleSourceFileName },
    { FILE_SOURCE_PATH, &FileDataConvertor::HandleSourcePath },
    { FILE_TIME_ZONE, &FileDataConvertor::HandleTimeZone },
    /* general */
    { FILE_GENERAL, &FileDataConvertor::HandleGeneral }
};

FileDataConvertor::FileDataConvertor(int32_t userId, string &bundleName, OperationType type) : userId_(userId),
    bundleName_(bundleName), type_(type)
{
}

int32_t FileDataConvertor::Convert(DriveKit::DKRecord &record, NativeRdb::ResultSet &resultSet)
{
    /* data */
    DriveKit::DKRecordData data;
    for (auto it = map_.begin(); it != map_.end(); it++) {
        string &key = const_cast<string &>(it->first);
        int32_t ret = (this->*(it->second))(key, data, resultSet);
        if (ret != E_OK) {
            LOGE("%{private}s convert err %{public}d", key.c_str(), ret);
            return ret;
        }
    }
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
int32_t FileDataConvertor::HandleProperties(string &key, DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    DriveKit::DKRecordFieldMap map;
    for (auto it = pMap_.begin(); it != pMap_.end(); it++) {
        string &key = const_cast<string &>(it->first);
        int32_t ret = (this->*(it->second))(key, map, resultSet);
        if (ret != E_OK) {
            LOGE("%{private}s convert err %{public}d", key.c_str(), ret);
            return ret;
        }
    }
    data[key] = DriveKit::DKRecordField(map);
    return E_OK;
}

/* attachments */
int32_t FileDataConvertor::HandleAttachments(std::string &key, DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    if (type_ != FILE_CREATE && type_ != FILE_DATA_MODIFY) {
        return E_OK;
    }

    /* path */
    string path;
    int32_t ret = GetString(MEDIA_DATA_DB_FILE_PATH, path, resultSet);
    if (ret != E_OK) {
        return ret;
    }

    DriveKit::DKRecordFieldList attachments;
    /* content */
    ret = HandleContent(attachments, path);
    if (ret != E_OK) {
        LOGE("handle thumbnail err %{public}d", ret);
        return ret;
    }
    /* thumb */
    ret = HandleThumbnail(attachments, path, resultSet);
    if (ret != E_OK) {
        LOGE("handle thumbnail err %{public}d", ret);
        return ret;
    }
    /* lcd */
    ret = HandleLcd(attachments, path, resultSet);
    if (ret != E_OK) {
        LOGE("handle lcd err %{public}d", ret);
        return ret;
    }

    data[key] = DriveKit::DKRecordField(attachments);
    return E_OK;
}

/* record id */
int32_t FileDataConvertor::FillRecordId(DriveKit::DKRecord &record,
    NativeRdb::ResultSet &resultSet)
{
    string val;
    int32_t ret = GetString(MEDIA_DATA_DB_CLOUD_ID, val, resultSet);
    if (ret != E_OK) {
        return ret;
    }
    record.SetRecordId(val);
    return E_OK;
}

/* basic */
int32_t FileDataConvertor::HandleAlbumId(string &key, DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    data[key] = DriveKit::DKRecordField("default-album-1");
    return E_OK;
}

int32_t FileDataConvertor::HandleFileName(string &key, DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    string val;
    int32_t ret = GetString(MEDIA_DATA_DB_NAME, val, resultSet);
    if (ret != E_OK) {
        return ret;
    }
    data[key] = DriveKit::DKRecordField(val);
    return E_OK;
}

int32_t FileDataConvertor::HandleHashId(string &key, DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    data[key] = DriveKit::DKRecordField("1");
    return E_OK;
}

int32_t FileDataConvertor::HandleSource(string &key, DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    data[key] = DriveKit::DKRecordField("");
    return E_OK;
}

int32_t FileDataConvertor::HandleFileType(string &key, DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    data[key] = DriveKit::DKRecordField(1);
    return E_OK;
}

int32_t FileDataConvertor::HandleCreatedTime(string &key, DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    int64_t val;
    int32_t ret = GetLong(MEDIA_DATA_DB_DATE_ADDED, val, resultSet);
    if (ret != E_OK) {
        return ret;
    }
    data[key] = DriveKit::DKRecordField(to_string(val));
    return E_OK;
}

int32_t FileDataConvertor::HandleFavorite(string &key, DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    int32_t val;
    int32_t ret = GetInt(MEDIA_DATA_DB_IS_FAV, val, resultSet);
    if (ret != E_OK) {
        return ret;
    }
    data[key] = DriveKit::DKRecordField(val);
    return E_OK;
}

int32_t FileDataConvertor::HandleDescription(string &key, DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    data[key] = DriveKit::DKRecordField("");
    return E_OK;
}

/* properties */
int32_t FileDataConvertor::HandleHeight(string &key, DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    int32_t val;
    int32_t ret = GetInt(MEDIA_DATA_DB_HEIGHT, val, resultSet);
    if (ret != E_OK) {
        return ret;
    }
    map[key] = DriveKit::DKRecordField(val);
    return E_OK;
}

int32_t FileDataConvertor::HandleRotation(string &key, DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    int32_t val;
    int32_t ret = GetInt(MEDIA_DATA_DB_ORIENTATION, val, resultSet);
    if (ret != E_OK) {
        return ret;
    }
    map[key] = DriveKit::DKRecordField(val);
    return E_OK;
}

int32_t FileDataConvertor::HandleWidth(string &key, DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    int32_t val;
    int32_t ret = GetInt(MEDIA_DATA_DB_WIDTH, val, resultSet);
    if (ret != E_OK) {
        return ret;
    }
    map[key] = DriveKit::DKRecordField(val);
    return E_OK;
}

int32_t FileDataConvertor::HandlePosition(string &key, DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    map[key] = DriveKit::DKRecordField("");
    return E_OK;
}

int32_t FileDataConvertor::HandleDataModified(string &key, DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    int64_t val;
    int32_t ret = GetLong(MEDIA_DATA_DB_WIDTH, val, resultSet);
    if (ret != E_OK) {
        return ret;
    }
    map[key] = DriveKit::DKRecordField(to_string(val));
    return E_OK;
}

int32_t FileDataConvertor::HandleDetailTime(string &key, DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    map[key] = DriveKit::DKRecordField("");
    return E_OK;
}

int32_t FileDataConvertor::HandleFileCreateTime(string &key, DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    int64_t val;
    int32_t ret = GetLong(MEDIA_DATA_DB_DATE_ADDED, val, resultSet);
    if (ret != E_OK) {
        return ret;
    }
    map[key] = DriveKit::DKRecordField(to_string(val));
    return E_OK;
}

int32_t FileDataConvertor::HandleFirstUpdateTime(string &key, DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    map[key] = DriveKit::DKRecordField("");
    return E_OK;
}

int32_t FileDataConvertor::HandleRelativeBucketId(string &key, DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    map[key] = DriveKit::DKRecordField("1");
    return E_OK;
}

int32_t FileDataConvertor::HandleSourceFileName(string &key, DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    string val;
    int32_t ret = GetString(MEDIA_DATA_DB_NAME, val, resultSet);
    if (ret != E_OK) {
        return ret;
    }
    map[key] = DriveKit::DKRecordField(val);
    return E_OK;
}

int32_t FileDataConvertor::HandleSourcePath(string &key, DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    string val;
    int32_t ret = GetString(MEDIA_DATA_DB_FILE_PATH, val, resultSet);
    if (ret != E_OK) {
        return ret;
    }
    map[key] = DriveKit::DKRecordField(val);
    return E_OK;
}

int32_t FileDataConvertor::HandleTimeZone(string &key, DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    map[key] = DriveKit::DKRecordField("");
    return E_OK;
}

/* properties - general */
int32_t FileDataConvertor::HandleGeneral(std::string &key, DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    int32_t size = GALLERY_FILE_COLUMNS.size();
    for (int32_t i = 0; i < size - 1; i++) {
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
int32_t FileDataConvertor::HandleContent(DriveKit::DKRecordFieldList &list,
    string &path)
{
    /* asset */
    DriveKit::DKAsset content;
    content.uri = GetLowerPath(path);
    content.assetName = FILE_CONTENT;
    content.operationType = DriveKit::DKAssetOperType::DK_ASSET_ADD;
    list.push_back(DriveKit::DKRecordField(content));
    return E_OK;
}

int32_t FileDataConvertor::HandleThumbnail(DriveKit::DKRecordFieldList &list,
    string &path, NativeRdb::ResultSet &resultSet)
{
    /* key */
    string key;
    int32_t ret = GetString(MEDIA_DATA_DB_THUMBNAIL, key, resultSet);
    if (ret != E_OK) {
        return ret;
    }

    /* asset */
    DriveKit::DKAsset content;
    content.uri = GetThumbPath(path, key);
    content.assetName = FILE_THUMBNAIL;
    content.operationType = DriveKit::DKAssetOperType::DK_ASSET_ADD;
    list.push_back(DriveKit::DKRecordField(content));
    return E_OK;
}

int32_t FileDataConvertor::HandleLcd(DriveKit::DKRecordFieldList &list,
    string &path, NativeRdb::ResultSet &resultSet)
{
    /* key */
    string key;
    int32_t ret = GetString(MEDIA_DATA_DB_LCD, key, resultSet);
    if (ret != E_OK) {
        return ret;
    }

    /* asset */
    DriveKit::DKAsset content;
    content.uri = GetThumbPath(path, key);
    content.assetName = FILE_LCD;
    content.operationType = DriveKit::DKAssetOperType::DK_ASSET_ADD;
    list.push_back(DriveKit::DKRecordField(content));
    return E_OK;
}

string FileDataConvertor::GetLowerPath(const std::string &path)
{
    size_t pos = path.find_first_of(sandboxPrefix_);
    if (pos == string::npos) {
        LOGE("invalid path %{private}s", path.c_str());
        return "";
    }
    return prefix_ + to_string(userId_) + suffix_ + path.substr(pos +
        sandboxPrefix_.size());
}


string FileDataConvertor::GetThumbPath(const std::string &path, const std::string &key)
{
    if (path.length() < ROOT_MEDIA_DIR.length()) {
        return "";
    }

    auto lastIndex = path.find_last_of('.');
    if (lastIndex == string::npos) {
        lastIndex = ROOT_MEDIA_DIR.length() - 1;
    }
    lastIndex = lastIndex - ROOT_MEDIA_DIR.length();

    return prefix_ + to_string(userId_) + suffix_ + "/.thumbs/" +
        path.substr(ROOT_MEDIA_DIR.length(), lastIndex) + "-" + key + ".jpg";
}

string FileDataConvertor::GetThumbPathDownloadLCD(const std::string &path, const std::string &key)
{
    if (path.length() < ROOT_MEDIA_DIR.length()) {
        return "";
    }

    auto lastIndex = path.find_last_of('.');
    if (lastIndex == string::npos) {
        lastIndex = ROOT_MEDIA_DIR.length() - 1;
    }
    lastIndex = lastIndex - ROOT_MEDIA_DIR.length();

    return prefixLCD_ + to_string(userId_) + suffixLCD_ + "/.thumbs/" +
        path.substr(ROOT_MEDIA_DIR.length(), lastIndex) + "-" + key + ".jpg";
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
