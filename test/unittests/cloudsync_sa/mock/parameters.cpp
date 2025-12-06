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

#include "parameters.h"
#include "parameter.h"

int WaitParameter(const char *key, const char *value, int timeout)
{
    return 0;
}

namespace OHOS {
namespace system {
/*
 * Returns true if the system parameter `key` has the value "1", "y", "yes", "on", or "true",
 * false for "0", "n", "no", "off", or "false", or `def` otherwise.
 */
bool GetBoolParameter(const std::string &key, bool def)
{
    if (ParameterMock::proxy_ != nullptr) {
        return ParameterMock::proxy_->GetBoolParameter(key, def);
    }
    return true;
}

std::string GetParameter(const std::string &key, const std::string &def)
{
    if (ParameterMock::proxy_ != nullptr) {
        return ParameterMock::proxy_->GetParameter(key, def);
    }
    return "";
}

bool SetParameter(const std::string& key, const std::string& value)
{
    if (ParameterMock::proxy_ != nullptr) {
        return ParameterMock::proxy_->SetParameter(key, value);
    }
    return true;
}
} // namespace system
} // namespace OHOS
