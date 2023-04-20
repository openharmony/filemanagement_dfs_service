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

#include "data_sync/data_sync_manager.h"

#include <thread>
#include <vector>

#include "dfs_error.h"
#include "gallery_data_syncer.h"
#include "ipc/cloud_sync_callback_manager.h"
#include "sync_rule/battery_status.h"
#include "sync_rule/cloud_status.h"
#include "sync_rule/network_status.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {

DataSyncManager::DataSyncManager()
{
    bundleNameConversionMap_["com.ohos.medialibrary.medialibrarydata"] = "com.ohos.photos";
    bundleNameConversionMap_["com.ohos.photos"] = "com.ohos.photos";
}

int32_t DataSyncManager::TriggerStartSync(const std::string bundleName,
                                          const int32_t userId,
                                          bool forceFlag,
                                          SyncTriggerType triggerType)
{
    auto it = bundleNameConversionMap_.find(bundleName);
    if (it == bundleNameConversionMap_.end()) {
        LOGE("trigger start sync failed, bundleName: %{private}s, useId: %{private}d", bundleName.c_str(), userId);
        return E_INVAL_ARG;
    }
    std::string appBundleName(it->second);
    auto ret = IsSkipSync(appBundleName, userId);
    if (ret != E_OK) {
        return ret;
    }
    auto dataSyncer = GetDataSyncer(appBundleName, userId);
    if (!dataSyncer) {
        LOGE("Get dataSyncer failed, bundleName: %{private}s", appBundleName.c_str());
        return E_SYNCER_NUM_OUT_OF_RANGE;
    }
    std::thread([dataSyncer, forceFlag, triggerType]() { dataSyncer->StartSync(forceFlag, triggerType); }).detach();
    return E_OK;
}

int32_t DataSyncManager::TriggerStopSync(const std::string bundleName,
                                         const int32_t userId,
                                         SyncTriggerType triggerType)
{
    auto it = bundleNameConversionMap_.find(bundleName);
    if (it == bundleNameConversionMap_.end()) {
        LOGE("trigger stop sync failed, bundleName: %{private}s, useId: %{private}d", bundleName.c_str(), userId);
        return E_INVAL_ARG;
    }
    std::string appBundleName(it->second);
    auto dataSyncer = GetDataSyncer(appBundleName, userId);
    if (!dataSyncer) {
        LOGE("Get dataSyncer failed, bundleName: %{private}s", appBundleName.c_str());
        return E_SYNCER_NUM_OUT_OF_RANGE;
    }
    std::thread([dataSyncer, triggerType]() { dataSyncer->StopSync(triggerType); }).detach();
    return E_OK;
}

int32_t DataSyncManager::TriggerRecoverySync(SyncTriggerType triggerType)
{
    std::vector<std::string> needSyncApps;
    {
        std::lock_guard<std::mutex> lck(dataSyncMutex_);
        if (currentUserId_ == INVALID_USER_ID) {
            LOGE("useId is invalid");
            return E_INVAL_ARG;
        }

        for (auto dataSyncer : dataSyncers_) {
            if ((dataSyncer->GetUserId() == currentUserId_) &&
                (dataSyncer->GetSyncState() == SyncState::SYNC_FAILED)) {
                auto bundleName = dataSyncer->GetBundleName();
                needSyncApps.push_back(bundleName);
            }
        }
    }

    if (needSyncApps.size() == 0) {
        LOGI("not need to trigger sync");
        return E_OK;
    }

    int32_t ret = E_OK;
    for (auto app : needSyncApps) {
        ret = TriggerStartSync(app, currentUserId_, false, triggerType);
        if (ret) {
            LOGE("trigger sync failed, ret = %{public}d, bundleName = %{public}s", ret, app.c_str());
        }
    }
    return E_OK;
}

std::shared_ptr<DataSyncer> DataSyncManager::GetDataSyncer(const std::string bundleName, const int32_t userId)
{
    std::lock_guard<std::mutex> lck(dataSyncMutex_);
    currentUserId_ = userId;
    for (auto dataSyncer : dataSyncers_) {
        if ((dataSyncer->GetBundleName() == bundleName) && (dataSyncer->GetUserId() == userId)) {
            return dataSyncer;
        }
    }

    std::shared_ptr<DataSyncer> dataSyncer_ = std::make_shared<GalleryDataSyncer>(bundleName, userId);
    dataSyncers_.push_back(dataSyncer_);
    return dataSyncer_;
}

int32_t DataSyncManager::IsSkipSync(const std::string bundleName, const int32_t userId) const
{
    if (!CloudStatus::IsCloudStatusOkay(bundleName)) {
        LOGE("cloud status is not OK");
        return E_SYNC_FAILED_CLOUD_NOT_READY;
    }
    if (NetworkStatus::GetNetConnStatus() == NetworkStatus::NetConnStatus::NO_NETWORK) {
        LOGE("network is not available");
        return E_SYNC_FAILED_NETWORK_NOT_AVAILABLE;
    }
    if (!BatteryStatus::IsBatteryCapcityOkay()) {
        return E_SYNC_FAILED_BATTERY_TOO_LOW;
    }
    return E_OK;
}
} // namespace OHOS::FileManagement::CloudSync