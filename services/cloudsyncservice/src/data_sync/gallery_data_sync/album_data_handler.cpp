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
#include <fstream>
#include "album_data_handler.h"

#include "medialibrary_db_const.h"
#include "medialibrary_type_const.h"
#include "data_sync_const.h"
#include "data_sync_notifier.h"
#include "dfs_error.h"
#include "gallery_album_const.h"
#include "utils_log.h"
#include <algorithm>

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
const int CONFIG_PARAM_NUM = 2;
static const std::string CONFIG_PATH = "/system/etc/dfs_service/path2bundle";

std::vector<std::string> SplitLine(std::string &line, std::string &token)
{
    std::vector<std::string> result;
    std::string::size_type first;
    std::string::size_type last;

    first = 0;
    last = line.find(token);
    while (std::string::npos != last) {
        result.push_back(line.substr(first, last - first));
        first = last + token.size();
        last = line.find(token, first);
    }

    if (first != line.length()) {
        result.push_back(line.substr(first));
    }

    return result;
}

static bool GetPath2BundleMap(std::unordered_map<std::string, std::string> &localPathtoBundle)
{
    std::ifstream infile;
    infile.open(CONFIG_PATH);
    if (!infile) {
        LOGE("Cannot open config");
        return false;
    }

    while (infile) {
        std::string line;
        std::getline(infile, line);
        if (line.empty()) {
            LOGI("Param config complete");
            break;
        }

        std::string token = " ";
        auto split = SplitLine(line, token);
        if (split.size() != CONFIG_PARAM_NUM) {
            LOGE("Invalids config line: number of parameters is incorrect");
            continue;
        }

        auto it = split.begin();
        auto path = *(it++);
        auto bundleName = *(it);
        localPathtoBundle.emplace(path, bundleName);
    }

    infile.close();
    return true;
}

AlbumDataHandler::AlbumDataHandler(int32_t userId, const std::string &bundleName,
                                   std::shared_ptr<RdbStore> rdb, shared_ptr<bool> stopFlag)
    : RdbDataHandler(userId, bundleName, PAC::TABLE, rdb, stopFlag)
{
    GetPath2BundleMap(localPathtoBundle_);
}

void AlbumDataHandler::GetFetchCondition(FetchCondition &cond)
{
    cond.limitRes = LIMIT_SIZE;
    cond.recordType = recordType_;
    cond.desiredKeys = desiredKeys_;
    cond.fullKeys = desiredKeys_;
}

static inline int32_t GetRecordFieMapFromRecord(const DriveKit::DKRecord &record, DKRecordFieldMap &map)
{
    DKRecordData data;
    DriveKit::DKRecordFieldMap properties;
    record.GetRecordData(data);
    if (data.find(ALBUM_PROPERTIES) == data.end()) {
        LOGE("get ALBUM_PROPERTIES fail");
        return E_INVAL_ARG;
    }
    data[ALBUM_PROPERTIES].GetRecordMap(map);
    return E_OK;
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

int32_t AlbumDataHandler::ModRecordIfFromDual(DKRecordData &data)
{
    string album_name;
    DriveKit::DKRecordFieldMap properties;
    if (data.find(ALBUM_PROPERTIES) == data.end() || data.find(ALBUM_NAME) == data.end() ||
        data[ALBUM_NAME].GetString(album_name) != DKLocalErrorCode::NO_ERROR ||
        data[ALBUM_PROPERTIES].GetRecordMap(properties) != DKLocalErrorCode::NO_ERROR) {
        LOGE("record data do not have properties set/name");
        return E_INVAL_ARG;
    }

    if (data.find(ALBUM_PATH) == data.end()) {
        return E_OK;
    }
    LOGI("record from dual first time to come, album name = %{public}s", album_name.c_str());
    properties[PAC::ALBUM_NAME] = DriveKit::DKRecordField(data[ALBUM_NAME]);
    properties[PAC::ALBUM_TYPE] = DriveKit::DKRecordField(to_string(static_cast<int32_t>(AlbumType::SOURCE)));
    properties[PAC::ALBUM_SUBTYPE] = DriveKit::DKRecordField(Media::PhotoAlbumSubType::USER_GENERIC);
    properties[PAC::ALBUM_DATE_MODIFIED] = DriveKit::DKRecordField(0);

    string local_path;
    if (data[ALBUM_PATH].GetString(local_path) != DKLocalErrorCode::NO_ERROR) {
        LOGE("record from dual get local_path failed");
    }
    for (auto &it : localPathtoBundle_) {
        if (local_path == it.first) {
            properties[TMP_ALBUM_BUNDLE_NAME] = DriveKit::DKRecordField(it.second);
            LOGI("album %{public}s has changed its localPath", album_name.c_str());
            break;
        }
    }

    data[ALBUM_PROPERTIES] = DriveKit::DKRecordField(properties);
    return E_OK;
}

int32_t AlbumDataHandler::QueryConflict(DriveKit::DKRecord &record, std::shared_ptr<NativeRdb::ResultSet> &resultSet)
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
        return E_INVAL_ARG;
    }
    /* query local */
    NativeRdb::AbsRdbPredicates predicates = NativeRdb::AbsRdbPredicates(PAC::TABLE);
    string albumType = to_string(static_cast<int32_t>(AlbumType::NORMAL));
    DKRecordFieldMap properties;
    string bundleName = "";
    if (GetRecordFieMapFromRecord(record, properties) == E_OK && properties.find(PAC::ALBUM_TYPE) != data.end() &&
        properties[PAC::ALBUM_TYPE].GetString(albumType) == DKLocalErrorCode::NO_ERROR &&
        albumType == to_string(static_cast<int32_t>(AlbumType::SOURCE)) &&
        properties[TMP_ALBUM_BUNDLE_NAME].GetString(bundleName) == DKLocalErrorCode::NO_ERROR) {
        int rowCount = 0;
        NativeRdb::AbsRdbPredicates sourcePredicates = NativeRdb::AbsRdbPredicates(PAC::TABLE);
        sourcePredicates.EqualTo(TMP_ALBUM_BUNDLE_NAME, bundleName);
        resultSet = Query(sourcePredicates, QUERY_SOURCE_ALBUM_COLUMNS);
        if (resultSet == nullptr || resultSet->GetRowCount(rowCount) != E_OK) {
            LOGE("query fail");
            return E_RDB;
        }
        if (rowCount >= 0) {
            return E_OK;
        }
        LOGW("conflict but no colmns");
        predicates.EqualTo(PAC::ALBUM_TYPE, albumType);
    }
    predicates.EqualTo(PAC::ALBUM_NAME, albumName);
    resultSet = Query(predicates, ALBUM_LOCAL_QUERY_COLUMNS);
    if (resultSet == nullptr) {
        LOGE("get nullptr query result");
        return E_RDB;
    }
    return E_OK;
}

bool AlbumDataHandler::IsConflict(DriveKit::DKRecord &record, int32_t &albumId)
{
    int32_t rowCount = -1;
    std::shared_ptr<NativeRdb::ResultSet> resultSet = nullptr;
    int32_t ret = QueryConflict(record, resultSet);
    if (ret != E_OK) {
        LOGE("query fail ret is %{public}d", ret);
        return false;
    }
    ret = resultSet->GetRowCount(rowCount);
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
    int32_t ret = Update(changedRows, values, PAC::ALBUM_ID + " = ?", { to_string(albumId) });
    if (ret != E_OK) {
        LOGE("rdb update failed, err = %{public}d", ret);
        return E_RDB;
    }
    return E_OK;
}

int32_t AlbumDataHandler::InsertCloudAlbum(DKRecord &record)
{
    RETURN_ON_ERR(IsStop());
    LOGI("insert of record %s", record.GetRecordId().c_str());
    /* mod properties if it's from dual */
    DriveKit::DKRecordData data;
    record.GetRecordData(data);
    ModRecordIfFromDual(data);
    record.SetRecordData(data);
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
    string albumType = "";
    DKRecordFieldMap properties;
    if (GetRecordFieMapFromRecord(record, properties) == E_OK &&
        properties.find(PAC::ALBUM_TYPE) != data.end() &&
        properties[PAC::ALBUM_TYPE].GetString(albumType) == DKLocalErrorCode::NO_ERROR) {
        if (albumType == to_string(static_cast<int32_t>(AlbumType::SOURCE))) {
            createConvertor_.ExtractBundleName(properties, values);
            createConvertor_.ExtractLocalLanguage(properties, values);
        }
    }
    values.PutInt(PAC::ALBUM_DIRTY, static_cast<int32_t>(Media::DirtyType::TYPE_SYNCED));
    values.PutString(PAC::ALBUM_CLOUD_ID, record.GetRecordId());
    values.PutInt(PAC::ALBUM_IS_LOCAL, ALBUM_FROM_CLOUD);
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
    RETURN_ON_ERR(IsStop());
    int32_t changedRows;
    ValuesBucket values;
    int32_t ret = createConvertor_.RecordToValueBucket(record, values);
    if (ret != E_OK) {
        LOGE("record to value bucket failed, ret = %{public}d", ret);
        return ret;
    }
    DriveKit::DKRecordData data;
    DKRecordFieldMap properties;
    record.GetRecordData(data);
    string albumType = to_string(static_cast<int32_t>(AlbumType::NORMAL));
    if (GetRecordFieMapFromRecord(record, properties) == E_OK &&
        properties.find(PAC::ALBUM_TYPE) != data.end() &&
        properties[PAC::ALBUM_TYPE].GetString(albumType) == DKLocalErrorCode::NO_ERROR) {
        if (albumType == to_string(static_cast<int32_t>(AlbumType::SOURCE))) {
            values.Delete(PAC::ALBUM_NAME);
        }
    }

    values.PutInt(PAC::ALBUM_DIRTY, static_cast<int32_t>(Media::DirtyType::TYPE_SYNCED));
    ret = Update(changedRows, values, PAC::ALBUM_CLOUD_ID + " = ?", {record.GetRecordId()});
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
        int32_t ret = Update(changedRows, values, PAC::ALBUM_CLOUD_ID + " = ?", {record.GetRecordId()});
        if (ret != E_OK) {
            LOGE("rdb update failed, err = %{public}d", ret);
            return E_RDB;
        }
        return E_OK;
    }
    return E_OK;
}

void AlbumDataHandler::UpdateDownloadAlbumStat(uint64_t success, uint64_t rdbFail, uint64_t dataFail)
{
    UpdateAlbumStat(INDEX_DL_ALBUM_SUCCESS, success);
    UpdateAlbumStat(INDEX_DL_ALBUM_ERROR_RDB, rdbFail);
    UpdateAlbumStat(INDEX_DL_ALBUM_ERROR_DATA, dataFail);
}

int32_t AlbumDataHandler::OnFetchRecords(shared_ptr<vector<DKRecord>> &records,
                                         OnFetchParams &params)
{
    LOGI("on fetch %{public}zu records", records->size());
    int32_t ret = E_OK;
    uint64_t success = 0;
    uint64_t rdbFail = 0;
    uint64_t dataFail = 0;

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
            ret = E_DATA;
        }

        /* check ret */
        if (ret != E_OK) {
            LOGE("recordId %s error %{public}d", record.GetRecordId().c_str(), ret);
            /* might need specific error type */
            ret == E_RDB ? rdbFail++ : dataFail++;
            if (ret == E_STOP) {
                UpdateDownloadAlbumStat(success, rdbFail, dataFail);
                (void)DataSyncNotifier::GetInstance().FinalNotify();
                return E_STOP;
            }
            continue;
        } else {
            /* notify */
            success++;
            (void)DataSyncNotifier::GetInstance().TryNotify(ALBUM_URI_PREFIX,
                ChangeType::UPDATE, INVALID_ASSET_ID);
        }
    }
    (void)DataSyncNotifier::GetInstance().FinalNotify();

    UpdateDownloadAlbumStat(success, rdbFail, dataFail);

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

    string albumCloudSql = " FROM " + PAC::TABLE + " WHERE " + PAC::ALBUM_IMAGE_COUNT + " = 0 AND " +
        PAC::ALBUM_IS_LOCAL + " = " + to_string(ALBUM_FROM_CLOUD);

    mapSql = "DELETE FROM " + PM::TABLE + " WHERE " + PM::ALBUM_ID + " = ( SELECT " +
        PAC::ALBUM_ID + albumCloudSql + ")";
    ret = ExecuteSql(mapSql);
    if (ret != NativeRdb::E_OK) {
        LOGE("delete album maps err %{public}d", ret);
        return ret;
    }

    mapSql = "DELETE" + albumCloudSql;
    ret = ExecuteSql(mapSql);
    if (ret != NativeRdb::E_OK) {
        LOGE("delete album from cloud err %{public}d", ret);
        return ret;
    }
    /* notify */
    (void)DataSyncNotifier::GetInstance().TryNotify(ALBUM_URI_PREFIX,
        ChangeType::DELETE, INVALID_ASSET_ID);
    (void)DataSyncNotifier::GetInstance().FinalNotify();
    return E_OK;
}

int32_t AlbumDataHandler::QueryUserAlbum(vector<DKRecord> &records, Media::DirtyType dirty,
                                         const vector<string> &failSet, AlbumDataConvertor &convertor)
{
    vector<DKRecord> userRecords = {};
    /* build predicates */
    auto createPredicates = NativeRdb::AbsRdbPredicates(PAC::TABLE);
    createPredicates.EqualTo(PAC::ALBUM_DIRTY, to_string(static_cast<int32_t>(dirty)));
    /* skip system albums */
    createPredicates.And()->EqualTo(PAC::ALBUM_SUBTYPE, to_string(Media::PhotoAlbumSubType::USER_GENERIC));
    if (!failSet.empty()) {
        createPredicates.And()->NotIn(PAC::ALBUM_CLOUD_ID, failSet);
    }
    createPredicates.Limit(LIMIT_SIZE);

    /* query */
    auto results = Query(createPredicates, QUERY_USER_ALBUM_COLUMNS);
    if (results == nullptr) {
        LOGE("get nullptr created result");
        return E_RDB;
    }
    /* results to records */
    convertor.ResultSetToRecords(move(results), userRecords);
    for (auto &record : userRecords) {
        records.emplace_back(move(record));
    }
    return E_OK;
}

int32_t AlbumDataHandler::QuerySourceAlbum(vector<DKRecord> &records, Media::DirtyType dirty,
                                           const vector<string> &failSet, AlbumDataConvertor &convertor)
{
    vector<DKRecord> suourcRecords = {};
    /* build predicates */
    auto createPredicates = NativeRdb::AbsRdbPredicates(PAC::TABLE);
    createPredicates.EqualTo(PAC::ALBUM_DIRTY, to_string(static_cast<int32_t>(dirty)));
    /* skip system albums */
    createPredicates.And()->EqualTo(PAC::ALBUM_SUBTYPE, to_string(SURCE_GENERIC));
    if (!failSet.empty()) {
        createPredicates.And()->NotIn(PAC::ALBUM_CLOUD_ID, failSet);
    }
    createPredicates.Limit(LIMIT_SIZE);

    /* query */
    auto results = Query(createPredicates, QUERY_SOURCE_ALBUM_COLUMNS);
    if (results == nullptr) {
        LOGE("get nullptr created result");
        return E_RDB;
    }
    /* results to records */
    convertor.ResultSetToRecords(move(results), suourcRecords);
    for (auto &record : suourcRecords) {
        records.emplace_back(move(record));
    }
    return E_OK;
}

int32_t AlbumDataHandler::GetCreatedRecords(vector<DKRecord> &records)
{
    RETURN_ON_ERR(QueryUserAlbum(records, Media::DirtyType::TYPE_NEW, createFailSet_, createConvertor_));
    RETURN_ON_ERR(QuerySourceAlbum(records, Media::DirtyType::TYPE_NEW, createFailSet_, createConvertor_));
    int32_t changedRows = -1;
    DKRecordData data;
    string albumId = "";
    for (auto &record : records) {
        ValuesBucket values;
        values.Put(PAC::ALBUM_CLOUD_ID, record.GetRecordId());
        record.GetRecordData(data);
        data[ALBUM_ID].GetString(albumId);
        data[ALBUM_ID] = DKRecordField(record.GetRecordId());
        record.SetRecordData(data);
        Update(changedRows, values, PAC::ALBUM_ID + " = ?", {albumId});
    }
    return E_OK;
}

int32_t AlbumDataHandler::GetDeletedRecords(vector<DKRecord> &records)
{
    RETURN_ON_ERR(QueryUserAlbum(records, Media::DirtyType::TYPE_DELETED, modifyFailSet_, deleteConvertor_));
    RETURN_ON_ERR(QuerySourceAlbum(records, Media::DirtyType::TYPE_DELETED, modifyFailSet_, deleteConvertor_));
    return E_OK;
}

int32_t AlbumDataHandler::GetMetaModifiedRecords(vector<DKRecord> &records)
{
    RETURN_ON_ERR(QueryUserAlbum(records, Media::DirtyType::TYPE_MDIRTY, modifyFailSet_, modifyConvertor_));
    RETURN_ON_ERR(QuerySourceAlbum(records, Media::DirtyType::TYPE_MDIRTY, modifyFailSet_, modifyConvertor_));
    return E_OK;
}

int32_t AlbumDataHandler::OnCreateRecords(const map<DKRecordId, DKRecordOperResult> &map)
{
    int32_t ret = E_OK;
    uint64_t success = 0;
    uint64_t failure = 0;

    for (auto &entry : map) {
        int32_t err;
        const DKRecordOperResult &result = entry.second;
        if (result.IsSuccess()) {
            err = OnCreateSuccess(entry);
        } else {
            err = OnRecordFailed(entry);
            OnCreateFail(entry);
            failure++;
        }
        if (err == E_OK) {
            success++;
        }
        GetReturn(err, ret);
    }

    UpdateAlbumStat(INDEX_UL_ALBUM_SUCCESS, success);
    UpdateAlbumStat(INDEX_UL_ALBUM_ERROR_SDK, failure);

    return ret;
}

int32_t AlbumDataHandler::OnDeleteRecords(const map<DKRecordId, DKRecordOperResult> &map)
{
    int32_t ret = E_OK;
    uint64_t success = 0;
    uint64_t failure = 0;

    for (auto &entry : map) {
        int32_t err;
        const DKRecordOperResult &result = entry.second;
        if (result.IsSuccess()) {
            err = OnDeleteSuccess(entry);
        } else {
            err = OnRecordFailed(entry);
            OnDeleteFail(entry);
            failure++;
        }
        if (err == E_OK) {
            success++;
        }
        GetReturn(err, ret);
    }

    UpdateAlbumStat(INDEX_UL_ALBUM_SUCCESS, success);
    UpdateAlbumStat(INDEX_UL_ALBUM_ERROR_SDK, failure);

    return ret;
}

int32_t AlbumDataHandler::OnModifyMdirtyRecords(const map<DKRecordId, DKRecordOperResult> &map)
{
    int32_t ret = E_OK;
    uint64_t success = 0;
    uint64_t failure = 0;

    for (auto &entry : map) {
        int32_t err;
        const DKRecordOperResult &result = entry.second;
        if (result.IsSuccess()) {
            err = OnUploadSuccess(entry);
        } else {
            err = OnRecordFailed(entry);
            OnModifyFail(entry);
            failure++;
        }
        if (err == E_OK) {
            success++;
        }
        GetReturn(err, ret);
    }
    return ret;
}

int32_t AlbumDataHandler::OnCreateSuccess(const pair<DriveKit::DKRecordId,
    DriveKit::DKRecordOperResult> &entry)
{
    return OnUploadSuccess(entry);
}

int32_t AlbumDataHandler::OnUploadSuccess(const pair<DriveKit::DKRecordId,
    DriveKit::DKRecordOperResult> &entry)
{
    int32_t changedRows;
    ValuesBucket valuesBucket;
    string whereClause = PAC::ALBUM_CLOUD_ID + " = ?";
    vector<string> whereArgs = { entry.first };
    valuesBucket.PutInt(PAC::ALBUM_DIRTY, static_cast<int32_t>(Media::DirtyType::TYPE_SYNCED));
    int32_t ret = Update(changedRows, valuesBucket, whereClause, whereArgs);
    if (ret != 0) {
        LOGE("update local records err %{public}d", ret);
        UpdateAlbumStat(INDEX_UL_ALBUM_ERROR_RDB, 1);
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
        UpdateAlbumStat(INDEX_UL_ALBUM_ERROR_RDB, 1);
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
