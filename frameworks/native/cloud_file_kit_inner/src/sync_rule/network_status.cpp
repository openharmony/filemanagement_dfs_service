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

#include "network_status.h"

#include <cstdint>
#include <unistd.h>

#include "net_conn_client.h"
#include "parameter.h"

#include "dfs_error.h"
#include "network_set_manager.h"
#include "net_conn_callback_observer.h"
#include "utils_log.h"
#include "settings_data_manager.h"

using namespace OHOS::NetManagerStandard;

namespace OHOS::FileManagement::CloudSync {
static constexpr const int32_t MIN_VALID_NETID = 100;
static constexpr const int32_t WAIT_NET_SERVICE_TIME = 4;
static const char *NET_MANAGER_ON_STATUS = "2";

int32_t NetworkStatus::RegisterNetConnCallback(std::shared_ptr<CloudFile::DataSyncManager> dataSyncManager)
{
    sptr<NetConnCallbackObserver> observer(new (std::nothrow) NetConnCallbackObserver(dataSyncManager));
    if (observer == nullptr) {
        LOGE("new operator error.observer is nullptr");
        return E_GET_NETWORK_MANAGER_FAILED;
    }
    int nRet = NetConnClient::GetInstance().RegisterNetConnCallback(observer);
    if (nRet != NETMANAGER_SUCCESS) {
        LOGE("RegisterNetConnCallback failed, ret = %{public}d", nRet);
        return E_GET_NETWORK_MANAGER_FAILED;
    }
    return E_OK;
}

int32_t NetworkStatus::GetDefaultNet()
{
    NetHandle netHandle;
    int ret = NetConnClient::GetInstance().GetDefaultNet(netHandle);
    if (ret != NETMANAGER_SUCCESS) {
        LOGE("GetDefaultNet failed, ret = %{public}d", ret);
        return E_GET_NETWORK_MANAGER_FAILED;
    }
    if (netHandle.GetNetId() < MIN_VALID_NETID) {
        SetNetConnStatus(NetConnStatus::NO_NETWORK);
        return E_OK;
    }
    NetAllCapabilities netAllCap;
    ret = NetConnClient::GetInstance().GetNetCapabilities(netHandle, netAllCap);
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
            SetNetConnStatus(NetConnStatus::ETHERNET_CONNECT);
        } else if (netAllCap.bearerTypes_.count(BEARER_WIFI)) {
            SetNetConnStatus(NetConnStatus::WIFI_CONNECT);
        } else if (netAllCap.bearerTypes_.count(BEARER_CELLULAR)) {
            SetNetConnStatus(NetConnStatus::CELLULAR_CONNECT);
        }
    } else {
        SetNetConnStatus(NetConnStatus::NO_NETWORK);
    }
}

int32_t NetworkStatus::GetAndRegisterNetwork(std::shared_ptr<CloudFile::DataSyncManager> dataSyncManager)
{
    int32_t res = GetDefaultNet();
    if (res != E_OK) {
        return res;
    }

    NetworkSetManager::InitDataSyncManager(dataSyncManager);
    return RegisterNetConnCallback(dataSyncManager);
}

void NetworkStatus::NetWorkChangeStopUploadTask()
{
    NetworkSetManager::NetWorkChangeStopUploadTask();
}

void NetworkStatus::InitNetwork(std::shared_ptr<CloudFile::DataSyncManager> dataSyncManager)
{
    int status = WaitParameter("startup.service.ctl.netmanager", NET_MANAGER_ON_STATUS, WAIT_NET_SERVICE_TIME);
    if (status != 0) {
        LOGE(" wait SAMGR error, return value %{public}d.", status);
        return;
    }
    constexpr int RETRY_MAX_TIMES = 2;
    int retryCount = 0;
    constexpr int RETRY_TIME_INTERVAL_MILLISECOND = 1 * 1000 * 1000;
    do {
        if (GetAndRegisterNetwork(dataSyncManager) == E_OK) {
            break;
        }
        LOGE("wait and retry registering network callback");
        retryCount++;
        usleep(RETRY_TIME_INTERVAL_MILLISECOND);
    } while (retryCount < RETRY_MAX_TIMES);
}

void NetworkStatus::SetNetConnStatus(NetworkStatus::NetConnStatus netStatus)
{
    netStatus_ = netStatus;
    NetworkSetManager::SetNetConnStatus(static_cast<NetworkSetManager::NetConnStatus>(netStatus));
    return;
}

NetworkStatus::NetConnStatus NetworkStatus::GetNetConnStatus()
{
    return netStatus_;
}

bool NetworkStatus::CheckMobileNetwork(const std::string &bundleName, const int32_t userId)
{
    if (bundleName == GALLERY_BUNDLE_NAME) {
        if (NetworkSetManager::IsAllowCellularConnect(bundleName, userId)) {
            LOGI("datashare status open, CheckMobileNetwork success");
            return true;
        }
    } else if (bundleName == HDC_BUNDLE_NAME) {
        if (SettingsDataManager::GetMobileDataStatus() == "on") {
            LOGI("ailife is setting mobie data sync");
            return true;
        }
    } else {
        return true;
    }
    return CheckWifiOrEthernet();
}

bool NetworkStatus::CheckNetwork(const std::string &bundleName, const int32_t userId)
{
    if (bundleName == GALLERY_BUNDLE_NAME) {
        if (NetworkSetManager::IsAllowNetConnect(bundleName, userId)) {
            LOGI("CheckNetwork on");
            return true;
        }
    } else if (bundleName == HDC_BUNDLE_NAME) {
        if (SettingsDataManager::GetNetworkConnectionStatus() == "on") {
            LOGI("ailife is setting network connection status");
            return true;
        }
    } else {
        return true;
    }
    LOGI("CheckNetwork off");
    return false;
}

bool NetworkStatus::CheckWifiOrEthernet()
{
    if (netStatus_ == WIFI_CONNECT) {
        LOGI("datashare status close, network_status:wifi");
        return true;
    }
    if (netStatus_ == ETHERNET_CONNECT) {
        LOGI("datashare status close, network_status:ethernet");
        return true;
    }
    LOGI("CheckWifiOrEthernet off");
    return false;
}
} // namespace OHOS::FileManagement::CloudSync