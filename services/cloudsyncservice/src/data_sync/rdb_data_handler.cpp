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

#include "rdb_data_handler.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
using namespace std;
using namespace NativeRdb;

int32_t RdbDataHandler::BatchInsert(int64_t &outRowId, const string &table,
    const vector<ValuesBucket> &initialBatchValues)
{
    if (initialBatchValues.size() != 0) {
        return rdb_->BatchInsert(outRowId, table, initialBatchValues);
    }
    return E_OK;
}

int32_t RdbDataHandler::BeginTransaction()
{
    return rdb_->BeginTransaction();
}

int32_t RdbDataHandler::RollBack()
{
    return rdb_->RollBack();
}

int32_t RdbDataHandler::Commit()
{
    return rdb_->Commit();
}

int32_t RdbDataHandler::Insert(int64_t &outRowId, const ValuesBucket &initiavalues)
{
    return rdb_->Insert(outRowId, tableName_, initiavalues);
}

int32_t RdbDataHandler::Update(int &changedRows, const ValuesBucket &values,
    const string &whereClause, const vector<string> &whereArgs)
{
    return rdb_->Update(changedRows, tableName_, values, whereClause, whereArgs);
}

int32_t RdbDataHandler::Delete(int &deletedRows, const string &whereClause, const vector<string> &whereArgs)
{
    return rdb_->Delete(deletedRows, tableName_, whereClause, whereArgs);
}

shared_ptr<NativeRdb::ResultSet> RdbDataHandler::Query(
    const NativeRdb::AbsRdbPredicates &predicates, const std::vector<std::string> &columns)
{
    return rdb_->Query(predicates, columns);
}

int32_t RdbDataHandler::Insert(int64_t &outRowId, const std::string &tableName, const ValuesBucket &initiavalues)
{
    return rdb_->Insert(outRowId, tableName, initiavalues);
}

int32_t RdbDataHandler::Update(int &changedRows, const std::string &tableName, const ValuesBucket &values,
    const string &whereClause, const vector<string> &whereArgs)
{
    return rdb_->Update(changedRows, tableName, values, whereClause, whereArgs);
}

int32_t RdbDataHandler::Delete(int &deletedRows, const std::string &tableName,
    const string &whereClause, const vector<string> &whereArgs)
{
    return rdb_->Delete(deletedRows, tableName, whereClause, whereArgs);
}

int32_t RdbDataHandler::ExecuteSql(const std::string &sql, const std::vector<NativeRdb::ValueObject> &bindArgs)
{
    return rdb_->ExecuteSql(sql, bindArgs);
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS