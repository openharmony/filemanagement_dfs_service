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

#include "data_syncer_rdb_store.h"

#include "data_convertor.h"
#include "data_syncer_rdb_col.h"
#include "dfs_error.h"
#include "utils_log.h"
#include "rdb_helper.h"
#include "rdb_sql_utils.h"
#include "rdb_store_config.h"

namespace OHOS::FileManagement::CloudSync {
using namespace std;

DataSyncerRdbStore &DataSyncerRdbStore::GetInstance()
{
    static DataSyncerRdbStore instance;
    return instance;
}

int32_t DataSyncerRdbStore::RdbInit()
{
    return E_OK;
}

int32_t DataSyncerRdbStore::Insert(int32_t userId, const std::string &bundleName)
{
    return E_OK;
}

int32_t DataSyncerRdbStore::UpdateSyncState(int32_t userId, const string &bundleName, SyncState syncState)
{
    return E_OK;
}

int32_t DataSyncerRdbStore::GetLastSyncTime(int32_t userId, const string &bundleName, int64_t &time)
{
    return E_OK;
}

int32_t DataSyncerRdbStore::QueryDataSyncer(int32_t userId, std::shared_ptr<NativeRdb::ResultSet> &resultSet)
{
    return E_OK;
}

int32_t DataSyncerRdbStore::Query(NativeRdb::AbsRdbPredicates predicates,
    std::shared_ptr<NativeRdb::ResultSet> &resultSet)
{
    return E_OK;
}


int32_t DataSyncerRdbCallBack::OnCreate(NativeRdb::RdbStore &store)
{
    return E_OK;
}

int32_t DataSyncerRdbCallBack::OnUpgrade(NativeRdb::RdbStore &store, int32_t oldVersion, int32_t newVersion)
{
    return E_OK;
}
} // namespace OHOS::FileManagement::CloudSync
