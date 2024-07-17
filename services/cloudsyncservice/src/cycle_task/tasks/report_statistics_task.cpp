/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "report_statistics_task.h"

#include "parameters.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
ReportStatisticsTask::ReportStatisticsTask(std::shared_ptr<CloudFile::DataSyncManager> dataSyncManager)
    : CycleTask("report_statistics_task", {"com.ohos.photos"}, TWO_DAY, dataSyncManager)
{
}

int32_t ReportStatisticsTask::RunTaskForBundle(int32_t userId, std::string bundleName)
{
    int32_t ret = dataSyncManager_->ReportDownloadStat(bundleName, userId);
    if (ret != E_OK) {
        LOGE("report download file stat failed");
    }
    ret = dataSyncManager_->ReportReadFileStat(bundleName, userId);
    if (ret != E_OK) {
        LOGE("report read file stat failed");
    }
    return ret;
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS