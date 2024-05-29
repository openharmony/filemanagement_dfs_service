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

#ifndef OHOS_FILEMGMT_DATA_SYNCER_RDB_STORE_H
#define OHOS_FILEMGMT_DATA_SYNCER_RDB_STORE_H

#include "rdb_open_callback.h"
#include "rdb_store.h"
#include "sync_state_manager.h"

namespace OHOS::FileManagement::CloudSync {
class DataSyncerRdbStore {
public:
    static DataSyncerRdbStore &GetInstance();
    ~DataSyncerRdbStore() = default;

    int32_t Insert(int32_t userId, const std::string &bundleName);
    int32_t UpdateSyncState(int32_t userId, const std::string &bundleName, SyncState syncState);
    int32_t GetLastSyncTime(int32_t userId, const std::string &bundleName, int64_t &time);
    int32_t QueryDataSyncer(int32_t userId, std::shared_ptr<NativeRdb::ResultSet> &resultSet);
private:
    DataSyncerRdbStore() = default;
    int32_t Query(NativeRdb::AbsRdbPredicates predicates, std::shared_ptr<NativeRdb::ResultSet> &resultSet);
    int32_t RdbInit();
    std::shared_ptr<NativeRdb::RdbStore> rdb_;
    std::mutex rdbMutex_;
};

class DataSyncerRdbCallBack : public NativeRdb::RdbOpenCallback {
public:
    int32_t OnCreate(NativeRdb::RdbStore &rdbStore) override;
    int32_t OnUpgrade(NativeRdb::RdbStore &rdbStore, int32_t oldVersion, int32_t newVersion) override;
};
} // namespace OHOS::FileManagement::CloudSync

#endif // OHOS_FILEMGMT_DATA_SYNCER_RDB_STORE_H