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

#ifndef SECURITY_GUARD_RDB_STORE_CONFIG_MOCK_H
#define SECURITY_GUARD_RDB_STORE_CONFIG_MOCK_H

#include <cstdint>
#include <string>

#include <gtest/gtest.h>

namespace OHOS::NativeRdb {
enum class SecurityLevel : int32_t {
    S0,
    S1,
    S2,
    S3,
};

enum class StorageMode {
    MODE_MEMORY = 101,
    MODE_DISK,
};

class RdbStoreConfig {
public:
    RdbStoreConfig(std::string path) {};
    void SetSecurityLevel(SecurityLevel level) {};
    void SetStorageMode(StorageMode storageMode) {};
};
} // namespace OHOS::NativeRdb
#endif // SECURITY_GUARD_RDB_STORE_CONFIG_MOCK_H