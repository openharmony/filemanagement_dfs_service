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

#include "network/softbus/softbus_agent.h"

#include <sstream>

#include "device_manager.h"
#include "dfs_daemon_event_dfx.h"
#include "dfs_error.h"
#include "dfs_radar.h"
#include "dfsu_exception.h"
#include "dm_device_info.h"
#include "ipc_skeleton.h"
#include "ipc/i_daemon.h"
#include "network/softbus/softbus_session_dispatcher.h"
#include "network/softbus/softbus_session_name.h"
#include "network/softbus/softbus_session.h"
#include "softbus_error_code.h"
#include "utils_directory.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace {
    constexpr int MAX_RETRY_COUNT = 7;
}
using namespace std;
const int32_t DFS_QOS_TYPE_MIN_BW = 90 * 1024 * 1024;
const int32_t DFS_QOS_TYPE_MAX_LATENCY = 10000;
const int32_t DFS_QOS_TYPE_MIN_LATENCY = 2000;
const std::string SESSION_NAME = "DistributedFileService/mnt/hmdfs/100/account";
SoftbusAgent::SoftbusAgent(weak_ptr<MountPoint> mountPoint) : NetworkAgentTemplate(mountPoint)
{
    sessionName_ = SESSION_NAME;
}

int32_t SoftbusAgent::JudgeNetworkTypeIsWifi(const DeviceInfo &info)
{
    int32_t networkType;
    auto &deviceManager = DistributedHardware::DeviceManager::GetInstance();
    int errCode = deviceManager.GetNetworkTypeByNetworkId(IDaemon::SERVICE_NAME, info.GetCid(), networkType);
    if (errCode) {
        LOGE("failed to get network type by network id errCode = %{public}d", errCode);
        return FileManagement::ERR_BAD_VALUE;
    }
    if (!(static_cast<uint32_t>(networkType) & (1 << DistributedHardware::BIT_NETWORK_TYPE_WIFI))) {
        LOGI("not wifi network networkType = %{public}d == %{public}d", networkType,
             1 << DistributedHardware::BIT_NETWORK_TYPE_WIFI);
        return FileManagement::ERR_BAD_VALUE;
    }
    return FileManagement::ERR_OK;
}

void SoftbusAgent::JoinDomain()
{
    LOGI("JoinDomain Enter.");
    ISocketListener sessionListener = {
        .OnBind = SoftbusSessionDispatcher::OnSessionOpened,
        .OnShutdown = SoftbusSessionDispatcher::OnSessionClosed,
        .OnBytes = nullptr,
        .OnMessage = nullptr,
        .OnStream = nullptr,
        .OnNegotiate2 = SoftbusSessionDispatcher::OnNegotiate2,
    };

    SoftbusSessionDispatcher::RegisterSessionListener(sessionName_, shared_from_this());
    SocketInfo serverInfo = {
        .name = const_cast<char*>(sessionName_.c_str()),
        .pkgName = const_cast<char*>(IDaemon::SERVICE_NAME.c_str()),
        .dataType = DATA_TYPE_BYTES,
    };
    int32_t socketId = Socket(serverInfo);
    if (socketId < 0) {
        LOGE("Create Socket fail socketId, socketId = %{public}d", socketId);
        RadarParaInfo info = {"JoinDomain", ReportLevel::INNER, DfxBizStage::SOFTBUS_OPENP2P,
            "softbus", "", socketId, "Create Socket fail"};
        DfsRadar::GetInstance().ReportLinkConnection(info);
        return;
    }
    QosTV qos[] = {
        {.qos = QOS_TYPE_MIN_BW,        .value = DFS_QOS_TYPE_MIN_BW},
        {.qos = QOS_TYPE_MAX_LATENCY,        .value = DFS_QOS_TYPE_MAX_LATENCY},
        {.qos = QOS_TYPE_MIN_LATENCY,        .value = DFS_QOS_TYPE_MIN_LATENCY},
    };

    int32_t ret = Listen(socketId, qos, sizeof(qos) / sizeof(qos[0]), &sessionListener);
    if (ret != FileManagement::E_OK) {
        Shutdown(socketId);
        stringstream ss;
        ss << "Failed to CreateSessionServer, errno:" << ret;
        LOGE("%{public}s, sessionName:%{public}s", ss.str().c_str(), sessionName_.c_str());
        RadarParaInfo info = {"JoinDomain", ReportLevel::INNER, DfxBizStage::SOFTBUS_OPENP2P,
            "softbus", "", ret, "Failed to CreateSessionServer"};
        DfsRadar::GetInstance().ReportLinkConnection(info);
        throw runtime_error(ss.str());
    }
    {
        std::lock_guard<std::mutex> lock(serverIdMapMutex_);
        serverIdMap_.insert(std::make_pair(sessionName_, socketId));
    }
    LOGI("Succeed to JoinDomain, busName:%{public}s", sessionName_.c_str());
}

void SoftbusAgent::QuitDomain()
{
    std::lock_guard<std::mutex> lock(serverIdMapMutex_);
    if (!serverIdMap_.empty()) {
        for (auto it = serverIdMap_.begin(); it != serverIdMap_.end(); it++) {
            if ((it->first).find(sessionName_) != std::string::npos) {
                int32_t serverId = serverIdMap_[sessionName_];
                Shutdown(serverId);
                LOGI("RemoveSessionServer success.");
            }
        }
    }
    SoftbusSessionDispatcher::UnregisterSessionListener(sessionName_.c_str());
    LOGI("Succeed to QuitDomain, busName:%{public}s", sessionName_.c_str());
}

void SoftbusAgent::StopTopHalf()
{
    QuitDomain();
}

void SoftbusAgent::StopBottomHalf() {}

int32_t SoftbusAgent::OpenSessionInner(const DeviceInfo &info)
{
    ISocketListener sessionListener = {
        .OnBind = SoftbusSessionDispatcher::OnSessionOpened,
        .OnShutdown = SoftbusSessionDispatcher::OnSessionClosed,
        .OnBytes = nullptr,
        .OnMessage = nullptr,
        .OnStream = nullptr,
    };
    QosTV qos[] = {
        {.qos = QOS_TYPE_MIN_BW,        .value = DFS_QOS_TYPE_MIN_BW},
        {.qos = QOS_TYPE_MAX_LATENCY,        .value = DFS_QOS_TYPE_MAX_LATENCY},
        {.qos = QOS_TYPE_MIN_LATENCY,        .value = DFS_QOS_TYPE_MIN_LATENCY},
    };
    SocketInfo clientInfo = {
        .name = const_cast<char*>((sessionName_.c_str())),
        .peerName = const_cast<char*>(sessionName_.c_str()),
        .peerNetworkId = const_cast<char*>(info.GetCid().c_str()),
        .pkgName = const_cast<char*>(IDaemon::SERVICE_NAME.c_str()),
        .dataType = DATA_TYPE_BYTES,
    };
    int32_t socketId = Socket(clientInfo);
    if (socketId < FileManagement::E_OK) {
        LOGE("Create OpenSoftbusChannel Socket error");
        RadarParaInfo radarInfo = {"OpenSessionInner", ReportLevel::INNER, DfxBizStage::SOFTBUS_OPENP2P,
            "softbus", info.GetCid(), socketId, "Create Socket error"};
        DfsRadar::GetInstance().ReportLinkConnection(radarInfo);
        return FileManagement::E_CONTEXT;
    }
    if (FindSocketId(socketId)) {
        LOGW("Has find socketId:%{public}d", socketId);
        return FileManagement::E_OK;
    }
    if (!SoftBusPermissionCheck::SetAccessInfoToSocket(socketId)) {
        LOGW("Fill and set accessInfo failed");
        Shutdown(socketId);
        return FileManagement::E_CONTEXT;
    }
    int32_t ret = Bind(socketId, qos, sizeof(qos) / sizeof(qos[0]), &sessionListener);
    if (ret != FileManagement::E_OK) {
        LOGE("Bind SocketClient error");
        Shutdown(socketId);
        RadarParaInfo radarInfo = {"OpenSessionInner", ReportLevel::INNER, DfxBizStage::SOFTBUS_OPENP2P,
            "softbus", info.GetCid(), ret, "Bind SocketClient error"};
        DfsRadar::GetInstance().ReportLinkConnection(radarInfo);
        return FileManagement::E_CONTEXT;
    }
    return E_OK;
}

int32_t SoftbusAgent::OpenSession(const DeviceInfo &info, const uint8_t &linkType)
{
    LOGI("Start to OpenSession, cid:%{public}s", Utils::GetAnonyString(info.GetCid()).c_str());
    if (!SoftBusPermissionCheck::CheckSrcPermission(info.GetCid())) {
        return FileManagement::E_PERMISSION_DENIED;
    }
    auto res = OpenSessionInner(info);
    if (res != E_OK) {
        return res;
    }
    HandleAfterOpenSession(socketId, info.GetCid());
    return FileManagement::E_OK;
}

void SoftbusAgent::HandleAfterOpenSession(const int32_t socketId, const std::string &networkId)
{
    auto session = make_shared<SoftbusSession>(socketId, networkId);
    session->DisableSessionListener();
    session->SetFromServer(false);
    AcceptSession(session, "Client");
    LOGI("Suc OpenSession socketId:%{public}d, cid:%{public}s", socketId, Utils::GetAnonyString(networkId).c_str());
}

void SoftbusAgent::CloseSession(shared_ptr<BaseSession> session)
{
    if (session == nullptr) {
        LOGE("Failed to close session, error:invalid session");
        return;
    }
    session->Release();
}

bool SoftbusAgent::IsContinueRetry(const string &cid)
{
    auto retriedTimesMap = OpenSessionRetriedTimesMap_.find(cid);
    if (retriedTimesMap != OpenSessionRetriedTimesMap_.end()) {
        if (retriedTimesMap->second >= MAX_RETRY_COUNT) {
            return false;
        }
    } else {
        OpenSessionRetriedTimesMap_[cid] = 0;
    }
    OpenSessionRetriedTimesMap_[cid]++;
    return true;
}

void SoftbusAgent::OnSessionOpened(const int32_t sessionId, PeerSocketInfo info)
{
    LOGI("OnSessionOpened sessionId = %{public}d", sessionId);
    std::string peerDeviceId = info.networkId;
    auto session = make_shared<SoftbusSession>(sessionId, peerDeviceId);
    session->DisableSessionListener();
    session->SetFromServer(true);
    AcceptSession(session, "Server");
}

void SoftbusAgent::OnSessionClosed(int32_t sessionId, const std::string peerDeviceId)
{
    LOGI("OnSessionClosed Enter.");
    CloseSessionForOneDevice(peerDeviceId);
}

bool SoftbusAgent::OnNegotiate2(int32_t socket, PeerSocketInfo info,
    SocketAccessInfo *peerInfo, SocketAccessInfo *localInfo)
{
    AccountInfo callerAccountInfo;
    std::string networkId = info.networkId;
    if (!SoftBusPermissionCheck::TransCallerInfo(peerInfo, callerAccountInfo, networkId)) {
        LOGE("Trans caller info failed.");
        return false;
    }
    if (!SoftBusPermissionCheck::FillLocalInfo(localInfo)) {
        LOGE("FillLocalInfo failed.");
        return false;
    }
    return SoftBusPermissionCheck::CheckSinkPermission(callerAccountInfo);
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
