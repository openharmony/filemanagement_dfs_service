/*
* Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "all_connect_manager_mock.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
AllConnectManager::AllConnectManager()
{
}

AllConnectManager &AllConnectManager::GetInstance()
{
    static AllConnectManager instance;
    return instance;
}

int32_t AllConnectManager::InitAllConnectManager()
{
    return 0;
}

int32_t AllConnectManager::UnInitAllConnectManager()
{
    return 0;
}

int32_t AllConnectManager::PublishServiceState(DfsConnectCode code, const std::string &peerNetworkId,
                                               ServiceCollaborationManagerBussinessStatus state)
{
    return 0;
}

int32_t AllConnectManager::ApplyAdvancedResource(const std::string &peerNetworkId,
    ServiceCollaborationManager_ResourceRequestInfoSets *resourceRequest)
{
    if (IAllConnectManagerMock::iAllConnectManagerMock_ == nullptr) {
        return -1;
    }
    return IAllConnectManagerMock::iAllConnectManagerMock_->ApplyAdvancedResource(peerNetworkId, resourceRequest);
}

bool AllConnectManager::GetPublicState(DfsConnectCode code, const std::string &peerNetworkId,
    ServiceCollaborationManagerBussinessStatus state)
{
    return true;
}

int32_t AllConnectManager::GetAllConnectSoLoad()
{
    return 0;
}

int32_t AllConnectManager::RegisterLifecycleCallback()
{
    return 0;
}

int32_t AllConnectManager::UnRegisterLifecycleCallback()
{
    return 0;
}

int32_t AllConnectManager::ApplyResult(int32_t errorcode, int32_t result, const char *reason)
{
    return 0;
}

int32_t AllConnectManager::OnStop(const char *peerNetworkId)
{
    return 0;
}

std::shared_ptr<ServiceCollaborationManager_ResourceRequestInfoSets> AllConnectManager::BuildResourceRequest()
{
    auto resourceRequest = std::make_shared<ServiceCollaborationManager_ResourceRequestInfoSets>();
    return resourceRequest;
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS