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

#include "file_data_handler.h"

#include "medialibrary_db_const.h"
#include "medialibrary_type_const.h"

#include "dfs_error.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
using namespace std;
using namespace NativeRdb;
using namespace DriveKit;

const string FileDataHandler::TABLE_NAME = "Files";
const int32_t FileDataHandler::LIMIT_SIZE = 50;

FileDataHandler::FileDataHandler(std::shared_ptr<RdbStore> rdb)
    : RdbDataHandler(TABLE_NAME, rdb)
{
}

int32_t FileDataHandler::GetFetchCondition()
{
    return 0;
}

int32_t FileDataHandler::OnFetchRecords(const shared_ptr<const map<DKRecordId, DKRecord>> &map)
{
    return 0;
}

int32_t FileDataHandler::GetCreatedRecords(vector<DKRecord> &records)
{
    /* build predicates */
    NativeRdb::AbsRdbPredicates createPredicates = NativeRdb::AbsRdbPredicates(TABLE_NAME);
    createPredicates.SetWhereClause(Media::MEDIA_DATA_DB_DIRTY + " = ? AND " +
        Media::MEDIA_DATA_DB_IS_TRASH + " = ?");
    createPredicates.SetWhereArgs({to_string(static_cast<int32_t>(Media::DirtyType::TYPE_NEW)), "0"});
    createPredicates.Offset(createOffset_);
    createPredicates.Limit(LIMIT_SIZE);

    /* query */
    auto results = Query(createPredicates, createConvertor_.GetLocalColumns());
    if (results == nullptr) {
        LOGE("get nullptr created result");
        return E_RDB;
    }
    /* update offset */
    createOffset_ += LIMIT_SIZE;

    /* results to records */
    int ret = createConvertor_.ResultSetToRecords(move(results), records);
    if (ret != 0) {
        LOGE("result set to records err %{public}d", ret);
        return ret;
    }

    return E_OK;
}

int32_t FileDataHandler::GetDeletedRecords(vector<DKRecord> &records)
{
    /* build predicates */
    NativeRdb::AbsRdbPredicates deletePredicates = NativeRdb::AbsRdbPredicates(TABLE_NAME);
    deletePredicates.SetWhereClause(Media::MEDIA_DATA_DB_DIRTY + " = ? AND " +
        Media::MEDIA_DATA_DB_IS_TRASH + " = ?");
    deletePredicates.SetWhereArgs({to_string(static_cast<int32_t>(Media::DirtyType::TYPE_DELETED)), "0"});
    deletePredicates.Offset(createOffset_);
    deletePredicates.Limit(LIMIT_SIZE);

    /* query */
    auto results = Query(deletePredicates, deleteConvertor_.GetLocalColumns());
    if (results == nullptr) {
        LOGE("get nullptr deleted result");
        return E_RDB;
    }
    /* update offset */
    deleteOffset_ += LIMIT_SIZE;

    /* results to records */
    int ret = deleteConvertor_.ResultSetToRecords(move(results), records);
    if (ret != 0) {
        LOGE("result set to records err %{public}d", ret);
        return ret;
    }

    return E_OK;
}

int32_t FileDataHandler::GetModifiedRecords(vector<DKRecord> &records)
{
    /* build predicates */
    NativeRdb::AbsRdbPredicates updatePredicates = NativeRdb::AbsRdbPredicates(TABLE_NAME);
    updatePredicates.SetWhereClause(Media::MEDIA_DATA_DB_DIRTY + " = ? AND " +
        Media::MEDIA_DATA_DB_IS_TRASH + " = ?");
    updatePredicates.SetWhereArgs({to_string(static_cast<int32_t>(Media::DirtyType::TYPE_FDIRTY)), "0"});
    updatePredicates.Offset(createOffset_);
    updatePredicates.Limit(LIMIT_SIZE);

    /* query */
    auto results = Query(updatePredicates, updateConvertor_.GetLocalColumns());
    if (results == nullptr) {
        LOGE("get nullptr modified result");
        return E_RDB;
    }
    /* update offset */
    updateOffset_ += LIMIT_SIZE;

    /* results to records */
    int ret = updateConvertor_.ResultSetToRecords(move(results), records);
    if (ret != 0) {
        LOGE("result set to records err %{public}d", ret);
        return ret;
    }

    return E_OK;
}

