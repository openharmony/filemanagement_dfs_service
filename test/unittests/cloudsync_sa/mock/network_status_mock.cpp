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
#include "dfs_error.h"

namespace OHOS::FileManagement::CloudSync {

int32_t NetworkStatus::RegisterNetConnCallback(std::shared_ptr<DataSyncManager> dataSyncManager)
{
    return E_OK;
}

int32_t NetworkStatus::GetDefaultNet()
{
    return E_OK;
}

void NetworkStatus::SetNetConnStatus(NetManagerStandard::NetAllCapabilities &netAllCap)
{
}

int32_t NetworkStatus::GetAndRegisterNetwork(std::shared_ptr<DataSyncManager> dataSyncManager)
{
    int32_t res = GetDefaultNet();
    if (res != E_OK) {
        return res;
    }

    return RegisterNetConnCallback(dataSyncManager);
}

void NetworkStatus::InitNetwork(std::shared_ptr<DataSyncManager> dataSyncManager)
{
}

void NetworkStatus::SetNetConnStatus(NetworkStatus::NetConnStatus netStatus)
{
    return;
}

NetworkStatus::NetConnStatus NetworkStatus::GetNetConnStatus()
{
    return NetworkStatus::NetConnStatus::WIFI_CONNECT;
}
} // namespace OHOS::FileManagement::CloudSync