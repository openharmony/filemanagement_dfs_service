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

#ifndef OHOS_CLOUD_SYNC_SERVICE_PERIODIC_CHOWN_TASK_H
#define OHOS_CLOUD_SYNC_SERVICE_PERIODIC_CHOWN_TASK_H

#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>

#include "cloud_pref_impl.h"
#include "cycle_task.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {

class PeriodicChownTask : public CycleTask {
public:
    PeriodicChownTask(std::shared_ptr<CloudFile::DataSyncManager> dataSyncManager);
    PeriodicChownTask(const PeriodicChownTask&) = delete;
    PeriodicChownTask& operator=(const PeriodicChownTask&) = delete;
    int32_t RunTaskForBundle(int32_t userId, std::string bundleName) override;

private:
    int32_t ExecuteChownTask(int32_t userId);
    int32_t ChownAllDirectories(int32_t userId);
    int32_t ProcessSingleDirectory(const std::string& path, int32_t index);
    int32_t ChownFiles(const std::string& path, int32_t index);
    std::string GetTargetDirectory(int32_t userId, int32_t index);
    int32_t ValidateAndChownDirectory(const std::string& path);
    void ChangeUidToMedia(const std::string& path, mode_t mode, const struct stat& st);
    bool CheckChownCondition();
    bool CheckAllDirectoriesDone();
    void InitializeDirectoryTemplates();
    std::string GenerateDoneKey(const std::string& dirPath);
    std::string GenerateTimestampKey(const std::string& dirPath);
    std::string GenerateKeyWithSuffix(const std::string& dirPath, const std::string& suffix);
    bool ShouldChownFile(const struct stat& st, int64_t dirTimestamp);
    void ReportChownFilesResult(const std::string& path, int32_t result);

private:
    std::unique_ptr<CloudPrefImpl> periodicChownConfig_;
    int32_t fileCount_ = 0;
    int64_t totalSize_ = 0;
    static constexpr int32_t BATCH_SIZE = 100;
    std::vector<std::string> directoryTemplates_;
    std::vector<std::string> directoryKeys_;
};
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
#endif // OHOS_CLOUD_SYNC_SERVICE_PERIODIC_CHOWN_TASK_H
