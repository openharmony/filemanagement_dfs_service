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

#include "data_convertor.h"

#include "rdb_errno.h"

#include "dfs_error.h"
#include "gallery_file_const.h"
#include "utils_log.h"
#include "sdk_helper.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
using namespace std;
using namespace NativeRdb;

int32_t DataConvertor::ResultSetToRecords(const unique_ptr<NativeRdb::ResultSet> resultSet,
    std::vector<DriveKit::DKRecord> &records)
{
    /* reserve to avoid repeatedly alloc and copy */
    int32_t rowCount = 0;
    int ret = resultSet->GetRowCount(rowCount);
    if (ret != 0) {
        LOGE("result set get row count err %{public}d", ret);
        return E_RDB;
    }
    records.reserve(rowCount);

    /* iterate all rows */
    while (resultSet->GoToNextRow() == 0) {
        DriveKit::DKRecord record;
        int32_t ret = Convert(record, *resultSet);
        if (ret != E_OK) {
            LOGE("covert result to record err %{public}d", ret);
            continue;
        }
        records.emplace_back(move(record));
    }

    return E_OK;
}

int32_t DataConvertor::GetInt(const string &key, int32_t &val, NativeRdb::ResultSet &resultSet)
{
    int32_t index;
    int32_t err = resultSet.GetColumnIndex(key, index);
    if (err != NativeRdb::E_OK) {
        LOGE("result set get column index err %{public}d", err);
        return E_RDB;
    }

    err = resultSet.GetInt(index, val);
    if (err != 0) {
        LOGE("result set get int err %{public}d", err);
        return E_RDB;
    }

    return E_OK;
}

int32_t DataConvertor::GetLong(const string &key, int64_t &val, NativeRdb::ResultSet &resultSet)
{
    int32_t index;
    int32_t err = resultSet.GetColumnIndex(key, index);
    if (err != NativeRdb::E_OK) {
        LOGE("result set get column index err %{public}d", err);
        return E_RDB;
    }

    err = resultSet.GetLong(index, val);
    if (err != 0) {
        LOGE("result set get int err %{public}d", err);
        return E_RDB;
    }

    return E_OK;
}

int32_t DataConvertor::GetDouble(const string &key, double &val, NativeRdb::ResultSet &resultSet)
{
    int32_t index;
    int32_t err = resultSet.GetColumnIndex(key, index);
    if (err != NativeRdb::E_OK) {
        LOGE("result set get column index err %{public}d", err);
        return E_RDB;
    }

    err = resultSet.GetDouble(index, val);
    if (err != 0) {
        LOGE("result set get double err %{public}d", err);
        return E_RDB;
    }

    return E_OK;
}

int32_t DataConvertor::GetString(const string &key, string &val, NativeRdb::ResultSet &resultSet)
{
    int32_t index;
    int32_t err = resultSet.GetColumnIndex(key, index);
    if (err != NativeRdb::E_OK) {
        LOGE("result set get column index err %{public}d", err);
        return E_RDB;
    }

    err = resultSet.GetString(index, val);
    if (err != 0) {
        LOGE("result set get string err %{public}d", err);
        return E_RDB;
    }

    return E_OK;
}

int32_t DataConvertor::GetBool(const string &key, bool &val, NativeRdb::ResultSet &resultSet)
{
    int32_t index;
    int32_t err = resultSet.GetColumnIndex(key, index);
    if (err != NativeRdb::E_OK) {
        LOGE("result set get column index err %{public}d", err);
        return E_RDB;
    }

    int32_t tmpValue;
    err = resultSet.GetInt(index, tmpValue);
    if (err != 0) {
        LOGE("result set get bool err %{public}d", err);
        return E_RDB;
    }
    val = !!tmpValue;

    return E_OK;
}

int32_t DataConvertor::RecordToValueBucket(const DriveKit::DKRecord &record,
    NativeRdb::ValuesBucket &valueBucket)
{
    DriveKit::DKRecordData data;
    record.GetRecordData(data);

    if (data.find(FILE_PROPERTIES) == data.end()) {
        LOGE("record data donnot have properties set");
        return E_INVAL_ARG;
    }
    DriveKit::DKRecordFieldMap properties = data[FILE_PROPERTIES];

    auto size = GALLERY_FILE_COLUMNS.size();
    for (int i = 0 ; i < size; i++) {
        auto field = GALLERY_FILE_COLUMNS[i];
        auto type = GALLERY_FILE_COLUMN_TYPES[i];
        if (properties.find(field) == properties.end()) {
            LOGE("filed %{public}s not found in record.properties", field.c_str());
            return E_INVAL_ARG;
        }
        switch (type) {
            case DataType::INT: {
                int value = properties[field];
                valueBucket.PutInt(field, value);
                break;
            }
            case DataType::LONG: {
                int64_t value = properties[field];
                valueBucket.PutInt(field, value);
                break;
            }
            case DataType::STRING: {
                string value = properties[field];
                valueBucket.PutString(field, value);
                break;
            }
            case DataType::BOOL: {
                bool value = properties[field];
                valueBucket.PutInt(field, value);
                break;
            }
            case DataType::DOUBLE: {
                double value = properties[field];
                valueBucket.PutDouble(field, value);
                break;
            }
            default: {
                LOGE("invalid data type %{public}d", static_cast<int>(type));
                break;
            }
        }
    }
    return E_OK;
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
