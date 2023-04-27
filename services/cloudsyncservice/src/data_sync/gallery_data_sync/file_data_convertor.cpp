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
string FileDataConvertor::realPrefix_ = "/data/service/el2/";
string FileDataConvertor::suffix_ = "/hmdfs/account/files";
string FileDataConvertor::sandboxPrefix_ = "/storage/media/local/files";

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
};

FileDataConvertor::FileDataConvertor(int32_t userId, string &bundleName, bool isNew) : userId_(userId),
    bundleName_(bundleName), isNew_(isNew)
{
}

int32_t FileDataConvertor::Convert(DriveKit::DKRecord &record, NativeRdb::ResultSet &resultSet)
{
    LOGI("covert userId %{private}d, bundle %{private}s", userId_, bundleName_.c_str());
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

    if (isNew_) {
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

/* record id */
int32_t FileDataConvertor::FillRecordId(DriveKit::DKRecord &record, NativeRdb::ResultSet &resultSet)
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
    int32_t ret = GetInt(MEDIA_DATA_DB_NAME, val, resultSet);
    if (ret != E_OK) {
        return ret;
    }
    data[key] = DriveKit::DKRecordField(!!val);
    return E_OK;
}

int32_t FileDataConvertor::HandleDescription(string &key, DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    data[key] = DriveKit::DKRecordField("");
    return E_OK;
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
