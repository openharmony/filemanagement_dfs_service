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

#ifndef OHOS_CLOUD_DISK_SERVICE_OPERATION_LOG_STORE_H
#define OHOS_CLOUD_DISK_SERVICE_OPERATION_LOG_STORE_H

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>

#include "rdb_errno.h"
#include "rdb_helper.h"
#include "rdb_open_callback.h"
#include "rdb_store.h"
#include "rdb_store_config.h"

namespace OHOS {
namespace FileManagement {
namespace CloudDisk {

class OperationLogStore {
public:
    static OperationLogStore& GetInstance();
    int32_t Init(int32_t userId);
    std::shared_ptr<NativeRdb::RdbStore> GetRaw();
    void Stop();

private:
    OperationLogStore() = default;
    ~OperationLogStore() = default;

    int32_t RdbInit(int32_t userId);
    int32_t CreateDirectory(const std::string& dirPath);
    bool TryOpenRdbStore(const std::string& customDir, const std::string& databasePath, int32_t& errCode);
    int32_t CheckAndCreateDir(const std::string& dirPath);

    std::shared_ptr<NativeRdb::RdbStore> rdbStore_;
    NativeRdb::RdbStoreConfig config_{""};
    std::mutex mutex_;
    int32_t userId_ = 0;
    //是否已进行数据库初始化
    bool isInited_ = false;
};

class OperationLogRdbCallBack : public NativeRdb::RdbOpenCallback {
public:
    int32_t OnCreate(NativeRdb::RdbStore &rdbStore) override;
    int32_t OnUpgrade(NativeRdb::RdbStore &rdbStore, int32_t oldVersion, int32_t newVersion) override;
};

} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS
#endif // OHOS_CLOUD_DISK_SERVICE_OPERATION_LOG_STORE_H
