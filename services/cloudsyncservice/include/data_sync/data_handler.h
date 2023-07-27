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

#ifndef OHOS_CLOUD_SYNC_SERVICE_DATA_HANDLER_H
#define OHOS_CLOUD_SYNC_SERVICE_DATA_HANDLER_H

#include "cloud_pref_impl.h"
#include "sdk_helper.h"
#include "values_bucket.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
struct OnFetchParams {
    int32_t totalPullCount{0};
    std::vector<DriveKit::DKDownloadAsset> assetsToDownload{};
    std::vector<NativeRdb::ValuesBucket> insertFiles{};
    std::map<std::string, std::set<int>> recordAlbumMaps{};
};
const static std::string DOWNLOAD_THUMB_LIMIT = "download_thumb_limit";
const static std::string BATCH_NO = "batch_no";
const static std::string RECORD_SIZE = "record_size";
class DataHandler {
public:
    DataHandler(int32_t userId, const std::string &bundleName, const std::string &table);
    /* download */
    virtual void GetFetchCondition(FetchCondition &cond) = 0;
    virtual int32_t OnFetchRecords(std::shared_ptr<std::vector<DriveKit::DKRecord>> &records,
                                   OnFetchParams &params) = 0;
    virtual int32_t GetRetryRecords(std::vector<DriveKit::DKRecordId> &records) = 0;
    virtual int32_t GetAssetsToDownload(std::vector<DriveKit::DKDownloadAsset> &outAssetsToDownload);

    virtual int32_t GetDownloadAsset(std::string cloudId,
                                     std::vector<DriveKit::DKDownloadAsset> &outAssetsToDownload);
    /* upload */
    virtual int32_t GetCreatedRecords(std::vector<DriveKit::DKRecord> &records) = 0;
    virtual int32_t GetDeletedRecords(std::vector<DriveKit::DKRecord> &records) = 0;
    virtual int32_t GetMetaModifiedRecords(std::vector<DriveKit::DKRecord> &records) = 0;
    virtual int32_t GetFileModifiedRecords(std::vector<DriveKit::DKRecord> &records);

    /* upload callback */
    virtual int32_t OnCreateRecords(const std::map<DriveKit::DKRecordId,
        DriveKit::DKRecordOperResult> &map) = 0;
    virtual int32_t OnDeleteRecords(const std::map<DriveKit::DKRecordId,
        DriveKit::DKRecordOperResult> &map) = 0;
    virtual int32_t OnModifyMdirtyRecords(const std::map<DriveKit::DKRecordId,
        DriveKit::DKRecordOperResult> &map) = 0;
    virtual int32_t OnModifyFdirtyRecords(const std::map<DriveKit::DKRecordId,
        DriveKit::DKRecordOperResult> &map);
    virtual int32_t OnDownloadSuccess(const DriveKit::DKDownloadAsset &asset);
    virtual int32_t OnDownloadThumb(const std::map<DriveKit::DKDownloadAsset, DriveKit::DKDownloadResult> &resultMap);

    /*clean*/
    virtual int32_t Clean(const int action);

    /* cursor */
    virtual void GetNextCursor(DriveKit::DKQueryCursor &cursor);
    virtual void SetTempStartCursor(const DriveKit::DKQueryCursor &cursor);
    virtual void GetTempStartCursor(DriveKit::DKQueryCursor &cursor);
    virtual void SetTempNextCursor(const DriveKit::DKQueryCursor &cursor, bool isFinish);
    virtual int32_t GetBatchNo();
    virtual bool IsPullRecords();
    virtual void ClearCursor();
    virtual void FinishPull(const int32_t barchNo);
    virtual void SetRecordSize(const int32_t recordSize);
    virtual int32_t GetRecordSize();

protected:
    /* cursor */
    DriveKit::DKQueryCursor startCursor_;
    DriveKit::DKQueryCursor nextCursor_;
    DriveKit::DKQueryCursor tempStartCursor_;
    DriveKit::DKQueryCursor tempNextCursor_;
    int32_t batchNo_{0};
    int32_t recordSize_{0};
    int32_t downloadThumbLimit_{0};
    bool isFinish_{false};
    std::map<int32_t, DriveKit::DKQueryCursor> cursorMap_;
    std::map<int32_t, bool> cursorFinishMap_;

    /* cloud preference impl */
    CloudPrefImpl cloudPrefImpl_;
};
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
#endif // OHOS_CLOUD_SYNC_SERVICE_DATA_HANDLER_H
