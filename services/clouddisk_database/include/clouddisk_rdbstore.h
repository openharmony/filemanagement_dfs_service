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

#ifndef OHOS_CLOUD_DISK_SERVICE_RDBSTORE_H
#define OHOS_CLOUD_DISK_SERVICE_RDBSTORE_H

#include "rdb_errno.h"
#include "rdb_helper.h"
#include "rdb_open_callback.h"
#include "rdb_store.h"
#include "rdb_store_config.h"

#include "drive_kit.h"
#include "clouddisk_db_const.h"
#include "file_column.h"
#include "cloud_file_utils.h"

namespace OHOS {
namespace FileManagement {
namespace CloudDisk {

class CloudDiskRdbStore final {
public:
    CloudDiskRdbStore(const std::string &bundleName, const int32_t &userId);
    ~CloudDiskRdbStore();

    int32_t RdbInit();
    std::shared_ptr<NativeRdb::RdbStore> GetRaw();
    void InitRootId();

    int32_t LookUp(const std::string &parentCloudId, const std::string &fileName, CloudDiskFileInfo &info);
    int32_t GetAttr(const std::string &cloudId, CloudDiskFileInfo &info);
    int32_t ReadDir(const std::string &cloudId, std::vector<CloudDiskFileInfo> &infos);
    int32_t MkDir(const std::string &cloudId, const std::string &parentCloudId,
        const std::string &directoryName);
    int32_t Create(const std::string &cloudId, const std::string &parentCloudId,
        const std::string &fileName);
    int32_t Write(const std::string &cloudId);
    int32_t GetXAttr(const std::string &cloudId, const std::string &position, std::string &value);
    int32_t SetXAttr(const std::string &cloudId, const std::string &position, const std::string &value);
    int32_t Rename(const std::string &cloudId, const std::string &newParentCloudId, const std::string &newFileName);
    int32_t UnlinkSyncedFile(const std::string &cloudId, const int32_t &isDirectory, const int32_t &position);
    int32_t UnlinkLocalFile(const std::string &cloudId, const int32_t &isDirectory, const int32_t &position);
    int32_t Unlink(const std::string &cloudId);

private:
    void Stop();
    const std::string CloudSyncTriggerFunc(const std::vector<std::string> &args);

    std::shared_ptr<NativeRdb::RdbStore> rdbStore_;
    NativeRdb::RdbStoreConfig config_{""};
    std::string bundleName_;
    int32_t userId_{0};
    DriveKit::DKRecordId rootId_;
};

class CloudDiskDataCallBack : public NativeRdb::RdbOpenCallback {
public:
    int32_t OnCreate(NativeRdb::RdbStore &rdbStore) override;
    int32_t OnUpgrade(NativeRdb::RdbStore &rdbStore, int32_t oldVersion, int32_t newVersion) override;
};

#define RDBPTR_IS_NULLPTR(rdbStore_)    \
    do {    \
        if ((rdbStore_) == nullptr) {    \
            LOGE("rdbStore_ is nullptr");    \
            return E_RDB;    \
        }    \
    } while (0)

#define CLOUDID_IS_NULL(cloudId)    \
    do {    \
        if ((cloudId).empty()) {    \
            LOGE("cloudId is null");    \
            return E_INVAL_ARG;    \
        }    \
    } while (0)
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS

#endif // OHOS_CLOUD_DISK_SERVICE_RDBSTORE_H