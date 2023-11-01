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

#include "album_data_handler.h"

#include "medialibrary_db_const.h"
#include "medialibrary_type_const.h"

#include "data_sync_const.h"
#include "data_sync_notifier.h"
#include "dfs_error.h"
#include "gallery_album_const.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
using namespace std;
using namespace NativeRdb;
using namespace DriveKit;
using namespace Media;
using PAC = Media::PhotoAlbumColumns;
using PM = Media::PhotoMap;
using ChangeType = OHOS::AAFwk::ChangeInfo::ChangeType;

AlbumDataHandler::AlbumDataHandler(int32_t userId, const std::string &bundleName, std::shared_ptr<RdbStore> rdb)
    : RdbDataHandler(userId, bundleName, PAC::TABLE, rdb)
{
}

void AlbumDataHandler::GetFetchCondition(FetchCondition &cond)
{
    cond.limitRes = LIMIT_SIZE;
    cond.recordType = recordType_;
    cond.desiredKeys = desiredKeys_;
    cond.fullKeys = desiredKeys_;
}


tuple<shared_ptr<ResultSet>, int> AlbumDataHandler::QueryLocalMatch(const std::string &recordId)
{
    NativeRdb::AbsRdbPredicates predicates = NativeRdb::AbsRdbPredicates(PAC::TABLE);
    predicates.EqualTo(PAC::ALBUM_CLOUD_ID, recordId);
    auto resultSet = Query(predicates, ALBUM_LOCAL_QUERY_COLUMNS);
    if (resultSet == nullptr) {
        LOGE("get nullptr query result");
        return {nullptr, 0};
    }
    int rowCount = 0;
    int ret = resultSet->GetRowCount(rowCount);
    if (ret != 0) {
        LOGE("result set get row count err %{public}d", ret);
        return {nullptr, 0};
    }
    return { move(resultSet), rowCount };
}

bool AlbumDataHandler::IsConflict(DriveKit::DKRecord &record, int32_t &albumId)
{
    /* get record album name */
    DKRecordData data;
    record.GetRecordData(data);
    if (data.find(ALBUM_NAME) == data.end()) {
        LOGE("no album name in record");
        return false;
    }
    string albumName;
    if (data[ALBUM_NAME].GetString(albumName) != DKLocalErrorCode::NO_ERROR) {
        LOGE("get album name err");
        return false;
    }

    /* query local */
    NativeRdb::AbsRdbPredicates predicates = NativeRdb::AbsRdbPredicates(PAC::TABLE);
    predicates.EqualTo(PAC::ALBUM_NAME, albumName);
    auto resultSet = Query(predicates, ALBUM_LOCAL_QUERY_COLUMNS);
    if (resultSet == nullptr) {
        LOGE("get nullptr query result");
        return false;
    }
    int rowCount = 0;
    int ret = resultSet->GetRowCount(rowCount);
    if (ret != 0) {
        LOGE("result set get row count err %{public}d", ret);
        return false;
    }

    if (rowCount > 0) {
        ret = resultSet->GoToNextRow();
        if (ret != NativeRdb::E_OK) {
            return false;
        }
        ret = createConvertor_.GetInt(PAC::ALBUM_ID, albumId, *resultSet);
        if (ret != E_OK) {
            return false;
        }
        return true;
    }
    return false;
}

int32_t AlbumDataHandler::MergeAlbumOnConflict(DriveKit::DKRecord &record, int32_t albumId)
{
    ValuesBucket values;
    values.PutInt(PAC::ALBUM_DIRTY, static_cast<int32_t>(Media::DirtyType::TYPE_MDIRTY));
    values.PutString(PAC::ALBUM_CLOUD_ID, record.GetRecordId());
    int32_t changedRows;
    int32_t ret = Update(changedRows, values, PAC::ALBUM_ID+ " = ?", { to_string(albumId) });
    if (ret != E_OK) {
        LOGE("rdb update failed, err = %{public}d", ret);
        return E_RDB;
    }
    return E_OK;
}

int32_t AlbumDataHandler::InsertCloudAlbum(DKRecord &record)
{
    LOGI("insert of record %s", record.GetRecordId().c_str());

    /* merge if same album name */
    int32_t albumId;
    if (IsConflict(record, albumId)) {
        int32_t ret = MergeAlbumOnConflict(record, albumId);
        if (ret != E_OK) {
            LOGE("merge album err %{public}d", ret);
        }
        return ret;
    }

    ValuesBucket values;
    int32_t ret = createConvertor_.RecordToValueBucket(record, values);
    if (ret != E_OK) {
        LOGE("record to value bucket failed, ret = %{public}d", ret);
        return ret;
    }
    values.PutInt(PAC::ALBUM_DIRTY, static_cast<int32_t>(Media::DirtyType::TYPE_SYNCED));
    values.PutString(PAC::ALBUM_CLOUD_ID, record.GetRecordId());

    /* update if a album with the same name exists? */
    int64_t rowId;
    ret = Insert(rowId, values);
    if (ret != E_OK) {
        LOGE("Insert pull record failed, rdb ret = %{public}d", ret);
        return E_RDB;
    }
    return E_OK;
}

int32_t AlbumDataHandler::DeleteCloudAlbum(DKRecord &record)
{
    int32_t deletedRows;
    int ret = Delete(deletedRows, PAC::ALBUM_CLOUD_ID + " = ?", { record.GetRecordId() });
    if (ret != E_OK) {
        LOGE("delete in rdb failed, ret: %{public}d", ret);
        return E_INVAL_ARG;
    }
    return E_OK;
}

int32_t AlbumDataHandler::UpdateCloudAlbum(DKRecord &record)
{
    int32_t changedRows;
    ValuesBucket values;
    int32_t ret = createConvertor_.RecordToValueBucket(record, values);
    if (ret != E_OK) {
        LOGE("record to value bucket failed, ret = %{public}d", ret);
        return ret;
    }
    values.PutInt(PAC::ALBUM_DIRTY, static_cast<int32_t>(Media::DirtyType::TYPE_SYNCED));

    ret = Update(changedRows, values, PAC::ALBUM_CLOUD_ID + " = ?", { record.GetRecordId() });
    if (ret != E_OK) {
        LOGE("rdb update failed, err = %{public}d", ret);
        return E_RDB;
    }
    return E_OK;
}

static int32_t GetLocalMatchDirty(NativeRdb::ResultSet &resultSet)
{
    int32_t dirty;
    int32_t ret = DataConvertor::GetInt(PAC::ALBUM_DIRTY, dirty, resultSet);
    if (ret != E_OK) {
        LOGE("fail to get album dirty value");
        /* assume dirty so that no follow-up actions */
        return static_cast<int32_t>(Media::DirtyType::TYPE_MDIRTY);
    }
    return dirty;
}

int32_t AlbumDataHandler::HandleLocalDirty(int32_t dirty, const DriveKit::DKRecord &record)
{
    /* new -> dirty: keep local info and update cloud's */
    if (dirty == static_cast<int32_t>(Media::DirtyType::TYPE_NEW)) {
        int32_t changedRows;
        ValuesBucket values;
        values.PutInt(PAC::ALBUM_DIRTY, static_cast<int32_t>(Media::DirtyType::TYPE_MDIRTY));
        int32_t ret = Update(changedRows, values, PAC::ALBUM_NAME + " = ?", { record.GetRecordId() });
        if (ret != E_OK) {
            LOGE("rdb update failed, err = %{public}d", ret);
            return E_RDB;
        }
        return E_OK;
    }
    return E_OK;
}

