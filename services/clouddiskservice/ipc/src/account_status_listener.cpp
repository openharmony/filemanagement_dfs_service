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
#include "cloud_disk_sync_folder_manager.h"
#endif
#include "cloud_disk_service_callback_manager.h"
#include "cloud_disk_service_syncfolder.h"
#include "cloud_disk_sync_folder.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudDiskService {

using namespace AccountSA;

void AccountStatusSubscriber::OnStateChanged(const OsAccountStateData &data)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    auto state = data.state;
    auto userId = data.toId;
    LOGI("OnStateChanged state:%{public}d", state);
    if (state == OsAccountState::SWITCHED) {
        LOGI("Switched user");
        DiskMonitor::GetInstance().StopMonitor();
        CloudDiskServiceSyncFolder::CloudDiskServiceClearAll();
        CloudDiskServiceCallbackManager::GetInstance().ClearMap();
        CloudDiskSyncFolder::GetInstance().ClearMap();

        std::vector<FileManagement::SyncFolderExt> syncFolders;
        OHOS::FileManagement::CloudDiskSyncFolderManager::GetInstance().GetAllSyncFoldersForSa(syncFolders);
        for (auto item : syncFolders) {
            std::string path;
            if (!CloudDiskSyncFolder::GetInstance().PathToPhysicalPath(item.path_, std::to_string(userId), path)) {
                LOGE("Get path failed");
                continue;
            }
            SyncFolderValue syncFolderValue;
            syncFolderValue.bundleName = item.bundleName_;
            syncFolderValue.path = path;
            uint32_t syncFolderIndex = CloudDisk::CloudFileUtils::DentryHash(path);
            CloudDiskSyncFolder::GetInstance().AddSyncFolder(syncFolderIndex, syncFolderValue);
        }
        if (CloudDiskSyncFolder::GetInstance().GetSyncFolderSize() > 0) {
            DiskMonitor::GetInstance().StartMonitor(userId);
        }
    }
    if (state == OsAccountState::STOPPED) {
        LOGI("Stopped user");
        UnloadSa();
    }
#endif
}

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

void AccountStatusListener::Start()
{
    std::set<OsAccountState> states = {OsAccountState::STOPPED, OsAccountState::SWITCHED};
    OsAccountSubscribeInfo subscribeInfo(states);
    osAccountSubscriber_ = std::make_shared<AccountStatusSubscriber>(subscribeInfo);
    ErrCode errCode = OsAccountManager::SubscribeOsAccount(osAccountSubscriber_);
    LOGI("account subscribe errCode:%{public}d", errCode);
}

void AccountStatusListener::Stop()
{
    if (osAccountSubscriber_!= nullptr) {
        ErrCode errCode = OsAccountManager::UnsubscribeOsAccount(osAccountSubscriber_);
        osAccountSubscriber_ = nullptr;
    }
}
} // namespace CloudDiskService
} // namespace FileManagement
} // namespace OHOS