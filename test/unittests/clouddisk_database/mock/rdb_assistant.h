/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_CLOUD_SYNC_SA_RDB_STORE_MOCK_H
#define OHOS_CLOUD_SYNC_SA_RDB_STORE_MOCK_H

#include "abs_rdb_predicates.h"
#include "rdb_helper.h"
#include "result_set.h"
#include "value_object.h"
#include "values_bucket.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace OHOS::FileManagement::CloudDisk {
using namespace std;
using namespace NativeRdb;


class RdbStoreMock final : public NativeRdb::RdbStore {
public:
    MOCK_METHOD3(Insert, int(int64_t &outRowId, const std::string &table, const ValuesBucket &initialValues));
    MOCK_METHOD3(BatchInsert,
                 int(int64_t &outInsertNum,
                     const std::string &table,
                     const std::vector<ValuesBucket> &initialBatchValues));
    MOCK_METHOD3(Replace, int(int64_t &outRowId, const std::string &table, const ValuesBucket &initialValues));
    MOCK_METHOD4(InsertWithConflictResolution,
                 int(int64_t &outRowId,
                     const std::string &table,
                     const ValuesBucket &initialValues,
                     ConflictResolution conflictResolution));
    MOCK_METHOD5(Update,
                 int(int &changedRows,
                     const std::string &table,
                     const ValuesBucket &values,
                     const std::string &whereClause,
                     const std::vector<std::string> &whereArgs));
    MOCK_METHOD5(Update,
                 int(int &changedRows,
                     const std::string &table,
                     const ValuesBucket &values,
                     const std::string &whereClause,
                     const std::vector<ValueObject> &bindArgs));

    MOCK_METHOD6(UpdateWithConflictResolution,
                 int(int &changedRows,
                     const std::string &table,
                     const ValuesBucket &values,
                     const std::string &whereClause,
                     const std::vector<std::string> &whereArgs,
                     ConflictResolution conflictResolution));

    MOCK_METHOD6(UpdateWithConflictResolution,
                 int(int &changedRows,
                     const std::string &table,
                     const ValuesBucket &values,
                     const std::string &whereClause,
                     const std::vector<ValueObject> &bindArgs,
                     ConflictResolution conflictResolution));

    MOCK_METHOD4(Delete,
                 int(int &deletedRows,
                     const std::string &table,
                     const std::string &whereClause,
                     const std::vector<std::string> &whereArgs));

    MOCK_METHOD4(Delete,
                 int(int &deletedRows,
                     const std::string &table,
                     const std::string &whereClause,
                     const std::vector<ValueObject> &bindArgs));

    MOCK_METHOD2(ExecuteSql, int(const std::string &sql, const std::vector<ValueObject> &bindArgs));

    MOCK_METHOD3(ExecuteAndGetLong,
                 int(int64_t &outValue, const std::string &sql, const std::vector<ValueObject> &bindArgs));

    MOCK_METHOD3(ExecuteAndGetString,
                 int(std::string &outValue, const std::string &sql, const std::vector<ValueObject> &bindArgs));

    MOCK_METHOD3(ExecuteForLastInsertedRowId,
                 int(int64_t &outValue, const std::string &sql, const std::vector<ValueObject> &bindArgs));

    MOCK_METHOD3(ExecuteForChangedRowCount,
                 int(int64_t &outValue, const std::string &sql, const std::vector<ValueObject> &bindArgs));

    MOCK_METHOD2(Backup, int(const std::string &databasePath, const std::vector<uint8_t> &destEncryptKey));

    MOCK_METHOD3(Attach,
                 int(const std::string &alias, const std::string &pathName, const std::vector<uint8_t> destEncryptKey));

    MOCK_METHOD2(Count, int(int64_t &outValue, const AbsRdbPredicates &predicates));
    MOCK_METHOD2(Query,
                 std::shared_ptr<AbsSharedResultSet>(const AbsRdbPredicates &predicates,
                                                     const std::vector<std::string> &columns));
    MOCK_METHOD11(Query,
                  std::shared_ptr<AbsSharedResultSet>(int &errCode,
                                                       bool distinct,
                                                       const std::string &table,
                                                       const std::vector<std::string> &columns,
                                                       const std::string &whereClause,
                                                       const std::vector<ValueObject> &bindArgs,
                                                       const std::string &groupBy,
                                                       const std::string &indexName,
                                                       const std::string &orderBy,
                                                       const int &limit,
                                                       const int &offset));
    MOCK_METHOD2(QuerySql,
                 std::shared_ptr<AbsSharedResultSet>(const std::string &sql,
                                                     const std::vector<std::string> &selectionArgs));
    MOCK_METHOD2(QuerySql,
                 std::shared_ptr<AbsSharedResultSet>(const std::string &sql,
                                                     const std::vector<ValueObject> &bindArgs));
    MOCK_METHOD2(QueryByStep,
                 std::shared_ptr<ResultSet>(const std::string &sql, const std::vector<std::string> &selectionArgs));
    MOCK_METHOD3(QueryByStep,
                 std::shared_ptr<ResultSet>(const std::string &sql, const std::vector<ValueObject> &bindArgs,
                                            bool preCount));

    MOCK_METHOD3(QueryByStep,
                 std::shared_ptr<ResultSet>(const AbsRdbPredicates &predicates,
                                            const std::vector<std::string> &columns, bool preCount));

    MOCK_METHOD4(RemoteQuery,
                 std::shared_ptr<ResultSet>(const std::string &device,
                                            const AbsRdbPredicates &predicates,
                                            const std::vector<std::string> &columns,
                                            int &errCode));

    MOCK_METHOD3(GetModifyTime, ModifyTime(const std::string &table, const std::string &columnName,
                                                       std::vector<PRIKey> &keys));

    MOCK_METHOD3(SetDistributedTables, int(const std::vector<std::string> &tables, int32_t type,
                                           const DistributedRdb::DistributedConfig &distributedConfig));
    MOCK_METHOD3(ObtainDistributedTableName,
                 std::string(const std::string &device, const std::string &table, int &errCode));
    MOCK_METHOD3(Sync, int(const SyncOption &option, const AbsRdbPredicates &predicate, const AsyncBrief& async));
    MOCK_METHOD3(Sync, int(const SyncOption &option, const AbsRdbPredicates &predicate, const AsyncDetail& async));
    MOCK_METHOD3(Sync, int(const SyncOption &option, const std::vector<std::string>& tables, const AsyncDetail& async));
    MOCK_METHOD2(Subscribe, int(const SubscribeOption &option, std::shared_ptr<RdbStoreObserver> observer));
    MOCK_METHOD2(UnSubscribe, int(const SubscribeOption &option, std::shared_ptr<RdbStoreObserver> observer));
    MOCK_METHOD1(RegisterAutoSyncCallback, int(std::shared_ptr<DetailProgressObserver> syncObserver));
    MOCK_METHOD1(UnregisterAutoSyncCallback, int(std::shared_ptr<DetailProgressObserver> syncObserver));
    MOCK_METHOD2(CleanDirtyData, int(const std::string &table, uint64_t cursor));
    MOCK_METHOD1(Notify, int(const std::string &event));
    MOCK_METHOD2(DropDeviceData, bool(const std::vector<std::string> &devices, const DropOption &option));
    MOCK_METHOD3(Update, int(int &changedRows, const ValuesBucket &values, const AbsRdbPredicates &predicates));
    MOCK_METHOD2(Delete, int(int &deletedRows, const AbsRdbPredicates &predicates));
    MOCK_METHOD1(GetVersion, int(int &version));
    MOCK_METHOD1(SetVersion, int(int version));
    MOCK_METHOD0(BeginTransaction, int());
    MOCK_METHOD0(RollBack, int());
    MOCK_METHOD0(Commit, int());
    MOCK_METHOD0(IsInTransaction, bool());
    MOCK_METHOD0(GetPath, std::string());
    MOCK_METHOD0(IsHoldingConnection, bool());
    MOCK_CONST_METHOD0(IsOpen, bool());
    MOCK_CONST_METHOD0(IsReadOnly, bool());
    MOCK_CONST_METHOD0(IsMemoryRdb, bool());
    MOCK_METHOD2(Restore, int(const std::string &backupPath, const std::vector<uint8_t> &newKey));
    MOCK_METHOD1(GetRebuilt, int(RebuiltType &rebuilt));
    MOCK_METHOD1(CreateTransaction, std::pair<int32_t, std::shared_ptr<Transaction>>(int32_t type));
};

