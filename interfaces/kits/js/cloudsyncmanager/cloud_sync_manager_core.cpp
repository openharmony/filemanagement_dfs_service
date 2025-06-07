/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "cloud_sync_manager_core.h"

#include <sys/types.h>

#include "cloud_sync_manager.h"
#include "dfs_error.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
using namespace std;

FsResult<void> CloudSyncManagerCore::DoChangeAppCloudSwitch(
    const string &accountId, const string &bundleName, bool status)
{
    LOGI("change app cloud switch.");
    int32_t result = CloudSyncManager::GetInstance().ChangeAppSwitch(accountId, bundleName, status);
    if (result == E_PERMISSION_DENIED || result == E_PERMISSION_SYSTEM) {
        LOGE("ChangeAppSwitch failed. ret = %{public}d", result);
        return FsResult<void>::Error(Convert2ErrNum(result));
    }

    return FsResult<void>::Success();
}

FsResult<void> CloudSyncManagerCore::DoNotifyEventChange(
    int32_t userId, const string &eventId, const string &extraData)
{
    LOGI("NotifyEventChange entrance");
    int32_t result = CloudSyncManager::GetInstance().NotifyEventChange(userId, eventId, extraData);
    if (result == E_PERMISSION_DENIED || result == E_PERMISSION_SYSTEM) {
        LOGE("NotifyEventChange failed. ret = %{public}d", result);
        return FsResult<void>::Error(Convert2ErrNum(result));
    }

    return FsResult<void>::Success();
}

FsResult<void> CloudSyncManagerCore::DoNotifyDataChange(const string &accountId, const string &bundleName)
{
    LOGI("NotifyDataChange entrance");
    int32_t result = CloudSyncManager::GetInstance().NotifyDataChange(accountId, bundleName);
    if (result == E_PERMISSION_DENIED || result == E_PERMISSION_SYSTEM) {
        LOGE("NotifyDataChange failed. ret = %{public}d", result);
        return FsResult<void>::Error(Convert2ErrNum(result));
    }

    return FsResult<void>::Success();
}

FsResult<void> CloudSyncManagerCore::DoDisableCloud(const string &accountId)
{
    LOGI("DisableCloud");
    int32_t result = CloudSyncManager::GetInstance().DisableCloud(accountId);
    if (result == E_PERMISSION_DENIED || result == E_PERMISSION_SYSTEM) {
        LOGE("DisableCloud failed. ret = %{public}d", result);
        return FsResult<void>::Error(Convert2ErrNum(result));
    }

    return FsResult<void>::Success();
}

FsResult<void> CloudSyncManagerCore::DoEnableCloud(const string &accountId, const SwitchDataObj &switchData)
{
    LOGI("EnableCloud");
    int32_t result = CloudSyncManager::GetInstance().EnableCloud(accountId, switchData);
    if (result == E_PERMISSION_DENIED || result == E_PERMISSION_SYSTEM) {
        LOGE("EnableCloud failed. ret = %{public}d", result);
        return FsResult<void>::Error(Convert2ErrNum(result));
    }

    return FsResult<void>::Success();
}

FsResult<void> CloudSyncManagerCore::DoClean(const std::string &accountId, const CleanOptions &cleanOptions)
{
    LOGI("Clean");
    int32_t result = CloudSyncManager::GetInstance().Clean(accountId, cleanOptions);
    if (result == E_PERMISSION_DENIED || result == E_PERMISSION_SYSTEM) {
        LOGE("Clean failed. ret = %{public}d", result);
        return FsResult<void>::Error(Convert2ErrNum(result));
    }

    return FsResult<void>::Success();
}
} // namespace OHOS::FileManagement::CloudSync