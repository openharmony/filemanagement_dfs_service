/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
#include "dfsu_exception.h"
#include "ipc/i_daemon.h"
#include "mountpoint/mount_manager.h"
#include "network/devsl_dispatcher.h"
#include "network/softbus/softbus_agent.h"
#include "parameters.h"
#include "softbus_bus_center.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace {
constexpr int MAX_RETRY_COUNT = 7;
constexpr int PEER_TO_PEER_GROUP = 256;
constexpr int ACROSS_ACCOUNT_AUTHORIZE_GROUP = 1282;
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

void DeviceManagerAgent::OnDeviceOnline(const DistributedHardware::DmDeviceInfo &deviceInfo)
{
    LOGI("networkId %{public}s, OnDeviceOnline begin", deviceInfo.deviceId);

    // online first query this dev's trust info
    DeviceInfo info(deviceInfo);
    QueryRelatedGroups(info.udid_, info.cid_);

    // based on dev's trust info, choose corresponding network agent to obtain socket
    unique_lock<mutex> lock(mpToNetworksMutex_);

    auto it = cidNetTypeRecord_.find(info.cid_);
    if (it == cidNetTypeRecord_.end()) {
        LOGE("cid %{public}s network is null!", info.cid_.c_str());
        LOGI("OnDeviceOnline end");
        return;
    }

    auto type_ = cidNetworkType_.find(info.cid_);
    if (type_ == cidNetworkType_.end()) {
        LOGE("cid %{public}s network type is null!", info.cid_.c_str());
        LOGI("OnDeviceOnline end");
        return;
    }

    int32_t newNetworkType = type_->second = GetNetworkType(info.cid_);
    LOGI("newNetworkType:%{public}d", newNetworkType);

    if (!IsWifiNetworkType(newNetworkType)) {
        LOGE("cid %{public}s networkType:%{public}d", info.cid_.c_str(), type_->second);
        LOGI("OnDeviceOnline end");
        return;
    }

    auto cmd =
        make_unique<DfsuCmd<NetworkAgentTemplate, const DeviceInfo>>(&NetworkAgentTemplate::ConnectDeviceAsync, info);
    cmd->UpdateOption({.tryTimes_ = MAX_RETRY_COUNT});
    it->second->Recv(move(cmd));
    LOGI("OnDeviceOnline end");
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
    cidNetTypeRecord_.erase(info.cid_);
    cidNetworkType_.erase(info.cid_);
    LOGI("OnDeviceOffline end");
}

int32_t DeviceManagerAgent::OnDeviceP2POnline(const DistributedHardware::DmDeviceInfo &deviceInfo)
{
    LOGI("[OnDeviceP2POnline] networkId %{public}s, OnDeviceOnline begin", deviceInfo.networkId);
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
        DeviceInfo info(deviceInfo);
        QueryRelatedGroups(info.udid_, info.cid_);
    }
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