class AbsSharedResultSetMock : public AbsSharedResultSet {
public:
    ~AbsSharedResultSetMock() {}
    MOCK_METHOD1(GetAllColumnNames, int(std::vector<std::string> &));
    MOCK_METHOD1(GetColumnCount, int(int &));
    MOCK_METHOD2(GetColumnType, int(int, ColumnType &));
    MOCK_METHOD2(GetColumnIndex, int(const std::string &, int &));
    MOCK_METHOD2(GetColumnName, int(int, std::string &));
    MOCK_METHOD1(GetRowCount, int(int &));
    MOCK_METHOD1(GoTo, int(int));
    MOCK_METHOD1(GoToRow, int(int));
    MOCK_METHOD0(GoToFirstRow, int());
    MOCK_METHOD0(GoToLastRow, int());
    MOCK_METHOD0(GoToNextRow, int());
    MOCK_METHOD0(GoToPreviousRow, int());
    MOCK_METHOD1(IsEnded, int(bool &));
    MOCK_METHOD1(IsAtLastRow, int(bool &));
    MOCK_METHOD2(GetBlob, int(int, std::vector<uint8_t> &));
    MOCK_METHOD2(GetString, int(int, std::string &));
    MOCK_METHOD2(GetInt, int(int, int &));
    MOCK_METHOD2(GetLong, int(int, int64_t &));
    MOCK_METHOD2(GetDouble, int(int, double &));
    MOCK_METHOD2(IsColumnNull, int(int, bool &));
    MOCK_METHOD0(Close, int());
    MOCK_METHOD1(GetRow, int(RowEntity &));
    
    MOCK_CONST_METHOD0(IsClosed, bool());
    MOCK_CONST_METHOD1(GetRowIndex, int(int &));
    MOCK_CONST_METHOD1(IsStarted, int(bool &));
    MOCK_CONST_METHOD1(IsAtFirstRow, int(bool &));
    
    MOCK_METHOD2(GetAsset, int(int32_t, ValueObject::Asset &));
    MOCK_METHOD2(GetAssets, int(int32_t, ValueObject::Assets &));
    MOCK_METHOD2(Get, int(int32_t, ValueObject &));
    MOCK_METHOD1(GetModifyTime, int(std::string &));
    MOCK_METHOD2(GetSize, int(int32_t, size_t &));
};

class Assistant {
public:
    static inline std::shared_ptr<Assistant> ins = nullptr;
public:
    virtual ~Assistant() = default;
    virtual shared_ptr<RdbStore> GetRdbStore(const RdbStoreConfig &, int, RdbOpenCallback &, int &) = 0;
    virtual int DeleteRdbStore(const std::string &) = 0;
    virtual std::string GetDefaultDatabasePath(const std::string &, const std::string &, int &) = 0;
};

class AssistantMock : public Assistant {
public:
    MOCK_METHOD4(GetRdbStore, shared_ptr<RdbStore>(const RdbStoreConfig &, int, RdbOpenCallback &, int &));
    MOCK_METHOD1(DeleteRdbStore, int(const std::string &));
    MOCK_METHOD3(GetDefaultDatabasePath, std::string(const std::string &, const std::string &, int &));
};
} // namespace OHOS::FileManagement::CloudSync::Test
#endif