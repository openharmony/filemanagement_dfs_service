/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "account_status_listener.h"

#include <fcntl.h>

#ifdef SUPPORT_CLOUD_DISK_SERVICE
#include "cloud_disk_comm.h"
#include "cloud_disk_service_error.h"
#include "cloud_disk_sync_folder_manager.h"
#endif
#include "cloud_disk_service_callback_manager.h"
#include "cloud_disk_service_syncfolder.h"
#include "cloud_disk_sync_folder.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "iservice_registry.h"
#include "placeholder_callback_manager.h"
#include "placeholder_progress_manager.h"
#include "placeholder_task_manager.h"
#include "system_ability_definition.h"

#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudDiskService {

using namespace AccountSA;

void AccountStatusSubscriber::SetCurrentUserId(int32_t userId)
{
    currentUserId_ = userId;
}

void AccountStatusSubscriber::OnStateChanged(const OsAccountStateData &data)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    auto state = data.state;
    auto userId = data.toId;
    LOGI("OnStateChanged state:%{public}d, userId: %{public}d, currentUserId: %{public}d", state, userId,
         currentUserId_);
    if (state == OsAccountState::SWITCHED) {
        HandleUserSwitched(userId);
        return;
    }
    if (state == OsAccountState::STOPPED) {
        if (userId != currentUserId_) {
            LOGI("Ignore stopped userId: %{public}d, current userId: %{public}d", userId, currentUserId_);
            return;
        }
        LOGI("Stopped user");
        UnloadSa();
    }
#endif
}

#ifdef SUPPORT_CLOUD_DISK_SERVICE
void AccountStatusSubscriber::HandleUserSwitched(int32_t userId)
{
    LOGI("Switched user");
    SetCurrentUserId(userId);
    DiskMonitor::GetInstance().StopMonitor();
    PlaceholderCallbackManager::GetInstance().ClearAll();
    PlaceholderTaskManager::GetInstance().CancelAllTasks(PlaceholderTaskCancelReason::USER_SWITCH);
    PlaceholderTaskManager::GetInstance().ClearCancellationRecords();
    PlaceholderProgressManager::GetInstance().Drain();
    PlaceholderProgressManager::GetInstance().Clear();
    CloudDiskServiceSyncFolder::CloudDiskServiceClearAll();
    CloudDiskServiceCallbackManager::GetInstance().ClearMap();
    CloudDiskSyncFolder::GetInstance().ClearMap();

    std::vector<FileManagement::SyncFolderExt> syncFolders;
    int32_t ret = CloudDiskSyncFolderManager::GetInstance().GetAllSyncFoldersForSa(syncFolders);
    if (ret != E_OK) {
        LOGE("Get all sync folders for sa failed, ret: %{public}d, syncFolderSize: %{public}zu", ret,
             syncFolders.size());
        UnloadSa();
        return;
    }
    for (const auto &item : syncFolders) {
        std::string path;
        if (CloudDiskSyncFolder::GetInstance().PathToPhysicalPath(item.path_, std::to_string(userId), path) != E_OK) {
            LOGE("Get path failed");
            continue;
        }
        SyncFolderValue syncFolderValue = {item.bundleName_, path};
        uint32_t syncFolderIndex = CloudDisk::CloudFileUtils::DentryHash(path);
        CloudDiskSyncFolder::GetInstance().AddSyncFolder(syncFolderIndex, syncFolderValue);
    }
    int32_t syncFolderSize = CloudDiskSyncFolder::GetInstance().GetSyncFolderSize();
    if (syncFolderSize == 0) {
        LOGI("No sync folder, unload sa");
        UnloadSa();
        return;
    }
    if (syncFolderSize > 0) {
        DiskMonitor::GetInstance().StartMonitor(userId);
    }
}
#endif

void AccountStatusSubscriber::UnloadSa()
{
    DiskMonitor::GetInstance().StopMonitor();
    auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        LOGE("get samgr failed");
        return;
    }
    int32_t ret = samgrProxy->UnloadSystemAbility(FILEMANAGEMENT_CLOUD_DISK_SERVICE_SA_ID);
    if (ret != ERR_OK) {
        LOGE("remove system ability failed");
        return;
    }
}

AccountStatusListener::~AccountStatusListener()
{
    Stop();
}

void AccountStatusListener::Start(int32_t currentUserId)
{
    std::set<OsAccountState> states = {OsAccountState::STOPPED, OsAccountState::SWITCHED};
    OsAccountSubscribeInfo subscribeInfo(states);
    osAccountSubscriber_ = std::make_shared<AccountStatusSubscriber>(subscribeInfo, currentUserId);
    ErrCode errCode = OsAccountManager::SubscribeOsAccount(osAccountSubscriber_);
    LOGI("account subscribe errCode:%{public}d", errCode);
}

void AccountStatusListener::Stop()
{
    if (osAccountSubscriber_ != nullptr) {
        ErrCode errCode = OsAccountManager::UnsubscribeOsAccount(osAccountSubscriber_);
        osAccountSubscriber_ = nullptr;
    }
}

} // namespace CloudDiskService
} // namespace FileManagement
} // namespace OHOS
