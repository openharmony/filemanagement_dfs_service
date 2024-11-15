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
constexpr int32_t CREATE_TRANSATION_MAX_TRY_TIMES = 30;
constexpr int32_t COMMIT_MAX_TRY_TIMES = 3;
constexpr int32_t TRANSACTION_WAIT_INTERVAL = 50;

TransactionOperations::TransactionOperations(
    const shared_ptr<OHOS::NativeRdb::RdbStore> &rdbStore) : rdbStore_(rdbStore) {}

TransactionOperations::~TransactionOperations()
{
    if (isStart && !isFinish) {
        LOGI("TransactionOperations::RollBack");
        TransactionRollback();
    }
}

template <typename Func>
int32_t RetryTransaction(Func func, int maxAttempts, int waitTimeMs)
{
    int curTryTime = 0;
    while (curTryTime < maxAttempts) {
        int32_t errCode = func();
        if (errCode != NativeRdb::E_OK) {
            curTryTime++;
            LOGE("try %{public}d times...", curTryTime);
            this_thread::sleep_for(chrono::milliseconds(waitTimeMs));
            continue;
        }
        return E_OK;
    }
    LOGE("Transaction operation still failed after try %{public}d times, abort.", maxAttempts);
    return E_HAS_DB_ERROR;
}

std::pair<int32_t, std::shared_ptr<NativeRdb::Transaction>> TransactionOperations::Start(
    NativeRdb::Transaction::TransactionType type)
{
    if (isStart || isFinish) {
        LOGE("start transaction failed, transaction has been started.");
        return make_pair(E_HAS_DB_ERROR, nullptr);
    }

    LOGI("TransactionOperations::Start");
    int32_t errCode = BeginTransaction(type);
    if (errCode == E_OK) {
        isStart = true;
    }
    return make_pair(errCode, transaction_);
}

void TransactionOperations::Finish()
{
    if (!isStart || isFinish) {
        return;
    }

    if (!isFinish) {
        LOGI("TransactionOperations::Finish");
        int32_t ret = TransactionCommit();
        if (ret == E_OK) {
            isFinish = true;
            isStart = false;
            transaction_.reset();
            return;
        }
        LOGE("TransactionCommit failed, errCode=%{public}d, try to rollback", ret);
        ret = TransactionRollback();
        if (ret != E_OK) {
            LOGE("TransactionRollback failed, errCode=%{public}d", ret);
        }
    }
}

int32_t TransactionOperations::BeginTransaction(NativeRdb::Transaction::TransactionType type)
{
    if (rdbStore_ == nullptr) {
        LOGE("Pointer rdbStore_ is nullptr. Maybe it didn't init successfully.");
        return E_HAS_DB_ERROR;
    }
    LOGI("Start transaction");
    int curTryTime = 0;
    while (curTryTime < CREATE_TRANSATION_MAX_TRY_TIMES) {
        int32_t errCode = E_OK;
        std::tie(errCode, transaction_) = rdbStore_->CreateTransaction(type);
        if (errCode == NativeRdb::E_SQLITE_LOCKED || errCode == NativeRdb::E_DATABASE_BUSY ||
            errCode == NativeRdb::E_SQLITE_BUSY) {
            curTryTime++;
            LOGE("Sqlite database file is locked! try %{public}d times...", curTryTime);
            this_thread::sleep_for(chrono::milliseconds(TRANSACTION_WAIT_INTERVAL));
            continue;
        } else if (errCode != NativeRdb::E_OK) {
            LOGE("Start Transaction failed, errCode=%{public}d", errCode);
            return errCode;
        }
        if (transaction_ != nullptr) {
            isStart = true;
            return E_OK;
        }
    }
    LOGE("RdbStore is still in transaction after try %{public}d times, abort.", CREATE_TRANSATION_MAX_TRY_TIMES);
    return E_HAS_DB_ERROR;
}

int32_t TransactionOperations::TransactionCommit()
{
    if (transaction_ == nullptr) {
        return E_HAS_DB_ERROR;
    }
    LOGI("Try commit transaction");

    return RetryTransaction(
        [this]() {
            return transaction_->Commit();
        }, COMMIT_MAX_TRY_TIMES, TRANSACTION_WAIT_INTERVAL);
}

int32_t TransactionOperations::TransactionRollback()
{
    if (transaction_ == nullptr) {
        return E_HAS_DB_ERROR;
    }
    LOGI("Try rollback transaction");

    return RetryTransaction(
        [this]() {
            return transaction_->Rollback();
        }, COMMIT_MAX_TRY_TIMES, TRANSACTION_WAIT_INTERVAL);
}
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS