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

#include "migration_manager.h"

#include <sys/stat.h>
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <string>
#include <vector>

#include "abs_shared_result_set.h"
#include "cloud_file_utils.h"
#include "clouddisk_db_const.h"
#include "file_column.h"
#include "ffrt_inner.h"
#include "parameters.h"
#include "rdb_helper.h"
#include "rdb_open_callback.h"
#include "rdb_sql_utils.h"
#include "rdb_store_config.h"
#include "result_set.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudDisk {

using namespace std;
using namespace OHOS::NativeRdb;

namespace {
    static const uint32_t STAT_MODE_DIR = 0771;
}

class MigrationDbCallBack : public RdbOpenCallback {
public:
    int OnCreate(RdbStore &rdbStore) override { return E_OK; }
    int OnUpgrade(RdbStore &rdbStore, int oldVersion, int newVersion) override { return E_OK; }
};

static const string FILEMANAGER_KEY = "persist.kernel.bundle_name.filemanager";
static const uint32_t STAT_MODE_REG = 0660;

MigrationManager& MigrationManager::GetInstance()
{
    static MigrationManager instance_;
    return instance_;
}

string MigrationManager::GetAppMarkerPath(int32_t userId, const string& bundleName, const string& type)
{
    return GetBaseDir(userId) + bundleName + "/" + type;
}

string MigrationManager::GetBaseDir(int32_t userId)
{
    return "/data/service/el2/" + to_string(userId) + "/hmdfs/cloudfile_manager/";
}

string MigrationManager::GetOldDbDir(int32_t userId)
{
    string filemanager = system::GetParameter(FILEMANAGER_KEY, "");
    return GetBaseDir(userId) + filemanager;
}

string MigrationManager::GetOldDbPath(int32_t userId)
{
    return GetOldDbDir(userId) + "/rdb/clouddisk.db";
}

string MigrationManager::GetNewDbDir(int32_t userId, const string& bundleName)
{
    return GetBaseDir(userId) + bundleName;
}

string MigrationManager::GetNewDbPath(int32_t userId, const string& bundleName)
{
    return GetNewDbDir(userId, bundleName) + "/rdb/clouddisk.db";
}

void MigrationManager::StartAsyncMigration(int32_t userId)
{
    string filemanager = system::GetParameter(FILEMANAGER_KEY, "");
    string checkedMarker = GetAppMarkerPath(userId, filemanager, ".migration_checked");
    if (HasMarkerFile(checkedMarker)) {
        LOGI("migration checked, userId=%{public}d", userId);
        return;
    }
    
    string oldDbPath = GetOldDbPath(userId);
    if (access(oldDbPath.c_str(), F_OK) != 0) {
        LOGI("old db not found, userId=%{public}d", userId);
        CreateMarkerFile(checkedMarker);
        return;
    }
    
    vector<string> bundleNames;
    if (!QueryBundleNamesFromOldDb(userId, bundleNames)) {
        LOGE("failed to query bundles, userId=%{public}d", userId);
        return;
    }
    
    vector<string> bundlesToMigrate;
    for (const auto& bundle : bundleNames) {
        if (bundle == filemanager) {
            continue;
        }
        string migratedMarker = GetAppMarkerPath(userId, bundle, ".migrated");
        string migratingMarker = GetAppMarkerPath(userId, bundle, ".migrating");
        if (HasMarkerFile(migratedMarker) || HasMarkerFile(migratingMarker)) {
            continue;
        }
        bundlesToMigrate.push_back(bundle);
    }
    
    if (bundlesToMigrate.empty()) {
        LOGI("no bundles to migrate, userId=%{public}d", userId);
        CreateMarkerFile(checkedMarker);
        return;
    }
    
    LOGI("found %{public}zu bundles, userId=%{public}d", bundlesToMigrate.size(), userId);
    
    CreateAppMigratingMarkers(userId, bundlesToMigrate);
    
    ffrt::submit([this, userId]() {
        DoMigration(userId);
    });
}

