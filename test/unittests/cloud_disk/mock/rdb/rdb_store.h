/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#ifndef SECURITY_GUARD_RDB_STORE_MOCK_H
#define SECURITY_GUARD_RDB_STORE_MOCK_H

#include <cstdint>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "result_set.h"
#include "values_bucket.h"
#include "rdb_predicates.h"

namespace OHOS::NativeRdb {
constexpr int E_ERROR = -1;
constexpr int E_OK = 0;

class RdbStoreInterface {
public:
    virtual ~RdbStoreInterface() = default;
    virtual int Insert(int64_t &outRowId, const std::string &table, const NativeRdb::ValuesBucket &initialValues) = 0;
    virtual int BatchInsert(int64_t &outInsertNum, const std::string &table,
        const std::vector<NativeRdb::ValuesBucket> &initialBatchValues) = 0;
    virtual int Update(int &changedRows, const NativeRdb::ValuesBucket &values,
        const NativeRdb::AbsRdbPredicates &predicates) = 0;
    virtual int Delete(int &deletedRows, const NativeRdb::AbsRdbPredicates &predicates) = 0;
    virtual std::shared_ptr<NativeRdb::ResultSet> Query(
        const NativeRdb::AbsRdbPredicates &predicates, const std::vector<std::string> columns) = 0;
    virtual int ExecuteSql(const std::string &sql, const std::vector<ValueObject> &bindArgs) = 0;
    virtual int ExecuteSql(const std::string &sql) = 0;
    virtual int ExecuteAndGetLong(int64_t &outValue, const std::string &sql,
        const std::vector<NativeRdb::ValueObject> &bindArgs = std::vector<NativeRdb::ValueObject>()) = 0;
    virtual int Count(int64_t &outValue, const NativeRdb::AbsRdbPredicates &predicates) = 0;
    virtual int BeginTransaction() = 0;
    virtual int RollBack() = 0;
    virtual int Commit() = 0;
    virtual int Attach(const std::string &alias, const std::string &pathName,
        const std::vector<uint8_t> destEncryptKey) = 0;
};

class RdbStore : public RdbStoreInterface {
public:
    RdbStore() = default;
    ~RdbStore() override = default;
    MOCK_METHOD3(Insert, int(int64_t &outRowId, const std::string &table, const ValuesBucket &initialValues));
    MOCK_METHOD3(BatchInsert, int(int64_t &outInsertNum, const std::string &table,
        const std::vector<ValuesBucket> &initialBatchValues));
    MOCK_METHOD2(ExecuteSql, int(
        const std::string &sql, const std::vector<ValueObject> &bindArgs));
    MOCK_METHOD1(ExecuteSql, int(const std::string &sql));
    MOCK_METHOD3(ExecuteAndGetLong, int(int64_t &outValue, const std::string &sql,
        const std::vector<ValueObject> &bindArgs));
    MOCK_METHOD3(Attach, int(
        const std::string &alias, const std::string &pathName, const std::vector<uint8_t> destEncryptKey));
    MOCK_METHOD2(Count, int(int64_t &outValue, const AbsRdbPredicates &predicates));
    MOCK_METHOD2(Query, std::shared_ptr<ResultSet>(
        const AbsRdbPredicates &predicates, const std::vector<std::string> columns));
    MOCK_METHOD3(Update, int(int &changedRows, const ValuesBucket &values, const AbsRdbPredicates &predicates));
    MOCK_METHOD2(Delete, int(int &deletedRows, const AbsRdbPredicates &predicates));
    MOCK_METHOD0(BeginTransaction, int());
    MOCK_METHOD0(RollBack, int());
    MOCK_METHOD0(Commit, int());
};
} // namespace OHOS::NativeRdb
#endif // SECURITY_GUARD_RDB_STORE_MOCK_H