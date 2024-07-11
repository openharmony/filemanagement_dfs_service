/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#ifndef SECURITY_GUARD_VALUES_BUCKET_MOCK_H
#define SECURITY_GUARD_VALUES_BUCKET_MOCK_H

#include <cstdint>
#include <string>

#include <gtest/gtest.h>

namespace OHOS::NativeRdb {
class ValuesBucket {
public:
    ValuesBucket() = default;
    void PutString(const std::string &columnName, const std::string &value) {};
    void PutInt(const std::string &columnName, int value) {};
    void PutLong(const std::string &columnName, int64_t value) {};
};

class ValueObject {
};
} // namespace OHOS::NativeRdb
#endif // SECURITY_GUARD_VALUES_BUCKET_MOCK_H