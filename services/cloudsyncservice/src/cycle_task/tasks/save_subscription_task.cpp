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

#include "cloud_file_kit.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
SaveSubscriptionTask::SaveSubscriptionTask(std::shared_ptr<CloudFile::DataSyncManager> dataSyncManager)
    : CycleTask("save_subscription_task", {}, ONE_DAY, dataSyncManager)
{
}

int32_t SaveSubscriptionTask::RunTaskForBundle(int32_t userId, std::string bundleName)
{
    auto instance = CloudFile::CloudFileKit::GetInstance();
    if (instance == nullptr) {
        LOGE("get cloud file helper instance failed");
        return E_NULLPTR;
    }

    auto cloudSyncHelper = instance->GetCloudSyncHelper(userId, bundleName);
    if (cloudSyncHelper == nullptr) {
        LOGE("get cloudSyncHelper failed");
        return E_NULLPTR;
    }

    return cloudSyncHelper->SaveSubScription();
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS