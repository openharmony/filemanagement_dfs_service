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
#include "database_manager.h"
#include "data_syncer_rdb_store.h"
#include "dfs_error.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudDisk {
using namespace std;
DatabaseManager &DatabaseManager::GetInstance()
{
    static DatabaseManager instance_;
    return instance_;
}

shared_ptr<CloudDiskRdbStore> DatabaseManager::GetRdbStore(const string &bundleName, int32_t userId)
{
    string key = to_string(userId) + bundleName;
    {
        std::unique_lock<std::shared_mutex> wLock(nonCloudLock_);
        nonCloudBundles_.erase(key);
    }
    std::unique_lock<std::shared_mutex> wLock(mapLock_, std::defer_lock);

    wLock.lock();
    if (rdbMap_.find(key) == rdbMap_.end()) {
        rdbMap_[key] = make_shared<CloudDiskRdbStore>(bundleName, userId);
    }
    wLock.unlock();

    return rdbMap_[key];
}

bool DatabaseManager::IsBundleCloudSyncEnabled(int32_t userId, const std::string &bundleName)
{
    std::string key = std::to_string(userId) + bundleName;

    {
        std::shared_lock<std::shared_mutex> rLock(nonCloudLock_);
        if (nonCloudBundles_.count(key) != 0) {
            LOGI("bundle cached as non-cloud: %{public}s, userId: %{public}d", bundleName.c_str(), userId);
            return false;
        }
    }

    {
        std::shared_lock<std::shared_mutex> rLock(mapLock_);
        if (rdbMap_.find(key) != rdbMap_.end()) {
            return true;
        }
    }

    std::shared_ptr<NativeRdb::ResultSet> resultSet;
    int32_t ret = CloudSync::DataSyncerRdbStore::GetInstance()
                      .QueryCloudSync(userId, bundleName, resultSet);
    if (ret != E_OK || resultSet == nullptr) {
        LOGE("QueryCloudSync failed, userId: %{public}d, bundle: %{public}s, ret: %{public}d, skip cache",
            userId, bundleName.c_str(), ret);
        return false;
    }
    int32_t rowCount = 0;
    ret = resultSet->GetRowCount(rowCount);
    if (ret != E_OK || rowCount < 0) {
        LOGE("GetRowCount failed, userId: %{public}d, bundle: %{public}s, ret: %{public}d, rowCount: %{public}d",
            userId, bundleName.c_str(), ret, rowCount);
        return false;
    }
    if (rowCount >= 1) {
        return true;
    }

    {
        std::unique_lock<std::shared_mutex> wLock(nonCloudLock_);
        nonCloudBundles_.insert(key);
    }
    LOGI("bundle cached as non-cloud: %{public}s, userId: %{public}d", bundleName.c_str(), userId);
    return false;
}

void DatabaseManager::ClearRdbStore()
{
    std::unique_lock<std::shared_mutex> wLock(mapLock_);
    rdbMap_.clear();
    std::unique_lock<std::shared_mutex> nWLock(nonCloudLock_);
    nonCloudBundles_.clear();
}
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS
