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

#ifndef OHOS_CLOUD_SYNC_SERVICE_PERIODIC_CLEAN_TASK_H
#define OHOS_CLOUD_SYNC_SERVICE_PERIODIC_CLEAN_TASK_H

#include <string>

#include "cloud_pref_impl.h"
#include "cycle_task.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
const std::string PeriodicCleanTaskName = "periodic_clean_task";

class PeriodicCleanTask : public CycleTask {
public:
    PeriodicCleanTask(std::shared_ptr<CloudFile::DataSyncManager> dataSyncManager);
    int32_t RunTaskForBundle(int32_t userId, std::string bundleName) override;

private:
    void PeriodicCleanTempDir(int32_t userId);
    void CleanTempDir(const std::string &dir, const std::string &prefix);

    int32_t RenameCacheDir(int32_t userId);
    int32_t RenameTempDir(const std::string &dir, const std::string &tempSuffix);

private:
    std::unique_ptr<CloudPrefImpl> periodicCleanConfig_;
};
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
#endif // OHOS_CLOUD_SYNC_SERVICE_PERIODIC_CLEAN_TASK_H