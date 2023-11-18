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
    NativeRdb::AbsRdbPredicates predicates = NativeRdb::AbsRdbPredicates(DATA_SYNCER_TABLE);
    predicates.EqualTo(USER_ID, userId)->And()->EqualTo(BUNDLE_NAME, bundleName);
    std::shared_ptr<NativeRdb::ResultSet> resultSet;
    RETURN_ON_ERR(Query(predicates, resultSet));
    int32_t rowCount = 0;
    if (resultSet->GetRowCount(rowCount) == E_OK && rowCount == 1) {
        return E_OK;
    }
    int64_t rowId;
    NativeRdb::ValuesBucket values;
    values.PutInt(USER_ID, userId);
    values.PutString(BUNDLE_NAME, bundleName);
    int32_t ret = rdb_->Insert(rowId, DATA_SYNCER_TABLE, values);
    if (ret != E_OK) {
        LOGE("fail to insert userId %d bundleName %s, ret %{public}d", userId, bundleName.c_str(), ret);
        return E_RDB;
    }

    return E_OK;
}

int32_t DataSyncerRdbStore::UpdateSyncState(int32_t userId, const string &bundleName, SyncState syncState)
{
    LOGI("update syncstate %{public}d", syncState);
    int updateRows;
    NativeRdb::ValuesBucket values;
    values.PutInt(SYNC_STATE, static_cast<int32_t>(syncState));
    string whereClause = USER_ID + " = ? AND " + BUNDLE_NAME + " = ?";
    vector<string> whereArgs = { to_string(userId), bundleName };
    int32_t ret = rdb_->Update(updateRows, DATA_SYNCER_TABLE, values, whereClause, whereArgs);
    if (ret != E_OK) {
        LOGE("update sync state failed: %{public}d", ret);
        return E_OK;
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
    if (rdb_ == nullptr) {
        if (RdbInit() != E_OK) {
            LOGE("Data Syner init rdb failed");
            return E_RDB;
        }
    }
    resultSet = rdb_->Query(predicates, {});
    int32_t rowCount = 0;
    int32_t ret = E_OK;
    if (resultSet) {
        ret = resultSet->GetRowCount(rowCount);
    }
    if (resultSet == nullptr || ret != E_OK || rowCount < 0) {
        LOGE("query sync state failed ret = %{public}d, rowcount = %{public}d", ret, rowCount);
        return E_RDB;
    }
    return E_OK;
}


int32_t DataSyncerRdbCallBack::OnCreate(NativeRdb::RdbStore &store)
{
    vector<string> executeSqlStrs = {
        CREATE_DATA_SYNCER_TABLE,
    };

    for (const string& sqlStr : executeSqlStrs) {
        if (store.ExecuteSql(sqlStr) != E_OK) {
            LOGE("create table failed.");
            return E_RDB;
        }
    }
    return E_OK;
}

int32_t DataSyncerRdbCallBack::OnUpgrade(NativeRdb::RdbStore &store, int32_t oldVersion, int32_t newVersion)
{
    return E_OK;
}
} // namespace OHOS::FileManagement::CloudSync
