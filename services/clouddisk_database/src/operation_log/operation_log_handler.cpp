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

#include "operation_log_handler.h"

#include <chrono>
#include <ctime>

#include "dfs_error.h"
#include "operation_log_column.h"
#include "operation_log_const.h"
#include "operation_log_entry.h"
#include "operation_log_store.h"
#include "rdb_errno.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudDisk {

using namespace std;

OperationLogHandler& OperationLogHandler::GetInstance()
{
    static OperationLogHandler instance;
    return instance;
}

OperationLogHandler::~OperationLogHandler()
{
    Stop();
}

int32_t OperationLogHandler::Init(int32_t userId)
{
    LOGI("Begin to init, userId=%{public}d", userId);
    if (isInited_) {
        LOGI("isInited_ true");
        return E_OK;
    }

    userId_ = userId;
    int32_t ret = OperationLogStore::GetInstance().Init(userId);
    if (ret != E_OK) {
        LOGE("operation log rdb init failed, ret = %{public}d", ret);
        return ret;
    }

    isInited_ = true;
    return E_OK;
}

void OperationLogHandler::Start()
{
    lock_guard<mutex> lock(lifecycleMutex_);
    if (running_.load()) {
        return;
    }

    if (writeThread_.joinable()) {
        writeThread_.join();
    }

    queue_.Reset();
    lastCleanTime_ = static_cast<int64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
    running_ = true;
    writeThread_ = thread([this]() {
        WriteThreadLoop();
    });

    LOGI("OperationLogHandler start end");
}

void OperationLogHandler::Stop()
{
    lock_guard<mutex> lock(lifecycleMutex_);
    queue_.Shutdown();
    running_ = false;

    if (writeThread_.joinable()) {
        writeThread_.join();
    }

    LOGI("OperationLogHandler stop end");
}

void OperationLogHandler::WriteThreadLoop()
{
    constexpr int64_t CLEAN_INTERVAL_MS = 60 * 60 * 1000;
    while (running_.load()) {
        auto batch = queue_.PopBatch();
        if (batch.empty()) {
            continue;
        }

        int32_t ret = WriteBatch(batch);
        if (ret != E_OK) {
            LOGE("write batch failed, ret=%{public}d, batchSize=%{public}zu", ret, batch.size());
        }

        int64_t now = static_cast<int64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count());
        if (now - lastCleanTime_ > CLEAN_INTERVAL_MS) {
            CleanOldRecords();
            lastCleanTime_ = now;
        }
    }

    while (!queue_.IsEmpty()) {
        auto batch = queue_.PopBatch();
        if (batch.empty()) {
            continue;
        }
        WriteBatch(batch);
    }
}

int32_t OperationLogHandler::RecordDelete(int64_t opTime, const string& filePath,
    int64_t fileInode, int64_t fileUid, const string& processName, int64_t processPid, int64_t processUid)
{
    if (!isInited_.load()) {
        LOGE("RecordDelete isInited_ is false");
        return E_RDB;
    }
    if (!running_.load()) {
        LOGE("RecordDelete running_ is false");
        return E_RDB;
    }
    if (queue_.IsFull()) {
        LOGE("RecordDelete queue_ is full");
        return E_RDB;
    }
    OperationLogEntry entry;
    entry.opTime = opTime;
    entry.opType = OperationLogConst::OP_TYPE_DELETE;
    entry.filePath = filePath;
    entry.fileInode = fileInode;
    entry.fileUid = fileUid;
    entry.processName = processName;
    entry.processPid = processPid;
    entry.processUid = processUid;

    queue_.Push(entry);
    return E_OK;
}

int32_t OperationLogHandler::WriteBatch(const vector<OperationLogEntry>& batch)
{
    for (int32_t retry = 0; retry < OperationLogConst::MAX_RETRIES; ++retry) {
        int32_t ret = DoInsert(batch);
        if (ret == E_OK) {
            return E_OK;
        }

        if (IsRecoverableError(ret)) {
            LOGE("write operation log batch for recoverable error failed, ret = %{public}d", ret);
            continue;
        }

        LOGE("write operation log batch for unrecoverable error failed, ret = %{public}d", ret);
        return ret;
    }

    return NativeRdb::E_ERROR;
}

int32_t OperationLogHandler::DoInsert(const vector<OperationLogEntry>& batch)
{
    auto rdbStore = OperationLogStore::GetInstance().GetRaw();
    if (rdbStore == nullptr) {
        LOGE("operation log rdb store is null");
        return E_RDB;
    }

    int32_t recordCount = 0;
    int32_t ret = GetRecordCount(recordCount);
    if (ret != E_OK) {
        LOGE("get operation log count failed, ret = %{public}d", ret);
        return ret;
    }

    if (recordCount >= OperationLogConst::MAX_RECORD_COUNT) {
        ret = CheckAndCleanRecords();
        if (ret != E_OK) {
            LOGE("clean operation log records failed, ret = %{public}d", ret);
            return ret;
        }
    }

    rdbStore->BeginTransaction();
    for (const auto& entry : batch) {
        NativeRdb::ValuesBucket values;
        values.PutLong(OperationLogColumn::OP_TIME, entry.opTime);
        values.PutInt(OperationLogColumn::OP_TYPE, entry.opType);
        values.PutString(OperationLogColumn::FILE_PATH, entry.filePath);
        values.PutLong(OperationLogColumn::FILE_INODE, entry.fileInode);
        values.PutLong(OperationLogColumn::FILE_UID, entry.fileUid);
        values.PutString(OperationLogColumn::PROCESS_NAME, entry.processName);
        values.PutLong(OperationLogColumn::PROCESS_PID, entry.processPid);
        values.PutLong(OperationLogColumn::PROCESS_UID, entry.processUid);
        int64_t rowId;
        ret = rdbStore->Insert(rowId, OperationLogColumn::TABLE_NAME, values);
        if (ret != NativeRdb::E_OK) {
            rdbStore->RollBack();
            LOGE("insert failed, ret = %{public}d", ret);
            return ret;
        }
    }
    ret = rdbStore->Commit();
    if (ret != NativeRdb::E_OK) {
        LOGE("commit failed, ret = %{public}d", ret);
        return ret;
    }

    return E_OK;
}

int32_t OperationLogHandler::GetRecordCount(int32_t& count)
{
    auto rdbStore = OperationLogStore::GetInstance().GetRaw();
    if (rdbStore == nullptr) {
        LOGE("operation log rdb store is null");
        return E_RDB;
    }
    NativeRdb::AbsRdbPredicates predicates(OperationLogColumn::TABLE_NAME);
    auto resultSet = rdbStore->QueryByStep(predicates, {});
    if (resultSet == nullptr) {
        LOGE("query operation log count failed");
        return E_RDB;
    }
    int32_t ret = resultSet->GetRowCount(count);
    if (ret != E_OK || count < 0) {
        LOGE("get operation log count failed, ret = %{public}d", ret);
        return E_RDB;
    }

    return E_OK;
}

int32_t OperationLogHandler::CheckAndCleanRecords()
{
    auto rdbStore = OperationLogStore::GetInstance().GetRaw();
    if (rdbStore == nullptr) {
        LOGE("operation log rdb store is null");
        return E_RDB;
    }
    constexpr int64_t DELETE_BATCH_SIZE = 1000;
    NativeRdb::AbsRdbPredicates queryPredicates(OperationLogColumn::TABLE_NAME);
    queryPredicates.OrderByAsc(OperationLogColumn::ID)->Limit(DELETE_BATCH_SIZE);
    auto resultSet = rdbStore->Query(queryPredicates, { OperationLogColumn::ID });
    if (resultSet == nullptr) {
        LOGE("query operation log ids failed");
        return E_RDB;
    }

    std::vector<string> ids;
    while (resultSet->GoToNextRow() == E_OK) {
        int64_t id = 0;
        int32_t ret = resultSet->GetLong(0, id);
        if (ret != E_OK) {
            LOGE("GetLong failed, ret=%{public}d", ret);
            continue;
        }
        ids.push_back(std::to_string(id));
    }
    resultSet->Close();
    if (ids.empty()) {
        LOGE("CheckAndCleanRecords ids is null");
        return E_RDB;
    }
    NativeRdb::AbsRdbPredicates deletePredicates(OperationLogColumn::TABLE_NAME);
    deletePredicates.In(OperationLogColumn::ID, ids);

    int32_t deletedRows = 0;
    int32_t ret = rdbStore->Delete(deletedRows, deletePredicates);
    if (ret != NativeRdb::E_OK) {
        LOGE("clean operation log records failed");
        return ret;
    }
    LOGI("CheckAndCleanRecords done");
    return E_OK;
}

int32_t OperationLogHandler::CleanOldRecords()
{
    LOGI("CleanOldRecords starts");
    auto rdbStore = OperationLogStore::GetInstance().GetRaw();
    if (rdbStore == nullptr) {
        LOGE("operation log rdb store is null");
        return E_RDB;
    }

    int64_t now = static_cast<int64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
    int64_t cutoffTime = now - OperationLogConst::CLEAN_THRESHOLD_DAYS * OperationLogConst::MILLISECONDS_PER_DAY;

    NativeRdb::AbsRdbPredicates deletePredicates(OperationLogColumn::TABLE_NAME);
    deletePredicates.LessThan(OperationLogColumn::OP_TIME, cutoffTime);

    int32_t deletedRows = 0;
    int32_t ret = rdbStore->Delete(deletedRows, deletePredicates);
    if (ret != NativeRdb::E_OK) {
        LOGE("clean old records failed, ret = %{public}d", ret);
        return ret;
    }
    LOGI("clean old records done, deletedRows = %{public}d", deletedRows);
    return E_OK;
}

bool OperationLogHandler::IsRecoverableError(int32_t errCode)
{
    return errCode == NativeRdb::E_SQLITE_BUSY ||
           errCode == NativeRdb::E_SQLITE_LOCKED ||
           errCode == NativeRdb::E_SQLITE_INTERRUPT ||
           errCode == NativeRdb::E_SQLITE_IOERR ||
           errCode == NativeRdb::E_SQLITE_NOMEM;
}
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS
