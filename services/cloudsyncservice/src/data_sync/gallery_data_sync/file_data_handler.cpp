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

#include "dfs_error.h"
#include "utils_log.h"
#include "gallery_file_const.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
using namespace std;
using namespace NativeRdb;
using namespace DriveKit;
using namespace Media;

const vector<string> FileDataHandler::GALLERY_FILE_COLUMNS = {
    MEDIA_DATA_DB_NAME,
    MEDIA_DATA_DB_MEDIA_TYPE,
    MEDIA_DATA_DB_DATE_ADDED,
    MEDIA_DATA_DB_IS_FAV,
    MEDIA_DATA_DB_HEIGHT,
    MEDIA_DATA_DB_WIDTH,
    MEDIA_DATA_DB_DATE_MODIFIED,
    MEDIA_DATA_DB_BUCKET_ID,
    MEDIA_DATA_DB_FILE_PATH,
    MEDIA_DATA_DB_THUMBNAIL,
    MEDIA_DATA_DB_LCD,
    MEDIA_DATA_DB_ORIENTATION
};

FileDataHandler::FileDataHandler(int32_t userId, const string &bundleName, std::shared_ptr<RdbStore> rdb)
    : RdbDataHandler(TABLE_NAME, rdb), userId_(userId), bundleName_(bundleName)
{
}

void FileDataHandler::GetFetchCondition(int32_t &limitRes, DKRecordType &recordType, DKFieldKeyArray &desiredKeys)
{
    limitRes = LIMIT_SIZE;
    recordType = recordType_;
    desiredKeys = desiredKeys_;
}

int32_t FileDataHandler::OnFetchRecords(const shared_ptr<vector<DKRecord>> &records)
{
    int32_t ret = E_OK;
    for (const auto &it : *records) {
        const auto &record = it;
        /* check for local same recordid */
        NativeRdb::AbsRdbPredicates predicates = NativeRdb::AbsRdbPredicates(TABLE_NAME);
        predicates.SetWhereClause(Media::MEDIA_DATA_DB_CLOUD_ID + " = ?");
        predicates.SetWhereArgs({record.GetRecordId()});
        predicates.Limit(LIMIT_SIZE);
        auto resultSet = Query(predicates, GALLERY_FILE_COLUMNS);
        if (resultSet == nullptr) {
            LOGE("get nullptr created result");
            ret = E_RDB;
            break;
        }
        int32_t rowCount = 0;
        ret = resultSet->GetRowCount(rowCount);
        if (ret != 0) {
            LOGE("result set get row count err %{public}d", ret);
            ret = E_RDB;
            break;
        }

        /* specific pull operation */
        if ((rowCount == 0) && !record.GetIsDelete()) {
            ret = PullRecordInsert(record);
        } else if (rowCount == 1) {
            vector<DKRecord> local;
            ret = localConvertor_.ResultSetToRecords(move(resultSet), local);
            if ((ret != E_OK) || (local.size() != 1)) {
                LOGE("result set to records err %{public}d, size %{public}zu", ret, local.size());
                break;
            }

            if (record.GetIsDelete()) {
                ret = PullRecordDelete(record, local[0]);
            } else {
                ret = PullRecordUpdate(record, local[0]);
            }
        } else {
            LOGE("recordId %s has multiple file in db!", record.GetRecordId().c_str());
        }

        if (ret == E_RDB) {
            break;
        }
    }
    return ret;
}

int32_t FileDataHandler::PullRecordInsert(const DKRecord &record)
{
    /* insert hmdfs dentry file here */

    int64_t rowId;
    ValuesBucket values;
    createConvertor_.RecordToValueBucket(record, values);

    int ret = Insert(rowId, values);
    if (ret != E_OK) {
        LOGE("Insert pull record failed");
        return E_RDB;
    }

    LOGI("Insert recordId %s success", record.GetRecordId().c_str());

    /* check and trigger lcd download */
    return E_OK;
}

int32_t FileDataHandler::PullRecordUpdate(const DKRecord &record, const DKRecord &local)
{
    return E_OK;
}

