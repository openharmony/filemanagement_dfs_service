/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef SECURITY_GUARD_RDB_HELPER_MOCK_H
#define SECURITY_GUARD_RDB_HELPER_MOCK_H

#include <memory>
#include <mutex>
#include <string>

#include "gmock/gmock.h"
#include "singleton.h"

#include "rdb_open_callback.h"
#include "rdb_store.h"
#include "rdb_store_config.h"

namespace OHOS::NativeRdb {
class RdbHelperInterface {
public:
    virtual ~RdbHelperInterface() = default;
    virtual std::shared_ptr<RdbStore> GetRdbStore(
        const RdbStoreConfig &config, int version, RdbOpenCallback &openCallback, int &errCode) = 0;
};

class MockRdbHelperInterface : public RdbHelperInterface {
public:
    MockRdbHelperInterface() = default;
    ~MockRdbHelperInterface() override = default;
    MOCK_METHOD4(GetRdbStore, std::shared_ptr<RdbStore>(const RdbStoreConfig &config, int version,
        RdbOpenCallback &openCallback, int &errCode));
};

class RdbHelper {
public:
    static std::shared_ptr<RdbStore> GetRdbStore(
        const RdbStoreConfig &config, int version, RdbOpenCallback &openCallback, int &errCode)
    {
        if (instance_ == nullptr) {
            return nullptr;
        }
        return instance_->GetRdbStore(config, version, openCallback, errCode);
    };

    static std::shared_ptr<MockRdbHelperInterface> GetInterface()
    {
        if (instance_ == nullptr) {
            std::lock_guard<std::mutex> lock(mutex_);
            if (instance_ == nullptr) {
                instance_ = std::make_shared<MockRdbHelperInterface>();
            }
        }
        return instance_;
    };

    static void DelInterface()
    {
        if (instance_ != nullptr) {
            instance_.reset();
        }
    };

private:
    static std::shared_ptr<MockRdbHelperInterface> instance_;
    static std::mutex mutex_;
};
} // namespace OHOS::NativeRdb
#endif
