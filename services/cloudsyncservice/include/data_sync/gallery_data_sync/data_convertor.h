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

#ifndef OHOS_CLOUD_SYNC_SERVICE_DATA_CONVERTOR_H
#define OHOS_CLOUD_SYNC_SERVICE_DATA_CONVERTOR_H

#include <memory>

#include "values_bucket.h"
#include "result_set.h"

#include "sdk_helper.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
enum class DataType : int32_t {
    INT,
    LONG,
    DOUBLE,
    STRING,
    BOOL
};

class DataConvertor {
public:
    DataConvertor() = default;
    virtual ~DataConvertor() = default;

    /* resultSet -> record */
    virtual int32_t Convert(DriveKit::DKRecord &record, NativeRdb::ResultSet &resultSet) = 0;
    virtual void HandleErr(int32_t err, NativeRdb::ResultSet &resultSet);
    /* record -> resultSet */
    virtual int32_t Convert(DriveKit::DKRecord &record, NativeRdb::ValuesBucket &valueBucket) = 0;

    int32_t ResultSetToRecords(const std::shared_ptr<NativeRdb::ResultSet> resultSet,
        std::vector<DriveKit::DKRecord> &records);
    int32_t RecordToValueBucket(DriveKit::DKRecord &record, NativeRdb::ValuesBucket &valueBucket);
    static int32_t GetLongComp(const DriveKit::DKRecordField &field, int64_t &val);
    static int32_t GetInt(const std::string &key, int32_t &val, NativeRdb::ResultSet &resultSet);
    static int32_t GetLong(const std::string &key, int64_t &val, NativeRdb::ResultSet &resultSet);
    static int32_t GetDouble(const std::string &key, double &val, NativeRdb::ResultSet &resultSet);
    static int32_t GetString(const std::string &key, std::string &val, NativeRdb::ResultSet &resultSet);
    static int32_t GetBool(const std::string &key, bool &val, NativeRdb::ResultSet &resultSet);
    static int32_t HandleField(const DriveKit::DKRecordField &value, NativeRdb::ValuesBucket &bucket,
        const std::string &field, DataType type);
};

#define SET_RECORD_INT(key, resultSet, record)    \
    do {    \
        int32_t val;    \
        int32_t ret = GetInt(key, val, resultSet);    \
        if (ret != E_OK) {    \
            return ret;    \
        }    \
        (record)[key] = DriveKit::DKRecordField(val);    \
    } while (0)

#define SET_RECORD_LONG(key, resultSet, record)    \
    do {    \
        int64_t val;    \
        int32_t ret = GetLong(key, val, resultSet);    \
        if (ret != E_OK) {    \
            return ret;    \
        }    \
        (record)[key] = DriveKit::DKRecordField(val);    \
    } while (0)

#define SET_RECORD_DOUBLE(key, resultSet, record)    \
    do {    \
        double val;    \
        int32_t ret = GetDouble(key, val, resultSet);    \
        if (ret != E_OK) {    \
            return ret;    \
        }    \
        (record)[key] = DriveKit::DKRecordField(val);    \
    } while (0)

#define SET_RECORD_STRING(key, resultSet, record)    \
    do {    \
        string val;    \
        int32_t ret = GetString(key, val, resultSet);    \
        if (ret != E_OK) {    \
            return ret;    \
        }    \
        (record)[key] = DriveKit::DKRecordField(val);    \
    } while (0)

#define SET_RECORD_BOOL(key, resultSet, record)    \
    do {    \
        bool val;    \
        int32_t ret = GetBool(key, val, resultSet);    \
        if (ret != E_OK) {    \
            return ret;    \
        }    \
        (record)[key] = DriveKit::DKRecordField(val);    \
    } while (0)

#define RETURN_ON_ERR(ret)    \
    do {    \
        if ((ret) != E_OK) {    \
            return ret;    \
        }    \
    } while (0)
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
#endif // OHOS_CLOUD_SYNC_SERVICE_DATA_CONVERTOR_H
