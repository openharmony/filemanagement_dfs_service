#include "clouddisk_rdb_transaction.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudDisk {
using namespace std;

constexpr int32_t E_HAS_DB_ERROR = -222;
constexpr int32_t E_OK = 0;
constexpr int32_t MAX_TRY_TIMES = 5;
constexpr int32_t TRANSACTION_WAIT_INTERVAL = 100;

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
        if (errCode == NativeRdb::E_OK) {
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

std::pair<int32_t, std::shared_ptr<NativeRdb::Transaction>> TransactionOperations::Start()
{
    if (isStart || isFinish) {
        return make_pair(0, nullptr);
    }
    LOGI("TransactionOperations::Start");
    int32_t errCode = BeginTransaction();
    if (errCode == 0) {
        isStart = true;
    }
    return make_pair(errCode, transaction_);
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
    int curTryTime = 0;
    while (curTryTime < MAX_TRY_TIMES) {
        int32_t errCode = 0;
        std::tie(errCode, transaction_) = rdbStore_->CreateTransaction(NativeRdb::Transaction::DEFERRED);
        if (errCode == NativeRdb::E_SQLITE_LOCKED || errCode == NativeRdb::E_DATABASE_BUSY ||
            errCode == NativeRdb::E_SQLITE_BUSY) {
            curTryTime++;
            LOGE("Sqlite database file is locked! try %{public}d times...", curTryTime);
            this_thread::sleep_for(chrono::milliseconds(TRANSACTION_WAIT_INTERVAL));
            continue;
        } else if (errCode != NativeRdb::E_OK) {
            LOGE("Start Transaction failed, errCode=%{public}d", errCode);
            return E_HAS_DB_ERROR;
        }
        if (transaction_ != nullptr) {
            isStart = true;
            return E_OK;
        }
    }
    LOGE("RdbStore is still in transaction after try %{public}d times, abort.", MAX_TRY_TIMES);
    return E_HAS_DB_ERROR;
}

int32_t TransactionOperations::TransactionCommit()
{
    if (transaction_ == nullptr) {
        return E_HAS_DB_ERROR;
    }
    LOGI("Try commit transaction");

    return RetryTransaction([this]() {
        return transaction_->Commit();
    }, MAX_TRY_TIMES, TRANSACTION_WAIT_INTERVAL);
}

int32_t TransactionOperations::TransactionRollback()
{
    if (transaction_ == nullptr) {
        return E_HAS_DB_ERROR;
    }
    LOGI("Try rollback transaction");

    return RetryTransaction([this]() {
        return transaction_->Rollback();
    }, MAX_TRY_TIMES, TRANSACTION_WAIT_INTERVAL);
}
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS