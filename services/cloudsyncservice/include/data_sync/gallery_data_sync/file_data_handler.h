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

#include "data_sync_notifier.h"
#include "medialibrary_db_const.h"
#include "medialibrary_type_const.h"

#include "cloud_pref_impl.h"
#include "rdb_data_handler.h"
#include "file_data_convertor.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
class FileDataHandler : public RdbDataHandler {
public:
    enum Action {
        RETAIN_DATA = 0,
        CLEAR_DATA
    };

    FileDataHandler(int32_t userId, const std::string &bundleName, std::shared_ptr<NativeRdb::RdbStore> rdb);
    virtual ~FileDataHandler() = default;

    /* download */
    void GetFetchCondition(FetchCondition &cond) override;
    virtual int32_t OnFetchRecords(std::shared_ptr<std::vector<DriveKit::DKRecord>> &records,
                                   OnFetchParams &params) override;
    virtual int32_t GetRetryRecords(std::vector<DriveKit::DKRecordId> &records) override;
    virtual int32_t GetAssetsToDownload(std::vector<DriveKit::DKDownloadAsset> &outAssetsToDownload) override;

    int32_t GetDownloadAsset(std::string cloudId,
                             std::vector<DriveKit::DKDownloadAsset> &outAssetsToDownload) override;

    /*clean*/
    int32_t Clean(const int action) override;

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
    int32_t OnDownloadSuccess(const DriveKit::DKDownloadAsset &asset) override;
    int32_t OnDownloadThumb(const std::map<DriveKit::DKDownloadAsset, DriveKit::DKDownloadResult> &resultMap) override;

    /* reset */
    void Reset();

private:
    int32_t OnCreateRecordSuccess(const std::pair<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> &entry,
                               const std::map<std::string, std::pair<std::int64_t, std::int64_t>> &localMap);
    int32_t OnDeleteRecordSuccess(const std::pair<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> &entry);
    int32_t OnModifyRecordSuccess(const std::pair<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> &entry,
                               const std::map<std::string, std::pair<std::int64_t, std::int64_t>> &localMap);
    bool IfTimeChanged(const DriveKit::DKRecord &record,
                             const std::map<std::string, std::pair<std::int64_t, std::int64_t>> &localMap,
                             const std::string &path,
                             const std::string &type);
    void GetLocalTimeMap(const std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> &map,
                         std::map<std::string, std::pair<std::int64_t, std::int64_t>> &cloudMap,
                         const std::string &type);
    void OnResultSetConvertToMap(const std::shared_ptr<NativeRdb::ResultSet> resultSet,
                                 std::map<std::string, std::pair<std::int64_t, std::int64_t>> &cloudMap,
                                 const std::string &type);
    int32_t OnRecordFailed(const std::pair<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> &entry);
    int32_t HandleCloudSpaceNotEnough();
    int32_t HandleATFailed();
    int32_t HandleNameConflict();
    int32_t HandleNameInvalid();
    int64_t UTCTimeSeconds();

    int32_t EraseLocalInfo(std::vector<DriveKit::DKRecord> &records);

    /* data calculate*/
    int32_t CompensateFilePath(DriveKit::DKRecord &record);
    int32_t CalculateFilePath(DriveKit::DKRecord &record, std::string &filePath);
    int32_t GetMediaType(DriveKit::DKRecord &record, int32_t &mediaType);
    int32_t GetAssetUniqueId(int32_t &type);
    int32_t CreateAssetPathById(DriveKit::DKRecord &record, int32_t &uniqueId,
        int32_t &mediaType, std::string &filePath);
    int32_t CreateAssetBucket(int32_t &uniqueId, int32_t &bucketNum);
    std::string GetFileExtension(DriveKit::DKRecord &record);
    int32_t CreateAssetRealName(int32_t &fileId, int32_t &mediaType,
        const std::string &extension, std::string &name);

    /* album map */
    int32_t BindAlbums(std::vector<DriveKit::DKRecord> &records);
    int32_t BindAlbumChanges(std::vector<DriveKit::DKRecord> &records);
    int32_t UpdateLocalAlbumMap(const std::string &cloudId);

    int32_t GetAlbumCloudIds(std::vector<int32_t> &localIds, std::vector<std::string> &cloudIds);
    int32_t GetAlbumIdsFromResultSet(const std::shared_ptr<NativeRdb::ResultSet> resultSet, std::vector<int32_t> &ids);
    int32_t GetAlbumIdsFromResultSet(const std::shared_ptr<NativeRdb::ResultSet> resultSet,
        std::vector<int32_t> &add, std::vector<int32_t> &rm);

