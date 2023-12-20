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

#include "album_data_convertor.h"
#include "dk_error.h"

#include <unistd.h>
#include <sys/stat.h>

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
using namespace std;
using namespace NativeRdb;
using namespace Media;

/* record type */
const string AlbumDataConvertor::recordType_ = "album";

AlbumDataConvertor::AlbumDataConvertor(OperationType type) : type_(type)
{
}

int32_t AlbumDataConvertor::Convert(DriveKit::DKRecord &record, NativeRdb::ResultSet &resultSet)
{
    DriveKit::DKRecordData data;
    /* basic */
    RETURN_ON_ERR(HandleAlbumName(data, resultSet));
    RETURN_ON_ERR(HandleAlbumLogicType(data, resultSet));
    RETURN_ON_ERR(HandlePath(data, resultSet));
    RETURN_ON_ERR(HandleType(data, resultSet));

    /* properties */
    RETURN_ON_ERR(HandleProperties(data, resultSet));

    /* control info */
    record.SetRecordType(recordType_);
    int32_t albumType = -1;
    if (GetInt(Media::PhotoAlbumColumns::ALBUM_TYPE, albumType, resultSet) == E_OK &&
        albumType == AlbumType::SOURCE) {
        HandleSourceAlbum(data, resultSet);
    }
    if (type_ == FILE_CREATE) {
        record.SetNewCreate(true);
        RETURN_ON_ERR(HandleAlbumId(data, resultSet));
        RETURN_ON_ERR(HandleRecordId(record, resultSet));
    } else {
        int32_t ret = FillRecordId(record, resultSet);
        if (ret != E_OK) {
            LOGE("fill record id err %{public}d", ret);
            return ret;
        }
        data[ALBUM_ID] = DriveKit::DKRecordField(record.GetRecordId());
    }
    /* set data */
    record.SetRecordData(data);

    return E_OK;
}

/* record id */
int32_t AlbumDataConvertor::FillRecordId(DriveKit::DKRecord &record,
    NativeRdb::ResultSet &resultSet)
{
    string val;
    int32_t ret = GetString(PhotoAlbumColumns::ALBUM_CLOUD_ID, val, resultSet);
    if (ret != E_OK) {
        return ret;
    }
    record.SetRecordId(val);
    return E_OK;
}

/* properties */
int32_t AlbumDataConvertor::HandleProperties(DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet)
{
    DriveKit::DKRecordFieldMap map;
    /* general */
    RETURN_ON_ERR(HandleGeneral(map, resultSet));

    /* set map */
    data[ALBUM_PROPERTIES] = DriveKit::DKRecordField(map);
    return E_OK;
}

/* properties - general */
int32_t AlbumDataConvertor::HandleGeneral(DriveKit::DKRecordFieldMap &map, NativeRdb::ResultSet &resultSet)
{
    auto size = GALLERY_ALBUM_COLUMNS.size();
    /* size - 1: skip cloud id */
    for (decltype(size) i = 0; i < size; i++) {
        const string &key = GALLERY_ALBUM_COLUMNS[i];
        DataType type = GALLERY_ALBUM_COLUMN_TYPES[i];
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

int32_t AlbumDataConvertor::HandleRecordId(DriveKit::DKRecord &record, NativeRdb::ResultSet &resultSet)
{
    DriveKit::DKRecordData data;
    record.GetRecordData(data);
    string recordId = "";
    int32_t ret = GetString(PhotoAlbumColumns::ALBUM_CLOUD_ID, recordId, resultSet);
    if (ret != 0 && !recordId.empty()) {
        record.SetRecordId(recordId);
        return E_OK;
    }
    int32_t albumType = -1;
    std::string prefix = "album-";
    ret = GetInt(Media::PhotoAlbumColumns::ALBUM_TYPE, albumType, resultSet);
    if (ret == E_OK && albumType == AlbumType::SOURCE) {
        std::string bundleName = "";
        GetString(TMP_ALBUM_BUNDLE_NAME, bundleName, resultSet);
        std::replace(bundleName.begin(), bundleName.end(), '.', '-');
        record.SetRecordId(prefix + bundleName);
        return E_OK;
    }
    uuid_t uuid;
    uuid_generate(uuid);
    char str[UUID_STR_LENGTH] = {};
    uuid_unparse(uuid, str);
    std::string suffix(str);
    record.SetRecordId(prefix + suffix);
    return E_OK;
}

int32_t AlbumDataConvertor::HandleSourceAlbum(DriveKit::DKRecordData &data, ResultSet &resultSet)
{
    DriveKit::DKRecordFieldMap map;
    int32_t albumType = -1;
    if (GetInt(Media::PhotoAlbumColumns::ALBUM_TYPE, albumType, resultSet) == E_OK &&
        albumType == AlbumType::SOURCE) {
        data[ALBUM_PROPERTIES].GetRecordMap(map);
        RETURN_ON_ERR(HandleLocalLanguage(map, resultSet));
        RETURN_ON_ERR(HandleBundleName(map, resultSet));
    }
    data[ALBUM_PROPERTIES] = DriveKit::DKRecordField(map);
    return E_OK;
}

int32_t AlbumDataConvertor::ExtractBundleName(DriveKit::DKRecordFieldMap &map, NativeRdb::ValuesBucket &valueBucket)
{
    string bundleName = "";
    if (map[TMP_ALBUM_BUNDLE_NAME].GetString(bundleName) != DriveKit::DKLocalErrorCode::NO_ERROR) {
        return E_INVAL_ARG;
    }
    valueBucket.PutString(TMP_ALBUM_BUNDLE_NAME, bundleName);
    return E_OK;
}

int32_t AlbumDataConvertor::ExtractLocalLanguage(DriveKit::DKRecordFieldMap &map, NativeRdb::ValuesBucket &valueBucket)
{
    string localLanguage = "";
    if (map[TMP_ALBUM_LOCAL_LANGUAGE].GetString(localLanguage) != DriveKit::DKLocalErrorCode::NO_ERROR) {
        return E_INVAL_ARG;
    }
    valueBucket.PutString(TMP_ALBUM_LOCAL_LANGUAGE, localLanguage);
    return E_OK;
}

int32_t AlbumDataConvertor::Convert(DriveKit::DKRecord &record, NativeRdb::ValuesBucket &valueBucket)
{
    DriveKit::DKRecordData data;
    record.GetRecordData(data);

    if (data.find(ALBUM_PROPERTIES) == data.end()) {
        LOGE("record data donnot have properties set");
        return E_INVAL_ARG;
    }
    DriveKit::DKRecordFieldMap properties = data[ALBUM_PROPERTIES];

    auto size = GALLERY_ALBUM_COLUMNS.size();
    for (decltype(size) i = 0; i < size; i++) {
        auto field = GALLERY_ALBUM_COLUMNS[i];
        auto type = GALLERY_ALBUM_COLUMN_TYPES[i];
        if (properties.find(field) == properties.end()) {
            LOGE("filed %s not found in record.properties", field.c_str());
            return E_DATA;
        }
        int32_t ret = HandleField(properties[field], valueBucket, field, type);
        if (ret != E_OK) {
            LOGE("HandleField %s failed", field.c_str());
            return ret;
        }
    }
    return E_OK;
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
