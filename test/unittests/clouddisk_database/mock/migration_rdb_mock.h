/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_MIGRATION_RDB_MOCK_H
#define OHOS_MIGRATION_RDB_MOCK_H

#include "abs_rdb_predicates.h"
#include "rdb_helper.h"
#include "result_set.h"
#include "transaction.h"
#include "value_object.h"
#include "values_bucket.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace OHOS::FileManagement::CloudDisk::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using namespace NativeRdb;

class MigrationRdbStoreMock : public RdbStore {
public:
    MOCK_METHOD2(QuerySql, shared_ptr<AbsSharedResultSet>(const string &, const Values &));
    MOCK_METHOD0(BeginTransaction, int());
    MOCK_METHOD0(RollBack, int());
    MOCK_METHOD0(Commit, int());
    MOCK_METHOD1(CreateTransaction, pair<int32_t, shared_ptr<Transaction>>(int32_t type));
    MOCK_METHOD1(GetVersion, int(int &));
    MOCK_METHOD1(SetVersion, int(int));
};

class MigrationResultSetMock : public AbsSharedResultSet {
public:
    MOCK_METHOD0(GoToNextRow, int());
    MOCK_METHOD2(GetString, int(int, string &));
    MOCK_METHOD2(GetInt, int(int, int &));
    MOCK_METHOD0(Close, int());
};

class MigrationAssistantMock {
public:
    MOCK_METHOD4(GetRdbStore, shared_ptr<RdbStore>(const RdbStoreConfig &, int, RdbOpenCallback &, int &));
    MOCK_METHOD3(GetDefaultDatabasePath, string(const string &, const string &, int &));

    static inline shared_ptr<MigrationAssistantMock> ins = nullptr;
};

shared_ptr<RdbStore> GetMockedRdbStore(const RdbStoreConfig &config, int version,
    RdbOpenCallback &openCallback, int &errCode);
string GetMockedDatabasePath(const string &baseDir, const string &name, int &errorCode);

} // namespace OHOS::FileManagement::CloudDisk::Test
#endif