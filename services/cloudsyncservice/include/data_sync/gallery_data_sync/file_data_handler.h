/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include "file_data_convertor.h"
#include "gallery_sysevent.h"
#include "rdb_data_handler.h"
namespace OHOS {
namespace FileManagement {
namespace CloudSync {

class FileDataHandler : public RdbDataHandler, public GallerySyncStatContainer,
    public GalleryCheckStatContainer {
public:
    friend class GalleryDataSyncer;
    enum Clean {
        NOT_NEED_CLEAN = 0,
        NEED_CLEAN,
    };
    FileDataHandler(int32_t userId, const std::string &bundleName,
                    std::shared_ptr<NativeRdb::RdbStore> rdb, std::shared_ptr<bool> stopFlag);
    virtual ~FileDataHandler() = default;

    /* download */
    void GetFetchCondition(FetchCondition &cond) override;
    virtual int32_t OnFetchRecords(std::shared_ptr<std::vector<DriveKit::DKRecord>> &records,
                                   OnFetchParams &params) override;
    virtual int32_t GetRetryRecords(std::vector<DriveKit::DKRecordId> &records) override;
    virtual int32_t GetCheckRecords(std::vector<DriveKit::DKRecordId> &checkRecords,
                                    const std::shared_ptr<std::vector<DriveKit::DKRecord>> &records) override;
    virtual int32_t GetAssetsToDownload(std::vector<DriveKit::DKDownloadAsset> &outAssetsToDownload) override;
    virtual int32_t GetThumbToDownload(std::vector<DriveKit::DKDownloadAsset> &outAssetsToDownload) override;
    int32_t GetDownloadAsset(std::string cloudId,
                             std::vector<DriveKit::DKDownloadAsset> &outAssetsToDownload,
                             std::shared_ptr<DentryContext> dentryContext = nullptr) override;
    int32_t HandleRecord(std::shared_ptr<std::vector<DriveKit::DKRecord>> &records, OnFetchParams &params,
        std::vector<std::string> &recordIds, const std::shared_ptr<NativeRdb::ResultSet> &resultSet,
        const std::map<std::string, int> &recordIdRowIdMap);

    /*clean*/
    int32_t Clean(const int action) override;
    int32_t MarkClean(const int32_t action);
    int32_t CleanRemainRecord();
    int32_t DeleteCloudPhotoDir();

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
    int32_t OnDownloadSuccess(const DriveKit::DKDownloadAsset &asset,
                              std::shared_ptr<DriveKit::DKContext> context = nullptr) override;
    int32_t OnDownloadAssets(const std::map<DriveKit::DKDownloadAsset, DriveKit::DKDownloadResult> &resultMap) override;
    int32_t OnDownloadAssets(const DriveKit::DKDownloadAsset &asset) override;
    void StopUpdataFiles();

    int32_t OnDownloadAssetsFailure(const std::vector<DriveKit::DKDownloadAsset> &assets) override;

    /* optimize storage */
    int32_t OptimizeStorage(const int32_t agingDays);
    std::shared_ptr<NativeRdb::ResultSet> GetAgingFile(const int64_t agingTime, int32_t &rowCount);
    int32_t UpdateAgingFile(const std::string cloudId);
    int32_t FileAgingDelete(const int64_t agingTime, const int64_t deleteSize);

    void UpdateAlbumInternal();
    void UpdateAllAlbums();

    /* reset */
    void Reset();

    /* check */
    void SetCheckReportStatus();

private:
    struct KeyData {
        std::string displayName;
        std::string filePath;
        int64_t isize;
        int64_t createTime;
    };
    int32_t OnCreateRecordSuccess(const std::pair<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> &entry,
        const std::unordered_map<std::string, LocalInfo> &localMap);
    int32_t CheckRecordData(DriveKit::DKRecordData &data, std::string &path);
    int32_t OnDeleteRecordSuccess(const std::pair<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> &entry);
    int32_t OnMdirtyRecordSuccess(const std::pair<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> &entry,
        const std::unordered_map<std::string, LocalInfo> &localMap);
    int32_t OnFdirtyRecordSuccess(const std::pair<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> &entry,
        const std::unordered_map<std::string, LocalInfo> &localMap);

    bool IsTimeChanged(const DriveKit::DKRecord &record, const std::unordered_map<std::string, LocalInfo> &localMap,
        const std::string &path, const std::string &type);
    int32_t GetLocalInfo(const std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> &map,
        std::unordered_map<std::string, LocalInfo> &infoMap, const std::string &type);
    int32_t BuildInfoMap(const std::shared_ptr<NativeRdb::ResultSet> resultSet,
        std::unordered_map<std::string, LocalInfo> &cloudMap, const std::string &type);
    int64_t UTCTimeSeconds();

    int32_t EraseLocalInfo(std::vector<DriveKit::DKRecord> &records);
    int32_t UpdateMediaFilePath(DriveKit::DKRecord &record, NativeRdb::ResultSet &local);
    int32_t UpdateRecordToDatabase(DriveKit::DKRecord &record, NativeRdb::ResultSet &local, int32_t &changeRows,
        OnFetchParams &params);

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

    /* data check */
    int32_t CheckDataConsistency(NativeRdb::ResultSet &resultSet);

    int32_t CheckAssetConsistency(NativeRdb::ResultSet &resultSet);
    int32_t CheckContentConsistency(NativeRdb::ResultSet &resultSet);
    int32_t CheckThumbConsistency(NativeRdb::ResultSet &resultSet, const std::string &suffix);

    int32_t CheckStatusConsistency(NativeRdb::ResultSet &resultSet);
    int32_t CheckDirtyConsistency(NativeRdb::ResultSet &resultSet);
    int32_t CheckPositionConsistency(NativeRdb::ResultSet &resultSet);
    int32_t CheckSyncStatusConsistency(NativeRdb::ResultSet &resultSet);
    int32_t SetSyncStatusConsistency(std::string &filePath, bool thumbLocal,
        bool lcdLocal, int32_t thumbStatus, int32_t syncStatus);

    /* report check */
    int32_t GetFilePosStat(std::vector<uint64_t> &filePosStat);
    int32_t GetCloudThmStat(std::vector<uint64_t> &cloudThmStat);
    int32_t GetDirtyTypeStat(std::vector<uint64_t> &dirtyTypeStat);
    int32_t QueryDataStat(const NativeRdb::AbsRdbPredicates &predicates, const std::string &queryData,
        const int32_t &value, int &num);
    int32_t QueryFilePosStat(const int32_t &position, int &num);
    int32_t QueryCloudThmStat(const int32_t &cloudThmStat, int &num);
    int32_t QueryDirtyTypeStat(const int32_t &dirtyType, int &num);
    void ReportFilePosStat();
    void ReportCloudThmStat();
    void ReportDirtyTypeStat();

    static inline const std::string TABLE_NAME = Media::PhotoColumn::PHOTOS_TABLE;
    static inline const int32_t LIMIT_SIZE = 5;
    static inline const int32_t MODIFY_BATCH_NUM = 20;
    static inline const int32_t DELETE_BATCH_NUM = 20;
    static inline const int32_t DOWNLOAD_LIMIT_SIZE = 200;
    static inline const int32_t UPDATE_VEC_SIZE = 20;
    DriveKit::DKRecordType recordType_ = "media";
    DriveKit::DKFieldKeyArray desiredKeys_;
    DriveKit::DKFieldKeyArray checkedKeys_ = {"version", "id"};
    std::vector<std::string> modifyFailSet_;
    std::vector<std::string> createFailSet_;
    std::vector<NativeRdb::ValueObject> retrySet_;

    /* identifier */
    int32_t userId_;
    std::string bundleName_;

    /*clean*/
    FileDataConvertor cleanConvertor_ = { userId_, bundleName_, FILE_CLEAN };
    int32_t ClearCloudInfo(const std::string &cloudId);
    int32_t CleanAllCloudInfo();
    int32_t CleanNotDirtyData(bool isReamin = false);
    int32_t CleanNotPureCloudRecord(const int32_t action);
    int32_t CleanPureCloudRecord(bool isReamin = false);
    int32_t DeleteDentryFile(void);
    int32_t GetFilePathAndId(NativeRdb::AbsRdbPredicates cleanPredicates,
                             std::vector<NativeRdb::ValueObject> &deleteFileId,
                             std::vector<std::string> &filePaths);
    void RemoveCloudRecord(std::shared_ptr<std::vector<std::string>> filePaths);
    void RemoveBothRecord(std::shared_ptr<std::vector<std::string>> filePaths);

    /* err handle */
    void HandleCreateConvertErr(int32_t err, NativeRdb::ResultSet &resultSet);
    void HandleFdirtyConvertErr(int32_t err, NativeRdb::ResultSet &resultSet);

    /* create */
    FileDataConvertor createConvertor_ = {
        userId_, bundleName_, FILE_CREATE,
        std::bind(&FileDataHandler::HandleCreateConvertErr, this, std::placeholders::_1, std::placeholders::_2)
    };

    /* delete */
    FileDataConvertor deleteConvertor_ = { userId_, bundleName_, FILE_DELETE };

    /* update */
    FileDataConvertor mdirtyConvertor_ = { userId_, bundleName_, FILE_METADATA_MODIFY };
    FileDataConvertor fdirtyConvertor_ = {
        userId_, bundleName_, FILE_DATA_MODIFY,
        std::bind(&FileDataHandler::HandleFdirtyConvertErr, this, std::placeholders::_1, std::placeholders::_2)
    };

    /* file Conflict */
    static inline const std::string CON_SUFFIX = "_1";
    std::string ConflictRenameThumb(std::string fullPath, std::string &tmpPath, std::string &newPath);
    int32_t ConflictRename(std::string &fullPath, std::string &relativePath);
    int32_t ConflictRenamePath(std::string &fullPath,
                               std::string &rdbPath,
                               std::string &localPath,
                               std::string &newLocalPath);
    int32_t ConflictDataMerge(DriveKit::DKRecord &record, std::string &fullPath, bool upflag);
    int32_t ConflictHandler(NativeRdb::ResultSet &resultSet,
                            const DriveKit::DKRecord &record,
                            int64_t isize,
                            bool &modifyPathflag);
    int32_t ConflictDifferent(NativeRdb::ResultSet &resultSet,
                              const DriveKit::DKRecord &record,
                              std::string &fullPath,
                              std::string &relativePath);
    int32_t ConflictMerge(NativeRdb::ResultSet &resultSet,
                          DriveKit::DKRecord &record,
                          std::string &fullPath,
                          bool &comflag,
                          int64_t &imetaModified);
    int32_t GetConflictData(const DriveKit::DKRecord &record,
                            std::string &fullPath,
                            int64_t &isize,
                            int64_t &imetaModified,
                            std::string &relativePath);
    int32_t PullRecordConflict(DriveKit::DKRecord &record, bool &comflag);

    /* pull operations */
    std::tuple<std::shared_ptr<NativeRdb::ResultSet>, std::map<std::string, int>> QueryLocalByCloudId(
        const std::vector<std::string> &recordIds);
    void PullRecordsInsert(std::vector<DriveKit::DKRecord> &records, OnFetchParams &params);
    int32_t GetInsertParams(DriveKit::DKRecord &record, OnFetchParams &params);
    int32_t PullRecordsConflictProc(std::vector<DriveKit::DKRecord> &records);
    std::tuple<std::shared_ptr<NativeRdb::ResultSet>, int32_t>
        BatchQueryLocal(std::vector<DriveKit::DKRecord> &records);
    int32_t GetLocalKeyData(NativeRdb::ResultSet &resultSet, KeyData &keyData);
    int32_t GetCloudKeyData(const DriveKit::DKRecord &record, KeyData &keyData);
    std::tuple<bool, bool> JudgeConflict(const KeyData &localKeyData, const KeyData &cloudKeyData);
    int32_t MergeRename(const std::string &localFilePath, const std::string &cloudFilePath);
    int32_t DoDataMerge(DriveKit::DKRecord &record,
                        std::string &localFilePath,
                        const std::string &cloudFilePath,
                        bool isSamePath);
    int32_t MergeRenamePath(const std::string &localFilePath, const std::string &cloudFilePath);
    std::string GetThumbDir(const std::string &filePath);
    int32_t MergeRenameThumb(const std::string &localFilePath, const std::string &cloudFilePath);
    int32_t PathConflictProc(const DriveKit::DKRecord &record);
    int32_t PullRecordInsert(DriveKit::DKRecord &record, OnFetchParams &params);
    int32_t PullRecordUpdate(DriveKit::DKRecord &record, NativeRdb::ResultSet &local,
                             OnFetchParams &params);
    void RemoveThmParentPath(const std::string &filePath);
    int32_t PullRecordDelete(DriveKit::DKRecord &record, NativeRdb::ResultSet &local);
    int32_t SetRetry(const std::string &recordId);
    int32_t SetRetry(std::vector<NativeRdb::ValueObject> &retryList);
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
    int DentryRemoveThumb(const std::string &downloadPath);
    bool ThumbsAtLocal(const DriveKit::DKRecord &record);
    int32_t UpdateAssetInPhotoMap(const DriveKit::DKRecord &record, int32_t fileId);
    int32_t InsertAssetToPhotoMap(const DriveKit::DKRecord &record, OnFetchParams &params);
    int32_t DeleteAssetInPhotoMap(int32_t fileId);
    int32_t GetAlbumIdFromCloudId(const std::string &recordId);
    int32_t BatchGetFileIdFromCloudId(const std::vector<NativeRdb::ValueObject> &recordIds, std::vector<int> &fileIds);
    int32_t QueryWithBatchCloudId(std::vector<int> &fileIds, std::vector<std::string> &thmStrVec);
    void QueryAndInsertMap(int32_t albumId, int32_t fileId);
    void QueryAndDeleteMap(int32_t fileId, const std::set<int> &cloudMapIds);
    int32_t BatchInsertAssetMaps(OnFetchParams &params);
    int32_t BatchInsertAssetAnalysisMaps(OnFetchParams &params);
    void UpdateThmVec();
    void UpdateLcdVec();
    void HandleShootingMode(const DriveKit::DKRecord &record, const NativeRdb::ValuesBucket &valuebucket,
        OnFetchParams &params);

    /* db result to record */
    FileDataConvertor localConvertor_ = { userId_, bundleName_, FILE_DOWNLOAD };
    std::mutex rdbMutex_;
    std::mutex thmMutex_;
    std::mutex lcdMutex_;
    std::mutex cleanMutex_;
    std::vector<NativeRdb::ValueObject> thmVec_;
    std::vector<NativeRdb::ValueObject> lcdVec_;
};
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
#endif // OHOS_CLOUD_SYNC_SERVICE_FILE_DATA_HANDLER_H
