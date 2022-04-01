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

#include "softbus_agent.h"

#include <fcntl.h>
#include <mutex>
#include <unistd.h>

#include "device_manager_agent.h"
#include "dfsu_exception.h"
#include "dfsu_fd_guard.h"
#include "distributedfile_service.h"
#include "i_distributedfile_service.h"
#include "iservice_registry.h"
#include "session.h"
#include "softbus_dispatcher.h"
#include "system_ability_definition.h"
#include "utils_directory.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace {
    constexpr int32_t SOFTBUS_OK = 0;
    constexpr int32_t DEVICE_ID_SIZE_MAX = 65;
    constexpr int MAX_RETRY_COUNT = 7;
    constexpr int32_t IS_CLIENT = 1;
    const std::string DEFAULT_ROOT_PATH = "/data/service/el2/100/hmdfs/non_account/data/";
}

SoftbusAgent::SoftbusAgent() : DfsuActor<SoftbusAgent>(this, std::numeric_limits<uint32_t>::max()) {}

SoftbusAgent::~SoftbusAgent()
{
    StopInstance();
}

void SoftbusAgent::StartInstance()
{
    StartActor();
}

void SoftbusAgent::StopInstance()
{
    StopActor();
}

void SoftbusAgent::Start()
{
    RegisterSessionListener();
    RegisterFileListener();
}

void SoftbusAgent::Stop()
{
    {
        std::unique_lock<std::mutex> lock(sessionMapMux_);
        for (auto iter = cidToSessionID_.begin(); iter != cidToSessionID_.end();) {
            CloseSession(iter->first);
            iter = cidToSessionID_.erase(iter);
        }
    }
    getSessionCV_.notify_all();

    UnRegisterSessionListener();
}

void SoftbusAgent::OnDeviceOnline(const std::string &cid)
{
}

int SoftbusAgent::SendFile(const std::string &cid, const char *sFileList[], const char *dFileList[], uint32_t fileCnt)
{
    // first check whether the sessionId available
    auto alreadyOnliceDev = DeviceManagerAgent::GetInstance()->getOnlineDevs();
    if (alreadyOnliceDev.find(cid) == alreadyOnliceDev.end()) {
        LOGE("cid:%{public}s has not been online yet, sendfile maybe will fail, try", cid.c_str());
    }
    int sessionId = -1;
    {
        std::unique_lock<std::mutex> lock(sessionMapMux_);
        if (cidToSessionID_.find(cid) != cidToSessionID_.end() &&
            cidToSessionID_[cid].empty() == false) { // to avoid list is empty
            sessionId = cidToSessionID_[cid].front();
        }
    }

    // build socket synchronously
    if (sessionId == -1) {
        auto cmd = std::make_unique<DfsuCmd<SoftbusAgent, const std::string>>(&SoftbusAgent::OpenSession, cid);
        cmd->UpdateOption({
            .tryTimes_ = MAX_RETRY_COUNT,
        });
        Recv(move(cmd));
    }

    std::unique_lock<std::mutex> lock(getSessionCVMut_);
    std::unordered_map<std::string, std::list<int>> &cidToSessionID { cidToSessionID_ };
    getSessionCV_.wait(lock, [&cidToSessionID, cid]() { return !cidToSessionID[cid].empty(); });

    if (cidToSessionID_[cid].empty()) {
        LOGE("there is no sessionId of cid:%{public}s", cid.c_str());
        return -1;
    }
    sessionId = cidToSessionID_[cid].front();

    int result = ::SendFile(sessionId, sFileList, dFileList, fileCnt);
    if (result != 0) {
        auto cmd = std::make_unique<DfsuCmd<SoftbusAgent, int, const char **, const char **, uint32_t>>(
            &SoftbusAgent::CallFileSend, sessionId, sFileList, dFileList, fileCnt);
        cmd->UpdateOption({
            .tryTimes_ = MAX_RETRY_COUNT,
        });
        Recv(move(cmd));
    }

    return result;
}

void SoftbusAgent::CallFileSend(const int sessionId, const char *sFileList[], const char *dFileList[], uint32_t fileCnt)
{
    int result = ::SendFile(sessionId, sFileList, dFileList, fileCnt);
    if (result != 0) {
        ThrowException(ERR_SOFTBUS_AGENT_ON_SESSION_OPENED_FAIL, "softbus sendfile failed, retry...");
    }
}

void SoftbusAgent::OpenSession(const std::string cid)
{
    LOGD("SoftbusAgent::OpenSession, deviceId %{public}s", cid.c_str());
    SessionAttribute attr;
    attr.dataType = TYPE_FILE; // files use UDP, CHANNEL_TYPE_UDP
    int sessionId = ::OpenSession(sessionName_.c_str(), sessionName_.c_str(), cid.c_str(), "DFS_wifiGroup", &attr);
    if (sessionId < 0) {
        LOGE("Failed to open session, cid:%{public}s", cid.c_str());
        ThrowException(ERR_SOFTBUS_AGENT_ON_SESSION_OPENED_FAIL, "Open Session failed");
    }
}

void SoftbusAgent::OnSessionOpened(const int sessionId, const int result)
{
    if (result != 0) {
        LOGE("open failed, result:%{public}d", result);
        return;
    }
    std::string cid = GetPeerDevId(sessionId);
    if (cid == "") {
        LOGE("get peer device id failed");
        return;
    }

    // client session priority use, so insert list head
    {
        std::unique_lock<std::mutex> lock(sessionMapMux_);
        if (::GetSessionSide(sessionId) == IS_CLIENT) {
            cidToSessionID_[cid].push_back(sessionId);
        }
    }

    getSessionCV_.notify_one();

    if (notifyCallback_ == nullptr) {
        LOGE("OnSessionOpened Nofify pointer is empty");
        return;
    }
    notifyCallback_->SessionOpened(cid);
}

void SoftbusAgent::OnSendFileFinished(const int sessionId, const std::string firstFile)
{
    if (notifyCallback_ == nullptr) {
        LOGE("OnSendFileFinished Nofify pointer is empty");
        return;
    }
    std::string cid = GetPeerDevId(sessionId);
    notifyCallback_->SendFinished(cid, firstFile);
}

void SoftbusAgent::OnFileTransError(const int sessionId)
{
    if (notifyCallback_ == nullptr) {
        LOGE("OnFileTransError Nofify pointer is empty");
        return;
    }

    std::string cid = GetPeerDevId(sessionId);
    if (::GetSessionSide(sessionId) == IS_CLIENT) {
        notifyCallback_->SendError(cid);
    } else {
        notifyCallback_->ReceiveError(cid);
    }
}

void SoftbusAgent::OnReceiveFileFinished(const int sessionId, const std::string files, int fileCnt)
{
    if (notifyCallback_ == nullptr) {
        LOGE("OnReceiveFileFinished Nofify pointer is empty");
        return;
    }
    std::string cid = GetPeerDevId(sessionId);
    if (cid.empty()) {
        LOGE("Receive file finished get peer device id failed");
        return;
    }

    std::string desFileName = DEFAULT_ROOT_PATH + files;
    DfsuFDGuard readFd(open(desFileName.c_str(), O_RDONLY));
    if (!readFd) {
        LOGE("NapiWriteFile open recive distributedfile %{public}s, %{public}d", desFileName.c_str(), errno);
        return;
    }
    notifyCallback_->WriteFile(readFd.GetFD(), files);

    notifyCallback_->ReceiveFinished(cid, files, fileCnt);
}

void SoftbusAgent::OnSessionClosed(int sessionId)
{
    std::string cid = GetPeerDevId(sessionId);
    if (cid == "") {
        LOGE("get peer device id failed");
        return;
    }
    {
        std::unique_lock<std::mutex> lock(sessionMapMux_);
        if (cidToSessionID_.find(cid) != cidToSessionID_.end()) {
            cidToSessionID_[cid].remove(sessionId);
        }
    }

    if (notifyCallback_ == nullptr) {
        LOGE("OnSessionClosed Nofify pointer is empty");
        return;
    }
    notifyCallback_->SessionClosed(cid);
}

std::string SoftbusAgent::GetPeerDevId(const int sessionId)
{
    std::string cid;
    char peerDevId[DEVICE_ID_SIZE_MAX] = { '\0' };
    int ret = ::GetPeerDeviceId(sessionId, peerDevId, sizeof(peerDevId));
    if (ret != SOFTBUS_OK) {
        LOGE("get my peer device id failed, errno:%{public}d, sessionId:%{public}d", ret, sessionId);
        cid = "";
    } else {
        cid = std::string(peerDevId);
    }

    return cid;
}

void SoftbusAgent::CloseSession(const std::string &cid)
{
    if (cidToSessionID_.find(cid) == cidToSessionID_.end()) {
        LOGE("not find this dev-cid:%{public}s", cid.c_str());
        return;
    }
    for (auto sessionId : cidToSessionID_[cid]) {
        ::CloseSession(sessionId);
    }
    LOGD("Close Session done, cid:%{public}s", cid.c_str());
}

void SoftbusAgent::OnDeviceOffline(const std::string &cid)
{
    CloseSession(cid);
    {
        std::unique_lock<std::mutex> lock(sessionMapMux_);
        cidToSessionID_.erase(cid);
    }
}

void SoftbusAgent::RegisterSessionListener()
{
    ISessionListener sessionListener = {
        .OnSessionOpened = SoftbusDispatcher::OnSessionOpened,
        .OnSessionClosed = SoftbusDispatcher::OnSessionClosed,
    };
    int ret = ::CreateSessionServer(DistributedFileService::pkgName_.c_str(), sessionName_.c_str(), &sessionListener);
    if (ret != 0) {
        std::stringstream ss;
        ss << "Failed to CreateSessionServer, errno:" << ret;
        LOGE("%{public}s, sessionName:%{public}s", ss.str().c_str(), sessionName_.c_str());
        ThrowException(ret, ss.str().c_str());
    }
    LOGD("Succeed to CreateSessionServer, pkgName %{public}s, sbusName:%{public}s",
        DistributedFileService::pkgName_.c_str(), sessionName_.c_str());
}

void SoftbusAgent::RegisterFileListener()
{
    IFileSendListener fileSendListener = {
        .OnSendFileFinished = SoftbusDispatcher::OnSendFileFinished,
        .OnFileTransError = SoftbusDispatcher::OnFileTransError,
    };
    std::string pkgName = DistributedFileService::pkgName_;
    int ret = ::SetFileSendListener(pkgName.c_str(), sessionName_.c_str(), &fileSendListener);
    if (ret != 0) {
        std::stringstream ss;
        ss << "Failed to SetFileSendListener, errno:" << ret;
        LOGE("%{public}s, sessionName:%{public}s", ss.str().c_str(), sessionName_.c_str());
        ThrowException(ret, ss.str().c_str());
    }
    LOGD("Succeed to SetFileSendListener, pkgName %{public}s, sbusName:%{public}s", pkgName.c_str(),
        sessionName_.c_str());

    IFileReceiveListener fileRecvListener = {
        .OnReceiveFileFinished = SoftbusDispatcher::OnReceiveFileFinished,
        .OnFileTransError = SoftbusDispatcher::OnFileTransError,
    };
    ret = ::SetFileReceiveListener(pkgName.c_str(), sessionName_.c_str(), &fileRecvListener,
        DEFAULT_ROOT_PATH.c_str());
    if (ret != 0) {
        std::stringstream ss;
        ss << "Failed to SetFileReceiveListener, errno:" << ret;
        LOGE("%{public}s, sessionName:%{public}s", ss.str().c_str(), sessionName_.c_str());
        throw std::runtime_error(ss.str());
    }
    LOGD("Succeed to SetFileReceiveListener, pkgName %{public}s, sbusName:%{public}s", pkgName.c_str(),
        sessionName_.c_str());
}

void SoftbusAgent::UnRegisterSessionListener()
{
    int ret = ::RemoveSessionServer(DistributedFileService::pkgName_.c_str(), sessionName_.c_str());
    if (ret != 0) {
        std::stringstream ss;
        ss << "Failed to RemoveSessionServer, errno:" << ret;
        LOGE("%{public}s", ss.str().c_str());
        ThrowException(ret, ss.str().c_str());
    }
    LOGD("RemoveSessionServer success!");
}

void SoftbusAgent::SetTransCallback(sptr<IFileTransferCallback> &callback)
{
    notifyCallback_ = callback;
}

void SoftbusAgent::RemoveTransCallbak()
{
    notifyCallback_ = nullptr;
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS