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

#include "sync_rule/package_status_listener.h"

#include "cloud_status.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "data_syncer_rdb_store.h"
#include "result_set.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
constexpr int32_t BASE_USER_RANGE = 200000;

PackageStatusSubscriber::PackageStatusSubscriber(const EventFwk::CommonEventSubscribeInfo &subscribeInfo,
                                                 std::shared_ptr<PackageStatusListener> listener)
    : EventFwk::CommonEventSubscriber(subscribeInfo), listener_(listener)
{
}

void PackageStatusSubscriber::OnReceiveEvent(const EventFwk::CommonEventData &eventData)
{
    auto action = eventData.GetWant().GetAction();
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED) {
        auto bundleName = eventData.GetWant().GetBundle();
        std::string userIdKey = "uid";
        auto userId = eventData.GetWant().GetIntParam(userIdKey.c_str(), -1);
        if (userId < 0) {
            LOGE("Get UserId Failed!");
            return;
        }
        userId = userId / BASE_USER_RANGE;
        listener_->RemovedClean(bundleName, userId);
    }
}

PackageStatusListener::PackageStatusListener(std::shared_ptr<CloudFile::DataSyncManager> dataSyncManager)
{
    dataSyncManager_ = dataSyncManager;
}

PackageStatusListener::~PackageStatusListener()
{
    Stop();
}

void PackageStatusListener::RemovedClean(const std::string &bundleName, const int32_t userId)
{
    LOGI("RemovedClean Start");
    if (!IsCloudSyncOn(userId, bundleName)) {
        LOGI("Cloud Sync is Off");
        return;
    }
    int ret = CloudStatus::ChangeAppSwitch(bundleName, userId, false);
    if (ret != 0) {
        LOGE("CloudStatus ChangeAppSwitch failed, ret: %{public}d", ret);
        return;
    }
    ret = dataSyncManager_->TriggerStopSync(bundleName, userId, false, SyncTriggerType::CLOUD_TRIGGER);
    if (ret != 0) {
        LOGE("DataSyncerManager Trigger Stopsync failed, ret: %{public}d", ret);
        return;
    }
    ret = dataSyncManager_->ChangeAppSwitch(bundleName, userId, false);
    if (ret != 0) {
        LOGE("DataSyncerManager ChangeAppSwitch failed, ret: %{public}d", ret);
        return;
    }
    ret = dataSyncManager_->CleanCloudFile(userId, bundleName, CLEAR_DATA);
    if (ret != 0) {
        LOGE("CleanCloudFile failed, ret: %{public}d", ret);
        return;
    }
    LOGI("RemovedClean Complete");
}

void PackageStatusListener::Start()
{
    /* subscribe Package Removed Status */
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED);
    EventFwk::CommonEventSubscribeInfo info(matchingSkills);
    commonEventSubscriber_ = std::make_shared<PackageStatusSubscriber>(info, shared_from_this());
    auto subRet = EventFwk::CommonEventManager::SubscribeCommonEvent(commonEventSubscriber_);
    LOGI("PackageStatusSubscirber SubscribeResult = %{public}d", subRet);
}

void PackageStatusListener::Stop()
{
    if (commonEventSubscriber_ != nullptr) {
        EventFwk::CommonEventManager::UnSubscribeCommonEvent(commonEventSubscriber_);
        commonEventSubscriber_ = nullptr;
    }
}

bool PackageStatusListener::IsCloudSyncOn(const int32_t userId, const std::string &bundleName)
{
    std::shared_ptr<NativeRdb::ResultSet> resultSet;
    int32_t ret = DataSyncerRdbStore::GetInstance().QueryCloudSync(userId, bundleName, resultSet);
    if (ret != 0 || resultSet == nullptr) {
        LOGE("DataSyncerRdbStore Query failed!");
        return false;
    }
    int rowCount = 0;
    ret = resultSet->GetRowCount(rowCount);
    if (ret != 0 || rowCount < 0) {
        LOGE("Get Row Count failed, ret: %{public}d, rowCount: %{public}d", ret, rowCount);
        return false;
    }
    return rowCount == 1;
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS