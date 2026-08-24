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
#include "utils_directory.h"
#include "utils_log.h"
#include "hisysevent.h"
#include "meta_file.h"

namespace OHOS {
namespace FileManagement {
namespace CloudDisk {
#ifndef CLOUD_SYNC_SYS_EVENT
#define CLOUD_SYNC_SYS_EVENT(eventName, type, ...)    \
    HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::FILEMANAGEMENT, eventName, type, \
                    ##__VA_ARGS__)
#endif

using namespace std;
static const int32_t MAX_FILE_COUNT_PER_DIR = 2;
static const string FILEMANAGER_BUNDLE_NAME = "com.hmos.filemanager";
static const int32_t OP_TYPE_INDEX = 1;
static const int32_t OP_TIME_INDEX = 2;
static const int32_t FILE_INODE_INDEX = 3;
static const int32_t FILE_UID_INDEX = 4;
static const int32_t PROCESS_NAME_INDEX = 5;
static const int32_t PROCESS_PID_INDEX = 6;
static const int32_t PROCESS_UID_INDEX = 7;
static const int32_t CLOUD_ID_INDEX = 8;

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
            std::chrono::steady_clock::now().time_since_epoch()).count());
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
                std::chrono::steady_clock::now().time_since_epoch()).count());
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
        int32_t ret = WriteBatch(batch);
        if (ret != E_OK) {
            LOGE("drain write batch failed, ret=%{public}d, batchSize=%{public}zu", ret, batch.size());
        }
    }
}

int32_t OperationLogHandler::RecordDelete(int64_t opTime, const string& filePath, int64_t fileInode,
    int64_t fileUid, const string& processName, int64_t processPid, int64_t processUid, const string& cloudId)
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
    entry.cloudId = cloudId;

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
        values.PutString(OperationLogColumn::CLOUD_ID, entry.cloudId);
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
        rdbStore->RollBack();
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
    constexpr int64_t DELETE_BATCH_SIZE = 10000;
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

std::string OperationLogHandler::GetAnonyStringLocal(const std::string &value)
{
    constexpr size_t shortIdLength = 20;
    constexpr size_t plaintextLength = 4;
    constexpr size_t minIdLength = 3;
    std::string res;
    std::string tmpStr("******");
    size_t strLen = value.length();
    if (strLen < minIdLength) {
        return tmpStr;
    }

    if (strLen <= shortIdLength) {
        res += value[0];
        res += tmpStr;
        res += value[strLen - 1];
    } else {
        res.append(value, 0, plaintextLength);
        res += tmpStr;
        res.append(value, strLen - plaintextLength, plaintextLength);
    }
    return res;
}

std::map<std::string, int32_t> OperationLogHandler::QueryDirFileCounts(
    std::shared_ptr<NativeRdb::RdbStore> rdbStore)
{
    std::map<std::string, int32_t> dirCounts;
    constexpr int32_t BATCH_SIZE = 1000;
    constexpr int32_t MAX_ITERATIONS = 100;
    int32_t offset = 0;
    int32_t iterationCount = 0;
    while (iterationCount < MAX_ITERATIONS) {
        NativeRdb::AbsRdbPredicates predicates(OperationLogColumn::TABLE_NAME);
        predicates.Limit(BATCH_SIZE)->Offset(offset);
        std::vector<std::string> columns = { OperationLogColumn::FILE_PATH };
        auto resultSet = rdbStore->Query(predicates, columns);
        if (resultSet == nullptr) {
            LOGE("QueryDirFileCounts query failed, offset=%{public}d", offset);
            break;
        }

        int32_t rowCount = 0;
        while (resultSet->GoToNextRow() == E_OK) {
            std::string filePath;
            int32_t ret = resultSet->GetString(0, filePath);
            if (ret != E_OK) {
                continue;
            }
            std::string dir = MetaFile::GetParentDir(filePath);
            dirCounts[dir]++;
            rowCount++;
        }
        resultSet->Close();

        if (rowCount < BATCH_SIZE) {
            break;
        }
        offset += BATCH_SIZE;
        iterationCount++;
    }
    return dirCounts;
}

std::string OperationLogHandler::GetCommonParentDir(const std::vector<std::string> &paths)
{
    if (paths.empty()) {
        return "";
    }
    std::string commonPrefix = paths[0];
    for (size_t j = 1; j < paths.size(); ++j) {
        const std::string &path = paths[j];
        size_t i = 0;
        while (i < commonPrefix.size() && i < path.size() && commonPrefix[i] == path[i]) {
            i++;
        }
        commonPrefix = commonPrefix.substr(0, i);
    }
    if (commonPrefix == paths[0]) {
        return paths[0];
    }
    size_t lastSlash = commonPrefix.find_last_of('/');
    if (lastSlash == std::string::npos) {
        return "";
    }
    if (lastSlash == 0) {
        return "/";
    }
    return commonPrefix.substr(0, lastSlash);
}

bool OperationLogHandler::ReadFileStatFields(NativeRdb::ResultSet &resultSet, const std::string &dir,
    OperationLogEntry &entry)
{
    int32_t ret = resultSet.GetString(0, entry.filePath);
    if (ret != E_OK) {
        return false;
    }
    if (MetaFile::GetParentDir(entry.filePath) != dir) {
        return false;
    }
    int32_t opTypeRet = resultSet.GetInt(OP_TYPE_INDEX, entry.opType);
    if (opTypeRet != E_OK) {
        return false;
    }
    int32_t opTimeRet = resultSet.GetLong(OP_TIME_INDEX, entry.opTime);
    if (opTimeRet != E_OK) {
        return false;
    }
    int32_t fileInodeRet = resultSet.GetLong(FILE_INODE_INDEX, entry.fileInode);
    if (fileInodeRet != E_OK) {
        return false;
    }
    int32_t fileUidRet = resultSet.GetLong(FILE_UID_INDEX, entry.fileUid);
    if (fileUidRet != E_OK) {
        return false;
    }
    int32_t processNameRet = resultSet.GetString(PROCESS_NAME_INDEX, entry.processName);
    if (processNameRet != E_OK) {
        return false;
    }
    int32_t processPidRet = resultSet.GetLong(PROCESS_PID_INDEX, entry.processPid);
    if (processPidRet != E_OK) {
        return false;
    }
    int32_t processUidRet = resultSet.GetLong(PROCESS_UID_INDEX, entry.processUid);
    if (processUidRet != E_OK) {
        return false;
    }
    int32_t cloudIdRet = resultSet.GetString(CLOUD_ID_INDEX, entry.cloudId);
    if (cloudIdRet != E_OK) {
        return false;
    }

    return true;
}

int32_t OperationLogHandler::ReportFileStats(
    std::shared_ptr<NativeRdb::RdbStore> rdbStore, const std::string &dir)
{
    NativeRdb::AbsRdbPredicates predicates(OperationLogColumn::TABLE_NAME);
    std::string likePattern = dir.empty() ? "/%" : dir + "/%";
    predicates.Like(OperationLogColumn::FILE_PATH, likePattern);
    std::vector<std::string> columns = {
        OperationLogColumn::FILE_PATH, OperationLogColumn::OP_TYPE,
        OperationLogColumn::OP_TIME, OperationLogColumn::FILE_INODE,
        OperationLogColumn::FILE_UID, OperationLogColumn::PROCESS_NAME,
        OperationLogColumn::PROCESS_PID, OperationLogColumn::PROCESS_UID,
        OperationLogColumn::CLOUD_ID
    };
    auto resultSet = rdbStore->Query(predicates, columns);
    if (resultSet == nullptr) {
        return 0;
    }

    int32_t actualCount = 0;
    while (resultSet->GoToNextRow() == E_OK) {
        OperationLogEntry entry;
        if (!ReadFileStatFields(*resultSet, dir, entry)) {
            continue;
        }
        int32_t ret = CLOUD_SYNC_SYS_EVENT("CLOUD_DISK_OPERATION_LOG_STAT",
            HiviewDFX::HiSysEvent::EventType::STATISTIC,
            "bundle_name", FILEMANAGER_BUNDLE_NAME, "cloud_id", entry.cloudId,
            "dir_path", GetAnonyStringLocal(dir), "file_count", 1,
            "file_inode", entry.fileInode, "file_name", GetAnonyStringLocal(MetaFile::GetFileName(entry.filePath)),
            "file_uid", entry.fileUid, "op_time", entry.opTime, "op_type", entry.opType,
            "process_name", GetAnonyStringLocal(entry.processName),
            "process_pid", entry.processPid, "process_uid", entry.processUid);
        if (ret != E_OK) {
            LOGE("report file stat failed, ret=%{public}d", ret);
        }
        actualCount++;
    }
    resultSet->Close();
    return actualCount;
}
 
int32_t OperationLogHandler::ReportDirStats(
    std::shared_ptr<NativeRdb::RdbStore> rdbStore, const std::string &dir)
{
    NativeRdb::AbsRdbPredicates predicates(OperationLogColumn::TABLE_NAME);
    std::string likePattern = dir.empty() ? "/%" : dir + "/%";
    predicates.Like(OperationLogColumn::FILE_PATH, likePattern);
    std::vector<std::string> columns = {
        OperationLogColumn::FILE_PATH
    };
    auto resultSet = rdbStore->Query(predicates, columns);
    if (resultSet == nullptr) {
        return 0;
    }

    int32_t fileCount = 0;
    int32_t opType = 0;
    while (resultSet->GoToNextRow() == E_OK) {
        std::string filePath;
        int32_t ret = resultSet->GetString(0, filePath);
        if (ret != E_OK) {
            continue;
        }
        if (MetaFile::GetParentDir(filePath) != dir) {
            continue;
        }
        fileCount++;
    }
    if (fileCount == 0) {
        resultSet->Close();
        return 0;
    }
    int32_t ret = CLOUD_SYNC_SYS_EVENT("CLOUD_DISK_OPERATION_LOG_STAT",
        HiviewDFX::HiSysEvent::EventType::STATISTIC,
        "bundle_name", FILEMANAGER_BUNDLE_NAME,
        "dir_path", GetAnonyStringLocal(dir), "file_count", fileCount);
    if (ret != E_OK) {
        LOGE("report dir stat failed, ret=%{public}d", ret);
    }
    resultSet->Close();
    return fileCount;
}

int32_t OperationLogHandler::ReportOperationLogStat()
{
    LOGI("ReportOperationLogStat starts");
    auto rdbStore = OperationLogStore::GetInstance().GetRaw();
    if (rdbStore == nullptr) {
        return E_RDB;
    }

    auto dirCounts = QueryDirFileCounts(rdbStore);
    constexpr int32_t MAX_DIR_COUNT_THRESHOLD = 10;
    if (dirCounts.size() > MAX_DIR_COUNT_THRESHOLD) {
        int32_t totalCount = 0;
        std::vector<std::string> paths;
        paths.reserve(dirCounts.size());
        for (const auto &kv : dirCounts) {
            totalCount += kv.second;
            paths.push_back(kv.first);
        }
        std::string commonParent = GetCommonParentDir(paths);
        int32_t ret = CLOUD_SYNC_SYS_EVENT("CLOUD_DISK_OPERATION_LOG_STAT",
            HiviewDFX::HiSysEvent::EventType::STATISTIC,
            "bundle_name", FILEMANAGER_BUNDLE_NAME,
            "dir_path", GetAnonyStringLocal(commonParent), "file_count", totalCount);
        if (ret != E_OK) {
            LOGE("report combined dir stat failed, ret=%{public}d", ret);
        }
        return E_OK;
    }
    for (const auto &kv : dirCounts) {
        int32_t expectedCount = kv.second;
        int32_t actualCount = 0;
        if (expectedCount < MAX_FILE_COUNT_PER_DIR) {
            actualCount = ReportFileStats(rdbStore, kv.first);
        } else {
            actualCount = ReportDirStats(rdbStore, kv.first);
        }
        if (actualCount != expectedCount) {
            LOGW("ReportOperationLogStat count mismatch, dir=%{public}s, expected=%{public}d + actual=%{public}d",
                GetAnonyStringLocal(kv.first).c_str(), expectedCount, actualCount);
        }
    }
    LOGI("ReportOperationLogStat done, dirs=%{public}zu", dirCounts.size());
    return E_OK;
}
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS
