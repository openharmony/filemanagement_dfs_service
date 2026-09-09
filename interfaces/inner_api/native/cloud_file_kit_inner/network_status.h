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

#ifndef OHOS_FILEMGMT_CLOUD_SYNC_NETWORK_STATUS_H
#define OHOS_FILEMGMT_CLOUD_SYNC_NETWORK_STATUS_H
#include <cstdint>
#include <mutex>
#include "iremote_object.h"
#include "net_all_capabilities.h"
#include "data_sync_manager.h"

namespace OHOS::FileManagement::CloudSync {
class NetworkStatus {
public:
    enum NetConnStatus {
        NO_NETWORK,
        ETHERNET_CONNECT,
        WIFI_CONNECT,
        CELLULAR_CONNECT,
        NETWORK_AVAIL,
        NETWORK_NOT_INIT,  // internal only; GetNetConnStatus never returns it
    };
    static int32_t RegisterNetConnCallback(std::shared_ptr<CloudFile::DataSyncManager> dataSyncManager);
    static int32_t GetDefaultNet(NetConnStatus &status);
    static int32_t GetAndRegisterNetwork(std::shared_ptr<CloudFile::DataSyncManager> dataSyncManager);
    static void InitNetwork(std::shared_ptr<CloudFile::DataSyncManager> dataSyncManager);
    static NetConnStatus SetNetConnStatus(NetManagerStandard::NetAllCapabilities &netAllCap);
    static NetConnStatus SetNetConnStatus(NetConnStatus netStatus);
    static NetConnStatus GetNetConnStatus();
    static void OnNetworkAvail();
    static bool CheckMobileNetwork(const std::string &bundleName, const int32_t userId);
    static bool CheckNetwork(const std::string &bundleName, const int32_t userId);
    static void NetWorkChangeStopUploadTask();
    static bool CheckWifiOrEthernet();
    static void SetCellularSignalStrength(int32_t status);
    static void SetWifiSignalStrength(int32_t status);
    static void SetCellularSignalStopStrength(int32_t status);
    static void SetCellularSignalStartStrength(int32_t status);
    static void SetWifiSignalStopStrength(int32_t status);
    static void SetWifiSignalStartStrength(int32_t status);
    static void SetWeakNetworkSyncEnable(bool flag);
    static int32_t GetDefaultWeakNetConfig();
    static void SetCellularAllowSync(bool flag);
    static void SetWifiAllowSync(bool flag);
    static bool IsAllowSync(SyncTriggerType triggerType);
    static void GetDefaultNetSignalStrength();
    static int32_t GetWifiSignalStrength(int32_t& wifiSignalStrength);
    static int32_t GetCellularSignalStrength(int32_t& cellularSignalStrength);
    static void InitDataSyncManager(std::shared_ptr<CloudFile::DataSyncManager> dataSyncManager);

private:
    static void DoInitialFetch();
    static NetConnStatus MapCapabilities(NetManagerStandard::NetAllCapabilities &netAllCap);
    static inline std::mutex netStatusMutex_;
    static inline std::shared_ptr<CloudFile::DataSyncManager> dataSyncManager_;
    static inline std::atomic<int32_t> cellularSignalStrength_{4};
    static inline std::atomic<int32_t> wifiSignalStrength_{3};
    static inline std::atomic<int32_t> cellularStopSyncSignal_{3};
    static inline std::atomic<int32_t> cellularStartSyncSignal_{5};
    static inline std::atomic<int32_t> wifiStopSyncSignal_{2};
    static inline std::atomic<int32_t> wifiStartSyncSignal_{4};
    static inline std::atomic<bool> cellularAllowSync_{true};
    static inline std::atomic<bool> wifiAllowSync_{true};
    static inline std::atomic<bool> weakNetworkSyncEnable_{true};
    static inline NetConnStatus netStatus_{NETWORK_NOT_INIT};
    static inline std::once_flag initNetStatusOnceFlag_;
};
} // namespace OHOS::FileManagement::CloudSync

#endif // OHOS_FILEMGMT_CLOUD_SYNC_NETWORK_STATUS_H