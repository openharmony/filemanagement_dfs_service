/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "device_auth.h"
#include "ipc/i_daemon.h"
#include "mountpoint/mount_manager.h"
#include "network/softbus/softbus_agent.h"
#include "softbus_bus_center.h"
#include "utils_exception.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace {
constexpr int MAX_RETRY_COUNT = 7;
constexpr int PEER_TO_PEER_GROUP = 256;
constexpr int ACROSS_ACCOUNT_AUTHORIZE_GROUP = 1282;
} // namespace
using namespace std;

DeviceManagerAgent::DeviceManagerAgent() : Actor<DeviceManagerAgent>(this, std::numeric_limits<uint32_t>::max()) {}

DeviceManagerAgent::~DeviceManagerAgent()
{
    StopInstance();
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
    try {
        RegisterToExternalDm();
        InitLocalNodeInfo();
    } catch (const Exception &e) {
        LOGE("interacting with devicemanager failed, err code %{public}d", e.code());
    } catch (const std::exception &e) {
        LOGE("Unexpect Low Level exception");
    }
}

void DeviceManagerAgent::Stop()
{
    UnregisterFromExternalDm();
}

void DeviceManagerAgent::JoinGroup(weak_ptr<MountPoint> mp)
{
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
        auto [ignored, inserted] = mpToNetworks_.insert({smp->GetID(), agent});
        if (!inserted) {
            stringstream ss;
            ss << "Failed to join group: Mountpoint existed" << smp->ToString();
            throw runtime_error(ss.str());
        }
    }
    LOGI("smp id %{public}d, groupId %{public}s", smp->GetID(), smp->GetAuthGroupId().c_str());
    agent->StartActor();
}

void DeviceManagerAgent::QuitGroup(weak_ptr<MountPoint> mp)
{
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
}

void DeviceManagerAgent::OfflineAllDevice()
{
    unique_lock<mutex> lock(mpToNetworksMutex_);
    for (auto &&networkAgent : mpToNetworks_) {
        auto cmd = make_unique<Cmd<NetworkAgentTemplate>>(&NetworkAgentTemplate::DisconnectAllDevices);
        cmd->UpdateOption({
            .tryTimes_ = 1,
        });
        networkAgent.second->Recv(move(cmd));
    }
}

void DeviceManagerAgent::ReconnectOnlineDevices()
{
    unique_lock<mutex> lock(mpToNetworksMutex_);
    for (auto &&networkAgent : mpToNetworks_) {
        auto cmd = make_unique<Cmd<NetworkAgentTemplate>>(&NetworkAgentTemplate::ConnectOnlineDevices);
        cmd->UpdateOption({
            .tryTimes_ = MAX_RETRY_COUNT,
        });
        networkAgent.second->Recv(move(cmd));
    }
}

void DeviceManagerAgent::OnDeviceOnline(const DistributedHardware::DmDeviceInfo &deviceInfo)
{
    LOGI("netwrorkId %{public}s, OnDeviceOnline begin", deviceInfo.deviceId);
    DeviceInfo info(deviceInfo);
    {
        unique_lock<mutex> lock(mpToNetworksMutex_);
        for (auto &&networkAgent : mpToNetworks_) {
            auto cmd = make_unique<Cmd<NetworkAgentTemplate, const DeviceInfo>>(
                &NetworkAgentTemplate::ConnectDeviceAsync, info);
            cmd->UpdateOption({
                .tryTimes_ = MAX_RETRY_COUNT,
            });
            networkAgent.second->Recv(move(cmd));
        }
    }
    AuthGroupOnlineProc(info);

    LOGI("OnDeviceOnline end");
}

