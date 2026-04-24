/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "database_supplement_task.h"

#include <sys/stat.h>
#include <unistd.h>

#include <filesystem>
#include <fstream>
#include <vector>

#include "abs_shared_result_set.h"
#include "battery_status.h"
#include "clouddisk_db_const.h"
#include "clouddisk_rdb_transaction.h"
#include "clouddisk_rdbstore.h"
#include "cloud_file_utils.h"
#include "dfs_error.h"
#include "file_column.h"
#include "parameters.h"
#include "rdb_helper.h"
#include "rdb_open_callback.h"
#include "rdb_sql_utils.h"
#include "rdb_store_config.h"
#include "result_set.h"
#include "screen_status.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {

using namespace std;
using namespace OHOS::NativeRdb;
using namespace CloudDisk;

class SupplementDbCallBack : public RdbOpenCallback {
public:
    int OnCreate(RdbStore &rdbStore) override { return E_OK; }
    int OnUpgrade(RdbStore &rdbStore, int oldVersion, int newVersion) override { return E_OK; }
};

static const string FILEMANAGER_KEY = "persist.kernel.bundle_name.filemanager";
static const uint32_t STAT_MODE_REG = 0660;
static const uint32_t STAT_MODE_DIR = 0771;

DatabaseSupplementTask::DatabaseSupplementTask(shared_ptr<CloudFile::DataSyncManager> dataSyncManager)
    : CycleTask("database_supplement_task", {system::GetParameter(FILEMANAGER_KEY, "")}, ONE_DAY, dataSyncManager)
{
    runTaskForSwitchOff_ = true;
}

string DatabaseSupplementTask::GetBaseDir(int32_t userId)
{
    return "/data/service/el2/" + to_string(userId) + "/hmdfs/cloudfile_manager/";
}

int32_t DatabaseSupplementTask::RunTaskForBundle(int32_t userId, string bundleName)
{
    LOGI("task start, userId=%{public}d", userId);
    
    if (!CheckPruneCondition()) {
        LOGI("condition not satisfied, stop");
        return E_STOP;
    }
    
    string filemanager = system::GetParameter(FILEMANAGER_KEY, "");
    
    int32_t ret = RunPruneTask(userId, filemanager);
    if (ret == E_STOP) {
        return E_STOP;
    }
    
    ret = RunChownTask(userId, filemanager);
    if (ret == E_STOP) {
        return E_STOP;
    }
    
    LOGI("task completed, userId=%{public}d", userId);
    return E_OK;
}

int32_t DatabaseSupplementTask::RunPruneTask(int32_t userId, const string& filemanager)
{
    if (HasAllPrunedMarker(userId, filemanager)) {
        return E_OK;
    }
    
    string baseDir = GetBaseDir(userId);
    vector<string> migratedBundles;
    FindMigratedApps(userId, baseDir, filemanager, migratedBundles);
    
    if (migratedBundles.empty()) {
        LOGI("no migrated apps, userId=%{public}d", userId);
        CreateAllPrunedMarker(userId, filemanager);
        return E_OK;
    }
    
    for (const auto& bundle : migratedBundles) {
        if (!CheckPruneCondition()) {
            LOGI("condition changed, stop prune");
            return E_STOP;
        }
        PruneAndCleanup(userId, bundle, filemanager);
    }
    
    vector<string> remaining;
    FindMigratedApps(userId, baseDir, filemanager, remaining);
    if (remaining.empty()) {
        CreateAllPrunedMarker(userId, filemanager);
    }
    
    return E_OK;
}

int32_t DatabaseSupplementTask::RunChownTask(int32_t userId, const string& filemanager)
{
    string baseDir = GetBaseDir(userId);
    vector<string> chownBundles;
    FindMigratedBundlesForChown(userId, baseDir, filemanager, chownBundles);
    
    for (const auto& bundle : chownBundles) {
        if (!CheckPruneCondition()) {
            LOGI("condition changed, stop chown");
            return E_STOP;
        }
        ChownMigratedAppDirectory(userId, bundle);
    }
    
    return E_OK;
}

void DatabaseSupplementTask::FindMigratedApps(int32_t userId, const string& baseDir,
    const string& filemanager, vector<string>& migratedBundles)
{
    string dirPath = baseDir;
    DIR* dir = opendir(dirPath.c_str());
    if (dir == nullptr) {
        LOGE("failed to open dir");
        return;
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_type != DT_DIR) {
            continue;
        }
        
        string bundleName = entry->d_name;
        if (bundleName == "." || bundleName == ".." || bundleName == filemanager) {
            continue;
        }
        
        string migratedMarker = baseDir + bundleName + "/.migrated";
        string prunedMarker = baseDir + bundleName + "/.pruned";
        
        if (access(migratedMarker.c_str(), F_OK) == 0 && access(prunedMarker.c_str(), F_OK) != 0) {
            migratedBundles.push_back(bundleName);
            LOGI("found migrated app, bundle=%{public}s", bundleName.c_str());
        }
    }
    
    closedir(dir);
}

