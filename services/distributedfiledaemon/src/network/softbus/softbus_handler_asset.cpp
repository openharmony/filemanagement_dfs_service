/*
* Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "network/softbus/softbus_handler_asset.h"

#include "network/softbus/softbus_session_pool.h"
#include "network/softbus/softbus_session_listener.h"
#include "network/softbus/softbus_asset_recv_listener.h"
#include "ipc_skeleton.h"
#include "dfs_error.h"
#include "utils_log.h"
#include "device_manager.h"
#include "dm_device_info.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
using namespace OHOS::FileManagement;
const int32_t DFS_QOS_TYPE_MIN_BW = 90 * 1024 * 1024;
const int32_t DFS_QOS_TYPE_MAX_LATENCY = 10000;
const int32_t DFS_QOS_TYPE_MIN_LATENCY = 2000;
SoftBusHandlerAsset::SoftBusHandlerAsset()
{
    ISocketListener fileSendListener;
    fileSendListener.OnBind = nullptr;
    fileSendListener.OnShutdown = DistributedFile::SoftBusSessionListener::OnSessionClosed;
    fileSendListener.OnFile = nullptr;
    fileSendListener.OnBytes = nullptr;
    fileSendListener.OnMessage = nullptr;
    fileSendListener.OnQos = nullptr;
    sessionListener_[DFS_ASSET_ROLE_SEND] = fileSendListener;

    ISocketListener fileReceiveListener;
    fileReceiveListener.OnBind = SoftbusAssetRecvListener::OnAssetRecvBind;
    fileReceiveListener.OnShutdown = DistributedFile::SoftBusSessionListener::OnSessionClosed;
    fileReceiveListener.OnFile = SoftbusAssetRecvListener::OnFile;
    fileReceiveListener.OnBytes = nullptr;
    fileReceiveListener.OnMessage = nullptr;
    fileReceiveListener.OnQos = nullptr;
    sessionListener_[DFS_ASSET_ROLE_RECV] = fileReceiveListener;
}

SoftBusHandlerAsset::~SoftBusHandlerAsset() = default;

SoftBusHandlerAsset &SoftBusHandlerAsset::GetInstance()
{
    LOGD("SoftBusHandlerAsset::GetInstance");
    static SoftBusHandlerAsset assetHandle;
    return assetHandle;
}

void SoftBusHandlerAsset::CreateAssetLocalSessionServer()
{
    LOGI("CreateAssetLocalSessionServer Enter.");
    {
        std::lock_guard<std::mutex> lock(serverIdMapMutex_);
        if (serverIdMap_.find(ASSET_LOCAL_SESSION_NAME) != serverIdMap_.end()) {
            LOGI("%s: Session already create.", ASSET_LOCAL_SESSION_NAME.c_str());
            return;
        }
    }

    SocketInfo serverInfo = {
        .name = const_cast<char*>(ASSET_LOCAL_SESSION_NAME.c_str()),
        .pkgName = const_cast<char*>(SERVICE_NAME.c_str()),
        .dataType = DATA_TYPE_FILE,
    };
    int32_t socketId = Socket(serverInfo);
    if (socketId < E_OK) {
        LOGE("Create Socket fail socketId, socketId = %{public}d", socketId);
        return;
    }
    QosTV qos[] = {
        {.qos = QOS_TYPE_MIN_BW,        .value = DFS_QOS_TYPE_MIN_BW},
        {.qos = QOS_TYPE_MAX_LATENCY,        .value = DFS_QOS_TYPE_MAX_LATENCY},
        {.qos = QOS_TYPE_MIN_LATENCY,        .value = DFS_QOS_TYPE_MIN_LATENCY},
    };
    int32_t ret = Listen(socketId, qos, sizeof(qos) / sizeof(qos[0]), &sessionListener_[DFS_ASSET_ROLE_RECV]);
    if (ret != E_OK) {
        LOGE("Listen SocketClient error");
        Shutdown(socketId);
        return;
    }
    SoftBusSessionPool::SessionInfo sessionInfo{.uid = IPCSkeleton::GetCallingUid()};
    SoftBusSessionPool::GetInstance().AddSessionInfo(ASSET_LOCAL_SESSION_NAME, sessionInfo);
    {
        std::lock_guard<std::mutex> lock(serverIdMapMutex_);
        serverIdMap_.insert(std::make_pair(ASSET_LOCAL_SESSION_NAME, socketId));
    }
    LOGI("CreateAssetLocalSessionServer Success.");
}
void SoftBusHandlerAsset::DeleteAssetLocalSessionServer()
{
    LOGI("DeleteAssetLocalSessionServer Enter.");
    if (!serverIdMap_.empty()) {
        std::lock_guard<std::mutex> lock(serverIdMapMutex_);
        auto it = serverIdMap_.find(ASSET_LOCAL_SESSION_NAME);
        if (it == serverIdMap_.end()) {
            LOGI("%s: Session already delete.", ASSET_LOCAL_SESSION_NAME.c_str());
            return;
        }
        int32_t socketId = it->second;
        serverIdMap_.erase(it);
        Shutdown(socketId);
        LOGI("RemoveSessionServer success.");
    }
    SoftBusSessionPool::GetInstance().DeleteSessionInfo(ASSET_LOCAL_SESSION_NAME);
}

} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS