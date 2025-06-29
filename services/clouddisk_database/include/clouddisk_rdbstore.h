/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "cloud_file_utils.h"
#include "clouddisk_db_const.h"
#include "file_column.h"
#include "meta_file.h"
#include "transaction.h"

namespace OHOS {
namespace FileManagement {
namespace CloudDisk {

class CloudDiskRdbStore final {
public:
    CloudDiskRdbStore(const std::string &bundleName, const int32_t &userId);
    ~CloudDiskRdbStore();

    int32_t RdbInit();
    std::shared_ptr<NativeRdb::RdbStore> GetRaw();

    int32_t LookUp(const std::string &parentCloudId, const std::string &fileName, CloudDiskFileInfo &info);
    int32_t GetAttr(const std::string &cloudId, CloudDiskFileInfo &info);
    int32_t SetAttr(const std::string &fileName, const std::string &parentCloudId, const std::string &cloudId,
        const unsigned long long &size);
    int32_t ReadDir(const std::string &cloudId, std::vector<CloudDiskFileInfo> &infos);
    int32_t MkDir(const std::string &cloudId, const std::string &parentCloudId,
        const std::string &directoryName, bool noNeedUpload);
    int32_t Create(const std::string &cloudId, const std::string &parentCloudId,
        const std::string &fileName, bool noNeedUpload);
    int32_t Write(const std::string &fileName, const std::string &parentCloudId, const std::string &cloudId);
    int32_t GetXAttr(const std::string &cloudId, const std::string &key, std::string &value,
        const CacheNode &node = {}, const std::string &extAttrKey = "");
    int32_t SetXAttr(const std::string &cloudId, const std::string &key, const std::string &value,
        const std::string &name = "", const std::string &parentCloudId = "");
    int32_t Rename(const std::string &oldParentCloudId, const std::string &oldFileName,
        const std::string &newParentCloudId, const std::string &newFileName);
    int32_t Unlink(const std::string &cloudId, const int32_t &noUpload);
    int32_t RecycleSetXattr(const std::string &name, const std::string &parentCloudId,
        const std::string &cloudId, const std::string &value);
    int32_t LocationSetXattr(const std::string &name, const std::string &parentCloudId,
        const std::string &cloudId, const std::string &value);
    int32_t HasTHMSetXattr(const std::string &name, const std::string &key,
        const std::string &cloudId, const std::string &value);
    int32_t FavoriteSetXattr(const std::string &cloudId, const std::string &value);
    int32_t LocationGetXattr(const std::string &name, const std::string &key, std::string &value,
        const std::string &parentCloudId);
    int32_t FavoriteGetXattr(const std::string &cloudId, const std::string &key, std::string &value);
    int32_t FileStatusGetXattr(const std::string &cloudId, const std::string &key, std::string &value);
    int32_t TimeRecycledGetXattr(const std::string &cloudId, const std::string &key, std::string &value);
    int32_t GetHasChild(const std::string &cloudId, bool &hasChild);
    int32_t GetRowId(const std::string &cloudId, int64_t &rowId);
    int32_t GetParentCloudId(const std::string &cloudId, std::string &parentCloudId);
    int32_t ExtAttributeSetXattr(const std::string &cloudId, const std::string &value, const std::string &key);
    int32_t GetExtAttr(const std::string &cloudId, std::string &value, int32_t &position);
    int32_t GetExtAttrValue(const std::string &cloudId, const std::string &key, std::string &value);
    int32_t GetRecycleInfo(std::shared_ptr<NativeRdb::Transaction> transaction,
        const std::string &cloudId, int64_t &rowId, int32_t &position, std::string &attr);
    int32_t GetSourcePath(const std::string &attr, const std::string &parentCloudId, std::string &sourcePath);
    int32_t SourcePathSetValue(const std::string &cloudId, const std::string &attr, NativeRdb::ValuesBucket &setXattr);
    int32_t HandleRecycleXattr(const std::string &name, const std::string &parentCloudId, const std::string &cloudId);
    int32_t HandleRestoreXattr(const std::string &name, const std::string &parentCloudId, const std::string &cloudId);

    /* clouddisk syncer */
    int32_t GetDirtyType(const std::string &cloudId, int32_t &fileStatus);
    int32_t GetCurNode(const std::string &cloudId, CacheNode &curNode);
    int32_t GetParentNode(const std::string &parentCloudId, std::string &nextCloudId, std::string &fileName);
    int32_t GetUriFromDB(const std::string &parentCloudId, std::string &uri);
    int32_t GetNotifyUri(const CacheNode &cacheNode, std::string &uri);
    int32_t GetNotifyData(const CacheNode &cacheNode, NotifyData &notifyData);
    int32_t CheckRootIdValid();
    int32_t InsertCopyData(std::string srcCloudId, std::string destCloudId, std::string destParentCloudId,
                           std::shared_ptr<NativeRdb::ResultSet> resultSet);
    int32_t CopyFile(std::string srcCloudId, std::string destCloudId, std::string bundleName,
                     const int userId, std::string destPath);
    int32_t CreateDentryFile(MetaBase metaBase, std::string destParentCloudId);
    int32_t GetSrcCloudId(const std::string &cloudId, std::string &srcCloudId);
    int32_t UpdateTHMStatus(std::shared_ptr<CloudDiskMetaFile> metaFile, MetaBase &metabase, int32_t status);

    void DatabaseRestore();

    static const int32_t BATCH_LIMIT_SIZE = 500;

private:
    void Stop();
    int32_t UnlinkSynced(const std::string &cloudId);
    int32_t UnlinkLocal(const std::string &cloudId);
    int32_t ReBuildDatabase(const std::string &databasePath);
    int32_t CheckIsConflict(const std::string &name, const std::string &parentCloudId, std::string &newName);
    int32_t RestoreUpdateRdb(const std::string &cloudId, const struct RestoreInfo &restoreInfo,
        const NativeRdb::ValuesBucket &setXattr);

    std::shared_ptr<NativeRdb::RdbStore> rdbStore_;
    NativeRdb::RdbStoreConfig config_{""};
    const int32_t CONNECT_SIZE = 8;
    std::string bundleName_;
    int32_t userId_{0};
    std::string tableName_ = FileColumn::FILES_TABLE;
    std::mutex rdbMutex_;
    std::mutex backupMutex_;
    std::string rootId_;
    static std::map<char, bool> illegalCharacter;
};

class CloudDiskDataCallBack : public NativeRdb::RdbOpenCallback {
public:
    int32_t OnCreate(NativeRdb::RdbStore &rdbStore) override;
    int32_t OnUpgradeExtend(NativeRdb::RdbStore &store, int32_t oldVersion, int32_t newVersion);
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