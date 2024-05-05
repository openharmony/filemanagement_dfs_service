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
#include <sstream>
#include "dfs_error.h"
#include "data_convertor.h"
#include "unistd.h"
namespace OHOS {
namespace FileManagement {
namespace CloudSync {
using namespace std;
using namespace NativeRdb;

static int32_t Execute(std::function<int32_t()> func)
{
    const uint32_t TRY_TIMES = 5;
    const uint32_t SLEEP_TIME = 1;
    int32_t ret = E_OK;
    for (uint32_t i = 0; i < TRY_TIMES; i++) {
        ret = func();
        if (ret == E_OK) {
            return ret;
        }
        sleep(SLEEP_TIME);
        LOGW("retry time is %{public}d", i);
    }
    LOGE("func execute fail, try too many times");
    return ret;
}

int32_t RdbDataHandler::BatchInsert(int64_t &outRowId, const string &table,
                                    const vector<ValuesBucket> &initialBatchValues)
{
    RETURN_ON_ERR(IsStop());
    const uint32_t TRY_TIMES = 5;
    const uint32_t SLEEP_TIME = 200 * 1000;
    int32_t ret = E_OK;
    uint32_t tryCount = 0;
    if (initialBatchValues.size() == 0) {
        return ret;
    }
    while (tryCount <= TRY_TIMES) {
        usleep(SLEEP_TIME * tryCount);
        {
            std::lock_guard<std::mutex> lock(rdbMutex_);
            ret = rdb_->BatchInsert(outRowId, table, initialBatchValues);
        }
        if (ret != 0) {
            LOGW("batch insert fail try next time, retry time is tryCount %{public}d", tryCount);
            tryCount++;
        } else {
            return ret;
        }
    }
    LOGE("batch insert fail, try too many times");
    return ret;
}

int32_t RdbDataHandler::BatchDetete(const string &whichTable,
                                    const string &whichColumn,
                                    const std::vector<NativeRdb::ValueObject> &bindArgs)
{
    if (bindArgs.size() > BATCH_LIMIT_SIZE || bindArgs.size() < 0) {
        return E_INVAL_ARG;
    }
    std::stringstream ss;
    for (unsigned int i = 0; i < bindArgs.size(); i++) {
        if (ss.tellp() != 0) {
            ss << ",";
        }
        ss <<" ? ";
    }
    string sql = "DELETE FROM " + whichTable + " WHERE " + whichColumn + " IN (" + ss.str() + ")";
    std::lock_guard<std::mutex> lock(rdbMutex_);
    std::function<int32_t()> func = [this, sql, &bindArgs] { return rdb_->ExecuteSql(sql, bindArgs); };
    return Execute(func);
}

int32_t RdbDataHandler::BatchUpdate(const string &sql,
                                    const string &whichColumn,
                                    std::vector<NativeRdb::ValueObject> &bindArgs,
                                    uint64_t &count)
{
    int32_t ret = E_OK;
    if (bindArgs.size() < 0) {
        return E_INVALID_ARGS;
    }
    do {
        uint32_t size = bindArgs.size() > BATCH_LIMIT_SIZE ? BATCH_LIMIT_SIZE : bindArgs.size();
        std::stringstream ss;
        for (unsigned int i = 0; i < size; i++) {
            if (ss.tellp() != 0) {
                ss << " ,";
            }
            ss<<" ?";
        }
        vector<ValueObject> tmp(bindArgs.begin(), bindArgs.begin() + size);
        string newSql = sql + " WHERE " + whichColumn + " IN (" + ss.str() + ")";
        {
            std::lock_guard<std::mutex> lock(rdbMutex_);
            ret = rdb_->ExecuteSql(newSql, tmp);
        }
        if (ret == E_OK) {
            bindArgs.erase(bindArgs.begin(), bindArgs.begin() + size);
            count += size;
        } else {
            LOGW("update fail try next time, ret is %{public}d", ret);
            return ret;
        }
    } while (bindArgs.size() > BATCH_LIMIT_SIZE);
    return E_OK;
}

shared_ptr<NativeRdb::RdbStore> RdbDataHandler::GetRaw()
{
    return rdb_;
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
    RETURN_ON_ERR(IsStop());
    std::lock_guard<std::mutex> lock(rdbMutex_);
    std::function<int32_t()> func = [this, &outRowId, &initiavalues] {
        return rdb_->Insert(outRowId, this->tableName_, initiavalues);
    };
    return Execute(func);
}

int32_t RdbDataHandler::Update(int &changedRows, const ValuesBucket &values,
    const string &whereClause, const vector<string> &whereArgs)
{
    std::lock_guard<std::mutex> lock(rdbMutex_);
    std::function<int32_t()> func = [this, &changedRows, &values, &whereClause, &whereArgs] {
        return rdb_->Update(changedRows, this->tableName_, values, whereClause, whereArgs);
    };
    return Execute(func);
}

int32_t RdbDataHandler::Delete(int &deletedRows, const string &whereClause, const vector<string> &whereArgs)
{
    std::lock_guard<std::mutex> lock(rdbMutex_);
    std::function<int32_t()> func = [this, &deletedRows, &whereClause, &whereArgs] {
        return rdb_->Delete(deletedRows, this->tableName_, whereClause, whereArgs);
    };
    return Execute(func);
}

shared_ptr<NativeRdb::ResultSet> RdbDataHandler::Query(
    const NativeRdb::AbsRdbPredicates &predicates, const std::vector<std::string> &columns)
{
    return rdb_->Query(predicates, columns);
}

int32_t RdbDataHandler::Insert(int64_t &outRowId, const std::string &tableName, const ValuesBucket &initiavalues)
{
    RETURN_ON_ERR(IsStop());
    std::function<int32_t()> func = [this, &outRowId, &tableName, &initiavalues] {
        std::lock_guard<std::mutex> lock(rdbMutex_);
        return rdb_->Insert(outRowId, tableName, initiavalues);
    };
    return Execute(func);
}

int32_t RdbDataHandler::Update(int &changedRows, const std::string &tableName, const ValuesBucket &values,
    const string &whereClause, const vector<string> &whereArgs)
{
    std::function<int32_t()> func = [this, &changedRows, &tableName, &values, &whereClause, &whereArgs] {
        return rdb_->Update(changedRows, tableName, values, whereClause, whereArgs);
    };
    return Execute(func);
}

int32_t RdbDataHandler::Delete(int &deletedRows, const std::string &tableName,
    const string &whereClause, const vector<string> &whereArgs)
{
    std::function<int32_t()> func = [this, &deletedRows, &tableName, &whereClause, &whereArgs] {
        return rdb_->Delete(deletedRows, tableName, whereClause, whereArgs);
    };
    return Execute(func);
}

int32_t RdbDataHandler::ExecuteSql(const std::string &sql, const std::vector<NativeRdb::ValueObject> &bindArgs)
{
    std::function<int32_t()> func = [this, &sql, &bindArgs] { return rdb_->ExecuteSql(sql, bindArgs); };
    int32_t ret = Execute(func);
    if (ret != E_OK) {
        LOGE("err sql is %{public}s", sql.c_str());
    }
    return ret;
}

int32_t RdbDataHandler::IsStop()
{
    if (stopFlag_ == nullptr) {
        stopFlag_ = make_shared<bool>(false);
    }
    if (*stopFlag_) {
        return E_STOP;
    }
    return E_OK;
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS