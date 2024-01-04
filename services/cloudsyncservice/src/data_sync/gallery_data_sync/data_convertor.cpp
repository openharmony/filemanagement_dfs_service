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

int32_t DataConvertor::ResultSetToRecords(const shared_ptr<NativeRdb::ResultSet> resultSet,
    vector<DriveKit::DKRecord> &records)
{
    int32_t rowCount = 0;
    int ret = resultSet->GetRowCount(rowCount);
    if (ret != NativeRdb::E_OK || rowCount < 0) {
        LOGE("result set get row count err %{public}d", ret);
        return E_RDB;
    }
    /* no match rows */
    if (rowCount == 0) {
        return E_STOP;
    }
    /* reserve to avoid repeatedly alloc and copy */
    records.reserve(rowCount);

    /* iterate all rows */
    while (resultSet->GoToNextRow() == NativeRdb::E_OK) {
        DriveKit::DKRecord record;
        ret = Convert(record, *resultSet);
        if (ret != E_OK) {
            LOGE("covert result to record err %{public}d", ret);
            HandleErr(ret, *resultSet);
            continue;
        }
        records.emplace_back(move(record));
    }

    return E_OK;
}

void DataConvertor::HandleErr(int32_t err, NativeRdb::ResultSet &resultSet)
{
}

int32_t DataConvertor::GetInt(const string &key, int32_t &val, NativeRdb::ResultSet &resultSet)
{
    int32_t index;
    int32_t err = resultSet.GetColumnIndex(key, index);
    if (err != NativeRdb::E_OK) {
        LOGE("result set get  %{public}s column index err %{public}d", key.c_str(), err);
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
        LOGE("result set get  %{public}s column index err %{public}d", key.c_str(), err);
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
        LOGE("result set get  %{public}s column index err %{public}d", key.c_str(), err);
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
        LOGE("result set get  %{public}s column index err %{public}d", key.c_str(), err);
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
        LOGE("result set get  %{public}s column index err %{public}d", key.c_str(), err);
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

static int32_t GetIntComp(const DriveKit::DKRecordField &field, int &val)
{
    if (field.GetInt(val) != DriveKit::DKLocalErrorCode::NO_ERROR) {
        string str;
        if (field.GetString(str) != DriveKit::DKLocalErrorCode::NO_ERROR) {
            LOGE("record filed bad type %{public}d", static_cast<int>(field.GetType()));
            return E_INVAL_ARG;
        }
        try {
            val = std::stoi(str);
        } catch (const std::out_of_range &e) {
            LOGE("record filed convert to int failed");
            return E_INVAL_ARG;
        }
        return E_OK;
    }
    return E_OK;
}

int32_t DataConvertor::GetLongComp(const DriveKit::DKRecordField &field, int64_t &val)
{
    if (field.GetLong(val) != DriveKit::DKLocalErrorCode::NO_ERROR) {
        string str;
        if (field.GetString(str) != DriveKit::DKLocalErrorCode::NO_ERROR) {
            LOGE("record filed bad type %{public}d", static_cast<int>(field.GetType()));
            return E_INVAL_ARG;
        }
        try {
            val = std::stoll(str);
        } catch (const std::out_of_range &e) {
            LOGE("record filed convert to int64 failed");
            return E_INVAL_ARG;
        }
        return E_OK;
    }
    return E_OK;
}

static int32_t GetDoubleComp(const DriveKit::DKRecordField &field, double &val)
{
    if (field.GetDouble(val) != DriveKit::DKLocalErrorCode::NO_ERROR) {
        string str;
        if (field.GetString(str) != DriveKit::DKLocalErrorCode::NO_ERROR) {
            LOGE("record filed bad type %{public}d", static_cast<int>(field.GetType()));
            return E_INVAL_ARG;
        }
        try {
            val = std::stod(str);
        } catch (const std::out_of_range &e) {
            LOGE("record filed convert to double failed");
            return E_INVAL_ARG;
        }
        return E_OK;
    }
    return E_OK;
}

static int32_t GetBoolComp(const DriveKit::DKRecordField &field, bool &val)
{
    if (field.GetBool(val) != DriveKit::DKLocalErrorCode::NO_ERROR) {
        string str;
        if (field.GetString(str) != DriveKit::DKLocalErrorCode::NO_ERROR) {
            LOGE("record filed bad type %{public}d", static_cast<int>(field.GetType()));
            return E_INVAL_ARG;
        }
        if (str == "true") {
            val = true;
        } else if (str == "false") {
            val = false;
        } else {
            return E_INVAL_ARG;
        }
        return E_OK;
    }
    return E_OK;
}

const std::unordered_map<DataType,
    std::function<int(const DriveKit::DKRecordField &, NativeRdb::ValuesBucket &, const std::string &)>>
    TYPE_HANDLERS = {
        {DataType::INT,
         [](const DriveKit::DKRecordField &value, NativeRdb::ValuesBucket &bucket, const std::string &field) {
             int intValue;
             if (GetIntComp(value, intValue) != E_OK) {
                 return E_INVAL_ARG;
             }
             bucket.PutInt(field, intValue);
             return E_OK;
         }},
        {DataType::LONG,
         [](const DriveKit::DKRecordField &value, NativeRdb::ValuesBucket &bucket, const std::string &field) {
             int64_t longValue;
             if (DataConvertor::GetLongComp(value, longValue) != E_OK) {
                 return E_INVAL_ARG;
             }
             bucket.PutLong(field, longValue);
             return E_OK;
         }},
        {DataType::STRING,
         [](const DriveKit::DKRecordField &value, NativeRdb::ValuesBucket &bucket, const std::string &field) {
             std::string stringValue;
             if (value.GetString(stringValue) != DriveKit::DKLocalErrorCode::NO_ERROR) {
                 return E_INVAL_ARG;
             }
             if (stringValue == "") {
                return E_OK;
             }
             bucket.PutString(field, stringValue);
             return E_OK;
         }},
        {DataType::DOUBLE,
         [](const DriveKit::DKRecordField &value, NativeRdb::ValuesBucket &bucket, const std::string &field) {
             double doubleValue;
             if (GetDoubleComp(value, doubleValue) != E_OK) {
                 return E_INVAL_ARG;
             }
             bucket.PutDouble(field, doubleValue);
             return E_OK;
         }},
        {DataType::BOOL,
         [](const DriveKit::DKRecordField &value, NativeRdb::ValuesBucket &bucket, const std::string &field) {
             bool boolValue;
             if (GetBoolComp(value, boolValue) != E_OK) {
                 return E_INVAL_ARG;
             }
             bucket.PutInt(field, static_cast<int>(boolValue));
             return E_OK;
         }}};

int DataConvertor::HandleField(const DriveKit::DKRecordField &value, NativeRdb::ValuesBucket &bucket,
    const std::string &field, DataType type)
{
    auto it = TYPE_HANDLERS.find(type);
    if (it == TYPE_HANDLERS.end()) {
        LOGE("invalid data type %d", static_cast<int>(type));
        return E_INVAL_ARG;
    }
    return it->second(value, bucket, field);
}

int32_t DataConvertor::RecordToValueBucket(DriveKit::DKRecord &record, NativeRdb::ValuesBucket &valueBucket)
{
    return Convert(record, valueBucket);
}

} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
