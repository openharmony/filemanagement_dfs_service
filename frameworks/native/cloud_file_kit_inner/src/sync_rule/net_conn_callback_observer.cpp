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

#include "net_conn_callback_observer.h"
#include "dfs_error.h"
#include "network_status.h"
#include "utils_log.h"

using namespace OHOS::NetManagerStandard;

namespace OHOS::FileManagement::CloudSync {
NetConnCallbackObserver::NetConnCallbackObserver(std::shared_ptr<CloudFile::DataSyncManager> dataSyncManager)
{
    dataSyncManager_ = dataSyncManager;
}

int32_t NetConnCallbackObserver::NetAvailable(sptr<NetHandle> &netHandle)
{
    LOGI("network is available");
    return E_OK;
}

int32_t NetConnCallbackObserver::NetCapabilitiesChange(sptr<NetHandle> &netHandle,
    const sptr<NetAllCapabilities> &netAllCap)
{
    NetworkStatus::NetConnStatus oldStatus = NetworkStatus::GetNetConnStatus();
    NetworkStatus::SetNetConnStatus(*netAllCap);
    NetworkStatus::NetConnStatus newStatus = NetworkStatus::GetNetConnStatus();
    if (oldStatus == newStatus) {
        LOGI("net status is not change, status is %{public}d", static_cast<int32_t>(newStatus));
        return E_OK;
    }
    if (newStatus == NetworkStatus::WIFI_CONNECT) {
        LOGI("NetCapabilitiesChanged wifi connected");
        dataSyncManager_->TriggerRecoverySync(triggerType_);
        dataSyncManager_->DownloadThumb();
        dataSyncManager_->CacheVideo();
    } else if (newStatus == NetworkStatus::CELLULAR_CONNECT) {
        LOGI("NetCapabilitiesChanged cellular connected");
        dataSyncManager_->TriggerRecoverySync(triggerType_);
        NetworkStatus::NetWorkChangeStopUploadTask();
    } else {
        LOGI("NetCapabilitiesChanged newStatus:%{public}d", newStatus);
    }
    return E_OK;
}

int32_t NetConnCallbackObserver::NetConnectionPropertiesChange(sptr<NetHandle> &netHandle,
    const sptr<NetLinkInfo> &info)
{
    return E_OK;
}

int32_t NetConnCallbackObserver::NetLost(sptr<NetHandle> &netHandle)
{
    LOGI("NetConnCallbackObserver::NetLost");
    NetworkStatus::SetNetConnStatus(NetworkStatus::NetConnStatus::NO_NETWORK);
    return E_OK;
}

int32_t NetConnCallbackObserver::NetUnavailable()
{
    return E_OK;
}

int32_t NetConnCallbackObserver::NetBlockStatusChange(sptr<NetHandle> &netHandle, bool blocked)
{
    return E_OK;
}
} // namespace OHOS::FileManagement::CloudSync