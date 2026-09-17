/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "database_manager_mock.h"

#include "dfs_error.h"

namespace OHOS::FileManagement::CloudSync {
DataSyncerRdbStore &DataSyncerRdbStore::GetInstance()
{
    static DataSyncerRdbStore instance;
    return instance;
}

int32_t DataSyncerRdbStore::QueryCloudSync(int32_t userId, const std::string &bundleName,
    std::shared_ptr<NativeRdb::ResultSet> &resultSet)
{
    if (DataSyncerRdbStoreMock::proxy_ != nullptr) {
        return DataSyncerRdbStoreMock::proxy_->QueryCloudSync(userId, bundleName, resultSet);
    }
    return E_OK;
}

int32_t DataSyncerRdbStore::RdbInit()
{
    return E_OK;
}
} // namespace OHOS::FileManagement::CloudSync
