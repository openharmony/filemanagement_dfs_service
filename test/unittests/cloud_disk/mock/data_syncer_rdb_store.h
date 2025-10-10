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

#ifndef OHOS_FILEMGMT_DATA_SYNCER_RDB_STORE_H
#define OHOS_FILEMGMT_DATA_SYNCER_RDB_STORE_H

#include <cstdint>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <string>

#include "result_set.h"

namespace OHOS::FileManagement::CloudSync {
class DataSyncerRdbStore {
public:
    static DataSyncerRdbStore &GetInstance();
    ~DataSyncerRdbStore() = default;

    int32_t QueryCloudSync(int32_t userId,
                           const std::string &bundleName,
                           std::shared_ptr<NativeRdb::ResultSet> &resultSet);
};

class DataSyncerRdbStoreMock : public DataSyncerRdbStore {
public:
    MOCK_METHOD3(QueryCloudSync, int32_t(int32_t, const std::string &, std::shared_ptr<NativeRdb::ResultSet> &));
    static inline std::shared_ptr<DataSyncerRdbStoreMock> proxy_ = nullptr;
};
} // namespace OHOS::FileManagement::CloudSync
#endif // OHOS_FILEMGMT_DATA_SYNCER_RDB_STORE_H