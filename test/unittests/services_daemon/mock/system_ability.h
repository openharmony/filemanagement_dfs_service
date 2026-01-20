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

#ifndef SYSTEM_ABILITY_H
#define SYSTEM_ABILITY_H

#include "iremote_object.h"

namespace OHOS {
#define REGISTER_SYSTEM_ABILITY_BY_ID(a, b, c)
#define DECLARE_SYSTEM_ABILITY(className)

class SystemAbility {
public:
    bool AddSystemAbilityListener(int32_t systemAbilityId);

protected:
    virtual void OnStart();

    virtual void OnStop();

    virtual void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId);

    virtual void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId);

    bool Publish(sptr<IRemoteObject> systemAbility);

    explicit SystemAbility(bool runOnCreate = false);

    explicit SystemAbility(const int32_t serviceId, bool runOnCreate = false);

    virtual ~SystemAbility();
};
}  // namespace OHOS
#endif  // SYSTEM_ABILITY_H
