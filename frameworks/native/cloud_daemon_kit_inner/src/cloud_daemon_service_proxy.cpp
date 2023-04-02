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
#include "cloud_daemon_service_proxy.h"

#include <sstream>

#include "dfs_error.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudFile {
using namespace std;
using namespace OHOS::FileManagement;

constexpr int LOAD_SA_TIMEOUT_MS = 4000;

int32_t CloudDaemonServiceProxy::StartFuse(int32_t devFd, const string &path)
{
    LOGI("Start fuse");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("Failed to write interface token");
        return E_BROKEN_IPC;
    }

    if (!data.WriteFileDescriptor(devFd)) {
        LOGE("Failed to send the device file fd");
        return E_INVAL_ARG;
    }

    if (!data.WriteString(path)) {
        LOGE("Failed to send the device file path");
        return E_INVAL_ARG;
    }

    auto remote = Remote();
    if (!remote) {
        LOGE("remote is nullptr");
        return E_BROKEN_IPC;
    }
    int32_t ret = remote->SendRequest(ICloudDaemon::CLOUD_DAEMON_CMD_START_FUSE, data, reply, option);
    if (ret != E_OK) {
        stringstream ss;
        ss << "Failed to send out the requeset, errno:" << ret;
        LOGE("%{public}s", ss.str().c_str());
        return E_BROKEN_IPC;
    }
    LOGI("StartFuseInner Success");
    return reply.ReadInt32();
}

sptr<ICloudDaemon> CloudDaemonServiceProxy::GetInstance()
{
    LOGI("getinstance");
    unique_lock<mutex> lock(proxyMutex_);
    if (serviceProxy_ != nullptr) {
        return serviceProxy_;
    }

    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        LOGE("Samgr is nullptr");
        return nullptr;
    }
    sptr<ServiceProxyLoadCallback> cloudDaemonLoadCallback = new ServiceProxyLoadCallback();
    if (cloudDaemonLoadCallback == nullptr) {
        LOGE("cloudDaemonLoadCallback is nullptr");
        return nullptr;
    }
    int32_t ret = samgr->LoadSystemAbility(FILEMANAGEMENT_CLOUD_DAEMON_SERVICE_SA_ID, cloudDaemonLoadCallback);
    if (ret != E_OK) {
        LOGE("Failed to Load systemAbility, systemAbilityId:%{pulbic}d, ret code:%{pulbic}d",
             FILEMANAGEMENT_CLOUD_DAEMON_SERVICE_SA_ID, ret);
        return nullptr;
    }

    auto waitStatus = cloudDaemonLoadCallback->proxyConVar_.wait_for(
        lock, std::chrono::milliseconds(LOAD_SA_TIMEOUT_MS),
        [cloudDaemonLoadCallback]() { return cloudDaemonLoadCallback->isLoadSuccess_.load(); });
    if (!waitStatus) {
        LOGE("Load CloudDaemon SA timeout");
        return nullptr;
    }
    return serviceProxy_;
}

void CloudDaemonServiceProxy::InvaildInstance()
{
    LOGI("invalid instance");
    unique_lock<mutex> lock(proxyMutex_);
    serviceProxy_ = nullptr;
}

void CloudDaemonServiceProxy::ServiceProxyLoadCallback::OnLoadSystemAbilitySuccess(
    int32_t systemAbilityId,
    const sptr<IRemoteObject> &remoteObject)
{
    LOGI("Load CloudDaemon SA success,systemAbilityId:%{public}d, remoteObj result:%{private}s", systemAbilityId,
         (remoteObject == nullptr ? "false" : "true"));
    unique_lock<mutex> lock(proxyMutex_);
    serviceProxy_ = iface_cast<ICloudDaemon>(remoteObject);
    isLoadSuccess_.store(true);
    proxyConVar_.notify_one();
}

void CloudDaemonServiceProxy::ServiceProxyLoadCallback::OnLoadSystemAbilityFail(int32_t systemAbilityId)
{
    LOGI("Load CloudDaemon SA failed,systemAbilityId:%{public}d", systemAbilityId);
    unique_lock<mutex> lock(proxyMutex_);
    serviceProxy_ = nullptr;
    isLoadSuccess_.store(false);
    proxyConVar_.notify_one();
}

} // namespace OHOS::FileManagement::CloudFile
