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

#include "cycle_task_runner.h"
#include "cloud_pref_impl.h"
#include "cycle_task.h"
#include "dfs_error.h"
#include "utils_log.h"
#include <cstdint>
#include <iomanip>
#include <sstream>

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
const std::string CycleTaskRunner::FILE_PATH = CLOUDFILE_DIR + "/cycletask";
const int32_t CycleTaskRunner::DEFAULT_VALUE = 0;
const int32_t CycleTaskRunner::DEFAULT_USER_ID = 100;

CycleTaskRunner::CycleTaskRunner()
{
    cloudPrefImpl_ = std::make_unique<CloudPrefImpl>(FILE_PATH);
    cloudPrefImpl_->GetInt("userId", userId_);
    setUpTime_ = std::time(nullptr);
    InitTasks();
}

void CycleTaskRunner::StartTask()
{
    if (userId_ == DEFAULT_VALUE) {
        LOGD("defaukt userId skip tasks");
        cloudPrefImpl_->SetInt("userId", DEFAULT_USER_ID);
        return;
    }
    for (const auto &task_data : cycleTasks_) {
        time_t lastRunTime = DEFAULT_VALUE;
        GetLastRunTime(task_data->GetTaskName(), lastRunTime);
        if (difftime(setUpTime_, lastRunTime) > task_data->GetIntervalTime()) {
            LOGD("run task task name is %{public}s", task_data->GetTaskName().c_str());
            int32_t ret = task_data->RunTask(userId_);
            if (ret == E_OK) {
                LOGD("task run success, taskName is %s, ret = %d", task_data->GetTaskName().c_str(), ret);
                SetLastRunTime(task_data->GetTaskName(), setUpTime_);
            } else {
                LOGD("task run fail, taskName is %s, ret = %d", task_data->GetTaskName().c_str(), ret);
            }
        }
    }
}

void CycleTaskRunner::InitTasks()
{
    //push tasks here
}

void CycleTaskRunner::GetLastRunTime(std::string taskName, std::time_t &time)
{
    cloudPrefImpl_->GetLong("lastRunTime-" + taskName, time);
}

void CycleTaskRunner::SetLastRunTime(std::string taskName, std::time_t time)
{
    cloudPrefImpl_->SetLong("lastRunTime-" + taskName, time);
}

} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS