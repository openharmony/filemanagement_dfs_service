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
#include <sys/stat.h>

#include "medialibrary_db_const.h"
#include "thumbnail_const.h"

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
string FileDataConvertor::prefixCloud_ = "/storage/cloud/";
string FileDataConvertor::suffixCloud_ = "/files";

FileDataConvertor::FileDataConvertor(int32_t userId, string &bundleName, OperationType type) : userId_(userId),
    bundleName_(bundleName), type_(type)
{
}

int32_t FileDataConvertor::Convert(DriveKit::DKRecord &record, NativeRdb::ResultSet &resultSet)
{
    DriveKit::DKRecordData data;
    /* properties */
    RETURN_ON_ERR(HandleProperties(data, resultSet));
    /* basic */
    RETURN_ON_ERR(HandleFileName(data, resultSet));
    RETURN_ON_ERR(HandleHashId(data, resultSet));
    RETURN_ON_ERR(HandleSource(data, resultSet));
    RETURN_ON_ERR(HandleFileType(data, resultSet));
    RETURN_ON_ERR(HandleCreatedTime(data, resultSet));
    RETURN_ON_ERR(HandleFavorite(data, resultSet));
    RETURN_ON_ERR(HandleDescription(data, resultSet));
    RETURN_ON_ERR(HandleRecycle(data, resultSet));
    /* attachments */
    RETURN_ON_ERR(HandleAttachments(data, resultSet));

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
int32_t FileDataConvertor::HandleProperties(DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    DriveKit::DKRecordFieldMap map;
    /* general */
    RETURN_ON_ERR(HandleGeneral(map, resultSet));
    /* basic properties */
    RETURN_ON_ERR(HandleHeight(map, resultSet));
    RETURN_ON_ERR(HandleRotation(map, resultSet));
    RETURN_ON_ERR(HandleWidth(map, resultSet));
    RETURN_ON_ERR(HandlePosition(map, resultSet));
    RETURN_ON_ERR(HandleDataModified(map, resultSet));
    RETURN_ON_ERR(HandleDetailTime(map, resultSet));
    RETURN_ON_ERR(HandleFileCreateTime(map, resultSet));
    RETURN_ON_ERR(HandleFirstUpdateTime(map, resultSet));
    RETURN_ON_ERR(HandleRelativeBucketId(map, resultSet));
    RETURN_ON_ERR(HandleSourceFileName(map, resultSet));
    RETURN_ON_ERR(HandleSourcePath(map, resultSet));
    RETURN_ON_ERR(HandleTimeZone(map, resultSet));
    RETURN_ON_ERR(HandleThumbSize(map, resultSet));
    RETURN_ON_ERR(HandleLcdSize(map, resultSet));

    /* set map */
    data[FILE_PROPERTIES] = DriveKit::DKRecordField(map);
    return E_OK;
}

/* attachments */
int32_t FileDataConvertor::HandleAttachments(DriveKit::DKRecordData &data,
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
int32_t FileDataConvertor::FillRecordId(DriveKit::DKRecord &record,
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

int32_t FileDataConvertor::HandleThumbSize(DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    if (type_ != FILE_CREATE && type_ != FILE_DATA_MODIFY) {
        return E_OK;
    }

    string path;
    int32_t ret = GetString(PhotoColumn::MEDIA_FILE_PATH, path, resultSet);
    if (ret != E_OK) {
        LOGE("get filepath failed");
        return ret;
    }

    string thumbnailPath = GetThumbPath(path, THUMB_SUFFIX);
    struct stat fileStat;
    int err = stat(thumbnailPath.c_str(), &fileStat);
    if (err < 0) {
        LOGE("get thumb size failed errno :%{public}d", errno);
        return E_INVAL_ARG;
    }

    map[FILE_THUMB_SIZE] = DriveKit::DKRecordField(int64_t(fileStat.st_size));
    return E_OK;
}

int32_t FileDataConvertor::HandleLcdSize(DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    if (type_ != FILE_CREATE && type_ != FILE_DATA_MODIFY) {
        return E_OK;
    }

    string path;
    int32_t ret = GetString(PhotoColumn::MEDIA_FILE_PATH, path, resultSet);
    if (ret != E_OK) {
        LOGE("get filepath failed");
        return ret;
    }

    string lcdPath = GetThumbPath(path, LCD_SUFFIX);
    struct stat fileStat;
    int err = stat(lcdPath.c_str(), &fileStat);
    if (err < 0) {
        LOGE("get lcd size failed errno :%{public}d", errno);
        return E_INVAL_ARG;
    }

    map[FILE_LCD_SIZE] = DriveKit::DKRecordField(int64_t(fileStat.st_size));
    return E_OK;
}

/* properties - general */
int32_t FileDataConvertor::HandleGeneral(DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    auto size = GALLERY_FILE_COLUMNS.size();
    for (decltype(size) i = 0; i < size; i++) {
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

string FileDataConvertor::GetHmdfsLocalPath(const string &path)
{
    size_t pos = path.find_first_of(sandboxPrefix_);
    if (pos == string::npos) {
        LOGE("invalid path %{private}s", path.c_str());
        return "";
    }
    /* transform sandbox path to hmdfs local path*/
    return prefixLCD_ + to_string(userId_) + suffixLCD_ + path.substr(pos + sandboxPrefix_.size());
}

string FileDataConvertor::GetThumbPathInCloud(const std::string &path, const std::string &key)
{
    if (path.length() < ROOT_MEDIA_DIR.length()) {
        return "";
    }
    return sandboxPrefix_ + "/" + Media::GetThumbnailPath(path, key).substr(ROOT_MEDIA_DIR.length());
}

int32_t FileDataConvertor::Convert(const DriveKit::DKRecord &record, NativeRdb::ValuesBucket &valueBucket)
{
    DriveKit::DKRecordData data;
    record.GetRecordData(data);

    if (data.find(FILE_PROPERTIES) == data.end()) {
        LOGE("record data donnot have properties set");
        return E_INVAL_ARG;
    }
    DriveKit::DKRecordFieldMap properties = data[FILE_PROPERTIES];

    auto size = GALLERY_FILE_COLUMNS.size();
    for (decltype(size) i = 0; i < size - NR_LOCAL_INFO; i++) {
        auto field = GALLERY_FILE_COLUMNS[i];
        auto type = GALLERY_FILE_COLUMN_TYPES[i];
        if (properties.find(field) == properties.end()) {
            LOGE("filed %s not found in record.properties", field.c_str());
            continue;
        }
        if (HandleField(properties[field], valueBucket, field, type) != E_OK) {
            LOGE("HandleField %s failed", field.c_str());
        }
    }
    return E_OK;
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
