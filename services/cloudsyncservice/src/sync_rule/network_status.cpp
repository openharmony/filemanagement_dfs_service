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

#include "sync_rule/network_status.h"

#include <cstdint>
#include <unistd.h>

#include "net_conn_client.h"
#include "parameter.h"

#include "dfs_error.h"
#include "sync_rule/net_conn_callback_observer.h"
#include "utils_log.h"

using namespace OHOS::NetManagerStandard;

namespace OHOS::FileManagement::CloudSync {
static constexpr const int32_t MIN_VALID_NETID = 100;

int32_t NetworkStatus::RegisterNetConnCallback()
{
    sptr<NetConnCallbackObserver> observer = new (std::nothrow) NetConnCallbackObserver();
    if (observer == nullptr) {
        LOGE("new operator error.observer is nullptr");
        return E_GET_NETWORK_MANAGER_FAILED;
    }
    int nRet = DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetConnCallback(observer);
    if (nRet != NETMANAGER_SUCCESS) {
        LOGE("RegisterNetConnCallback failed, ret = %{public}d", nRet);
        return E_GET_NETWORK_MANAGER_FAILED;
    }
    return E_OK;
}

int32_t NetworkStatus::GetDefaultNet()
{
    NetHandle netHandle;
    int ret = DelayedSingleton<NetConnClient>::GetInstance()->GetDefaultNet(netHandle);
    if (ret != NETMANAGER_SUCCESS) {
        LOGE("GetDefaultNet failed, ret = %{public}d", ret);
        return E_GET_NETWORK_MANAGER_FAILED;
    }
    if (netHandle.GetNetId() < MIN_VALID_NETID) {
        SetNetConnStatus(NetConnStatus::NO_NETWORK);
        return E_OK;
    }
    NetAllCapabilities netAllCap;
    ret = DelayedSingleton<NetConnClient>::GetInstance()->GetNetCapabilities(netHandle, netAllCap);
    if (ret != NETMANAGER_SUCCESS) {
        LOGE("GetNetCapbilities failed, ret = %{public}d", ret);
        return E_GET_NETWORK_MANAGER_FAILED;
    }
    SetNetConnStatus(netAllCap);
    return E_OK;
}

void NetworkStatus::SetNetConnStatus(NetManagerStandard::NetAllCapabilities &netAllCap)
{
    if (netAllCap.netCaps_.count(NetCap::NET_CAPABILITY_INTERNET)) {
        if (netAllCap.bearerTypes_.count(BEARER_ETHERNET)) {
            LOGI("ethernet connected!");
            SetNetConnStatus(NetConnStatus::ETHERNET_CONNECT);
        } else if (netAllCap.bearerTypes_.count(BEARER_WIFI)) {
            LOGI("wifi connected!");
            SetNetConnStatus(NetConnStatus::WIFI_CONNECT);
        } else if (netAllCap.bearerTypes_.count(BEARER_CELLULAR)) {
            LOGI("celluar connected!");
            SetNetConnStatus(NetConnStatus::CELLULAR_CONNECT);
        }
    } else {
        SetNetConnStatus(NetConnStatus::NO_NETWORK);
    }
}

int32_t NetworkStatus::GetAndRegisterNetwork()
{
    int32_t res = GetDefaultNet();
    if (res != E_OK) {
        return res;
    }

    return RegisterNetConnCallback();
}

void NetworkStatus::InitNetwork()
{
    int status = WaitParameter("startup.service.ctl.netmanager", "2", 4);
    if (status != 0) {
        LOGE(" wait SAMGR error, return value %{public}d.", status);
        return;
    }
    constexpr int RETRY_MAX_TIMES = 2;
    int retryCount = 0;
    constexpr int RETRY_TIME_INTERVAL_MILLISECOND = 1 * 1000 * 1000;
    do {
        if (GetAndRegisterNetwork() == E_OK) {
            break;
        }
        LOGE("wait and retry registering network callback");
        retryCount++;
        usleep(RETRY_TIME_INTERVAL_MILLISECOND);
    } while (retryCount < RETRY_MAX_TIMES);
}

void NetworkStatus::SetNetConnStatus(NetworkStatus::NetConnStatus netStatus)
{
    NetStatus_ = netStatus;
    return;
}

NetworkStatus::NetConnStatus NetworkStatus::GetNetConnStatus()
{
    return NetStatus_;
}

} // namespace OHOS::FileManagement::CloudSync