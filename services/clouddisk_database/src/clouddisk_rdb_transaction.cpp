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

namespace OHOS {
namespace FileManagement {
namespace CloudDisk {
using namespace std;

constexpr int32_t E_HAS_DB_ERROR = -222;
constexpr int32_t E_OK = 0;

constexpr int RDB_TRANSACTION_WAIT_MS = 1000;
atomic<bool> TransactionOperations::isInTransactionMap_[RDB_NUM] = {false, false};
mutex TransactionOperations::transactionMutexMap_[RDB_NUM] = {mutex(), mutex()};
condition_variable TransactionOperations::transactionCvMap_[RDB_NUM] = {condition_variable(), condition_variable()};
constexpr int32_t MAX_TRY_TIMES = 5;
constexpr int32_t TRANSACTION_WAIT_INTERVAL = 50; // in milliseconds.

TransactionOperations::TransactionOperations(
    const shared_ptr<OHOS::NativeRdb::RdbStore> &rdbStore, TRANS_DB_IDX idx) : rdbStore_(rdbStore), idx_(idx) {}

TransactionOperations::~TransactionOperations()
{
    if (isStart && !isFinish) {
        LOGI("TransactionOperations::RollBack");
        TransactionRollback();
    }
}

int32_t TransactionOperations::Start()
{
    if (isStart || isFinish) {
        return 0;
    }
    LOGI("TransactionOperations::Start");
    int32_t errCode = BeginTransaction();
    if (errCode == 0) {
        isStart = true;
    }
    return errCode;
}

void TransactionOperations::Finish()
{
    if (!isStart) {
        return;
    }
    if (!isFinish) {
        LOGI("TransactionOperations::Finish");
        int32_t ret = TransactionCommit();
        if (ret == 0) {
            isFinish = true;
        }
    }
}

int32_t TransactionOperations::BeginTransaction()
{
    if (rdbStore_ == nullptr) {
        LOGE("Pointer rdbStore_ is nullptr. Maybe it didn't init successfully.");
        return E_HAS_DB_ERROR;
    }
    LOGI("Start transaction");

    unique_lock<mutex> cvLock(transactionMutexMap_[idx_]);
    if (isInTransactionMap_[idx_].load()) {
        transactionCvMap_[idx_].wait_for(cvLock, chrono::milliseconds(RDB_TRANSACTION_WAIT_MS),
            [this] () { return !(isInTransactionMap_[idx_].load()); });
    }

    int curTryTime = 0;
    while (curTryTime < MAX_TRY_TIMES) {
        if (rdbStore_->IsInTransaction()) {
            this_thread::sleep_for(chrono::milliseconds(TRANSACTION_WAIT_INTERVAL));
            if (isInTransactionMap_[idx_].load() || rdbStore_->IsInTransaction()) {
                curTryTime++;
                LOGI("RdbStore is in transaction, try %{public}d times...", curTryTime);
                continue;
            }
        }

        int32_t errCode = rdbStore_->BeginTransaction();
        if (errCode == NativeRdb::E_SQLITE_LOCKED || errCode == NativeRdb::E_DATABASE_BUSY ||
            errCode == NativeRdb::E_SQLITE_BUSY) {
            curTryTime++;
            LOGE("Sqlite database file is locked! try %{public}d times...", curTryTime);
            continue;
        } else if (errCode != NativeRdb::E_OK) {
            LOGE("Start Transaction failed, errCode=%{public}d", errCode);
            isInTransactionMap_[idx_].store(false);
            transactionCvMap_[idx_].notify_one();
            return E_HAS_DB_ERROR;
        } else {
            isInTransactionMap_[idx_].store(true);
            return E_OK;
        }
    }

    LOGE("RdbStore is still in transaction after try %{public}d times, abort.", MAX_TRY_TIMES);
    return E_HAS_DB_ERROR;
}

int32_t TransactionOperations::TransactionCommit()
{
    if (rdbStore_ == nullptr) {
        return E_HAS_DB_ERROR;
    }
    LOGI("Try commit transaction");

    if (!(isInTransactionMap_[idx_].load()) || !(rdbStore_->IsInTransaction())) {
        LOGE("no transaction now");
        return E_HAS_DB_ERROR;
    }

    int32_t errCode = rdbStore_->Commit();
    isInTransactionMap_[idx_].store(false);
    transactionCvMap_[idx_].notify_all();
    if (errCode != NativeRdb::E_OK) {
        LOGE("commit failed, errCode=%{public}d", errCode);
        return E_HAS_DB_ERROR;
    }

    return E_OK;
}

int32_t TransactionOperations::TransactionRollback()
{
    if (rdbStore_ == nullptr) {
        return E_HAS_DB_ERROR;
    }
    LOGI("Try rollback transaction");

    if (!(isInTransactionMap_[idx_].load()) || !(rdbStore_->IsInTransaction())) {
        LOGE("no transaction now");
        return E_HAS_DB_ERROR;
    }

    int32_t errCode = rdbStore_->RollBack();
    isInTransactionMap_[idx_].store(false);
    transactionCvMap_[idx_].notify_all();
    if (errCode != NativeRdb::E_OK) {
        LOGE("rollback failed, errCode=%{public}d", errCode);
        return E_HAS_DB_ERROR;
    }

    return E_OK;
}
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS
