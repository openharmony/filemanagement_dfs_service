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
#include "iremote_object.h"
#include "net_all_capabilities.h"
#include "data_sync/data_sync_manager.h"

namespace OHOS::FileManagement::CloudSync {
class NetworkStatus {
public:
    enum NetConnStatus {
        NO_NETWORK,
        ETHERNET_CONNECT,
        WIFI_CONNECT,
        CELLULAR_CONNECT,
        NETWORK_AVAIL,
    };
    static int32_t RegisterNetConnCallback(std::shared_ptr<DataSyncManager> dataSyncManager);
    static int32_t GetDefaultNet();
    static int32_t GetAndRegisterNetwork(std::shared_ptr<DataSyncManager> dataSyncManager);
    static void InitNetwork(std::shared_ptr<DataSyncManager> dataSyncManager);
    static void SetNetConnStatus(NetManagerStandard::NetAllCapabilities &netAllCap);
    static void SetNetConnStatus(NetConnStatus netStatus);
    static NetConnStatus GetNetConnStatus();
    static void OnNetworkAvail();

private:
    static inline NetConnStatus netStatus_{NO_NETWORK};
};
} // namespace OHOS::FileManagement::CloudSync

#endif // OHOS_FILEMGMT_CLOUD_SYNC_NETWORK_STATUS_H