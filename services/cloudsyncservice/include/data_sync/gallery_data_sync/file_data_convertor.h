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

#ifndef OHOS_CLOUD_SYNC_SERVICE_FILE_DATA_CONVERTOR_H
#define OHOS_CLOUD_SYNC_SERVICE_FILE_DATA_CONVERTOR_H

#include <unordered_map>

#include "data_convertor.h"
#include "dfs_error.h"
#include "gallery_file_const.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {

/* thumb */
const std::string THUMB_SUFFIX = "THM";
const std::string LCD_SUFFIX = "LCD";

/* s -> ms */
const int32_t SECOND_TO_MILLISECOND = 1000;

class FileDataConvertor : public DataConvertor {
public:
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

    FileDataConvertor(int32_t userId, std::string &bundleName, OperationType type);
    ~FileDataConvertor() = default;

    int32_t Convert(DriveKit::DKRecord &record, NativeRdb::ResultSet &resultSet);
    std::string GetThumbPath(const std::string &path, const std::string &key);
    std::string GetThumbPathInCloud(const std::string &path, const std::string &key);
    /* path conversion */
    std::string GetLowerPath(const std::string &path);
    std::string GetLowerTmpPath(const std::string &path);
    std::string GetSandboxPath(const std::string &path);

private:
    /* record id */
    int32_t FillRecordId(DriveKit::DKRecord &record, NativeRdb::ResultSet &resultSet);

    /* basic */
    int32_t HandleAlbumId(DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet);
    int32_t HandleFileName(DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet);
    int32_t HandleHashId(DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet);
    int32_t HandleSource(DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet);
    int32_t HandleFileType(DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet);
    int32_t HandleCreatedTime(DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet);
    int32_t HandleFavorite(DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet);
    int32_t HandleDescription(DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet);
    int32_t HandleRecycle(DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet);

    /* properties */
    int32_t HandleProperties(DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet);

    int32_t HandleHeight(DriveKit::DKRecordFieldMap &map, NativeRdb::ResultSet &resultSet);
    int32_t HandleRotation(DriveKit::DKRecordFieldMap &map, NativeRdb::ResultSet &resultSet);
    int32_t HandleWidth(DriveKit::DKRecordFieldMap &map, NativeRdb::ResultSet &resultSet);
    int32_t HandlePosition(DriveKit::DKRecordFieldMap &map, NativeRdb::ResultSet &resultSet);
    int32_t HandleDataModified(DriveKit::DKRecordFieldMap &map, NativeRdb::ResultSet &resultSet);
    int32_t HandleDetailTime(DriveKit::DKRecordFieldMap &map, NativeRdb::ResultSet &resultSet);
    int32_t HandleFileCreateTime(DriveKit::DKRecordFieldMap &map, NativeRdb::ResultSet &resultSet);
    int32_t HandleFirstUpdateTime(DriveKit::DKRecordFieldMap &map, NativeRdb::ResultSet &resultSet);
    int32_t HandleRelativeBucketId(DriveKit::DKRecordFieldMap &map, NativeRdb::ResultSet &resultSet);
    int32_t HandleSourceFileName(DriveKit::DKRecordFieldMap &map, NativeRdb::ResultSet &resultSet);
    int32_t HandleSourcePath(DriveKit::DKRecordFieldMap &map, NativeRdb::ResultSet &resultSet);
    int32_t HandleTimeZone(DriveKit::DKRecordFieldMap &map, NativeRdb::ResultSet &resultSet);
    int32_t HandleThumbSize(DriveKit::DKRecordFieldMap &map, NativeRdb::ResultSet &resultSet);
    int32_t HandleLcdSize(DriveKit::DKRecordFieldMap &map, NativeRdb::ResultSet &resultSet);
    /* properties - general */
    int32_t HandleGeneral(DriveKit::DKRecordFieldMap &map, NativeRdb::ResultSet &resultSet);

    /* attachments */
    int32_t HandleAttachments(DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet);

    int32_t HandleContent(DriveKit::DKRecordData &data, std::string &path);
    int32_t HandleThumbnail(DriveKit::DKRecordData &data, std::string &path);
    int32_t HandleLcd(DriveKit::DKRecordData &data, std::string &path);

    /* identifier */
    int32_t userId_;
    std::string bundleName_;
    static std::string recordType_;
    OperationType type_;

    /* path */
    static std::string prefix_;
    static std::string suffix_;
    static std::string sandboxPrefix_;
    static std::string prefixLCD_;
    static std::string suffixLCD_;
    static std::string prefixCloud_;
    static std::string suffixCloud_;
};

/* basic */
inline int32_t FileDataConvertor::HandleAlbumId(DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    data[FILE_ALBUM_ID] = DriveKit::DKRecordField("default-album-1");
    return E_OK;
}

inline int32_t FileDataConvertor::HandleFileName(DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    std::string val;
    int32_t ret = GetString(Media::PhotoColumn::MEDIA_NAME, val, resultSet);
    if (ret != E_OK) {
        return ret;
    }
    data[FILE_FILE_NAME] = DriveKit::DKRecordField(val);
    return E_OK;
}

inline int32_t FileDataConvertor::HandleHashId(DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    data[FILE_HASH_ID] = DriveKit::DKRecordField("1");
    return E_OK;
}

inline int32_t FileDataConvertor::HandleSource(DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    data[FILE_SOURCE] = DriveKit::DKRecordField("");
    return E_OK;
}

