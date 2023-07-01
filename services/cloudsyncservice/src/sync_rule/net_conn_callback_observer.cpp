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

#include "sync_rule/net_conn_callback_observer.h"
#include "dfs_error.h"
#include "sync_rule/network_status.h"
#include "utils_log.h"

using namespace OHOS::NetManagerStandard;

namespace OHOS::FileManagement::CloudSync {
NetConnCallbackObserver::NetConnCallbackObserver(std::shared_ptr<DataSyncManager> dataSyncManager)
{
    dataSyncManager_ = dataSyncManager;
}

int32_t NetConnCallbackObserver::NetAvailable(sptr<NetHandle> &netHandle)
{
    LOGI("network is available");
    NetworkStatus::OnNetworkAvail();
    dataSyncManager_->TriggerRecoverySync(triggerType_);
    return E_OK;
}

int32_t NetConnCallbackObserver::NetCapabilitiesChange(sptr<NetHandle> &netHandle,
    const sptr<NetAllCapabilities> &netAllCap)
{
    NetworkStatus::SetNetConnStatus(*netAllCap);
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