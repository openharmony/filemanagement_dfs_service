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

#include "network/softbus/softbus_agent.h"

#include <sstream>

#include "dfsu_exception.h"
#include "ipc/i_daemon.h"
#include "ipc_skeleton.h"
#include "network/softbus/softbus_session.h"
#include "network/softbus/softbus_session_dispatcher.h"
#include "network/softbus/softbus_session_name.h"
#include "session.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace {
    constexpr int MAX_RETRY_COUNT = 7;
    constexpr uint8_t LINK_TYPE_AP_MAX = 2;
    constexpr uint8_t LINK_TYPE_P2P_MAX = 4;
    constexpr uint8_t LINK_TYPE_WIFI_IDX = 2;
}
using namespace std;
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

void SoftbusAgent::JoinDomain()
{
    ISessionListener sessionListener = {
        .OnSessionOpened = SoftbusSessionDispatcher::OnSessionOpened,
        .OnSessionClosed = SoftbusSessionDispatcher::OnSessionClosed,
        .OnBytesReceived = nullptr,
        .OnMessageReceived = nullptr,
        .OnStreamReceived = nullptr,
    };

    SoftbusSessionDispatcher::RegisterSessionListener(sessionName_, shared_from_this());
    int ret = ::CreateSessionServer(IDaemon::SERVICE_NAME.c_str(), sessionName_.c_str(), &sessionListener);
    if (ret != 0) {
        stringstream ss;
        ss << "Failed to CreateSessionServer, errno:" << ret;
        LOGE("%{public}s, sessionName:%{public}s", ss.str().c_str(), sessionName_.c_str());
        throw runtime_error(ss.str());
    }
    LOGD("Succeed to JoinDomain, busName:%{public}s", sessionName_.c_str());
}

void SoftbusAgent::QuitDomain()
{
    int ret = ::RemoveSessionServer(IDaemon::SERVICE_NAME.c_str(), sessionName_.c_str());
    if (ret != 0) {
        stringstream ss;
        ss << "Failed to RemoveSessionServer, errno:" << ret;
        LOGE("%{public}s", ss.str().c_str());
        throw runtime_error(ss.str());
    }

    SoftbusSessionDispatcher::UnregisterSessionListener(sessionName_.c_str());
    LOGD("Succeed to QuitDomain, busName:%{public}s", sessionName_.c_str());
}

void SoftbusAgent::StopTopHalf()
{
    QuitDomain();
}

void SoftbusAgent::StopBottomHalf() {}

void SoftbusAgent::OpenSession(const DeviceInfo &info, const uint8_t &linkType)
{
    LOGI("Start to OpenSession, cid:%{public}s, linkType:%{public}d", info.GetCid().c_str(), linkType);
    SessionAttribute attr;
    attr.dataType = TYPE_BYTES;
    string groupId;
    if (linkType == LINK_TYPE_AP) {
        groupId = GROUP_TYPE_AP;
        attr.linkTypeNum = LINK_TYPE_AP_MAX;
        attr.linkType[0] = LINK_TYPE_WIFI_WLAN_5G;
        attr.linkType[1] = LINK_TYPE_WIFI_WLAN_2G;
    } else if (linkType == LINK_TYPE_P2P) {
        groupId = GROUP_TYPE_P2P;
        attr.linkTypeNum = LINK_TYPE_P2P_MAX;
        attr.linkType[0] = LINK_TYPE_WIFI_P2P;
        attr.linkType[1] = LINK_TYPE_WIFI_WLAN_5G;
        attr.linkType[LINK_TYPE_WIFI_IDX] = LINK_TYPE_WIFI_WLAN_2G;
    } else {
        LOGE("Fail to OpenSession, cid:%{public}s, linkType:%{public}d", info.GetCid().c_str(), linkType);
        ThrowException(ERR_CONNECT_LINK_TYPE, "Fail to OpenSession");
    }
    int sessionId =
        ::OpenSession(sessionName_.c_str(), sessionName_.c_str(), info.GetCid().c_str(), groupId.c_str(), &attr);
    if (sessionId < 0) {
        LOGE("Fail to OpenSession, cid:%{public}s, sessionId:%{public}d, linkType:%{public}d", info.GetCid().c_str(),
             sessionId, linkType);
        ThrowException(ERR_SOFTBUS_AGENT_ON_SESSION_OPENED_FAIL, "Fail to OpenSession");
    }
    LOGI("Success to OpenSession, cid:%{public}s, sessionId:%{public}d, linkType:%{public}d", info.GetCid().c_str(),
         sessionId, linkType);
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

int SoftbusAgent::OnSessionOpened(const int sessionId, const int result)
{
    auto session = make_shared<SoftbusSession>(sessionId);
    auto cid = session->GetCid();

    DeviceInfo info;
    info.SetCid(cid);
    if (result != 0) {
        LOGE("OnSessionOpened failed, Is %{public}s Side, result:%{public}d",
             (session->IsFromServer() == true) ? "Server" : "Client", result);
        if (!session->IsFromServer()) { // client retry
            if (IsContinueRetry(cid)) {
                auto cmd = make_unique<DfsuCmd<NetworkAgentTemplate, const DeviceInfo>>(
                    &NetworkAgentTemplate::ConnectDeviceAsync, info);
                cmd->UpdateOption({.tryTimes_ = 1});
                Recv(move(cmd));
            } else {
                LOGE("Exceeded the maximum number of retries, not retry");
            }
        }
        return result;
    }

    auto retriedTimesMap = OpenSessionRetriedTimesMap_.find(cid);
    if (retriedTimesMap != OpenSessionRetriedTimesMap_.end()) {
        OpenSessionRetriedTimesMap_.erase(cid);
    }

    int socket_fd = session->GetHandle();
    LOGI(
        "accept sesion, sessionid:%{public}d, Is %{public}s Side, fd %{public}d, from cid %{public}s, result "
        "%{public}d",
        sessionId, (session->IsFromServer() == true) ? "Server" : "Client", socket_fd, cid.c_str(), result);
    session->DisableSessionListener();
    AcceptSession(session);
    return 0;
}

void SoftbusAgent::OnSessionClosed(int sessionId)
{
    auto session = make_shared<SoftbusSession>(sessionId);
    auto cid = session->GetCid();
    LOGI("Session to %{public}s closed by unknown reason, Is %{public}s Side", cid.c_str(),
         (session->IsFromServer() == true) ? "Server" : "Client");
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
