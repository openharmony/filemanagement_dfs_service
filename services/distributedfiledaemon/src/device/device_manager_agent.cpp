/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "connect_count/connect_count.h"
#include "device_auth.h"
#include "dfs_daemon_event_dfx.h"
#include "dfs_error.h"
#include "dfs_radar.h"
#include "radar_report.h"
#include "dfsu_exception.h"
#include "ipc/i_daemon.h"
#include "ipc_skeleton.h"
#include "iremote_object.h"
#include "iservice_registry.h"
#include "istorage_manager.h"
#include "mountpoint/mount_manager.h"
#include "network/devsl_dispatcher.h"
#include "network/softbus/softbus_agent.h"
#include "os_account_manager.h"
#include "parameters.h"
#include "softbus_bus_center.h"
#include "system_ability_definition.h"
#include "system_notifier.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace {
constexpr int32_t DEVICE_OS_TYPE_OH = 10;
constexpr int MAX_RETRY_COUNT = 7;
const int32_t MOUNT_DFS_COUNT_ONE = 1;
const uint32_t MAX_ONLINE_DEVICE_SIZE = 10000;
constexpr const char* PARAM_KEY_OS_TYPE = "OS_TYPE";
const std::string SAME_ACCOUNT_MARK = "const.distributed_file_only_for_same_account_test";
constexpr const char *MOUNT_PATH = "/storage/hmdfs/";
constexpr int32_t VALID_MOUNT_PATH_LEN = 16;
} // namespace
using namespace std;
using namespace OHOS::FileManagement;
MountCountInfo::MountCountInfo(std::string networkId, uint32_t callingTokenId)
{
    networkId_ = networkId;
    callingCountMap_.emplace(callingTokenId, MOUNT_DFS_COUNT_ONE);
}

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

void DeviceManagerAgent::QuitGroup(shared_ptr<MountPoint> smp)
{
    LOGI("quit group begin");
    OfflineAllDevice();
    SystemNotifier::GetInstance().ClearAllConnect();

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

    if (it->second != nullptr) {
        it->second->StopActor();
    } else {
        LOGE("QuitGroup NetworkAgentTemplate is nullptr");
    }
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
    cidNetTypeRecord_.clear();
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

void DeviceManagerAgent::OnDeviceOffline(const DistributedHardware::DmDeviceInfo &deviceInfo)
{
    LOGI("OnDeviceOffline  begin networkId %{public}s", Utils::GetAnonyString(deviceInfo.networkId).c_str());
    RadarParaInfo radarInfo = {"OnDeviceOffline", ReportLevel::DEFAULT, DfxBizStage::DEFAULT, DEFAULT_PKGNAME,
        deviceInfo.networkId, FileManagement::ERR_OK,
        "Device Offline, networkid:" + Utils::GetAnonyString(deviceInfo.networkId)};
    RadarReportAdapter::GetInstance().ReportLinkConnectionAdapter(radarInfo);

    DeviceInfo info(deviceInfo);
    unique_lock<mutex> lock(mpToNetworksMutex_);
    auto it = cidNetTypeRecord_.find(info.cid_);
    if (it == cidNetTypeRecord_.end()) {
        LOGE("OnDeviceOffline end, cid %{public}s network is null!", Utils::GetAnonyString(info.cid_).c_str());
        return;
    }

    auto networkId = std::string(deviceInfo.networkId);
    if (!networkId.empty()) {
        UMountDfsDocs(networkId, networkId.substr(0, VALID_MOUNT_PATH_LEN), true);
    }

    auto cmd2 = make_unique<DfsuCmd<NetworkAgentTemplate, const std::string>>(
        &NetworkAgentTemplate::DisconnectDeviceByP2PHmdfs, info.cid_);
    if (it->second != nullptr) {
        it->second->Recv(move(cmd2));
    } else {
        LOGE("OnDeviceOffline NetworkAgentTemplate is nullptr");
        return;
    }

    cidNetTypeRecord_.erase(info.cid_);
    LOGI("OnDeviceOffline end");
}

int32_t DeviceManagerAgent::OnDeviceP2POnline(const DistributedHardware::DmDeviceInfo &deviceInfo)
{
    LOGI("[OnDeviceP2POnline] networkId %{public}s, OnDeviceOnline begin",
        Utils::GetAnonyString(deviceInfo.networkId).c_str());
    int32_t ret = IsSupportedDevice(deviceInfo);
    if (ret != FileManagement::ERR_OK) {
        LOGI("not support device, networkId %{public}s", Utils::GetAnonyString(deviceInfo.networkId).c_str());
        return P2P_FAILED;
    }
    DeviceInfo info(deviceInfo);
    QueryRelatedGroups(info.udid_, info.cid_);

    unique_lock<mutex> lock(mpToNetworksMutex_);
    auto it = cidNetTypeRecord_.find(info.cid_);
    if (it == cidNetTypeRecord_.end()) {
        LOGE("[OnDeviceP2POnline] cid %{public}s network is null!", Utils::GetAnonyString(info.cid_).c_str());
        return P2P_FAILED;
    }
    auto cmd = make_unique<DfsuCmd<NetworkAgentTemplate, const DeviceInfo>>(
        &NetworkAgentTemplate::ConnectDeviceByP2PAsync, info);
    cmd->UpdateOption({.tryTimes_ = MAX_RETRY_COUNT});
    if (it->second != nullptr) {
        it->second->Recv(move(cmd));
    } else {
        LOGE("OnDeviceP2POnline NetworkAgentTemplate is nullptr");
        return P2P_FAILED;
    }
    LOGI("OnDeviceP2POnline end networkId %{public}s", Utils::GetAnonyString(deviceInfo.networkId).c_str());
    return P2P_SUCCESS;
}

int32_t DeviceManagerAgent::OnDeviceP2POffline(const DistributedHardware::DmDeviceInfo &deviceInfo)
{
    LOGI("OnDeviceP2POffline  begin networkId %{public}s", Utils::GetAnonyString(deviceInfo.networkId).c_str());
    DeviceInfo info(deviceInfo);

    unique_lock<mutex> lock(mpToNetworksMutex_);
    auto it = cidNetTypeRecord_.find(info.cid_);
    if (it == cidNetTypeRecord_.end()) {
        LOGE("cid %{public}s network is null!",  Utils::GetAnonyString(info.cid_).c_str());
        return P2P_FAILED;
    }
    auto cmd = make_unique<DfsuCmd<NetworkAgentTemplate, const std::string>>(
        &NetworkAgentTemplate::DisconnectDeviceByP2P, info.cid_);
    if (it->second != nullptr) {
        it->second->Recv(move(cmd));
    } else {
        LOGE("OnDeviceP2POffline it->second is nullptr");
        RadarParaInfo radarInfo = {"OnDeviceP2POffline", ReportLevel::INNER, DfxBizStage::SOFTBUS_OPENP2P,
            DEFAULT_PKGNAME, deviceInfo.networkId, P2P_FAILED, "OnDeviceP2POffline is null"};
        RadarReportAdapter::GetInstance().ReportLinkConnectionAdapter(radarInfo);
        return P2P_FAILED;
    }
    cidNetTypeRecord_.erase(info.cid_);
    LOGI("OnDeviceP2POffline end");
    return P2P_SUCCESS;
}

bool DeviceManagerAgent::MountDfsCountOnly(const std::string &deviceId)
{
    if (deviceId.empty()) {
        LOGI("deviceId empty");
        return false;
    }
    std::lock_guard<std::mutex> lock(mountDfsCountMutex_);
    auto itCount = mountDfsCount_.find(deviceId);
    if (itCount == mountDfsCount_.end()) {
        LOGI("mountDfsCount_ can not find key");
        return false;
    }
    return true;
}

bool DeviceManagerAgent::UMountDfsCountOnly(const std::string &deviceId, bool needClear)
{
    std::lock_guard<std::mutex> lock(mountDfsCountMutex_);
    auto itCount = mountDfsCount_.find(deviceId);
    if (itCount == mountDfsCount_.end()) {
        LOGI("mountDfsCount_ can not find key");
        return true;
    }
    if (needClear) {
        LOGI("mountDfsCount_ erase");
        std::map<uint32_t, int32_t> emptyMap;
        itCount->second.callingCountMap_.swap(emptyMap);
        return false;
    }
    auto &callingCountMap = itCount->second.callingCountMap_;
    auto callingTokenId = IPCSkeleton::GetCallingTokenID();
    auto callingIter = callingCountMap.find(callingTokenId);
    if (callingIter != callingCountMap.end() && callingIter->second > MOUNT_DFS_COUNT_ONE) {
        LOGI("[UMountDfsCountOnly] deviceId %{public}s has already established more than one link, \
            count %{public}d, decrease count by one now",
            Utils::GetAnonyString(deviceId).c_str(), callingIter->second);
        --callingIter->second;
        return true;
    }
    LOGI("[UMountDfsCountOnly] deviceId %{public}s erase count", Utils::GetAnonyString(deviceId).c_str());
    return false;
}

int32_t DeviceManagerAgent::GetCurrentUserId()
{
    std::vector<int32_t> userIds{};
    auto ret = AccountSA::OsAccountManager::QueryActiveOsAccountIds(userIds);
    if (ret != NO_ERROR || userIds.empty()) {
        RadarParaInfo info = {"GetCurrentUserId", ReportLevel::INNER, DfxBizStage::SOFTBUS_OPENP2P,
            "account", "", ret, "query active os account id fail"};
        RadarReportAdapter::GetInstance().ReportLinkConnectionAdapter(info);
        return INVALID_USER_ID;
    }
    LOGI("DeviceManagerAgent::GetCurrentUserId end.");
    return userIds[0];
}

sptr<StorageManager::IStorageManager> DeviceManagerAgent::GetStorageManager()
{
    auto saMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (saMgr == nullptr) {
        LOGE("GetSystemAbilityManager filed");
        return nullptr;
    }

    auto storageObj = saMgr->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    if (storageObj == nullptr) {
        LOGE("filed to get STORAGE_MANAGER_MANAGER_ID proxy");
        return nullptr;
    }

    LOGI("GetStorageManager end.");
    return iface_cast<StorageManager::IStorageManager>(storageObj);
}

int32_t DeviceManagerAgent::MountDfsDocs(const std::string &networkId,
                                         const std::string &deviceId,
                                         const uint32_t callingTokenId)
{
    if (networkId.empty() || deviceId.empty()) {
        LOGE("NetworkId or DeviceId is empty");
        return FileManagement::ERR_BAD_VALUE;
    }
    int32_t ret = NO_ERROR;
    if (MountDfsCountOnly(deviceId)) {
        LOGI("only count plus one, do not need mount");
        IncreaseMountDfsCount(networkId, deviceId, callingTokenId);
        return ret;
    }
    int32_t userId = GetCurrentUserId();
    if (userId == INVALID_USER_ID) {
        LOGE("GetCurrentUserId Fail");
        return FileManagement::ERR_BAD_VALUE;
    }
    currentUserId_ = userId;
    auto storageMgrProxy = GetStorageManager();
    if (storageMgrProxy == nullptr) {
        LOGE("storageMgrProxy is null");
        return FileManagement::ERR_BAD_VALUE;
    }
    ret = storageMgrProxy->MountDfsDocs(userId, "account", networkId, deviceId);
    if (ret != NO_ERROR) {
        LOGE("MountDfsDocs fail, ret = %{public}d", ret);
        RadarParaInfo info = {"MountDfsDocs", ReportLevel::INNER, DfxBizStage::MOUNT_DOCS,
            "storage", networkId, ret, "MountDfsDocs fail"};
        RadarReportAdapter::GetInstance().ReportLinkConnectionAdapter(info);
    } else {
        LOGE("MountDfsDocs success, deviceId %{public}s increase count by one now",
             Utils::GetAnonyString(deviceId).c_str());
        IncreaseMountDfsCount(networkId, deviceId, callingTokenId);
    }
    LOGI("storageMgr.MountDfsDocs end.");
    return ret;
}

int32_t DeviceManagerAgent::UMountDfsDocs(const std::string &networkId, const std::string &deviceId, bool needClear)
{
    LOGI("UMountDfsDocs start in ConnectDfs, networkId: %{public}s, deviceId: %{public}s",
        Utils::GetAnonyString(networkId).c_str(), Utils::GetAnonyString(deviceId).c_str());
    if (networkId.empty() || deviceId.empty()) {
        LOGE("NetworkId or DeviceId is empty");
        RadarParaInfo info = {"UMountDfsDocs", ReportLevel::INNER, DfxBizStage::SOFTBUS_OPENP2P,
            DEFAULT_PKGNAME, networkId, ERR_BAD_VALUE, "NetworkId or DeviceId is empty"};
        RadarReportAdapter::GetInstance().ReportGenerateDisUriAdapter(info);
        return FileManagement::ERR_BAD_VALUE;
    }
    int32_t ret = NO_ERROR;
    if (UMountDfsCountOnly(deviceId, needClear)) {
        LOGW("do not need umount");
        return ret;
    }
    int32_t userId = currentUserId_;
    if (userId == INVALID_USER_ID) {
        LOGE("GetCurrentUserId Fail");
        RadarParaInfo info = {"UMountDfsDocs", ReportLevel::INNER, DfxBizStage::SOFTBUS_OPENP2P,
            DEFAULT_PKGNAME, networkId, ERR_BAD_VALUE, "GetCurrentUserId Fail"};
        RadarReportAdapter::GetInstance().ReportGenerateDisUriAdapter(info);
        return FileManagement::ERR_BAD_VALUE;
    }
    auto storageMgrProxy = GetStorageManager();
    if (storageMgrProxy == nullptr) {
        LOGE("storageMgrProxy is null");
        RadarParaInfo info = {"UMountDfsDocs", ReportLevel::INNER, DfxBizStage::SOFTBUS_OPENP2P,
            DEFAULT_PKGNAME, networkId, ERR_BAD_VALUE, "storageMgrProxy is null"};
        RadarReportAdapter::GetInstance().ReportGenerateDisUriAdapter(info);
        return FileManagement::ERR_BAD_VALUE;
    }
    ret = storageMgrProxy->UMountDfsDocs(userId, "account", networkId, deviceId);
    if (ret != NO_ERROR) {
        LOGE("UMountDfsDocs fail, ret = %{public}d", ret);
        RadarParaInfo info = {"UMountDfsDocs", ReportLevel::INNER, DfxBizStage::SOFTBUS_OPENP2P,
            "storage", networkId, ret, "UMountDfsDocs fail"};
        RadarReportAdapter::GetInstance().ReportGenerateDisUriAdapter(info);
    } else {
        LOGI("UMountDfsDocs success, deviceId %{public}s erase count",
            Utils::GetAnonyString(deviceId).c_str());
        RemoveMountDfsCount(deviceId);
        ConnectCount::GetInstance().NotifyFileStatusChange(networkId, Status::DISCONNECT_OK,
                                                           MOUNT_PATH + networkId.substr(0, VALID_MOUNT_PATH_LEN),
                                                           StatusType::CONNECTION_STATUS);
    }
    LOGI("storageMgr.UMountDfsDocs end.");
    return ret;
}

void DeviceManagerAgent::IncreaseMountDfsCount(const std::string &networkId,
    const std::string &mountPath, const uint32_t callingTokenId)
{
    std::lock_guard<std::mutex> lock(mountDfsCountMutex_);
    auto iter = mountDfsCount_.find(mountPath);
    if (iter == mountDfsCount_.end()) {
        MountCountInfo tmpInfo(networkId, callingTokenId);
        mountDfsCount_.emplace(mountPath, tmpInfo);
    } else {
        auto &callingCountMap = iter->second.callingCountMap_;
        auto callingIter = callingCountMap.find(callingTokenId);
        if (callingIter != callingCountMap.end()) {
            ++callingIter->second;
            LOGI("Mountpath %{public}s has already established more than one link, \
                count %{public}d, Increase count by one now",
                Utils::GetAnonyString(mountPath).c_str(), callingIter->second);
        } else {
            callingCountMap.emplace(callingTokenId, MOUNT_DFS_COUNT_ONE);
        }
    }
}

void DeviceManagerAgent::RemoveMountDfsCount(const std::string &mountPath)
{
    std::lock_guard<std::mutex> lock(mountDfsCountMutex_);
    auto iter = mountDfsCount_.find(mountPath);
    if (iter == mountDfsCount_.end()) {
        LOGE("Can not find mountPath: %{public}s", mountPath.c_str());
        return;
    }
    mountDfsCount_.erase(mountPath);
}

void DeviceManagerAgent::GetConnectedDeviceList(std::vector<DfsDeviceInfo> &deviceList)
{
    std::lock_guard<std::mutex> lock(mountDfsCountMutex_);
    for (const auto &ele : mountDfsCount_) {
        deviceList.emplace_back(DfsDeviceInfo(ele.second.networkId_, MOUNT_PATH + ele.first));
    }
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
    if (jsonObject.find(FIELD_GROUP_NAME) != jsonObject.end() && jsonObject[FIELD_GROUP_NAME].is_string()) {
        groupInfo.groupName = jsonObject.at(FIELD_GROUP_NAME).get<std::string>();
    }

    if (jsonObject.find(FIELD_GROUP_ID) != jsonObject.end() && jsonObject[FIELD_GROUP_ID].is_string()) {
        groupInfo.groupId = jsonObject.at(FIELD_GROUP_ID).get<std::string>();
    }

    if (jsonObject.find(FIELD_GROUP_OWNER) != jsonObject.end() && jsonObject[FIELD_GROUP_OWNER].is_string()) {
        groupInfo.groupOwner = jsonObject.at(FIELD_GROUP_OWNER).get<std::string>();
    }

    if (jsonObject.find(FIELD_GROUP_TYPE) != jsonObject.end() && jsonObject[FIELD_GROUP_TYPE].is_number()) {
        groupInfo.groupType = jsonObject.at(FIELD_GROUP_TYPE).get<int32_t>();
    }
}

void DeviceManagerAgent::QueryRelatedGroups(const std::string &udid, const std::string &networkId)
{
    unique_lock<mutex> lock(mpToNetworksMutex_);
    auto network = FindNetworkBaseTrustRelation(false);
    if (network != nullptr) {
        cidNetTypeRecord_.insert({ networkId, network });
    }
}

int32_t DeviceManagerAgent::IsSupportedDevice(const DistributedHardware::DmDeviceInfo &deviceInfo)
{
    std::vector<DistributedHardware::DmDeviceInfo> deviceList;
    DistributedHardware::DeviceManager::GetInstance().GetTrustedDeviceList(IDaemon::SERVICE_NAME, "", deviceList);
    if (deviceList.size() == 0 || deviceList.size() > MAX_ONLINE_DEVICE_SIZE) {
        LOGE("trust device list size is invalid, size=%zu", deviceList.size());
        RadarParaInfo info = {"IsSupportedDevice", ReportLevel::INNER, DfxBizStage::SOFTBUS_OPENP2P,
            "devicemanager", deviceInfo.networkId, ERR_BAD_VALUE, "list size is invalid"};
        RadarReportAdapter::GetInstance().ReportLinkConnectionAdapter(info);
        return FileManagement::ERR_BAD_VALUE;
    }
    DistributedHardware::DmDeviceInfo infoTemp;
    for (const auto &info : deviceList) {
        if (std::string_view(info.networkId) == std::string_view(deviceInfo.networkId)) {
            infoTemp = info;
            break;
        }
    }

    if (infoTemp.extraData.empty()) {
        LOGE("extraData is empty");
        RADAR_REPORT(RadarReporter::DFX_SET_DFS, RadarReporter::DFX_BUILD__LINK, RadarReporter::DFX_FAILED,
            RadarReporter::BIZ_STATE, RadarReporter::DFX_END, RadarReporter::ERROR_CODE,
            RadarReporter::GET_SAME_ACCOUNT_ERROR, RadarReporter::PACKAGE_NAME, RadarReporter::deviceManager);
        return FileManagement::ERR_BAD_VALUE;
    }
    nlohmann::json entraDataJson = nlohmann::json::parse(infoTemp.extraData, nullptr, false);
    if (entraDataJson.is_discarded()) {
        LOGE("entraDataJson parse failed.");
        RadarParaInfo info = {"IsSupportedDevice", ReportLevel::INNER, DfxBizStage::SOFTBUS_OPENP2P,
            DEFAULT_PKGNAME, deviceInfo.networkId, ERR_BAD_VALUE, "entraDataJson parse failed."};
        RadarReportAdapter::GetInstance().ReportLinkConnectionAdapter(info);
        return FileManagement::ERR_BAD_VALUE;
    }
    if (!Utils::IsInt32(entraDataJson, PARAM_KEY_OS_TYPE)) {
        LOGE("error json int32_t param.");
        RadarParaInfo info = {"IsSupportedDevice", ReportLevel::INNER, DfxBizStage::SOFTBUS_OPENP2P,
            DEFAULT_PKGNAME, deviceInfo.networkId, ERR_BAD_VALUE, "error json int32_t param"};
        RadarReportAdapter::GetInstance().ReportLinkConnectionAdapter(info);
        return FileManagement::ERR_BAD_VALUE;
    }
    int32_t osType = entraDataJson[PARAM_KEY_OS_TYPE].get<int32_t>();
    if (osType != DEVICE_OS_TYPE_OH) {
        LOGE("%{private}s  the device os type = %{private}d is not openharmony.",
            Utils::GetAnonyString(infoTemp.deviceId).c_str(), osType);
        RadarParaInfo info = {"IsSupportedDevice", ReportLevel::INNER, DfxBizStage::SOFTBUS_OPENP2P,
            DEFAULT_PKGNAME, deviceInfo.networkId, ERR_BAD_VALUE, "type is not openharmony"};
        RadarReportAdapter::GetInstance().ReportLinkConnectionAdapter(info);
        return FileManagement::ERR_BAD_VALUE;
    }
    return FileManagement::ERR_OK;
}

void DeviceManagerAgent::InitLocalNodeInfo()
{
    auto &deviceManager = DistributedHardware::DeviceManager::GetInstance();
    DistributedHardware::DmDeviceInfo localDeviceInfo{};
    int errCode = deviceManager.GetLocalDeviceInfo(IDaemon::SERVICE_NAME, localDeviceInfo);
    if (errCode != 0) {
        ThrowException(errCode, "Failed to get info of local devices");
    }
    localDeviceInfo_.SetCid(string(localDeviceInfo.networkId));
}

void DeviceManagerAgent::OnRemoteDied()
{
    LOGI("device manager service died");
}

DeviceInfo &DeviceManagerAgent::GetLocalDeviceInfo()
{
    return localDeviceInfo_;
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

std::unordered_map<std::string, MountCountInfo> DeviceManagerAgent::GetAllMountInfo()
{
    std::lock_guard<std::mutex> lock(mountDfsCountMutex_);
    return mountDfsCount_;
}

void DeviceManagerAgent::UMountDisShareFile()
{
    LOGI("UMountDisShareFile begin");
    int32_t ret = NO_ERROR;
    int32_t userId = GetCurrentUserId();
    auto localNetworkId = GetLocalDeviceInfo().GetCid();
    if (userId == INVALID_USER_ID || localNetworkId.empty()) {
        LOGE("userId or networkId is invalid");
        return;
    }
    auto storageMgrProxy = GetStorageManager();
    if (storageMgrProxy == nullptr) {
        LOGE("storageMgrProxy is null");
        return;
    }
    ret = storageMgrProxy->UMountDisShareFile(userId, localNetworkId);
    LOGI("UMountDisShareFile end, ret = %{public}d", ret);
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
