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

#include "network/network_agent_template.h"

#include "device/device_manager_agent.h"
#include "dfs_error.h"
#include "dfsu_exception.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
using namespace std;
namespace {
constexpr int32_t DEVICE_OS_TYPE_OH = 10;
constexpr int OPEN_SESSSION_DELAY_TIME = 100;
constexpr int32_t NOTIFY_GET_SESSION_WAITING_TIME = 2;
constexpr int32_t WAITING_REMOTE_SA_ONLINE = 5;
constexpr const char* PARAM_KEY_OS_TYPE = "OS_TYPE";
} // namespace

void NetworkAgentTemplate::Start()
{
    LOGI("Start Enter");
    JoinDomain();
    kernerlTalker_->CreatePollThread();
    ConnectOnlineDevices();
}

void NetworkAgentTemplate::Stop()
{
    LOGI("Stop Enter");
    StopTopHalf();
    StopBottomHalf();
    kernerlTalker_->WaitForPollThreadExited();
}

void NetworkAgentTemplate::ConnectDeviceAsync(const DeviceInfo info)
{
    LOGI("ConnectDeviceAsync Enter");
    std::this_thread::sleep_for(std::chrono::seconds(WAITING_REMOTE_SA_ONLINE));
    OpenApSession(info, LINK_TYPE_AP);
}

void NetworkAgentTemplate::ConnectDeviceByP2PAsync(const DeviceInfo info)
{
    LOGI("ConnectDeviceByP2PAsync Enter");
    std::this_thread::sleep_for(std::chrono::milliseconds(OPEN_SESSSION_DELAY_TIME));
    OpenSession(info, LINK_TYPE_P2P);
}

void NetworkAgentTemplate::ConnectOnlineDevices()
{
    string pkgName = IDaemon::SERVICE_NAME;
    auto &deviceManager = DistributedHardware::DeviceManager::GetInstance();

    auto dma = DeviceManagerAgent::GetInstance();
    auto infos = dma->GetRemoteDevicesInfo();
    LOGI("Have %{public}zu devices Online", infos.size());
    for (const auto &info : infos) {
        if (info.GetExtraData().empty()) {
            LOGE("extraData is empty.");
            return;
        }
        nlohmann::json entraDataJson = nlohmann::json::parse(info.GetExtraData(), nullptr, false);
        if (entraDataJson.is_discarded()) {
            LOGE("entraDataJson parse failed.");
            return;
        }
        if (!Utils::IsInt32(entraDataJson, PARAM_KEY_OS_TYPE)) {
            LOGE("error json int32_t param.");
            return;
        }
        int32_t osType = entraDataJson[PARAM_KEY_OS_TYPE].get<int32_t>();
        if (osType != DEVICE_OS_TYPE_OH) {
            LOGE("%{private}s  the device os type = %{private}d is not openharmony.",
                Utils::GetAnonyString(info.GetCid()).c_str(), osType);
            return;
        }

        int32_t networkType;
        int errCode = deviceManager.GetNetworkTypeByNetworkId(pkgName, info.GetCid(), networkType);
        if (errCode) {
            LOGE("failed to get network type by network id errCode = %{public}d", errCode);
            continue;
        }
        if (!(static_cast<uint32_t>(networkType) & (1 << DistributedHardware::BIT_NETWORK_TYPE_WIFI))) {
            LOGI("not wifi network networkType = %{public}d == %{public}d", networkType,
                 1 << DistributedHardware::BIT_NETWORK_TYPE_WIFI);
            continue;
        }
    }
}

void NetworkAgentTemplate::DisconnectAllDevices()
{
    sessionPool_.ReleaseAllSession();
}

void NetworkAgentTemplate::DisconnectDevice(const DeviceInfo info)
{
    LOGI("DeviceOffline, cid:%{public}s", Utils::GetAnonyString(info.GetCid()).c_str());
    sessionPool_.ReleaseSession(info.GetCid(), LINK_TYPE_AP);
}

void NetworkAgentTemplate::DisconnectDeviceByP2P(const DeviceInfo info)
{
    LOGI("DeviceOffline, cid:%{public}s", Utils::GetAnonyString(info.GetCid()).c_str());
    sessionPool_.ReleaseSession(info.GetCid(), LINK_TYPE_P2P);
}

void NetworkAgentTemplate::DisconnectDeviceByP2PHmdfs(const DeviceInfo info)
{
    LOGI("DeviceOffline, cid:%{public}s", Utils::GetAnonyString(info.GetCid()).c_str());
    sessionPool_.DeviceDisconnectCountOnly(info.GetCid(), LINK_TYPE_P2P, true);
    sessionPool_.ReleaseSession(info.GetCid(), LINK_TYPE_P2P);
}

void NetworkAgentTemplate::OccupySession(int32_t sessionId, uint8_t linkType)
{
    sessionPool_.OccupySession(sessionId, linkType);
}

bool NetworkAgentTemplate::FindSession(int32_t sessionId)
{
    return sessionPool_.FindSession(sessionId);
}

void NetworkAgentTemplate::CloseSessionForOneDevice(int32_t fd)
{
    if (fd > 0) {
        sessionPool_.ReleaseSession(fd);
        LOGI("session closed");
    }
}

void NetworkAgentTemplate::AcceptSession(shared_ptr<BaseSession> session, const std::string backStage)
{
    auto cid = session->GetCid();
    LOGI("AcceptSesion, cid:%{public}s", Utils::GetAnonyString(cid).c_str());
    sessionPool_.HoldSession(session, backStage);
}

void NetworkAgentTemplate::GetSessionProcess(NotifyParam &param)
{
    auto cmd = make_unique<DfsuCmd<NetworkAgentTemplate, NotifyParam>>(
        &NetworkAgentTemplate::GetSessionProcessInner, param);
    cmd->UpdateOption({.tryTimes_ = 1});
    Recv(move(cmd));
}

void NetworkAgentTemplate::GetSessionProcessInner(NotifyParam param)
{
    string cidStr(param.remoteCid, CID_MAX_LEN);
    int fd = param.fd;
    LOGI("NOTIFY_GET_SESSION, old fd %{public}d, remote cid %{public}s", fd, Utils::GetAnonyString(cidStr).c_str());
    uint8_t linkType = sessionPool_.ReleaseSession(fd);
    GetSession(cidStr, linkType);
}

void NetworkAgentTemplate::GetSession(const string &cid, uint8_t linkType)
{
    std::this_thread::sleep_for(std::chrono::seconds(NOTIFY_GET_SESSION_WAITING_TIME));
    DeviceInfo deviceInfo;
    deviceInfo.SetCid(cid);
    if (linkType == LINK_TYPE_AP) {
        try {
            OpenApSession(deviceInfo, LINK_TYPE_AP);
        } catch (const DfsuException &e) {
            LOGE("reget session failed, code: %{public}d", e.code());
        }
    } else if (linkType == LINK_TYPE_P2P) {
        try {
            if (OpenSession(deviceInfo, LINK_TYPE_P2P) == FileManagement::E_OK) {
                return;
            }
            LOGE("reget session failed");
            auto deviceManager = DeviceManagerAgent::GetInstance();
            deviceManager->RemoveNetworkIdForAllToken(cid);
            auto deviceId = deviceManager->GetDeviceIdByNetworkId(cid);
            if (!deviceId.empty()) {
                deviceManager->UMountDfsDocs(cid, deviceId, true);
            }
            sessionPool_.DeviceDisconnectCountOnly(cid, linkType, true);
            deviceManager->NotifyRemoteReverseObj(cid, ON_STATUS_OFFLINE);
            deviceManager->RemoveRemoteReverseObj(true, 0);
        } catch (const DfsuException &e) {
            LOGE("reget session failed, code: %{public}d", e.code());
        }
    }
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
