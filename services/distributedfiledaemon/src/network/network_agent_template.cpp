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
#include "dfsu_exception.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
using namespace std;
namespace {
constexpr int MAX_RETRY_COUNT = 7;
constexpr int OPEN_SESSSION_DELAY_TIME = 100;
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
    std::this_thread::sleep_for(std::chrono::milliseconds(
        OPEN_SESSSION_DELAY_TIME)); // Temporary workaround for time sequence issues(offline-onSessionOpened)
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
        auto cmd = make_unique<DfsuCmd<NetworkAgentTemplate, const DeviceInfo>>(
            &NetworkAgentTemplate::ConnectDeviceAsync, info);
        cmd->UpdateOption({.tryTimes_ = MAX_RETRY_COUNT});
        Recv(move(cmd));
    }
}

void NetworkAgentTemplate::DisconnectAllDevices()
{
    sessionPool_.ReleaseAllSession();
}

void NetworkAgentTemplate::DisconnectDevice(const DeviceInfo info)
{
    LOGI("DeviceOffline, cid:%{public}s", info.GetCid().c_str());
    sessionPool_.ReleaseSession(info.GetCid(), LINK_TYPE_AP);
}

void NetworkAgentTemplate::DisconnectDeviceByP2P(const DeviceInfo info)
{
    LOGI("DeviceOffline, cid:%{public}s", info.GetCid().c_str());
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

void NetworkAgentTemplate::CloseSessionForOneDevice(const string &cid)
{
    (void)cid;
    LOGI("session closed!");
}

void NetworkAgentTemplate::AcceptSession(shared_ptr<BaseSession> session, const std::string backStage)
{
    LOGI("AcceptSession Enter");
    auto cmd = make_unique<DfsuCmd<NetworkAgentTemplate, shared_ptr<BaseSession>, const std::string>>(
        &NetworkAgentTemplate::AcceptSessionInner, session, backStage);
    cmd->UpdateOption({.tryTimes_ = 1});
    Recv(move(cmd));
}

void NetworkAgentTemplate::AcceptSessionInner(shared_ptr<BaseSession> session, const std::string backStage)
{
    auto cid = session->GetCid();
    LOGI("AcceptSesion, cid:%{public}s", cid.c_str());
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
    LOGI("NOTIFY_GET_SESSION, old fd %{public}d, remote cid %{public}s", fd, cidStr.c_str());
    uint8_t linkType = sessionPool_.ReleaseSession(fd);
    GetSession(cidStr, linkType);
}

void NetworkAgentTemplate::GetSession(const string &cid, uint8_t linkType)
{
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
            OpenSession(deviceInfo, LINK_TYPE_P2P);
        } catch (const DfsuException &e) {
            LOGE("reget session failed, code: %{public}d", e.code());
        }
    }
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
