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
#include "distributedfile_service.h"
#include "i_distributedfile_service.h"
#include "iservice_registry.h"
#include "session.h"
#include "softbus_dispatcher.h"
#include "system_ability_definition.h"
#include "utils_exception.h"
#include "utils_directory.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace {
    constexpr int32_t SOFTBUS_OK = 0;
    constexpr int32_t DEVICE_ID_SIZE_MAX = 65;
    constexpr int32_t IS_CLIENT = 0;
    const std::string DEFAULT_ROOT_PATH = "/data/service/el2/100/hmdfs/non_account/data/";
}

SoftbusAgent::SoftbusAgent() {}

SoftbusAgent::~SoftbusAgent()
{
    StopInstance();
}

void SoftbusAgent::StartInstance()
{
    RegisterSessionListener();
    RegisterFileListener();
}

void SoftbusAgent::StopInstance()
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
        OpenSession(cid);
        // wait for get sessionId
        LOGD("openSession, wait");
        std::unique_lock<std::mutex> lock(getSessionCVMut_);
        getSessionCV_.wait(lock, [this, cid]() { return !cidToSessionID_[cid].empty(); });
        LOGD("openSession success, wakeup");
        if (cidToSessionID_[cid].empty()) { // wakeup check
            LOGE("there is no sessionId of cid:%{public}s", cid.c_str());
            return -1;
        }
        sessionId = cidToSessionID_[cid].front();
    }

    int ret = ::SendFile(sessionId, sFileList, dFileList, fileCnt);
    if (0 != ret) {
        LOGD("sendfile is processing, sessionId:%{public}d, ret %{public}d", sessionId, ret);
        return -1;
    }
    return 0;
}

void SoftbusAgent::OpenSession(const std::string &cid)
{
    SessionAttribute attr;
    attr.dataType = TYPE_FILE; // files use UDP, CHANNEL_TYPE_UDP

    LOGD("Start to Open Session, cid:%{public}s", cid.c_str());
    int sessionId = ::OpenSession(sessionName_.c_str(), sessionName_.c_str(), cid.c_str(), "DFS_wifiGroup", &attr);
    if (sessionId < 0) {
        LOGE("Failed to open session, cid:%{public}s, sessionId:%{public}d", cid.c_str(), sessionId);
        return;
    }
    LOGD("Open Session SUCCESS, cid:%{public}s, sessionId %{public}d", cid.c_str(), sessionId);
}

void SoftbusAgent::OnSessionOpened(const int sessionId, const int result)
{
    if (result != 0) {
        LOGE("open failed, result:%{public}d, sessionId:%{public}d", result, sessionId);
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
            // cidToSessionID_[cid].push_front(sessionId);
        } else {
            cidToSessionID_[cid].push_back(sessionId);
        }
    }

    getSessionCV_.notify_one();
    LOGD("get session SUCCESS, sessionId:%{public}d", sessionId);

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
        auto alreadyOnliceDev = DeviceManagerAgent::GetInstance()->getOnlineDevs();
        LOGI("IsDeviceOnline size, %{public}d", alreadyOnliceDev.size());
        for (std::string item : alreadyOnliceDev) {
            LOGI("IsDeviceOnline item, %{public}s", item.c_str());
            cid = item;
        }
    }

    std::string desFileName = DEFAULT_ROOT_PATH + files;
    int32_t fd = open(desFileName.c_str(), O_RDONLY);
    if (fd <= 0) {
        LOGE("NapiWriteFile open recive distributedfile %{public}d, %{public}s, %{public}d",
            fd, strerror(errno), errno);
        return;
    }
    notifyCallback_->WriteFile(fd, files);
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
        return throw std::runtime_error(ss.str());
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
        throw std::runtime_error(ss.str());
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
        throw std::runtime_error(ss.str());
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