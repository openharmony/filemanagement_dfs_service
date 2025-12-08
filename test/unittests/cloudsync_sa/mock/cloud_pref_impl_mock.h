/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#ifndef CLOUD_PERF_IMPL_MOCK_H
#define CLOUD_PERF_IMPL_MOCK_H

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <string>

namespace OHOS::FileManagement::CloudSync {
class CloudPrefImplMock {
public:
    MOCK_METHOD2(SetString, void(const std::string &, const std::string &));
    MOCK_METHOD2(GetString, void(const std::string &, std::string &));
    MOCK_METHOD2(SetLong, void(const std::string &, const int64_t));
    MOCK_METHOD2(GetLong, void(const std::string &, int64_t &));
    MOCK_METHOD2(SetInt, void(const std::string &, const int32_t));
    MOCK_METHOD2(GetInt, void(const std::string &, int32_t &));
    MOCK_METHOD2(SetBool, void(const std::string &, const bool));
    MOCK_METHOD2(GetBool, void(const std::string &, bool &));

    static inline std::shared_ptr<CloudPrefImplMock> proxy_ = nullptr;
};
} // namespace OHOS::FileManagement::CloudSync
#endif // CLOUD_PERF_IMPL_MOCK_H