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

#include "data_syncer_rdb_col.h"
#include "dfs_error.h"
#include "rdb_helper.h"
#include "rdb_sql_utils.h"
#include "rdb_store_config.h"
#include "result_set.h"
#include "utils_log.h"

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
    auto queryRet = Query(predicates, resultSet);
    if (queryRet != E_OK) {
        return queryRet;
    }
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
        if (rdb_ == nullptr) {
            if (RdbInit() != E_OK) {
                LOGE("Data Syner init rdb failed");
                return E_RDB;
            }
        }
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

int32_t DataSyncerRdbStore::UpdateSyncState(int32_t userId, const string &bundleName,
    CloudSyncState cloudSyncState, ErrorType errorType)
{
    int updateRows;
    NativeRdb::ValuesBucket values;
    values.PutInt(SYNC_STATE, static_cast<int32_t>(cloudSyncState));
    if (cloudSyncState == CloudSyncState::COMPLETED && errorType == ErrorType::NO_ERROR) {
        values.PutLong(LAST_SYNC_TIME, UTCTimeMilliSeconds());
    }
    values.PutInt(SYNC_ERROR_TYPE, static_cast<int32_t>(errorType));
    string whereClause = USER_ID + " = ? AND " + BUNDLE_NAME + " = ?";
    vector<string> whereArgs = { to_string(userId), bundleName };
    if (rdb_ == nullptr) {
        if (RdbInit() != E_OK) {
            LOGE("Data Syner init rdb failed");
            return E_RDB;
        }
    }
    int32_t ret = rdb_->Update(updateRows, DATA_SYNCER_TABLE, values, whereClause, whereArgs);
    if (ret != E_OK) {
        LOGE("update sync state failed: %{public}d", ret);
        return E_RDB;
    }
    if (updateRows <= 0) {
        LOGE("update sync state with no lines changed");
        return E_RDB;
    }
    return E_OK;
}

static int32_t GetLong(const string &key, int64_t &val, NativeRdb::ResultSet &resultSet)
{
    int32_t index;
    int32_t err = resultSet.GetColumnIndex(key, index);
    if (err != NativeRdb::E_OK) {
        LOGE("result set get  %{public}s column index err %{public}d", key.c_str(), err);
        return E_RDB;
    }

    err = resultSet.GetLong(index, val);
    if (err != NativeRdb::E_OK) {
        LOGE("result set get long err %{public}d", err);
        return E_RDB;
    }

    return E_OK;
}

int32_t DataSyncerRdbStore::GetLastSyncTime(int32_t userId, const string &bundleName, int64_t &time)
{
    LOGI("get sync time uid: %{public}d, name: %{public}s", userId, bundleName.c_str());
    NativeRdb::AbsRdbPredicates predicates = NativeRdb::AbsRdbPredicates(DATA_SYNCER_TABLE);
    predicates.EqualTo(USER_ID, userId)->EqualTo(BUNDLE_NAME, bundleName);
    std::shared_ptr<NativeRdb::ResultSet> resultSet = nullptr;

    auto queryRet = Query(predicates, resultSet);
    if (queryRet != E_OK) {
        return queryRet;
    }
    if (resultSet->GoToNextRow() != E_OK) {
        return E_INVAL_ARG;
    }
    int32_t ret = GetLong(LAST_SYNC_TIME, time, *resultSet);
    if (ret != E_OK) {
        LOGE("get last sync time failed");
        return ret;
    }
    return E_OK;
}

static int32_t GetInt(const string &key, int32_t &val, NativeRdb::ResultSet &resultSet)
{
    int32_t index;
    int32_t err = resultSet.GetColumnIndex(key, index);
    if (err != NativeRdb::E_OK) {
        LOGE("result set get  %{public}s column index err %{public}d", key.c_str(), err);
        return E_RDB;
    }

    err = resultSet.GetInt(index, val);
    if (err != NativeRdb::E_OK) {
        LOGE("result set get int err %{public}d", err);
        return E_RDB;
    }

    return E_OK;
}

int32_t DataSyncerRdbStore::GetSyncStateAndErrorType(int32_t userId, const std::string &bundleName,
    CloudSyncState &cloudSyncState, ErrorType &errorType)
{
    NativeRdb::AbsRdbPredicates predicates = NativeRdb::AbsRdbPredicates(DATA_SYNCER_TABLE);
    predicates.EqualTo(USER_ID, userId)->EqualTo(BUNDLE_NAME, bundleName);
    std::shared_ptr<NativeRdb::ResultSet> resultSet = nullptr;

    auto queryRet = Query(predicates, resultSet);
    if (queryRet != E_OK) {
        LOGE("get sync state query failed");
        return queryRet;
    }
    if (resultSet->GoToNextRow() != E_OK) {
        LOGE("get sync state no more rows");
        return E_INVAL_ARG;
    }

    int32_t syncState = static_cast<int32_t>(CloudSyncState::COMPLETED);
    int32_t ret = GetInt(SYNC_STATE, syncState, *resultSet);
    if (ret != E_OK) {
        LOGE("get sync state failed");
        return ret;
    } else {
        cloudSyncState = static_cast<CloudSyncState>(syncState);
    }

    int32_t err = static_cast<int32_t>(ErrorType::NO_ERROR);
    ret = GetInt(SYNC_ERROR_TYPE, err, *resultSet);
    if (ret != E_OK) {
        LOGE("get error type failed");
        return ret;
    } else {
        errorType = static_cast<ErrorType>(err);
    }
    return E_OK;
}

int32_t DataSyncerRdbStore::QueryDataSyncer(int32_t userId, std::shared_ptr<NativeRdb::ResultSet> &resultSet)
{
    NativeRdb::AbsRdbPredicates predicates = NativeRdb::AbsRdbPredicates(DATA_SYNCER_TABLE);
    predicates.EqualTo(USER_ID, userId);
    return Query(predicates, resultSet);
}

int32_t DataSyncerRdbStore::QueryCloudSync(int32_t userId,
                                           const std::string &bundleName,
                                           std::shared_ptr<NativeRdb::ResultSet> &resultSet)
{
    NativeRdb::AbsRdbPredicates predicates = NativeRdb::AbsRdbPredicates(DATA_SYNCER_TABLE);
    predicates.EqualTo(USER_ID, userId);
    predicates.EqualTo(BUNDLE_NAME, bundleName);
    return Query(predicates, resultSet);
}

int32_t DataSyncerRdbStore::Query(NativeRdb::AbsRdbPredicates predicates,
    std::shared_ptr<NativeRdb::ResultSet> &resultSet)
{
    if (rdb_ == nullptr) {
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

static void VersionAddDataSyncerErrorType(NativeRdb::RdbStore &store)
{
    const string addDataSyncerErrorType = CREATE_DATA_SYNCER_ERROR_TYPE;
    if (store.ExecuteSql(addDataSyncerErrorType) != NativeRdb::E_OK) {
        LOGE("upgrade fail add errorType");
    }
    const string updateCloudSyncState = UPDATE_CLOUD_SYNC_STATE;
    if (store.ExecuteSql(updateCloudSyncState) != NativeRdb::E_OK) {
        LOGE("upgrade fail sync state -> cloud sync state");
    }
}

int32_t DataSyncerRdbCallBack::OnUpgrade(NativeRdb::RdbStore &store, int32_t oldVersion, int32_t newVersion)
{
    LOGD("OnUpgrade old:%d, new:%d", oldVersion, newVersion);
    if (oldVersion < VERSION_ADD_DATA_SYNCER_UNIQUE_INDEX) {
        VersionAddDataSyncerUniqueIndex(store);
    }
    if (oldVersion < VERSION_ADD_ERROR_TYPE) {
        VersionAddDataSyncerErrorType(store);
    }
    return E_OK;
}
} // namespace OHOS::FileManagement::CloudSync
