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

#include "data_syncer_rdb_col.h"
#include "data_syncer_rdb_store.h"
#include "dfs_error.h"
#include "gallery_data_syncer.h"
#include "ipc/cloud_sync_callback_manager.h"
#include "sdk_helper.h"
#include "sync_rule/battery_status.h"
#include "sync_rule/cloud_status.h"
#include "sync_rule/network_status.h"
#include "utils_log.h"

#include "os_account_manager.h"
#include "rdb_sql_utils.h"
#include "rdb_store_config.h"
namespace OHOS::FileManagement::CloudSync {
using namespace std;

int32_t DataSyncManager::TriggerStartSync(const std::string &bundleName,
                                          const int32_t userId,
                                          bool forceFlag,
                                          SyncTriggerType triggerType)
{
    auto dataSyncer = GetDataSyncer(bundleName, userId);
    if (!dataSyncer) {
        LOGE("Get dataSyncer failed, bundleName: %{private}s", bundleName.c_str());
        return E_INVAL_ARG;
    }

    auto ret = IsSkipSync(bundleName, userId);
    if (ret != E_OK) {
        return ret;
    }

    std::thread([dataSyncer, forceFlag, triggerType]() { dataSyncer->StartSync(forceFlag, triggerType); }).detach();

    return E_OK;
}

int32_t DataSyncManager::TriggerStopSync(const std::string &bundleName,
                                         const int32_t userId,
                                         SyncTriggerType triggerType)
{
    auto dataSyncer = GetDataSyncer(bundleName, userId);
    if (!dataSyncer) {
        LOGE("Get dataSyncer failed, bundleName: %{private}s", bundleName.c_str());
        return E_INVAL_ARG;
    }

    std::thread([dataSyncer, triggerType]() { dataSyncer->StopSync(triggerType); }).detach();
    return E_OK;
}

void DataSyncManager::RegisterCloudSyncCallback(const std::string &bundleName, const int32_t userId)
{
    auto dataSyncer = GetDataSyncer(bundleName, userId);
    if (!dataSyncer) {
        LOGE("Get dataSyncer failed, bundleName: %{private}s", bundleName.c_str());
        return;
    }

    /* notify app current sync state */
    dataSyncer->NotifyCurrentSyncState();
}

int32_t DataSyncManager::StartDownloadFile(const std::string &bundleName, const int32_t userId, const std::string path)
{
    auto dataSyncer = GetDataSyncer(bundleName, userId);
    if (!dataSyncer) {
        LOGE("Get dataSyncer failed, bundleName: %{private}s", bundleName.c_str());
        return E_INVAL_ARG;
    }

    return dataSyncer->StartDownloadFile(path, userId);
}

int32_t DataSyncManager::StopDownloadFile(const std::string &bundleName, const int32_t userId, const std::string path)
{
    auto dataSyncer = GetDataSyncer(bundleName, userId);
    if (!dataSyncer) {
        LOGE("Get dataSyncer failed, bundleName: %{private}s", bundleName.c_str());
        return E_INVAL_ARG;
    }

    return dataSyncer->StopDownloadFile(path, userId);
}

int32_t DataSyncManager::RegisterDownloadFileCallback(const std::string &bundleName,
                                                      const int32_t userId,
                                                      const sptr<ICloudDownloadCallback>& downloadCallback)
{
    auto dataSyncer = GetDataSyncer(bundleName, userId);
    if (!dataSyncer) {
        LOGE("Get dataSyncer failed, bundleName: %{private}s", bundleName.c_str());
        return E_INVAL_ARG;
    }
    std::thread([dataSyncer, userId, downloadCallback]() {
        dataSyncer->RegisterDownloadFileCallback(userId, downloadCallback);
    }).detach();
    return E_OK;
}

int32_t DataSyncManager::UnregisterDownloadFileCallback(const std::string &bundleName,
                                                        const int32_t userId)
{
    auto dataSyncer = GetDataSyncer(bundleName, userId);
    if (!dataSyncer) {
        LOGE("Get dataSyncer failed, bundleName: %{private}s", bundleName.c_str());
        return E_INVAL_ARG;
    }
    std::thread([dataSyncer, userId]() { dataSyncer->UnregisterDownloadFileCallback(userId); }).detach();
    return E_OK;
}

int32_t DataSyncManager::DelayedClean(const std::string &bundleName, const int32_t userId, int32_t action)
{
    auto dataSyncer = GetDataSyncer(bundleName, userId);
    return dataSyncer->ActualClean(action);
}


int32_t DataSyncManager::RestoreClean(const std::string &bundleName, const int32_t userId)
{
    auto dataSyncer = GetDataSyncer(bundleName, userId);
    return dataSyncer->CancelClean();
}

int32_t DataSyncManager::IsUserVerified(const int32_t userId)
{
    bool isVerified = false;
    if (AccountSA::OsAccountManager::IsOsAccountVerified(userId, isVerified) != E_OK) {
        LOGE("check user verified failed");
        return E_OSACCOUNT;
    }
    if (!isVerified) {
        LOGE("user is locked");
        return E_USER_LOCKED;
    }
    return E_OK;
}

int32_t DataSyncManager::TriggerRecoverySync(SyncTriggerType triggerType)
{
    std::vector<std::string> needSyncApps;
    {
        std::lock_guard<std::mutex> lck(dataSyncMutex_);
        vector<int32_t> activeUsers;
        if (AccountSA::OsAccountManager::QueryActiveOsAccountIds(activeUsers) != E_OK || activeUsers.empty()) {
            LOGE("query active user failed");
            return E_OSACCOUNT;
        }
        currentUserId_ = activeUsers.front();
        RETURN_ON_ERR(IsUserVerified(currentUserId_));
        std::shared_ptr<NativeRdb::ResultSet> resultSet;
        RETURN_ON_ERR(DataSyncerRdbStore::GetInstance().QueryDataSyncer(currentUserId_, resultSet));

        while (resultSet->GoToNextRow() == E_OK) {
            string bundleName;
            int32_t ret = DataConvertor::GetString(BUNDLE_NAME, bundleName, *resultSet);
            if (ret != E_OK) {
                LOGE("get bundle name failed");
                continue;
            }
            int32_t state;
            ret = DataConvertor::GetInt(SYNC_STATE, state, *resultSet);
            if (ret != E_OK) {
                LOGE("get sync state failed");
                continue;
            }
            if ((triggerType == SyncTriggerType::NETWORK_AVAIL_TRIGGER) ||
                static_cast<SyncState>(state) == SyncState::SYNC_FAILED) {
                needSyncApps.push_back(bundleName);
            }
        }
    }

    if (needSyncApps.size() == 0) {
        LOGI("not need to trigger sync");
        return E_OK;
    }

    int32_t ret = E_OK;
    for (const auto &app : needSyncApps) {
        ret = TriggerStartSync(app, currentUserId_, false, triggerType);
        if (ret) {
            LOGE("trigger sync failed, ret = %{public}d, bundleName = %{public}s", ret, app.c_str());
        }
    }
    return ret;
}

std::shared_ptr<DataSyncer> DataSyncManager::GetDataSyncer(const std::string &bundleName, const int32_t userId)
{
    std::lock_guard<std::mutex> lck(dataSyncMutex_);
    currentUserId_ = userId;
    for (auto dataSyncer : dataSyncers_) {
        if ((dataSyncer->GetBundleName() == bundleName) && (dataSyncer->GetUserId() == userId)) {
            return dataSyncer;
        }
    }

    std::shared_ptr<DataSyncer> dataSyncer = std::make_shared<GalleryDataSyncer>(bundleName, userId);
    int32_t ret = dataSyncer->Init(bundleName, userId);
    if (ret != E_OK) {
        return nullptr;
    }

    /* get sdk helper */
    auto sdkHelper = std::make_shared<SdkHelper>();
    ret = sdkHelper->Init(userId, bundleName);
    if (ret != E_OK) {
        LOGE("get sdk helper err %{public}d", ret);
        return nullptr;
    }
    dataSyncer->SetSdkHelper(sdkHelper);
    dataSyncers_.push_back(dataSyncer);
    DataSyncerRdbStore::GetInstance().Insert(userId, bundleName);
    return dataSyncer;
}

int32_t DataSyncManager::IsSkipSync(const std::string &bundleName, const int32_t userId) const
{
    if (NetworkStatus::GetNetConnStatus() == NetworkStatus::NetConnStatus::NO_NETWORK) {
        LOGE("network is not available");
        return E_SYNC_FAILED_NETWORK_NOT_AVAILABLE;
    }
    if (!CloudStatus::IsCloudStatusOkay(bundleName, userId)) {
        LOGE("cloud status is not OK");
        return E_CLOUD_SDK;
    }
    if (!BatteryStatus::IsBatteryCapcityOkay()) {
        return E_SYNC_FAILED_BATTERY_TOO_LOW;
    }
    return E_OK;
}

int32_t DataSyncManager::CleanCloudFile(const int32_t userId, const std::string &bundleName, const int action)
{
    LOGD("Enter function CleanCloudFile");
    auto dataSyncer = GetDataSyncer(bundleName, userId);
    if (!dataSyncer) {
        LOGE(" Clean Get dataSyncer failed, bundleName: %{private}s", bundleName.c_str());
        return E_INVAL_ARG;
    }

    LOGD("bundleName:%{private}s, userId:%{private}d", dataSyncer->GetBundleName().c_str(), dataSyncer->GetUserId());
    return dataSyncer->Clean(action);
}

int32_t DataSyncManager::OptimizeStorage(const std::string &bundleName, const int32_t userId, const int32_t agingDays)
{
    auto dataSyncer = GetDataSyncer(bundleName, userId);
    if (!dataSyncer) {
        LOGE("Get dataSyncer failed, bundleName: %{private}s", bundleName.c_str());
        return E_INVAL_ARG;
    }

    return dataSyncer->OptimizeStorage(agingDays);
}
} // namespace OHOS::FileManagement::CloudSync