int32_t FileDataHandler::OnCreateRecords(const map<DKRecordId, DKRecordOperResult> &map)
{
    for (auto &entry : map) {
        auto record = const_cast<DKRecordOperResult &>(entry.second).GetDKRecord();

        /* record to value bucket */
        ValuesBucket valuesBucket;
        int32_t ret = onCreateConvertor_.RecordToValueBucket(record, valuesBucket);
        if (ret != E_OK) {
            LOGE("record to value bucket err %{public}d", ret);
            continue;
        }
        valuesBucket.PutString(Media::MEDIA_DATA_DB_CLOUD_ID, entry.first);
        valuesBucket.PutInt(Media::MEDIA_DATA_DB_DIRTY,
            static_cast<int32_t>(Media::DirtyType::TYPE_SYNCED));

        DKRecordDatas data;
        record.GetRecordDatas(data);
        auto iter = data.find(Media::MEDIA_DATA_DB_ID);
        if (iter == data.end()) {
            LOGE("no id in record data");
            continue;
        }
        int32_t id = iter->second;

        /* update local */
        int32_t changedRows;
        string whereClause = Media::MEDIA_DATA_DB_ID + " = ?";
        ret = Update(changedRows, valuesBucket, whereClause, { to_string(id) });
        if (ret != 0) {
            LOGE("on create records update err %{public}d, id %{public}d", ret, id);
            continue;
        }
    }

    return E_OK;
}

int32_t FileDataHandler::OnDeleteRecords(const map<DKRecordId, DKRecordOperResult> &map)
{
    for (auto &entry : map) {
        auto record = const_cast<DKRecordOperResult &>(entry.second).GetDKRecord();

        DKRecordDatas data;
        record.GetRecordDatas(data);
        auto iter = data.find(Media::MEDIA_DATA_DB_CLOUD_ID);
        if (iter == data.end()) {
            LOGE("no id in record data");
            continue;
        }
        string cloudId = iter->second;

        /* delete local */
        int32_t deletedRows;
        string whereClause = Media::MEDIA_DATA_DB_CLOUD_ID + " = ?";
        int32_t ret = Delete(deletedRows, whereClause, { cloudId });
        if (ret != 0) {
            LOGE("on delete records update err %{public}d, cloudId %{private}s",
                ret, cloudId.c_str());
            continue;
        }
    }

    return E_OK;
}

int32_t FileDataHandler::OnModifyRecords(const map<DKRecordId, DKRecordOperResult> &map)
{
    for (auto &entry : map) {
        auto record = const_cast<DKRecordOperResult &>(entry.second).GetDKRecord();

        /* record to value bucket */
        ValuesBucket valuesBucket;
        int32_t ret = updateConvertor_.RecordToValueBucket(record, valuesBucket);
        if (ret != E_OK) {
            LOGE("record to value bucket err %{public}d", ret);
            continue;
        }
        valuesBucket.PutInt(Media::MEDIA_DATA_DB_DIRTY,
            static_cast<int32_t>(Media::DirtyType::TYPE_SYNCED));

        DKRecordDatas data;
        record.GetRecordDatas(data);
        auto iter = data.find(Media::MEDIA_DATA_DB_CLOUD_ID);
        if (iter == data.end()) {
            LOGE("no id in record data");
            continue;
        }
        string cloudId = iter->second;

        /* update local */
        int32_t changedRows;
        string whereClause = Media::MEDIA_DATA_DB_CLOUD_ID + " = ?";
        ret = Update(changedRows, valuesBucket, whereClause, { cloudId });
        if (ret != 0) {
            LOGE("on modify records update err %{public}d, cloudId %{private}s",
                ret, cloudId.c_str());
            continue;
        }
    }

    return E_OK;
}

void FileDataHandler::Reset()
{
    createOffset_ = 0;
    deleteOffset_ = 0;
    updateOffset_ = 0;
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS