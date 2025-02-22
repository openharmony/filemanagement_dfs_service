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

#ifndef FILEMANAGEMENT_DFS_SERVICE_ALL_CONNECT_MANAGER_H
#define FILEMANAGEMENT_DFS_SERVICE_ALL_CONNECT_MANAGER_H

#include <map>
#include <memory>
#include <mutex>
#include <string>

#include "block_object.h"
#include "service_collaboration_manager_capi.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
enum class DfsConnectCode {
    OPEN_P2P = 1,
    PUSH_ASSET,
    COPY_FILE,
};

class AllConnectManager {
public:
    static AllConnectManager &GetInstance();
    int32_t InitAllConnectManager();
    int32_t UnInitAllConnectManager();
    int32_t PublishServiceState(DfsConnectCode code, const std::string &peerNetworkId,
                                ServiceCollaborationManagerBussinessStatus state);
    int32_t ApplyAdvancedResource(const std::string &peerNetworkId,
                                  ServiceCollaborationManager_ResourceRequestInfoSets *resourceRequest);
    std::shared_ptr<ServiceCollaborationManager_ResourceRequestInfoSets> BuildResourceRequest();

private:
    AllConnectManager();
    ~AllConnectManager() = default;
    int32_t GetAllConnectSoLoad();
    int32_t RegisterLifecycleCallback();
    int32_t UnRegisterLifecycleCallback();
    bool GetPublicState(DfsConnectCode code, const std::string &peerNetworkId,
        ServiceCollaborationManagerBussinessStatus state);

    static int32_t OnStop(const char *peerNetworkId);
    static int32_t ApplyResult(int32_t errorcode, int32_t result, const char *reason);

    std::mutex allConnectLock_;
    void *dllHandle_ = nullptr;

    ServiceCollaborationManager_API allConnect_ = {
        .ServiceCollaborationManager_PublishServiceState = nullptr,
        .ServiceCollaborationManager_ApplyAdvancedResource = nullptr,
        .ServiceCollaborationManager_RegisterLifecycleCallback = nullptr,
        .ServiceCollaborationManager_UnRegisterLifecycleCallback = nullptr,
    };
    ServiceCollaborationManager_Callback allConnectCallback_;
    std::shared_ptr<ServiceCollaborationManager_HardwareRequestInfo> remoteHardwareList_;
    std::shared_ptr<ServiceCollaborationManager_HardwareRequestInfo> localHardwareList_;
    std::shared_ptr<ServiceCollaborationManager_CommunicationRequestInfo> communicationRequest_;

    std::mutex connectStatesMutex_;
    std::map<std::string, std::map<DfsConnectCode, ServiceCollaborationManagerBussinessStatus>> connectStates_;

    static std::shared_ptr<BlockObject<bool>> applyResultBlock_;
    static constexpr uint32_t BLOCK_INTERVAL_ALLCONNECT = 60 * 1000;
    static inline const std::string SERVICE_NAME {"Distributedfile"};
};

} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // FILEMANAGEMENT_DFS_SERVICE_ALL_CONNECT_MANAGER_H