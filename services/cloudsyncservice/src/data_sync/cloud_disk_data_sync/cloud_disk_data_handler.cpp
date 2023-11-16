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
#include <tuple>
#include <unistd.h>
#include "clouddisk_db_const.h"
#include "data_sync_const.h"
#include "dfs_error.h"
#include "dk_record.h"
#include "file_column.h"
#include "rdb_errno.h"
#include "utils_log.h"
#include "cloud_file_utils.h"

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
    cond.limitRes = LIMIT_SIZE;
    cond.recordType = recordType_;
    if (isChecking_) {
        cond.desiredKeys = checkedKeys_;
    } else {
        cond.desiredKeys = desiredKeys_;
    }
    cond.fullKeys = desiredKeys_;
}
int64_t CloudDiskDataHandler::UTCTimeSeconds()
{
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 0;
    clock_gettime(CLOCK_REALTIME, &ts);
    return static_cast<int64_t>(ts.tv_sec);
}

int32_t CloudDiskDataHandler::GetRetryRecords(std::vector<DriveKit::DKRecordId> &records)
{
    NativeRdb::AbsRdbPredicates retryPredicates = NativeRdb::AbsRdbPredicates(FC::FILES_TABLE);
    retryPredicates.SetWhereClause(FC::DIRTY_TYPE + " = ? ");
    retryPredicates.SetWhereArgs({to_string(static_cast<int32_t>(DirtyType::TYPE_RETRY))});
    retryPredicates.Limit(LIMIT_SIZE);
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
    return false;
}
static bool FileIsRecycled(NativeRdb::ResultSet &local)
{
    return false;
}
static bool FileIsLocal(NativeRdb::ResultSet &local)
{
    return false;
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
    bool comflag = false;
    int ret = PullRecordConflict(record, comflag);
    if (comflag) {
        LOGI("Conflict:same document no Insert");
        return E_OK;
    } else if (ret != E_OK) {
        LOGE("MetaFile Conflict failed %{public}d", ret);
    }
    ValuesBucket values;
    localConvertor_.Convert(record, values);
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

int32_t CloudDiskDataHandler::PullRecordConflict(DKRecord &record, bool &comflag)
{
    return E_OK;
}

int32_t CloudDiskDataHandler::PullRecordUpdate(DKRecord &record, NativeRdb::ResultSet &local, OnFetchParams &params)
{
    LOGI("update of record %s", record.GetRecordId().c_str());
    if (IsLocalDirty(local)) {
        LOGI("local record dirty, ignore cloud update");
        return E_OK;
    }
    int ret = E_OK;
    if (FileIsLocal(local)) {
        string localPath = CloudFileUtils::GetLocalFilePath(record.GetRecordId(), bundleName_, userId_);
    }
    LOGI("cloud file META changed, %s", record.GetRecordId().c_str());
    /* update rdb */
    ValuesBucket values;
    localConvertor_.Convert(record, values);
    values.PutInt(FC::DIRTY_TYPE, static_cast<int32_t>(DirtyTypes::TYPE_SYNCED));
    int32_t changedRows;
    string whereClause = FC::CLOUD_ID + " = ?";
    ret = Update(changedRows, values, whereClause, {record.GetRecordId()});
    if (ret != E_OK) {
        LOGE("rdb update failed, err=%{public}d", ret);
        return E_RDB;
    }
    LOGI("update of record success");
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
        return ClearCloudInfo(record.GetRecordId());
    }
    return RecycleFile(record.GetRecordId());
}

int CloudDiskDataHandler::RecycleFile(const string &recordId)
{
    LOGI("recycle of record %s", recordId.c_str());
    ValuesBucket values;
    values.PutInt(FC::DIRTY_TYPE, static_cast<int32_t>(DirtyType::TYPE_NEW));
    values.PutInt(FC::POSITION, POSITION_LOCAL);
    values.PutLong(FC::FILE_TIME_RECYCLED, UTCTimeSeconds());
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

int32_t CloudDiskDataHandler::ClearCloudInfo(const string &cloudId)
{
    return E_OK;
}
int32_t CloudDiskDataHandler::GetCheckRecords(vector<DriveKit::DKRecordId> &checkRecords,
                                              const shared_ptr<std::vector<DriveKit::DKRecord>> &records)
{
    return E_OK;
}
int32_t CloudDiskDataHandler::Clean(const int action)
{
    return E_OK;
}
int32_t CloudDiskDataHandler::CleanCloudRecord(NativeRdb::ResultSet &local,
                                               const int action,
                                               const std::string &cloudId)
{
    return E_OK;
}
int32_t CloudDiskDataHandler::GetDownloadAsset(std::string cloudId,
                                               vector<DriveKit::DKDownloadAsset> &outAssetsToDownload)
{
    return E_OK;
}
int32_t CloudDiskDataHandler::UpdateDirPosition(string cloudId)
{
    return E_OK;
}
int32_t CloudDiskDataHandler::OnDownloadSuccess(const DriveKit::DKDownloadAsset &asset)
{
    return E_OK;
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
    for (auto &record : records) {
        DKRecordData data;
        record.GetRecordData(data);
        string isDirectory;
        data[DK_IS_DIRECTORY].GetString(isDirectory);
        if (isDirectory == "directory") {
            DriveKit::DKQueryCursor baseCursor;
            GetStartCursor(baseCursor);
            record.SetBaseCursor(baseCursor);
        }
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
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
