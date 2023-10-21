/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_CLOUD_DISK_SERVICE_RDBSTORE_H
#define OHOS_CLOUD_DISK_SERVICE_RDBSTORE_H

#include "rdb_errno.h"
#include "rdb_helper.h"
#include "rdb_open_callback.h"
#include "rdb_store.h"
#include "rdb_store_config.h"
#include "timer.h"
#include <memory>

#include <clouddisk_db_const.h>
#include <file_column.h>

namespace OHOS {
namespace FileManagement {
namespace CloudDisk {

class CloudDiskRdbStore final {
public:
    CloudDiskRdbStore() = default;
    ~CloudDiskRdbStore() = default;

    std::shared_ptr<NativeRdb::RdbStore> RdbInit(const std::string &bundleName,
                                                 const int32_t &userId);
    void Stop();

private:
    std::shared_ptr<NativeRdb::RdbStore> rdbStore_ = nullptr;
    NativeRdb::RdbStoreConfig config_{""};
};

class CloudDiskDataCallBack : public NativeRdb::RdbOpenCallback {
public:
    int32_t OnCreate(NativeRdb::RdbStore &rdbStore) override;
    int32_t OnUpgrade(NativeRdb::RdbStore &rdbStore, int32_t oldVersion, int32_t newVersion) override;
};
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS

#endif // OHOS_CLOUD_DISK_SERVICE_RDBSTORE_H