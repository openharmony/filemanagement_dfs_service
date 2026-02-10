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

#include "network/network_agent_template.h"

#include "connect_count/connect_count.h"
#include "device/device_manager_agent.h"
#include "dfs_error.h"
#include "dfsu_exception.h"
#include "system_notifier.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
using namespace std;
namespace {
constexpr int OPEN_SESSSION_DELAY_TIME = 100;
constexpr int32_t NOTIFY_GET_SESSION_WAITING_TIME = 2;
constexpr int32_t VALID_MOUNT_PATH_LEN = 16;
} // namespace

void NetworkAgentTemplate::Start()
{
    if (kernerlTalker_ == nullptr) {
        LOGE("kernerlTalker_ is nullptr.");
        return;
    }
    LOGI("Start Enter");
    JoinDomain();
    kernerlTalker_->CreatePollThread();
}

void NetworkAgentTemplate::Stop()
{
    if (kernerlTalker_ == nullptr) {
        LOGE("kernerlTalker_ is nullptr.");
        return;
    }
    LOGI("Stop Enter");
    StopTopHalf();
    StopBottomHalf();
    kernerlTalker_->WaitForPollThreadExited();
}

void NetworkAgentTemplate::ConnectDeviceByP2PAsync(const DeviceInfo info)
{
    LOGI("ConnectDeviceByP2PAsync Enter");
    std::this_thread::sleep_for(std::chrono::milliseconds(OPEN_SESSSION_DELAY_TIME));
    OpenSession(info, LINK_TYPE_P2P);
}

void NetworkAgentTemplate::DisconnectAllDevices()
{
    sessionPool_.ReleaseAllSession();
    ConnectCount::GetInstance().RemoveAllConnect();
}

// for closeP2P
void NetworkAgentTemplate::DisconnectDeviceByP2P(const std::string networkId)
{
    LOGI("CloseP2P, networkId:%{public}s", Utils::GetAnonyString(networkId).c_str());
    sessionPool_.ReleaseSession(networkId, false);
}

// dm device offline
void NetworkAgentTemplate::DisconnectDeviceByP2PHmdfs(const std::string networkId)
{
    LOGI("DeviceOffline, networkId:%{public}s", Utils::GetAnonyString(networkId).c_str());
    sessionPool_.ReleaseSession(networkId, true);
    ConnectCount::GetInstance().NotifyRemoteReverseObj(networkId, ON_STATUS_OFFLINE);
    ConnectCount::GetInstance().RemoveConnect(networkId);
    ConnectCount::GetInstance().NotifyFileStatusChange(networkId, Status::DEVICE_OFFLINE,
                                                       networkId.substr(0, VALID_MOUNT_PATH_LEN),
                                                       StatusType::CONNECTION_STATUS);
}

// softbus offline, allConnect offline, hmdfs never has socket
void NetworkAgentTemplate::CloseSessionForOneDevice(const string &cid)
{
    auto cmd = make_unique<DfsuCmd<NetworkAgentTemplate, std::string>>(
        &NetworkAgentTemplate::CloseSessionForOneDeviceInner, cid);
    cmd->UpdateOption({.tryTimes_ = 1});
    Recv(move(cmd));
}

void NetworkAgentTemplate::CloseSessionForOneDeviceInner(std::string cid)
{
    sessionPool_.ReleaseSession(cid, true);
    ConnectCount::GetInstance().NotifyRemoteReverseObj(cid, ON_STATUS_OFFLINE);
    ConnectCount::GetInstance().NotifyFileStatusChange(
        cid, Status::DEVICE_OFFLINE, cid.substr(0, VALID_MOUNT_PATH_LEN), StatusType::CONNECTION_STATUS);
    ConnectCount::GetInstance().RemoveConnect(cid);
}

void NetworkAgentTemplate::AcceptSession(shared_ptr<BaseSession> session, const std::string backStage)
{
    if (session == nullptr) {
        LOGE("session is nullptr.");
        return;
    }
    auto cid = session->GetCid();
    LOGI("AcceptSesion, cid:%{public}s", Utils::GetAnonyString(cid).c_str());
    sessionPool_.HoldSession(session, backStage);
}

bool NetworkAgentTemplate::FindSocketId(int32_t socketId)
{
    return sessionPool_.FindSocketId(socketId);
}

// hmdfs offline
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
    bool isServer = false;
    bool ifGetSession = sessionPool_.CheckIfGetSession(fd, isServer);
    sessionPool_.ReleaseSession(fd);
    if (ifGetSession && ConnectCount::GetInstance().CheckCount(cidStr)) {
        // for client
        GetSession(cidStr);
    } else {
        // for server and client active close
        sessionPool_.SinkOffline(cidStr);
    }
    if (isServer) {
        SystemNotifier::GetInstance().DestroyNotifyByNetworkId(cidStr, false);
    }
}

void NetworkAgentTemplate::GetSession(const string &cid)
{
    std::this_thread::sleep_for(std::chrono::seconds(NOTIFY_GET_SESSION_WAITING_TIME));
    DeviceInfo deviceInfo;
    deviceInfo.SetCid(cid);
    try {
        if (OpenSession(deviceInfo, LINK_TYPE_P2P) == FileManagement::E_OK) {
            return;
        }
        LOGE("reget session failed");
        sessionPool_.SinkOffline(cid);
        ConnectCount::GetInstance().NotifyRemoteReverseObj(cid, ON_STATUS_OFFLINE);
        ConnectCount::GetInstance().RemoveConnect(cid);
        DeviceManagerAgent::GetInstance()->UMountDfsDocs(cid, cid.substr(0, VALID_MOUNT_PATH_LEN), true);
    } catch (const DfsuException &e) {
        LOGE("reget session failed, code: %{public}d", e.code());
    }
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
