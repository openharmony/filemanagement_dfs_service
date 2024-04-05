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

#ifndef OHOS_CLOUD_SYNC_SERVICE_CYCLE_TASK_RUNNER_H
#define OHOS_CLOUD_SYNC_SERVICE_CYCLE_TASK_RUNNER_H

#include <set>
#include <chrono>
#include <ctime>
#include "cloud_pref_impl.h"
#include "cycle_task.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
class CycleTaskRunner {
public:
    static const std::string FILE_PATH;
    static const int32_t DEFAULT_VALUE;
    static const int32_t DEFAULT_USER_ID;
    CycleTaskRunner(std::shared_ptr<DataSyncManager> dataSyncManager);
    void StartTask();

private:
    void InitTasks();
    void SetRunableBundleNames();

    int32_t userId_{0};
    std::time_t setUpTime_{0};
    std::shared_ptr<DataSyncManager> dataSyncManager_;
    std::vector<std::shared_ptr<CycleTask>> cycleTasks_ {};
};

} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
#endif // OHOS_CLOUD_SYNC_SERVICE_CYCLE_TASK_RUNNER_H