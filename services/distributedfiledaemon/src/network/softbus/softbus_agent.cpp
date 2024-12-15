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

#include "network/softbus/softbus_agent.h"

#include <sstream>

#include "device_manager.h"
#include "dfs_daemon_event_dfx.h"
#include "dfs_error.h"
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
#ifdef SUPPORT_SAME_ACCOUNT
const uint32_t MAX_ONLINE_DEVICE_SIZE = 10000;
#endif
SoftbusAgent::SoftbusAgent(weak_ptr<MountPoint> mountPoint) : NetworkAgentTemplate(mountPoint)
{
    auto spt = mountPoint.lock();
    if (spt == nullptr) {
        LOGE("mountPoint is not exist! bad weak_ptr");
        sessionName_ = "";
        return;
    }

    string path = spt->GetMountArgument().GetFullDst();
    SoftbusSessionName sessionName(path);
    sessionName_ = sessionName.ToString();
}

bool SoftbusAgent::IsSameAccount(const std::string &networkId)
{
#ifdef SUPPORT_SAME_ACCOUNT
    std::vector<DistributedHardware::DmDeviceInfo> deviceList;
    DistributedHardware::DeviceManager::GetInstance().GetTrustedDeviceList(IDaemon::SERVICE_NAME, "", deviceList);
    if (deviceList.size() == 0 || deviceList.size() > MAX_ONLINE_DEVICE_SIZE) {
        LOGE("trust device list size is invalid, size=%zu", deviceList.size());
        return false;
    }
    for (const auto &deviceInfo : deviceList) {
        if (std::string(deviceInfo.networkId) == networkId) {
            return (deviceInfo.authForm == DistributedHardware::DmAuthForm::IDENTICAL_ACCOUNT);
        }
    }
    LOGI("The source and sink device is not same account, not support.");
    return false;
#else
    return true;
#endif
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

int32_t SoftbusAgent::OpenSession(const DeviceInfo &info, const uint8_t &linkType)
{
    LOGI("Start to OpenSession, cid:%{public}s", Utils::GetAnonyString(info.GetCid()).c_str());
    if (!IsSameAccount(info.GetCid())) {
        return FileManagement::E_INVAL_ARG;
    }
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
        return FileManagement::E_CONTEXT;
    }
    if (FindSocketId(socketId)) {
        LOGW("Has find socketId:%{public}d", socketId);
        return FileManagement::E_OK;
    }
    int32_t ret = Bind(socketId, qos, sizeof(qos) / sizeof(qos[0]), &sessionListener);
    if (ret != FileManagement::E_OK) {
        LOGE("Bind SocketClient error");
        RADAR_REPORT(RadarReporter::DFX_SET_DFS, RadarReporter::DFX_BUILD__LINK, RadarReporter::DFX_FAILED,
            RadarReporter::BIZ_STATE, RadarReporter::DFX_END, RadarReporter::ERROR_CODE,
            RadarReporter::BIND_SOCKET_ERROR, RadarReporter::PACKAGE_NAME, RadarReporter::dSoftBus + to_string(ret));
        Shutdown(socketId);
        return FileManagement::E_CONTEXT;
    }
    auto session = make_shared<SoftbusSession>(socketId, info.GetCid());
    session->DisableSessionListener();
    session->SetFromServer(false);
    AcceptSession(session, "Client");
    LOGI("Suc OpenSession socketId:%{public}d, cid:%{public}s", socketId, Utils::GetAnonyString(info.GetCid()).c_str());
    return FileManagement::E_OK;
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
    Shutdown(sessionId);
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
