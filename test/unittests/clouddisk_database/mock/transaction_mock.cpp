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

#include <gmock/gmock.h>
#include "transaction.h"

namespace OHOS::FileManagement::CloudDisk {
using namespace NativeRdb;
class TransactionMock : public NativeRdb::Transaction {
public:
    MOCK_METHOD3(Update, std::pair<int32_t, int32_t>(const Row &row, const AbsRdbPredicates &predicates,
                 Resolution resolution));
    MOCK_METHOD5(Update, std::pair<int, int>(const std::string &table, const Row &row,
                 const std::string &where, const Values &args, Resolution resolution));
    MOCK_METHOD3(Insert, std::pair<int32_t, int64_t>(const std::string &table, const Row &row,
                 Resolution resolution));
    MOCK_METHOD0(Commit, int32_t());
    MOCK_METHOD0(Rollback, int32_t());
    MOCK_METHOD0(Close, int32_t());
    MOCK_METHOD2(BatchInsert, std::pair<int32_t, int64_t>(const std::string &table, const Rows &rows));
    MOCK_METHOD2(BatchInsert, std::pair<int32_t, int64_t>(const std::string &table, const RefRows &rows));
    MOCK_METHOD3(Delete, std::pair<int32_t, int32_t>(const std::string &table, const std::string &whereClause,
                 const Values &args));
    MOCK_METHOD1(Delete, std::pair<int32_t, int32_t>(const AbsRdbPredicates &predicates));
    MOCK_METHOD2(QueryByStep, std::shared_ptr<ResultSet>(const std::string &sql, const Values &args));
    MOCK_METHOD2(QueryByStep, std::shared_ptr<ResultSet>(const AbsRdbPredicates &predicates, const Fields &columns));
    MOCK_METHOD2(Execute, std::pair<int32_t, ValueObject>(const std::string &sql, const Values &args));
};
}