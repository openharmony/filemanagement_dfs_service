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

#ifndef OHOS_CLOUD_SYNC_SERVICE_CLOUD_DISK_DATA_HANDLER_H
#define OHOS_CLOUD_SYNC_SERVICE_CLOUD_DISK_DATA_HANDLER_H

#include "cloud_disk_data_convertor.h"
#include "medialibrary_db_const.h"
#include "medialibrary_type_const.h"
#include "rdb_data_handler.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
class CloudDiskDataHandler : public RdbDataHandler {
public:
    CloudDiskDataHandler(int32_t userId, const std::string &bundleName, std::shared_ptr<NativeRdb::RdbStore> rdb);
    virtual ~CloudDiskDataHandler() = default;

    void GetFetchCondition(FetchCondition &cond) override;
    virtual int32_t OnFetchRecords(std::shared_ptr<std::vector<DriveKit::DKRecord>> &records,
                                   OnFetchParams &params) override;
    virtual int32_t GetRetryRecords(std::vector<DriveKit::DKRecordId> &records) override;
    virtual int32_t GetCheckRecords(std::vector<DriveKit::DKRecordId> &checkRecords,
                                    const std::shared_ptr<std::vector<DriveKit::DKRecord>> &records) override;
    int32_t GetDownloadAsset(std::string cloudId, std::vector<DriveKit::DKDownloadAsset> &outAssetsToDownload) override;
    int32_t Clean(const int action) override;
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
    int32_t OnDownloadSuccess(const DriveKit::DKDownloadAsset &asset) override;
    int32_t OnDownloadAssets(const std::map<DriveKit::DKDownloadAsset, DriveKit::DKDownloadResult> &resultMap) override;
    int32_t OnDownloadAssets(const DriveKit::DKDownloadAsset &asset) override;

private:
    static inline const int32_t LIMIT_SIZE = 5;
    static inline const int32_t MODIFY_BATCH_NUM = 20;
    static inline const int32_t DELETE_BATCH_NUM = 20;
    DriveKit::DKRecordType recordType_ = "clouddrive";
    DriveKit::DKFieldKeyArray desiredKeys_;
    DriveKit::DKFieldKeyArray checkedKeys_ = {"version", "id"};
    std::vector<std::string> modifyFailSet_;
    std::vector<std::string> createFailSet_;

    int32_t userId_;
    std::string bundleName_;

    CloudDiskDataConvertor cleanConvertor_ = { userId_, bundleName_, FILE_CLEAN };
    int32_t ClearCloudInfo(const std::string &cloudId);
    int32_t CleanCloudRecord(NativeRdb::ResultSet &local, const int action, const std::string &filePath);
    void HandleCreateConvertErr(NativeRdb::ResultSet &resultSet);
    CloudDiskDataConvertor createConvertor_ = {
        userId_, bundleName_, FILE_CREATE,
        std::bind(&CloudDiskDataHandler::HandleCreateConvertErr, this, std::placeholders::_1)
    };
    std::tuple<std::shared_ptr<NativeRdb::ResultSet>, std::map<std::string, int>> QueryLocalByCloudId(
        const std::vector<std::string> &recordIds);
    int32_t PullRecordInsert(DriveKit::DKRecord &record, OnFetchParams &params);
    int32_t PullRecordUpdate(DriveKit::DKRecord &record, NativeRdb::ResultSet &local,
                             OnFetchParams &params);
    int32_t PullRecordDelete(DriveKit::DKRecord &record, NativeRdb::ResultSet &local);
    int32_t RecycleFile(const std::string &recordId);
    int32_t GetMetaFilePath(const std::string &cloudId, std::string &path);
    int32_t PullRecordConflict(DriveKit::DKRecord &record, bool &comflag);
    int SetRetry(const std::string &recordId);
    int32_t UpdateDirPosition(std::string cloudId);

    CloudDiskDataConvertor localConvertor_ = { userId_, bundleName_, FILE_DOWNLOAD };
    int64_t UTCTimeSeconds();
    std::mutex rdbMutex_;
};
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
#endif // OHOS_CLOUD_SYNC_SERVICE_CLOUD_DISK_DATA_HANDLER_H
