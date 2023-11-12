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
#include "delay_clean_task.h"

#include <memory>
#include <sstream>
#include "cycle_task.h"
#include "dfs_error.h"
#include "data_sync_manager.h"
#include "cycle_task_runner.h"
#include "sync_rule/cloud_status.h"
#include "utils_log.h"


namespace OHOS {
namespace FileManagement {
namespace CloudSync {
DelayCleanTask::DelayCleanTask(std::shared_ptr<DataSyncManager> dataSyncManager)
    : CycleTask("delay_clean_task", "", CycleTaskRunner::ONE_DAY, dataSyncManager) {}

int32_t DelayCleanTask::RunTask(int32_t userId)
{
      int32_t ret = E_OK;
    std::string bundleNames[] = {"com.ohos.photos"};
    for (auto bundleName : bundleNames) {
        int32_t needClean;
        auto cloudPrefImpl = CloudPrefImpl(CycleTaskRunner::FILE_PATH);
        cloudPrefImpl.GetInt("needClean-" + bundleName + "-" + std::to_string(userId), needClean);
        if(needClean == NOT_NEED_CLEAN) {
            continue;
        } else if (needClean == NEED_CLEAN) {
            cloudPrefImpl.SetInt("needClean-" + bundleName + "-" + std::to_string(userId), DO_CLEAN);
        } else if (needClean == DO_CLEAN) {
            int32_t retTmp = dataSyncManager_->DelayedClean(bundleName, userId);
            if (retTmp != E_OK) {
                std::stringstream ss;
                ss << "sync task fail bundleName is " << bundleName;
                LOGE("%{public}s", ss.str().c_str());
                ret = retTmp;
            } else {
                cloudPrefImpl.SetInt("needClean-" + bundleName + "-" + std::to_string(userId), NOT_NEED_CLEAN);
            }
        }
    }
    return ret;
}


} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS