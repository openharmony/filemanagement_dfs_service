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

#ifndef OHOS_CLOUD_SYNC_SERVICE_ALBUM_DATA_HANDLER_H
#define OHOS_CLOUD_SYNC_SERVICE_ALBUM_DATA_HANDLER_H

#include "rdb_data_handler.h"
#include "data_convertor.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
class AlbumDataHandler : public RdbDataHandler {
public:
    AlbumDataHandler(std::shared_ptr<NativeRdb::RdbStore> rdb);
    virtual ~AlbumDataHandler() = default;

    /* download */
    virtual void GetFetchCondition(int32_t &limitRes, DriveKit::DKRecordType &recordType,
        DriveKit::DKFieldKeyArray &desiredKeys) override;
    virtual int32_t OnFetchRecords(const std::shared_ptr<std::vector<DriveKit::DKRecord>> &records) override;

    /* upload */
    virtual int32_t GetCreatedRecords(std::vector<DriveKit::DKRecord> &records) override;
    virtual int32_t GetDeletedRecords(std::vector<DriveKit::DKRecord> &records) override;
    virtual int32_t GetModifiedRecords(std::vector<DriveKit::DKRecord> &records) override;

    virtual int32_t OnCreateRecords(const std::map<DriveKit::DKRecordId,
        DriveKit::DKRecordOperResult> &map) override;
    virtual int32_t OnDeleteRecords(const std::map<DriveKit::DKRecordId,
        DriveKit::DKRecordOperResult> &map) override;
    virtual int32_t OnModifyRecords(const std::map<DriveKit::DKRecordId,
        DriveKit::DKRecordOperResult> &map) override;

private:
    static inline const std::string TABLE_NAME = "albums";
    static inline const int32_t LIMIT_SIZE = 50;
    DriveKit::DKRecordType recordType_ = "album";
    DriveKit::DKFieldKeyArray desiredKeys_ = { "id", "albumName", "type", "localPath", "properties",
        "createdTime", "source" };
};
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
#endif // OHOS_CLOUD_SYNC_SERVICE_ALBUM_DATA_HANDLER_H