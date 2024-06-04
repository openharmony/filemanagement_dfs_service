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

#include "device/device_manager_agent.h"

#include <limits>
#include <sstream>
#include <string>
#include <unordered_set>

#include "device_auth.h"
#include "dfs_error.h"
#include "dfsu_exception.h"
#include "distributed_device_profile_client.h"
#include "ipc/i_daemon.h"
#include "iservice_registry.h"
#include "iremote_object.h"
#include "mountpoint/mount_manager.h"
#include "network/devsl_dispatcher.h"
#include "network/softbus/softbus_agent.h"
#include "os_account_manager.h"
#include "parameters.h"
#include "softbus_bus_center.h"
#include "system_ability_definition.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace {
constexpr int32_t DEVICE_OS_TYPE_OH = 10;
constexpr int MAX_RETRY_COUNT = 7;
constexpr int PEER_TO_PEER_GROUP = 256;
constexpr int ACROSS_ACCOUNT_AUTHORIZE_GROUP = 1282;
const int32_t MOUNT_DFS_COUNT_ONE = 1;
const uint32_t MAX_ONLINE_DEVICE_SIZE = 10000;
const int32_t INVALID_USER_ID = -1;
const std::string SAME_ACCOUNT_MARK = "const.distributed_file_only_for_same_account_test";
} // namespace
using namespace std;

DeviceManagerAgent::DeviceManagerAgent() : DfsuActor<DeviceManagerAgent>(this, std::numeric_limits<uint32_t>::max()) {}

DeviceManagerAgent::~DeviceManagerAgent()
{
    try {
        StopInstance();
    } catch (const DfsuException &e) {
        // do not throw exception
    } catch (const std::exception &e) {
        // do not throw exception
    }
}

void DeviceManagerAgent::StartInstance()
{
    StartActor();
}

void DeviceManagerAgent::StopInstance()
{
    StopActor();
}

void DeviceManagerAgent::Start()
{
    DevslDispatcher::Start();
    RegisterToExternalDm();
    InitLocalNodeInfo();
    InitDeviceAuthService();
}

void DeviceManagerAgent::Stop()
{
    DestroyDeviceAuthService();
    UnregisterFromExternalDm();
    DevslDispatcher::Stop();
}

void DeviceManagerAgent::JoinGroup(weak_ptr<MountPoint> mp)
{
    LOGI("join group begin");
    auto smp = mp.lock();
    if (!smp) {
        stringstream ss("Failed to join group: Received empty mountpoint");
        LOGE("%{public}s", ss.str().c_str());
        throw runtime_error(ss.str());
    }

    shared_ptr<SoftbusAgent> agent = nullptr;
    {
        unique_lock<mutex> lock(mpToNetworksMutex_);
        agent = make_shared<SoftbusAgent>(mp);
        auto [ignored, inserted] = mpToNetworks_.insert({ smp->GetID(), agent });
        if (!inserted) {
            stringstream ss;
            ss << "Failed to join group: Mountpoint existed" << smp->ToString();
            throw runtime_error(ss.str());
        }
    }
    agent->StartActor();
    LOGI("join group end, id : %{public}d, account : %{public}s", smp->GetID(), smp->isAccountLess() ? "no" : "yes");
}

void DeviceManagerAgent::QuitGroup(weak_ptr<MountPoint> mp)
{
    LOGI("quit group begin");
    OfflineAllDevice();

    auto smp = mp.lock();
    if (!smp) {
        stringstream ss("Failed to quit group: Received empty mountpoint");
        LOGE("%{public}s", ss.str().c_str());
        throw runtime_error(ss.str());
    }

    unique_lock<mutex> lock(mpToNetworksMutex_);
    auto it = mpToNetworks_.find(smp->GetID());
    if (it == mpToNetworks_.end()) {
        stringstream ss;
        ss << "Failed to quit group: Mountpoint didn't exist " << smp->ToString();
        throw runtime_error(ss.str());
    }

    it->second->StopActor();
    mpToNetworks_.erase(smp->GetID());
    LOGI("quit group end, id : %{public}d, account : %{public}s", smp->GetID(), smp->isAccountLess() ? "no" : "yes");
}

void DeviceManagerAgent::OfflineAllDevice()
{
    unique_lock<mutex> lock(mpToNetworksMutex_);
    for (auto [ignore, net] : cidNetTypeRecord_) {
        if (net == nullptr) {
            continue;
        }
        auto cmd = make_unique<DfsuCmd<NetworkAgentTemplate>>(&NetworkAgentTemplate::DisconnectAllDevices);
        net->Recv(move(cmd));
    }
}

void DeviceManagerAgent::ReconnectOnlineDevices()
{
    unique_lock<mutex> lock(mpToNetworksMutex_);
    for (auto [ignore, net] : cidNetTypeRecord_) {
        if (net == nullptr) {
            continue;
        }
        auto cmd = make_unique<DfsuCmd<NetworkAgentTemplate>>(&NetworkAgentTemplate::ConnectOnlineDevices);
        cmd->UpdateOption({
            .tryTimes_ = MAX_RETRY_COUNT,
        });
        net->Recv(move(cmd));
    }
}

std::shared_ptr<NetworkAgentTemplate> DeviceManagerAgent::FindNetworkBaseTrustRelation(bool isAccountless)
{
    LOGI("enter: isAccountless %{public}d", isAccountless);
    for (auto [ignore, net] : mpToNetworks_) {
        if (net != nullptr) {
            auto smp = net->GetMountPoint();
            if (smp != nullptr && smp->isAccountLess() == isAccountless) {
                return net;
            }
        }
    }
    LOGE("not find this net in mpToNetworks, isAccountless %{public}d", isAccountless);
    return nullptr;
}

int32_t DeviceManagerAgent::GetNetworkType(const string &cid)
{
    int32_t networkType = 0;
    string pkgName = IDaemon::SERVICE_NAME;
    auto &deviceManager = DistributedHardware::DeviceManager::GetInstance();
    int errCode = deviceManager.GetNetworkTypeByNetworkId(pkgName, cid, networkType);
    if (errCode) {
        LOGE("get network type error:%{public}d", errCode);
    }

    return networkType;
}

bool DeviceManagerAgent::IsWifiNetworkType(int32_t networkType)
{
    if ((networkType == -1) ||
        !(static_cast<uint32_t>(networkType) & (1 << DistributedHardware::BIT_NETWORK_TYPE_WIFI))) {
        return false;
    }

    return true;
}

void DeviceManagerAgent::OnDeviceReady(const DistributedHardware::DmDeviceInfo &deviceInfo)
{
    LOGI("networkId %{public}s, OnDeviceReady begin", deviceInfo.deviceId);
    int32_t ret = IsSupportDevice(deviceInfo);
    if (ret != FileManagement::ERR_OK) {
        LOGI("not support device, networkId %{public}s", Utils::GetAnonyString(deviceInfo.deviceId).c_str());
        return;
    }

    // online first query this dev's trust info
    DeviceInfo info(deviceInfo);
    QueryRelatedGroups(info.udid_, info.cid_);

    // based on dev's trust info, choose corresponding network agent to obtain socket
    unique_lock<mutex> lock(mpToNetworksMutex_);

    auto it = cidNetTypeRecord_.find(info.cid_);
    if (it == cidNetTypeRecord_.end()) {
        LOGE("cid %{public}s network is null!", info.cid_.c_str());
        LOGI("OnDeviceReady end");
        return;
    }

    auto type_ = cidNetworkType_.find(info.cid_);
    if (type_ == cidNetworkType_.end()) {
        LOGE("cid %{public}s network type is null!", info.cid_.c_str());
        LOGI("OnDeviceReady end");
        return;
    }

    int32_t newNetworkType = type_->second = GetNetworkType(info.cid_);
    LOGI("newNetworkType:%{public}d", newNetworkType);

    if (!IsWifiNetworkType(newNetworkType)) {
        LOGE("cid %{public}s networkType:%{public}d", info.cid_.c_str(), type_->second);
        LOGI("OnDeviceReady end");
        return;
    }

    auto cmd =
        make_unique<DfsuCmd<NetworkAgentTemplate, const DeviceInfo>>(&NetworkAgentTemplate::ConnectDeviceAsync, info);
    cmd->UpdateOption({.tryTimes_ = MAX_RETRY_COUNT});
    it->second->Recv(move(cmd));
    LOGI("OnDeviceReady end");
}

void DeviceManagerAgent::OnDeviceOffline(const DistributedHardware::DmDeviceInfo &deviceInfo)
{
    LOGI("OnDeviceOffline begin");
    DeviceInfo info(deviceInfo);

    unique_lock<mutex> lock(mpToNetworksMutex_);
    auto it = cidNetTypeRecord_.find(info.cid_);
    if (it == cidNetTypeRecord_.end()) {
        LOGE("cid %{public}s network is null!", info.cid_.c_str());
        LOGI("OnDeviceOffline end");
        return;
    }

    auto type_ = cidNetworkType_.find(info.cid_);
    if (type_ == cidNetworkType_.end()) {
        LOGE("cid %{public}s network type is null!", info.cid_.c_str());
        LOGI("OnDeviceOffline end");
        return;
    }

    auto cmd =
        make_unique<DfsuCmd<NetworkAgentTemplate, const DeviceInfo>>(&NetworkAgentTemplate::DisconnectDevice, info);
    it->second->Recv(move(cmd));

    auto networkId = std::string(deviceInfo.networkId);
    auto deviceId = std::string(deviceInfo.deviceId);
    if (deviceId.empty()) {
        deviceId = GetDeviceIdByNetworkId(networkId);
    }
    if (!networkId.empty()) {
        NotifyRemoteReverseObj(networkId, ON_STATUS_OFFLINE);
        UMountDfsDocs(networkId, deviceId, true);
    }

    auto cmd2 = make_unique<DfsuCmd<NetworkAgentTemplate, const DeviceInfo>>(
        &NetworkAgentTemplate::DisconnectDeviceByP2PHmdfs, info);
    it->second->Recv(move(cmd2));

    if (RemoveRemoteReverseObj(true, 0) != 0) {
        LOGI("RemoveRemoteReverseObj fail");
    }
    RemoveNetworkIdForAllToken(networkId);
    cidNetTypeRecord_.erase(info.cid_);
    cidNetworkType_.erase(info.cid_);
    LOGI("OnDeviceOffline end");
}

int32_t DeviceManagerAgent::OnDeviceP2POnline(const DistributedHardware::DmDeviceInfo &deviceInfo)
{
    LOGI("[OnDeviceP2POnline] networkId %{public}s, OnDeviceOnline begin", deviceInfo.networkId);
    int32_t ret = IsSupportDevice(deviceInfo);
    if (ret != FileManagement::ERR_OK) {
        LOGI("not support device, networkId %{public}s", Utils::GetAnonyString(deviceInfo.deviceId).c_str());
        return P2P_FAILED;
    }
    DeviceInfo info(deviceInfo);
    QueryRelatedGroups(info.udid_, info.cid_);

    unique_lock<mutex> lock(mpToNetworksMutex_);
    auto it = cidNetTypeRecord_.find(info.cid_);
    if (it == cidNetTypeRecord_.end()) {
        LOGE("[OnDeviceP2POnline] cid %{public}s network is null!", info.cid_.c_str());
        return P2P_FAILED;
    }
    auto type_ = cidNetworkType_.find(info.cid_);
    if (type_ == cidNetworkType_.end()) {
        LOGE("[OnDeviceP2POnline] cid %{public}s network type is null!", info.cid_.c_str());
        return P2P_FAILED;
    }
    auto cmd = make_unique<DfsuCmd<NetworkAgentTemplate, const DeviceInfo>>(
        &NetworkAgentTemplate::ConnectDeviceByP2PAsync, info);
    cmd->UpdateOption({.tryTimes_ = MAX_RETRY_COUNT});
    it->second->Recv(move(cmd));
    LOGI("[OnDeviceP2POnline] networkId %{public}s, OnDeviceOnline end", deviceInfo.networkId);
    return P2P_SUCCESS;
}

int32_t DeviceManagerAgent::OnDeviceP2POffline(const DistributedHardware::DmDeviceInfo &deviceInfo)
{
    LOGI("OnDeviceP2POffline begin");
    DeviceInfo info(deviceInfo);

    unique_lock<mutex> lock(mpToNetworksMutex_);
    auto it = cidNetTypeRecord_.find(info.cid_);
    if (it == cidNetTypeRecord_.end()) {
        LOGE("cid %{public}s network is null!", info.cid_.c_str());
        return P2P_FAILED;
    }
    auto type_ = cidNetworkType_.find(info.cid_);
    if (type_ == cidNetworkType_.end()) {
        LOGE("cid %{public}s network type is null!", info.cid_.c_str());
        return P2P_FAILED;
    }
    auto cmd = make_unique<DfsuCmd<NetworkAgentTemplate, const DeviceInfo>>(
        &NetworkAgentTemplate::DisconnectDeviceByP2P, info);
    it->second->Recv(move(cmd));
    cidNetTypeRecord_.erase(info.cid_);
    cidNetworkType_.erase(info.cid_);
    LOGI("OnDeviceP2POffline end");
    return P2P_SUCCESS;
}

bool DeviceManagerAgent::MountDfsCountOnly(const std::string &deviceId)
{
    if (deviceId.empty()) {
        LOGI("deviceId empty");
        return false;
    }
    // Count plus one operation
    unique_lock<mutex> lock(mpToNetworksMutex_);
    auto itCount = mountDfsCount_.find(deviceId);
    if (itCount != mountDfsCount_.end() && itCount->second > 0) {
        LOGI("[MountDfsCountOnly] deviceIde %{public}s has already established a link, count %{public}d, \
            increase count by one now", Utils::GetAnonyString(deviceId).c_str(), itCount->second);
        mountDfsCount_[deviceId]++;
        return true;
    } else {
        LOGI("[MountDfsCountOnly] deviceId %{public}s increase count by one now",
            Utils::GetAnonyString(deviceId).c_str());
        mountDfsCount_[deviceId]++;
    }
    return false;
}

bool DeviceManagerAgent::UMountDfsCountOnly(const std::string &deviceId, bool needClear)
{
    if (deviceId.empty()) {
        LOGI("deviceId empty");
        return false;
    }
    // Count sub one operation
    auto itCount = mountDfsCount_.find(deviceId);
    if (itCount == mountDfsCount_.end()) {
        LOGI("mountDfsCount_ can not find key");
        return false;
    }
    if (needClear) {
        LOGI("mountDfsCount_ erase");
        mountDfsCount_.erase(itCount);
        return false;
    }
    if (itCount->second > MOUNT_DFS_COUNT_ONE) {
        LOGI("[UMountDfsCountOnly] deviceId %{public}s has already established more than one link, \
            count %{public}d, decrease count by one now",
            Utils::GetAnonyString(deviceId).c_str(), itCount->second);
        mountDfsCount_[deviceId]--;
        return true;
    } else {
        LOGI("[UMountDfsCountOnly] deviceId %{public}s erase count", Utils::GetAnonyString(deviceId).c_str());
        mountDfsCount_.erase(itCount);
    }
    return false;
}

int32_t DeviceManagerAgent::GetCurrentUserId()
{
    std::vector<int32_t> userIds{};
    auto ret = AccountSA::OsAccountManager::QueryActiveOsAccountIds(userIds);
    if (ret != NO_ERROR || userIds.empty()) {
        LOGE("query active os account id failed, ret = %{public}d", ret);
        return INVALID_USER_ID;
    }
    LOGI("DeviceManagerAgent::GetCurrentUserId end.");
    return userIds[0];
}

void DeviceManagerAgent::GetStorageManager()
{
    auto saMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (saMgr == nullptr) {
        LOGE("GetSystemAbilityManager filed");
        return;
    }

    auto storageObj = saMgr->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    if (storageObj == nullptr) {
        LOGE("filed to get STORAGE_MANAGER_MANAGER_ID proxy");
        return;
    }

    storageMgrProxy_ = iface_cast<StorageManager::IStorageManager>(storageObj);
    if (storageMgrProxy_ == nullptr) {
        LOGE("filed to get STORAGE_MANAGER_MANAGER_ID proxy!");
        return;
    }

    LOGI("GetStorageManager end.");
    return;
}

void DeviceManagerAgent::AddNetworkId(uint32_t tokenId, const std::string &networkId)
{
    LOGI("DeviceManagerAgent::AddNetworkId, networkId: %{public}s", Utils::GetAnonyString(networkId).c_str());
    std::lock_guard<std::mutex> lock(networkIdMapMutex_);
    networkIdMap_[tokenId].insert(networkId);
}

void DeviceManagerAgent::RemoveNetworkId(uint32_t tokenId)
{
    LOGI("DeviceManagerAgent::RemoveNetworkId start");
    std::lock_guard<std::mutex> lock(networkIdMapMutex_);
    networkIdMap_.erase(tokenId);
}

void DeviceManagerAgent::RemoveNetworkIdByOne(uint32_t tokenId, const std::string &networkId)
{
    std::lock_guard<std::mutex> lock(networkIdMapMutex_);
    auto it = networkIdMap_.find(tokenId);
    if (it != networkIdMap_.end()) {
        (it->second).erase(networkId);
        if (it->second.empty()) {
            networkIdMap_.erase(it);
        }
        LOGI("DeviceManagerAgent::RemoveNetworkIdByOne success, networkId: %{public}s",
            Utils::GetAnonyString(networkId).c_str());
    }
}

void DeviceManagerAgent::RemoveNetworkIdForAllToken(const std::string &networkId)
{
    if (networkId.empty()) {
        LOGE("networkId is empty");
        return;
    }
    for (auto it = networkIdMap_.begin(); it != networkIdMap_.end();) {
        it->second.erase(networkId);
        if (it->second.empty()) {
            networkIdMap_.erase(it);
        }
        LOGI("RemoveNetworkIdForAllToken, networkId: %{public}s",
            Utils::GetAnonyString(networkId).c_str());
        ++it;
    }
}

void DeviceManagerAgent::ClearNetworkId()
{
    std::lock_guard<std::mutex> lock(networkIdMapMutex_);
    networkIdMap_.clear();
}

std::unordered_set<std::string> DeviceManagerAgent::GetNetworkIds(uint32_t tokenId)
{
    std::lock_guard<std::mutex> lock(networkIdMapMutex_);
    return networkIdMap_[tokenId];
}

void DeviceManagerAgent::MountDfsDocs(const std::string &networkId, const std::string &deviceId)
{
    LOGI("MountDfsDocs start");
    if (networkId.empty() || deviceId.empty()) {
        LOGI("NetworkId or DeviceId is empty");
        return;
    }

    if (MountDfsCountOnly(deviceId)) {
        LOGI("only count plus one, do not neet mount");
        return;
    }

    int32_t userId = GetCurrentUserId();
    if (userId == INVALID_USER_ID) {
        LOGI("GetCurrentUserId Fail");
        return;
    }

    GetStorageManager();
    if (storageMgrProxy_ == nullptr) {
        LOGI("storageMgrProxy_ is null");
        return;
    }

    int32_t ret = storageMgrProxy_->MountDfsDocs(userId, "account", networkId, deviceId);
    if (ret != FileManagement::E_OK) {
        LOGI("MountDfsDocs fail, ret = %{public}d", ret);
    }
    LOGI("storageMgr.MountDfsDocs end.");
}

int32_t DeviceManagerAgent::UMountDfsDocs(const std::string &networkId, const std::string &deviceId, bool needClear)
{
    LOGI("UMountDfsDocs start in OpenP2PConnection, networkId: %{public}s, deviceId: %{public}s",
        Utils::GetAnonyString(networkId).c_str(), Utils::GetAnonyString(deviceId).c_str());
    if (networkId.empty() || deviceId.empty()) {
        LOGE("NetworkId or DeviceId is empty");
        return INVALID_USER_ID;
    }

    if (UMountDfsCountOnly(deviceId, needClear)) {
        LOGE("only count sub one, do not neet umount");
        return INVALID_USER_ID;
    }

    int32_t userId = GetCurrentUserId();
    if (userId == INVALID_USER_ID) {
        LOGE("GetCurrentUserId Fail");
        return INVALID_USER_ID;
    }

    GetStorageManager();
    if (storageMgrProxy_ == nullptr) {
        LOGE("storageMgrProxy_ is null");
        return INVALID_USER_ID;
    }
    int32_t ret = storageMgrProxy_->UMountDfsDocs(userId, "account", networkId, deviceId);
    if (ret != FileManagement::E_OK) {
        LOGE("UMountDfsDocs fail, ret = %{public}d", ret);
    }
    LOGI("storageMgr.UMountDfsDocs end.");
    return ret;
}

void DeviceManagerAgent::NotifyRemoteReverseObj(const std::string &networkId, int32_t status)
{
    for (auto it = appCallConnect_.begin(); it != appCallConnect_.end(); ++it) {
        auto onstatusReverseProxy = it->second;
        if (onstatusReverseProxy == nullptr) {
            LOGI("get onstatusReverseProxy fail");
            return;
        }
        onstatusReverseProxy->OnStatus(networkId, status);
        LOGI("NotifyRemoteReverseObj, deviceId: %{public}s", Utils::GetAnonyString(networkId).c_str());
    }
}

int32_t DeviceManagerAgent::AddRemoteReverseObj(uint32_t callingTokenId, sptr<IFileDfsListener> remoteReverseObj)
{
    std::lock_guard<std::mutex> lock(appCallConnectMutex_);
    auto it = appCallConnect_.find(callingTokenId);
    if (it != appCallConnect_.end()) {
        LOGE("AddRemoteReverseObj fail");
        return FileManagement::E_INVAL_ARG;
    }
    appCallConnect_[callingTokenId] = remoteReverseObj;
    LOGI("DeviceManagerAgent::AddRemoteReverseObj::add new value suceess");
    return FileManagement::E_OK;
}

