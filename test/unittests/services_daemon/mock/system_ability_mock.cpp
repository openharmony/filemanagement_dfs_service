/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "system_ability.h"

namespace OHOS {
bool SystemAbility::AddSystemAbilityListener(int32_t systemAbilityId)
{
    return true;
}

void SystemAbility::OnStart()
{
}

void SystemAbility::OnStop()
{
}

void SystemAbility::OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
}

void SystemAbility::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
}

bool SystemAbility::Publish(sptr<IRemoteObject> systemAbility)
{
    return true;
}

SystemAbility::SystemAbility(bool runOnCreate)
{
}

SystemAbility::SystemAbility(const int32_t serviceId, bool runOnCreate)
{
}

SystemAbility::~SystemAbility()
{
}
}  // namespace OHOS