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

#ifndef SYSTEM_PARAMETERS_H
#define SYSTEM_PARAMETERS_H

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <limits>
#include <string>

namespace OHOS {
namespace system {
class ParameterMock {
public:
    MOCK_METHOD2(GetBoolParameter, bool(const std::string &, bool));
    MOCK_METHOD2(GetParameter, std::string(const std::string &, const std::string &));
    MOCK_METHOD2(SetParameter, bool(const std::string &, const std::string &));

    static inline std::shared_ptr<ParameterMock> proxy_ = nullptr;
};

/*
 * Returns true if the system parameter `key` has the value "1", "y", "yes", "on", or "true",
 * false for "0", "n", "no", "off", or "false", or `def` otherwise.
 */
bool GetBoolParameter(const std::string &key, bool def);
std::string GetParameter(const std::string &key, const std::string &def);
bool SetParameter(const std::string& key, const std::string& value);
} // namespace system
} // namespace OHOS

#endif // SYSTEM_PARAMETERS_H