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
#include "cloud_disk_sync_folder.h"
#include "common_event_manager.h"
#include "common_event_support.h"

#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudDiskService {

AccountStatusSubscriber::AccountStatusSubscriber(const EventFwk::CommonEventSubscribeInfo &subscribeInfo)
    : EventFwk::CommonEventSubscriber(subscribeInfo)
{
}

void AccountStatusSubscriber::OnReceiveEvent(const EventFwk::CommonEventData &eventData)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    auto action = eventData.GetWant().GetAction();
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_USER_STARTED) {
        LOGI("OnStartUser!");

        std::vector<FileManagement::SyncFolderExt> syncFolders;
        OHOS::FileManagement::CloudDiskSyncFolderManager::GetInstance().GetAllSyncFoldersForSa(syncFolders);
        int32_t userId = DfsuAccessTokenHelper::GetUserId();
        if (userId == 0) {
            DfsuAccessTokenHelper::GetAccountId(userId);
        }
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
            DiskMonitor::GetInstance().StartMonitor(eventData.GetCode());
        }
    }
#endif
}

AccountStatusListener::~AccountStatusListener()
{
    Stop();
}

void AccountStatusListener::Start()
{
    /* subscribe Account login, logout, Package remove and Datashare ready */
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_USER_STARTED);
    EventFwk::CommonEventSubscribeInfo info(matchingSkills);
    commonEventSubscriber_ = std::make_shared<AccountStatusSubscriber>(info);
    auto subRet = EventFwk::CommonEventManager::SubscribeCommonEvent(commonEventSubscriber_);
    LOGI("Subscriber end, SubscribeResult = %{public}d", subRet);
}

void AccountStatusListener::Stop()
{
    if (commonEventSubscriber_ != nullptr) {
        EventFwk::CommonEventManager::UnSubscribeCommonEvent(commonEventSubscriber_);
        commonEventSubscriber_ = nullptr;
    }
}

} // namespace CloudDiskService
} // namespace FileManagement
} // namespace OHOS