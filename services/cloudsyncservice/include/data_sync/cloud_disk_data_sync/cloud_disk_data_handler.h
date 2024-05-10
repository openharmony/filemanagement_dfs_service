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

#ifndef OHOS_CLOUD_SYNC_SERVICE_CLOUD_DISK_DATA_HANDLER_H
#define OHOS_CLOUD_SYNC_SERVICE_CLOUD_DISK_DATA_HANDLER_H

#include "cloud_disk_data_convertor.h"
#include "clouddisk_type_const.h"
#include "medialibrary_db_const.h"
#include "medialibrary_type_const.h"
#include "meta_file.h"
#include "rdb_data_handler.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
struct ContextData {
    std::string parentCloudId;
    std::string fileName;
    ContextData(const std::string &parentCloudId, const std::string &fileName)
    {
        this->fileName = fileName;
        this->parentCloudId = parentCloudId;
    }
};
class CloudDiskDataHandler : public RdbDataHandler {
public:
    CloudDiskDataHandler(int32_t userId, const std::string &bundleName,
                         std::shared_ptr<NativeRdb::RdbStore> rdb, std::shared_ptr<bool> stopFlag);
    virtual ~CloudDiskDataHandler() = default;

    void GetFetchCondition(FetchCondition &cond) override;
    virtual int32_t OnFetchRecords(std::shared_ptr<std::vector<DriveKit::DKRecord>> &records,
                                   OnFetchParams &params) override;
    virtual int32_t GetRetryRecords(std::vector<DriveKit::DKRecordId> &records) override;
    virtual int32_t GetCheckRecords(std::vector<DriveKit::DKRecordId> &checkRecords,
                                    const std::shared_ptr<std::vector<DriveKit::DKRecord>> &records) override;
    int32_t GetDownloadAsset(std::string cloudId, std::vector<DriveKit::DKDownloadAsset> &outAssetsToDownload,
        std::shared_ptr<DentryContext> dentryContext = nullptr) override;
    int32_t Clean(const int32_t action) override;
    int32_t GetCreatedRecords(std::vector<DriveKit::DKRecord> &records) override;
    int32_t GetDeletedRecords(std::vector<DriveKit::DKRecord> &records) override;
    int32_t GetMetaModifiedRecords(std::vector<DriveKit::DKRecord> &records) override;
    int32_t GetFileModifiedRecords(std::vector<DriveKit::DKRecord> &records) override;
    int32_t OnCreateRecords(const std::map<DriveKit::DKRecordId,
        DriveKit::DKRecordOperResult> &map) override;
    int32_t OnDeleteRecords(const std::map<DriveKit::DKRecordId,
        DriveKit::DKRecordOperResult> &map) override;
    int32_t OnModifyMdirtyRecords(const std::map<DriveKit::DKRecordId,
        DriveKit::DKRecordOperResult> &map) override;
    int32_t OnModifyFdirtyRecords(const std::map<DriveKit::DKRecordId,
        DriveKit::DKRecordOperResult> &map) override;
    int32_t OnDownloadSuccess(const DriveKit::DKDownloadAsset &asset,
        std::shared_ptr<DriveKit::DKContext> context = nullptr) override;
    int32_t OnDownloadAssets(const std::map<DriveKit::DKDownloadAsset, DriveKit::DKDownloadResult> &resultMap) override;
    int32_t OnDownloadAssets(const DriveKit::DKDownloadAsset &asset) override;
    void Reset();
    int32_t CleanCache(const std::string &uri);
private:
    static inline const int32_t LIMIT_SIZE = 5;
    static inline const int32_t CREATE_LIMIT_SIZE = 20;
    static inline const int32_t PULL_LIMIT_SIZE = 200;
    static inline const int32_t MODIFY_BATCH_NUM = 20;
    static inline const int32_t DELETE_BATCH_NUM = 20;
    static const int32_t MAX_RENAME = 10;
    DriveKit::DKRecordType recordType_ = "file";
    DriveKit::DKFieldKeyArray desiredKeys_;
    DriveKit::DKFieldKeyArray checkedKeys_ = {"version", "id"};
    std::vector<std::string> modifyFailSet_;
    std::vector<std::string> createFailSet_;

    int32_t userId_{0};
    std::string bundleName_;
    DriveKit::DKRecordId rootId_;

    int32_t CleanCloudRecord(const int32_t action);
    void HandleCreateConvertErr(int32_t err, NativeRdb::ResultSet &resultSet);
    void HandleFdirtyConvertErr(int32_t err, NativeRdb::ResultSet &resultSet);
    CloudDiskDataConvertor createConvertor_ = {
        userId_, bundleName_, FILE_CREATE,
        std::bind(&CloudDiskDataHandler::HandleCreateConvertErr, this, std::placeholders::_1, std::placeholders::_2)
    };
    CloudDiskDataConvertor deleteConvertor_ = { userId_, bundleName_, FILE_DELETE };
    CloudDiskDataConvertor mdirtyConvertor_ = { userId_, bundleName_, FILE_METADATA_MODIFY };
    CloudDiskDataConvertor fdirtyConvertor_ = {
        userId_, bundleName_, FILE_DATA_MODIFY,
        std::bind(&CloudDiskDataHandler::HandleFdirtyConvertErr, this, std::placeholders::_1, std::placeholders::_2)
    };
    int32_t OnCreateRecordSuccess(const std::pair<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> &entry,
        const std::unordered_map<std::string, LocalInfo> &localMap);
    int32_t OnDeleteRecordSuccess(const std::pair<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> &entry);
    int32_t OnModifyRecordSuccess(const std::pair<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> &entry,
        const std::unordered_map<std::string, LocalInfo> &localMap);
    bool IsTimeChanged(const DriveKit::DKRecord &record, const std::unordered_map<std::string, LocalInfo> &localMap,
        const std::string &cloudId, const std::string &type);
    int32_t GetLocalInfo(const std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> &map,
        std::unordered_map<std::string, LocalInfo> &infoMap, const std::string &type);
    int32_t BuildInfoMap(const std::shared_ptr<NativeRdb::ResultSet> resultSet,
        std::unordered_map<std::string, LocalInfo> &cloudMap, const std::string &type);
    std::tuple<std::shared_ptr<NativeRdb::ResultSet>, std::map<std::string, int>> QueryLocalByCloudId(
        const std::vector<std::string> &recordIds);
    int32_t PullRecordInsert(DriveKit::DKRecord &record, OnFetchParams &params);
    int32_t PullRecordUpdate(DriveKit::DKRecord &record, NativeRdb::ResultSet &local,
                             OnFetchParams &params);
    int32_t PullRecordDelete(DriveKit::DKRecord &record, NativeRdb::ResultSet &local);
    int32_t RecycleFile(const std::string &recordId, const std::string &parentCloudId, const std::string &name,
        NativeRdb::ResultSet &local);
    int32_t GetMetaFilePath(const std::string &cloudId, std::string &path);
    int32_t PullRecordConflict(DriveKit::DKRecord &record);
    int32_t HandleConflict(const std::shared_ptr<NativeRdb::ResultSet> resultSet, std::string &fullName,
        const int &lastDot, const DriveKit::DKRecord &record);
    int32_t FindRenameFile(const std::shared_ptr<NativeRdb::ResultSet> resultSet, std::string &renameFileCloudId,
                           std::string &fullName, const int &lastDot);
    int32_t ConflictReName(const std::string &cloudId, std::string newFileName, const DriveKit::DKRecord &record);
    int SetRetry(const std::string &recordId);
    void AppendFileToDownload(const std::string &cloudId,
                              const std::string &fieldKey,
                              std::vector<DriveKit::DKDownloadAsset> &assetsToDownload);
    int32_t PushFileStatus(std::vector<DriveKit::DKRecord> &records);
    int32_t InsertRDBAndDentryFile(DriveKit::DKRecord &record, NativeRdb::ValuesBucket &values);
    int32_t UpdateRDBAndDentryFile(DriveKit::DKRecord &record, MetaBase &metaBase, const std::string &oldFileName,
        const std::string &oldParentCloudId, std::function<int32_t()> updateRDBCallback);
    int32_t DeleteRDBAndDentryFile(MetaBase &metaBase, std::string &parentCloudId, std::string &name,
        DriveKit::DKRecord &record, NativeRdb::ResultSet &local);
    int32_t UpdateDBDentryAndUnlink(DriveKit::DKRecord &record, NativeRdb::ResultSet &local,
        NativeRdb::ValuesBucket &values, const std::string &oldFileName, const std::string &oldParentCloudId);
    int32_t GetParentCloudId(std::shared_ptr<NativeRdb::RdbStore> rdbStore, const std::string &cloudId,
        std::string &parentCloudId);
    int32_t GetCleanCacheData(const std::string &path, std::string &parentCloudId, std::string &fileName,
        std::string &cloudId);

    CloudDiskDataConvertor localConvertor_ = { userId_, bundleName_, FILE_DOWNLOAD };
    int64_t UTCTimeMilliSeconds();
    std::mutex rdbMutex_;
};
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
#endif // OHOS_CLOUD_SYNC_SERVICE_CLOUD_DISK_DATA_HANDLER_H
