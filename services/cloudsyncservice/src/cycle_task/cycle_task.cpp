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

#include "cycle_task.h"
#include "gallery_data_syncer.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
CycleTask::CycleTask(std::string taskName,
                     std::set<std::string> bundleNames,
                     int32_t intervalTime,
                     std::shared_ptr<DataSyncManager> dataSyncManager)
    : userId_(-1), taskName_(taskName), bundleNames_(bundleNames), intervalTime_(intervalTime),
    dataSyncManager_(dataSyncManager)
{
}

const std::string CycleTask::FILE_PATH = "CycleTask";

void CycleTask::SetRunnableBundleNames(std::shared_ptr<std::set<std::string>> &bundleNames)
{
    this->runnableBundleNames_ = bundleNames;
}

std::shared_ptr<DataSyncManager> CycleTask::GetDataSyncManager() const
{
    return this->dataSyncManager_;
}

std::string CycleTask::GetTaskName() const
{
    return this->taskName_;
}

void CycleTask::SetLastRunTime(std::time_t time)
{
    if (cloudPrefImpl_ == nullptr) {
        LOGE(" cloudPrefImpl_ is nullptr");
        return;
    }
    cloudPrefImpl_->SetLong("lastRunTime-" + taskName_, time);
}

void CycleTask::GetLastRunTime(std::time_t &time)
{
    if (cloudPrefImpl_ == nullptr) {
        LOGE(" cloudPrefImpl_ is nullptr");
        time = std::time(nullptr);
        return;
    }
    cloudPrefImpl_->GetLong("lastRunTime-" + taskName_, time);
}

bool CycleTask::IsEligibleToRun(std::time_t currentTime, std::string bundleName)
{
    std::time_t lastRuntime;
    GetLastRunTime(lastRuntime);
    if (lastRuntime == 0) {
        SetLastRunTime(currentTime);
        LOGE("skip first run, bundle name is %{public}s", bundleName.c_str());
        return false;
    }
    if ((bundleNames_.find(bundleName) != bundleNames_.end() || bundleNames_.size() == 0) &&
        difftime(currentTime, lastRuntime) >= intervalTime_) {
        return true;
    }
    return false;
}

void CycleTask::RunTask(int32_t userId)
{
    userId_ = userId;
    std::time_t currentTime = std::time(nullptr);
    for (const auto &bundleName : *runnableBundleNames_) {
        cloudPrefImpl_ = std::make_unique<CloudPrefImpl>(userId_, bundleName, FILE_PATH);
        if (IsEligibleToRun(currentTime, bundleName)) {
            LOGI("begin task, task name is %{public}s, bundle name is %{public}s",
                taskName_.c_str(), bundleName.c_str());
            int32_t ret = RunTaskForBundle(userId, bundleName);
            if (ret == E_OK) {
                SetLastRunTime(currentTime);
            }
            LOGI("end task, task name is %{public}s, bundle name is %{public}s, ret is %{public}d",
                taskName_.c_str(), bundleName.c_str(), ret);
        }
    }
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS