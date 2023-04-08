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
enum DataType : int32_t {
    INT,
    LONG,
    STRING,
    DOUBLE,
    BOOL,
    BLOB,
    ASSET
};

class DataConvertor {
public:
    DataConvertor(const std::vector<std::string> localColumns,
        const std::vector<std::string> cloudColumns,
        const std::vector<DataType> types, int32_t size);
    virtual ~DataConvertor() = default;

    int32_t ResultSetToRecords(const std::unique_ptr<NativeRdb::ResultSet> resultSet,
        std::vector<DriveKit::DKRecord> &records);
    int32_t RecordToValueBucket(const DriveKit::DKRecord &record,
        NativeRdb::ValuesBucket &valueBucket);
    int32_t RecordsToValueBuckets(const std::vector<DriveKit::DKRecord> &records,
        std::vector<NativeRdb::ValuesBucket> &valueBuckets);

    const std::vector<std::string> &GetLocalColumns() const
    {
        return localColumns_;
    }

    const std::vector<std::string> &GetCloudColumns() const
    {
        return cloudColumns_;
    }

    const std::vector<DataType> &GetTypes() const
    {
        return types_;
    }

    int32_t GetSize() const
    {
        return size_;
    }

private:
    void HandleInt(DriveKit::DKRecordDatas &data, const DriveKit::DKFieldKey &key,
        int32_t index, NativeRdb::ResultSet &resultSet);
    void HandleLong(DriveKit::DKRecordDatas &data, const DriveKit::DKFieldKey &key,
        int32_t index, NativeRdb::ResultSet &resultSet);
    void HandleString(DriveKit::DKRecordDatas &data, const DriveKit::DKFieldKey &key,
        int32_t index, NativeRdb::ResultSet &resultSet);
    void HandleAsset(DriveKit::DKRecordDatas &data, const DriveKit::DKFieldKey &key,
        int32_t index, NativeRdb::ResultSet &resultSet);

    /* handler map */
    using ConvertorHandler = void (DataConvertor::*)(DriveKit::DKRecordDatas &data,
        const DriveKit::DKFieldKey &key, int32_t index, NativeRdb::ResultSet &resultSet);
    std::map<int32_t, ConvertorHandler> opToHandlerMap_;

    const std::vector<std::string> localColumns_;
    const std::vector<std::string> cloudColumns_;
    const std::vector<DataType> types_;
    const int32_t size_;
};
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
#endif // OHOS_CLOUD_SYNC_SERVICE_DATA_CONVERTOR_H