/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#include "cloud_status.h"
#include "cycle_task.h"
#include "data_syncer_rdb_col.h"
#include "data_syncer_rdb_store.h"
#include "os_account_manager.h"
#include "parameter.h"
#include "result_set.h"
#include "tasks/database_backup_task.h"
#include "tasks/optimize_cache_task.h"
#include "tasks/optimize_storage_task.h"
#include "tasks/periodic_check_task.h"
#include "tasks/report_statistics_task.h"
#include "tasks/save_subscription_task.h"
#include "utils_log.h"
#include <memory>

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
using namespace std;

CycleTaskRunner::CycleTaskRunner(std::shared_ptr<CloudFile::DataSyncManager> dataSyncManager)
{
    dataSyncManager_ = dataSyncManager;
    vector<int32_t> activeUsers;
    if (AccountSA::OsAccountManager::QueryActiveOsAccountIds(activeUsers) != E_OK || activeUsers.empty()) {
        LOGE("query active user failed");
        return;
    }
    userId_ = activeUsers.front();
    setUpTime_ = std::time(nullptr);
    if (dataSyncManager_ == nullptr) {
        LOGI("dataSyncManager is nullptr");
        return;
    }
    InitTasks();
    SetRunableBundleNames();
}

void CycleTaskRunner::StartTask()
{
#ifdef EMULATOR
    return;
#endif

    constexpr int32_t MOVE_FILE_TIME_SERVICE = 5;
    int status = WaitParameter("persist.kernel.move.finish", "true", MOVE_FILE_TIME_SERVICE);
    if (status != 0) {
        LOGE("wait move error, return value %{public}d.", status);
        return;
    }
    for (const auto &task_data : cycleTasks_) {
        task_data->RunTask(userId_);
    }
}

void CycleTaskRunner::InitTasks()
{
    //push tasks here
    cycleTasks_.push_back(std::make_shared<OptimizeCacheTask>(dataSyncManager_));
    cycleTasks_.push_back(std::make_shared<OptimizeStorageTask>(dataSyncManager_));
    cycleTasks_.push_back(std::make_shared<SaveSubscriptionTask>(dataSyncManager_));
    cycleTasks_.push_back(std::make_shared<ReportStatisticsTask>(dataSyncManager_));
    cycleTasks_.push_back(std::make_shared<DatabaseBackupTask>(dataSyncManager_));

    //do periodic check task last
    cycleTasks_.push_back(std::make_shared<PeriodicCheckTask>(dataSyncManager_));
}

static int32_t GetString(const string &key, string &val, NativeRdb::ResultSet &resultSet)
{
    int32_t index;
    int32_t err = resultSet.GetColumnIndex(key, index);
    if (err != NativeRdb::E_OK) {
        LOGE("result set get  %{public}s column index err %{public}d", key.c_str(), err);
        return E_RDB;
    }

    err = resultSet.GetString(index, val);
    if (err != 0) {
        LOGE("result set get string err %{public}d", err);
        return E_RDB;
    }

    return E_OK;
}

void CycleTaskRunner::SetRunableBundleNames()
{
    std::shared_ptr<std::set<std::string>> runnableBundleNames = make_shared<std::set<std::string>>();
    std::shared_ptr<NativeRdb::ResultSet> resultSet = nullptr;
    int32_t ret = DataSyncerRdbStore::GetInstance().QueryDataSyncer(userId_, resultSet);
    if (ret != 0 || resultSet == nullptr) {
        LOGE("query data syncer fail %{public}d", ret);
        return;
    }
    while (resultSet->GoToNextRow() == E_OK) {
        string bundleName;
        ret = GetString(BUNDLE_NAME, bundleName, *resultSet);
        if (ret != E_OK) {
            LOGE("get bundle name failed");
            continue;
        }
        std::time_t currentTime = std::time(nullptr);
        std::unique_ptr<CloudPrefImpl> cloudPrefImpl =
            std::make_unique<CloudPrefImpl>(userId_, bundleName,  CycleTask::FILE_PATH);
        std::time_t lastCheckTime;
        if (cloudPrefImpl == nullptr) {
            LOGE("cloudPrefImpl is nullptr");
            continue;
        }
        cloudPrefImpl->GetLong("lastCheckTime", lastCheckTime);
        if (lastCheckTime != 0 && difftime(currentTime, lastCheckTime) < CycleTask::ONE_DAY) {
            continue;
        }
        bool cloudStatus = CloudStatus::IsCloudStatusOkay(bundleName, userId_);
        if (!cloudStatus) {
            LOGI(" %{public}s cloud status is not ok, skip task, ret is %{public}d", bundleName.c_str(), ret);
            cloudPrefImpl->SetLong("lastCheckTime", currentTime);
            continue;
        }
        cloudPrefImpl->Delete("lastCheckTime");
        runnableBundleNames->insert(bundleName);
    }

    for (auto task_data  : cycleTasks_) {
        task_data->SetRunnableBundleNames(runnableBundleNames);
    }
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS