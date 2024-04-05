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

#ifndef OHOS_CLOUD_SYNC_SERVICE_CYCLE_TASK_H
#define OHOS_CLOUD_SYNC_SERVICE_CYCLE_TASK_H
#include <cstdint>
#include <memory>
#include <string>
#include "dfs_error.h"
#include "data_sync_manager.h"
namespace OHOS {
namespace FileManagement {
namespace CloudSync {
class CycleTask {
public:
    enum IntervalTime {
        ONE_DAY = 24 * 60 * 60,
        TWO_DAY = ONE_DAY * 2,
        THREE_DAY = ONE_DAY * 3,
        ONE_WEEK = ONE_DAY * 7,
    };
    CycleTask(std::string taskName, std::set<std::string> bundleNames, int32_t intervalTime,
              std::shared_ptr<DataSyncManager> dataSyncManager);
    virtual ~CycleTask() = default;
    std::string GetTaskName() const;
    void RunTask(int32_t userId);
    void SetRunnableBundleNames(std::shared_ptr<std::set<std::string>> &bundleNames);
    std::shared_ptr<DataSyncManager> GetDataSyncManager() const;
    static const std::string FILE_PATH;

protected:
    int32_t userId_;
    std::string taskName_;
    std::set<std::string> bundleNames_;
    int32_t intervalTime_;
    std::shared_ptr<DataSyncManager> dataSyncManager_;

private:
    void GetLastRunTime(std::time_t &time);
    void SetLastRunTime(std::time_t time);
    bool IsEligibleToRun(std::time_t currentTime, std::string bundleName);
    virtual int32_t RunTaskForBundle(int32_t userId, std::string bundleName) = 0;

    std::unique_ptr<CloudPrefImpl> cloudPrefImpl_;
    std::shared_ptr<std::set<std::string>> runnableBundleNames_;
};
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
#endif // OHOS_CLOUD_SYNC_SERVICE_CYCLE_TASK_H