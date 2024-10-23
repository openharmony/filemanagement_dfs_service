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

#include "clouddisk_rdb_transaction.h"
#include "utils_log.h"
#include <utility>

namespace OHOS {
namespace FileManagement {
namespace CloudDisk {
using namespace std;

constexpr int32_t E_HAS_DB_ERROR = -222;
constexpr int32_t E_OK = 0;

constexpr int RDB_TRANSACTION_WAIT_MS = 1000;
constexpr int32_t MAX_TRY_TIMES = 5;
constexpr int32_t TRANSACTION_WAIT_INTERVAL = 50; // in milliseconds.

TransactionOperations::TransactionOperations(
    const shared_ptr<OHOS::NativeRdb::RdbStore> &rdbStore) : rdbStore_(rdbStore) {}

TransactionOperations::~TransactionOperations()
{
    if (isStart && !isFinish) {
        LOGI("TransactionOperations::RollBack");
        TransactionRollback();
    }
}

std::pair<int32_t, std::shared_ptr<NativeRdb::Transaction>> TransactionOperations::Start()
{
    return make_pair(E_OK, nullptr);
}

void TransactionOperations::Finish()
{
}

int32_t TransactionOperations::BeginTransaction()
{
    return E_OK;
}

int32_t TransactionOperations::TransactionCommit()
{
    return E_OK;
}

int32_t TransactionOperations::TransactionRollback()
{
    return E_OK;
}
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS