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

#ifndef OHOS_CLOUD_SYNC_SERVICE_FILE_DATA_HANDLER_H
#define OHOS_CLOUD_SYNC_SERVICE_FILE_DATA_HANDLER_H

#include "medialibrary_db_const.h"
#include "medialibrary_type_const.h"

#include "rdb_data_handler.h"
#include "file_data_convertor.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
class FileDataHandler : public RdbDataHandler {
public:
    FileDataHandler(int32_t userId, const std::string &bundleName, std::shared_ptr<NativeRdb::RdbStore> rdb);
    virtual ~FileDataHandler() = default;

    /* download */
    void GetFetchCondition(FetchCondition &cond) override;
    virtual int32_t OnFetchRecords(
        const std::shared_ptr<std::vector<DriveKit::DKRecord>> &records,
        std::vector<DriveKit::DKDownloadAsset> &outAssetsToDownload,
        std::shared_ptr<std::function<void(std::shared_ptr<DriveKit::DKContext>,
                           std::shared_ptr<const DriveKit::DKDatabase>,
                           const std::map<DriveKit::DKDownloadAsset, DriveKit::DKDownloadResult> &,
                           const DriveKit::DKError &)>> &downloadResultCallback) override;

    /* upload */
    int32_t GetCreatedRecords(std::vector<DriveKit::DKRecord> &records) override;
    int32_t GetDeletedRecords(std::vector<DriveKit::DKRecord> &records) override;
    int32_t GetMetaModifiedRecords(std::vector<DriveKit::DKRecord> &records) override;
    int32_t GetFileModifiedRecords(std::vector<DriveKit::DKRecord> &records) override;

    /* callback */
    int32_t OnCreateRecords(const std::map<DriveKit::DKRecordId,
        DriveKit::DKRecordOperResult> &map) override;
    int32_t OnDeleteRecords(const std::map<DriveKit::DKRecordId,
        DriveKit::DKRecordOperResult> &map) override;
    int32_t OnModifyMdirtyRecords(const std::map<DriveKit::DKRecordId,
        DriveKit::DKRecordOperResult> &map) override;
    int32_t OnModifyFdirtyRecords(const std::map<DriveKit::DKRecordId,
        DriveKit::DKRecordOperResult> &map) override;

    /* reset */
    void Reset();

private:
    static inline const std::string TABLE_NAME = "Files";
    static inline const int32_t LIMIT_SIZE = 5;
    DriveKit::DKRecordType recordType_ = "media";
    DriveKit::DKFieldKeyArray desiredKeys_;

    /* identifier */
    int32_t userId_;
    std::string bundleName_;

    /* create */
    int32_t createOffset_ = 0;
    FileDataConvertor createConvertor_ = { userId_, bundleName_, FileDataConvertor::FILE_CREATE };

    /* delete */
    int32_t deleteOffset_ = 0;
    FileDataConvertor deleteConvertor_ = { userId_, bundleName_, FileDataConvertor::FILE_DELETE };

    /* update */
    int32_t mdirtyOffset_ = 0;
    int32_t fdirtyOffset_ = 0;
    FileDataConvertor mdirtyConvertor_ = { userId_, bundleName_, FileDataConvertor::FILE_METADATA_MODIFY };
    FileDataConvertor fdirtyConvertor_ = { userId_, bundleName_, FileDataConvertor::FILE_DATA_MODIFY };

    /* pull operations */
    int32_t PullRecordInsert(const DriveKit::DKRecord &record, bool &outPullThumbs);
    int32_t PullRecordUpdate(const DriveKit::DKRecord &record, NativeRdb::ResultSet &local,
                             bool &outPullThumbs);
    int32_t PullRecordDelete(const DriveKit::DKRecord &record, NativeRdb::ResultSet &local);
    int32_t SetRetry(const std::string &recordId);
    int32_t RecycleFile(const std::string &recordId);

    void AppendToDownload(const DriveKit::DKRecord &record,
                          const std::string &fieldKey,
                          std::vector<DriveKit::DKDownloadAsset> &assetsToDownload);

    /* db result to record */
    FileDataConvertor localConvertor_ = { userId_, bundleName_, FileDataConvertor::FILE_DOWNLOAD };
};
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
#endif // OHOS_CLOUD_SYNC_SERVICE_FILE_DATA_HANDLER_H
