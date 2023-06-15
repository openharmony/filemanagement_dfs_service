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

namespace OHOS {
namespace FileManagement {
namespace CloudSync {

/* thumb */
const std::string THUMB_SUFFIX = "THM";
const std::string LCD_SUFFIX = "LCD";

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
    /* path conversion */
    std::string GetLowerPath(const std::string &path);
    std::string GetLowerTmpPath(const std::string &path);
    std::string GetSandboxPath(const std::string &path);

private:
    /* record id */
    int32_t FillRecordId(DriveKit::DKRecord &record, NativeRdb::ResultSet &resultSet);

    /* basic */
    int32_t HandleAlbumId(const std::string &key, DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet);
    int32_t HandleFileName(const std::string &key, DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet);
    int32_t HandleHashId(const std::string &key, DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet);
    int32_t HandleSource(const std::string &key, DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet);
    int32_t HandleFileType(const std::string &key, DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet);
    int32_t HandleCreatedTime(const std::string &key, DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet);
    int32_t HandleFavorite(const std::string &key, DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet);
    int32_t HandleDescription(const std::string &key, DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet);
    int32_t HandleRecycle(const std::string &key, DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet);

    /* properties */
    int32_t HandleProperties(const std::string &key, DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet);

    int32_t HandleHeight(const std::string &key, DriveKit::DKRecordFieldMap &map, NativeRdb::ResultSet &resultSet);
    int32_t HandleRotation(const std::string &key, DriveKit::DKRecordFieldMap &map, NativeRdb::ResultSet &resultSet);
    int32_t HandleWidth(const std::string &key, DriveKit::DKRecordFieldMap &map, NativeRdb::ResultSet &resultSet);
    int32_t HandlePosition(const std::string &key, DriveKit::DKRecordFieldMap &map, NativeRdb::ResultSet &resultSet);
    int32_t HandleDataModified(const std::string &key, DriveKit::DKRecordFieldMap &map, NativeRdb::ResultSet &resultSet);
    int32_t HandleDetailTime(const std::string &key, DriveKit::DKRecordFieldMap &map, NativeRdb::ResultSet &resultSet);
    int32_t HandleFileCreateTime(const std::string &key, DriveKit::DKRecordFieldMap &map, NativeRdb::ResultSet &resultSet);
    int32_t HandleFirstUpdateTime(const std::string &key, DriveKit::DKRecordFieldMap &map, NativeRdb::ResultSet &resultSet);
    int32_t HandleRelativeBucketId(const std::string &key, DriveKit::DKRecordFieldMap &map, NativeRdb::ResultSet &resultSet);
    int32_t HandleSourceFileName(const std::string &key, DriveKit::DKRecordFieldMap &map, NativeRdb::ResultSet &resultSet);
    int32_t HandleSourcePath(const std::string &key, DriveKit::DKRecordFieldMap &map, NativeRdb::ResultSet &resultSet);
    int32_t HandleTimeZone(const std::string &key, DriveKit::DKRecordFieldMap &map, NativeRdb::ResultSet &resultSet);
    /* properties - general */
    int32_t HandleGeneral(const std::string &key, DriveKit::DKRecordFieldMap &map, NativeRdb::ResultSet &resultSet);

    /* attachments */
    int32_t HandleAttachments(const std::string &key, DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet);

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

    /* thumb */
    static std::string thumb_suffix_;
    static std::string lcd_suffix_;
};
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
#endif // OHOS_CLOUD_SYNC_SERVICE_FILE_DATA_CONVERTOR_H