    static inline const std::string TABLE_NAME = Media::PhotoColumn::PHOTOS_TABLE;
    static inline const int32_t LIMIT_SIZE = 5;
    DriveKit::DKRecordType recordType_ = "media";
    DriveKit::DKFieldKeyArray desiredKeys_;
    std::vector<std::string> modifyFailSet_;
    std::vector<std::string> createFailSet_;

    /* identifier */
    int32_t userId_;
    std::string bundleName_;

    /*clean*/
    FileDataConvertor cleanConvertor_ = { userId_, bundleName_, FileDataConvertor::FILE_CLEAN };
    int32_t ClearCloudInfo(const std::string &cloudId);
    int32_t CleanCloudRecord(NativeRdb::ResultSet &local, const int action, const std::string &filePath);
    int32_t CleanNotDirtyData(const std::string &thmbDir, const std::string &assetPath, const std::string &cloudId);
    int32_t CleanNotPureCloudRecord(NativeRdb::ResultSet &local, const int action, const std::string &filePath);
    int32_t CleanPureCloudRecord(NativeRdb::ResultSet &local, const int action, const std::string &filePath);
    int32_t DeleteDentryFile(void);

    /* create */
    FileDataConvertor createConvertor_ = { userId_, bundleName_, FileDataConvertor::FILE_CREATE };

    /* delete */
    FileDataConvertor deleteConvertor_ = { userId_, bundleName_, FileDataConvertor::FILE_DELETE };

    /* update */
    FileDataConvertor mdirtyConvertor_ = { userId_, bundleName_, FileDataConvertor::FILE_METADATA_MODIFY };
    FileDataConvertor fdirtyConvertor_ = { userId_, bundleName_, FileDataConvertor::FILE_DATA_MODIFY };

    /* file Conflict */
    std::string ConflictRenameThumb(NativeRdb::ResultSet &resultSet, std::string fullPath);
    int32_t ConflictRename(NativeRdb::ResultSet &resultSet, std::string &fullPath);
    int32_t ConflictDataMerge(const DriveKit::DKRecord &record, std::string &fullPath);
    int32_t ConflictHandler(NativeRdb::ResultSet &resultSet, const DriveKit::DKRecord &record,
                            std::string &fullPath, int64_t isize, int64_t mtime, bool &comflag);
    int32_t GetConflictData(DriveKit::DKRecord &record, std::string &fullPath, int64_t &isize, int64_t &mtime);
    int32_t PullRecordConflict(DriveKit::DKRecord &record, bool &comflag);

    /* pull operations */
    std::tuple<std::shared_ptr<NativeRdb::ResultSet>, int> QueryLocalByCloudId(const std::string &recordId);
    int32_t PullRecordInsert(DriveKit::DKRecord &record, OnFetchParams &params);
    int32_t PullRecordUpdate(DriveKit::DKRecord &record, NativeRdb::ResultSet &local,
                             OnFetchParams &params);
    void RemoveThmParentPath(const std::string &filePath);
    int32_t PullRecordDelete(DriveKit::DKRecord &record, NativeRdb::ResultSet &local);
    int32_t SetRetry(const std::string &recordId);
    int32_t RecycleFile(const std::string &recordId);
    void AppendToDownload(NativeRdb::ResultSet &local,
                          const std::string &fieldKey,
                          std::vector<DriveKit::DKDownloadAsset> &assetsToDownload);
    void AppendToDownload(const DriveKit::DKRecord &record,
                          const std::string &fieldKey,
                          std::vector<DriveKit::DKDownloadAsset> &assetsToDownload);
    int AddCloudThumbs(const DriveKit::DKRecord &record);
    int DentryInsertThumb(const std::string &fullPath,
                          const std::string &recordId,
                          uint64_t size,
                          uint64_t mtime,
                          const std::string &type);
    bool ThumbsAtLocal(const DriveKit::DKRecord &record);
    int32_t UpdateAssetInPhotoMap(const DriveKit::DKRecord &record, int32_t fileId);
    int32_t InsertAssetToPhotoMap(const DriveKit::DKRecord &record, OnFetchParams &params);
    int32_t DeleteAssetInPhotoMap(int32_t fileId);
    int32_t GetAlbumIdFromCloudId(const std::string &recordId);
    void QueryAndInsertMap(int32_t albumId, int32_t fileId);
    void QueryAndDeleteMap(int32_t fileId, const std::set<int> &cloudMapIds);
    int32_t BatchInsertAssetMaps(OnFetchParams &params);

    /* db result to record */
    FileDataConvertor localConvertor_ = { userId_, bundleName_, FileDataConvertor::FILE_DOWNLOAD };
};
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
#endif // OHOS_CLOUD_SYNC_SERVICE_FILE_DATA_HANDLER_H
