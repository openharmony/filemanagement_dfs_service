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
#include "dfs_error.h"

namespace OHOS::FileManagement::CloudSync {
using namespace CloudFile;

int32_t NetworkStatus::RegisterNetConnCallback(std::shared_ptr<DataSyncManager> dataSyncManager)
{
    return E_OK;
}

int32_t NetworkStatus::GetDefaultNet(NetConnStatus &status)
{
    status = NetConnStatus::WIFI_CONNECT;
    return E_OK;
}

NetworkStatus::NetConnStatus NetworkStatus::SetNetConnStatus(NetManagerStandard::NetAllCapabilities &netAllCap)
{
    return NetworkStatus::NetConnStatus::NO_NETWORK;
}

int32_t NetworkStatus::GetAndRegisterNetwork(std::shared_ptr<DataSyncManager> dataSyncManager)
{
    NetConnStatus status;
    int32_t res = GetDefaultNet(status);
    if (res != E_OK) {
        return res;
    }

    return RegisterNetConnCallback(dataSyncManager);
}

void NetworkStatus::InitNetwork(std::shared_ptr<DataSyncManager> dataSyncManager)
{
}

NetworkStatus::NetConnStatus NetworkStatus::SetNetConnStatus(NetworkStatus::NetConnStatus netStatus)
{
    return NetworkStatus::NetConnStatus::NO_NETWORK;
}

bool NetworkStatus::CheckMobileNetwork(const std::string &bundleName, const int32_t userId)
{
    return true;
}

bool NetworkStatus::CheckNetwork(const std::string &bundleName, const int32_t userId)
{
    return true;
}

NetworkStatus::NetConnStatus NetworkStatus::GetNetConnStatus()
{
    return NetworkStatus::NetConnStatus::WIFI_CONNECT;
}

void NetworkStatus::NetWorkChangeStopUploadTask()
{
}
} // namespace OHOS::FileManagement::CloudSync