int32_t DeviceManagerAgent::RemoveRemoteReverseObj(bool clear, uint32_t callingTokenId)
{
    if (clear) {
        appCallConnect_.clear();
        return FileManagement::E_OK;
    }

    auto it = appCallConnect_.find(callingTokenId);
    if (it == appCallConnect_.end()) {
        LOGE("RemoveRemoteReverseObj fail");
        return FileManagement::E_INVAL_ARG;
    }
    appCallConnect_.erase(it);
    return FileManagement::E_OK;
}

int32_t DeviceManagerAgent::FindListenerByObject(const wptr<IRemoteObject> &remote,
                                                 uint32_t &tokenId, sptr<IFileDfsListener>& listener)
{
    std::lock_guard<std::mutex> lock(appCallConnectMutex_);
    for (auto it = appCallConnect_.begin(); it != appCallConnect_.end(); ++it) {
        if (remote != (it->second)->AsObject()) {
            continue;
        }
        tokenId = it->first;
        listener = it->second;
        return FileManagement::E_OK;
    }
    return FileManagement::E_INVAL_ARG;
}

std::string DeviceManagerAgent::GetDeviceIdByNetworkId(const std::string &networkId)
{
    LOGI("DeviceManagerAgent::GetDeviceIdByNetworkId called");
    if (networkId.empty()) {
        return "";
    }
    std::vector<DistributedHardware::DmDeviceInfo> deviceList;
    DistributedHardware::DeviceManager::GetInstance().GetTrustedDeviceList(IDaemon::SERVICE_NAME, "", deviceList);
    if (deviceList.size() == 0 || deviceList.size() > MAX_ONLINE_DEVICE_SIZE) {
        LOGE("the size of trust device list is invalid, size=%zu", deviceList.size());
        return "";
    }
    std::string deviceId = "";
    for (const auto &device : deviceList) {
        if (std::string(device.networkId) == networkId) {
            deviceId = std::string(device.deviceId);
        }
    }
    LOGI("DeviceManagerAgent::GetDeviceIdByNetworkId end");
    return deviceId;
}

void from_json(const nlohmann::json &jsonObject, GroupInfo &groupInfo)
{
    if (jsonObject.find(FIELD_GROUP_NAME) != jsonObject.end()) {
        groupInfo.groupName = jsonObject.at(FIELD_GROUP_NAME).get<std::string>();
    }

    if (jsonObject.find(FIELD_GROUP_ID) != jsonObject.end()) {
        groupInfo.groupId = jsonObject.at(FIELD_GROUP_ID).get<std::string>();
    }

    if (jsonObject.find(FIELD_GROUP_OWNER) != jsonObject.end()) {
        groupInfo.groupOwner = jsonObject.at(FIELD_GROUP_OWNER).get<std::string>();
    }

    if (jsonObject.find(FIELD_GROUP_TYPE) != jsonObject.end()) {
        groupInfo.groupType = jsonObject.at(FIELD_GROUP_TYPE).get<int32_t>();
    }
}

void DeviceManagerAgent::QueryRelatedGroups(const std::string &udid, const std::string &networkId)
{
    auto hichainDevGroupMgr_ = GetGmInstance();
    if (hichainDevGroupMgr_ == nullptr) {
        LOGE("failed to get hichain device group manager");
        return;
    }

    char *returnGroupVec = nullptr;
    uint32_t groupNum = 0;
    int ret = hichainDevGroupMgr_->getRelatedGroups(ANY_OS_ACCOUNT, IDaemon::SERVICE_NAME.c_str(), udid.c_str(),
                                                    &returnGroupVec, &groupNum);
    if (ret != 0 || returnGroupVec == nullptr) {
        LOGE("failed to get related groups, ret %{public}d", ret);
        return;
    }

    if (groupNum == 0) {
        LOGE("failed to get related groups, groupNum is %{public}u", groupNum);
        return;
    }

    std::string groups = std::string(returnGroupVec);
    nlohmann::json jsonObject = nlohmann::json::parse(groups); // transform from cjson to cppjson
    if (jsonObject.is_discarded()) {
        LOGE("returnGroupVec parse failed");
        return;
    }

    std::vector<GroupInfo> groupList;
    groupList = jsonObject.get<std::vector<GroupInfo>>();
    for (auto &a : groupList) {
        LOGI("group info:[groupName] %{public}s, [groupId] %{public}s, [groupOwner] %{public}s,[groupType] %{public}d,",
             a.groupName.c_str(), a.groupId.c_str(), a.groupOwner.c_str(), a.groupType);
    }

    unique_lock<mutex> lock(mpToNetworksMutex_);
    for (const auto &group : groupList) {
        auto network = FindNetworkBaseTrustRelation(CheckIsAccountless(group));
        if (network != nullptr) {
            cidNetTypeRecord_.insert({ networkId, network });
            cidNetworkType_.insert({ networkId, GetNetworkType(networkId) });
        }
    }

    return;
}

bool DeviceManagerAgent::CheckIsAccountless(const GroupInfo &group)
{
    // because of there no same_account, only for test, del later
    LOGI("SAME_ACCOUNT_MARK val is %{public}d", system::GetBoolParameter(SAME_ACCOUNT_MARK, false));
    if (system::GetBoolParameter(SAME_ACCOUNT_MARK, false) == true) { // isaccountless == false
        LOGI("SAME_ACCOUNT_MARK val is true(same account)");
        return false;
    } else { // isaccountless == true
        return true;
    }

    if (group.groupType == PEER_TO_PEER_GROUP || group.groupType == ACROSS_ACCOUNT_AUTHORIZE_GROUP) {
        return true;
    }
    return false;
}

void DeviceManagerAgent::OnDeviceChanged(const DistributedHardware::DmDeviceInfo &deviceInfo)
{
    LOGI("OnDeviceInfoChanged begin");
    if (deviceInfo.networkType == -1) {
        LOGI("OnDeviceInfoChanged end");
        return;
    }
    int32_t ret = IsSupportDevice(deviceInfo);
    if (ret != FileManagement::ERR_OK) {
        LOGI("not support device, networkId %{public}s", Utils::GetAnonyString(deviceInfo.deviceId).c_str());
        return;
    }

    DeviceInfo info(deviceInfo);
    unique_lock<mutex> lock(mpToNetworksMutex_);

    auto it = cidNetTypeRecord_.find(info.cid_);
    if (it == cidNetTypeRecord_.end()) {
        LOGE("cid %{public}s network is null!", info.cid_.c_str());
        LOGI("OnDeviceInfoChanged end");
        return;
    }

    auto type_ = cidNetworkType_.find(info.cid_);
    if (type_ == cidNetworkType_.end()) {
        LOGE("cid %{public}s network type is null!", info.cid_.c_str());
        LOGI("OnDeviceInfoChanged end");
        return;
    }

    int32_t oldNetworkType = type_->second;
    int32_t newNetworkType = type_->second = deviceInfo.networkType;
    LOGI("oldNetworkType %{public}d, newNetworkType %{public}d", oldNetworkType, newNetworkType);
    if (IsWifiNetworkType(newNetworkType) && !IsWifiNetworkType(oldNetworkType)) {
        LOGI("Wifi connect");
        auto cmd = make_unique<DfsuCmd<NetworkAgentTemplate, const DeviceInfo>>(
            &NetworkAgentTemplate::ConnectDeviceAsync, info);
        cmd->UpdateOption({.tryTimes_ = MAX_RETRY_COUNT});
        it->second->Recv(move(cmd));
        LOGI("OnDeviceInfoChanged end");
        return;
    }

    if (!IsWifiNetworkType(newNetworkType) && IsWifiNetworkType(oldNetworkType)) {
        LOGI("Wifi disconnect");
        auto cmd = make_unique<DfsuCmd<NetworkAgentTemplate, const DeviceInfo>>
            (&NetworkAgentTemplate::DisconnectDevice, info);
        it->second->Recv(move(cmd));
        LOGI("OnDeviceInfoChanged end");
        return;
    }
    LOGI("OnDeviceInfoChanged end");
}

void DeviceManagerAgent::InitDeviceInfos()
{
    string extra = "";
    string pkgName = IDaemon::SERVICE_NAME;
    vector<DistributedHardware::DmDeviceInfo> deviceInfoList;

    auto &deviceManager = DistributedHardware::DeviceManager::GetInstance();
    int errCode = deviceManager.GetTrustedDeviceList(pkgName, extra, deviceInfoList);
    if (errCode) {
        THROW_EXCEPTION(errCode, "Failed to get info of remote devices");
    }

    for (const auto &deviceInfo : deviceInfoList) {
        int32_t ret = IsSupportDevice(deviceInfo);
        if (ret != FileManagement::ERR_OK) {
            LOGI("not support device, networkId %{public}s", Utils::GetAnonyString(deviceInfo.deviceId).c_str());
            continue;
        }
        DeviceInfo info(deviceInfo);
        QueryRelatedGroups(info.udid_, info.cid_);
    }
}

int32_t DeviceManagerAgent::IsSupportDevice(const DistributedHardware::DmDeviceInfo &deviceInfo)
{
    std::string udid = "";
    if (DistributedHardware::DeviceManager::GetInstance().
        GetUdidByNetworkId(IDaemon::SERVICE_NAME, deviceInfo.networkId, udid) != 0) {
        LOGE("GetUdidByNetworkId failed networkId %{public}s", Utils::GetAnonyString(deviceInfo.deviceId).c_str());
        return FileManagement::ERR_BAD_VALUE;
    }
    DistributedDeviceProfile::DeviceProfile outDeviceProfile;
    int32_t ret = DistributedDeviceProfile::DistributedDeviceProfileClient::GetInstance().
        GetDeviceProfile(udid, outDeviceProfile);
    if (ret != FileManagement::E_OK) {
        LOGE("GetDeviceProfile failed, errorCode: %{public}d, udid: %{public}s", ret,
            Utils::GetAnonyString(udid).c_str());
        return FileManagement::ERR_BAD_VALUE;
    }
    if (outDeviceProfile.GetOsType() != DEVICE_OS_TYPE_OH) {
        LOGE("%{private}s  the device os type = %{private}d is not openharmony.", deviceInfo.deviceName,
            outDeviceProfile.GetOsType());
        Utils::SysEventWrite(udid);
        return FileManagement::ERR_BAD_VALUE;
    }
    return FileManagement::ERR_OK;
}

void DeviceManagerAgent::InitLocalNodeInfo()
{
    NodeBasicInfo tmpNodeInfo;
    int errCode = GetLocalNodeDeviceInfo(IDaemon::SERVICE_NAME.c_str(), &tmpNodeInfo);
    if (errCode != 0) {
        THROW_EXCEPTION(errCode, "Failed to get info of local devices");
    }
    localDeviceInfo_.SetCid(string(tmpNodeInfo.networkId));
}

void DeviceManagerAgent::OnRemoteDied()
{
    LOGI("device manager service died");
}

DeviceInfo &DeviceManagerAgent::GetLocalDeviceInfo()
{
    return localDeviceInfo_;
}

vector<DeviceInfo> DeviceManagerAgent::GetRemoteDevicesInfo()
{
    string extra = "";
    string pkgName = IDaemon::SERVICE_NAME;
    vector<DistributedHardware::DmDeviceInfo> deviceList;

    auto &deviceManager = DistributedHardware::DeviceManager::GetInstance();
    int errCode = deviceManager.GetTrustedDeviceList(pkgName, extra, deviceList);
    if (errCode) {
        THROW_EXCEPTION(errCode, "Failed to get info of remote devices");
    }

    vector<DeviceInfo> res;
    for (const auto &item : deviceList) {
        res.push_back(DeviceInfo(item));
    }
    return res;
}

void DeviceManagerAgent::RegisterToExternalDm()
{
    auto &deviceManager = DistributedHardware::DeviceManager::GetInstance();
    string pkgName = IDaemon::SERVICE_NAME;
    int errCode = deviceManager.InitDeviceManager(pkgName, shared_from_this());
    if (errCode != 0) {
        THROW_EXCEPTION(errCode, "Failed to InitDeviceManager");
    }
    string extra = "";
    errCode = deviceManager.RegisterDevStateCallback(pkgName, extra, shared_from_this());
    if (errCode != 0) {
        THROW_EXCEPTION(errCode, "Failed to RegisterDevStateCallback");
    }
    LOGI("RegisterToExternalDm Succeed");
}

void DeviceManagerAgent::UnregisterFromExternalDm()
{
    auto &deviceManager = DistributedHardware::DeviceManager::GetInstance();
    string pkgName = IDaemon::SERVICE_NAME;
    int errCode = deviceManager.UnRegisterDevStateCallback(pkgName);
    if (errCode != 0) {
        THROW_EXCEPTION(errCode, "Failed to UnRegisterDevStateCallback");
    }
    errCode = deviceManager.UnInitDeviceManager(pkgName);
    if (errCode != 0) {
        THROW_EXCEPTION(errCode, "Failed to UnInitDeviceManager");
    }
    LOGI("UnregisterFromExternalDm Succeed");
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