int32_t FileDataHandler::PullRecordDelete(const DKRecord &record, const DKRecord &local)
{
    return E_OK;
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
    auto results = Query(createPredicates, GALLERY_FILE_COLUMNS);
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
    deletePredicates.SetWhereClause(Media::MEDIA_DATA_DB_DIRTY + " = ?");
    deletePredicates.SetWhereArgs({to_string(static_cast<int32_t>(Media::DirtyType::TYPE_DELETED))});
    deletePredicates.Offset(deleteOffset_);
    deletePredicates.Limit(LIMIT_SIZE);

    /* query */
    auto results = Query(deletePredicates, GALLERY_FILE_COLUMNS);
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

int32_t FileDataHandler::GetMetaModifiedRecords(vector<DKRecord> &records)
{
    /* build predicates */
    NativeRdb::AbsRdbPredicates updatePredicates = NativeRdb::AbsRdbPredicates(TABLE_NAME);
    updatePredicates.SetWhereClause(Media::MEDIA_DATA_DB_DIRTY + " = ? AND " +
        Media::MEDIA_DATA_DB_IS_TRASH + " = ?");
    updatePredicates.SetWhereArgs({to_string(static_cast<int32_t>(Media::DirtyType::TYPE_MDIRTY)), "0"});
    updatePredicates.Offset(metaUpdateOffset_);
    updatePredicates.Limit(LIMIT_SIZE);

    /* query */
    auto results = Query(updatePredicates, GALLERY_FILE_COLUMNS);
    if (results == nullptr) {
        LOGE("get nullptr modified result");
        return E_RDB;
    }
    /* update offset */
    metaUpdateOffset_ += LIMIT_SIZE;

    /* results to records */
    int ret = updateConvertor_.ResultSetToRecords(move(results), records);
    if (ret != 0) {
        LOGE("result set to records err %{public}d", ret);
        return ret;
    }

    return E_OK;
}

int32_t FileDataHandler::GetFileModifiedRecords(vector<DKRecord> &records)
{
    /* build predicates */
    NativeRdb::AbsRdbPredicates updatePredicates = NativeRdb::AbsRdbPredicates(TABLE_NAME);
    updatePredicates.SetWhereClause(Media::MEDIA_DATA_DB_DIRTY + " = ? AND " +
        Media::MEDIA_DATA_DB_IS_TRASH + " = ?");
    updatePredicates.SetWhereArgs({to_string(static_cast<int32_t>(Media::DirtyType::TYPE_FDIRTY)), "0"});
    updatePredicates.Offset(fileUpdateOffset_);
    updatePredicates.Limit(LIMIT_SIZE);

    /* query */
    auto results = Query(updatePredicates, GALLERY_FILE_COLUMNS);
    if (results == nullptr) {
        LOGE("get nullptr modified result");
        return E_RDB;
    }
    /* update offset */
    fileUpdateOffset_ += LIMIT_SIZE;

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
        valuesBucket.PutString(Media::MEDIA_DATA_DB_CLOUD_ID, entry.first);
        valuesBucket.PutInt(Media::MEDIA_DATA_DB_DIRTY,
            static_cast<int32_t>(Media::DirtyType::TYPE_SYNCED));

        DKRecordData data;
        record.GetRecordData(data);
        auto iter = data.find(FILE_FILE_NAME);
        if (iter == data.end()) {
            LOGE("no id in record data");
            continue;
        }
        string fileName = iter->second;

        /* update local */
        int32_t changedRows;
        string whereClause = Media::MEDIA_DATA_DB_NAME + " = ?";
        int32_t ret = Update(changedRows, valuesBucket, whereClause, { fileName });
        if (ret != 0) {
            LOGE("on create records update err %{public}d, file name %{private}s", ret, fileName.c_str());
            continue;
        }
    }

    return E_OK;
}

int32_t FileDataHandler::OnDeleteRecords(const map<DKRecordId, DKRecordOperResult> &map)
{
    for (auto &entry : map) {
        auto record = const_cast<DKRecordOperResult &>(entry.second).GetDKRecord();

        DKRecordData data;
        record.GetRecordData(data);
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

int32_t FileDataHandler::OnModifyMdirtyRecords(const map<DKRecordId, DKRecordOperResult> &map)
{
    for (auto &entry : map) {
        auto record = const_cast<DKRecordOperResult &>(entry.second).GetDKRecord();

        /* record to value bucket */
        ValuesBucket valuesBucket;
        valuesBucket.PutInt(Media::MEDIA_DATA_DB_DIRTY,
            static_cast<int32_t>(Media::DirtyType::TYPE_SYNCED));

        DKRecordData data;
        record.GetRecordData(data);
        auto iter = data.find(Media::MEDIA_DATA_DB_CLOUD_ID);
        if (iter == data.end()) {
            LOGE("no id in record data");
            continue;
        }
        string cloudId = iter->second;

        /* update local */
        int32_t changedRows;
        string whereClause = Media::MEDIA_DATA_DB_CLOUD_ID + " = ?";
        int32_t ret = Update(changedRows, valuesBucket, whereClause, { cloudId });
        if (ret != 0) {
            LOGE("on modify mdirty records update err %{public}d, cloudId %{private}s",
                ret, cloudId.c_str());
            continue;
        }
    }

    return E_OK;
}

int32_t FileDataHandler::OnModifyFdirtyRecords(const map<DKRecordId, DKRecordOperResult> &map)
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

        DKRecordData data;
        record.GetRecordData(data);
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
            LOGE("on modify fdirty records update err %{public}d, cloudId %{private}s",
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
    metaUpdateOffset_ = 0;
    fileUpdateOffset_ = 0;
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
