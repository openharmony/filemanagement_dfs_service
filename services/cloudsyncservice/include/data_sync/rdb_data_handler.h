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

#ifndef OHOS_CLOUD_SYNC_SERVICE_RDB_DATA_HANDLER_H
#define OHOS_CLOUD_SYNC_SERVICE_RDB_DATA_HANDLER_H

#include "abs_rdb_predicates.h"
#include "rdb_helper.h"
#include "values_bucket.h"
#include "result_set.h"

#include "data_handler.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
class RdbDataHandler : public DataHandler {
protected:
    /* init */
    RdbDataHandler(const std::string &table, std::shared_ptr<NativeRdb::RdbStore> rdb)
        : rdb_(rdb),
          tableName_(table) {}
    virtual ~RdbDataHandler() = default;

    /* insert, delete, update, query */
    virtual int32_t Insert(int64_t &outRowId, const NativeRdb::ValuesBucket &initialValues);
    virtual int32_t Update(int &changedRows, const NativeRdb::ValuesBucket &values,
        const std::string &whereClause = "",
        const std::vector<std::string> &whereArgs = std::vector<std::string>());
    virtual int32_t Delete(int &deletedRows, const std::string &whereClause = "",
        const std::vector<std::string> &whereArgs = std::vector<std::string>());
    virtual std::unique_ptr<NativeRdb::ResultSet> Query(
        const NativeRdb::AbsRdbPredicates &predicates, const std::vector<std::string> &columns);

private:
    std::shared_ptr<NativeRdb::RdbStore> rdb_;
    std::string tableName_;
};
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
#endif // OHOS_CLOUD_SYNC_SERVICE_RDB_DATA_HANDLER_H