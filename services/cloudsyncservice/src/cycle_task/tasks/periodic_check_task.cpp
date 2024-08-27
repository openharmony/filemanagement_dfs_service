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
#include "battery_status.h"
#include "periodic_check_task.h"
#include "parameters.h"
#include "screen_status.h"
#include "system_load.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
static const std::string FILEMANAGER_KEY = "persist.kernel.bundle_name.filemanager";
PeriodicCheckTask::PeriodicCheckTask(std::shared_ptr<CloudFile::DataSyncManager> dataSyncManager)
    : CycleTask(PeriodicCheckTaskName, {"com.ohos.photos", system::GetParameter(FILEMANAGER_KEY, "")},
                THREE_DAY,
                dataSyncManager)
{
}

int32_t PeriodicCheckTask::RunTaskForBundle(int32_t userId, std::string bundleName)
{
    if (ScreenStatus::IsScreenOn() || !BatteryStatus::IsCharging()
        || !SystemLoadStatus::IsLoadStatusUnderHot()) {
        LOGI("PeriodicCheckTask::RunTaskForBundle isScreenOn or not charging");
        return E_STOP;
    }
    int32_t ret = dataSyncManager_->TriggerStartSync(bundleName, userId, false, SyncTriggerType::TASK_TRIGGER);
    if (ret != E_OK) {
        LOGE("trigger %{public}s periodic sync failed", bundleName.c_str());
    }
    return ret;
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS