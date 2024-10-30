/*
* Copyright (c) 2024 Huawei Device Co., Ltd.
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

void DeviceManagerAgent::QuitGroup(weak_ptr<MountPoint> mp)
{
    return;
}

void DeviceManagerAgent::OfflineAllDevice()
{
    return;
}

void DeviceManagerAgent::ReconnectOnlineDevices()
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

bool DeviceManagerAgent::IsWifiNetworkType(int32_t networkType)
{
    return true;
}

void DeviceManagerAgent::OnDeviceReady(const DistributedHardware::DmDeviceInfo &deviceInfo)
{
    return;
}

void DeviceManagerAgent::OnDeviceOffline(const DistributedHardware::DmDeviceInfo &deviceInfo)
{
    return;
}

void DeviceManagerAgent::ClearCount(const DistributedHardware::DmDeviceInfo &deviceInfo)
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

void DeviceManagerAgent::GetStorageManager()
{
    return;
}

void DeviceManagerAgent::AddNetworkId(uint32_t tokenId, const std::string &networkId)
{
    return;
}

void DeviceManagerAgent::RemoveNetworkId(uint32_t tokenId)
{
    return;
}

void DeviceManagerAgent::RemoveNetworkIdByOne(uint32_t tokenId, const std::string &networkId)
{
    return;
}

void DeviceManagerAgent::RemoveNetworkIdForAllToken(const std::string &networkId)
{
    return;
}

void DeviceManagerAgent::ClearNetworkId()
{
    return;
}

std::unordered_set<std::string> DeviceManagerAgent::GetNetworkIds(uint32_t tokenId)
{
    if (IDeviceManagerAgentMock::iDeviceManagerAgentMock_ == nullptr) {
        return {};
    }
    return IDeviceManagerAgentMock::iDeviceManagerAgentMock_->GetNetworkIds(tokenId);
}

int32_t DeviceManagerAgent::MountDfsDocs(const std::string &networkId, const std::string &deviceId)
{
    if (IDeviceManagerAgentMock::iDeviceManagerAgentMock_ == nullptr) {
        return -1;
    }
    return IDeviceManagerAgentMock::iDeviceManagerAgentMock_->MountDfsDocs(networkId, deviceId);
}

int32_t DeviceManagerAgent::UMountDfsDocs(const std::string &networkId, const std::string &deviceId, bool needClear)
{
    if (IDeviceManagerAgentMock::iDeviceManagerAgentMock_ == nullptr) {
        return -1;
    }
    return IDeviceManagerAgentMock::iDeviceManagerAgentMock_->UMountDfsDocs(networkId, deviceId, needClear);
}

void DeviceManagerAgent::IncreaseMountDfsCount(const std::string &deviceId)
{
    return;
}

void DeviceManagerAgent::RemoveMountDfsCount(const std::string &deviceId)
{
    return;
}

void DeviceManagerAgent::NotifyRemoteReverseObj(const std::string &networkId, int32_t status)
{
    return;
}

int32_t DeviceManagerAgent::AddRemoteReverseObj(uint32_t callingTokenId, sptr<IFileDfsListener> remoteReverseObj)
{
    if (IDeviceManagerAgentMock::iDeviceManagerAgentMock_ == nullptr) {
        return -1;
    }
    return IDeviceManagerAgentMock::iDeviceManagerAgentMock_->AddRemoteReverseObj(callingTokenId, remoteReverseObj);
}

int32_t DeviceManagerAgent::RemoveRemoteReverseObj(bool clear, uint32_t callingTokenId)
{
    if (IDeviceManagerAgentMock::iDeviceManagerAgentMock_ == nullptr) {
        return -1;
    }
    return IDeviceManagerAgentMock::iDeviceManagerAgentMock_->RemoveRemoteReverseObj(clear, callingTokenId);
}

int32_t DeviceManagerAgent::FindListenerByObject(const wptr<IRemoteObject> &remote,
                                                 uint32_t &tokenId, sptr<IFileDfsListener>& listener)
{
    if (IDeviceManagerAgentMock::iDeviceManagerAgentMock_ == nullptr) {
        return -1;
    }
    auto ret = IDeviceManagerAgentMock::iDeviceManagerAgentMock_->FindListenerByObject(remote, tokenId, listener);
    switch (ret) {
        case 1:
            tokenId = 0;
            listener = nullptr;
            break;
        case 2:
            tokenId = 1;
            listener = nullptr;
            break;
        case 3:
            tokenId = 0;
            listener = IDeviceManagerAgentMock::iDeviceManagerAgentMock_->GetDfsListener();
            break;
        case 4:
            tokenId = 1;
            listener = IDeviceManagerAgentMock::iDeviceManagerAgentMock_->GetDfsListener();
            break;
        default:
            break;
    }
    return 0;
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

bool DeviceManagerAgent::CheckIsAccountless(const GroupInfo &group)
{
    return true;
}

void DeviceManagerAgent::OnDeviceChanged(const DistributedHardware::DmDeviceInfo &deviceInfo)
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
    DeviceInfo deviceInfo;
    if (IDeviceManagerAgentMock::iDeviceManagerAgentMock_ == nullptr) {
        return deviceInfo;
    }
    return IDeviceManagerAgentMock::iDeviceManagerAgentMock_->GetLocalDeviceInfo();
}

vector<DeviceInfo> DeviceManagerAgent::GetRemoteDevicesInfo()
{
    if (IDeviceManagerAgentMock::iDeviceManagerAgentMock_ == nullptr) {
        return {};
    }
    return IDeviceManagerAgentMock::iDeviceManagerAgentMock_->GetRemoteDevicesInfo();
}

void DeviceManagerAgent::RegisterToExternalDm()
{
    return;
}

void DeviceManagerAgent::UnregisterFromExternalDm()
{
    return;
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS