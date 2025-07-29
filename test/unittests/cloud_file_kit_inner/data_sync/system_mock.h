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
#ifndef MOCK_SYSTEM_H
#define MOCK_SYSTEM_H

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <string>

namespace OHOS {
namespace system {
class ISystem {
public:
    virtual std::string GetParameter(const std::string &key, const std::string &def) = 0;
    virtual bool SetParameter(const std::string &key, const std::string &def) = 0;
    virtual ~ISystem() = default;
    static inline std::shared_ptr<ISystem> system_{nullptr};
};

class SystemMock : public ISystem {
public:
    MOCK_METHOD2(GetParameter, std::string(const std::string &key, const std::string &def));
    MOCK_METHOD2(SetParameter, bool(const std::string &key, const std::string &def));
};

std::string GetParameter(const std::string &key, const std::string &def)
{
    if (ISystem::system_ != nullptr) {
        return ISystem::system_->GetParameter(key, def);
    }
    return "";
}

bool SetParameter(const std::string &key, const std::string &def)
{
    if (ISystem::system_ != nullptr) {
        return ISystem::system_->SetParameter(key, def);
    }
    return true;
}
} // system
} // OHOS
#endif // MOCK_SYSTEM_H