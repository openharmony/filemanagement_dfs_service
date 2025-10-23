/*
* Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "device_manager_agent_mock.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
using namespace std;
DeviceManagerAgent::DeviceManagerAgent() : DfsuActor<DeviceManagerAgent>(this,
    std::numeric_limits<uint32_t>::max()) {}

DeviceManagerAgent::~DeviceManagerAgent() {}

void DeviceManagerAgent::StartInstance()
{
    return;
}

void DeviceManagerAgent::StopInstance()
{
    return;
}

void DeviceManagerAgent::Start()
{
    return;
}

void DeviceManagerAgent::Stop()
{
    return;
}

void DeviceManagerAgent::JoinGroup(weak_ptr<MountPoint> mp)
{
    return;
}

void DeviceManagerAgent::QuitGroup(shared_ptr<MountPoint> mp)
{
    return;
}

void DeviceManagerAgent::OfflineAllDevice()
{
    return;
}

std::shared_ptr<NetworkAgentTemplate> DeviceManagerAgent::FindNetworkBaseTrustRelation(bool isAccountless)
{
    return nullptr;
}

int32_t DeviceManagerAgent::GetNetworkType(const string &cid)
{
    return 0;
}

void DeviceManagerAgent::OnDeviceOffline(const DistributedHardware::DmDeviceInfo &deviceInfo)
{
    return;
}

int32_t DeviceManagerAgent::OnDeviceP2POnline(const DistributedHardware::DmDeviceInfo &deviceInfo)
{
    if (IDeviceManagerAgentMock::iDeviceManagerAgentMock_ == nullptr) {
        return -1;
    }
    return IDeviceManagerAgentMock::iDeviceManagerAgentMock_->OnDeviceP2POnline(deviceInfo);
}

int32_t DeviceManagerAgent::OnDeviceP2POffline(const DistributedHardware::DmDeviceInfo &deviceInfo)
{
    if (IDeviceManagerAgentMock::iDeviceManagerAgentMock_ == nullptr) {
        return -1;
    }
    return IDeviceManagerAgentMock::iDeviceManagerAgentMock_->OnDeviceP2POffline(deviceInfo);
}

bool DeviceManagerAgent::MountDfsCountOnly(const std::string &deviceId)
{
    return true;
}

bool DeviceManagerAgent::UMountDfsCountOnly(const std::string &deviceId, bool needClear)
{
    return true;
}

int32_t DeviceManagerAgent::GetCurrentUserId()
{
    return 0;
}

sptr<StorageManager::IStorageManager> DeviceManagerAgent::GetStorageManager()
{
    return nullptr;
}

int32_t DeviceManagerAgent::MountDfsDocs(const std::string &networkId,
    const std::string &deviceId, const uint32_t callingTokenId)
{
    if (IDeviceManagerAgentMock::iDeviceManagerAgentMock_ == nullptr) {
        return -1;
    }
    return IDeviceManagerAgentMock::iDeviceManagerAgentMock_->MountDfsDocs(networkId, deviceId, callingTokenId);
}

int32_t DeviceManagerAgent::UMountDfsDocs(const std::string &networkId, const std::string &deviceId, bool needClear)
{
    if (IDeviceManagerAgentMock::iDeviceManagerAgentMock_ == nullptr) {
        return -1;
    }
    return IDeviceManagerAgentMock::iDeviceManagerAgentMock_->UMountDfsDocs(networkId, deviceId, needClear);
}

void DeviceManagerAgent::IncreaseMountDfsCount(const std::string &networkId,
    const std::string &mountPath, const uint32_t callingTokenId)
{
    return;
}

void DeviceManagerAgent::RemoveMountDfsCount(const std::string &mountPath)
{
    return;
}

std::string DeviceManagerAgent::GetDeviceIdByNetworkId(const std::string &networkId)
{
    if (IDeviceManagerAgentMock::iDeviceManagerAgentMock_ == nullptr) {
        return "";
    }
    return IDeviceManagerAgentMock::iDeviceManagerAgentMock_->GetDeviceIdByNetworkId(networkId);
}

void from_json(const nlohmann::json &jsonObject, GroupInfo &groupInfo)
{
    return;
}

void DeviceManagerAgent::QueryRelatedGroups(const std::string &udid, const std::string &networkId)
{
    return;
}

void DeviceManagerAgent::InitDeviceInfos()
{
    return;
}

int32_t DeviceManagerAgent::IsSupportedDevice(const DistributedHardware::DmDeviceInfo &deviceInfo)
{
    if (IDeviceManagerAgentMock::iDeviceManagerAgentMock_ == nullptr) {
        return -1;
    }
    return IDeviceManagerAgentMock::iDeviceManagerAgentMock_->IsSupportedDevice(deviceInfo);
}

void DeviceManagerAgent::InitLocalNodeInfo()
{
    return;
}

void DeviceManagerAgent::OnRemoteDied()
{
    return;
}

DeviceInfo &DeviceManagerAgent::GetLocalDeviceInfo()
{
    if (IDeviceManagerAgentMock::iDeviceManagerAgentMock_ == nullptr) {
        return localDeviceInfo_;
    }
    return IDeviceManagerAgentMock::iDeviceManagerAgentMock_->GetLocalDeviceInfo();
}

void DeviceManagerAgent::RegisterToExternalDm()
{
    return;
}

void DeviceManagerAgent::UnregisterFromExternalDm()
{
    return;
}

std::unordered_map<std::string, MountCountInfo> DeviceManagerAgent::GetAllMountInfo()
{
    return IDeviceManagerAgentMock::iDeviceManagerAgentMock_->GetAllMountInfo();
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS