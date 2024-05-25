/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "gallery_rdb_utils.h"

#include "data_convertor.h"
#include "dfs_error.h"
#include "medialibrary_business_record_column.h"
#include "media_column.h"
#include "media_refresh_album_column.h"
#include "photo_album_column.h"
#include "rdb_predicates.h"
#include "userfile_manager_types.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
using namespace std;
using namespace NativeRdb;
using namespace Media;

const std::vector<std::string> ALL_SYS_PHOTO_ALBUM = {
    std::to_string(PhotoAlbumSubType::FAVORITE),
    std::to_string(PhotoAlbumSubType::VIDEO),
    std::to_string(PhotoAlbumSubType::HIDDEN),
    std::to_string(PhotoAlbumSubType::TRASH),
    std::to_string(PhotoAlbumSubType::SCREENSHOT),
    std::to_string(PhotoAlbumSubType::CAMERA),
    std::to_string(PhotoAlbumSubType::IMAGE),
    std::to_string(PhotoAlbumSubType::SOURCE_GENERIC),
};

struct BussinessRecordValue {
    string bussinessType;
    string key;
    string value;
};

const string ANALYSIS_REFRESH_BUSINESS_TYPE = "ANALYSIS_ALBUM_REFRESH";

static shared_ptr<AbsSharedResultSet> Query(const shared_ptr<NativeRdb::RdbStore> &rdbStore,
    AbsRdbPredicates &predicates, const vector<string> &columns)
{
    /* build all-table vector */
    string tableName = predicates.GetTableName();
    vector<string> joinTables = predicates.GetJoinTableNames();
    joinTables.push_back(tableName);
    /* add filters */
    string filters;
    for (auto t : joinTables) {
        string filter = PhotoAlbumColumns::TABLE + "." + PhotoAlbumColumns::ALBUM_DIRTY + " != " +
            to_string(static_cast<int32_t>(DirtyTypes::TYPE_DELETED));
        if (filter.empty()) {
            continue;
        }
        if (filters.empty()) {
            filters += filter;
        } else {
            filters += " AND " + filter;
        }
    }
    if (filters.empty()) {
        return nullptr;
    }

    /* rebuild */
    string queryCondition = predicates.GetWhereClause();
    queryCondition = queryCondition.empty() ? filters : filters + " AND " + queryCondition;
    predicates.SetWhereClause(queryCondition);
    return rdbStore->Query(predicates, columns);
}

static inline shared_ptr<ResultSet> GetAlbums(const shared_ptr<RdbStore> &rdbStore,
    const vector<string> &subtypes, const vector<string> &columns)
{
    RdbPredicates predicates(PhotoAlbumColumns::TABLE);
    if (subtypes.empty()) {
        predicates.In(PhotoAlbumColumns::ALBUM_SUBTYPE, ALL_SYS_PHOTO_ALBUM);
    } else {
        predicates.In(PhotoAlbumColumns::ALBUM_SUBTYPE, subtypes);
    }
    return Query(rdbStore, predicates, columns);
}

static int32_t UpdateAlbumReplacedSignal(const shared_ptr<RdbStore> &rdbStore,
    const vector<string> &albumIdVector)
{
    if (albumIdVector.empty()) {
        return E_OK;
    }

    ValuesBucket refreshValues;
    string insertRefreshTableSql = "INSERT OR IGNORE INTO " + ALBUM_REFRESH_TABLE + " VALUES ";
    for (size_t i = 0; i < albumIdVector.size(); ++i) {
        if (i != albumIdVector.size() - 1) {
            insertRefreshTableSql += "(" + albumIdVector[i] + "), ";
        } else {
            insertRefreshTableSql += "(" + albumIdVector[i] + ");";
        }
    }
    LOGD("output insertRefreshTableSql:%{public}s", insertRefreshTableSql.c_str());

    int32_t ret = rdbStore->ExecuteSql(insertRefreshTableSql);
    if (ret != NativeRdb::E_OK) {
        LOGW("Can not insert refreshed table, ret:%{public}d", ret);
        return E_RDB;
    }
    return E_OK;
}

void GalleryRdbUtils::UpdateAlbumCountInternal(const shared_ptr<RdbStore> &rdbStore,
    const vector<string> &subtypes)
{
    vector<string> columns = { PhotoAlbumColumns::ALBUM_ID };
    auto albumResult = GetAlbums(rdbStore, subtypes, columns);
    if (albumResult == nullptr) {
        return;
    }

    vector<string> replaceSignalAlbumVector;
    while (albumResult->GoToNextRow() == NativeRdb::E_OK) {
        int32_t albumId = 0;
        int32_t ret = DataConvertor::GetInt(PhotoAlbumColumns::ALBUM_ID, albumId, *albumResult);
        if (ret != E_OK) {
            LOGW("Can not get ret:%{public}d", ret);
        } else {
            replaceSignalAlbumVector.push_back(to_string(albumId));
        }
    }
    if (!replaceSignalAlbumVector.empty()) {
        int32_t ret = UpdateAlbumReplacedSignal(rdbStore, replaceSignalAlbumVector);
        if (ret != E_OK) {
            LOGW("Update sysalbum replaced signal failed ret:%{public}d", ret);
        }
    }
}

static int32_t UpdateBussinessRecord(const shared_ptr<RdbStore> &rdbStore,
    const vector<BussinessRecordValue> &updateValue)
{
    if (updateValue.empty()) {
        return E_OK;
    }

    ValuesBucket refreshValues;
    string insertTableSql = "INSERT OR IGNORE INTO " + MedialibraryBusinessRecordColumn::TABLE + "(" +
        MedialibraryBusinessRecordColumn::BUSINESS_TYPE + "," + MedialibraryBusinessRecordColumn::KEY + "," +
        MedialibraryBusinessRecordColumn::VALUE + ") VALUES ";
    for (size_t i = 0; i < updateValue.size(); ++i) {
        if (i != updateValue.size() - 1) {
            insertTableSql += "('" + updateValue[i].bussinessType + "', '" + updateValue[i].key + "', '" +
                updateValue[i].value + "'), ";
        } else {
            insertTableSql += "('" + updateValue[i].bussinessType + "', '" + updateValue[i].key + "', '" +
                updateValue[i].value + "');";
        }
    }
    LOGD("output insertTableSql:%{public}s", insertTableSql.c_str());

    int32_t ret = rdbStore->ExecuteSql(insertTableSql);
    if (ret != NativeRdb::E_OK) {
        LOGW("Can not insert bussinessRecord table, ret:%{public}d", ret);
        return E_RDB;
    }
    return E_OK;
}

void GalleryRdbUtils::UpdateAnalysisAlbumCountInternal(const shared_ptr<RdbStore> &rdbStore,
    const vector<string> &subtypes)
{
    vector<string> columns = { PhotoAlbumColumns::ALBUM_ID, PhotoAlbumColumns::ALBUM_SUBTYPE };
    auto albumResult = GetAlbums(rdbStore, subtypes, columns);
    if (albumResult == nullptr) {
        return;
    }

    vector<BussinessRecordValue> updateAlbumIdList;
    while (albumResult->GoToNextRow() == NativeRdb::E_OK) {
        int32_t albumId = 0;
        int32_t subtype = 0;
        if (DataConvertor::GetInt(PhotoAlbumColumns::ALBUM_ID, albumId, *albumResult) != E_OK) {
            LOGW("Can not get album id");
            continue;
        }
        if (DataConvertor::GetInt(PhotoAlbumColumns::ALBUM_SUBTYPE, subtype, *albumResult) != E_OK) {
            LOGW("Can not get album subtype");
            continue;
        }
        updateAlbumIdList.push_back({ ANALYSIS_REFRESH_BUSINESS_TYPE, to_string(subtype), to_string(albumId) });
    }
    if (!updateAlbumIdList.empty()) {
        int32_t ret = UpdateBussinessRecord(rdbStore, updateAlbumIdList);
        if (ret != E_OK) {
            LOGW("Update sysalbum replaced signal failed ret:%{public}d", ret);
        }
    }
}

void GalleryRdbUtils::UpdateAllAlbumsCountForCloud(const std::shared_ptr<NativeRdb::RdbStore> &rdbStore)
{
    GalleryRdbUtils::UpdateAlbumCountInternal(rdbStore, ALL_SYS_PHOTO_ALBUM);
    GalleryRdbUtils::UpdateAlbumCountInternal(rdbStore, {to_string(PhotoAlbumSubType::USER_GENERIC)});
    vector<string> subtype = { "4101" };
    GalleryRdbUtils::UpdateAnalysisAlbumCountInternal(rdbStore, subtype);
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