int32_t AlbumDataHandler::OnFetchRecords(shared_ptr<vector<DKRecord>> &records,
                                         OnFetchParams &params)
{
    LOGI("on fetch %{public}zu records", records->size());
    int32_t ret = E_OK;
    for (auto &record : *records) {
        auto [resultSet, rowCount] = QueryLocalMatch(record.GetRecordId());
        if (resultSet == nullptr) {
            LOGE("get nullptr query result");
            continue;
        }
        /* need to handle album cover uri */
        if (rowCount == 0) {
            if (!record.GetIsDelete()) {
                /* insert */
                ret = InsertCloudAlbum(record);
            }
        } else if (rowCount == 1) {
            resultSet->GoToNextRow();
            int32_t dirty = GetLocalMatchDirty(*resultSet);
            if (dirty != static_cast<int32_t>(Media::DirtyType::TYPE_SYNCED)) {
                /* local dirty */
                ret = HandleLocalDirty(dirty, record);
            } else if (record.GetIsDelete()) {
                /* delete */
                ret = DeleteCloudAlbum(record);
            } else {
                /* update */
                ret = UpdateCloudAlbum(record);
            }
        } else {
            /* invalid cases */
            LOGE("recordId %s rowCount %{public}d", record.GetRecordId().c_str(), rowCount);
        }

        /* check ret */
        if (ret != E_OK) {
            LOGE("recordId %s error %{public}d", record.GetRecordId().c_str(), ret);
            if (ret == E_STOP) {
                return E_STOP;
            }
            continue;
        } else {
            /* notify */
            (void)DataSyncNotifier::GetInstance().TryNotify(ALBUM_URI_PREFIX,
                ChangeType::UPDATE, INVALID_ASSET_ID);
        }
    }
    (void)DataSyncNotifier::GetInstance().FinalNotify();
    return E_OK;
}

int32_t AlbumDataHandler::GetRetryRecords(std::vector<DriveKit::DKRecordId> &records)
{
    return E_OK;
}

int32_t AlbumDataHandler::Clean(const int action)
{
    /* update album */
    string albumSql = "UPDATE " + PAC::TABLE + " SET " + PAC::ALBUM_CLOUD_ID + " = NULL, " +
        PAC::ALBUM_DIRTY + " = " + to_string(static_cast<int32_t>(Media::DirtyType::TYPE_NEW));
    int32_t ret = ExecuteSql(albumSql);
    if (ret != NativeRdb::E_OK) {
        LOGE("update all albums err %{public}d", ret);
        return ret;
    }
    /* update album map */
    string mapSql = "UPDATE " + PM::TABLE + " SET " + PM::DIRTY + " = " + to_string(static_cast<int32_t>(
        Media::DirtyType::TYPE_NEW)) + " WHERE " + PM::DIRTY + " = " + to_string(static_cast<int32_t>(
        Media::DirtyType::TYPE_SYNCED));
    ret = ExecuteSql(mapSql);
    if (ret != NativeRdb::E_OK) {
        LOGE("update album maps err %{public}d", ret);
        return ret;
    }
    mapSql = "DELETE FROM " + PM::TABLE + " WHERE " + PM::DIRTY + " = " + to_string(static_cast<int32_t>(
        Media::DirtyType::TYPE_DELETED));
    ret = ExecuteSql(mapSql);
    if (ret != NativeRdb::E_OK) {
        LOGE("delete album maps err %{public}d", ret);
        return ret;
    }
    /* notify */
    (void)DataSyncNotifier::GetInstance().TryNotify(ALBUM_URI_PREFIX,
        ChangeType::DELETE, INVALID_ASSET_ID);
    (void)DataSyncNotifier::GetInstance().FinalNotify();
    return E_OK;
}

