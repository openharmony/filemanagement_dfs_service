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

#include "dfs_error.h"
#include "utils_log.h"
#include "sdk_helper.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
using namespace std;
using namespace NativeRdb;

DataConvertor::DataConvertor(const vector<std::string> localColumns,
    const vector<std::string> cloudColumns,
    const vector<DataType> types, int32_t size)
    : localColumns_(localColumns),
      cloudColumns_(cloudColumns),
      types_(types),
      size_(size)
{
    opToHandlerMap_[INT] = &DataConvertor::HandleInt;
    opToHandlerMap_[LONG] = &DataConvertor::HandleLong;
    opToHandlerMap_[STRING] = &DataConvertor::HandleString;
    opToHandlerMap_[ASSET] = &DataConvertor::HandleAsset;
}

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

    auto size = this->GetSize();
    auto &types = this->GetTypes();
    auto &cloudColumns = this->GetCloudColumns();

    /* iterate all rows */
    while (resultSet->GoToNextRow() == 0) {
        DriveKit::DKRecord record;
        DriveKit::DKRecordDatas data;
        for (int i = 0; i < size; i++) {
            DataType type = types[i];
            auto entry = opToHandlerMap_.find(type);
            if (entry == opToHandlerMap_.end() || !entry->second) {
                LOGE("invalid type %d", type);
                return E_INVAL_ARG;
            }
            (this->*(entry->second))(data, cloudColumns[i], i, *resultSet);
        }
        record.SetRecordDatas(data);
        records.emplace_back(move(record));
    }

    return E_OK;
}

void DataConvertor::HandleInt(DriveKit::DKRecordDatas &data, const DriveKit::DKFieldKey &key,
    int32_t index, NativeRdb::ResultSet &resultSet)
{
    int32_t val;
    int32_t err = resultSet.GetInt(index, val);
    if (err != 0) {
        LOGE("result set get int err %{public}d", err);
    }
    data[key] = DriveKit::DKRecordField(val);
}

void DataConvertor::HandleLong(DriveKit::DKRecordDatas &data, const DriveKit::DKFieldKey &key,
    int32_t index, NativeRdb::ResultSet &resultSet)
{
    int64_t val;
    int32_t err = resultSet.GetLong(index, val);
    if (err != 0) {
        LOGE("result set get int err %{public}d", err);
    }
    data[key] = DriveKit::DKRecordField(val);
}

void DataConvertor::HandleString(DriveKit::DKRecordDatas &data, const DriveKit::DKFieldKey &key,
    int32_t index, NativeRdb::ResultSet &resultSet)
{
    string val;
    int32_t err = resultSet.GetString(index, val);
    if (err != 0) {
        LOGE("result set get string err %{public}d", err);
    }
    data[key] = DriveKit::DKRecordField(val);
}

void DataConvertor::HandleAsset(DriveKit::DKRecordDatas &data, const DriveKit::DKFieldKey &key,
    int32_t index, NativeRdb::ResultSet &resultSet)
{
    string val;
    int32_t err = resultSet.GetString(index, val);
    if (err != 0) {
        LOGE("result set get string err %{public}d", err);
    }
    data[key] = DriveKit::DKRecordField(val);
}

int32_t DataConvertor::RecordToValueBucket(const DriveKit::DKRecord &record,
    NativeRdb::ValuesBucket &valueBucket)
{
    auto size = this->GetSize();
    auto &types = this->GetTypes();
    auto &localColumns = this->GetLocalColumns();
    auto &cloudColumns = this->GetCloudColumns();

    DriveKit::DKRecordDatas data;
    record.GetRecordDatas(data);
    for (int32_t j = 0; j < size; j++) {
        DataType type = types[j];
        switch (type) {
            case INT: {
                valueBucket.PutInt(localColumns[j], data[cloudColumns[j]]);
                break;
            }
            case LONG: {
                valueBucket.PutLong(localColumns[j], data[cloudColumns[j]]);
                break;
            }
            case STRING: {
                valueBucket.PutString(localColumns[j], data[cloudColumns[j]]);
                break;
            }
            default: {
                LOGE("invalid data type %{public}d", type);
                break;
            }
        }
    }

    return E_OK;
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS