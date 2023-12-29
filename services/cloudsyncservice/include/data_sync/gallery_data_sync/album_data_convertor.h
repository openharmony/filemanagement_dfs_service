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

#ifndef OHOS_CLOUD_SYNC_SERVICE_ALBUM_DATA_CONVERTOR_H
#define OHOS_CLOUD_SYNC_SERVICE_ALBUM_DATA_CONVERTOR_H

#include <unordered_map>

#include <uuid/uuid.h>

#include "data_convertor.h"
#include "dfs_error.h"
#include "gallery_album_const.h"
#include "gallery_file_const.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {

#define UUID_STR_LENGTH 37

class AlbumDataConvertor : public DataConvertor {
public:

    AlbumDataConvertor(OperationType type);
    ~AlbumDataConvertor() = default;

    int32_t Convert(DriveKit::DKRecord &record, NativeRdb::ResultSet &resultSet) override;
    int32_t Convert(DriveKit::DKRecord &record, NativeRdb::ValuesBucket &valueBucket) override;
    int32_t ExtractBundleName(DriveKit::DKRecordData &data, NativeRdb::ValuesBucket &valueBucket);
    int32_t ExtractLocalLanguage(DriveKit::DKRecordData &data, NativeRdb::ValuesBucket &valueBucket);

private:
    /* record id */
    int32_t FillRecordId(DriveKit::DKRecord &record, NativeRdb::ResultSet &resultSet);
    int32_t HandleRecordId(DriveKit::DKRecord &record, NativeRdb::ResultSet &resultSet);

    /* basic */
    int32_t HandleAlbumId(DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet);
    int32_t HandleAlbumName(DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet);
    int32_t HandleType(DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet);
    int32_t HandleAlbumLogicType(DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet);
    int32_t HandlePath(DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet);
    int32_t HandleSourceAlbum(DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet);
    int32_t HandleBundleName(DriveKit::DKRecordFieldMap &map, NativeRdb::ResultSet &resultSet);
    int32_t HandleLocalLanguage(DriveKit::DKRecordFieldMap &map, NativeRdb::ResultSet &resultSet);

    /* properties */
    int32_t HandleProperties(DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet);
    /* properties - general */
    int32_t HandleGeneral(DriveKit::DKRecordFieldMap &map, NativeRdb::ResultSet &resultSet);

    /* identifier */
    static const std::string recordType_;
    OperationType type_;
};

inline int32_t AlbumDataConvertor::HandleAlbumId(DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet)
{
    std::string val;
    int32_t ret = GetString(Media::PhotoAlbumColumns::ALBUM_ID, val, resultSet);
    if (ret != E_OK) {
        return ret;
    }
    data[ALBUM_ID] = DriveKit::DKRecordField(val);
    return E_OK;
}

inline int32_t AlbumDataConvertor::HandleAlbumName(DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet)
{
    std::string val;
    int32_t ret = GetString(Media::PhotoAlbumColumns::ALBUM_NAME, val, resultSet);
    if (ret != E_OK) {
        return ret;
    }
    data[ALBUM_NAME] = DriveKit::DKRecordField(val);
    return E_OK;
}

inline int32_t AlbumDataConvertor::HandleType(DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet)
{
    data[ALBUM_TYPE] = DriveKit::DKRecordField(AlbumType::NORMAL);
    return E_OK;
}

inline int32_t AlbumDataConvertor::HandleAlbumLogicType(DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet)
{
    data[ALBUM_LOGIC_TYPE] = DriveKit::DKRecordField(LogicType::LOGICAL);
    return E_OK;
}

inline int32_t AlbumDataConvertor::HandlePath(DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet)
{
    std::string val;
    int32_t ret = GetString(Media::PhotoAlbumColumns::ALBUM_NAME, val, resultSet);
    if (ret != E_OK) {
        return ret;
    }
    data[ALBUM_PATH] = DriveKit::DKRecordField(ALBUM_LOCAL_PATH_PREFIX + val);
    return E_OK;
}

inline int32_t AlbumDataConvertor::HandleBundleName(DriveKit::DKRecordFieldMap &map, NativeRdb::ResultSet &resultSet)
{
    std::string val;
    int32_t ret = GetString(TMP_ALBUM_BUNDLE_NAME, val, resultSet);
    if (ret != E_OK) {
        return ret;
    }
    if (val.empty()) {
        return E_INVAL_ARG;
    }
    map[TMP_ALBUM_BUNDLE_NAME] = DriveKit::DKRecordField(val);
    return E_OK;
}

inline int32_t AlbumDataConvertor::HandleLocalLanguage(DriveKit::DKRecordFieldMap &map, NativeRdb::ResultSet &resultSet)
{
    std::string val;
    int32_t ret = GetString(TMP_ALBUM_LOCAL_LANGUAGE, val, resultSet);
    if (ret != E_OK) {
        return ret;
    }
    map[TMP_ALBUM_LOCAL_LANGUAGE] = DriveKit::DKRecordField(val);
    return E_OK;
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
#endif // OHOS_CLOUD_SYNC_SERVICE_ALBUM_DATA_CONVERTOR_H
