/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#ifndef DEVICE_MANAGER_AGENT_H
#define DEVICE_MANAGER_AGENT_H

#include <map>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "device_auth.h"
#include "device_info.h"
#include "device_manager.h"
#include "dfsu_actor.h"
#include "dfsu_singleton.h"
#include "dfsu_startable.h"
#include "i_file_dfs_listener.h"
#include "mountpoint/mount_point.h"
#include "network/network_agent_template.h"
#include "nlohmann/json.hpp"
#include "storage_manager_proxy.h"
#include "utils_directory.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
const int32_t ON_STATUS_OFFLINE = 13900046;
struct GroupInfo {
    std::string groupName;
    std::string groupId;
    std::string groupOwner;
    int32_t groupType;
    GroupInfo() : groupType(0) {}
    GroupInfo(std::string name, std::string id, std::string owner, int32_t type)
        : groupName(name), groupId(id), groupOwner(owner), groupType(type)
    {
    }
};

void from_json(const nlohmann::json &jsonObject, GroupInfo &groupInfo);

class DeviceManagerAgent final : public DistributedHardware::DmInitCallback,
                                 public DistributedHardware::DeviceStateCallback,
                                 public std::enable_shared_from_this<DeviceManagerAgent>,
                                 public DfsuStartable,
                                 public DfsuActor<DeviceManagerAgent>,
                                 public Utils::DfsuSingleton<DeviceManagerAgent> {
    DECLARE_SINGLETON(DeviceManagerAgent);

public:
    enum P2PErrCode:int32_t {
        P2P_SUCCESS,
        P2P_FAILED,
    };
    void Start() override;
    void Stop() override;
    void JoinGroup(std::weak_ptr<MountPoint> mp);
    void QuitGroup(std::weak_ptr<MountPoint> mp);

    void InitDeviceInfos();
    int32_t IsSupportDevice(const DistributedHardware::DmDeviceInfo &deviceInfo);

    void OnDeviceReady(const DistributedHardware::DmDeviceInfo &deviceInfo) override;
    void OnDeviceOffline(const DistributedHardware::DmDeviceInfo &deviceInfo) override;
    void OnDeviceChanged(const DistributedHardware::DmDeviceInfo &deviceInfo) override;
    void OnDeviceOnline(const DistributedHardware::DmDeviceInfo &deviceInfo) override {}

    int32_t OnDeviceP2POnline(const DistributedHardware::DmDeviceInfo &deviceInfo);
    int32_t OnDeviceP2POffline(const DistributedHardware::DmDeviceInfo &deviceInfo);
    int32_t AddRemoteReverseObj(uint32_t callingTokenId, sptr<IFileDfsListener> remoteReverseObj);
    int32_t RemoveRemoteReverseObj(bool clear, uint32_t callingTokenId);
    void NotifyRemoteReverseObj(const std::string &networkId, int32_t status);
    int32_t FindListenerByObject(const wptr<IRemoteObject> &remote, uint32_t &tokenId,
        sptr<IFileDfsListener>& listener);
    std::string GetDeviceIdByNetworkId(const std::string &networkId);
    void MountDfsDocs(const std::string &networkId, const std::string &deviceId);
    int32_t UMountDfsDocs(const std::string &networkId, const std::string &deviceId, bool needClear);
    void AddNetworkId(uint32_t tokenId, const std::string &networkId);
    void RemoveNetworkId(uint32_t tokenId);
    void RemoveNetworkIdByOne(uint32_t tokenId, const std::string &networkId);
    void RemoveNetworkIdForAllToken(const std::string &networkId);
    void ClearNetworkId();
    std::unordered_set<std::string> GetNetworkIds(uint32_t tokenId);

    void OfflineAllDevice();
    void ReconnectOnlineDevices();
    void OnRemoteDied() override;

    DeviceInfo &GetLocalDeviceInfo();
    std::vector<DeviceInfo> GetRemoteDevicesInfo();
    std::mutex appCallConnectMutex_;
    std::unordered_map<uint32_t, sptr<IFileDfsListener>> appCallConnect_;

private:
    void StartInstance() override;
    void StopInstance() override;
    void InitLocalNodeInfo();

    void RegisterToExternalDm();
    void UnregisterFromExternalDm();

    int32_t GetNetworkType(const std::string &cid);
    bool IsWifiNetworkType(int32_t networkType);

    void QueryRelatedGroups(const std::string &udid, const std::string &networkId);
    bool CheckIsAccountless(const GroupInfo &group);
    std::shared_ptr<NetworkAgentTemplate> FindNetworkBaseTrustRelation(bool isAccountless);
    // We use a mutex instead of a shared_mutex to serialize online/offline procedures
    std::mutex mpToNetworksMutex_;
    std::map<uintptr_t, std::shared_ptr<NetworkAgentTemplate>> mpToNetworks_;
    DeviceInfo localDeviceInfo_;

    // cid-->same_account/accoutless's network
    std::unordered_map<std::string, std::shared_ptr<NetworkAgentTemplate>> cidNetTypeRecord_;
    std::unordered_map<std::string, int32_t> cidNetworkType_;
    bool MountDfsCountOnly(const std::string &deviceId);
    bool UMountDfsCountOnly(const std::string &deviceId, bool needClear);
    int32_t GetCurrentUserId();
    void GetStorageManager();
    sptr<StorageManager::IStorageManager> storageMgrProxy_;
    std::unordered_map<std::string, int32_t> mountDfsCount_;
    std::mutex networkIdMapMutex_;
    std::unordered_map<uint32_t, std::unordered_set<std::string>> networkIdMap_;
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // DEVICE_MANAGER_AGENT_H
