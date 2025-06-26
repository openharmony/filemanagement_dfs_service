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

#include "channel_manager.h"
#include "network/softbus/softbus_permission_check.h"
#include "nlohmann/json.hpp"
#include "securec.h"
#include "softbus_error_code.h"
#include "utils_log.h"

#include <algorithm>
#include <chrono>
#include <future>
#include <sys/prctl.h>
#include <unordered_set>

namespace OHOS {
namespace Storage {
namespace DistributedFile {
IMPLEMENT_SINGLE_INSTANCE(ChannelManager);
using namespace std;
using namespace FileManagement;
using FileManagement::ERR_OK;

namespace {
static inline const std::string SERVICE_NAME = "ohos.storage.distributedfile.daemon";
static inline const std::string SESSION_NAME = "DistributedFileService_ChannelManager";

static constexpr int32_t DFS_LOW_QOS_TYPE_MIN_BW = 4 * 1024 * 1024;
static constexpr int32_t DFS_LOW_QOS_TYPE_MAX_LATENCY = 10000;
static constexpr int32_t DFS_LOW_QOS_TYPE_MIN_LATENCY = 2000;
static constexpr int32_t DFS_LOW_QOS_TYPE_MAX_IDLE_TIMEOUT = 60 * 60 * 1000;

static QosTV g_low_qosInfo[] = {{.qos = QOS_TYPE_MIN_BW, .value = DFS_LOW_QOS_TYPE_MIN_BW},
                                {.qos = QOS_TYPE_MAX_LATENCY, .value = DFS_LOW_QOS_TYPE_MAX_LATENCY},
                                {.qos = QOS_TYPE_MIN_LATENCY, .value = DFS_LOW_QOS_TYPE_MIN_LATENCY},
                                {.qos = QOS_TYPE_MAX_IDLE_TIMEOUT, .value = DFS_LOW_QOS_TYPE_MAX_IDLE_TIMEOUT}};

static uint32_t g_lowQosTvParamIndex = static_cast<uint32_t>(sizeof(g_low_qosInfo) / sizeof(QosTV));

} // namespace

static void OnSocketConnected(int32_t socket, PeerSocketInfo info)
{
    ChannelManager::GetInstance().OnSocketConnected(socket, info);
}

static void OnSocketClosed(int32_t socket, ShutdownReason reason)
{
    ChannelManager::GetInstance().OnSocketClosed(socket, reason);
}

static void OnBytesRecv(int32_t socket, const void *data, uint32_t dataLen)
{
    ChannelManager::GetInstance().OnBytesReceived(socket, data, dataLen);
}

static void OnError(int32_t socket, int32_t errCode)
{
    ChannelManager::GetInstance().OnSocketError(socket, errCode);
}

static bool OnNegotiate2(int32_t socket, PeerSocketInfo info, SocketAccessInfo *peerInfo, SocketAccessInfo *localInfo)
{
    return ChannelManager::GetInstance().OnNegotiate2(socket, info, peerInfo, localInfo);
}

ISocketListener channelManagerListener = {
    .OnBind = OnSocketConnected,
    .OnShutdown = OnSocketClosed,
    .OnBytes = OnBytesRecv,
    .OnMessage = nullptr,
    .OnStream = nullptr,
    .OnFile = nullptr,
    .OnQos = nullptr,
    .OnError = OnError,
    .OnNegotiate2 = OnNegotiate2,
};

ChannelManager::~ChannelManager()
{
    DeInit();
};

int32_t ChannelManager::GetVersion()
{
    return version_;
}

int32_t ChannelManager::Init()
{
    LOGI("start init channel manager");
    std::lock_guard<std::mutex> lockInit(initMutex_);

    if (eventHandler_ != nullptr && callbackEventHandler_ != nullptr && serverSocketId_ > 0) {
        LOGW("server channel already init");
        return ERR_OK;
    }

    int32_t socketServerId = CreateServerSocket();
    if (socketServerId <= 0) {
        LOGE("create socket failed, ret: %{public}d", socketServerId);
        return ERR_CREATE_SOCKET_FAILED;
    }

    int32_t ret = Listen(socketServerId, g_low_qosInfo, g_lowQosTvParamIndex, &channelManagerListener);
    if (ret != ERR_OK) {
        LOGE("service listen failed, ret: %{public}d", ret);
        return ERR_LISTEN_SOCKET_FAILED;
    }

    eventThread_ = std::thread(&ChannelManager::StartEvent, this);
    std::unique_lock<std::mutex> lock(eventMutex_);
    eventCon_.wait(lock, [this] { return eventHandler_ != nullptr; });

    callbackEventThread_ = std::thread(&ChannelManager::StartCallbackEvent, this);
    std::unique_lock<std::mutex> callbackLock(callbackEventMutex_);
    callbackEventCon_.wait(callbackLock, [this] { return callbackEventHandler_ != nullptr; });
    serverSocketId_ = socketServerId;

    LOGI("end init channel manager");
    return ERR_OK;
}

void ChannelManager::DeInit()
{
    LOGI("start deinit channel manager");
    // stop send task
    if (eventHandler_ != nullptr) {
        eventHandler_->GetEventRunner()->Stop();
        if (eventThread_.joinable()) {
            eventThread_.join();
        }
        eventHandler_ = nullptr;
    }

    // stop callback task
    if (callbackEventHandler_ != nullptr) {
        callbackEventHandler_->GetEventRunner()->Stop();
        if (callbackEventThread_.joinable()) {
            callbackEventThread_.join();
        }
        callbackEventHandler_ = nullptr;
    }

    // release socket
    {
        std::unique_lock<std::shared_mutex> writeLock(clientMutex_);
        for (auto it = clientNetworkSocketMap_.begin(); it != clientNetworkSocketMap_.end(); ++it) {
            LOGI("ShutDown socketId %{public}d from clientNetworkSocketMap_", it->second);
            Shutdown(it->second);
        }
        clientNetworkSocketMap_.clear();
    }

    {
        std::unique_lock<std::shared_mutex> writeLock(serverMutex_);
        for (auto it = serverNetworkSocketMap_.begin(); it != serverNetworkSocketMap_.end(); ++it) {
            LOGI("ShutDown socketId %{public}d from serverNetworkSocketMap_", it->second);
            Shutdown(it->second);
        }
        serverNetworkSocketMap_.clear();
    }

    Shutdown(serverSocketId_);
    serverSocketId_ = -1;
    LOGI("end deinit");
}

void ChannelManager::StartEvent()
{
    LOGI("StartEvent start");
    prctl(PR_SET_NAME, ownerName_.c_str());
    auto runner = AppExecFwk::EventRunner::Create(false);
    {
        std::lock_guard<std::mutex> lock(eventMutex_);
        eventHandler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
    }
    eventCon_.notify_one();
    runner->Run();
    LOGI("StartEvent end");
}

void ChannelManager::StartCallbackEvent()
{
    LOGI("Start callback event start");
    std::string callbackName = ownerName_ + "callback";
    prctl(PR_SET_NAME, callbackName.c_str());
    auto runner = AppExecFwk::EventRunner::Create(false);
    {
        std::lock_guard<std::mutex> lock(callbackEventMutex_);
        callbackEventHandler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
    }
    callbackEventCon_.notify_one();
    runner->Run();
    LOGI("callback event end");
}

int32_t ChannelManager::CreateClientChannel(const std::string &networkId)
{
    LOGI("CreateClientChannel start, networkId: %{public}.6s", networkId.c_str());
    if (!SoftBusPermissionCheck::CheckSrcPermission(networkId)) {
        LOGE("CheckSrcPermission failed");
        return ERR_CHECK_PERMISSION_FAILED;
    }

    if (!SoftBusPermissionCheck::IsSameAccount(networkId)) {
        LOGE("The calling device is not trusted");
        return ERR_CHECK_PERMISSION_FAILED;
    }

    {
        std::shared_lock<std::shared_mutex> readLock(clientMutex_);
        auto channelIt = clientNetworkSocketMap_.find(networkId);
        if (channelIt != clientNetworkSocketMap_.end()) {
            LOGW("has connect to this network");
            return ERR_OK;
        }
    }

    int32_t socketId = CreateClientSocket(networkId);
    if (socketId <= 0) {
        LOGE("create client socket failed");
        return ERR_BAD_VALUE;
    }
    if (!SoftBusPermissionCheck::SetAccessInfoToSocket(socketId)) {
        LOGE("Fill and set accessInfo failed");
        return ERR_BAD_VALUE;
    }

    LOGI("start to bind socket, id:%{public}d", socketId);
    int32_t ret = Bind(socketId, g_low_qosInfo, g_lowQosTvParamIndex, &channelManagerListener);
    if (ret != ERR_OK) {
        LOGE("client bind failed, ret: %{public}d", ret);
        return ERR_BIND_SOCKET_FAILED;
    }

    std::lock_guard<std::shared_mutex> writeLock(clientMutex_);
    clientNetworkSocketMap_[networkId] = socketId;

    LOGI("CreateClientChannel end");
    return ERR_OK;
}

int32_t ChannelManager::DestroyClientChannel(const std::string &networkId)
{
    LOGI("DestroyClientChannel start, networkId: %{public}.6s", networkId.c_str());
    int32_t socketId = -1;
    {
        std::unique_lock<std::shared_mutex> writeLock(clientMutex_);
        auto channelIt = clientNetworkSocketMap_.find(networkId);
        if (channelIt == clientNetworkSocketMap_.end()) {
            LOGE("has not connect to this network");
            return ERR_BAD_VALUE;
        } else {
            socketId = channelIt->second;
            clientNetworkSocketMap_.erase(channelIt);
        }
    }

    if (socketId <= 0) {
        LOGE("socket id invalid");
        return ERR_BAD_VALUE;
    }

    Shutdown(socketId);
    return ERR_OK;
}

bool ChannelManager::HasExistChannel(const std::string &networkId)
{
    std::shared_lock<std::shared_mutex> readLock(clientMutex_);
    return clientNetworkSocketMap_.count(networkId) > 0;
}

int32_t ChannelManager::CreateServerSocket()
{
    LOGI("start create server socket");
    SocketInfo info = {
        .name = const_cast<char *>(SESSION_NAME.c_str()),
        .pkgName = const_cast<char *>(SERVICE_NAME.c_str()),
    };

    int32_t socketId = Socket(info);
    LOGI("finish, socket id: %{public}d", socketId);
    return socketId;
}

int32_t ChannelManager::CreateClientSocket(const std::string &peerNetworkId)
{
    LOGI("CreateClientSocket start");
    SocketInfo socketInfo = {.name = const_cast<char *>(SESSION_NAME.c_str()),
                             .peerName = const_cast<char *>(SESSION_NAME.c_str()),
                             .peerNetworkId = const_cast<char *>(peerNetworkId.c_str()),
                             .pkgName = const_cast<char *>(SERVICE_NAME.c_str()),
                             .dataType = DATA_TYPE_BYTES};

    int32_t socketId = Socket(socketInfo);
    LOGI("finish, socket session id: %{public}d", socketId);
    return socketId;
}

void ChannelManager::OnSocketConnected(int32_t socketId, const PeerSocketInfo &info)
{
    LOGI("socket %{public}d binded", socketId);
    if (socketId <= 0) {
        LOGE("invalid socket id, %{public}d", socketId);
        return;
    }
    string clientNetworkId = info.networkId;

    std::lock_guard<std::shared_mutex> writeLock(serverMutex_);
    serverNetworkSocketMap_[clientNetworkId] = socketId;

    LOGI("add update channel task into handler");
}

void ChannelManager::OnSocketError(int32_t socketId, const int32_t errorCode)
{
    LOGE("socket error. socketId: %{public}d, errorCode: %{public}d", socketId, errorCode);
}

void ChannelManager::OnSocketClosed(int32_t socketId, const ShutdownReason &reason)
{
    LOGI("socket closed. socketId: %{public}d, reason: %{public}d", socketId, reason);
    if (socketId <= 0) {
        LOGE("invalid socket id, %{public}d", socketId);
        return;
    }
    {
        std::unique_lock<std::shared_mutex> writeLock(clientMutex_);
        for (auto it = clientNetworkSocketMap_.begin(); it != clientNetworkSocketMap_.end();) {
            if (it->second == socketId) {
                it = clientNetworkSocketMap_.erase(it);
                LOGI("Removed socketId %{public}d from clientNetworkSocketMap_", socketId);
            } else {
                ++it;
            }
        }
    }

    {
        std::unique_lock<std::shared_mutex> writeLock(serverMutex_);
        for (auto it = serverNetworkSocketMap_.begin(); it != serverNetworkSocketMap_.end();) {
            if (it->second == socketId) {
                it = serverNetworkSocketMap_.erase(it);
                LOGI("Removed socketId %{public}d from serverNetworkSocketMap_", socketId);
            } else {
                ++it;
            }
        }
    }
}

bool ChannelManager::OnNegotiate2(int32_t socket,
                                  PeerSocketInfo info,
                                  SocketAccessInfo *peerInfo,
                                  SocketAccessInfo *localInfo)
{
    AccountInfo callerAccountInfo;
    std::string networkId = info.networkId;
    if (!SoftBusPermissionCheck::TransCallerInfo(peerInfo, callerAccountInfo, networkId)) {
        LOGE("peerInfo is nullptr.");
        return false;
    }
    if (!SoftBusPermissionCheck::FillLocalInfo(localInfo)) {
        LOGE("FillLocalInfo failed.");
        return false;
    }
    if (!SoftBusPermissionCheck::IsSameAccount(networkId)) {
        LOGE("The calling device is not trusted");
        return false;
    }
    return SoftBusPermissionCheck::CheckSinkPermission(callerAccountInfo);
}

void ChannelManager::OnBytesReceived(int32_t socketId, const void *data, const uint32_t dataLen)
{}

int32_t ChannelManager::SendRequest(const std::string &networkId,
                                    ControlCmd &request,
                                    ControlCmd &response,
                                    bool needResponse)
{
    LOGI("start sendRequest, networkId: %{public}.6s", networkId.c_str());

    // implement later, now for temp ut test
    return ERR_OK;
}

} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS