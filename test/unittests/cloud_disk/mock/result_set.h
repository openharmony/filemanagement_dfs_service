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

#ifndef NATIVE_RDB_RESULT_SET_H
#define NATIVE_RDB_RESULT_SET_H

#include <cstdint>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace OHOS::NativeRdb {
class ResultSet {
public:
    ResultSet() = default;
    ~ResultSet() = default;
    int32_t GetRowCount(int32_t &count);
};

class ResultSetMock : public ResultSet {
public:
    MOCK_METHOD1(GetRowCount, int32_t(int32_t &));
    static inline std::shared_ptr<ResultSetMock> proxy_ = nullptr;
    static inline int32_t count_ = 0;
};
} // namespace OHOS::NativeRdb
#endif // NATIVE_RDB_RESULT_SET_H