/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "cloud_lru_rdb_store.h"

#include <vector>

#include "rdb_helper.h"
#include "rdb_sql_utils.h"
#include "rdb_store_config.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudFile {

const std::string CLOUD_LRU_DB = "CloudLru.db";

const std::string INODE_INO = "inodeIno";
const std::string PARENT_INODE_INO = "parentInodeIno";
const std::string CLOUD_ID = "cloudId";
const std::string HASH = "hash";

const std::string LRU_INODE_TABLE = "LruInode";

const std::string CREATE_LRU_INODE_TABLE = "CREATE TABLE IF NOT EXISTS " +
    LRU_INODE_TABLE + " (" +
    INODE_INO + " BIGINT PRIMARY KEY, " +
    PARENT_INODE_INO + " BIGINT, " +
    CLOUD_ID + " TEXT, " +
    HASH + " BIGINT)";

const int32_t CLOUD_LRU_RDB_VERSION = 1;
const int32_t E_OK = 0;
const int32_t E_RDB = -1;

CloudLruRdbStore::CloudLruRdbStore(int32_t userId) : userId_(userId) {}

int32_t CloudLruRdbStore::RdbInit(const std::string bundleName)
{
    LOGI("Init rdb store, userId_ = %{public}d", userId_);
    std::string baseDir = "/data/service/el2/" + std::to_string(userId_) + "/hmdfs/cloudfile_manager/" + bundleName;
    int32_t errCode = E_OK;
    std::string databasePath = NativeRdb::RdbSqlUtils::GetDefaultDatabasePath(baseDir, CLOUD_LRU_DB, errCode);
    if (errCode != E_OK) {
        LOGE("Create Default Database Path is failed, errCode = %{public}d", errCode);
        return E_RDB;
    }
    NativeRdb::RdbStoreConfig config{""};
    config.SetName(CLOUD_LRU_DB);
    config.SetPath(databasePath);
    errCode = 0;
    CloudLruRdbCallBack rdbDataCallBack;
    auto rdb = NativeRdb::RdbHelper::GetRdbStore(config, CLOUD_LRU_RDB_VERSION, rdbDataCallBack, errCode);
    if (rdb == nullptr) {
        LOGE("GetRdbStore is failed,  errCode = %{public}d", errCode);
        return E_RDB;
    }
    std::lock_guard<std::mutex> lck(rdbMutex_);
    rdbMap_[bundleName] = rdb;

    return E_OK;
}

int32_t CloudLruRdbStore::LookUp(int64_t ino, std::string bundleName, CloudNodeInfo &info)
{
    auto it = rdbMap_.find(bundleName);
    if (it == rdbMap_.end()) {
        return E_RDB;
    }
    auto rdb = it->second;

    NativeRdb::AbsRdbPredicates predicates = NativeRdb::AbsRdbPredicates(LRU_INODE_TABLE);
    predicates.EqualTo(INODE_INO, std::to_string(ino));
    std::shared_ptr<NativeRdb::ResultSet> resultSet = rdb->QueryByStep(predicates, {});
    if (resultSet == nullptr) {
        LOGE("CloudLruRdbStore query failed");
        return E_RDB;
    }
    if (resultSet->GoToNextRow() != E_OK) {
        return E_RDB;
    }

    NativeRdb::RowEntity rowEntity;
    if (resultSet->GetRow(rowEntity) != E_OK) {
        LOGE("result set to file info get row failed");
        return E_RDB;
    }
    int64_t long_variable;
    rowEntity.Get(INODE_INO).GetLong(long_variable);
    info.inodeIno = static_cast<unsigned long long>(long_variable);
    rowEntity.Get(PARENT_INODE_INO).GetLong(long_variable);
    info.parentIno = static_cast<unsigned long long>(long_variable);
    rowEntity.Get(CLOUD_ID).GetString(info.cloudId);
    rowEntity.Get(HASH).GetLong(long_variable);
    info.hash = static_cast<unsigned long long>(long_variable);
    return E_OK;
}

int32_t CloudLruRdbStore::Insert(std::shared_ptr<NativeRdb::RdbStore> rdb, NativeRdb::ValuesBucket& values)
{
    int64_t rowId;
    auto ret = rdb->Insert(rowId, LRU_INODE_TABLE, values);
    if (ret != E_OK) {
        LOGE("fail to insert ino");
    }
    return ret;
}

int32_t CloudLruRdbStore::Update(int64_t ino, std::shared_ptr<NativeRdb::RdbStore> rdb, NativeRdb::ValuesBucket& values)
{
    int rowId;
    std::string whereClause = INODE_INO + "=?";
    std::vector<std::string> whereArgs = { std::to_string(ino) };
    auto ret = rdb->Update(rowId, LRU_INODE_TABLE, values, whereClause, whereArgs);
    if (ret != E_OK) {
        LOGE("fail to update ino %{public}lu ret %{public}d", ino, ret);
    }
    return ret;
}

int32_t CloudLruRdbStore::InsertOrUpdate(const std::string bundleName, const CloudNodeInfo info)
{
    std::lock_guard<std::mutex> lck(rdbMutex_);
    int32_t ret = E_OK;
    auto it = rdbMap_.find(bundleName);
    if (it == rdbMap_.end()) {
        ret = RdbInit(bundleName);
    }
    if (ret != E_OK) {
        return E_RDB;
    }
    auto rdb = rdbMap_.find(bundleName)->second;

    NativeRdb::ValuesBucket values;
    values.PutInt(INODE_INO, info.inodeIno);
    values.PutInt(PARENT_INODE_INO, info.parentIno);
    values.PutString(CLOUD_ID, info.cloudId);
    values.PutInt(HASH, info.hash);
    CloudNodeInfo infoForLookup;
    if (LookUp(info.inodeIno, bundleName, infoForLookup) != E_OK) {
        ret = Insert(rdb, values);
    } else {
        ret = Update(info.inodeIno, rdb, values);
    }

    return ret;
}

int32_t CloudLruRdbStore::Delete(int64_t ino, const std::string bundleName)
{
    std::lock_guard<std::mutex> lck(rdbMutex_);
    auto it = rdbMap_.find(bundleName);
    if (it == rdbMap_.end()) {
        return E_RDB;
    }
    auto rdb = it->second;

    NativeRdb::AbsRdbPredicates predicates = NativeRdb::AbsRdbPredicates(LRU_INODE_TABLE);
    predicates.EqualTo(INODE_INO, std::to_string(ino));
    std::shared_ptr<NativeRdb::ResultSet> resultSet = rdb->QueryByStep(predicates, {});
    if (resultSet == nullptr) {
        LOGE("CloudLruRdbStore query failed");
        return -1;
    }
    if (resultSet->GoToNextRow() != 0) {
        LOGE("CloudLruRdbStore GoToNextRow failed");
        return -1;
    }
    {
        int changedRows = -1;
        int32_t ret = rdb->Delete(changedRows, predicates);
        if (ret != 0) {
            LOGE("unlink local directory fail, ret %{public}d", ret);
            return -1;
        }
    }
    return 0;
}

int32_t CloudLruRdbCallBack::OnCreate(NativeRdb::RdbStore &store)
{
    std::vector<std::string> executeSqlStrs = {
        CREATE_LRU_INODE_TABLE,
    };

    for (const std::string& sqlStr : executeSqlStrs) {
        if (store.ExecuteSql(sqlStr) != E_OK) {
            LOGE("create table failed.");
            return -1;
        }
    }
    return 0;
}

int32_t CloudLruRdbCallBack::OnUpgrade(NativeRdb::RdbStore &store, int32_t oldVersion, int32_t newVersion)
{
    return E_OK;
}

} // namespace CloudFile
} // namespace FileManagement
} // namespace OHOS