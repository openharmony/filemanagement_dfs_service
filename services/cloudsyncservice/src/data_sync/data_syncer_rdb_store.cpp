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

#include "data_syncer_rdb_store.h"

#include "data_convertor.h"
#include "data_syncer_rdb_col.h"
#include "dfs_error.h"
#include "utils_log.h"
#include "rdb_helper.h"
#include "rdb_sql_utils.h"
#include "rdb_store_config.h"

namespace OHOS::FileManagement::CloudSync {
using namespace std;

DataSyncerRdbStore &DataSyncerRdbStore::GetInstance()
{
    static DataSyncerRdbStore instance;
    return instance;
}

int32_t DataSyncerRdbStore::RdbInit()
{
    LOGI("Init rdb store");
    int32_t errCode = 0;
    string databasePath = NativeRdb::RdbSqlUtils::GetDefaultDatabasePath(EL1_CLOUDFILE_DIR, DATA_SYNCER_DB, errCode);

    if (errCode != E_OK) {
        LOGE("Create Default Database Path is failed, errCode = %{public}d", errCode);
        return E_RDB;
    }
    NativeRdb::RdbStoreConfig config{""};
    config.SetName(move(DATA_SYNCER_DB));
    config.SetPath(move(databasePath));
    errCode = E_OK;
    DataSyncerRdbCallBack rdbDataCallBack;
    rdb_ = NativeRdb::RdbHelper::GetRdbStore(config, CLOUD_DISK_RDB_VERSION, rdbDataCallBack, errCode);
    if (rdb_ == nullptr) {
        LOGE("GetRdbStore is failed,  errCode = %{public}d", errCode);
        return E_RDB;
    }

    return E_OK;
}

int32_t DataSyncerRdbStore::Insert(int32_t userId, const std::string &bundleName)
{
    LOGI("datasyncer insert userId %d, bundleName %s", userId, bundleName.c_str());
    string uniqueId = to_string(userId) + bundleName;
    NativeRdb::AbsRdbPredicates predicates = NativeRdb::AbsRdbPredicates(DATA_SYNCER_TABLE);
    predicates.EqualTo(DATA_SYNCER_UNIQUE_ID, uniqueId);
    std::shared_ptr<NativeRdb::ResultSet> resultSet = nullptr;
    RETURN_ON_ERR(Query(predicates, resultSet));
    if (resultSet->GoToNextRow() == E_OK) {
        return E_OK;
    }
    NativeRdb::ValuesBucket values;
    values.PutInt(USER_ID, userId);
    values.PutString(BUNDLE_NAME, bundleName);
    values.PutString(DATA_SYNCER_UNIQUE_ID, uniqueId);
    int32_t ret = 0;
    {
        std::lock_guard<std::mutex> lck(rdbMutex_);
        int64_t rowId;
        ret = rdb_->Insert(rowId, DATA_SYNCER_TABLE, values);
    }
    if (ret != E_OK) {
        LOGE("fail to insert userId %d bundleName %s, ret %{public}d", userId, bundleName.c_str(), ret);
        return E_RDB;
    }

    return E_OK;
}

static int64_t UTCTimeMilliSeconds()
{
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    return t.tv_sec * SECOND_TO_MILLISECOND + t.tv_nsec / MILLISECOND_TO_NANOSECOND;
}

int32_t DataSyncerRdbStore::UpdateSyncState(int32_t userId, const string &bundleName, SyncState syncState)
{
    LOGI("update syncstate %{public}d", syncState);
    int updateRows;
    NativeRdb::ValuesBucket values;
    values.PutInt(SYNC_STATE, static_cast<int32_t>(syncState));
    if (syncState == SyncState::SYNC_SUCCEED) {
        values.PutLong(LAST_SYNC_TIME, UTCTimeMilliSeconds());
    }
    string whereClause = USER_ID + " = ? AND " + BUNDLE_NAME + " = ?";
    vector<string> whereArgs = { to_string(userId), bundleName };
    int32_t ret = rdb_->Update(updateRows, DATA_SYNCER_TABLE, values, whereClause, whereArgs);
    if (ret != E_OK) {
        LOGE("update sync state failed: %{public}d", ret);
        return E_OK;
    }
    return E_OK;
}

int32_t DataSyncerRdbStore::GetLastSyncTime(int32_t userId, const string &bundleName, int64_t &time)
{
    LOGI("get sync time uid: %{public}d, name: %{public}s", userId, bundleName.c_str());
    NativeRdb::AbsRdbPredicates predicates = NativeRdb::AbsRdbPredicates(DATA_SYNCER_TABLE);
    predicates.EqualTo(USER_ID, userId)->EqualTo(BUNDLE_NAME, bundleName);
    std::shared_ptr<NativeRdb::ResultSet> resultSet = nullptr;

    RETURN_ON_ERR(Query(predicates, resultSet));
    if (resultSet->GoToNextRow() != E_OK) {
        return E_INVAL_ARG;
    }
    int32_t ret = DataConvertor::GetLong(LAST_SYNC_TIME, time, *resultSet);
    if (ret != E_OK) {
        LOGE("get last sync time failed");
        return ret;
    }
    return E_OK;
}

int32_t DataSyncerRdbStore::QueryDataSyncer(int32_t userId, std::shared_ptr<NativeRdb::ResultSet> &resultSet)
{
    NativeRdb::AbsRdbPredicates predicates = NativeRdb::AbsRdbPredicates(DATA_SYNCER_TABLE);
    predicates.EqualTo(USER_ID, userId);
    return Query(predicates, resultSet);
}

int32_t DataSyncerRdbStore::Query(NativeRdb::AbsRdbPredicates predicates,
    std::shared_ptr<NativeRdb::ResultSet> &resultSet)
{
    while (rdb_ == nullptr) {
        if (RdbInit() != E_OK) {
            LOGE("Data Syner init rdb failed");
            return E_RDB;
        }
    }
    resultSet = rdb_->QueryByStep(predicates, {});
    if (resultSet == nullptr) {
        LOGE("DataSyncerRdbStore query failed");
        return E_RDB;
    }
    return E_OK;
}


int32_t DataSyncerRdbCallBack::OnCreate(NativeRdb::RdbStore &store)
{
    vector<string> executeSqlStrs = {
        CREATE_DATA_SYNCER_TABLE,
        CREATE_DATA_SYNCER_UNIQUE_INDEX,
    };

    for (const string& sqlStr : executeSqlStrs) {
        if (store.ExecuteSql(sqlStr) != E_OK) {
            LOGE("create table failed.");
            return E_RDB;
        }
    }
    return E_OK;
}

static void VersionAddDataSyncerUniqueIndex(NativeRdb::RdbStore &store)
{
    const string addDataSyncerUniqueIdColumn =
        "ALTER TABLE " + DATA_SYNCER_TABLE + " ADD COLUMN " +
        DATA_SYNCER_UNIQUE_ID + " TEXT";
    const string initUniqueIdColumn =
        "UPDATE " + DATA_SYNCER_TABLE + " SET " +
        DATA_SYNCER_UNIQUE_ID + " = userId || bundleName";
    const string addDataSyncerUniqueIndex = CREATE_DATA_SYNCER_UNIQUE_INDEX;
    const vector<string> addUniqueIndex = { addDataSyncerUniqueIdColumn, initUniqueIdColumn,
        addDataSyncerUniqueIndex};
    for (size_t i = 0; i < addUniqueIndex.size(); i++) {
        if (store.ExecuteSql(addUniqueIndex[i]) != NativeRdb::E_OK) {
            LOGE("upgrade fail idx:%{public}zu", i);
        }
    }
}

int32_t DataSyncerRdbCallBack::OnUpgrade(NativeRdb::RdbStore &store, int32_t oldVersion, int32_t newVersion)
{
    LOGD("OnUpgrade old:%d, new:%d", oldVersion, newVersion);
    if (oldVersion < VERSION_ADD_DATA_SYNCER_UNIQUE_INDEX) {
        VersionAddDataSyncerUniqueIndex(store);
    }
    return E_OK;
}
} // namespace OHOS::FileManagement::CloudSync