void DatabaseSupplementTask::PruneAndCleanup(int32_t userId, const string& bundleName,
    const string& filemanager)
{
    string newDbDir = GetBaseDir(userId) + bundleName;
    int errCode = 0;
    string databasePath = RdbSqlUtils::GetDefaultDatabasePath(newDbDir, CLOUD_DISK_DATABASE_NAME, errCode);
    if (access(databasePath.c_str(), F_OK) != 0) {
        LOGI("db not found, bundle=%{public}s, skip", bundleName.c_str());
        return;
    }
    
    bool needPrune = CheckNeedPrune(userId, bundleName, filemanager);
    if (!needPrune) {
        LOGI("no need prune, bundle=%{public}s", bundleName.c_str());
        string prunedMarker = newDbDir + "/.pruned";
        ofstream file(prunedMarker);
        file.close();
        return;
    }
    
    PruneDatabase(userId, bundleName, filemanager);
    
    string prunedMarker = newDbDir + "/.pruned";
    ofstream file(prunedMarker);
    file.close();
    
    LOGI("pruned marker created, bundle=%{public}s", bundleName.c_str());
    
    CleanupOldDatabase(userId, bundleName, filemanager);
}

bool DatabaseSupplementTask::CheckNeedPrune(int32_t userId, const string& bundleName,
    const string& filemanager)
{
    string newDbDir = GetBaseDir(userId) + bundleName;
    int errCode = 0;
    string databasePath = RdbSqlUtils::GetDefaultDatabasePath(newDbDir, CLOUD_DISK_DATABASE_NAME, errCode);
    if (errCode != E_OK) {
        LOGE("failed to get db path for check, err=%{public}d", errCode);
        return false;
    }
    
    RdbStoreConfig config(databasePath);
    config.SetName(CLOUD_DISK_DATABASE_NAME);
    
    SupplementDbCallBack callBack;
    auto rdbStore = RdbHelper::GetRdbStore(config, CLOUD_DISK_RDB_VERSION, callBack, errCode);
    if (rdbStore == nullptr) {
        LOGE("failed to open db for check, err=%{public}d", errCode);
        return false;
    }
    
    string sql = "SELECT COUNT(*) FROM " + FileColumn::FILES_TABLE +
                 " WHERE " + FileColumn::ROOT_DIRECTORY + " != '" + bundleName + "'";
    
    auto resultSet = rdbStore->QuerySql(sql);
    if (resultSet == nullptr) {
        return false;
    }
    
    int count = 0;
    if (resultSet->GoToNextRow() == E_OK) {
        resultSet->GetInt(0, count);
    }
    resultSet->Close();
    
    LOGI("check prune, bundle=%{public}s, count=%{public}d", bundleName.c_str(), count);
    return count > 0;
}

void DatabaseSupplementTask::PruneDatabase(int32_t userId, const string& bundleName,
    const string& filemanager)
{
    string newDbDir = GetBaseDir(userId) + bundleName;
    int errCode = 0;
    string databasePath = RdbSqlUtils::GetDefaultDatabasePath(newDbDir, CLOUD_DISK_DATABASE_NAME, errCode);
    if (errCode != E_OK) {
        LOGE("failed to get db path for prune, err=%{public}d", errCode);
        return;
    }
    
    RdbStoreConfig config(databasePath);
    config.SetName(CLOUD_DISK_DATABASE_NAME);
    
    SupplementDbCallBack callBack;
    auto rdbStore = RdbHelper::GetRdbStore(config, CLOUD_DISK_RDB_VERSION, callBack, errCode);
    if (rdbStore == nullptr) {
        LOGE("failed to open db for prune, err=%{public}d", errCode);
        return;
    }
    
    TransactionOperations rdbTransaction(rdbStore);
    auto [ret, transaction] = rdbTransaction.Start();
    if (ret != E_OK) {
        LOGE("failed to begin tx for prune, err=%{public}d", ret);
        return;
    }
    
    NativeRdb::AbsRdbPredicates predicates(FileColumn::FILES_TABLE);
    predicates.NotEqualTo(FileColumn::ROOT_DIRECTORY, bundleName);
    
    int32_t changedRows = 0;
    std::tie(ret, changedRows) = transaction->Delete(predicates);
    if (ret != E_OK) {
        LOGW("failed to prune data, bundle=%{public}s, err=%{public}d", bundleName.c_str(), ret);
        return;
    }
    
    rdbTransaction.Finish();
    LOGI("prune completed, bundle=%{public}s, rows=%{public}d", bundleName.c_str(), changedRows);
}

void DatabaseSupplementTask::CleanupOldDatabase(int32_t userId, const string& bundleName,
    const string& filemanager)
{
    string oldDbDir = GetBaseDir(userId) + filemanager;
    int errCode = 0;
    string databasePath = RdbSqlUtils::GetDefaultDatabasePath(oldDbDir, CLOUD_DISK_DATABASE_NAME, errCode);
    if (errCode != E_OK) {
        LOGE("failed to get old db path, err=%{public}d", errCode);
        return;
    }
    
    if (access(databasePath.c_str(), F_OK) != 0) {
        LOGI("old db not found, skip cleanup");
        return;
    }
    
    RdbStoreConfig config(databasePath);
    config.SetName(CLOUD_DISK_DATABASE_NAME);
    
    SupplementDbCallBack callBack;
    auto rdbStore = RdbHelper::GetRdbStore(config, CLOUD_DISK_RDB_VERSION, callBack, errCode);
    if (rdbStore == nullptr) {
        LOGE("failed to open old db, err=%{public}d", errCode);
        return;
    }
    
    TransactionOperations rdbTransaction(rdbStore);
    auto [ret, transaction] = rdbTransaction.Start();
    if (ret != E_OK) {
        LOGE("failed to begin tx for cleanup, err=%{public}d", ret);
        return;
    }
    
    NativeRdb::AbsRdbPredicates predicates(FileColumn::FILES_TABLE);
    predicates.EqualTo(FileColumn::ROOT_DIRECTORY, bundleName);
    
    int32_t changedRows = 0;
    std::tie(ret, changedRows) = transaction->Delete(predicates);
    if (ret != E_OK) {
        LOGW("failed to delete data, bundle=%{public}s, err=%{public}d", bundleName.c_str(), ret);
        return;
    }
    
    rdbTransaction.Finish();
    LOGI("cleanup completed, bundle=%{public}s, rows=%{public}d", bundleName.c_str(), changedRows);
}

bool DatabaseSupplementTask::CheckPruneCondition()
{
    return !ScreenStatus::IsScreenOn() && BatteryStatus::IsCharging();
}

bool DatabaseSupplementTask::HasAllPrunedMarker(int32_t userId, const string& filemanager)
{
    string markerPath = GetBaseDir(userId) + filemanager + "/.all_pruned";
    return access(markerPath.c_str(), F_OK) == 0;
}

void DatabaseSupplementTask::CreateAllPrunedMarker(int32_t userId, const string& filemanager)
{
    string markerPath = GetBaseDir(userId) + filemanager + "/.all_pruned";
    ofstream file(markerPath);
    file.close();
    LOGI("created all_pruned marker, userId=%{public}d", userId);
}

void DatabaseSupplementTask::FindMigratedBundlesForChown(int32_t userId, const string& baseDir,
    const string& filemanager, vector<string>& migratedBundles)
{
    string dirPath = baseDir;
    DIR* dir = opendir(dirPath.c_str());
    if (dir == nullptr) {
        LOGE("failed to open dir");
        return;
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_type != DT_DIR) {
            continue;
        }
        
        string bundleName = entry->d_name;
        if (bundleName == "." || bundleName == "..") {
            continue;
        }
        
        if (bundleName == filemanager) {
            migratedBundles.push_back(bundleName);
            LOGI("found filemanager for chown, bundle=%{public}s", bundleName.c_str());
            continue;
        }
        
        string migratedMarker = baseDir + bundleName + "/.migrated";
        if (access(migratedMarker.c_str(), F_OK) == 0) {
            migratedBundles.push_back(bundleName);
            LOGI("found migrated for chown, bundle=%{public}s", bundleName.c_str());
        }
    }
    
    closedir(dir);
}

void DatabaseSupplementTask::ChownMigratedAppDirectory(int32_t userId, const string& bundleName)
{
    string appDir = GetBaseDir(userId) + bundleName;
    if (access(appDir.c_str(), F_OK) != 0) {
        LOGI("app dir not found, bundle=%{public}s, skip", bundleName.c_str());
        return;
    }
    
    error_code ec;
    for (const auto& entry : filesystem::recursive_directory_iterator(appDir, ec)) {
        if (ec) {
            LOGW("iterator error, bundle=%{public}s", bundleName.c_str());
            continue;
        }
        
        string path = entry.path().string();
        struct stat st{};
        if (stat(path.c_str(), &st) != 0) {
            LOGW("stat failed, errno=%{public}d", errno);
            continue;
        }
        
        uint32_t mode = S_ISDIR(st.st_mode) ? STAT_MODE_DIR : STAT_MODE_REG;
        CloudFileUtils::ChangeUid(userId, bundleName, mode, path);
    }
    
    LOGI("chown completed, bundle=%{public}s", bundleName.c_str());
}

} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS