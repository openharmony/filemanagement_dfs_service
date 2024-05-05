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
#include "result_set.h"
#include "values_bucket.h"
#include "data_handler.h"
namespace OHOS {
namespace FileManagement {
namespace CloudSync {
class RdbDataHandler : public DataHandler {
protected:
    /* init */
    RdbDataHandler(int32_t userId, const std::string &bundleName, const std::string &table,
        std::shared_ptr<NativeRdb::RdbStore> rdb, std::shared_ptr<bool> stopFlag)
        : DataHandler(userId, bundleName, table),
          stopFlag_(stopFlag),
          rdb_(rdb),
          tableName_(table) {}
    virtual ~RdbDataHandler() = default;

    std::shared_ptr<NativeRdb::RdbStore> GetRaw();

    virtual int32_t BeginTransaction();
    virtual int32_t RollBack();
    virtual int32_t Commit();

    /* insert, delete, update, query */
    virtual int32_t BatchInsert(int64_t &outRowId,
                                const std::string &table,
                                const std::vector<NativeRdb::ValuesBucket> &initialBatchValues);
    virtual int32_t BatchDetete(const std::string &whichTable,
                                const std::string &whichColumn,
                                const std::vector<NativeRdb::ValueObject> &bindArgs);
    virtual int32_t BatchUpdate(const std::string &whichTable,
                                const std::string &whichColumn,
                                std::vector<NativeRdb::ValueObject> &bindArgs,
                                uint64_t &count);
    virtual int32_t Insert(int64_t &outRowId, const NativeRdb::ValuesBucket &initialValues);
    virtual int32_t Update(int &changedRows, const NativeRdb::ValuesBucket &values,
        const std::string &whereClause, const std::vector<std::string> &whereArgs);
    virtual int32_t Delete(int &deletedRows, const std::string &whereClause,
        const std::vector<std::string> &whereArgs);
    virtual std::shared_ptr<NativeRdb::ResultSet> Query(const NativeRdb::AbsRdbPredicates &predicates,
                                                        const std::vector<std::string> &columns);
    /* insert, delete, update with tableName */
    virtual int32_t Insert(int64_t &outRowId, const std::string &tableName,
        const NativeRdb::ValuesBucket &initialValues);
    virtual int32_t Update(int &changedRows, const std::string &tableName,
        const NativeRdb::ValuesBucket &values, const std::string &whereClause,
        const std::vector<std::string> &whereArgs);
    virtual int32_t Delete(int &deletedRows, const std::string &tableName,
        const std::string &whereClause, const std::vector<std::string> &whereArgs);

    /* raw */
    virtual int32_t ExecuteSql(const std::string &sql,
        const std::vector<NativeRdb::ValueObject> &bindArgs = std::vector<NativeRdb::ValueObject>());
    int32_t IsStop();

    static const int32_t BATCH_LIMIT_SIZE = 500;
    std::shared_ptr<bool> stopFlag_;

private:
    std::shared_ptr<NativeRdb::RdbStore> rdb_;
    std::string tableName_;
    std::mutex rdbMutex_;
};
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
#endif // OHOS_CLOUD_SYNC_SERVICE_RDB_DATA_HANDLER_H