int32_t AlbumDataHandler::GetCreatedRecords(vector<DKRecord> &records)
{
    /* build predicates */
    auto createPredicates = NativeRdb::AbsRdbPredicates(PAC::TABLE);
    createPredicates.EqualTo(PAC::ALBUM_DIRTY, to_string(static_cast<int32_t>(Media::DirtyType::TYPE_NEW)));
    /* skip system albums */
    createPredicates.And()->EqualTo(PAC::ALBUM_SUBTYPE, to_string(Media::PhotoAlbumSubType::USER_GENERIC));
    if (!createFailSet_.empty()) {
        createPredicates.And()->NotIn(PAC::ALBUM_NAME, createFailSet_);
    }
    createPredicates.Limit(LIMIT_SIZE);

    /* query */
    auto results = Query(createPredicates, GALLERY_ALBUM_COLUMNS);
    if (results == nullptr) {
        LOGE("get nullptr created result");
        return E_RDB;
    }

    /* results to records */
    int ret = createConvertor_.ResultSetToRecords(move(results), records);
    if (ret != 0) {
        LOGE("result set to records err %{public}d", ret);
        return ret;
    }

    return E_OK;
}

int32_t AlbumDataHandler::GetDeletedRecords(vector<DKRecord> &records)
{
    /* build predicates */
    auto createPredicates = NativeRdb::AbsRdbPredicates(PAC::TABLE);
    createPredicates.EqualTo(PAC::ALBUM_DIRTY, to_string(static_cast<int32_t>(Media::DirtyType::TYPE_DELETED)));
    /* skip system albums */
    createPredicates.And()->EqualTo(PAC::ALBUM_SUBTYPE, to_string(Media::PhotoAlbumSubType::USER_GENERIC));
    if (!modifyFailSet_.empty()) {
        createPredicates.And()->NotIn(PAC::ALBUM_NAME, modifyFailSet_);
    }
    createPredicates.Limit(LIMIT_SIZE);

    /* query */
    auto results = Query(createPredicates, GALLERY_ALBUM_COLUMNS);
    if (results == nullptr) {
        LOGE("get nullptr created result");
        return E_RDB;
    }

    /* results to records */
    int ret = deleteConvertor_.ResultSetToRecords(move(results), records);
    if (ret != 0) {
        LOGE("result set to records err %{public}d", ret);
        return ret;
    }

    return E_OK;
}

int32_t AlbumDataHandler::GetMetaModifiedRecords(vector<DKRecord> &records)
{
    /* build predicates */
    auto createPredicates = NativeRdb::AbsRdbPredicates(PAC::TABLE);
    createPredicates.EqualTo(PAC::ALBUM_DIRTY, to_string(static_cast<int32_t>(Media::DirtyType::TYPE_MDIRTY)));
    /* skip system albums */
    createPredicates.And()->EqualTo(PAC::ALBUM_SUBTYPE, to_string(Media::PhotoAlbumSubType::USER_GENERIC));
    if (!modifyFailSet_.empty()) {
        createPredicates.And()->NotIn(PAC::ALBUM_NAME, modifyFailSet_);
    }
    createPredicates.Limit(LIMIT_SIZE);

    /* query */
    auto results = Query(createPredicates, GALLERY_ALBUM_COLUMNS);
    if (results == nullptr) {
        LOGE("get nullptr created result");
        return E_RDB;
    }

    /* results to records */
    int ret = modifyConvertor_.ResultSetToRecords(move(results), records);
    if (ret != 0) {
        LOGE("result set to records err %{public}d", ret);
        return ret;
    }

    return E_OK;
}

int32_t AlbumDataHandler::OnCreateRecords(const map<DKRecordId, DKRecordOperResult> &map)
{
    int32_t ret = E_OK;
    for (auto &entry : map) {
        int32_t err;
        const DKRecordOperResult &result = entry.second;
        if (result.IsSuccess()) {
            err = OnCreateSuccess(entry);
        } else {
            err = OnRecordFailed(entry);
            OnCreateFail(entry);
        }
        GetReturn(err, ret);
    }
    return ret;
}

int32_t AlbumDataHandler::OnDeleteRecords(const map<DKRecordId, DKRecordOperResult> &map)
{
    int32_t ret = E_OK;
    for (auto &entry : map) {
        int32_t err;
        const DKRecordOperResult &result = entry.second;
        if (result.IsSuccess()) {
            err = OnDeleteSuccess(entry);
        } else {
            err = OnRecordFailed(entry);
            OnDeleteFail(entry);
        }
        GetReturn(err, ret);
    }
    return ret;
}

int32_t AlbumDataHandler::OnModifyMdirtyRecords(const map<DKRecordId, DKRecordOperResult> &map)
{
    int32_t ret = E_OK;
    for (auto &entry : map) {
        int32_t err;
        const DKRecordOperResult &result = entry.second;
        if (result.IsSuccess()) {
            err = OnUploadSuccess(entry);
        } else {
            err = OnRecordFailed(entry);
            OnModifyFail(entry);
        }
        GetReturn(err, ret);
    }
    return ret;
}

static inline string GetAlbumNameFromRecord(const DriveKit::DKRecordOperResult &result)
{
    auto record = result.GetDKRecord();
    DKRecordData data;
    record.GetRecordData(data);
    return data[ALBUM_NAME];
}

int32_t AlbumDataHandler::OnCreateSuccess(const pair<DriveKit::DKRecordId,
    DriveKit::DKRecordOperResult> &entry)
{
    ValuesBucket valuesBucket;
    int32_t changedRows;
    string whereClause = PAC::ALBUM_NAME + " = ?";
    vector<string> whereArgs = { GetAlbumNameFromRecord(entry.second) };
    valuesBucket.PutInt(PAC::ALBUM_DIRTY, static_cast<int32_t>(Media::DirtyType::TYPE_SYNCED));
    valuesBucket.PutString(PAC::ALBUM_CLOUD_ID, entry.first);
    int32_t ret = Update(changedRows, valuesBucket, whereClause, whereArgs);
    if (ret != 0) {
        LOGE("update local records err %{public}d", ret);
        return ret;
    }
    return E_OK;
}

int32_t AlbumDataHandler::OnUploadSuccess(const pair<DriveKit::DKRecordId,
    DriveKit::DKRecordOperResult> &entry)
{
    ValuesBucket valuesBucket;
    int32_t changedRows;
    string whereClause = PAC::ALBUM_CLOUD_ID + " = ?";
    vector<string> whereArgs = { entry.first };
    valuesBucket.PutInt(PAC::ALBUM_DIRTY, static_cast<int32_t>(Media::DirtyType::TYPE_SYNCED));
    int32_t ret = Update(changedRows, valuesBucket, whereClause, whereArgs);
    if (ret != 0) {
        LOGE("update local records err %{public}d", ret);
        return ret;
    }
    return E_OK;
}

int32_t AlbumDataHandler::OnDeleteSuccess(const pair<DriveKit::DKRecordId,
    DriveKit::DKRecordOperResult> &entry)
{
    int32_t deletedRows;
    string whereClause = PAC::ALBUM_CLOUD_ID + " = ?";
    vector<string> whereArgs = { entry.first };
    int32_t ret = Delete(deletedRows, whereClause, whereArgs);
    if (ret != 0) {
        LOGE("delete local records err %{public}d", ret);
        return ret;
    }
    return E_OK;
}

int32_t AlbumDataHandler::OnCreateFail(const pair<DriveKit::DKRecordId,
    DriveKit::DKRecordOperResult> &entry)
{
    createFailSet_.push_back(entry.first);
    return E_OK;
}

int32_t AlbumDataHandler::OnDeleteFail(const pair<DriveKit::DKRecordId,
    DriveKit::DKRecordOperResult> &entry)
{
    modifyFailSet_.push_back(entry.first);
    return E_OK;
}

int32_t AlbumDataHandler::OnModifyFail(const pair<DriveKit::DKRecordId,
    DriveKit::DKRecordOperResult> &entry)
{
    modifyFailSet_.push_back(entry.first);
    return E_OK;
}

void AlbumDataHandler::Reset()
{
    modifyFailSet_.clear();
    createFailSet_.clear();
}

void AlbumDataHandler::SetChecking()
{
    ClearCursor();
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
