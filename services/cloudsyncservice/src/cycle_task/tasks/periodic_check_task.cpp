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
#include "tasks/periodic_check_task.h"
#include <memory>
#include <sstream>
#include "cycle_task.h"
#include "dfs_error.h"
#include "data_sync_manager.h"
#include "cycle_task_runner.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
PeriodicCheckTask::PeriodicCheckTask(std::shared_ptr<DataSyncManager> dataSyncManager)
    : CycleTask("periodic_check_task", "com.ohos.photos", CycleTaskRunner::ONE_WEEK, dataSyncManager) {}

int32_t PeriodicCheckTask::RunTask(int32_t userId)
{
    int32_t ret = E_OK;
    std::string bundleNames[] = {"com.ohos.photos"};
    for (auto bundleName : bundleNames) {
        int32_t ret = dataSyncManager_->TriggerStartSync(bundleName, userId, false, SyncTriggerType::TASK_TRIGGER);
        if (ret != E_OK) {
            LOGE("trigger %{public}s periodic sync failed", bundleName.c_str());
        }
    }
    return ret;
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS