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

#ifndef OHOS_CLOUD_SYNC_SERVICE_DATA_HANDLER_H
#define OHOS_CLOUD_SYNC_SERVICE_DATA_HANDLER_H

#include <mutex>

#include "cloud_pref_impl.h"
#include "dk_context.h"
#include "sdk_helper.h"
#include "sysevent.h"
#include "values_bucket.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
struct OnFetchParams {
    int32_t totalPullCount{0};
    std::vector<DriveKit::DKDownloadAsset> assetsToDownload{};
    std::vector<NativeRdb::ValuesBucket> insertFiles{};
    std::map<std::string, std::set<int>> recordAlbumMaps{};
    std::map<std::string, int> recordAnalysisAlbumMaps{};
};
const static std::string DOWNLOAD_THUMB_LIMIT = "download_thumb_limit";
const static std::string BATCH_NO = "batch_no";
const static std::string RECORD_SIZE = "record_size";
const static std::string CHECKING_FLAG = "checking_flag";
class DentryContext;
class DataHandler {
public:
    enum DownloadThmType {
        SYNC_TRIGGER = 1,
        SCREENOFF_TRIGGER
    };
    DataHandler(int32_t userId, const std::string &bundleName, const std::string &table);
    /* download */
    virtual void GetFetchCondition(FetchCondition &cond) = 0;
    virtual int32_t OnFetchRecords(std::shared_ptr<std::vector<DriveKit::DKRecord>> &records,
                                   OnFetchParams &params) = 0;
    virtual int32_t GetRetryRecords(std::vector<DriveKit::DKRecordId> &records) = 0;
    virtual int32_t GetCheckRecords(std::vector<DriveKit::DKRecordId> &checkRecords,
                                    const std::shared_ptr<std::vector<DriveKit::DKRecord>> &records);
    virtual int32_t GetAssetsToDownload(std::vector<DriveKit::DKDownloadAsset> &outAssetsToDownload);
    virtual int32_t GetThumbToDownload(std::vector<DriveKit::DKDownloadAsset> &outAssetsToDownload);
    /* 'dentryContext' is only used by cloudDisk */
    virtual int32_t GetDownloadAsset(std::string cloudId,
                                     std::vector<DriveKit::DKDownloadAsset> &outAssetsToDownload,
                                     std::shared_ptr<DentryContext> dentryContext = nullptr);
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
    /* 'context' is only used by cloudDisk */
    virtual int32_t OnDownloadSuccess(const DriveKit::DKDownloadAsset &asset,
                                      std::shared_ptr<DriveKit::DKContext> context = nullptr);
    virtual int32_t OnDownloadAssets(const std::map<DriveKit::DKDownloadAsset, DriveKit::DKDownloadResult> &resultMap);
    virtual int32_t OnDownloadAssets(const DriveKit::DKDownloadAsset &asset);
    virtual int32_t OnDownloadAssetsFailure(const std::vector<DriveKit::DKDownloadAsset> &assets);

    /*clean*/
    virtual int32_t Clean(const int action);

    /* cursor */
    virtual void GetStartCursor(DriveKit::DKQueryCursor &cursor);
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
    virtual bool GetCheckFlag();
    virtual void SetChecking();
    void SetDownloadType(const int32_t type);
    int32_t GetDownloadType();
    virtual int32_t HandleDetailcode(DriveKit::DKDetailErrorCode detailCode);

protected:
    int32_t OnRecordFailed(const std::pair<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> &entry);
    void GetReturn(const int32_t error, int32_t &retCode);

    /* cursor */
    DriveKit::DKQueryCursor startCursor_ = "";
    DriveKit::DKQueryCursor nextCursor_;
    DriveKit::DKQueryCursor tempStartCursor_;
    DriveKit::DKQueryCursor tempNextCursor_;
    int32_t batchNo_{0};
    int32_t recordSize_{0};
    int32_t downloadThumbLimit_{0};
    bool isFinish_{false};
    std::map<int32_t, DriveKit::DKQueryCursor> cursorMap_;
    std::map<int32_t, bool> cursorFinishMap_;
    bool isChecking_{false};
    int32_t downloadType_{0};

    std::mutex mutex_;

    /* cloud preference impl */
    CloudPrefImpl cloudPrefImpl_;

private:
    int32_t HandleCloudSpaceNotEnough();
    int32_t HandleNotSupportSync();
    int32_t HandleNetworkErr();
};

class DentryContext : public DriveKit::DKContext {
public:
    DentryContext() {}
    DentryContext(const std::string &parentCloudId, const
    std::string &fileName) : parentCloudId_(parentCloudId),
        fileName_(fileName)
    {
    }

    void SetParentCloudId(const std::string &parentCloudId)
    {
        parentCloudId_ = parentCloudId;
    }

    void SetFileName(const std::string &fileName)
    {
        fileName_ = fileName;
    }

    std::string GetParentCloudId()
    {
        return parentCloudId_;
    }

    std::string GetFileName()
    {
        return fileName_;
    }
private:
    std::string parentCloudId_{};
    std::string fileName_{};
};
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
#endif // OHOS_CLOUD_SYNC_SERVICE_DATA_HANDLER_H
