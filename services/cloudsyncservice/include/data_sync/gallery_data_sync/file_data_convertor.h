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

#include <ctime>
#include <iostream>
#include <sstream>
#include <unordered_map>

#include "data_convertor.h"
#include "dfs_error.h"
#include "utils_log.h"
#include "gallery_file_const.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {

class FileDataConvertor : public DataConvertor {
public:

    enum PhotoSubType {
        DEFAULT,
        SCREENSHOT,
        CAMERA
    };

    FileDataConvertor(int32_t userId, std::string &bundleName, OperationType type, const std::function<void(
        int32_t err, NativeRdb::ResultSet &resultSet)> &func = nullptr);
    ~FileDataConvertor() = default;

    /* resultSet -> record */
    int32_t Convert(DriveKit::DKRecord &record, NativeRdb::ResultSet &resultSet);
    void HandleErr(int32_t err, NativeRdb::ResultSet &resultSet);
    /* record -> resultSet */
    int32_t Convert(DriveKit::DKRecord &record, NativeRdb::ValuesBucket &valuesBucket);
    int32_t ConvertAsset(DriveKit::DKRecord &record, NativeRdb::ResultSet &resultSet);

    std::string GetThumbPath(const std::string &path, const std::string &key);
    std::string GetHmdfsLocalPath(const std::string &path);
    std::string GetThumbParentPath(const std::string &path);
    std::string GetThumbPathInCloud(const std::string &path, const std::string &key);
    std::string GetLocalPathToCloud(const std::string &path);
    /* path conversion */
    std::string GetLowerPath(const std::string &path);
    std::string GetLowerTmpPath(const std::string &path);
    std::string GetSandboxPath(const std::string &path);
    std::string GetCloudPath(const std::string &path);
    std::string GetPathWithoutTmp(const std::string &path);

private:
    /* record id */
    int32_t FillRecordId(DriveKit::DKRecord &record, NativeRdb::ResultSet &resultSet);

    /* basic */
    int32_t HandleAlbumId(DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet);
    int32_t HandleFileName(DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet);
    int32_t HandleHash(DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet);
    int32_t HandleSource(DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet);
    int32_t HandleFileType(DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet);
    int32_t HandleCreatedTime(DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet);
    int32_t HandleFavorite(DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet);
    int32_t HandleDescription(DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet);
    int32_t HandleRecycleTime(DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet);
    int32_t HandleRecycled(DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet);
    int32_t HandleSize(DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet);
    int32_t HandleMimeType(DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet);
    int32_t HandleEditedTime(DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet);

    int32_t HandleUniqueFileds(DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet);
    int32_t HandleCompatibleFileds(DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet);

    /* properties */
    int32_t HandleProperties(DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet);

    int32_t HandleDuration(DriveKit::DKRecordFieldMap &map, NativeRdb::ResultSet &resultSet);
    int32_t HandleHeight(DriveKit::DKRecordFieldMap &map, NativeRdb::ResultSet &resultSet);
    int32_t HandleRotate(DriveKit::DKRecordFieldMap &map, NativeRdb::ResultSet &resultSet);
    int32_t HandleWidth(DriveKit::DKRecordFieldMap &map, NativeRdb::ResultSet &resultSet);
    int32_t HandlePosition(DriveKit::DKRecordFieldMap &map, NativeRdb::ResultSet &resultSet);
    int32_t HandleDataAdded(DriveKit::DKRecordFieldMap &map, NativeRdb::ResultSet &resultSet);
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
    int32_t HandleFormattedDate(DriveKit::DKRecordFieldMap &map, NativeRdb::ResultSet &resultSet);
    std::string StrCreateTime(const std::string &format, int64_t time);
    /* properties - general */
    int32_t HandleGeneral(DriveKit::DKRecordFieldMap &map, NativeRdb::ResultSet &resultSet);

    int32_t HandleAttributes(DriveKit::DKRecordFieldMap &map, NativeRdb::ResultSet &resultSet);
    /* attachments */
    int32_t HandleAttachments(DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet);

    /* local info */
    int32_t HandleLocalInfo(DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet);

    int32_t HandleContent(DriveKit::DKRecordData &data, std::string &path);
    int32_t HandleThumbnail(DriveKit::DKRecordData &data, std::string &path);
    int32_t HandleLcd(DriveKit::DKRecordData &data, std::string &path);

    int32_t TryCompensateValue(const DriveKit::DKRecord &record, DriveKit::DKRecordData &data,
        NativeRdb::ValuesBucket &valueBucket);
    int32_t ExtractAttributeValue(DriveKit::DKRecordData &data, NativeRdb::ValuesBucket &valueBucket);

    /* compensate missing filed according gallery value*/
    int32_t CompensateData(DriveKit::DKRecordData &data, NativeRdb::ValuesBucket &valueBucket);
    int32_t CompensateTitle(DriveKit::DKRecordData &data, NativeRdb::ValuesBucket &valueBucket);
    int32_t CompensateMediaType(DriveKit::DKRecordData &data, NativeRdb::ValuesBucket &valueBucket);
    int32_t CompensateDataAdded(const DriveKit::DKRecord &record, NativeRdb::ValuesBucket &valueBucket);
    int32_t CompensateMetaDateModified(const DriveKit::DKRecord &record, NativeRdb::ValuesBucket &valueBucket);
    int32_t CompensateSubtype(DriveKit::DKRecordData &data, NativeRdb::ValuesBucket &valueBucket);
    int32_t CompensateDuration(DriveKit::DKRecordData &data, NativeRdb::ValuesBucket &valueBucket);
    int32_t CompensateFormattedDate(const DriveKit::DKRecord &record, NativeRdb::ValuesBucket &valueBucket);
    int32_t CompensateFormattedDate(uint64_t dateAdded, NativeRdb::ValuesBucket &valueBucket);
    /* extract compatible value stored in properties */
    int32_t ExtractCompatibleValue(const DriveKit::DKRecord &record,
        DriveKit::DKRecordData &data, NativeRdb::ValuesBucket &valueBucket);
    int32_t ExtractOrientation(DriveKit::DKRecordFieldMap &map, NativeRdb::ValuesBucket &valueBucket);
    int32_t ExtractPosition(DriveKit::DKRecordFieldMap &map, NativeRdb::ValuesBucket &valueBucket);
    int32_t ExtractHeight(DriveKit::DKRecordFieldMap &map, NativeRdb::ValuesBucket &valueBucket);
    int32_t ExtractWidth(DriveKit::DKRecordFieldMap &map, NativeRdb::ValuesBucket &valueBucket);

    int32_t ExtractSize(DriveKit::DKRecordData &data, NativeRdb::ValuesBucket &valueBucket);
    int32_t ExtractDisplayName(DriveKit::DKRecordData &data, NativeRdb::ValuesBucket &valueBucket);
    int32_t ExtractMimeType(DriveKit::DKRecordData &data, NativeRdb::ValuesBucket &valueBucket);
    int32_t ExtractDeviceName(DriveKit::DKRecordData &data, NativeRdb::ValuesBucket &valueBucket);
    int32_t ExtractDateModified(const DriveKit::DKRecord &record, NativeRdb::ValuesBucket &valueBucket);
    int32_t ExtractDateTaken(const DriveKit::DKRecord &record, NativeRdb::ValuesBucket &valueBucket);
    int32_t ExtractFavorite(DriveKit::DKRecordData &data, NativeRdb::ValuesBucket &valueBucket);
    int32_t ExtractDateTrashed(DriveKit::DKRecordData &data, NativeRdb::ValuesBucket &valueBucket);
    int32_t ExtractCloudId(const DriveKit::DKRecord &record, NativeRdb::ValuesBucket &valueBucket);
    int32_t ExtractDescription(DriveKit::DKRecordData &data, NativeRdb::ValuesBucket &valueBucket);

    bool IfContainsAttributes(const DriveKit::DKRecord &record);

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
    static std::string tmpSuffix_;

    /* err */
    std::function<void(int32_t, NativeRdb::ResultSet &resultSet)> errHandler_;
};

inline int32_t FileDataConvertor::HandleFileName(DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    std::string displayName;
    int32_t ret = GetString(Media::PhotoColumn::MEDIA_NAME, displayName, resultSet);
    if (ret != E_OK) {
        return ret;
    }
    data[FILE_FILE_NAME] = DriveKit::DKRecordField(displayName);
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
    /* local ms to cloud ms */
    data[FILE_CREATED_TIME] = DriveKit::DKRecordField(std::to_string(val));
    return E_OK;
}

inline int32_t FileDataConvertor::HandleHash(DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    data[FILE_HASH_ID] = DriveKit::DKRecordField("Md5_default_hash");
    return E_OK;
}

inline int32_t FileDataConvertor::HandleSize(DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    int32_t val;
    int32_t ret = GetInt(Media::PhotoColumn::MEDIA_SIZE, val, resultSet);
    if (ret != E_OK) {
        return ret;
    }
    data[FILE_SIZE] = DriveKit::DKRecordField(val);
    return E_OK;
}

inline int32_t FileDataConvertor::HandleSource(DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    std::string val;
    int32_t ret = GetString(Media::PhotoColumn::MEDIA_DEVICE_NAME, val, resultSet);
    if (ret != E_OK) {
        return ret;
    }
    data[FILE_SOURCE] = DriveKit::DKRecordField(val);
    return E_OK;
}

inline int32_t FileDataConvertor::HandleFileType(DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    int32_t val;
    int32_t ret = GetInt(Media::PhotoColumn::MEDIA_TYPE, val, resultSet);
    if (ret != E_OK) {
        return ret;
    }
    data[FILE_FILETYPE] = DriveKit::DKRecordField(val ==
        Media::MEDIA_TYPE_VIDEO ? FILE_TYPE_VIDEO : FILE_TYPE_IMAGE);
    return E_OK;
}

inline int32_t FileDataConvertor::HandleRecycled(DriveKit::DKRecordData &data,
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

inline int32_t FileDataConvertor::HandleRecycleTime(DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    int64_t val;
    int32_t ret = GetLong(Media::PhotoColumn::MEDIA_DATE_TRASHED, val, resultSet);
    if (ret != E_OK) {
        return ret;
    }
    data[FILE_RECYCLE_TIME] = DriveKit::DKRecordField(std::to_string(val));
    return E_OK;
}

inline int32_t FileDataConvertor::HandleMimeType(DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    std::string mimeType;
    int32_t ret = GetString(Media::PhotoColumn::MEDIA_MIME_TYPE, mimeType, resultSet);
    if (ret != E_OK) {
        return ret;
    }
    data[FILE_MIME_TYPE] = DriveKit::DKRecordField(mimeType);
    return E_OK;
}

inline int32_t FileDataConvertor::HandleEditedTime(DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    int64_t val;
    int32_t ret = GetLong(Media::PhotoColumn::MEDIA_DATE_MODIFIED, val, resultSet);
    if (ret != E_OK) {
        return ret;
    }
    data[FILE_EDITED_TIME] = DriveKit::DKRecordField(std::to_string(val));
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
    data[FILE_FAVORITE] = DriveKit::DKRecordField(!!val);
    return E_OK;
}

/* properties */
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

inline int32_t FileDataConvertor::HandleFirstUpdateTime(DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    int64_t val;
    int32_t ret = GetLong(Media::PhotoColumn::MEDIA_DATE_ADDED, val, resultSet);
    if (ret != E_OK) {
        return ret;
    }
    /* local ms to cloud ms */
    map[FILE_FIRST_UPDATE_TIME] = DriveKit::DKRecordField(std::to_string(val));
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
    /* local ms to cloud ms */
    map[FILE_FILE_CREATE_TIME] = DriveKit::DKRecordField(std::to_string(val));
    return E_OK;
}

inline int32_t FileDataConvertor::HandleSourcePath(DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    std::string displayName;
    int32_t ret = GetString(Media::PhotoColumn::MEDIA_NAME, displayName, resultSet);
    if (ret != E_OK) {
        return ret;
    }
    if (type_ == FILE_CREATE) {
        map[FILE_SOURCE_PATH] = DriveKit::DKRecordField("/storage/emulated/0/Pictures/cloud/Imports/" + displayName);
    }
    return E_OK;
}

inline int32_t FileDataConvertor::HandleRelativeBucketId(DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    return E_OK;
}

inline int32_t FileDataConvertor::HandleDuration(DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    int32_t duration;
    int32_t ret = GetInt(Media::PhotoColumn::MEDIA_DURATION, duration, resultSet);
    if (ret != E_OK) {
        return ret;
    }
    map[FILE_DURATION] = DriveKit::DKRecordField(duration);
    return E_OK;
}

inline int32_t FileDataConvertor::HandleDataAdded(DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    int64_t val;
    int32_t ret = GetLong(Media::PhotoColumn::MEDIA_DATE_ADDED, val, resultSet);
    if (ret != E_OK) {
        return ret;
    }
    map[Media::PhotoColumn::MEDIA_DATE_ADDED] = DriveKit::DKRecordField(val / MILLISECOND_TO_SECOND);
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
    map[FILE_EDITED_TIME_MS] = DriveKit::DKRecordField(val);
    map[Media::PhotoColumn::MEDIA_DATE_MODIFIED] = DriveKit::DKRecordField(val / MILLISECOND_TO_SECOND);
    return E_OK;
}

inline int32_t FileDataConvertor::HandleTimeZone(DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    map[FILE_TIME_ZONE] = DriveKit::DKRecordField("");
    return E_OK;
}

inline int32_t FileDataConvertor::HandleDescription(DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    std::string val;
    int32_t ret = GetString(Media::PhotoColumn::PHOTO_USER_COMMENT, val, resultSet);
    if (ret != E_OK) {
        LOGE("Get user comment err %{public}d", ret);
        return E_RDB;
    }
    data[FILE_DESCRIPTION] = DriveKit::DKRecordField(val);
    return E_OK;
}
inline int32_t FileDataConvertor::HandleAlbumId(DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    return E_OK;
}

inline int32_t FileDataConvertor::HandleLocalInfo(DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    int32_t val;
    int32_t ret = GetInt(Media::PhotoColumn::MEDIA_ID, val, resultSet);
    if (ret != E_OK) {
        return ret;
    }
    data[FILE_LOCAL_ID] = DriveKit::DKRecordField(val);
    return E_OK;
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
#endif // OHOS_CLOUD_SYNC_SERVICE_FILE_DATA_CONVERTOR_H