void MigrationManager::WaitForAppMigration(int32_t userId, const string& bundleName)
{
    string migratingMarker = GetAppMarkerPath(userId, bundleName, ".migrating");
    if (!HasMarkerFile(migratingMarker)) {
        return;
    }
    
    string appKey = to_string(userId) + "_" + bundleName;
    
    unique_lock<mutex> lock(mutex_);
    
    if (appStates_.find(appKey) == appStates_.end()) {
        LOGW("stale marker, bundle=%{public}s", bundleName.c_str());
        lock.unlock();
        DeleteMarkerFile(migratingMarker);
        return;
    }
    
    auto& state = appStates_[appKey];
    LOGI("waiting migration, bundle=%{public}s", bundleName.c_str());
    bool completed = state->cv->wait_for(lock,
        chrono::seconds(TIMEOUT_SECONDS),
        [&state]() {
            return state->completed.load();
        });
    if (!completed) {
        LOGW("migration timeout, bundle=%{public}s", bundleName.c_str());
        DeleteMarkerFile(migratingMarker);
    }
}

void MigrationManager::DoMigration(int32_t userId)
{
    LOGI("migration start, userId=%{public}d", userId);
    
    vector<string> bundlesToMigrate;
    {
        string filemanager = system::GetParameter(FILEMANAGER_KEY, "");
        vector<string> bundleNames;
        QueryBundleNamesFromOldDb(userId, bundleNames);
        
        for (const auto& bundle : bundleNames) {
            if (bundle == filemanager) {
                continue;
            }
            string migratingMarker = GetAppMarkerPath(userId, bundle, ".migrating");
            if (HasMarkerFile(migratingMarker)) {
                bundlesToMigrate.push_back(bundle);
            }
        }
    }
    
    if (bundlesToMigrate.empty()) {
        LOGI("no migrating bundles, userId=%{public}d", userId);
        return;
    }
    
    ParallelCopyDatabase(userId, bundlesToMigrate);
    
    LOGI("migration done, userId=%{public}d", userId);
}

bool MigrationManager::QueryBundleNamesFromOldDb(int32_t userId, vector<string>& bundleNames)
{
    string oldDbDir = GetOldDbDir(userId);
    int errCode = 0;
    string databasePath = RdbSqlUtils::GetDefaultDatabasePath(oldDbDir, CLOUD_DISK_DATABASE_NAME, errCode);
    if (errCode != E_OK) {
        LOGE("failed to get old db path, err=%{public}d", errCode);
        return false;
    }
    
    RdbStoreConfig config(databasePath);
    config.SetName(CLOUD_DISK_DATABASE_NAME);
    
    MigrationDbCallBack callBack;
    auto rdbStore = RdbHelper::GetRdbStore(config, CLOUD_DISK_RDB_VERSION, callBack, errCode);
    if (rdbStore == nullptr) {
        LOGE("failed to open old db, err=%{public}d", errCode);
        return false;
    }
    
    string sql = "SELECT DISTINCT " + FileColumn::ROOT_DIRECTORY + " FROM " +
                 FileColumn::FILES_TABLE +
                 " WHERE " + FileColumn::ROOT_DIRECTORY + " IS NOT NULL AND " +
                 FileColumn::ROOT_DIRECTORY + " != ''";
    
    auto resultSet = rdbStore->QuerySql(sql);
    if (resultSet == nullptr) {
        LOGE("failed to query bundles");
        return false;
    }
    
    while (resultSet->GoToNextRow() == E_OK) {
        string value;
        int ret = resultSet->GetString(0, value);
        if (ret == E_OK && !value.empty()) {
            bundleNames.push_back(value);
        }
    }
    
    resultSet->Close();
    return true;
}

void MigrationManager::CreateAppMigratingMarkers(int32_t userId, const vector<string>& bundleNames)
{
    for (const auto& bundle : bundleNames) {
        string newDbDir = GetNewDbDir(userId, bundle);
        if (access(newDbDir.c_str(), F_OK) != 0) {
            if (mkdir(newDbDir.c_str(), STAT_MODE_DIR) != 0 && errno != EEXIST) {
                LOGE("failed to create dir, err=%{public}d", errno);
                continue;
            }
        }
        
        string migratingMarker = GetAppMarkerPath(userId, bundle, ".migrating");
        CreateMarkerFile(migratingMarker);
        
        string appKey = to_string(userId) + "_" + bundle;
        {
            lock_guard<mutex> lock(mutex_);
            appStates_[appKey] = make_unique<AppMigrationState>();
        }
        
        LOGI("created marker, bundle=%{public}s", bundle.c_str());
    }
}

bool MigrationManager::ParallelCopyDatabase(int32_t userId, const vector<string>& bundleNames)
{
    ffrt::queue copyQueue(ffrt::queue_concurrent, "db_copy_queue",
                          ffrt::queue_attr().max_concurrency(COPY_THREAD_COUNT));
    
    vector<ffrt::task_handle> handles;
    
    for (const auto& bundle : bundleNames) {
        auto handle = copyQueue.submit_h([this, userId, bundle]() {
            bool success = CopyDatabase(userId, bundle);
            if (success) {
                OnAppMigrationSuccess(userId, bundle);
            } else {
                OnAppMigrationFailed(userId, bundle);
            }
        });
        handles.push_back(handle);
    }
    
    for (auto& handle : handles) {
        copyQueue.wait(handle);
    }
    
    LOGI("copy completed, userId=%{public}d", userId);
    return true;
}

bool MigrationManager::CopyDatabase(int32_t userId, const string& bundleName)
{
    string oldDbPath = GetOldDbPath(userId);
    string newDbDir = GetNewDbDir(userId, bundleName);
    if (access(newDbDir.c_str(), F_OK) != 0) {
        if (mkdir(newDbDir.c_str(), STAT_MODE_DIR) != 0 && errno != EEXIST) {
            LOGE("failed to create dir, err=%{public}d", errno);
            return false;
        }
    }
    
    int errCode = 0;
    string newDbPath = RdbSqlUtils::GetDefaultDatabasePath(newDbDir, CLOUD_DISK_DATABASE_NAME, errCode);
    if (errCode != E_OK) {
        LOGE("failed to get new db path, err=%{public}d", errCode);
        return false;
    }
    
    error_code ec;
    bool ret = filesystem::copy_file(oldDbPath, newDbPath,
                                     filesystem::copy_options::overwrite_existing, ec);
    if (!ret || ec.value() != 0) {
        LOGE("failed to copy db, err=%{public}d", ec.value());
        return false;
    }
    
    CloudFileUtils::ChangeUid(userId, bundleName, STAT_MODE_REG, newDbPath);
    
    LOGI("copy done, bundle=%{public}s", bundleName.c_str());
    return true;
}

void MigrationManager::OnAppMigrationSuccess(int32_t userId, const string& bundleName)
{
    string migratingMarker = GetAppMarkerPath(userId, bundleName, ".migrating");
    string migratedMarker = GetAppMarkerPath(userId, bundleName, ".migrated");
    
    string appKey = to_string(userId) + "_" + bundleName;
    
    if (!HasMarkerFile(migratingMarker)) {
        LOGW("migrating not found (timeout), skip migrated, bundle=%{public}s", bundleName.c_str());
        lock_guard<mutex> lock(mutex_);
        appStates_.erase(appKey);
        return;
    }
    
    DeleteMarkerFile(migratingMarker);
    CreateMarkerFile(migratedMarker);
    
    {
        lock_guard<mutex> lock(mutex_);
        if (appStates_.find(appKey) != appStates_.end()) {
            auto& state = appStates_[appKey];
            state->completed = true;
            state->cv->notify_all();
        }
    }
    
    LOGI("migration success, bundle=%{public}s", bundleName.c_str());
}

void MigrationManager::OnAppMigrationFailed(int32_t userId, const string& bundleName)
{
    string migratingMarker = GetAppMarkerPath(userId, bundleName, ".migrating");
    DeleteMarkerFile(migratingMarker);
    
    string appKey = to_string(userId) + "_" + bundleName;
    {
        lock_guard<mutex> lock(mutex_);
        if (appStates_.find(appKey) != appStates_.end()) {
            auto& state = appStates_[appKey];
            state->completed = true;
            state->cv->notify_all();
        }
    }
    
    LOGW("migration failed, bundle=%{public}s", bundleName.c_str());
}

void MigrationManager::CreateMarkerFile(const string& path)
{
    ofstream file(path);
    file.close();
}

void MigrationManager::DeleteMarkerFile(const string& path)
{
    if (remove(path.c_str()) != 0 && errno != ENOENT) {
        LOGW("failed to remove marker, err=%{public}d", errno);
    }
}

bool MigrationManager::HasMarkerFile(const string& path)
{
    return access(path.c_str(), F_OK) == 0;
}

} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS