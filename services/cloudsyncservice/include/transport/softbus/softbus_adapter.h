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

#ifndef OHOS_FILEMGMT_SOFTBUS_ADAPTER_H
#define OHOS_FILEMGMT_SOFTBUS_ADAPTER_H

#include <memory>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "nocopyable.h"
#include "session.h"
#include "socket.h"

#include "i_softbus_listener.h"

namespace OHOS::FileManagement::CloudSync {
class SoftbusAdapter : public NoCopyable {
public:
    static SoftbusAdapter &GetInstance();

    int32_t CreateSessionServer(const char *packageName, const char *sessionName);
    int32_t RemoveSessionServer(const char *packageName, const char *sessionName);

    int OpenSession(char *sessionName,
                    char *peerDeviceId,
                    char *groupId,
                    TransDataType dataType);

    int OpenSessionByP2P(char *sessionName, char *peerDeviceId, char *groupId, bool isFileType);
    void CloseSession(int sessionId);

    static void OnBind(int socket, PeerSocketInfo info);
    static void OnShutdown(int32_t socket, ShutdownReason reason);
    static void OnFile(int32_t socket, FileEvent *event);
    static void OnBytes(int sessionId, const void *data, unsigned int dataLen);
    static int OnReceiveFileProcess(int sessionId, const char *firstFile, uint64_t bytesUpload, uint64_t bytesTotal);
    static void OnReceiveFileFinished(int sessionId, const char *files, int fileCnt);
    static const char* GetRecvPath();

    int SendBytes(int sessionId, const void *data, unsigned int dataLen);
    int SendFile(int sessionId, const std::vector<std::string> &sFileList, const std::vector<std::string> &dFileList);
    std::string GetSessionNameFromMap(int sessionId);
    std::string GetPeerNetworkId(int sessionId);
    int32_t GetSocketNameFromMap(std::string sessionAndPack);
    bool IsFromServer(int sessionId);

    void RegisterSessionListener(std::string sessionName, std::shared_ptr<ISoftbusListener> listener);
    void UnRegisterSessionListener(std::string sessionName);

    std::shared_ptr<ISoftbusListener> GetListener(std::string sessionName);
    bool IsSessionOpened(int sessionId);
    void AcceptSesion(int sessionId, const std::string &sessionName, const std::string &networkId);
    void RemoveSesion(int sessionId);

private:
    SoftbusAdapter() = default;
    std::unordered_map<std::string, std::shared_ptr<ISoftbusListener>> listeners_;
    std::shared_mutex listenerMutex_;

    std::mutex sessionMutex_;
    std::unordered_map<int32_t, bool> sessionOpenedMap_;
    std::unordered_map<int32_t, std::string> sessionNameMap_;
    std::unordered_map<int32_t, std::string> networkIdMap_;
    std::unordered_map<int32_t, std::string> sessionAndPackageMap_;
    static constexpr const int QOS_COUNT = 3;
};
} // namespace OHOS::FileManagement::CloudSync

#endif // OHOS_FILEMGMT_SOFTBUS_ADAPTER_H