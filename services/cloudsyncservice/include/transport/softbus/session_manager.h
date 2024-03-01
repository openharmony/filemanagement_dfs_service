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

#ifndef OHOS_FILEMGMT_SESSION_MANAGER_H
#define OHOS_FILEMGMT_SESSION_MANAGER_H

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <vector>

#include "i_softbus_listener.h"
#include "sync_rule/i_user_status_observer.h"
#include "softbus_session.h"
namespace OHOS::FileManagement::CloudSync {
class RecieveDataHandler;
class SessionManager : public ISoftbusListener, public std::enable_shared_from_this<SessionManager>,
    public IUserStatusObserver {
public:
    SessionManager() = default;
    ~SessionManager();

    void Init();

    int32_t SendData(const std::string &peerNetworkId, const void *data, uint32_t dataLen);
    int32_t SendData(int sessionId, const void *data, uint32_t dataLen);
    int32_t SendFile(const std::string &peerNetworkId,
                     const std::vector<std::string> &sFileList,
                     const std::vector<std::string> &dFileList);

    void ReleaseSession(SoftbusSession::DataType type, const std::string &peerDeviceId);

    int OnSessionOpened(int sesssionId, int result) override;
    void OnSessionClosed(int sessionId) override;
    void OnDataReceived(const std::string &senderNetworkId,
                        int receiverSessionId,
                        const void *data,
                        unsigned int dataLen) override;
    void OnFileReceived(const std::string &senderNetworkId, const char *filePath, int result) override;

    void RegisterDataHandler(std::shared_ptr<RecieveDataHandler> handler);
    void OnUserUnlocked() override;

private:
    void CreateServer();
    void RemoveServer();

    std::shared_ptr<SoftbusSession> CreateSession(SoftbusSession::DataType type, const std::string &peerDeviceId);
    std::shared_ptr<SoftbusSession> GetSendSession(SoftbusSession::DataType type, const std::string &peerDeviceId);
    void CacheSendSession(std::shared_ptr<SoftbusSession> session);
    void RemoveSendSession(int sessionId);

    bool IsSessionOpened(int sessionId);
    bool IsDeviceIdVailid(const std::string &peerDeviceId);

    std::mutex sessionVecMutex_;
    std::vector<std::shared_ptr<SoftbusSession>> sendSessionVec_;
    std::shared_ptr<RecieveDataHandler> dataHandler_;
    bool SetFileRecvListenerFlag_{true};
};

class RecieveDataHandler {
public:
    virtual ~RecieveDataHandler() = default;
    virtual void OnMessageHandle(const std::string &senderNetworkId,
                                 int receiverSessionId,
                                 const void *data,
                                 unsigned int dataLen) = 0;
    virtual void OnFileRecvHandle(const std::string &senderNetworkId, const char *filePath, int result) = 0;
    virtual void OnSessionClosed();
};
} // namespace OHOS::FileManagement::CloudSync

#endif // OHOS_FILEMGMT_SESSION_MANAGER_H
