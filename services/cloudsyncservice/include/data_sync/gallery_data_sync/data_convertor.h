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
class DataConvertor {
public:
    DataConvertor() = default;
    virtual ~DataConvertor() = default;

    virtual int32_t Convert(DriveKit::DKRecord &record, NativeRdb::ResultSet &resultSet) = 0;

    int32_t ResultSetToRecords(const std::unique_ptr<NativeRdb::ResultSet> resultSet,
        std::vector<DriveKit::DKRecord> &records);
    int32_t RecordToValueBucket(const DriveKit::DKRecord &record, NativeRdb::ValuesBucket &valueBucket);

protected:
    int32_t GetInt(const std::string &key, int32_t &val, NativeRdb::ResultSet &resultSet);
    int32_t GetLong(const std::string &key, int64_t &val, NativeRdb::ResultSet &resultSet);
    int32_t GetString(const std::string &key, std::string &val, NativeRdb::ResultSet &resultSet);
};
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
#endif // OHOS_CLOUD_SYNC_SERVICE_DATA_CONVERTOR_H
