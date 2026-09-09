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

#include <chrono>
#include <cstdint>
#include <future>
#include <thread>
#include <unistd.h>

#include "dfs_error.h"
#include "network_set_manager.h"
#include "net_conn_callback_observer.h"
#include "net_conn_client.h"
#include "parameter.h"
#include "settings_data_manager.h"
#include "utils_log.h"
#include "wifi_device.h"
#include "battery_status.h"
#ifdef ENABLE_WEAK_NETWORK_SYNC
#include "signal_information.h"
#include "cellular_data_client.h"
#include "core_service_client.h"
#endif

using namespace OHOS::NetManagerStandard;

namespace OHOS::FileManagement::CloudSync {
static constexpr const int32_t MIN_VALID_NETID = 100;
static constexpr const int32_t WAIT_NET_SERVICE_TIME = 4;
static constexpr const int32_t WAIT_GET_DEFAULT_NET_TIMEOUT_S = 4;
static constexpr const int32_t DEFAULT_CELLULAR_SIGNAL_STRENGTH = 4;
static const char *NET_MANAGER_ON_STATUS = "2";
static const int32_t SIGNAL_INTENSITY_INVALID = 0;
 
void NetworkStatus::SetCellularSignalStrength(int32_t status)
{
    cellularSignalStrength_.store(status);
}
 
void NetworkStatus::SetWifiSignalStrength(int32_t status)
{
    wifiSignalStrength_.store(status);
}
 
void NetworkStatus::SetCellularAllowSync(bool flag)
{
    cellularAllowSync_.store(flag);
}
 
void NetworkStatus::SetWifiAllowSync(bool flag)
{
    wifiAllowSync_.store(flag);
}
 
void NetworkStatus::SetCellularSignalStopStrength(int32_t status)
{
    LOGI("SetCellularSignalStopStrength status:%{public}d!", status);
    cellularStopSyncSignal_.store(status);
}
 
void NetworkStatus::SetCellularSignalStartStrength(int32_t status)
{
    LOGI("SetCellularSignalStartStrength status:%{public}d!", status);
    cellularStartSyncSignal_.store(status);
}
 
void NetworkStatus::SetWifiSignalStopStrength(int32_t status)
{
    LOGI("SetWifiSignalStopStrength status:%{public}d!", status);
    wifiStopSyncSignal_.store(status);
}
 
void NetworkStatus::SetWifiSignalStartStrength(int32_t status)
{
    LOGI("SetWifiSignalStartStrength status:%{public}d!", status);
    wifiStartSyncSignal_.store(status);
}
 
void NetworkStatus::SetWeakNetworkSyncEnable(bool flag)
{
    weakNetworkSyncEnable_.store(flag);
}
 
bool NetworkStatus::IsAllowSync(SyncTriggerType triggerType)
{
    LOGI("IsAllowSync");
    NetworkStatus::NetConnStatus netStatus = NetworkStatus::GetNetConnStatus();
    if (!weakNetworkSyncEnable_.load()) {
        LOGI("weak network sync disable, no need to verify network signal status!");
        return true;
    }
 
    if (BatteryStatus::IsCharging()) {
        LOGI("is charging, no need to verify network signal status!");
        return true;
    }
 
    if (triggerType == SyncTriggerType::APP_TRIGGER) {
        LOGI("not auto trigger, no need to verify network signal status!");
        return true;
    }
    
    if (netStatus == NetworkStatus::NetConnStatus::CELLULAR_CONNECT) {
        std::lock_guard<std::mutex> lock(netStatusMutex_);
        if (cellularSignalStrength_.load() <= cellularStopSyncSignal_.load()) {
            cellularAllowSync_.store(false);
        } else if (cellularSignalStrength_.load() >= cellularStartSyncSignal_.load()) {
            cellularAllowSync_.store(true);
        }
        LOGI("IsAllowSync = %{public}d, cellularSignalStrength_ = %{public}d", cellularAllowSync_.load(),
            cellularSignalStrength_.load());
        LOGI("cellularStartSyncSignal_ = %{public}d, cellularStopSyncSignal_ = %{public}d",
            cellularStartSyncSignal_.load(), cellularStopSyncSignal_.load());
        return cellularAllowSync_.load();
    }
 
    if (netStatus == NetworkStatus::NetConnStatus::WIFI_CONNECT) {
        std::lock_guard<std::mutex> lock(netStatusMutex_);
        if (wifiSignalStrength_.load() <= wifiStopSyncSignal_.load()) {
            wifiAllowSync_.store(false);
        } else if (wifiSignalStrength_.load() >= wifiStartSyncSignal_.load()) {
            wifiAllowSync_.store(true);
        }
        LOGI("IsAllowSync = %{public}d, wifiSignalStrength_ = %{public}d", wifiAllowSync_.load(),
            wifiSignalStrength_.load());
        LOGI("wifiStartSyncSignal_ = %{public}d, wifiStopSyncSignal_ = %{public}d",
            wifiStartSyncSignal_.load(), wifiStopSyncSignal_.load());
        return wifiAllowSync_.load();
    }

    return true;
}
 
void NetworkStatus::InitDataSyncManager(std::shared_ptr<CloudFile::DataSyncManager> dataSyncManager)
{
    dataSyncManager_ = dataSyncManager;
}
 
int32_t NetworkStatus::GetDefaultWeakNetConfig()
{
    LOGI("GetDefaultNetSignalStrength dataSyncManager_ starts!!");
    if (dataSyncManager_ != nullptr) {
        return dataSyncManager_->GetDefaultWeakNetConfig();
    }
    LOGW("dataSyncManager_ is null, use default thresholds");
    return E_GET_NETWORK_SIGNAL_STRENGTH_FAILED;
}
 
void NetworkStatus::GetDefaultNetSignalStrength()
{
    LOGI("GetDefaultNetSignalStrength starts!!");
    int32_t ret = GetDefaultWeakNetConfig();
    if (ret != E_OK) {
        LOGE("get weak network config failed, ret:%{public}d!", ret);
        return;
    }
 
    if (netStatus_ == NetworkStatus::NetConnStatus::WIFI_CONNECT) {
        int32_t wifiSignalStrength = 0;
        ret = GetWifiSignalStrength(wifiSignalStrength);
        if (ret != E_OK) {
            LOGE("get wifi signal strength failed, ret:%{public}d!", ret);
            return;
        }
        NetworkStatus::SetWifiSignalStrength(wifiSignalStrength);
    }
 
    if (netStatus_ == NetworkStatus::NetConnStatus::CELLULAR_CONNECT) {
        int32_t cellularSignalStrength = 0;
        ret = GetCellularSignalStrength(cellularSignalStrength);
        if (ret != E_OK) {
            LOGE("get cellular signal strength failed, ret:%{public}d!", ret);
            return;
        }
        NetworkStatus::SetCellularSignalStrength(cellularSignalStrength);
    }
}
 
int32_t NetworkStatus::GetWifiSignalStrength(int32_t& wifiSignalStrength)
{
    auto wifiDeviceSharedPtr = Wifi::WifiDevice::GetInstance(WIFI_DEVICE_ABILITY_ID);
    Wifi::WifiDevice* wifiDevicePtr = wifiDeviceSharedPtr.get();
    if (wifiDevicePtr == nullptr) {
        LOGE("wifiDevicePtr is null!!");
        return E_GET_NETWORK_SIGNAL_STRENGTH_FAILED;
    }
    Wifi::WifiLinkedInfo linkedInfo;
    int linkedInfoRet = wifiDevicePtr->GetLinkedInfo(linkedInfo);
    if (linkedInfoRet != E_OK) {
        LOGE("GetWifiSignalStrength GetLinkedInfo failed, ret:%{public}d!!", linkedInfoRet);
        return E_GET_NETWORK_SIGNAL_STRENGTH_FAILED;
    }
    int32_t rssi = linkedInfo.rssi;
    int32_t band = linkedInfo.band;
    linkedInfoRet = wifiDevicePtr->GetSignalLevel(rssi, band, wifiSignalStrength);
    if (linkedInfoRet != E_OK) {
        LOGE("GetWifiSignalStrength GetSignalLevel failed, ret:%{public}d!!", linkedInfoRet);
        return E_GET_NETWORK_SIGNAL_STRENGTH_FAILED;
    }
    LOGI("GetWifiSignalStrength wifiSignalStrength %{public}d!!", wifiSignalStrength);
    return E_OK;
}
 
int32_t NetworkStatus::GetCellularSignalStrength(int32_t& cellularSignalStrength)
{
#ifdef ENABLE_WEAK_NETWORK_SYNC
    int32_t defaultSlotId = Telephony::CellularDataClient::GetInstance().GetDefaultCellularDataSlotId();
    if (defaultSlotId < 0) {
        LOGW("Get default slotId failed, ret = %{public}d.", defaultSlotId);
        return E_GET_NETWORK_SIGNAL_STRENGTH_FAILED;
    }
    std::vector<sptr<Telephony::SignalInformation>> cellInfoList;
    int32_t ret = Telephony::CoreServiceClient::GetInstance().GetSignalInfoList(defaultSlotId, cellInfoList);
    if (ret != E_OK || cellInfoList.empty()) {
        LOGE("GetCellInfoList failed:%{public}d", ret);
        return E_GET_NETWORK_SIGNAL_STRENGTH_FAILED;
    }
 
    auto currentCellInfo = cellInfoList.front();
    if (currentCellInfo == nullptr) {
        LOGE("currentCellInfo is null");
        return E_GET_NETWORK_SIGNAL_STRENGTH_FAILED;
    }
    cellularSignalStrength = currentCellInfo->GetSignalLevel();
    if (cellularSignalStrength == SIGNAL_INTENSITY_INVALID) {
        LOGE("cellular signal Intensity is invalid");
        return E_GET_NETWORK_SIGNAL_STRENGTH_FAILED;
    }
    LOGI("GetCellularSignalStrength signalStrength:%{public}d", cellularSignalStrength);
    return E_OK;
#else
    cellularSignalStrength = DEFAULT_CELLULAR_SIGNAL_STRENGTH;
    return E_OK;
#endif
}

static bool FetchDefaultNetWithTimeout(NetworkStatus::NetConnStatus &out)
{
    std::packaged_task<NetworkStatus::NetConnStatus()> task([] {
        NetworkStatus::NetConnStatus status = NetworkStatus::NO_NETWORK;
        return NetworkStatus::GetDefaultNet(status) == E_OK ? status : NetworkStatus::NETWORK_NOT_INIT;
    });
    auto fut = task.get_future();
    std::thread(std::move(task)).detach();
    if (fut.wait_for(std::chrono::seconds(WAIT_GET_DEFAULT_NET_TIMEOUT_S)) != std::future_status::ready) {
        LOGE("GetDefaultNet timed out after %{public}ds, rely on callback to recover",
             WAIT_GET_DEFAULT_NET_TIMEOUT_S);
        return false;
    }
    out = fut.get();
    return true;
}

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

NetworkStatus::NetConnStatus NetworkStatus::MapCapabilities(NetManagerStandard::NetAllCapabilities &netAllCap)
{
    NetConnStatus newStatus = NetConnStatus::NO_NETWORK;
    if (netAllCap.netCaps_.count(NetCap::NET_CAPABILITY_INTERNET)) {
        if (netAllCap.bearerTypes_.count(BEARER_ETHERNET)) {
            newStatus = NetConnStatus::ETHERNET_CONNECT;
        } else if (netAllCap.bearerTypes_.count(BEARER_WIFI)) {
            newStatus = NetConnStatus::WIFI_CONNECT;
        } else if (netAllCap.bearerTypes_.count(BEARER_CELLULAR)) {
            newStatus = NetConnStatus::CELLULAR_CONNECT;
        }
    }
    return newStatus;
}

int32_t NetworkStatus::GetDefaultNet(NetConnStatus &status)
{
    NetHandle netHandle;
    int ret = NetConnClient::GetInstance().GetDefaultNet(netHandle);
    if (ret != NETMANAGER_SUCCESS) {
        LOGE("GetDefaultNet failed, ret = %{public}d", ret);
        return E_GET_NETWORK_MANAGER_FAILED;
    }
    if (netHandle.GetNetId() < MIN_VALID_NETID) {
        status = NetConnStatus::NO_NETWORK;
        return E_OK;
    }
    NetAllCapabilities netAllCap;
    ret = NetConnClient::GetInstance().GetNetCapabilities(netHandle, netAllCap);
    if (ret != NETMANAGER_SUCCESS) {
        LOGE("GetNetCapbilities failed, ret = %{public}d", ret);
        return E_GET_NETWORK_MANAGER_FAILED;
    }
    status = MapCapabilities(netAllCap);
    return E_OK;
}

NetworkStatus::NetConnStatus NetworkStatus::SetNetConnStatus(NetManagerStandard::NetAllCapabilities &netAllCap)
{
    return SetNetConnStatus(MapCapabilities(netAllCap));
}

int32_t NetworkStatus::GetAndRegisterNetwork(std::shared_ptr<CloudFile::DataSyncManager> dataSyncManager)
{
    NetworkSetManager::InitDataSyncManager(dataSyncManager);
    NetworkStatus::InitDataSyncManager(dataSyncManager);
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

NetworkStatus::NetConnStatus NetworkStatus::SetNetConnStatus(NetworkStatus::NetConnStatus netStatus)
{
    std::lock_guard<std::mutex> lock(netStatusMutex_);
    NetConnStatus oldStatus = netStatus_;
    netStatus_ = netStatus;
    return oldStatus;
}

void NetworkStatus::DoInitialFetch()
{
    {
        std::lock_guard<std::mutex> lock(netStatusMutex_);
        if (netStatus_ != NETWORK_NOT_INIT) {
            GetDefaultNetSignalStrength();
            return;
        }
    }
    NetConnStatus fetched = NETWORK_NOT_INIT;
    if (FetchDefaultNetWithTimeout(fetched)) {
        std::lock_guard<std::mutex> lock(netStatusMutex_);
        if (netStatus_ == NETWORK_NOT_INIT && fetched != NETWORK_NOT_INIT) {
            netStatus_ = fetched;
            LOGI("net status initial: %{public}d", static_cast<int32_t>(netStatus_));
        }

        GetDefaultNetSignalStrength();
    }
}

NetworkStatus::NetConnStatus NetworkStatus::GetNetConnStatus()
{
    // Lazy fetch on first read via call_once; skip if a callback already set it.
    // Default NETWORK_NOT_INIT (not NO_NETWORK) to avoid wrong read at startup.
    std::call_once(initNetStatusOnceFlag_, DoInitialFetch);
    std::lock_guard<std::mutex> lock(netStatusMutex_);
    return netStatus_ == NETWORK_NOT_INIT ? NO_NETWORK : netStatus_;
}

bool NetworkStatus::CheckMobileNetwork(const std::string &bundleName, const int32_t userId)
{
    if (bundleName == GALLERY_BUNDLE_NAME) {
        if (NetworkSetManager::IsAllowCellularConnect(bundleName, userId)) {
            LOGI("datashare status open, CheckMobileNetwork success");
            return true;
        }
    } else if (bundleName == HDC_BUNDLE_NAME) {
        if (SettingsDataManager::GetMobileDataStatus()) {
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
    if (bundleName == GALLERY_BUNDLE_NAME || bundleName == HDC_BUNDLE_NAME) {
        if (SettingsDataManager::GetNetworkConnectionStatus()) {
            LOGI("settingData is set network connection");
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
    NetConnStatus status = GetNetConnStatus();
    if (status == WIFI_CONNECT) {
        LOGI("datashare status close, network_status:wifi");
        return true;
    }
    if (status == ETHERNET_CONNECT) {
        LOGI("datashare status close, network_status:ethernet");
        return true;
    }
    LOGI("CheckWifiOrEthernet off");
    return false;
}
} // namespace OHOS::FileManagement::CloudSync