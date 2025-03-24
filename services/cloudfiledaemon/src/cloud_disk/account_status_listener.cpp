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

#include "account_status_listener.h"

#include <fcntl.h>

#include "cloud_daemon_statistic.h"
#include "cloud_file_utils.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "account_status.h"
#include "file_operations_base.h"
#include "file_operations_helper.h"
#include "fuse_manager.h"
#include "meta_file.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudDisk {

constexpr int32_t BASE_USER_RANGE = 200000;
static const std::string LOCAL_PATH_MNT_FUSE = "/mnt/data/";
static const std::string LOCAL_PATH_CLOUD_FUSE = "/cloud_fuse";
static const std::string LOCAL_PATH_DATA = "/data";

static void SwapMemory()
{
    std::FILE *file = fopen("/proc/self/reclaim", "w");
    if (file == nullptr) {
        LOGE("Failed to open reclaim, errno:%{public}d", errno);
        return;
    }
    std::string content = "1";
    if (fwrite(content.c_str(), content.size(), 1, file) < 0) {
        LOGE("Failed to write reclaim, errno:%{public}d", errno);
    }
    fclose(file);
    return;
}

AccountStatusSubscriber::AccountStatusSubscriber(const EventFwk::CommonEventSubscribeInfo &subscribeInfo)
    : EventFwk::CommonEventSubscriber(subscribeInfo)
{
}

void AccountStatusSubscriber::OnReceiveEvent(const EventFwk::CommonEventData &eventData)
{
    auto action = eventData.GetWant().GetAction();
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_HWID_LOGIN) {
        LOGI("Account Login!");
        AccountStatus::SetAccountState(AccountStatus::AccountState::ACCOUNT_LOGIN);
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_HWID_LOGOUT) {
        LOGI("Account Logout!");
        AccountStatus::SetAccountState(AccountStatus::AccountState::ACCOUNT_LOGOUT);
        SwapMemory();
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF &&
        AccountStatus::GetAccountState() == AccountStatus::AccountState::ACCOUNT_LOGIN) {
        LOGI("Screen Off and Swap Memory!");
        SwapMemory();
        CloudFile::CloudDaemonStatistic::GetInstance().UpdateStatData();
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED) {
        LOGI("Package removed and Clean clouddisk!");
        RemovedClean(eventData);
    }
}

AccountStatusListener::~AccountStatusListener()
{
    Stop();
}

void AccountStatusListener::Start()
{
    /* subscribe Account login and logout status */
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_HWID_LOGIN);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_HWID_LOGOUT);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED);
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

void AccountStatusSubscriber::RemovedClean(const EventFwk::CommonEventData &eventData)
{
    auto bundleName = eventData.GetWant().GetBundle();
    std::string userIdKey = "uid";
    auto userId = eventData.GetWant().GetIntParam(userIdKey.c_str(), -1);
    if (userId < 0) {
        LOGE("Get UserId Failed!");
        return;
    }
    userId = userId / BASE_USER_RANGE;
    struct fuse_session *se = nullptr;
    struct stat childSt {};
    struct stat parentSt {};
    std::string childDest = LOCAL_PATH_MNT_FUSE + std::to_string(userId) + LOCAL_PATH_CLOUD_FUSE +
        LOCAL_PATH_DATA + "/" + bundleName;
    std::string parentDest = LOCAL_PATH_MNT_FUSE + std::to_string(userId) + LOCAL_PATH_CLOUD_FUSE +
        LOCAL_PATH_DATA;
    std::string getSessionPath = LOCAL_PATH_MNT_FUSE + std::to_string(userId) + LOCAL_PATH_CLOUD_FUSE;
    se = CloudFile::FuseManager::GetInstance().GetSession(getSessionPath);
    if (se == nullptr) {
        LOGE("Session is Null!");
        return;
    }
    if (stat(childDest.c_str(), &childSt) == -1) {
        LOGE("Stat childDest Failed!");
        return;
    }
    if (stat(parentDest.c_str(), &parentSt) == -1) {
        LOGE("Stat parentDest Failed!");
        return;
    }
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(se->userdata);
    auto node = FileOperationsHelper::FindCloudDiskInode(data, static_cast<int64_t>(childSt.st_ino));
    if (node == nullptr) {
        LOGE("Node is Null!");
        return;
    }
    std::string localIdKey = std::to_string(node->parent) + node->fileName;
    int64_t key = static_cast<int64_t>(childSt.st_ino);
    FileOperationsHelper::PutCloudDiskInode(data, node, node->refCount, key);
    FileOperationsHelper::PutLocalId(data, node, node->refCount, localIdKey);
    if (fuse_lowlevel_notify_inval_entry(se, parentSt.st_ino, bundleName.c_str(), bundleName.size())) {
        fuse_lowlevel_notify_inval_inode(se, childSt.st_ino, 0, 0);
    }
    MetaFileMgr::GetInstance().CloudDiskClearAll();
    LOGI("Package Removed Complete Clean");
}
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS