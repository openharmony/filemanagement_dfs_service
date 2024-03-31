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

#include "save_subscription_task.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
SaveSubscriptionTask::SaveSubscriptionTask(std::shared_ptr<DataSyncManager> dataSyncManager)
    : CycleTask("save_subscription_task", {}, ONE_DAY, dataSyncManager)
{
}

int32_t SaveSubscriptionTask::RunTaskForBundle(int32_t userId, std::string bundleName)
{
    auto dataSyncer = dataSyncManager_->GetDataSyncer(bundleName, userId);
    if (!dataSyncer) {
        LOGE("Get dataSyncer failed, bundleName: %{private}s", bundleName.c_str());
        return E_INVAL_ARG;
    }
    int32_t ret = dataSyncManager_->InitSdk(userId, bundleName, dataSyncer);
    if (ret != E_OK) {
        return ret;
    }
    if (!dataSyncer) {
        LOGE("Get dataSyncer failed, bundleName: %{private}s", bundleName.c_str());
        return E_INVAL_ARG;
    }
    dataSyncer->SaveSubscription();
    return E_OK;
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS