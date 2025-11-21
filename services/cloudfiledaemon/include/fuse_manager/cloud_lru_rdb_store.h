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

#ifndef CLOUD_LRU_RDB_STORE_H
#define CLOUD_LRU_RDB_STORE_H

#include <cstdint>
#include <cstring>
#include <mutex>

#include "rdb_open_callback.h"
#include "rdb_store.h"

namespace OHOS {
namespace FileManagement {
namespace CloudFile {

struct CloudNodeInfo {
    int64_t inodeIno;
    int64_t parentIno;
    std::string cloudId;
    int64_t hash;
};

class CloudLruRdbStore final {
public:
    static CloudLruRdbStore &GetInstance();
    CloudLruRdbStore(int32_t userId);
    ~CloudLruRdbStore() = default;
    int32_t RdbInit(const std::string bundleName);
    int32_t LookUp(int64_t ino, const std::string bundleName, CloudNodeInfo &info);
    int32_t Insert(std::shared_ptr<NativeRdb::RdbStore> rdb, NativeRdb::ValuesBucket& values);
    int32_t Update(int64_t ino, std::shared_ptr<NativeRdb::RdbStore> rdb, NativeRdb::ValuesBucket& values);
    int32_t InsertOrUpdate(const std::string bundleName, const CloudNodeInfo info);
    int32_t Delete(int64_t ino, const std::string bundleName);

private:
    std::unordered_map<std::string, std::shared_ptr<NativeRdb::RdbStore>> rdbMap_;
    std::mutex rdbMutex_;
    int32_t userId_{0};
};

class CloudLruRdbCallBack : public NativeRdb::RdbOpenCallback {
public:
    int32_t OnCreate(NativeRdb::RdbStore &rdbStore) override;
    int32_t OnUpgrade(NativeRdb::RdbStore &rdbStore, int32_t oldVersion, int32_t newVersion) override;
};
} // namespace CloudFile
} // namespace FileManagement
} // namespace OHOS

#endif // CLOUD_LRU_DATABASE_H