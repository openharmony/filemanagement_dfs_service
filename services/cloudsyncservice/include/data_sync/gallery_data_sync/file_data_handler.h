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

#include "rdb_data_handler.h"
#include "data_convertor.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
class FileDataHandler : public RdbDataHandler {
public:
    FileDataHandler(std::shared_ptr<NativeRdb::RdbStore> rdb);
    virtual ~FileDataHandler() = default;

    /* download */
    void GetFetchCondition(int32_t &limitRes, DriveKit::DKRecordType &recordType,
        DriveKit::DKFieldKeyArray &desiredKeys) override;
    int32_t OnFetchRecords(const std::shared_ptr<std::vector<DriveKit::DKRecord>> &records) override;

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
    static inline const int32_t LIMIT_SIZE = 50;
    DriveKit::DKRecordType recordType_ = "media";
    DriveKit::DKFieldKeyArray desiredKeys_ = { "albumId", "createdTime", "editedTime", "favorite", "fileName",
        "fileType", "hashId", "sha256", "id", "properties", "size", "source", "recycledTime", "recycled",
        "pictureMetadata", "videoMetadata", "cipher", "description" };

    /* create */
    int32_t createOffset_ = 0;
    DataConvertor createConvertor_ = {
        { "file_id", "data", "size", "data" },
        { "file_id", "data", "size", "asset" },
        { INT, STRING, INT, ASSET },
        4
    };
    DataConvertor onCreateConvertor_ = {
        { "file_id", "data", "size" },
        { "file_id", "data", "size" },
        { INT, STRING, INT },
        3
    };

    /* delete */
    int32_t deleteOffset_ = 0;
    DataConvertor deleteConvertor_ = {
        { "file_id", "data", "size" },
        { "id", "path", "size" },
        { INT, STRING, INT },
        3
    };
    DataConvertor onDeleteConvertor_ = {
        { "file_id", "data", "size" },
        { "id", "path", "size" },
        { INT, STRING, INT },
        3
    };

    /* update */
    int32_t updateOffset_ = 0;
    DataConvertor updateConvertor_ = {
        { "file_id", "data", "size" },
        { "id", "path", "size" },
        { INT, STRING, INT },
        3
    };
    DataConvertor onUpdateConvertor_ = {
        { "file_id", "data", "size" },
        { "id", "path", "size" },
        { INT, STRING, INT },
        3
    };

    /* fetch */

    /* pull operations */
    int32_t PullRecordInsert(const DriveKit::DKRecord &record);
    int32_t PullRecordUpdate(const DriveKit::DKRecord &record, const DriveKit::DKRecord &local);
    int32_t PullRecordDelete(const DriveKit::DKRecord &record, const DriveKit::DKRecord &local);

    /* db result to record */
    DataConvertor localConvertor_ = {
        { "file_id", "data", "size" },
        { "id", "path", "size" },
        { INT, STRING, INT },
        3
    };
};
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
#endif // OHOS_CLOUD_SYNC_SERVICE_FILE_DATA_HANDLER_H