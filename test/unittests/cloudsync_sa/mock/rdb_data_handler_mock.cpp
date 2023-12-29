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

#include "rdb_data_handler.h"
#include <gtest/gtest.h>
#include <sstream>
#include "dfs_error.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
using namespace std;
using namespace NativeRdb;

int32_t RdbDataHandler::BatchInsert(int64_t &outRowId, const string &table,
    const vector<ValuesBucket> &initialBatchValues)
{
    return E_OK;
}

int32_t RdbDataHandler::BatchDetete(const string &whichTable,
                                    const string &whichColumn,
                                    const std::vector<NativeRdb::ValueObject> &bindArgs)
{
    return E_OK;
}

shared_ptr<NativeRdb::RdbStore> RdbDataHandler::GetRaw()
{
    return rdb_;
}

int32_t RdbDataHandler::BeginTransaction()
{
    return E_OK;
}

int32_t RdbDataHandler::RollBack()
{
    return E_OK;
}

int32_t RdbDataHandler::Commit()
{
    return E_OK;
}

int32_t RdbDataHandler::Insert(int64_t &outRowId, const ValuesBucket &initiavalues)
{
    return E_OK;
}

int32_t RdbDataHandler::Update(int &changedRows, const ValuesBucket &values,
    const string &whereClause, const vector<string> &whereArgs)
{
    return -1;
}

int32_t RdbDataHandler::Delete(int &deletedRows, const string &whereClause, const vector<string> &whereArgs)
{
    return E_OK;
}

shared_ptr<NativeRdb::ResultSet> RdbDataHandler::Query(
    const NativeRdb::AbsRdbPredicates &predicates, const std::vector<std::string> &columns)
{
    return nullptr;
}

int32_t RdbDataHandler::Insert(int64_t &outRowId, const std::string &tableName, const ValuesBucket &initiavalues)
{
    return E_OK;
}

int32_t RdbDataHandler::Update(int &changedRows, const std::string &tableName, const ValuesBucket &values,
    const string &whereClause, const vector<string> &whereArgs)
{
    return E_OK;
}

int32_t RdbDataHandler::Delete(int &deletedRows, const std::string &tableName,
    const string &whereClause, const vector<string> &whereArgs)
{
    return E_OK;
}

int32_t RdbDataHandler::ExecuteSql(const std::string &sql, const std::vector<NativeRdb::ValueObject> &bindArgs)
{
    return E_OK;
}

}
}
}