/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef FILEMANAGEMENT_DFS_SERVICE_CHANNEL_MANAGER_H
#define FILEMANAGEMENT_DFS_SERVICE_CHANNEL_MANAGER_H

#include "control_cmd_parser.h"
#include "dfs_error.h"
#include "event_handler.h"
#include "single_instance.h"
#include "socket.h"

#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <string>
#include <thread>
#include <vector>

namespace OHOS {
namespace Storage {
namespace DistributedFile {

class ChannelManager final {
    DECLARE_SINGLE_INSTANCE_BASE(ChannelManager);

public:
    int32_t Init();
    void DeInit();

    int32_t GetVersion();

    int32_t CreateClientChannel(const std::string &networkId);
    int32_t DestroyClientChannel(const std::string &networkId);
    bool HasExistChannel(const std::string &networkId);

    int32_t SendRequest(
        const std::string &networkId, ControlCmd &request, ControlCmd &response, bool needResponse = false);
    int32_t SendBytes(const std::string &networkId, const std::string &data);

    void OnSocketError(int32_t socketId, const int32_t errorCode);
    void OnSocketConnected(int32_t socketId, const PeerSocketInfo &info);
    void OnSocketClosed(int32_t socketId, const ShutdownReason &reason);
    void OnBytesReceived(int32_t socketId, const void *data, const uint32_t dataLen);
    bool OnNegotiate2(int32_t socket, PeerSocketInfo info, SocketAccessInfo *peerInfo, SocketAccessInfo *localInfo);

private:
    explicit ChannelManager() = default;
    ~ChannelManager();

    void StartEvent();
    void StartCallbackEvent();
    int32_t PostTask(const AppExecFwk::InnerEvent::Callback &callback, const AppExecFwk::EventQueue::Priority priority);
    int32_t PostCallbackTask(const AppExecFwk::InnerEvent::Callback &callback,
                             const AppExecFwk::EventQueue::Priority priority);

    int32_t CreateServerSocket();
    int32_t CreateClientSocket(const std::string &peerNetworkId);

    int32_t DoSendBytes(const std::int32_t socketId, const std::string &data);

    void HandleRemoteBytes(const std::string &jsonStr, int32_t socketId);
    void DoSendBytesAsync(const ControlCmd &request, const std::string &networkId);

private:
    std::mutex initMutex_;
    int32_t serverSocketId_ = -1;
    std::string ownerName_ = "dfs_channel_manager";

    // networkId -> socketId (client)
    std::shared_mutex clientMutex_;
    std::map<std::string, int32_t> clientNetworkSocketMap_;

    // networkId -> socketId (server)
    std::shared_mutex serverMutex_;
    std::map<std::string, int32_t> serverNetworkSocketMap_;

    // for send data
    std::mutex eventMutex_;
    std::thread eventThread_;
    std::shared_ptr<OHOS::AppExecFwk::EventHandler> eventHandler_;
    std::condition_variable eventCon_;

    // for receive data and softbus callback
    std::mutex callbackEventMutex_;
    std::thread callbackEventThread_;
    std::shared_ptr<OHOS::AppExecFwk::EventHandler> callbackEventHandler_;
    std::condition_variable callbackEventCon_;

    struct ResponseWaiter {
        std::mutex mutex;
        std::condition_variable cv;
        ControlCmd response;
        bool received = false;
    };

    // for control cmd, msgId -> ResponseWaiter
    std::mutex mtx_;
    std::condition_variable cmdEventCon_;
    std::unordered_map<int32_t, std::shared_ptr<ResponseWaiter>> pendingResponses_;
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // FILEMANAGEMENT_DFS_SERVICE_CHANNEL_MANAGER_H