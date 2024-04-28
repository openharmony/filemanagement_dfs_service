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
#include <regex>

#include "cloud_disk_data_syncer.h"
#include "data_syncer_rdb_col.h"
#include "data_syncer_rdb_store.h"
#include "data_sync_const.h"
#include "dfs_error.h"
#include "gallery_data_syncer.h"
#include "ipc/cloud_sync_callback_manager.h"
#include "sdk_helper.h"
#include "sync_rule/battery_status.h"
#include "sync_rule/cloud_status.h"
#include "sync_rule/network_status.h"
#include "sync_rule/system_load.h"
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

    if (NetworkStatus::GetNetConnStatus() == NetworkStatus::NetConnStatus::NO_NETWORK) {
        LOGE("network is not available");
        dataSyncer->UpdateErrorCode(E_SYNC_FAILED_NETWORK_NOT_AVAILABLE);
        return E_SYNC_FAILED_NETWORK_NOT_AVAILABLE;
    }

    auto ret = InitSdk(userId, bundleName, dataSyncer);
    if (ret != E_OK) {
        return ret;
    }
    ret = IsSkipSync(bundleName, userId, forceFlag);
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

    auto ret = InitSdk(userId, bundleName, dataSyncer);
    if (ret != E_OK) {
        return ret;
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
    auto ret = InitSdk(userId, bundleName, dataSyncer);
    if (ret != E_OK) {
        return ret;
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
    auto ret = InitSdk(userId, bundleName, dataSyncer);
    if (ret != E_OK) {
        return ret;
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
    int32_t userId = 0;
    RETURN_ON_ERR(GetUserId(userId));
    map<string, DataSyncerInfo> dataSyncerInfoMaps;
    GetAllDataSyncerInfo(userId, dataSyncerInfoMaps);

    if (dataSyncerInfoMaps.size() == 0) {
        LOGI("not need to trigger sync");
        return E_OK;
    }

    int32_t ret = E_OK;
    for (const auto &iter : dataSyncerInfoMaps) {
        auto &bundleName = iter.first;
        if (triggerType == SyncTriggerType::NETWORK_AVAIL_TRIGGER) {
            auto &dataSyncerInfo = iter.second;
            auto dataSyncer = GetDataSyncer(bundleName, userId);
            if (!dataSyncer) {
                LOGE(" Clean Get dataSyncer failed, bundleName: %{private}s", bundleName.c_str());
                continue;
            }
            if ((dataSyncerInfo.syncState != SyncState::SYNC_FAILED) &&
                (dataSyncer->GetErrorType() != ErrorType::NETWORK_UNAVAILABLE)) {
                LOGI("last sync is ok, no need trigger sync, bundleName:%{public}s, syncState:%{public}d",
                     bundleName.c_str(), static_cast<int32_t>(dataSyncerInfo.syncState));
                continue;
            }
        }
        ret = TriggerStartSync(bundleName, userId, false, triggerType);
        if (ret) {
            LOGE("trigger sync failed, ret = %{public}d, bundleName = %{public}s", ret, bundleName.c_str());
        }
    }
    return ret;
}

std::shared_ptr<DataSyncer> DataSyncManager::GetDataSyncer(const std::string &bundle, const int32_t userId)
{
    std::lock_guard<std::mutex> lck(dataSyncMutex_);
    currentUserId_ = userId;
    string bundleName;
    Convert2BundleName(bundle, bundleName);
    string key = bundleName + to_string(userId);
    std::shared_ptr<DataSyncer> dataSyncer;
    if (dataSyncersMap_.Find(key, dataSyncer)) {
        return dataSyncer;
    }

    if (bundleName == GALLERY_BUNDLE_NAME) {
        dataSyncer = std::make_shared<GalleryDataSyncer>(GALLERY_BUNDLE_NAME, userId);
    } else {
        dataSyncer = std::make_shared<CloudDiskDataSyncer>(bundleName, userId);
    }
    int32_t ret = dataSyncer->Init(bundleName, userId);
    if (ret != E_OK) {
        return nullptr;
    }
    dataSyncersMap_.EnsureInsert(key, dataSyncer);
    DataSyncerRdbStore::GetInstance().Insert(userId, bundleName);
    return dataSyncer;
}

int32_t DataSyncManager::IsSkipSync(const std::string &bundle, const int32_t userId, bool forceFlag)
{
    string bundleName;
    Convert2BundleName(bundle, bundleName);
    if (!CloudStatus::IsCloudStatusOkay(bundleName, userId)) {
        LOGE("cloud status is not OK");
        return E_CLOUD_SDK;
    }
    if (!BatteryStatus::IsBatteryCapcityOkay()) {
        return E_SYNC_FAILED_BATTERY_TOO_LOW;
    }
    if (!forceFlag && !SystemLoadStatus::IsLoadStatusOkay()) {
        return E_SYSTEM_LOAD_OVER;
    }
    return E_OK;
}

int32_t DataSyncManager::CleanCloudFile(const int32_t userId, const std::string &bundleName, const int action)
{
    LOGI("Enter function CleanCloudFile");
    auto dataSyncer = GetDataSyncer(bundleName, userId);
    if (!dataSyncer) {
        LOGE(" Clean Get dataSyncer failed, bundleName: %{private}s", bundleName.c_str());
        return E_INVAL_ARG;
    }
    auto ret = InitSdk(userId, bundleName, dataSyncer);
    if (ret != E_OK) {
        LOGW("sdk init fail");
    }
    LOGI("bundleName:%{private}s, userId:%{private}d", dataSyncer->GetBundleName().c_str(), dataSyncer->GetUserId());
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

int32_t DataSyncManager::InitSdk(const int32_t userId, const string &bundle, std::shared_ptr<DataSyncer> dataSyncer)
{
    std::lock_guard<std::mutex> lck(sdkHelperMutex_);
    if (dataSyncer->HasSdkHelper()) {
        return E_OK;
    }
    /* get sdk helper */
    auto sdkHelper = std::make_shared<SdkHelper>();
    string bundleName;
    Convert2BundleName(bundle, bundleName);
    auto ret = sdkHelper->Init(userId, bundleName);
    if (ret != E_OK) {
        LOGE("get sdk helper err %{public}d", ret);
        return ret;
    }
    dataSyncer->SetSdkHelper(sdkHelper);
    return E_OK;
}

void DataSyncManager::Convert2BundleName(const string &bundle, string &bundleName)
{
    string photo = ".photos";
    bundleName = bundle;
    if (bundleName == MEDIALIBRARY_BUNDLE_NAME || (bundle.size() > photo.size() &&
        (bundle.substr(bundle.size() - photo.size()) == photo))) {
        bundleName = GALLERY_BUNDLE_NAME;
    }
}

int32_t DataSyncManager::DownloadThumb()
{
    int32_t userId = 0;
    RETURN_ON_ERR(GetUserId(userId));
    auto dataSyncer = GetDataSyncer(GALLERY_BUNDLE_NAME, userId);
    if (!dataSyncer) {
        LOGE("Get dataSyncer failed, bundleName: %{private}s", GALLERY_BUNDLE_NAME.c_str());
        return E_INVAL_ARG;
    }

    auto ret = InitSdk(userId, GALLERY_BUNDLE_NAME, dataSyncer);
    if (ret != E_OK) {
        return ret;
    }
    return dataSyncer->DownloadThumb(DataHandler::DownloadThmType::SCREENOFF_TRIGGER);
}

int32_t DataSyncManager::GetUserId(int32_t &userId)
{
    vector<int32_t> activeUsers;
    if (AccountSA::OsAccountManager::QueryActiveOsAccountIds(activeUsers) != E_OK || activeUsers.empty()) {
        LOGE("query active user failed");
        return E_OSACCOUNT;
    }
    userId = activeUsers.front();
    RETURN_ON_ERR(IsUserVerified(userId));
    return E_OK;
}

int32_t DataSyncManager::CleanCache(const string &bundleName, const int32_t userId, const string &uri)
{
    auto dataSyncer = GetDataSyncer(bundleName, userId);
    if (!dataSyncer) {
        LOGE("Get dataSyncer failed, bundleName: %{private}s", GALLERY_BUNDLE_NAME.c_str());
        return E_INVAL_ARG;
    }

    return dataSyncer->CleanCache(uri);
}

int32_t DataSyncManager::GetAllDataSyncerInfo(const int32_t userId, map<string, DataSyncerInfo> &dataSyncerInfoMaps)
{
    std::shared_ptr<NativeRdb::ResultSet> resultSet = nullptr;
    RETURN_ON_ERR(DataSyncerRdbStore::GetInstance().QueryDataSyncer(userId, resultSet));

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

        DataSyncerInfo dataSyncerInfo = {
            .syncState = static_cast<SyncState>(state),
        };
        dataSyncerInfoMaps.insert({bundleName, dataSyncerInfo});
    }
    return E_OK;
}

int32_t DataSyncManager::DisableCloud(const int32_t userId)
{
    map<string, DataSyncerInfo> dataSyncerInfoMaps;
    GetAllDataSyncerInfo(userId, dataSyncerInfoMaps);
    if (dataSyncerInfoMaps.size() == 0) {
        LOGI("not need to clean");
        return E_OK;
    }

    for (const auto &iter : dataSyncerInfoMaps) {
        auto &bundle = iter.first;
        auto dataSyncer = GetDataSyncer(bundle, userId);
        if (!dataSyncer) {
            LOGE("Get dataSyncer failed, bundleName: %{private}s", bundle.c_str());
            return E_INVAL_ARG;
        }

        dataSyncer->DisableCloud();
    }
    return E_OK;
}
} // namespace OHOS::FileManagement::CloudSync
