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

#include "album_data_convertor.h"
#include "gallery_file_const.h"
#include "gallery_sysevent.h"
#include "rdb_data_handler.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
class AlbumDataHandler : public RdbDataHandler, public GallerySyncStatContainer {
public:
    AlbumDataHandler(int32_t userId, const std::string &bundleName,
                     std::shared_ptr<NativeRdb::RdbStore> rdb, std::shared_ptr<bool> stopFlag);
    virtual ~AlbumDataHandler() = default;

    /* download */
    virtual void GetFetchCondition(FetchCondition &cond) override;
    virtual int32_t OnFetchRecords(std::shared_ptr<std::vector<DriveKit::DKRecord>> &records,
                                   OnFetchParams &params) override;
    virtual int32_t GetRetryRecords(std::vector<DriveKit::DKRecordId> &records) override;

    /* upload */
    virtual int32_t GetCreatedRecords(std::vector<DriveKit::DKRecord> &records) override;
    virtual int32_t GetDeletedRecords(std::vector<DriveKit::DKRecord> &records) override;
    virtual int32_t GetMetaModifiedRecords(std::vector<DriveKit::DKRecord> &records) override;

    virtual int32_t OnCreateRecords(const std::map<DriveKit::DKRecordId,
        DriveKit::DKRecordOperResult> &map) override;
    virtual int32_t OnDeleteRecords(const std::map<DriveKit::DKRecordId,
        DriveKit::DKRecordOperResult> &map) override;
    virtual int32_t OnModifyMdirtyRecords(const std::map<DriveKit::DKRecordId,
        DriveKit::DKRecordOperResult> &map) override;

    virtual int32_t Clean(const int action) override;

    virtual void SetChecking() override;
    /* reset */
    void Reset();

private:
    std::tuple<std::shared_ptr<NativeRdb::ResultSet>, int> QueryLocalMatch(const std::string &recordId);
    int32_t InsertCloudAlbum(DriveKit::DKRecord &record);
    int32_t DeleteCloudAlbum(DriveKit::DKRecord &record);
    int32_t UpdateCloudAlbum(DriveKit::DKRecord &record);
    int32_t HandleLocalDirty(int32_t dirty, const DriveKit::DKRecord &record);

    int32_t OnUploadSuccess(const std::pair<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> &entry);
    int32_t OnDeleteSuccess(const std::pair<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> &entry);
    int32_t OnCreateSuccess(const std::pair<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> &entry);
    int32_t OnCreateFail(const std::pair<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> &entry);
    int32_t OnDeleteFail(const std::pair<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> &entry);
    int32_t OnModifyFail(const std::pair<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> &entry);

    bool IsConflict(DriveKit::DKRecord &record, int32_t &albumId);
    int32_t MergeAlbumOnConflict(DriveKit::DKRecord &record, int32_t albumId);
    int32_t QueryUserAlbum(std::vector<DriveKit::DKRecord> &records, Media::DirtyType dirty,
                           const std::vector<std::string> &failSet, AlbumDataConvertor &convertor);
    int32_t QuerySourceAlbum(std::vector<DriveKit::DKRecord> &records,  Media::DirtyType dirty,
                             const std::vector<std::string> &failSet, AlbumDataConvertor &convertor);
    int32_t QueryConflict(DriveKit::DKRecord &record, std::shared_ptr<NativeRdb::ResultSet> &resultSet);
    int32_t ModRecordIfFromDual(DriveKit::DKRecordData &data);

    void UpdateDownloadAlbumStat(uint64_t success, uint64_t rdbFail, uint64_t dataFail);

    static inline const std::string TABLE_NAME = "albums";
    static inline const int32_t LIMIT_SIZE = 5;
    DriveKit::DKRecordType recordType_ = "album";
    DriveKit::DKFieldKeyArray desiredKeys_;

    /* failure records */
    std::vector<std::string> modifyFailSet_;
    std::vector<std::string> createFailSet_;

    /* convert */
    AlbumDataConvertor createConvertor_ = { FILE_CREATE };
    AlbumDataConvertor deleteConvertor_ = { FILE_DELETE };
    AlbumDataConvertor modifyConvertor_ = { FILE_METADATA_MODIFY };
    std::unordered_map<std::string, std::string> localPathtoBundle_;
};

} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
#endif // OHOS_CLOUD_SYNC_SERVICE_ALBUM_DATA_HANDLER_H