inline int32_t FileDataConvertor::HandleFileType(DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    DriveKit::DKRecordFieldMap properties;
    auto err = data[FILE_PROPERTIES].GetRecordMap(properties);
    if (err != DriveKit::DKLocalErrorCode::NO_ERROR) {
        LOGE("get properties err %{public}d", err);
        return E_DATA;
    }
    int32_t mediaType = properties[Media::MediaColumn::MEDIA_TYPE];
    data[FILE_FILETYPE] = DriveKit::DKRecordField(mediaType ==
        Media::MEDIA_TYPE_VIDEO ? FILE_TYPE_VIDEO : FILE_TYPE_IMAGE);
    return E_OK;
}

inline int32_t FileDataConvertor::HandleCreatedTime(DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    int64_t val;
    int32_t ret = GetLong(Media::PhotoColumn::MEDIA_DATE_ADDED, val, resultSet);
    if (ret != E_OK) {
        return ret;
    }
    /* no overflow: 64 >> 32 + 3 */
    data[FILE_CREATED_TIME] = DriveKit::DKRecordField(std::to_string(val * SECOND_TO_MILLISECOND));
    return E_OK;
}

inline int32_t FileDataConvertor::HandleFavorite(DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    int32_t val;
    int32_t ret = GetInt(Media::PhotoColumn::MEDIA_IS_FAV, val, resultSet);
    if (ret != E_OK) {
        return ret;
    }
    data[FILE_FAVORITE] = DriveKit::DKRecordField(val);
    return E_OK;
}

inline int32_t FileDataConvertor::HandleDescription(DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    data[FILE_DESCRIPTION] = DriveKit::DKRecordField("");
    return E_OK;
}

inline int32_t FileDataConvertor::HandleRecycle(DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    int64_t val;
    int32_t ret = GetLong(Media::PhotoColumn::MEDIA_DATE_TRASHED, val, resultSet);
    if (ret != E_OK) {
        return ret;
    }
    data[FILE_RECYCLED] = DriveKit::DKRecordField(!!val);
    return E_OK;
}

/* properties */
inline int32_t FileDataConvertor::HandleHeight(DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    int32_t val;
    int32_t ret = GetInt(Media::PhotoColumn::PHOTO_HEIGHT, val, resultSet);
    if (ret != E_OK) {
        return ret;
    }
    map[FILE_HEIGHT] = DriveKit::DKRecordField(val);
    return E_OK;
}

inline int32_t FileDataConvertor::HandleRotation(DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    int32_t val;
    int32_t ret = GetInt(Media::PhotoColumn::PHOTO_ORIENTATION, val, resultSet);
    if (ret != E_OK) {
        return ret;
    }
    map[FILE_ROTATION] = DriveKit::DKRecordField(val);
    return E_OK;
}

inline int32_t FileDataConvertor::HandleWidth(DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    int32_t val;
    int32_t ret = GetInt(Media::PhotoColumn::PHOTO_WIDTH, val, resultSet);
    if (ret != E_OK) {
        return ret;
    }
    map[FILE_WIDTH] = DriveKit::DKRecordField(val);
    return E_OK;
}

inline int32_t FileDataConvertor::HandlePosition(DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    map[FILE_POSITION] = DriveKit::DKRecordField("");
    return E_OK;
}

inline int32_t FileDataConvertor::HandleDataModified(DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    int64_t val;
    int32_t ret = GetLong(Media::PhotoColumn::MEDIA_DATE_MODIFIED, val, resultSet);
    if (ret != E_OK) {
        return ret;
    }
    map[FILE_DATA_MODIFIED] = DriveKit::DKRecordField(std::to_string(val));
    return E_OK;
}

inline int32_t FileDataConvertor::HandleDetailTime(DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    map[FILE_DETAIL_TIME] = DriveKit::DKRecordField("");
    return E_OK;
}

inline int32_t FileDataConvertor::HandleFileCreateTime(DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    int64_t val;
    int32_t ret = GetLong(Media::PhotoColumn::MEDIA_DATE_ADDED, val, resultSet);
    if (ret != E_OK) {
        return ret;
    }
    map[FILE_FILE_CREATE_TIME] = DriveKit::DKRecordField(std::to_string(val));
    return E_OK;
}

inline int32_t FileDataConvertor::HandleFirstUpdateTime(DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    map[FILE_FIRST_UPDATE_TIME] = DriveKit::DKRecordField("");
    return E_OK;
}

inline int32_t FileDataConvertor::HandleRelativeBucketId(DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    map[FILE_RELATIVE_BUCKET_ID] = DriveKit::DKRecordField("1");
    return E_OK;
}

inline int32_t FileDataConvertor::HandleSourceFileName(DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    std::string val;
    int32_t ret = GetString(Media::PhotoColumn::MEDIA_NAME, val, resultSet);
    if (ret != E_OK) {
        return ret;
    }
    map[FILE_SOURCE_FILE_NAME] = DriveKit::DKRecordField(val);
    return E_OK;
}

inline int32_t FileDataConvertor::HandleSourcePath(DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    std::string val;
    int32_t ret = GetString(Media::PhotoColumn::MEDIA_FILE_PATH, val, resultSet);
    if (ret != E_OK) {
        return ret;
    }
    map[FILE_SOURCE_PATH] = DriveKit::DKRecordField(val);
    return E_OK;
}

inline int32_t FileDataConvertor::HandleTimeZone(DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    map[FILE_TIME_ZONE] = DriveKit::DKRecordField("");
    return E_OK;
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
#endif // OHOS_CLOUD_SYNC_SERVICE_FILE_DATA_CONVERTOR_H
