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

#ifndef OHOS_CLOUDDISK_DATABASE_MIGRATION_MANAGER_H
#define OHOS_CLOUDDISK_DATABASE_MIGRATION_MANAGER_H

#include <atomic>
#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace OHOS {
namespace FileManagement {
namespace CloudDisk {

struct AppMigrationState {
    std::atomic<bool> completed{false};
    std::unique_ptr<std::condition_variable> cv{std::make_unique<std::condition_variable>()};
};

class MigrationManager {
public:
    static MigrationManager& GetInstance();

    void StartAsyncMigration(int32_t userId);
    void WaitForAppMigration(int32_t userId, const std::string& bundleName);
    
    MigrationManager(const MigrationManager&) = delete;
    MigrationManager& operator=(const MigrationManager&) = delete;

private:
    MigrationManager() = default;
    ~MigrationManager() = default;

    void DoMigration(int32_t userId);
    
    std::string GetAppMarkerPath(int32_t userId, const std::string& bundleName, const std::string& type);
    std::string GetBaseDir(int32_t userId);
    std::string GetOldDbDir(int32_t userId);
    std::string GetOldDbPath(int32_t userId);
    std::string GetNewDbDir(int32_t userId, const std::string& bundleName);
    std::string GetNewDbPath(int32_t userId, const std::string& bundleName);
    
    bool QueryBundleNamesFromOldDb(int32_t userId, std::vector<std::string>& bundleNames);
    
    void CreateAppMigratingMarkers(int32_t userId, const std::vector<std::string>& bundleNames);
    
    bool ParallelCopyDatabase(int32_t userId, const std::vector<std::string>& bundleNames);
    bool CopyDatabase(int32_t userId, const std::string& bundleName);
    
    void OnAppMigrationSuccess(int32_t userId, const std::string& bundleName);
    void OnAppMigrationFailed(int32_t userId, const std::string& bundleName);
    
    void CreateMarkerFile(const std::string& path);
    void DeleteMarkerFile(const std::string& path);
    bool HasMarkerFile(const std::string& path);
    
    std::mutex mutex_;
    std::map<std::string, std::unique_ptr<AppMigrationState>> appStates_;
    
    static constexpr int TIMEOUT_SECONDS = 10;
    static constexpr int COPY_THREAD_COUNT = 4;
};

} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS

#endif // OHOS_CLOUDDISK_DATABASE_MIGRATION_MANAGER_H