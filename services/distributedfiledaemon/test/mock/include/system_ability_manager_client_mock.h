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
#ifndef FILEMANAGEMENT_DFS_SERVICE_SYSTEM_ABILITY_MANAGER_CLIENT_MOCK_H
#define FILEMANAGEMENT_DFS_SERVICE_SYSTEM_ABILITY_MANAGER_CLIENT_MOCK_H

#include <gmock/gmock.h>
#include "iservice_registry.h"
#include "mock/if_system_ability_manager_mock.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
class DfsSystemAbilityManagerClient {
public:
    virtual SystemAbilityManagerClient &GetInstance() = 0;
    virtual sptr<ISystemAbilityManager> GetSystemAbilityManager() = 0;
public:
    DfsSystemAbilityManagerClient() = default;
    virtual ~DfsSystemAbilityManagerClient() = default;
    static inline std::shared_ptr<DfsSystemAbilityManagerClient> smc = nullptr;
};

class DfsSystemAbilityManagerClientMock : public DfsSystemAbilityManagerClient {
public:
    MOCK_METHOD0(GetInstance, SystemAbilityManagerClient &());
    MOCK_METHOD0(GetSystemAbilityManager, sptr<ISystemAbilityManager>());
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // FILEMANAGEMENT_DFS_SERVICE_SYSTEM_ABILITY_MANAGER_CLIENT_MOCK_H
