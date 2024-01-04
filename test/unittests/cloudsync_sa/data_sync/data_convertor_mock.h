/*
 * Copyright (C) 2023 Huawei Device Co., Ltd. All rights reserved.
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

#ifndef OHOS_CLOUD_SYNC_SA_DATA_CONVERTOR_MOCK_H
#define OHOS_CLOUD_SYNC_SA_DATA_CONVERTOR_MOCK_H

#include "abs_rdb_predicates.h"
#include "rdb_helper.h"
#include "result_set.h"
#include "value_object.h"
#include "values_bucket.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using namespace NativeRdb;

class DataConvertorMock final : public DataConvertor {
public:
    DataConvertorMock() = default;
    virtual ~DataConvertorMock() = default;

    int32_t Convert(DriveKit::DKRecord &record, NativeRdb::ResultSet &resultSet)
    {
        return E_OK;
    }
    int32_t Convert(DriveKit::DKRecord &record, NativeRdb::ValuesBucket &valueBucket)
    {
        return E_OK;
    }

    MOCK_METHOD2(HandleErr, void(int32_t, NativeRdb::ResultSet &resultSet));
    MOCK_METHOD2(ResultSetToRecords, int32_t(const std::shared_ptr<NativeRdb::ResultSet> resultSet,
        std::vector<DriveKit::DKRecord> &records));
    MOCK_METHOD2(RecordToValueBucket, int32_t(DriveKit::DKRecord &record, NativeRdb::ValuesBucket &valueBucket));
    MOCK_METHOD2(GetLongComp, int32_t(const DriveKit::DKRecordField &field, int64_t &val));
    MOCK_METHOD3(GetInt, int32_t(const std::string &key, int32_t &val, NativeRdb::ResultSet &resultSet));
    MOCK_METHOD3(GetLong, int32_t(const std::string &key, int64_t &val, NativeRdb::ResultSet &resultSet));
    MOCK_METHOD3(GetDouble, int32_t(const std::string &key, double &val, NativeRdb::ResultSet &resultSet));
    MOCK_METHOD3(GetString, int32_t(const std::string &key, std::string &val, NativeRdb::ResultSet &resultSet));
    MOCK_METHOD3(GetBool, int32_t(const std::string &key, bool &val, NativeRdb::ResultSet &resultSet));
    MOCK_METHOD4(HandleField,
                 int32_t(const DriveKit::DKRecordField &value,
                         NativeRdb::ValuesBucket &bucket,
                         const std::string &field,
                         DataType type));
};

} // namespace OHOS::FileManagement::CloudSync::Test
#endif