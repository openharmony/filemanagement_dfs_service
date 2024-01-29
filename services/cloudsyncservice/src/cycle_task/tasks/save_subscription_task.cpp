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
#include <memory>
#include "dfs_error.h"
#include "gallery_data_sync/data_convertor.h"
#include "data_syncer.h"
#include "data_sync_manager.h"
#include "data_syncer_rdb_col.h"
#include "data_syncer_rdb_store.h"
#include "cycle_task_runner.h"
#include "tasks/save_subscription_task.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
SaveSubscriptionTask::SaveSubscriptionTask(std::shared_ptr<DataSyncManager> dataSyncManager)
    : CycleTask("save_subscription_task", "", CycleTaskRunner::ONE_DAY, dataSyncManager) {}

int32_t SaveSubscriptionTask::RunTask(int32_t userId)
{
    std::shared_ptr<NativeRdb::ResultSet> resultSet = nullptr;
    RETURN_ON_ERR(DataSyncerRdbStore::GetInstance().QueryDataSyncer(userId, resultSet));

    while (resultSet->GoToNextRow() == E_OK) {
        int32_t ret = DataConvertor::GetString(BUNDLE_NAME, bundleName_, *resultSet);
        if (ret != E_OK) {
            LOGE("get bundle name failed");
            continue;
        }
        auto dataSyncer = dataSyncManager_->GetDataSyncer(bundleName_, userId);
        if (!dataSyncer) {
            LOGE("Get dataSyncer failed, bundleName: %{private}s", bundleName_.c_str());
            return E_INVAL_ARG;
        }
        ret = dataSyncManager_->InitSdk(userId, bundleName_, dataSyncer);
        if (ret != E_OK) {
            return ret;
        }
        if (!dataSyncer) {
            LOGE("Get dataSyncer failed, bundleName: %{private}s", bundleName_.c_str());
            return E_INVAL_ARG;
        }
        dataSyncer->SaveSubscription();
    }
    return E_OK;
}

} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS