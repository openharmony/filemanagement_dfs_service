/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "cloud_sync_service_proxy.h"

#include <sstream>

#include "dfs_error.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
using namespace std;

constexpr int LOADSA_TIMEOUT_MS = 4000;

int32_t CloudSyncServiceProxy::RegisterCallbackInner(const string &appPackageName,
                                                     const sptr<IRemoteObject> &remoteObject)
{
    LOGI("Start RegisterCallbackInner");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!remoteObject) {
        LOGI("Empty callback stub");
        return E_INVAL_ARG;
    }
    data.WriteInterfaceToken(GetDescriptor());

    if (!data.WriteRemoteObject(remoteObject)) {
        LOGE("Failed to send the callback stub");
        return E_INVAL_ARG;
    }

    if (!data.WriteString(appPackageName)) {
        LOGE("Failed to send the appPackageName");
        return E_INVAL_ARG;
    }

    int32_t ret = Remote()->SendRequest(ICloudSyncService::SERVICE_CMD_REGISTER_CALLBACK, data, reply, option);
    if (ret != E_OK) {
        stringstream ss;
        ss << "Failed to send out the requeset, errno:" << ret;
        LOGE("%{public}s, appPackageName:%{private}s", ss.str().c_str(), appPackageName.c_str());
        return E_BROKEN_IPC;
    }
    LOGI("RegisterCallbackInner Success");
    return reply.ReadInt32();
}

int32_t CloudSyncServiceProxy::StartSyncInner(const std::string &appPackageName, SyncType type, bool forceFlag)
{
    LOGI("Start Sync");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(GetDescriptor());

    if (!data.WriteString(appPackageName)) {
        LOGE("Failed to send the appPackageName");
        return E_INVAL_ARG;
    }

    if (!data.WriteInt32(static_cast<int32_t>(type))) {
        LOGE("Failed to send the sync type");
        return E_INVAL_ARG;
    }

    if (!data.WriteBool(forceFlag)) {
        LOGE("Failed to send the force flag");
        return E_INVAL_ARG;
    }

    int32_t ret = Remote()->SendRequest(ICloudSyncService::SERVICE_CMD_START_SYNC, data, reply, option);
    if (ret != E_OK) {
        stringstream ss;
        ss << "Failed to send out the requeset, errno:" << ret;
        LOGE("appPackageName:%{private}s", appPackageName.c_str());
        return E_BROKEN_IPC;
    }
    LOGI("StartSyncInner Success");
    return reply.ReadInt32();
}

int32_t CloudSyncServiceProxy::StopSyncInner(const std::string &appPackageName)
{
    LOGI("StopSync");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(GetDescriptor());

    if (!data.WriteString(appPackageName)) {
        LOGE("Failed to send the appPackageName");
        return E_INVAL_ARG;
    }

    int32_t ret = Remote()->SendRequest(ICloudSyncService::SERVICE_CMD_STOP_SYNC, data, reply, option);
    if (ret != E_OK) {
        stringstream ss;
        ss << "Failed to send out the requeset, errno:" << ret;
        LOGE("appPackageName:%{private}s", appPackageName.c_str());
        return E_BROKEN_IPC;
    }
    LOGI("StopSyncInner Success");
    return reply.ReadInt32();
}

sptr<ICloudSyncService> CloudSyncServiceProxy::GetInstance()
{
    LOGE("getinstance");
    unique_lock<mutex> lock(proxyMutex_);
    if (serviceProxy_ != nullptr) {
        return serviceProxy_;
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
        LOGE("Failed to Load systemAbility, systemAbilityId:%{pulbic}d, ret code:%{pulbic}d",
             FILEMANAGEMENT_CLOUD_SYNC_SERVICE_SA_ID, ret);
        return nullptr;
    }

    auto waitStatus = cloudSyncLoadCallback->proxyConVar_.wait_for(
        lock, std::chrono::milliseconds(LOADSA_TIMEOUT_MS),
        [cloudSyncLoadCallback]() { return cloudSyncLoadCallback->isLoadSuccess_.load(); });

    if (!waitStatus) {
        LOGE("Load CloudSynd SA timeout");
        return nullptr;
    }
    return serviceProxy_;
}

void CloudSyncServiceProxy::ServiceProxyLoadCallback::OnLoadSystemAbilitySuccess(
    int32_t systemAbilityId,
    const sptr<IRemoteObject> &remoteObject)
{
    LOGI("Load CloudSync SA success,systemAbilityId:%{public}d, remoteObj result:%{private}s", systemAbilityId,
         (remoteObject == nullptr ? "false" : "true"));
    unique_lock<mutex> lock(proxyMutex_);
    serviceProxy_ = iface_cast<ICloudSyncService>(remoteObject);
    isLoadSuccess_.store(true);
    proxyConVar_.notify_one();
}

void CloudSyncServiceProxy::ServiceProxyLoadCallback::OnLoadSystemAbilityFail(int32_t systemAbilityId)
{
    LOGI("Load CloudSync SA failed,systemAbilityId:%{public}d", systemAbilityId);
    unique_lock<mutex> lock(proxyMutex_);
    serviceProxy_ = nullptr;
    isLoadSuccess_.store(false);
    proxyConVar_.notify_one();
}

} // namespace OHOS::FileManagement::CloudSync