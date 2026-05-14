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

#ifndef OHOS_CLOUD_SYNC_SERVICE_DATABASE_SUPPLEMENT_TASK_H
#define OHOS_CLOUD_SYNC_SERVICE_DATABASE_SUPPLEMENT_TASK_H

#include "cycle_task.h"

#include <dirent.h>

namespace OHOS {
namespace FileManagement {
namespace CloudSync {

class DatabaseSupplementTask : public CycleTask {
public:
    DatabaseSupplementTask(std::shared_ptr<CloudFile::DataSyncManager> dataSyncManager);
    int32_t RunTaskForBundle(int32_t userId, std::string bundleName) override;

private:
    std::string GetBaseDir(int32_t userId);
    
    int32_t RunPruneTask(int32_t userId, const std::string& filemanager);
    int32_t RunChownTask(int32_t userId, const std::string& filemanager);
    
    void FindMigratedApps(int32_t userId, const std::string& baseDir,
                          const std::string& filemanager,
                          std::vector<std::string>& migratedBundles);
    
    void PruneAndCleanup(int32_t userId, const std::string& bundleName,
                          const std::string& filemanager);
    
    bool CheckNeedPrune(int32_t userId, const std::string& bundleName,
                        const std::string& filemanager);
    
    void PruneDatabase(int32_t userId, const std::string& bundleName,
                       const std::string& filemanager);
    
    void CleanupOldDatabase(int32_t userId, const std::string& bundleName,
                             const std::string& filemanager);
    
    bool CheckPruneCondition();
    bool HasAllPrunedMarker(int32_t userId, const std::string& filemanager);
    void CreateAllPrunedMarker(int32_t userId, const std::string& filemanager);
    
    void FindMigratedBundlesForChown(int32_t userId, const std::string& baseDir,
                                      const std::string& filemanager,
                                      std::vector<std::string>& migratedBundles);
    void ChownMigratedAppDirectory(int32_t userId, const std::string& bundleName);
};

} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS

#endif // OHOS_CLOUD_SYNC_SERVICE_DATABASE_SUPPLEMENT_TASK_H