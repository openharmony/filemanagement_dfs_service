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
#include "service_proxy.h"

#include <sstream>

#include "cloud_file_sync_service_interface_code.h"
#include "cloud_sync_common.h"
#include "dfs_error.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {

constexpr int LOAD_SA_TIMEOUT_MS = 4000;

int32_t CloudSyncServiceProxy::TriggerSyncInner(const std::string &bundleName, const int32_t &userId)
{
    LOGI("Trigger Sync");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("Failed to write interface token");
        return E_BROKEN_IPC;
    }

    if (!data.WriteString(bundleName)) {
        LOGE("Failed to send the bundle name");
        return E_INVAL_ARG;
    }

    if (!data.WriteInt32(userId)) {
        LOGE("Failed to send the user id");
        return E_INVAL_ARG;
    }

    auto remote = Remote();
    if (!remote) {
        LOGE("remote is nullptr");
        return E_BROKEN_IPC;
    }

    int32_t ret = remote->SendRequest(
        static_cast<uint32_t> (CloudFileSyncServiceInterfaceCode::SERVICE_CMD_TRIGGER_SYNC),
        data, reply, option);
    if (ret != E_OK) {
        LOGE("Failed to send out the request, errno: %{public}d", ret);
        return E_BROKEN_IPC;
    }
    LOGI("TriggerSyncInner Success");
    return reply.ReadInt32();
}

sptr<ICloudSyncService> ServiceProxy::GetInstance()
{
    LOGI("GetInstance");
    std::unique_lock<std::mutex> lock(instanceMutex_);
    if (serviceProxy_ != nullptr) {
        if (serviceProxy_->AsObject() != nullptr && !serviceProxy_->AsObject()->IsObjectDead()) {
            return serviceProxy_;
        }
    }

    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        LOGE("Samgr is nullptr");
        return nullptr;
    }
    sptr<ServiceProxyLoadCallback> cloudSyncLoadCallback = new ServiceProxyLoadCallback();
    if (cloudSyncLoadCallback == nullptr) {
        LOGE("cloudSyncLoadCallback is nullptr");
        return nullptr;
    }
    int32_t ret = samgr->LoadSystemAbility(FILEMANAGEMENT_CLOUD_SYNC_SERVICE_SA_ID, cloudSyncLoadCallback);
    if (ret != E_OK) {
        LOGE("Failed to load System Ability, systemAbilityId: %{public}d, ret code: %{public}d",
            FILEMANAGEMENT_CLOUD_SYNC_SERVICE_SA_ID, ret);
        return nullptr;
    }
    std::unique_lock<std::mutex> proxyLock(proxyMutex_);
    auto waitStatus = cloudSyncLoadCallback->proxyConVar_.wait_for(
        proxyLock, std::chrono::milliseconds(LOAD_SA_TIMEOUT_MS),
        [cloudSyncLoadCallback]() { return cloudSyncLoadCallback->isLoadSuccess_.load(); });
    if (!waitStatus) {
        LOGE("Load CloudSync SA timeout");
        return nullptr;
    }
    return serviceProxy_;
}

void ServiceProxy::InvaildInstance()
{
    LOGI("Invalid Instance");
    std::unique_lock<std::mutex> lock(instanceMutex_);
    serviceProxy_ = nullptr;
}

void CloudSyncServiceProxy::ServiceProxyLoadCallback::OnLoadSystemAbilitySuccess(
    int32_t systemAbilityId,
    const sptr<IRemoteObject> &remoteObject)
{
    LOGI("Load CloudSync SA success, systemAbilityId: %{public}d, remote Obj result: %{private}s",
        systemAbilityId, (remoteObject == nullptr ? "false" : "true"));
    std::unique_lock<std::mutex> lock(proxyMutex_);
    if (serviceProxy_ != nullptr) {
        LOGE("CloudSync SA proxy has been loaded");
    } else {
        serviceProxy_ = iface_cast<ICloudSyncService>(remoteObject);
    }
    isLoadSuccess_.store(true);
    proxyConVar_.notify_one();
}

void CloudSyncServiceProxy::ServiceProxyLoadCallback::OnLoadSystemAbilityFail(
    int32_t systemAbilityId)
{
    LOGI("Load CloudSync SA failed, systemAbilityId: %{public}d", systemAbilityId);
    std::unique_lock<std::mutex> lock(proxyMutex_);
    serviceProxy_ = nullptr;
    isLoadSuccess_.store(false);
    proxyConVar_.notify_one();
}

int32_t CloudSyncServiceProxy::UnRegisterCallbackInner(const std::string &bundleName)
{
    return E_OK;
}

int32_t CloudSyncServiceProxy::UnRegisterFileSyncCallbackInner(const std::string &bundleName)
{
    return E_OK;
}

int32_t CloudSyncServiceProxy::RegisterCallbackInner(const sptr<IRemoteObject> &remoteObject,
                                                     const std::string &bundleName)
{
    return E_OK;
}

int32_t CloudSyncServiceProxy::RegisterFileSyncCallbackInner(const sptr<IRemoteObject> &remoteObject,
    const std::string &bundleName)
{
return E_OK;
}

int32_t CloudSyncServiceProxy::StartSyncInner(bool forceFlag, const std::string &bundleName)
{
    return E_OK;
}

int32_t CloudSyncServiceProxy::StartFileSyncInner(bool forceFlag, const std::string &bundleName)
{
    return E_OK;
}

int32_t CloudSyncServiceProxy::GetSyncTimeInner(int64_t &syncTime, const std::string &bundleName)
{
    return E_OK;
}

int32_t CloudSyncServiceProxy::CleanCacheInner(const std::string &uri)
{
    return E_OK;
}

int32_t CloudSyncServiceProxy::BatchDentryFileInsert(const std::vector<DentryFileInfoObj> &fileInfo,
    std::vector<std::string> &failCloudId)
{
    return E_OK;
}

int32_t CloudSyncServiceProxy::StopSyncInner(const std::string &bundleName, bool forceFlag)
{
    return E_OK;
}

int32_t CloudSyncServiceProxy::StopFileSyncInner(const std::string &bundleName, bool forceFlag)
{
    return E_OK;
}

int32_t CloudSyncServiceProxy::ResetCursor(const std::string &bundleName)
{
    return E_OK;
}

int32_t CloudSyncServiceProxy::ChangeAppSwitch(const std::string &accoutId,
                                               const std::string &bundleName,
                                               bool status)
{
    return E_OK;
}

int32_t CloudSyncServiceProxy::OptimizeStorage(const OptimizeSpaceOptions &optimizeOptions, bool isCallbackValid,
    const sptr<IRemoteObject> &optimizeCallback)
{
    return E_OK;
}

int32_t CloudSyncServiceProxy::StopOptimizeStorage()
{
    return E_OK;
}

int32_t CloudSyncServiceProxy::Clean(const std::string &accountId, const CleanOptions &cleanOptions)
{
    return E_OK;
}

int32_t CloudSyncServiceProxy::EnableCloud(const std::string &accoutId, const SwitchDataObj &switchData)
{
    return E_OK;
}

int32_t CloudSyncServiceProxy::DisableCloud(const std::string &accoutId)
{
    return E_OK;
}

int32_t CloudSyncServiceProxy::NotifyDataChange(const std::string &accoutId, const std::string &bundleName)
{
    return E_OK;
}

int32_t CloudSyncServiceProxy::NotifyEventChange(
    int32_t userId, const std::string &eventId, const std::string &extraData)
{
    return E_OK;
}

int32_t CloudSyncServiceProxy::StartDownloadFile(const std::string &uri,
                                                 const std::shared_ptr<CloudDownloadCallback> downloadCallback,
                                                 int64_t &downloadId)
{
    return E_OK;
}

int32_t CloudSyncServiceProxy::StartFileCacheWriteParcel(MessageParcel &data,
                                                         const std::vector<std::string> &pathVec,
                                                         int32_t &fieldkey,
                                                         const sptr<IRemoteObject> &downloadCallback,
                                                         int32_t timeout)
{
    return E_OK;
}

int32_t CloudSyncServiceProxy::StartFileCache(const std::vector<std::string> &uriVec,
                                              int64_t &downloadId,
                                              int32_t fieldkey,
                                              const sptr<IRemoteObject> &downloadCallback,
                                              int32_t timeout)
{
    return E_OK;
}

int32_t CloudSyncServiceProxy::StopDownloadFile(int64_t downloadId, bool needClean)
{
    return E_OK;
}

int32_t CloudSyncServiceProxy::StopFileCache(int64_t downloadId, bool needClean, int32_t timeout)
{
    return E_OK;
}

int32_t CloudSyncServiceProxy::DownloadThumb()
{
    return E_OK;
}

int32_t CloudSyncServiceProxy::UploadAsset(const int32_t userId, const std::string &request, std::string &result)
{
    return E_OK;
}

int32_t CloudSyncServiceProxy::DownloadFile(const int32_t userId,
                                            const std::string &bundleName,
                                            AssetInfoObj &assetInfoObj)
{
    return E_OK;
}

int32_t CloudSyncServiceProxy::DownloadFiles(const int32_t userId,
                                             const std::string &bundleName,
                                             const std::vector<AssetInfoObj> &assetInfoObj,
                                             std::vector<bool> &assetResultMap,
                                             int32_t connectTime)
{
    return E_OK;
}

int32_t CloudSyncServiceProxy::BatchCleanFile(const std::vector<CleanFileInfoObj> &fileInfo,
    std::vector<std::string> &failCloudId)
{
    return E_OK;
}

int32_t CloudSyncServiceProxy::DownloadAsset(const uint64_t taskId,
                                             const int32_t userId,
                                             const std::string &bundleName,
                                             const std::string &networkId,
                                             AssetInfoObj &assetInfoObj)
{
    return E_OK;
}

int32_t CloudSyncServiceProxy::RegisterDownloadAssetCallback(const sptr<IRemoteObject> &remoteObject)
{
    return E_OK;
}

int32_t CloudSyncServiceProxy::DeleteAsset(const int32_t userId, const std::string &uri)
{
    return E_OK;
}
} // namespace OHOS::FileManagement::CloudSync
