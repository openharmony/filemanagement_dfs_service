/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "session_manager.h"

#include <chrono>
#include <memory>

#include "dfs_error.h"
#include "softbus_adapter.h"
#include "softbus_session.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
using namespace std;
const string SERVICE_NAME = "OHOS.Filemanagement.Dfs.ICloudSyncService";
/* need config softbus_trans_permission.json */
const string SESSION_NAME = "DistributedFileService_edge2edge_Session";

SessionManager::~SessionManager()
{
    RemoveServer();
}

void SessionManager::Init()
{
    CreateServer();
}

void SessionManager::CreateServer()
{
    auto ret = SoftbusAdapter::GetInstance().CreateSessionServer(SERVICE_NAME.c_str(), SESSION_NAME.c_str());
    if (ret != E_OK) {
        LOGE("create session server failed");
        return;
    }

    auto res = SoftbusAdapter::GetInstance().SetFileReceiveListener(SERVICE_NAME.c_str(), SESSION_NAME.c_str());
    if (res != E_OK) {
        SetFileRecvListenerFlag_ = false;
    } else {
        SetFileRecvListenerFlag_ = true;
    }
    SoftbusAdapter::GetInstance().RegisterSessionListener(SESSION_NAME.c_str(), shared_from_this());
}

void SessionManager::OnUserUnlocked()
{
    if (SetFileRecvListenerFlag_) {
        LOGI("file recive listener has been successfully set up");
        return;
    }
    /* setfileReceiveListener again when user unlocked to avoid register fall when user locked */
    LOGI("UserUnlocked, SetFileReceiveListener again");
    auto ret = SoftbusAdapter::GetInstance().SetFileReceiveListener(SERVICE_NAME.c_str(), SESSION_NAME.c_str());
    if (ret == E_OK) {
        LOGI("SetFileReceiveListener success");
    }
}

void SessionManager::RemoveServer()
{
    auto ret = SoftbusAdapter::GetInstance().RemoveSessionServer(SERVICE_NAME.c_str(), SESSION_NAME.c_str());
    if (ret != E_OK) {
        LOGE("create session server failed");
        return;
    }

    SoftbusAdapter::GetInstance().UnRegisterSessionListener(SESSION_NAME.c_str());
}

int32_t SessionManager::SendData(int sessionId, const void *data, uint32_t dataLen)
{
    return SoftbusAdapter::GetInstance().SendBytes(sessionId, data, dataLen);
}

int32_t SessionManager::SendData(const std::string &peerNetworkId, const void *data, uint32_t dataLen)
{
    auto sendSession = CreateSession(SoftbusSession::DataType::TYPE_BYTES, peerNetworkId);
    if (sendSession == nullptr) {
        return E_CREATE_SESSION;
    }
    return sendSession->SendData(data, dataLen);
}

int32_t SessionManager::SendFile(const std::string &peerNetworkId,
                                 const std::vector<std::string> &sFileList,
                                 const std::vector<std::string> &dFileList)
{
    auto sendSession = CreateSession(SoftbusSession::DataType::TYPE_FILE, peerNetworkId);
    if (sendSession == nullptr) {
        return E_CREATE_SESSION;
    }
    return sendSession->SendFile(sFileList, dFileList);
}

std::shared_ptr<SoftbusSession> SessionManager::CreateSession(SoftbusSession::DataType type,
                                                              const std::string &peerDeviceId)
{
    if (!IsDeviceIdVailid(peerDeviceId)) {
        LOGE("peer DeviceId is Invalid");
        return nullptr;
    }

    lock_guard<mutex> lock(sessionVecMutex_);
    for (auto session : sendSessionVec_) {
        if ((session->GetDataType() == type) && (session->GetPeerDeviceId() == peerDeviceId)) {
            return session;
        }
    }

    auto sendSession = make_shared<SoftbusSession>(peerDeviceId, SESSION_NAME.c_str(), type);
    auto ret = sendSession->Start();
    if (ret != E_OK) {
        LOGE("create session failed");
        return nullptr;
    }

    CacheSendSession(sendSession);
    return sendSession;
}

bool SessionManager::IsDeviceIdVailid(const std::string &peerDeviceId)
{
    return true;
}

void SessionManager::ReleaseSession(SoftbusSession::DataType type, const std::string &peerDeviceId)
{
    auto sendSession = GetSendSession(type, peerDeviceId);
    if (sendSession == nullptr) {
        LOGE("session not exist");
        return;
    }

    sendSession->Stop();
}

int SessionManager::OnSessionOpened(int sesssionId, int result)
{
    if (result != E_OK) {
        LOGE("OnSessionOpened failed:%{public}d", result);
        return -1;
    }

    return E_OK;
}

void SessionManager::OnSessionClosed(int sessionId)
{
    RemoveSendSession(sessionId);
    if (dataHandler_ != nullptr) {
        dataHandler_->OnSessionClosed();
    }
}

void SessionManager::OnDataReceived(const std::string &senderNetworkId,
                                    int receiverSessionId,
                                    const void *data,
                                    unsigned int dataLen)
{
    if (dataHandler_ != nullptr) {
        dataHandler_->OnMessageHandle(senderNetworkId, receiverSessionId, data, dataLen);
    }
}

void SessionManager::OnFileReceived(const std::string &senderNetworkId, const char *filePath, int result)
{
    if (dataHandler_ != nullptr) {
        dataHandler_->OnFileRecvHandle(senderNetworkId, filePath, result);
    }
}

void SessionManager::RegisterDataHandler(std::shared_ptr<RecieveDataHandler> handler)
{
    dataHandler_ = handler;
}

std::shared_ptr<SoftbusSession> SessionManager::GetSendSession(SoftbusSession::DataType type,
                                                               const std::string &peerDeviceId)
{
    lock_guard<mutex> lock(sessionVecMutex_);
    for (auto session : sendSessionVec_) {
        if ((session->GetDataType() == type) && (session->GetPeerDeviceId() == peerDeviceId)) {
            return session;
        }
    }
    return nullptr;
}

void SessionManager::CacheSendSession(std::shared_ptr<SoftbusSession> session)
{
    sendSessionVec_.push_back(session);
}

void SessionManager::RemoveSendSession(int sessionId)
{
    lock_guard<mutex> lock(sessionVecMutex_);
    for (auto iter = sendSessionVec_.begin(); iter != sendSessionVec_.end();) {
        if ((*iter)->GetSessionId() == sessionId) {
            iter = sendSessionVec_.erase(iter);
        } else {
            ++iter;
        }
    }
}
} // namespace OHOS::FileManagement::CloudSync