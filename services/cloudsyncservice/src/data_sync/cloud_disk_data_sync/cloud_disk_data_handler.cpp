/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "cloud_disk_data_handler.h"

#include <cwchar>
#include <dirent.h>
#include <fcntl.h>
#include <map>
#include <string>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <tuple>
#include <unistd.h>
#include "cloud_disk_data_const.h"
#include "clouddisk_db_const.h"
#include "cloud_file_utils.h"
#include "data_sync_const.h"
#include "dfs_error.h"
#include "directory_ex.h"
#include "dk_record.h"
#include "errors.h"
#include "file_column.h"
#include "sandbox_helper.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
using namespace std;
using namespace NativeRdb;
using namespace DriveKit;
using namespace CloudDisk;
using namespace Media;
using FC = CloudDisk::FileColumn;
CloudDiskDataHandler::CloudDiskDataHandler(int32_t userId, const string &bundleName, std::shared_ptr<RdbStore> rdb)
    : RdbDataHandler(userId, bundleName, FC::FILES_TABLE, rdb), userId_(userId), bundleName_(bundleName)
{
}
void CloudDiskDataHandler::GetFetchCondition(FetchCondition &cond)
{
    cond.limitRes = PULL_LIMIT_SIZE;
    cond.recordType = recordType_;
    if (isChecking_) {
        cond.desiredKeys = checkedKeys_;
    } else {
        cond.desiredKeys = desiredKeys_;
    }
    cond.fullKeys = desiredKeys_;
}
int64_t CloudDiskDataHandler::UTCTimeMilliSeconds()
{
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    return t.tv_sec * SECOND_TO_MILLISECOND + t.tv_nsec / MILLISECOND_TO_NANOSECOND;
}

int32_t CloudDiskDataHandler::GetRetryRecords(std::vector<DriveKit::DKRecordId> &records)
{
    NativeRdb::AbsRdbPredicates retryPredicates = NativeRdb::AbsRdbPredicates(FC::FILES_TABLE);
    retryPredicates.SetWhereClause(FC::DIRTY_TYPE + " = ? ");
    retryPredicates.SetWhereArgs({to_string(static_cast<int32_t>(DirtyType::TYPE_RETRY))});
    retryPredicates.Limit(PULL_LIMIT_SIZE);
    auto results = Query(retryPredicates, {FC::CLOUD_ID});
    if (results == nullptr) {
        LOGE("get nullptr modified result");
        return E_RDB;
    }

    while (results->GoToNextRow() == 0) {
        string record;
        int ret = DataConvertor::GetString(FC::CLOUD_ID, record, *results);
        if (ret == E_OK) {
            records.emplace_back(record);
        }
    }
    return E_OK;
}

static bool IsLocalDirty(NativeRdb::ResultSet &local)
{
    int dirty;
    int ret = DataConvertor::GetInt(FC::DIRTY_TYPE, dirty, local);
    if (ret != E_OK) {
        LOGE("Get dirty int failed");
        return false;
    }
    return (dirty == static_cast<int32_t>(DirtyType::TYPE_MDIRTY)) ||
           (dirty == static_cast<int32_t>(DirtyType::TYPE_FDIRTY)) ||
           (dirty == static_cast<int32_t>(DirtyType::TYPE_DELETED));
}
static bool FileIsRecycled(NativeRdb::ResultSet &local)
{
    int64_t recycledTime = 0;
    int ret = DataConvertor::GetLong(FC::FILE_TIME_RECYCLED, recycledTime, local);
    if (ret != E_OK) {
        LOGE("Get local recycled failed");
        return false;
    }
    return recycledTime > 0;
}
static bool FileIsLocal(NativeRdb::ResultSet &local)
{
    int32_t position = 0;
    int32_t ret = DataConvertor::GetInt(FC::POSITION, position, local);
    if (ret != E_OK) {
        LOGE("Get local position failed");
        return false;
    }

    return position != static_cast<int32_t>(POSITION_CLOUD);
}

void CloudDiskDataHandler::HandleCreateConvertErr(NativeRdb::ResultSet &resultSet)
{
    string cloudId;
    int32_t ret = createConvertor_.GetString(FC::CLOUD_ID, cloudId, resultSet);
    if (ret != E_OK) {
        LOGE("get create cloudId err, %{public}d", ret);
        return;
    }
    createFailSet_.push_back(cloudId);
}

void CloudDiskDataHandler::HandleFdirtyConvertErr(NativeRdb::ResultSet &resultSet)
{
    string cloudId;
    int32_t ret = fdirtyConvertor_.GetString(FC::CLOUD_ID, cloudId, resultSet);
    if (ret != E_OK) {
        LOGE("get fdirty cloudId err, %{public}d", ret);
        return;
    }
    modifyFailSet_.push_back(cloudId);
}

tuple<shared_ptr<NativeRdb::ResultSet>, map<string, int>> CloudDiskDataHandler::QueryLocalByCloudId(
    const vector<string> &recordIds)
{
    NativeRdb::AbsRdbPredicates predicates = NativeRdb::AbsRdbPredicates(FC::FILES_TABLE);
    predicates.In(FC::CLOUD_ID, recordIds);
    auto resultSet = Query(predicates, FC::PULL_QUERY_COLUMNS);
    auto recordIdRowIdMap = map<string, int>();
    if (resultSet == nullptr) {
        LOGE("get nullptr created result");
        return {nullptr, std::move(recordIdRowIdMap)};
    }
    int rowCount = 0;
    int ret = resultSet->GetRowCount(rowCount);
    if (ret != 0 || rowCount < 0) {
        LOGE("result set get row count err %{public}d, rowCount %{public}d", ret, rowCount);
        return {nullptr, std::move(recordIdRowIdMap)};
    }
    int columnIndex = 0;
    resultSet->GetColumnIndex(FC::CLOUD_ID, columnIndex);
    for (int rowId = 0; rowId < rowCount; ++rowId) {
        resultSet->GoToNextRow();
        string recordId = "";
        resultSet->GetString(columnIndex, recordId);
        recordIdRowIdMap.insert(make_pair(recordId, rowId));
    }
    return {std::move(resultSet), std::move(recordIdRowIdMap)};
}

int32_t CloudDiskDataHandler::OnFetchRecords(shared_ptr<vector<DKRecord>> &records, OnFetchParams &params)
{
    LOGI("on fetch %{public}zu records", records->size());
    int32_t ret = E_OK;
    auto recordIds = vector<string>();
    for (auto &record : *records) {
        recordIds.push_back(record.GetRecordId());
    }
    auto [resultSet, recordIdRowIdMap] = QueryLocalByCloudId(recordIds);
    if (resultSet == nullptr) {
        return E_RDB;
    }
    for (auto &record : *records) {
        if ((recordIdRowIdMap.find(record.GetRecordId()) == recordIdRowIdMap.end()) && !record.GetIsDelete()) {
            ret = PullRecordInsert(record, params);
        } else if (recordIdRowIdMap.find(record.GetRecordId()) != recordIdRowIdMap.end()) {
            resultSet->GoToRow(recordIdRowIdMap.at(record.GetRecordId()));
            if (record.GetIsDelete()) {
                ret = PullRecordDelete(record, *resultSet);
            } else {
                ret = PullRecordUpdate(record, *resultSet, params);
            }
        }
        if (ret != E_OK) {
            LOGE("recordId %s error %{public}d", record.GetRecordId().c_str(), ret);
            if (ret == E_RDB) {
                break;
            }
            ret = E_OK;
        }
    }
    return ret;
}

int32_t CloudDiskDataHandler::PullRecordInsert(DKRecord &record, OnFetchParams &params)
{
    LOGI("insert of record %{public}s", record.GetRecordId().c_str());

    /* check local file conflict */
    int32_t ret = PullRecordConflict(record);
    if (ret != E_OK) {
        LOGE("MetaFile Conflict failed %{public}d", ret);
        return ret;
    }
    ValuesBucket values;
    ret = localConvertor_.Convert(record, values);
    if (ret != E_OK) {
        return ret;
    }
    if (ret != E_OK) {
        LOGE("record to valuebucket failed, ret=%{public}d", ret);
        return ret;
    }
    int64_t rowId;
    values.PutInt(FC::POSITION, POSITION_CLOUD);
    values.PutInt(FC::DIRTY_TYPE, static_cast<int32_t>(DirtyType::TYPE_SYNCED));
    params.insertFiles.push_back(values);
    ret = Insert(rowId, FC::FILES_TABLE, values);
    return E_OK;
}

int32_t CloudDiskDataHandler::PullRecordConflict(DKRecord &record)
{
    int32_t ret = E_OK;
    DKRecordData data;
    record.GetRecordData(data);
    if (data.find(DK_FILE_NAME) == data.end() || data.find(DK_DIRECTLY_RECYCLED) == data.end()) {
        return E_INVALID_ARGS;
    }
    string fullName = "";
    int64_t recycledTime = 0;
    string parentId = localConvertor_.GetParentCloudId(data);
    data[DK_FILE_NAME].GetString(fullName);
    data[DK_FILE_TIME_RECYCLED].GetLong(recycledTime);
    if (recycledTime > 0) {
        return E_OK;
    }
    size_t lastDot = fullName.rfind('.');
    if (lastDot == std::string::npos) {
        lastDot = fullName.length();
    }
    string fileName = fullName.substr(0, lastDot);
    NativeRdb::AbsRdbPredicates predicates = NativeRdb::AbsRdbPredicates(FC::FILES_TABLE);
    predicates.EqualTo(FC::PARENT_CLOUD_ID, parentId);
    predicates.EqualTo(FC::FILE_TIME_RECYCLED, 0);
    predicates.NotEqualTo(FC::CLOUD_ID, record.GetRecordId());
    predicates.Like(FC::FILE_NAME, fileName + "%");
    auto resultSet = Query(predicates, {FC::CLOUD_ID, FC::FILE_NAME});
    ret = HandleConflict(resultSet, fullName, lastDot);
    return ret;
}

int32_t CloudDiskDataHandler::HandleConflict(const std::shared_ptr<NativeRdb::ResultSet> resultSet,
                                             string &fullName,
                                             const int &lastDot)
{
    if (resultSet == nullptr) {
        LOGE(" resultSet is null");
        return E_RDB;
    }
    int32_t count = 0;
    int32_t ret = resultSet->GetRowCount(count);
    if (ret != E_OK || count < 0) {
        return E_RDB;
    }
    if (count == 0) {
        return E_OK;
    }
    string dbFileName = "";
    string renameFileCloudId = "";
    ret = FindRenameFile(resultSet, renameFileCloudId, fullName, lastDot);
    if (ret != E_OK) {
        LOGW("try to rename too many times");
        return E_INVAL_ARG;
    }
    if (!renameFileCloudId.empty()) {
        ret = ConflictReName(renameFileCloudId, fullName);
    } else {
        LOGD("no same name");
    }
    return ret;
}

int32_t CloudDiskDataHandler::FindRenameFile(const std::shared_ptr<NativeRdb::ResultSet> resultSet,
                                             string &renameFileCloudId,
                                             string &fullName,
                                             const int &lastDot)
{
    string fileName = fullName.substr(0, lastDot);
    string extension = fullName.substr(lastDot);
    string dbFileName = "";
    int32_t renameTimes = 0;
    bool findSameFile = true;
    while (findSameFile) {
        renameTimes++;
        findSameFile = false;
        resultSet->GoToFirstRow();
        string dbFileCloudId = "";
        do {
            DataConvertor::GetString(FC::FILE_NAME, dbFileName, *resultSet);
            if (fullName == dbFileName) {
                LOGD("find same name file try to rename");
                fullName = fileName + "(" + to_string(renameTimes) + ")" + extension;
                DataConvertor::GetString(FC::CLOUD_ID, dbFileCloudId, *resultSet);
                findSameFile = true;
                break;
            }
        } while (resultSet->GoToNextRow() == 0);
        if (fullName == fileName + extension) {
            renameFileCloudId = dbFileCloudId;
        }
        if (renameTimes >= MAX_RENAME) {
            break;
        }
    }
    if (findSameFile) {
        LOGW("try to rename too many times");
        return E_INVAL_ARG;
    }
    return E_OK;
}

int32_t CloudDiskDataHandler::ConflictReName(const string &cloudId, string newFileName)
{
    LOGD("do conflict rename, new fileName is %{public}s", newFileName.c_str());
    ValuesBucket values;
    values.PutString(FC::FILE_NAME, newFileName);
    int32_t changedRows;
    string whereClause = FC::CLOUD_ID + " = ?";
    vector<string> whereArgs = {cloudId};
    values.PutLong(FileColumn::META_TIME_EDITED, UTCTimeMilliSeconds());
    int32_t ret = Update(changedRows, values, whereClause, whereArgs);
    if (ret != E_OK) {
        LOGE("update db, rename fail");
        return E_RDB;
    }
    return E_OK;
}

static int32_t IsEditTimeChange(const DKRecord &record, NativeRdb::ResultSet &local, bool &isChange)
{
    int64_t localEditTime = 0;
    int64_t recordEditTime = record.GetEditedTime();
    int32_t ret = DataConvertor::GetLong(FC::FILE_TIME_EDITED, localEditTime, local);
    if (ret != E_OK) {
        return ret;
    }
    isChange = localEditTime != recordEditTime;
    return E_OK;
}

int32_t CloudDiskDataHandler::PullRecordUpdate(DKRecord &record, NativeRdb::ResultSet &local, OnFetchParams &params)
{
    int32_t ret = E_OK;
    LOGI("update of record %s", record.GetRecordId().c_str());
    if (IsLocalDirty(local)) {
        LOGI("local record dirty, ignore cloud update");
        return E_OK;
    }
    ValuesBucket values;
    ret = localConvertor_.Convert(record, values);
    if (ret != E_OK) {
        return ret;
    }
    bool isEditTimeChange = false;
    ret = IsEditTimeChange(record, local, isEditTimeChange);
    if (ret != E_OK) {
        return ret;
    }
    if (FileIsLocal(local)) {
        string localPath = CloudFileUtils::GetLocalFilePath(record.GetRecordId(), bundleName_, userId_);
        if (CloudFileUtils::LocalWriteOpen(localPath)) {
            return SetRetry(record.GetRecordId());
        }
        if (isEditTimeChange) {
            LOGD("cloud file DATA changed, %s", record.GetRecordId().c_str());
            ret = unlink(localPath.c_str());
            if (ret != 0) {
                LOGE("unlink local failed, errno %{public}d", errno);
            }
            values.PutInt(FC::POSITION, static_cast<int32_t>(POSITION_CLOUD));
        }
    }
    LOGD("cloud file changed, %s", record.GetRecordId().c_str());
    ret = PullRecordConflict(record);
    if (ret != E_OK) {
        LOGE("MetaFile Conflict failed %{public}d", ret);
        return E_RDB;
    }
    /* update rdb */
    values.PutInt(FC::DIRTY_TYPE, static_cast<int32_t>(DirtyTypes::TYPE_SYNCED));
    int32_t changedRows;
    string whereClause = FC::CLOUD_ID + " = ?";
    ret = Update(changedRows, values, whereClause, {record.GetRecordId()});
    if (ret != E_OK) {
        LOGE("rdb update failed, err=%{public}d", ret);
        return E_RDB;
    }
    LOGD("update of record success, change %{pubilc}d row", changedRows);
    return E_OK;
}

int32_t CloudDiskDataHandler::PullRecordDelete(DKRecord &record, NativeRdb::ResultSet &local)
{
    LOGI("delete of record %s", record.GetRecordId().c_str());
    bool isLocal = FileIsLocal(local);
    string path = CloudFileUtils::GetLocalFilePath(record.GetRecordId(), bundleName_, userId_);
    if (FileIsRecycled(local) || !isLocal) {
        if (isLocal) {
            LOGI("force delete instead");
            int32_t ret = unlink(path.c_str());
            if (ret != 0) {
                LOGE("unlink local failed, errno %{public}d", errno);
            }
        }
        int32_t deletedRows;
        int32_t ret = Delete(deletedRows, FC::CLOUD_ID + " = ?", {record.GetRecordId()});
        if (ret != E_OK) {
            LOGE("delete in rdb failed, ret:%{public}d", ret);
            return E_INVAL_ARG;
        }
        return E_OK;
    }
    if (IsLocalDirty(local)) {
        LOGI("local record dirty, ignore cloud delete");
        int32_t changedRows = -1;
        ValuesBucket values;
        values.PutInt(FC::VERSION, static_cast<int32_t>(record.GetVersion()));
        string whereClause = FC::CLOUD_ID + " = ?";
        int32_t ret = Update(changedRows, values, whereClause, {record.GetRecordId()});
        if (ret != E_OK) {
            LOGE("Update in rdb failed, ret:%{public}d", ret);
            return ret;
        }
    }

    if (CloudDisk::CloudFileUtils::LocalWriteOpen(path)) {
        return SetRetry(record.GetRecordId());
    }
    return RecycleFile(record.GetRecordId());
}

int CloudDiskDataHandler::RecycleFile(const string &recordId)
{
    LOGI("recycle of record %s", recordId.c_str());
    ValuesBucket values;
    values.PutInt(FC::DIRTY_TYPE, static_cast<int32_t>(DirtyType::TYPE_NEW));
    values.PutInt(FC::OPERATE_TYPE, static_cast<int32_t>(CloudDisk::OperationType::DELETE));
    values.PutInt(FC::POSITION, POSITION_LOCAL);
    values.PutLong(FC::FILE_TIME_RECYCLED, UTCTimeMilliSeconds());
    values.PutLong(FC::VERSION, 0);
    int32_t changedRows;
    string whereClause = FC::CLOUD_ID + " = ?";
    int ret = Update(changedRows, values, whereClause, {recordId});
    if (ret != E_OK) {
        LOGE("rdb update failed, err=%{public}d", ret);
        return E_RDB;
    }
    return E_OK;
}

int CloudDiskDataHandler::SetRetry(const string &recordId)
{
    LOGI("set retry of record %s", recordId.c_str());
    int updateRows;
    ValuesBucket values;
    values.PutInt(FC::DIRTY_TYPE, static_cast<int32_t>(DirtyType::TYPE_RETRY));
    string whereClause = FC::CLOUD_ID + " = ?";
    int32_t ret = Update(updateRows, values, whereClause, {recordId});
    if (ret != E_OK) {
        LOGE("update retry flag failed, ret=%{public}d", ret);
        return E_RDB;
    }
    return E_OK;
}

int32_t CloudDiskDataHandler::GetCheckRecords(vector<DriveKit::DKRecordId> &checkRecords,
                                              const shared_ptr<std::vector<DriveKit::DKRecord>> &records)
{
    auto recordIds = vector<string>();
    for (const auto &record : *records) {
        recordIds.push_back(record.GetRecordId());
    }
    auto [resultSet, recordIdRowIdMap] = QueryLocalByCloudId(recordIds);
    if (resultSet == nullptr) {
        return E_RDB;
    }
    for (const auto &record : *records) {
        if ((recordIdRowIdMap.find(record.GetRecordId()) == recordIdRowIdMap.end()) && !record.GetIsDelete()) {
            checkRecords.push_back(record.GetRecordId());
        } else if (recordIdRowIdMap.find(record.GetRecordId()) != recordIdRowIdMap.end()) {
            resultSet->GoToRow(recordIdRowIdMap.at(record.GetRecordId()));
            int64_t version = 0;
            DataConvertor::GetLong(FC::VERSION, version, *resultSet);
            if (record.GetVersion() != static_cast<unsigned long>(version) &&
                (!IsLocalDirty(*resultSet) || record.GetIsDelete())) {
                checkRecords.push_back(record.GetRecordId());
            }
        } else {
            LOGE("recordId %s has multiple file in db!", record.GetRecordId().c_str());
        }
    }
    return E_OK;
}

int32_t CloudDiskDataHandler::Clean(const int action)
{
    LOGD("Enter function CloudDiskDataHandler::Clean");
    CloudPrefImpl cloudPrefImpl(userId_, bundleName_, FileColumn::FILES_TABLE);
    cloudPrefImpl.Delete("rootId");
    localConvertor_.SetRootId("");
    createConvertor_.SetRootId("");
    deleteConvertor_.SetRootId("");
    mdirtyConvertor_.SetRootId("");
    fdirtyConvertor_.SetRootId("");
    RETURN_ON_ERR(CleanCloudRecord(action));
    return E_OK;
}

int32_t CloudDiskDataHandler::CleanCloudRecord(const int32_t action)
{
    int32_t ret = E_OK;
    NativeRdb::AbsRdbPredicates cleanPredicates = NativeRdb::AbsRdbPredicates(FC::FILES_TABLE);
    cleanPredicates.EqualTo(FC::POSITION, static_cast<int32_t>(POSITION_LOCAL));
    cleanPredicates.Or()->EqualTo(FC::POSITION, static_cast<int32_t>(POSITION_BOTH));
    cleanPredicates.Limit(DELETE_LIMIT_SIZE);
    int32_t count = 0;
    vector<NativeRdb::ValueObject> deleteIds;
    string cloudId = "";
    do {
        auto resultSet = Query(cleanPredicates, {FC::CLOUD_ID});
        ret = resultSet->GetRowCount(count);
        if (ret != E_OK) {
            LOGE("get row count error , res: %{public}d", ret);
            break;
        }
        while (resultSet->GoToNextRow() == 0) {
            ret = DataConvertor::GetString(FC::CLOUD_ID, cloudId, *resultSet);
            string localPath = CloudFileUtils::GetLocalFilePath(cloudId, bundleName_, userId_);
            if (ret != E_OK) {
                LOGE("Get path wrong");
                return E_INVAL_ARG;
            }
            if (unlink(localPath.c_str()) != 0) {
                LOGE("unlink local failed, errno %{public}d", errno);
            }
            deleteIds.emplace_back(cloudId);
        }
        BatchDetete(FC::FILES_TABLE, FC::CLOUD_ID, deleteIds);
        deleteIds.clear();
    } while (count != 0);
    int32_t deletedRows;
    ret = Delete(deletedRows, "", {});
    return ret;
}

int32_t CloudDiskDataHandler::OnDownloadAssets(
    const std::map<DriveKit::DKDownloadAsset, DriveKit::DKDownloadResult> &resultMap)
{
    return E_OK;
}
int32_t CloudDiskDataHandler::GetCreatedRecords(vector<DKRecord> &records)
{
    while (records.size() == 0) {
        NativeRdb::AbsRdbPredicates createPredicates = NativeRdb::AbsRdbPredicates(FC::FILES_TABLE);
        createPredicates
            .EqualTo(FC::DIRTY_TYPE, to_string(static_cast<int32_t>(CloudDisk::DirtyType::TYPE_NEW)))
            ->And()->EqualTo(FC::FILE_TIME_RECYCLED, "0");
        if (!createFailSet_.empty()) {
            createPredicates.And()->NotIn(FC::CLOUD_ID, createFailSet_);
        }
        createPredicates.OrderByDesc(FC::IS_DIRECTORY);
        createPredicates.OrderByAsc(FC::FILE_TIME_ADDED);
        createPredicates.OrderByAsc(FC::FILE_SIZE);
        createPredicates.Limit(LIMIT_SIZE);
        auto results = Query(createPredicates, FileColumn::DISK_CLOUD_SYNC_COLUMNS);
        if (results == nullptr) {
            LOGE("get nullptr created result");
            return E_RDB;
        }
        int32_t ret = createConvertor_.ResultSetToRecords(move(results), records);
        if (ret != E_OK) {
            if (ret == E_STOP) {
                return E_OK;
            }
            LOGE("result set to records err %{public}d", ret);
            return ret;
        }
    }
    return E_OK;
}
int32_t CloudDiskDataHandler::GetDeletedRecords(vector<DKRecord> &records)
{
    NativeRdb::AbsRdbPredicates deletePredicates = NativeRdb::AbsRdbPredicates(FC::FILES_TABLE);
    deletePredicates
        .EqualTo(FC::DIRTY_TYPE, to_string(static_cast<int32_t>(CloudDisk::DirtyType::TYPE_DELETED)));
    if (!modifyFailSet_.empty()) {
        deletePredicates.And()->NotIn(FC::CLOUD_ID, modifyFailSet_);
    }
    deletePredicates.Limit(DELETE_BATCH_NUM);
    auto results = Query(deletePredicates, FileColumn::DISK_CLOUD_SYNC_COLUMNS);
    if (results == nullptr) {
        LOGE("get nullptr deleted result");
        return E_RDB;
    }
    int ret = deleteConvertor_.ResultSetToRecords(move(results), records);
    if (ret != E_OK) {
        LOGE("result set to records err %{public}d", ret);
        return ret;
    }
    return E_OK;
}
int32_t CloudDiskDataHandler::GetMetaModifiedRecords(vector<DKRecord> &records)
{
    NativeRdb::AbsRdbPredicates updatePredicates = NativeRdb::AbsRdbPredicates(FC::FILES_TABLE);
    updatePredicates
        .EqualTo(FC::DIRTY_TYPE, to_string(static_cast<int32_t>(CloudDisk::DirtyType::TYPE_MDIRTY)));
    if (!modifyFailSet_.empty()) {
        updatePredicates.And()->NotIn(FC::CLOUD_ID, modifyFailSet_);
    }
    updatePredicates.Limit(MODIFY_BATCH_NUM);
    auto results = Query(updatePredicates, FileColumn::DISK_CLOUD_SYNC_COLUMNS);
    if (results == nullptr) {
        LOGE("get nullptr meta modified result");
        return E_RDB;
    }
    int ret = mdirtyConvertor_.ResultSetToRecords(move(results), records);
    if (ret != E_OK) {
        LOGE("meta modified result set to records err %{public}d", ret);
        return ret;
    }
    return E_OK;
}
int32_t CloudDiskDataHandler::GetFileModifiedRecords(vector<DKRecord> &records)
{
    while (records.size() == 0) {
        NativeRdb::AbsRdbPredicates updatePredicates = NativeRdb::AbsRdbPredicates(FC::FILES_TABLE);
        updatePredicates
            .EqualTo(FC::DIRTY_TYPE, to_string(static_cast<int32_t>(CloudDisk::DirtyType::TYPE_FDIRTY)));
        if (!modifyFailSet_.empty()) {
            updatePredicates.And()->NotIn(FC::CLOUD_ID, modifyFailSet_);
        }
        updatePredicates.OrderByAsc(FC::FILE_SIZE);
        updatePredicates.Limit(LIMIT_SIZE);
        auto results = Query(updatePredicates, FileColumn::DISK_CLOUD_SYNC_COLUMNS);
        if (results == nullptr) {
            LOGE("get nullptr file modified result");
            return E_RDB;
        }
        int32_t ret = fdirtyConvertor_.ResultSetToRecords(move(results), records);
        if (ret != E_OK) {
            if (ret == E_STOP) {
                return E_OK;
            }
            LOGE("file modified result set to records err %{public}d", ret);
            return ret;
        }
    }
    return E_OK;
}

int32_t CloudDiskDataHandler::OnCreateRecords(const map<DKRecordId, DKRecordOperResult> &map)
{
    unordered_map<string, LocalInfo> localMap;
    int32_t ret = GetLocalInfo(map, localMap, FC::CLOUD_ID);
    if (ret != E_OK) {
        LOGE("get created local match info err %{public}d", ret);
        return ret;
    }
    for (auto &entry : map) {
        int32_t err;
        const DKRecordOperResult &result = entry.second;
        if (result.IsSuccess()) {
            err = OnCreateRecordSuccess(entry, localMap);
        } else {
            err = OnRecordFailed(entry);
        }
        if (err != E_OK) {
            createFailSet_.push_back(entry.first);
            LOGE("create record fail, cloud id: %{private}s", entry.first.c_str());
        }
        GetReturn(err, ret);
    }
    return ret;
}
int32_t CloudDiskDataHandler::OnDeleteRecords(const map<DKRecordId, DKRecordOperResult> &map)
{
    int32_t ret = E_OK;
    for (auto &entry : map) {
        const DKRecordOperResult &result = entry.second;
        int32_t err;
        if (result.IsSuccess()) {
            err = OnDeleteRecordSuccess(entry);
        } else {
            err = OnRecordFailed(entry);
        }
        if (err != E_OK) {
            modifyFailSet_.push_back(entry.first);
            LOGE("delete record fail, cloud id: %{private}s", entry.first.c_str());
        }
        GetReturn(err, ret);
    }
    return ret;
}
int32_t CloudDiskDataHandler::OnModifyMdirtyRecords(const map<DKRecordId, DKRecordOperResult> &map)
{
    unordered_map<string, LocalInfo> localMap;
    int32_t ret = GetLocalInfo(map, localMap, FC::CLOUD_ID);
    if (ret != E_OK) {
        LOGE("get meta modified local match info err %{public}d", ret);
        return ret;
    }
    for (auto &entry : map) {
        int32_t err;
        const DKRecordOperResult &result = entry.second;
        if (result.IsSuccess()) {
            err = OnModifyRecordSuccess(entry, localMap);
        } else {
            err = OnRecordFailed(entry);
        }
        if (err != E_OK) {
            modifyFailSet_.push_back(entry.first);
            LOGE("modify mdirty record fail, cloud id: %{private}s", entry.first.c_str());
        }
        GetReturn(err, ret);
    }
    return ret;
}
int32_t CloudDiskDataHandler::OnModifyFdirtyRecords(const map<DKRecordId, DKRecordOperResult> &map)
{
    unordered_map<string, LocalInfo> localMap;
    int32_t ret = GetLocalInfo(map, localMap, FC::CLOUD_ID);
    if (ret != E_OK) {
        LOGE("get file modified local match info err %{public}d", ret);
        return ret;
    }
    for (auto &entry : map) {
        int32_t err;
        const DKRecordOperResult &result = entry.second;
        if (result.IsSuccess()) {
            err = OnModifyRecordSuccess(entry, localMap);
        } else {
            err = OnRecordFailed(entry);
        }
        if (err != E_OK) {
            modifyFailSet_.push_back(entry.first);
            LOGE("modify fdirty record fail, cloud id: %{private}s", entry.first.c_str());
        }
        GetReturn(err, ret);
    }
    return ret;
}
int32_t CloudDiskDataHandler::OnDownloadAssets(const DriveKit::DKDownloadAsset &asset)
{
    return E_OK;
}
static int32_t UpdateFileAsset(DKRecordData data, ValuesBucket &values)
{
    DriveKit::DKAsset content;
    if (data.find(DK_FILE_CONTENT) == data.end()) {
        LOGE("record data cannot find content");
        return E_INVAL_ARG;
    }
    DKLocalErrorCode ret = data[DK_FILE_CONTENT].GetAsset(content);
    if (ret != DKLocalErrorCode::NO_ERROR) {
        LOGE("record field get content fail, ret %{public}d", ret);
        return E_INVAL_ARG;
    }
    int64_t fileSize = static_cast<int64_t>(content.size);
    string sha256 = content.hash;
    values.PutLong(FC::FILE_SIZE, fileSize);
    values.PutString(FC::FILE_SHA256, sha256);
    return E_OK;
}
int32_t CloudDiskDataHandler::OnCreateRecordSuccess(
    const pair<DKRecordId, DKRecordOperResult> &entry,
    const unordered_map<string, LocalInfo> &localMap)
{
    auto record = entry.second.GetDKRecord();
    if (localMap.find(entry.first) == localMap.end()) {
        return E_OK;
    }
    DKRecordData data;
    record.GetRecordData(data);
    ValuesBucket valuesBucket;
    if (UpdateFileAsset(data, valuesBucket)) {
        LOGE("update file content fail");
    }
    valuesBucket.PutInt(FC::POSITION, POSITION_BOTH);
    valuesBucket.PutLong(FC::VERSION, record.GetVersion());
    if (IsTimeChanged(record, localMap, entry.first, FC::FILE_TIME_EDITED)) {
        valuesBucket.PutInt(FC::DIRTY_TYPE, static_cast<int32_t>(CloudDisk::DirtyType::TYPE_FDIRTY));
    } else if (IsTimeChanged(record, localMap, entry.first, FC::META_TIME_EDITED)) {
        valuesBucket.PutInt(FC::DIRTY_TYPE, static_cast<int32_t>(CloudDisk::DirtyType::TYPE_MDIRTY));
    } else {
        valuesBucket.PutInt(FC::DIRTY_TYPE, static_cast<int32_t>(CloudDisk::DirtyType::TYPE_SYNCED));
    }
    int32_t changedRows;
    int32_t ret = Update(changedRows, valuesBucket, FC::CLOUD_ID + " = ?", { entry.first });
    if (ret != E_OK) {
        LOGE("on create records update synced err %{public}d, cloudId %{private}s", ret, entry.first.c_str());
        return ret;
    }
    return E_OK;
}
int32_t CloudDiskDataHandler::OnDeleteRecordSuccess(const std::pair<DKRecordId, DKRecordOperResult> &entry)
{
    int32_t deletedRows;
    int32_t ret = Delete(deletedRows, FC::CLOUD_ID + " = ?", { entry.first });
    if (ret != E_OK) {
        LOGE("on delete records update err %{public}d, cloudId %{private}s", ret, entry.first.c_str());
        return ret;
    }
    return E_OK;
}
int32_t CloudDiskDataHandler::OnModifyRecordSuccess(
    const pair<DKRecordId, DKRecordOperResult> &entry,
    const unordered_map<string, LocalInfo> &localMap)
{
    auto record = entry.second.GetDKRecord();
    DKRecordData data;
    record.GetRecordData(data);
    if (data.find(DK_FILE_ATTRIBUTES) == data.end()) {
        LOGE("record data cannot find attributes");
        return E_INVAL_ARG;
    }
    DriveKit::DKRecordFieldMap attributes;
    data[DK_FILE_ATTRIBUTES].GetRecordMap(attributes);
    if (attributes.find(DK_META_TIME_EDITED) == attributes.end()) {
        LOGE("attributes cannot find meta time edited");
        return E_INVAL_ARG;
    }
    int64_t meta_date_modified;
    if (attributes[DK_META_TIME_EDITED].GetLong(meta_date_modified) != DKLocalErrorCode::NO_ERROR) {
        LOGE("obtain meta time edited error, cloudId %{private}s", entry.first.c_str());
        return E_INVAL_ARG;
    }
    if (localMap.find(entry.first) == localMap.end()) {
        return E_OK;
    }
    int32_t changedRows;
    ValuesBucket valuesBucket;
    valuesBucket.PutLong(FC::VERSION, record.GetVersion());
    if (IsTimeChanged(record, localMap, entry.first, FC::FILE_TIME_EDITED)) {
        valuesBucket.PutInt(FC::DIRTY_TYPE, static_cast<int32_t>(CloudDisk::DirtyType::TYPE_FDIRTY));
    } else if (IsTimeChanged(record, localMap, entry.first, FC::META_TIME_EDITED)) {
        valuesBucket.PutInt(FC::DIRTY_TYPE, static_cast<int32_t>(CloudDisk::DirtyType::TYPE_MDIRTY));
    } else {
        valuesBucket.PutInt(FC::DIRTY_TYPE, static_cast<int32_t>(CloudDisk::DirtyType::TYPE_SYNCED));
    }
    int32_t ret = Update(changedRows, valuesBucket, FC::CLOUD_ID + " = ?", { entry.first });
    if (ret != E_OK) {
        LOGE("on meta modify records update synced err %{public}d, cloudId %{private}s", ret, entry.first.c_str());
        valuesBucket.Clear();
        valuesBucket.PutLong(FC::VERSION, record.GetVersion());
        ret = Update(changedRows, valuesBucket, FC::CLOUD_ID + " = ?", { entry.first });
        if (ret != E_OK) {
            LOGE("update meta record version err %{public}d, cloudId %{private}s", ret, entry.first.c_str());
            return ret;
        }
    }
    return E_OK;
}
bool CloudDiskDataHandler::IsTimeChanged(const DriveKit::DKRecord &record,
    const unordered_map<string, LocalInfo> &localMap, const string &cloudId, const string &type)
{
    int64_t cloudTime = 0;
    int64_t localTime = 0;
    auto it = localMap.find(cloudId);
    if (it == localMap.end()) {
        return true;
    }
    DKRecordData data;
    record.GetRecordData(data);
    if (data.find(DK_FILE_ATTRIBUTES) == data.end()) {
        LOGE("record data cannot find attributes");
        return false;
    }
    DriveKit::DKRecordFieldMap attributes;
    data[DK_FILE_ATTRIBUTES].GetRecordMap(attributes);
    if (type == FC::FILE_TIME_EDITED) {
        localTime = it->second.fdirtyTime;
        if (attributes[DK_FILE_TIME_EDITED].GetLong(cloudTime) != DKLocalErrorCode::NO_ERROR) {
            LOGE("obtain file time edited error, cloudId %{private}s", cloudId.c_str());
            return false;
        }
    } else {
        localTime = it->second.mdirtyTime;
        if (attributes[DK_META_TIME_EDITED].GetLong(cloudTime) != DKLocalErrorCode::NO_ERROR) {
            LOGE("obtain meta time edited error, cloudId %{private}s", cloudId.c_str());
            return false;
        }
    }
    if (localTime == cloudTime) {
        return false;
    }
    return true;
}
int32_t CloudDiskDataHandler::GetLocalInfo(const map<DKRecordId, DKRecordOperResult> &map,
    unordered_map<string, LocalInfo> &infoMap, const string &type)
{
    vector<string> cloudId;
    if (type != FC::CLOUD_ID) {
        LOGE("query type for getting local info is invalid, %{public}s", type.c_str());
        return E_UNKNOWN;
    }
    for (auto &entry : map) {
        cloudId.push_back(entry.first);
    }
    NativeRdb::AbsRdbPredicates createPredicates = NativeRdb::AbsRdbPredicates(FC::FILES_TABLE);
    createPredicates.In(type, cloudId);
    auto resultSet = Query(createPredicates, FileColumn::DISK_ON_UPLOAD_COLUMNS);
    if (resultSet == nullptr) {
        LOGE("query rdb err");
        return E_RDB;
    }
    return BuildInfoMap(move(resultSet), infoMap, type);
}
int32_t CloudDiskDataHandler::BuildInfoMap(const shared_ptr<NativeRdb::ResultSet> resultSet,
    unordered_map<string, LocalInfo> &infoMap, const string &type)
{
    int32_t idIndex = -1;
    int32_t mTimeIndex = -1;
    int32_t metaTimeIndex = -1;
    RETURN_ON_ERR(resultSet->GetColumnIndex(type, idIndex));
    RETURN_ON_ERR(resultSet->GetColumnIndex(FC::FILE_TIME_EDITED, mTimeIndex));
    RETURN_ON_ERR(resultSet->GetColumnIndex(FC::META_TIME_EDITED, metaTimeIndex));
    while (resultSet->GoToNextRow() == 0) {
        string idValue;
        int64_t mTime;
        int64_t metaTime;
        if (resultSet->GetString(idIndex, idValue) == 0 && resultSet->GetLong(mTimeIndex, mTime) == 0 &&
            resultSet->GetLong(metaTimeIndex, metaTime) == 0) {
            infoMap.insert({ idValue, { metaTime, mTime } });
        }
    }
    return E_OK;
}

void CloudDiskDataHandler::Reset()
{
    modifyFailSet_.clear();
    createFailSet_.clear();
}

int32_t CloudDiskDataHandler::GetDownloadAsset(std::string uri,
                                               vector<DriveKit::DKDownloadAsset> &outAssetsToDownload)
{
    string path;
    int32_t ret = AppFileService::SandboxHelper::GetPhysicalPath(uri, to_string(userId_), path);
    if (ret != E_OK) {
        LOGE("get physical path failed");
        return E_INVAL_ARG;
    }
    string cloudId = CloudFileUtils::GetCloudId(path);
    if (cloudId.empty()) {
        LOGE("get cloud id failed, errno %{public}d", errno);
        return E_INVAL_ARG;
    }
    NativeRdb::AbsRdbPredicates predicates = NativeRdb::AbsRdbPredicates(FileColumn::FILES_TABLE);
    predicates.SetWhereClause(FileColumn::CLOUD_ID + " = ?");
    predicates.SetWhereArgs({cloudId});
    predicates.Limit(LIMIT_SIZE);
    auto resultSet = Query(predicates, {FileColumn::IS_DIRECTORY});
    if (resultSet == nullptr) {
        LOGE("get nullptr created result");
        return E_RDB;
    }
    int32_t rowCount = 0;
    ret = resultSet->GetRowCount(rowCount);
    if (ret != 0) {
        LOGE("result set get row count err %{public}d", ret);
        return E_RDB;
    }
    int32_t columnIndex = 0;
    int32_t isDirectory = 0;
    resultSet->GetColumnIndex(FileColumn::IS_DIRECTORY, columnIndex);
    resultSet->GetInt(columnIndex, isDirectory);
    if (isDirectory == FILE) {
        AppendFileToDownload(cloudId, "content", outAssetsToDownload);
    } else {
        return ERR_INVALID_VALUE;
    }
    return E_OK;
}

void CloudDiskDataHandler::AppendFileToDownload(const string &cloudId,
                                                const std::string &fieldKey,
                                                std::vector<DKDownloadAsset> &assetsToDownload)
{
    DKDownloadAsset downloadAsset;
    downloadAsset.recordType = recordType_;
    downloadAsset.fieldKey = fieldKey;
    downloadAsset.recordId = cloudId;
    downloadAsset.downLoadPath = CloudFileUtils::GetLocalBucketPath(cloudId, bundleName_, userId_);
    downloadAsset.asset.assetName = cloudId + CloudFileUtils::TMP_SUFFIX;
    ForceCreateDirectory(downloadAsset.downLoadPath);
    assetsToDownload.push_back(downloadAsset);
}

int32_t CloudDiskDataHandler::OnDownloadSuccess(const DriveKit::DKDownloadAsset &asset)
{
    string tmpLocalPath = asset.downLoadPath + "/" + asset.asset.assetName;
    string localPath = CloudFileUtils::GetPathWithoutTmp(tmpLocalPath);
    string cloudId = CloudFileUtils::GetPathWithoutTmp(asset.asset.assetName);
    if (rename(tmpLocalPath.c_str(), localPath.c_str()) != 0) {
        LOGE("err rename, errno: %{public}d, tmpLocalPath: %s, localPath: %s",
             errno, tmpLocalPath.c_str(), localPath.c_str());
    }
    // update rdb
    int32_t changedRows;
    ValuesBucket valuesBucket;
    valuesBucket.PutInt(FC::POSITION, POSITION_BOTH);
    string whereClause = FC::CLOUD_ID + " = ?";
    vector<string> whereArgs = {cloudId};
    int32_t ret = Update(changedRows, valuesBucket, whereClause, whereArgs);
    if (ret != E_OK) {
        LOGE("on download file from cloud err %{public}d", ret);
    }
    return ret;
}

int32_t CloudDiskDataHandler::CleanCache(const string &uri)
{
    string path;
    int32_t ret = AppFileService::SandboxHelper::GetPhysicalPath(uri, to_string(userId_), path);
    if (ret != E_OK) {
        LOGE("get physical path failed");
        return E_INVAL_ARG;
    }
    string cloudId = CloudFileUtils::GetCloudId(path);
    if (cloudId.empty()) {
        LOGE("get cloud id failed, errno %{public}d", errno);
        return E_INVAL_ARG;
    }
    string bucketPaht = CloudFileUtils::GetLocalBucketPath(cloudId, bundleName_, userId_) + "/" + cloudId;
    ret = unlink(bucketPaht.c_str());
    if (ret != 0) {
        LOGE("Unlink failed errno %{public}d", errno);
        return E_DELETE_FAILED;
    }
    ValuesBucket values;
    values.PutInt(FC::POSITION, POSITION_CLOUD);
    int32_t changedRows = 0;
    string whereClause = FC::CLOUD_ID + " = ?";
    ret = Update(changedRows, values, whereClause, {cloudId});
    if (ret != E_OK) {
        LOGE("rdb update failed, err=%{public}d", ret);
        return E_RDB;
    }
    return E_OK;
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
