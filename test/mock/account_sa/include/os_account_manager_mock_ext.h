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
#ifndef OHOS_OS_ACCOUNT_MOCK_EXT_H
#define OHOS_OS_ACCOUNT_MOCK_EXT_H

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>
#include "os_account_manager.h"

namespace OHOS {
class IOsAccountManagerMethod {
public:
    virtual ~IOsAccountManagerMethod() = default;
public:
    virtual int32_t GetForegroundOsAccountLocalId(int32_t &foregroundLocalId) = 0;

public:
    static inline std::shared_ptr<IOsAccountManagerMethod> osMethod_ = nullptr;
};

class OsAccountManagerMethodMockExt : public IOsAccountManagerMethod {
public:
    MOCK_METHOD1(GetForegroundOsAccountLocalId, int32_t(int32_t &foregroundLocalId));
};
}
#endif