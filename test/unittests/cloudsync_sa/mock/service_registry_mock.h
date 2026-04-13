/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#ifndef SERVICE_REGISTRY_MOCK_H
#define SERVICE_REGISTRY_MOCK_H

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "if_system_ability_manager_mock.h"

namespace OHOS::FileManagement::CloudSync::Test {
class IServiceRegistry {
public:
    virtual sptr<ISystemAbilityManager> GetSystemAbilityManager() = 0;
    virtual ~IServiceRegistry() {}
};

class ServiceRegistryMock final : public IServiceRegistry {
public:
    MOCK_METHOD0(GetSystemAbilityManager, sptr<ISystemAbilityManager>());
    ~ServiceRegistryMock() override {}

    static inline std::shared_ptr<ServiceRegistryMock> proxy_ = nullptr;
};
} // namespace OHOS::FileManagement::CloudSync::Test
#endif // SERVICE_REGISTRY_MOCK_H