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
    string path;
    ret = localConvertor_.GetMetaFilePath(cloudId, path);
    if (ret != E_OK) {
        LOGE("get path err");
        return;
    }
    createFailSet_.push_back(path);
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
    if (ret != E_OK) {
        LOGE("record to valuebucket failed, ret=%{public}d", ret);
        return ret;
    }
    int64_t rowId;
    values.PutInt(FC::POSITION, POSITION_CLOUD);
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
        string localPath;
        int32_t ret = localConvertor_.GetMetaFilePath(record.GetRecordId(), localPath);
        if (ret != E_OK) {
            LOGE("get mate file path by cloudId error");
        }
    }
    LOGI("cloud file META changed, %s", record.GetRecordId().c_str());
    /* update rdb */
    ValuesBucket values;
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
    string path;
    int32_t ret = localConvertor_.GetMetaFilePath(record.GetRecordId(), path);
    if (FileIsRecycled(local) || !isLocal) {
        if (isLocal) {
            LOGI("force delete instead");
            ret = unlink(path.c_str());
            if (ret != 0) {
                LOGE("unlink local failed, errno %{public}d", errno);
            }
        }
        int32_t deletedRows;
        int ret = Delete(deletedRows, FC::CLOUD_ID + " = ?", {record.GetRecordId()});
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
    return E_OK;
}
int32_t CloudDiskDataHandler::GetDeletedRecords(vector<DKRecord> &records)
{
    return E_OK;
}
int32_t CloudDiskDataHandler::GetMetaModifiedRecords(vector<DKRecord> &records)
{
    return E_OK;
}
int32_t CloudDiskDataHandler::GetFileModifiedRecords(vector<DKRecord> &records)
{
    return E_OK;
}
int32_t CloudDiskDataHandler::OnCreateRecords(const map<DKRecordId, DKRecordOperResult> &map)
{
    return E_OK;
}
int32_t CloudDiskDataHandler::OnDeleteRecords(const map<DKRecordId, DKRecordOperResult> &map)
{
    return E_OK;
}
int32_t CloudDiskDataHandler::OnModifyMdirtyRecords(const map<DKRecordId, DKRecordOperResult> &map)
{
    return E_OK;
}
int32_t CloudDiskDataHandler::OnModifyFdirtyRecords(const map<DKRecordId, DKRecordOperResult> &map)
{
    return E_OK;
}
int32_t CloudDiskDataHandler::OnDownloadAssets(const DriveKit::DKDownloadAsset &asset)
{
    return E_OK;
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
