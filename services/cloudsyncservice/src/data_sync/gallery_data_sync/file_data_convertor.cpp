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

#include <regex>
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
using DriveKit::DKLocalErrorCode;

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
string FileDataConvertor::tmpSuffix_ = ".temp.download";

constexpr size_t DEFAULT_TIME_SIZE = 32;
constexpr size_t FORMATTED_YEAR_SIZE = 4;
FileDataConvertor::FileDataConvertor(int32_t userId, string &bundleName, OperationType type,
    const function<void(int32_t, NativeRdb::ResultSet &resultSet)> &func) : userId_(userId),
    bundleName_(bundleName), type_(type), errHandler_(func)
{
}

int32_t FileDataConvertor::Convert(DriveKit::DKRecord &record, NativeRdb::ResultSet &resultSet)
{
    DriveKit::DKRecordData data;

    /* process cloud sync unique fileds*/
    RETURN_ON_ERR(HandleUniqueFileds(data, resultSet));

    /* process compatible fileds shared with gallery or gallery-specific fileds*/
    RETURN_ON_ERR(HandleCompatibleFileds(data, resultSet));
    record.SetRecordData(data);
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

int32_t FileDataConvertor::ConvertAsset(DriveKit::DKRecord &record, NativeRdb::ResultSet &resultSet)
{
    DriveKit::DKRecordData data;
    int32_t ret = FillRecordId(record, resultSet);
    if (ret != E_OK) {
        LOGE("fill record id err %{public}d", ret);
        return ret;
    }
    string path;
    DriveKit::DKRecordFieldMap map;
    DataConvertor::GetString(Media::PhotoColumn::MEDIA_FILE_PATH, path, resultSet);
    map[Media::PhotoColumn::MEDIA_FILE_PATH] = DriveKit::DKRecordField(path);
    data[FILE_ATTRIBUTES] = DriveKit::DKRecordField(map);
    record.SetRecordData(data);
    record.SetRecordType(recordType_);
    return E_OK;
}

void FileDataConvertor::HandleErr(int32_t err, NativeRdb::ResultSet &resultSet)
{
    if (errHandler_ != nullptr) {
        errHandler_(err, resultSet);
    }
}

int32_t FileDataConvertor::HandleUniqueFileds(DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    DriveKit::DKRecordFieldMap map;
    /* store media unique fileds in attributes*/
    RETURN_ON_ERR(HandleAttributes(map, resultSet));
    data[FILE_ATTRIBUTES] = DriveKit::DKRecordField(map);
    /* local info */
    RETURN_ON_ERR(HandleLocalInfo(data, resultSet));
    return E_OK;
}

/* Cloud sync unique filed processing */
int32_t FileDataConvertor::HandleAttributes(DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    auto size = CLOUD_SYNC_UNIQUE_COLUMNS.size();
    for (decltype(size) i = 0; i < size; i++) {
        const string &key = CLOUD_SYNC_UNIQUE_COLUMNS[i];
        DataType type = CLOUD_SYNC_UNIQUE_COLUMN_TYPES[i];
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
    RETURN_ON_ERR(HandleDataAdded(map, resultSet));
    RETURN_ON_ERR(HandleDataModified(map, resultSet));
    RETURN_ON_ERR(HandleThumbSize(map, resultSet));
    RETURN_ON_ERR(HandleLcdSize(map, resultSet));
    RETURN_ON_ERR(HandleFormattedDate(map, resultSet));
    return E_OK;
}

int32_t FileDataConvertor::HandleCompatibleFileds(DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    /* gallery-specific or shared fileds */
    RETURN_ON_ERR(HandleFileName(data, resultSet));
    RETURN_ON_ERR(HandleCreatedTime(data, resultSet));
    RETURN_ON_ERR(HandleHash(data, resultSet));
    RETURN_ON_ERR(HandleSize(data, resultSet));
    RETURN_ON_ERR(HandleSource(data, resultSet));
    RETURN_ON_ERR(HandleFileType(data, resultSet));
    RETURN_ON_ERR(HandleRecycled(data, resultSet));
    RETURN_ON_ERR(HandleRecycleTime(data, resultSet));
    RETURN_ON_ERR(HandleFavorite(data, resultSet));
    RETURN_ON_ERR(HandleDescription(data, resultSet));

    /* gallery expand fields */
    RETURN_ON_ERR(HandleProperties(data, resultSet));

    /* cloud sdk extra feature*/
    RETURN_ON_ERR(HandleAttachments(data, resultSet));

    /* cloudsync-specific fields */
    RETURN_ON_ERR(HandleMimeType(data, resultSet));
    RETURN_ON_ERR(HandleEditedTime(data, resultSet));
    return E_OK;
}

/* properties */
int32_t FileDataConvertor::HandleProperties(DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    DriveKit::DKRecordFieldMap map;

    /* gallery expand properties */
    RETURN_ON_ERR(HandleSourceFileName(map, resultSet));
    RETURN_ON_ERR(HandleFirstUpdateTime(map, resultSet));
    RETURN_ON_ERR(HandleFileCreateTime(map, resultSet));
    RETURN_ON_ERR(HandleDetailTime(map, resultSet));
    RETURN_ON_ERR(HandleSourcePath(map, resultSet));
    RETURN_ON_ERR(HandleRelativeBucketId(map, resultSet));
    RETURN_ON_ERR(HandlePosition(map, resultSet));
    RETURN_ON_ERR(HandleRotate(map, resultSet));
    RETURN_ON_ERR(HandleDuration(map, resultSet));

    /* Resolution is combined by cloud sdk, just upload height and width */
    RETURN_ON_ERR(HandleHeight(map, resultSet));
    RETURN_ON_ERR(HandleWidth(map, resultSet));
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

int32_t FileDataConvertor::HandleHeight(DriveKit::DKRecordFieldMap &map, NativeRdb::ResultSet &resultSet)
{
    int32_t val = 0;
    int32_t ret = GetInt(Media::PhotoColumn::PHOTO_HEIGHT, val, resultSet);
    if (ret != E_OK) {
        LOGE("Get local height err %{public}d", ret);
        return ret;
    }
    if (val == 0) {
        LOGW("Get local height is 0 ");
        return ret;
    }
    map[FILE_HEIGHT] = DriveKit::DKRecordField(val);
    return E_OK;
}

int32_t FileDataConvertor::HandleWidth(DriveKit::DKRecordFieldMap &map, NativeRdb::ResultSet &resultSet)
{
    int32_t val = 0;
    int32_t ret = GetInt(Media::PhotoColumn::PHOTO_WIDTH, val, resultSet);
    if (ret != E_OK) {
        LOGE("Get local wdith err %{public}d", ret);
        return ret;
    }
    if (val == 0) {
        LOGW("Get local wdith is 0 ");
        return ret;
    }
    map[FILE_WIDTH] = DriveKit::DKRecordField(val);
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
        LOGE("get thumb size failed errno :%{public}d, path :%{public}s", errno, thumbnailPath.c_str());
        return E_PATH;
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
        LOGE("get lcd size failed errno :%{public}d, path :%{public}s", errno, lcdPath.c_str());
        return E_PATH;
    }

    map[FILE_LCD_SIZE] = DriveKit::DKRecordField(int64_t(fileStat.st_size));
    return E_OK;
}

string FileDataConvertor::StrCreateTime(const string &format, int64_t time)
{
    char strTime[DEFAULT_TIME_SIZE] = "";
    auto tm = localtime(&time);
    if (tm != nullptr) {
        (void)strftime(strTime, sizeof(strTime), format.c_str(), tm);
        return strTime;
    } else {
        std::cout << "Failed to convert time." << std::endl;
        return "";
    }
}

int32_t FileDataConvertor::HandleFormattedDate(DriveKit::DKRecordFieldMap &map, NativeRdb::ResultSet &resultSet)
{
    string year;
    string month;
    string day;
    RETURN_ON_ERR(GetString(PhotoColumn::PHOTO_DATE_YEAR, year, resultSet));
    RETURN_ON_ERR(GetString(PhotoColumn::PHOTO_DATE_MONTH, month, resultSet));
    RETURN_ON_ERR(GetString(PhotoColumn::PHOTO_DATE_DAY, day, resultSet));

    if (year.empty() || month.empty() || day.empty()) {
        int64_t createTime = 0;
        RETURN_ON_ERR(GetLong(PhotoColumn::MEDIA_DATE_ADDED, createTime, resultSet));
        createTime = createTime / MILLISECOND_TO_SECOND;
        year = StrCreateTime(PhotoColumn::PHOTO_DATE_YEAR_FORMAT, createTime);
        month = StrCreateTime(PhotoColumn::PHOTO_DATE_MONTH_FORMAT, createTime);
        day = StrCreateTime(PhotoColumn::PHOTO_DATE_DAY_FORMAT, createTime);
    }

    map[PhotoColumn::PHOTO_DATE_YEAR] = DriveKit::DKRecordField(year);
    map[PhotoColumn::PHOTO_DATE_MONTH] = DriveKit::DKRecordField(month);
    map[PhotoColumn::PHOTO_DATE_DAY] = DriveKit::DKRecordField(day);
    return E_OK;
}

int32_t FileDataConvertor::HandleDetailTime(DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    int64_t val;
    int32_t ret = GetLong(Media::PhotoColumn::MEDIA_DATE_ADDED, val, resultSet);
    if (ret != E_OK) {
        return ret;
    }
    time_t dataAddedStamp = time_t(val / MILLISECOND_TO_SECOND);
    struct tm timeinfo;
    char buffer[80];
    localtime_r(&dataAddedStamp, &timeinfo);
    size_t size = strftime(buffer, sizeof(buffer), "%Y:%m:%d %H:%M:%S", &timeinfo);
    if (size == 0) {
        return E_OK;
    }
    std::string detailTime(buffer);
    map[FILE_DETAIL_TIME] = DriveKit::DKRecordField(detailTime);
    return E_OK;
}

int32_t FileDataConvertor::HandlePosition(DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    double latitudeVal;
    double longitudeVal;
    int32_t ret = GetDouble(Media::PhotoColumn::PHOTO_LATITUDE, latitudeVal, resultSet);
    if (ret != E_OK) {
        return ret;
    }
    ret = GetDouble(Media::PhotoColumn::PHOTO_LONGITUDE, longitudeVal, resultSet);
    if (ret != E_OK) {
        return ret;
    }
    std::stringstream latitudestream;
    std::stringstream longitudestream;
    latitudestream.precision(15); // 15:precision
    longitudestream.precision(15); // 15:precision
    latitudestream << latitudeVal;
    longitudestream << longitudeVal;
    std::string position = "{\"x\":\"" + latitudestream.str() + "\",\"y\":\"" + longitudestream.str() +"\"}";
    map[FILE_POSITION] = DriveKit::DKRecordField(position);
    return E_OK;
}

int32_t FileDataConvertor::HandleRotate(DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    int32_t val;
    int32_t ret = GetInt(Media::PhotoColumn::PHOTO_ORIENTATION, val, resultSet);
    if (ret != E_OK) {
        return ret;
    }
    switch (val) {
        case ROTATE_ANGLE_0:
            val = ORIENTATION_NORMAL;
            break;
        case ROTATE_ANGLE_90:
            val = ORIENTATION_ROTATE_90;
            break;
        case ROTATE_ANGLE_180:
            val = ORIENTATION_ROTATE_180;
            break;
        case ROTATE_ANGLE_270:
            val = ORIENTATION_ROTATE_270;
            break;
        default:
            val = ORIENTATION_NORMAL;
            break;
    }
    map[FILE_ROTATION] = DriveKit::DKRecordField(val);
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
    struct stat fileStat;
    int err = stat(lowerPath.c_str(), &fileStat);
    if (err < 0) {
        LOGE("get content size failed errno :%{public}d", errno);
        return E_PATH;
    }

    if (fileStat.st_size <= 0) {
        LOGE("content size err");
        return E_INVAL_ARG;
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
    string lowerPath = GetLowerPath(path);
    if (lowerPath == "") {
        return "";
    }
    return lowerPath + tmpSuffix_;
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

string FileDataConvertor::GetCloudPath(const std::string &path)
{
    return prefixCloud_ + to_string(userId_) + "/" + path.substr(prefixCloud_.size());
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

string FileDataConvertor::GetThumbParentPath(const string &path)
{
    size_t pos = path.find_first_of(sandboxPrefix_);
    if (pos == string::npos) {
        LOGE("invalid path %{private}s", path.c_str());
        return "";
    }
    /* transform sandbox path to hmdfs local path*/
    return "/storage/cloud/" + to_string(userId_) + "/files/.thumbs" + path.substr(pos + sandboxPrefix_.size());
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

string FileDataConvertor::GetLocalPathToCloud(const std::string &path)
{
    string localPrefix = prefixLCD_ + to_string(userId_) + suffixLCD_;
    size_t pos = path.find_first_of(localPrefix);
    if (pos == string::npos || pos != 0) {
        LOGE("invalid path %{private}s", path.c_str());
        return "";
    }

    return sandboxPrefix_ + path.substr(localPrefix.length());
}

bool FileDataConvertor::IfContainsAttributes(const DriveKit::DKRecord &record)
{
    DriveKit::DKRecordData data;
    record.GetRecordData(data);
    if (data.find(FILE_ATTRIBUTES) != data.end()) {
        DriveKit::DKRecordFieldMap attributes;
        data[FILE_ATTRIBUTES].GetRecordMap(attributes);
        return !(attributes.find(PhotoColumn::MEDIA_TITLE) == attributes.end());
    }
    return false;
}

int32_t FileDataConvertor::Convert(DriveKit::DKRecord &record, NativeRdb::ValuesBucket &valueBucket)
{
    DriveKit::DKRecordData data;
    record.GetRecordData(data);
    if (!IfContainsAttributes(record)) {
        int32_t ret = TryCompensateValue(record, data, valueBucket);
        if (ret != E_OK) {
            LOGE("record data lose key value");
            return ret;
        }
    } else {
        int32_t ret = ExtractAttributeValue(data, valueBucket);
        if (ret != E_OK) {
            LOGE("record data donnot have attributes set, need");
            return ret;
        }
        RETURN_ON_ERR(CompensateTitle(data, valueBucket));
    }
    RETURN_ON_ERR(ExtractCompatibleValue(record, data, valueBucket));
    return E_OK;
}

int32_t FileDataConvertor::TryCompensateValue(const DriveKit::DKRecord &record,
    DriveKit::DKRecordData &data, NativeRdb::ValuesBucket &valueBucket)
{
    RETURN_ON_ERR(CompensateData(data, valueBucket));
    RETURN_ON_ERR(CompensateTitle(data, valueBucket));
    RETURN_ON_ERR(CompensateMediaType(data, valueBucket));
    RETURN_ON_ERR(CompensateMetaDateModified(record, valueBucket));
    RETURN_ON_ERR(CompensateSubtype(data, valueBucket));
    RETURN_ON_ERR(CompensateDuration(data, valueBucket));
    return E_OK;
}

int32_t FileDataConvertor::CompensateData(DriveKit::DKRecordData &data, NativeRdb::ValuesBucket &valueBucket)
{
    if (data.find(FILE_ATTRIBUTES) == data.end()) {
        LOGE("record data cannot find attributes or size");
        return E_INVAL_ARG;
    }
    DriveKit::DKRecordFieldMap attributes;
    data[FILE_ATTRIBUTES].GetRecordMap(attributes);
    string dataPath;
    if (attributes[PhotoColumn::MEDIA_FILE_PATH].GetString(dataPath) != DKLocalErrorCode::NO_ERROR) {
        LOGE("bad file_path in attributes");
        return E_INVAL_ARG;
    }
    valueBucket.PutString(PhotoColumn::MEDIA_FILE_PATH, dataPath);
    return E_OK;
}

int32_t FileDataConvertor::CompensateTitle(DriveKit::DKRecordData &data, NativeRdb::ValuesBucket &valueBucket)
{
    if (data.find(FILE_PROPERTIES) == data.end()) {
        LOGW("record data cannot find properties");
        return E_OK;
    }
    DriveKit::DKRecordFieldMap prop;
    data[FILE_PROPERTIES].GetRecordMap(prop);
    if (prop.find(FILE_SOURCE_FILE_NAME) == prop.end()) {
        LOGE("record data cannot find title, ignore it");
        return E_OK;
    }
    string sourceFileName;
    if (prop[FILE_SOURCE_FILE_NAME].GetString(sourceFileName) != DKLocalErrorCode::NO_ERROR) {
        LOGE("bad sourceFileName in props");
        return E_INVAL_ARG;
    }
    string title = sourceFileName;
    size_t pos = sourceFileName.find_last_of(".");
    if (pos != string::npos) {
        title = sourceFileName.substr(0, pos);
    }
    if (valueBucket.HasColumn(PhotoColumn::MEDIA_TITLE)) {
        valueBucket.Delete(PhotoColumn::MEDIA_TITLE);
    }
    valueBucket.PutString(PhotoColumn::MEDIA_TITLE, title);
    return E_OK;
}

int32_t FileDataConvertor::CompensateMediaType(DriveKit::DKRecordData &data,
    NativeRdb::ValuesBucket &valueBucket)
{
    if (data.find(FILE_FILETYPE) == data.end()) {
        LOGE("record data cannot find properties");
        return E_INVAL_ARG;
    }
    int32_t fileType;
    if (data[FILE_FILETYPE].GetInt(fileType) != DKLocalErrorCode::NO_ERROR) {
        LOGE("record data cannot find fileType");
        return E_INVAL_ARG;
    }
    valueBucket.PutInt(PhotoColumn::MEDIA_TYPE, (fileType == FILE_TYPE_VIDEO) ?
        MEDIA_TYPE_VIDEO : MEDIA_TYPE_IMAGE);
    return E_OK;
}

int32_t FileDataConvertor::CompensateDataAdded(const DriveKit::DKRecord &record,
    NativeRdb::ValuesBucket &valueBucket)
{
    DriveKit::DKRecordData data;
    record.GetRecordData(data);
    int64_t dataAdded = 0;
    if (data.find(FILE_CREATED_TIME) != data.end()) {
        data[FILE_CREATED_TIME].GetLong(dataAdded);
    } else {
        dataAdded = static_cast<int64_t>(record.GetCreateTime());
    }
    if (dataAdded == 0) {
        LOGE("The createTime of record is incorrect");
    }
    valueBucket.PutLong(PhotoColumn::MEDIA_DATE_ADDED, dataAdded);
    CompensateFormattedDate(dataAdded / MILLISECOND_TO_SECOND, valueBucket);
    return E_OK;
}

int32_t FileDataConvertor::CompensateMetaDateModified(const DriveKit::DKRecord &record,
    NativeRdb::ValuesBucket &valueBucket)
{
    uint64_t metaDataModified = record.GetEditedTime();

    // imputed value, may not be accurate
    valueBucket.PutLong(PhotoColumn::PHOTO_META_DATE_MODIFIED, metaDataModified);
    return E_OK;
}

int32_t FileDataConvertor::CompensateSubtype(DriveKit::DKRecordData &data,
    NativeRdb::ValuesBucket &valueBucket)
{
    if (data.find(FILE_PROPERTIES) == data.end()) {
        LOGW("record data cannot find properties");
        return E_OK;
    }
    DriveKit::DKRecordFieldMap prop;
    data[FILE_PROPERTIES].GetRecordMap(prop);
    if (prop.find(FILE_SOURCE_PATH) == prop.end()) {
        valueBucket.PutInt(PhotoColumn::PHOTO_SUBTYPE, PhotoSubType::DEFAULT);
        return E_OK;
    }
    string sourcePath;
    if (prop[FILE_SOURCE_PATH].GetString(sourcePath) != DKLocalErrorCode::NO_ERROR) {
        LOGE("bad Subtype in props");
        valueBucket.PutInt(PhotoColumn::PHOTO_SUBTYPE, PhotoSubType::DEFAULT);
        return E_OK;
    }
    int32_t subType = PhotoSubType::DEFAULT;
    if (sourcePath.find("DCIM") != string::npos && sourcePath.find("Camera") != string::npos) {
        subType = PhotoSubType::CAMERA;
    } else if (sourcePath.find("Screenshots") != string::npos) {
        subType = PhotoSubType::SCREENSHOT;
    } else {
        subType = PhotoSubType::DEFAULT;
    }
    valueBucket.PutInt(PhotoColumn::PHOTO_SUBTYPE, subType);
    return E_OK;
}

int32_t FileDataConvertor::CompensateDuration(DriveKit::DKRecordData &data,
    NativeRdb::ValuesBucket &valueBucket)
{
    if (data.find(FILE_PROPERTIES) == data.end()) {
        LOGW("record data cannot find properties");
        return E_OK;
    }
    DriveKit::DKRecordFieldMap prop;
    data[FILE_PROPERTIES].GetRecordMap(prop);
    if (prop.find(FILE_DURATION) == prop.end()) {
        valueBucket.PutInt(PhotoColumn::MEDIA_DURATION, 0);
        return E_OK;
    }
    int32_t duration = 0;
    if (prop[FILE_DURATION].GetInt(duration) != DKLocalErrorCode::NO_ERROR) {
        LOGE("bad duration in props");
        valueBucket.PutInt(PhotoColumn::MEDIA_DURATION, 0);
        return E_OK;
    }
    valueBucket.PutInt(PhotoColumn::MEDIA_DURATION, duration);
    return E_OK;
}

int32_t FileDataConvertor::CompensateFormattedDate(const DriveKit::DKRecord &record,
    NativeRdb::ValuesBucket &valueBucket)
{
    LOGD("try to compensate formatted date");
    DriveKit::DKRecordData data;
    record.GetRecordData(data);
    DriveKit::DKRecordFieldMap attributes = data[FILE_ATTRIBUTES];
    string dateYear;
    if (attributes.find(PhotoColumn::PHOTO_DATE_YEAR) != attributes.end() &&
        attributes[PhotoColumn::PHOTO_DATE_YEAR].GetString(dateYear) == DriveKit::DKLocalErrorCode::NO_ERROR) {
        if (dateYear.length() == FORMATTED_YEAR_SIZE && dateYear != "1970") {
            LOGD("formatted date in cloud is correct");
            return E_OK;
        } else {
            LOGE("formatted date in cloud is incorrect");
            valueBucket.Delete(PhotoColumn::PHOTO_DATE_YEAR);
            valueBucket.Delete(PhotoColumn::PHOTO_DATE_MONTH);
            valueBucket.Delete(PhotoColumn::PHOTO_DATE_DAY);
        }
    }

    LOGI("record data cannot find formatted date");
    uint64_t timeAdded = 0;
    if (data.find(FILE_CREATED_TIME) != attributes.end()) {
        int64_t sTimeAdded;
        data[FILE_CREATED_TIME].GetLong(sTimeAdded);
        timeAdded = static_cast<uint64_t>(sTimeAdded);
    } else {
        timeAdded = record.GetCreateTime();
        if (timeAdded == 0) {
            LOGE("cloud date is invalid");
        }
    }
    CompensateFormattedDate(timeAdded / MILLISECOND_TO_SECOND, valueBucket);
    return E_OK;
}

int32_t FileDataConvertor::CompensateFormattedDate(uint64_t dateAdded, NativeRdb::ValuesBucket &valueBucket)
{
    string year = StrCreateTime(PhotoColumn::PHOTO_DATE_YEAR_FORMAT, dateAdded);
    string month = StrCreateTime(PhotoColumn::PHOTO_DATE_MONTH_FORMAT, dateAdded);
    string day = StrCreateTime(PhotoColumn::PHOTO_DATE_DAY_FORMAT, dateAdded);
    valueBucket.PutString(PhotoColumn::PHOTO_DATE_YEAR, year);
    valueBucket.PutString(PhotoColumn::PHOTO_DATE_MONTH, month);
    valueBucket.PutString(PhotoColumn::PHOTO_DATE_DAY, day);
    return E_OK;
}
int32_t FileDataConvertor::ExtractAttributeValue(DriveKit::DKRecordData &data,
    NativeRdb::ValuesBucket &valueBucket)
{
    DriveKit::DKRecordFieldMap attributes = data[FILE_ATTRIBUTES];
    auto size = CLOUD_SYNC_UNIQUE_COLUMNS.size() - NR_LOCAL_INFO;
    for (decltype(size) i = 0; i < size; i++) {
        auto field = CLOUD_SYNC_UNIQUE_COLUMNS[i];
        auto type = CLOUD_SYNC_UNIQUE_COLUMN_TYPES[i];
        if (attributes.find(field) == attributes.end()) {
            LOGE("filed %s not found in record.attributes", field.c_str());
            continue;
        }
        if (HandleField(attributes[field], valueBucket, field, type) != E_OK) {
            LOGE("HandleField %s failed", field.c_str());
        }
    }
    return E_OK;
}

int32_t FileDataConvertor::ExtractCompatibleValue(const DriveKit::DKRecord &record,
    DriveKit::DKRecordData &data, NativeRdb::ValuesBucket &valueBucket)
{
    /* extract value in first level*/
    RETURN_ON_ERR(ExtractSize(data, valueBucket));
    RETURN_ON_ERR(ExtractDisplayName(data, valueBucket));
    RETURN_ON_ERR(ExtractMimeType(data, valueBucket));
    RETURN_ON_ERR(ExtractDeviceName(data, valueBucket));
    RETURN_ON_ERR(CompensateDataAdded(record, valueBucket));
    RETURN_ON_ERR(ExtractDateModified(record, valueBucket));
    RETURN_ON_ERR(ExtractDateTaken(record, valueBucket));
    RETURN_ON_ERR(ExtractFavorite(data, valueBucket));
    RETURN_ON_ERR(ExtractDateTrashed(data, valueBucket));
    RETURN_ON_ERR(ExtractCloudId(record, valueBucket));
    RETURN_ON_ERR(ExtractDescription(data, valueBucket));

    if (data.find(FILE_PROPERTIES) == data.end()) {
        LOGW("record data cannot find properties");
        return E_OK;
    }
    DriveKit::DKRecordFieldMap prop;
    data[FILE_PROPERTIES].GetRecordMap(prop);

    /* extract value in properties*/
    RETURN_ON_ERR(ExtractOrientation(prop, valueBucket));
    RETURN_ON_ERR(ExtractPosition(prop, valueBucket));
    RETURN_ON_ERR(ExtractHeight(prop, valueBucket));
    RETURN_ON_ERR(ExtractWidth(prop, valueBucket));
    return E_OK;
}

int32_t FileDataConvertor::ExtractOrientation(DriveKit::DKRecordFieldMap &map,
    NativeRdb::ValuesBucket &valueBucket)
{
    if (map.find(FILE_ROTATION) == map.end()) {
        LOGI("RecordFieldMap cannot find orientation");
        return E_OK;
    }
    int32_t exifRotateValue;
    if (map[FILE_ROTATION].GetInt(exifRotateValue) != DKLocalErrorCode::NO_ERROR) {
        LOGE("extract orientation error");
        return E_INVAL_ARG;
    }
    switch (exifRotateValue) {
        case ORIENTATION_NORMAL:
            exifRotateValue = ROTATE_ANGLE_0;
            break;
        case ORIENTATION_ROTATE_90:
            exifRotateValue = ROTATE_ANGLE_90;
            break;
        case ORIENTATION_ROTATE_180:
            exifRotateValue = ROTATE_ANGLE_180;
            break;
        case ORIENTATION_ROTATE_270:
            exifRotateValue = ROTATE_ANGLE_270;
            break;
        default:
            exifRotateValue = ROTATE_ANGLE_0;
            break;
    }
    valueBucket.PutInt(PhotoColumn::PHOTO_ORIENTATION, exifRotateValue);
    return E_OK;
}

int32_t FileDataConvertor::ExtractPosition(DriveKit::DKRecordFieldMap &map,
    NativeRdb::ValuesBucket &valueBucket)
{
    if (map.find(FILE_POSITION) == map.end()) {
        LOGI("RecordFieldMap cannot find position");
        return E_OK;
    }
    string position;
    if (map[FILE_POSITION].GetString(position) != DKLocalErrorCode::NO_ERROR) {
        LOGE("extract orientation error");
        return E_INVAL_ARG;
    }
    string latitude;
    string longitude;
    regex positionPattern("(-?\\d+\\.?\\d+|0).*?(-?\\d+\\.?\\d+|0)");
    smatch match;
    if (regex_search(position, match, positionPattern)) {
        latitude = match[FIRST_MATCH_PARAM];
        longitude = match[SECOND_MATCH_PARAM];
    } else {
        LOGE("position %{public}s extract latitude or longitude error", position.c_str());
        return E_INVAL_ARG;
    }
    stringstream latitudestream(latitude);
    stringstream longitudestream(longitude);
    latitudestream.precision(15); // 15:precision
    longitudestream.precision(15); // 15:precision
    double latitudeValue;
    double longitudeValue;
    latitudestream >> latitudeValue;
    longitudestream >> longitudeValue;

    valueBucket.PutDouble(PhotoColumn::PHOTO_LATITUDE, latitudeValue);
    valueBucket.PutDouble(PhotoColumn::PHOTO_LONGITUDE, longitudeValue);
    return E_OK;
}

int32_t FileDataConvertor::ExtractHeight(DriveKit::DKRecordFieldMap &map,
    NativeRdb::ValuesBucket &valueBucket)
{
    if (map.find(FILE_HEIGHT) == map.end()) {
        LOGI("RecordFieldMap cannot find height");
        return E_OK;
    }
    int32_t height;
    if (map[FILE_HEIGHT].GetInt(height) != DKLocalErrorCode::NO_ERROR) {
        LOGE("extract height error");
        return E_INVAL_ARG;
    }
    if (height == 0) {
        LOGE("The height of the record is incorrect");
    }
    valueBucket.PutInt(PhotoColumn::PHOTO_HEIGHT, height);
    return E_OK;
}

int32_t FileDataConvertor::ExtractWidth(DriveKit::DKRecordFieldMap &map,
    NativeRdb::ValuesBucket &valueBucket)
{
    if (map.find(FILE_WIDTH) == map.end()) {
        LOGI("RecordFieldMap cannot find width");
        return E_OK;
    }
    int32_t width;
    if (map[FILE_WIDTH].GetInt(width) != DKLocalErrorCode::NO_ERROR) {
        LOGE("extract height error");
        return E_INVAL_ARG;
    }
    if (width == 0) {
        LOGE("The width of the record is incorrect");
    }
    valueBucket.PutInt(PhotoColumn::PHOTO_WIDTH, width);
    return E_OK;
}

int32_t FileDataConvertor::ExtractSize(DriveKit::DKRecordData &data,
    NativeRdb::ValuesBucket &valueBucket)
{
    if (data.find(FILE_SIZE) == data.end()) {
        LOGE("record data cannot find FILE_SIZE");
        return E_OK;
    }
    int64_t size;
    if (data[FILE_SIZE].GetLong(size) != DKLocalErrorCode::NO_ERROR) {
        LOGE("extract size error");
        return E_INVAL_ARG;
    }
    valueBucket.PutLong(PhotoColumn::MEDIA_SIZE, size);
    return E_OK;
}

int32_t FileDataConvertor::ExtractDisplayName(DriveKit::DKRecordData &data,
    NativeRdb::ValuesBucket &valueBucket)
{
    if (data.find(FILE_FILE_NAME) == data.end()) {
        LOGI("record data cannot find FILE_FILE_NAME");
        return E_OK;
    }
    string displayName;
    if (data[FILE_FILE_NAME].GetString(displayName) != DKLocalErrorCode::NO_ERROR) {
        LOGE("extract displayName error");
        return E_INVAL_ARG;
    }
    valueBucket.PutString(PhotoColumn::MEDIA_NAME, displayName);
    return E_OK;
}

int32_t FileDataConvertor::ExtractMimeType(DriveKit::DKRecordData &data,
    NativeRdb::ValuesBucket &valueBucket)
{
    if (data.find(FILE_MIME_TYPE) == data.end()) {
        LOGD("record data cannot find FILE_MIME_TYPE");
        return E_OK;
    }
    string mimeType;
    if (data[FILE_MIME_TYPE].GetString(mimeType) != DKLocalErrorCode::NO_ERROR) {
        LOGE("extract mimeType error");
        return E_INVAL_ARG;
    }
    valueBucket.PutString(PhotoColumn::MEDIA_MIME_TYPE, mimeType);
    return E_OK;
}

int32_t FileDataConvertor::ExtractDeviceName(DriveKit::DKRecordData &data,
    NativeRdb::ValuesBucket &valueBucket)
{
    if (data.find(FILE_SOURCE) == data.end()) {
        LOGI("record data cannot find FILE_SOURCE");
        return E_OK;
    }
    string deviceName;
    if (data[FILE_SOURCE].GetString(deviceName) != DKLocalErrorCode::NO_ERROR) {
        LOGE("extract deviceName error");
        return E_INVAL_ARG;
    }
    valueBucket.PutString(PhotoColumn::MEDIA_DEVICE_NAME, deviceName);
    return E_OK;
}

int32_t FileDataConvertor::ExtractDateModified(const DriveKit::DKRecord &record,
    NativeRdb::ValuesBucket &valueBucket)
{
    DriveKit::DKRecordData data;
    record.GetRecordData(data);
    int64_t dateModified = 0;
    if (data.find(FILE_ATTRIBUTES) != data.end()) {
        DriveKit::DKRecordFieldMap attributes;
        data[FILE_ATTRIBUTES].GetRecordMap(attributes);
        if (attributes.find(FILE_EDITED_TIME_MS) != attributes.end()) {
            if (attributes[FILE_EDITED_TIME_MS].GetLong(dateModified) == DKLocalErrorCode::NO_ERROR) {
                valueBucket.PutLong(PhotoColumn::MEDIA_DATE_MODIFIED, dateModified);
                return E_OK;
            }
        }
    }
    dateModified = static_cast<int64_t>(record.GetEditedTime());
    valueBucket.PutLong(PhotoColumn::MEDIA_DATE_MODIFIED, dateModified);
    return E_OK;
}

int32_t FileDataConvertor::ExtractDateTaken(const DriveKit::DKRecord &record,
    NativeRdb::ValuesBucket &valueBucket)
{
    uint64_t dataTaken = record.GetCreateTime() / MILLISECOND_TO_SECOND;
    valueBucket.PutLong(PhotoColumn::MEDIA_DATE_TAKEN, dataTaken);
    return E_OK;
}

int32_t FileDataConvertor::ExtractFavorite(DriveKit::DKRecordData &data,
    NativeRdb::ValuesBucket &valueBucket)
{
    if (data.find(FILE_FAVORITE) == data.end()) {
        LOGI("record data cannot find FILE_FAVORITE");
        return E_OK;
    }
    bool isFavorite;
    if (data[FILE_FAVORITE].GetBool(isFavorite) != DKLocalErrorCode::NO_ERROR) {
        LOGE("extract isFavorite error");
        return E_INVAL_ARG;
    }
    valueBucket.PutInt(PhotoColumn::MEDIA_IS_FAV, isFavorite ? 1 : 0);
    return E_OK;
}

int32_t FileDataConvertor::ExtractDateTrashed(DriveKit::DKRecordData &data,
    NativeRdb::ValuesBucket &valueBucket)
{
    if (data.find(FILE_RECYCLE_TIME) == data.end() && data.find(FILE_RECYCLED) == data.end()) {
        return E_OK;
    }
    bool isRecycle = false;
    if (data[FILE_RECYCLED].GetBool(isRecycle) != DKLocalErrorCode::NO_ERROR) {
        LOGE("extract FILE_RECYCLED error");
        return E_INVAL_ARG;
    }
    if (isRecycle) {
        int64_t dataTrashed = 0;
        if (data[FILE_RECYCLE_TIME].GetLong(dataTrashed) != DKLocalErrorCode::NO_ERROR) {
            LOGE("extract dataTrashed error");
            return E_INVAL_ARG;
        }
        valueBucket.PutLong(PhotoColumn::MEDIA_DATE_TRASHED, dataTrashed);
    } else {
        valueBucket.PutLong(PhotoColumn::MEDIA_DATE_TRASHED, 0);
    }
    return E_OK;
}

int32_t FileDataConvertor::ExtractCloudId(const DriveKit::DKRecord &record,
    NativeRdb::ValuesBucket &valueBucket)
{
    string cloudId = record.GetRecordId();
    valueBucket.PutString(PhotoColumn::PHOTO_CLOUD_ID, cloudId);
    return E_OK;
}

int32_t FileDataConvertor::ExtractDescription(DriveKit::DKRecordData &data,
    NativeRdb::ValuesBucket &valueBucket)
{
    if (data.find(FILE_DESCRIPTION) == data.end()) {
        LOGD("record data cannot find FILE_DESCRIPTION");
        return E_OK;
    }
    string description;
    if (data[FILE_DESCRIPTION].GetString(description) != DKLocalErrorCode::NO_ERROR) {
        LOGE("extract description error");
        return E_INVAL_ARG;
    }
    valueBucket.PutString(PhotoColumn::PHOTO_USER_COMMENT, description);
    return E_OK;
}

} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