void DeviceManagerAgent::OnDeviceOffline(const DistributedHardware::DmDeviceInfo &deviceInfo)
{
    LOGI("OnDeviceOffline begin");
    DeviceInfo info(deviceInfo);
    {
        unique_lock<mutex> lock(mpToNetworksMutex_);
        for (auto &&networkAgent : mpToNetworks_) {
            auto cmd =
                make_unique<Cmd<NetworkAgentTemplate, const DeviceInfo>>(&NetworkAgentTemplate::DisconnectDevice, info);
            cmd->UpdateOption({
                .tryTimes_ = 1,
            });
            networkAgent.second->Recv(move(cmd));
        }
    }
    AuthGroupOfflineProc(info);
    LOGI("OnDeviceOffline end");
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

void DeviceManagerAgent::QueryRelatedGroups(const std::string &udid, std::vector<GroupInfo> &groupList)
{
    LOGI("use udid %{public}s query hichain related groups", udid.c_str());
    int ret = InitDeviceAuthService();
    if (ret != 0) {
        LOGE("InitDeviceAuthService failed, ret %{public}d", ret);
        return;
    }

    hichainDeviceGroupManager_ = GetGmInstance();
    if (hichainDeviceGroupManager_ == nullptr) {
        LOGE("failed to get hichain device group manager");
        return;
    }

    char *returnGroupVec = nullptr;
    uint32_t groupNum = 0;
    ret = hichainDeviceGroupManager_->getRelatedGroups(IDaemon::SERVICE_NAME.c_str(), udid.c_str(), &returnGroupVec, &groupNum);
    if (ret != 0 || returnGroupVec == nullptr) {
        LOGE("failed to get related groups, ret %{public}d", ret);
        return;
    }

    if (groupNum == 0) {
        LOGE("failed to get related groups, groupNum is %{public}d", groupNum);
        return;
    }

    std::string groups = std::string(returnGroupVec);
    nlohmann::json jsonObject = nlohmann::json::parse(groups); // transform from cjson to cppjson
    if (jsonObject.is_discarded()) {
        LOGE("returnGroupVec parse failed");
        return;
    }

    groupList = jsonObject.get<std::vector<GroupInfo>>();
    for (auto &a : groupList) {
        LOGI("group info:[groupName] %{public}s, [groupId] %{public}s, [groupOwner] %{public}s,[groupId] %{public}d,", a.groupName.c_str(), a.groupId.c_str(), a.groupOwner.c_str(), a.groupType);
    }
    return;
}

void DeviceManagerAgent::AuthGroupOnlineProc(const DeviceInfo info)
{
    // convert networkId to uuid
    // todo: udid获取放到DeviceInfo类中实现
    std::string udid;
    auto &deviceManager = DistributedHardware::DeviceManager::GetInstance();
    int32_t ret = deviceManager.GetUdidByNetworkId(IDaemon::SERVICE_NAME, info.GetCid(), udid);
    std::string uuid;
    ret += deviceManager.GetUuidByNetworkId(IDaemon::SERVICE_NAME, info.GetCid(), uuid);
    LOGI("ret %{public}d, get udid %{public}s, uuid %{public}s", ret, udid.c_str(), uuid.c_str());
    std::vector<GroupInfo> groupList;
    QueryRelatedGroups(udid, groupList);
    for (const auto &group : groupList) {
        if (!CheckIsAuthGroup(group)) {
            continue;
        }
        if (authGroupMap_.find(group.groupId) == authGroupMap_.end()) {
            LOGI("groupId %{public}s not exist, then mount", group.groupId.c_str());
            MountManager::GetInstance()->Mount(make_unique<MountPoint>(
                Utils::MountArgumentDescriptors::SetAuthGroupMountArgument(group.groupId, group.groupOwner, true)));
        }
        auto [iter, status] = authGroupMap_[group.groupId].insert(info.GetCid());
        if (status == false) {
            LOGI("cid %{public}s has already inserted into groupId %{public}s", info.GetCid().c_str(),
                 group.groupId.c_str());
            continue;
        }
    }
}

void DeviceManagerAgent::AuthGroupOfflineProc(const DeviceInfo &info)
{
    for (auto iter = authGroupMap_.begin(); iter != authGroupMap_.end();) {
        auto set = iter->second;
        auto groupId = iter->first;
        if (set.find(info.GetCid()) == set.end()) {
            continue;
        }

        if (authGroupMap_.find(groupId) == authGroupMap_.end()) {
            LOGI("can not find groupId %{public}s ", groupId.c_str());
            continue;
        }
        authGroupMap_[groupId].erase(info.GetCid());
        if (authGroupMap_[groupId].empty()) {
            std::vector<GroupInfo> groupList;
            if (groupList.size() == 0) {
                MountManager::GetInstance()->Umount(groupId);
                iter = authGroupMap_.erase(iter);
                continue;
            }
        }
        iter++;
    }
}

void DeviceManagerAgent::AllAuthGroupsOfflineProc()
{
    for (auto iter = authGroupMap_.begin(); iter != authGroupMap_.end();) {
        auto groupId = iter->first;
        MountManager::GetInstance()->Umount(groupId);
        authGroupMap_.erase(iter++);
    }
}

bool DeviceManagerAgent::CheckIsAuthGroup(const GroupInfo &group)
{
    if (group.groupType == PEER_TO_PEER_GROUP || group.groupType == ACROSS_ACCOUNT_AUTHORIZE_GROUP) {
        return true;
    }
    return false;
}

void DeviceManagerAgent::OnDeviceChanged(const DistributedHardware::DmDeviceInfo &deviceInfo)
{
    LOGI("OnDeviceInfoChanged");
}

void DeviceManagerAgent::InitLocalNodeInfo()
{
    NodeBasicInfo tmpNodeInfo;
    int errCode = GetLocalNodeDeviceInfo(IDaemon::SERVICE_NAME.c_str(), &tmpNodeInfo);
    if (errCode != 0) {
        ThrowException(errCode, "Failed to get info of local devices");
    }
    localDeviceInfo_.SetCid(string(tmpNodeInfo.networkId));
}

void DeviceManagerAgent::OnRemoteDied()
{
    LOGI("device manager service died");
    StopInstance();
    OfflineAllDevice(); // cannot commit a cmd to queue
    AllAuthGroupsOfflineProc();
    StartInstance();
    ReconnectOnlineDevices();
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
        ThrowException(errCode, "Failed to get info of remote devices");
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
        ThrowException(errCode, "Failed to InitDeviceManager");
    }
    string extra = "";
    errCode = deviceManager.RegisterDevStateCallback(pkgName, extra, shared_from_this());
    if (errCode != 0) {
        ThrowException(errCode, "Failed to RegisterDevStateCallback");
    }
    LOGI("RegisterToExternalDm Succeed");
}

void DeviceManagerAgent::UnregisterFromExternalDm()
{
    auto &deviceManager = DistributedHardware::DeviceManager::GetInstance();
    string pkgName = IDaemon::SERVICE_NAME;
    int errCode = deviceManager.UnRegisterDevStateCallback(pkgName);
    if (errCode != 0) {
        ThrowException(errCode, "Failed to UnRegisterDevStateCallback");
    }
    errCode = deviceManager.UnInitDeviceManager(pkgName);
    if (errCode != 0) {
        ThrowException(errCode, "Failed to UnInitDeviceManager");
    }
    LOGI("UnregisterFromExternalDm Succeed");
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS