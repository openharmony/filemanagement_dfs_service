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

#ifndef OHOS_CLOUD_SYNC_SERVICE_SDK_HELPER_H
#define OHOS_CLOUD_SYNC_SERVICE_SDK_HELPER_H

#include <functional>
#include <vector>

#include "dk_database.h"
#include "drive_kit.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
struct FetchCondition {
    int32_t limitRes;
    DriveKit::DKRecordType recordType;
    DriveKit::DKFieldKeyArray desiredKeys;
    DriveKit::DKFieldKeyArray fullKeys;
};
class SdkHelper {
public:
    SdkHelper() = default;
    ~SdkHelper() = default;

    /* init */
    int32_t Init(const int32_t userId, const std::string &bundleName);

    /* lock */
    int32_t GetLock(DriveKit::DKLock &lock);
    void DeleteLock(DriveKit::DKLock &lock);

    /* record download */
    using FetchRecordsCallback = std::function<void(const std::shared_ptr<DriveKit::DKContext>,
        std::shared_ptr<const DriveKit::DKDatabase>,
        std::shared_ptr<std::vector<DriveKit::DKRecord>>, DriveKit::DKQueryCursor,
        const DriveKit::DKError &)>;

    using FetchDatabaseChangesCallback = std::function<void(const std::shared_ptr<DriveKit::DKContext>,
        std::shared_ptr<const DriveKit::DKDatabase>,
        std::shared_ptr<std::vector<DriveKit::DKRecord>>, DriveKit::DKQueryCursor,
        bool, const DriveKit::DKError &)>;
    using FetchRecordCallback = std::function<void(std::shared_ptr<DriveKit::DKContext>,
        std::shared_ptr<DriveKit::DKDatabase>, DriveKit::DKRecordId, DriveKit::DKRecord &,
        const DriveKit::DKError &)>;
    using FetchRecordIdsCallback = std::function<void(std::shared_ptr<DriveKit::DKContext>,
        std::shared_ptr<DriveKit::DKDatabase>, std::shared_ptr<std::map<DriveKit::DKRecordId,
        DriveKit::DKRecordOperResult>>, const DriveKit::DKError &)>;

    int32_t FetchRecords(std::shared_ptr<DriveKit::DKContext> context, FetchCondition &cond, DriveKit::DKQueryCursor,
        FetchRecordsCallback callback);
    int32_t FetchRecordWithId(std::shared_ptr<DriveKit::DKContext> context, FetchCondition &cond,
        DriveKit::DKRecordId recordId, FetchRecordCallback callback);
    int32_t FetchRecordWithIds(std::shared_ptr<DriveKit::DKContext> context, FetchCondition &cond,
        std::vector<DriveKit::DKRecord> &&records, FetchRecordIdsCallback callback);
    int32_t FetchDatabaseChanges(std::shared_ptr<DriveKit::DKContext> context, FetchCondition &cond,
        DriveKit::DKQueryCursor cursor, FetchDatabaseChangesCallback callback);

    /* record upload */
    int32_t CreateRecords(std::shared_ptr<DriveKit::DKContext> context,
        std::vector<DriveKit::DKRecord> &records,
        std::function<void(std::shared_ptr<DriveKit::DKContext>,
            std::shared_ptr<const DriveKit::DKDatabase>,
            std::shared_ptr<const std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult>>,
            const DriveKit::DKError &)> callback
    );

    int32_t DeleteRecords(std::shared_ptr<DriveKit::DKContext> context,
        std::vector<DriveKit::DKRecord> &records,
        std::function<void(std::shared_ptr<DriveKit::DKContext>,
            std::shared_ptr<const DriveKit::DKDatabase>,
            std::shared_ptr<const std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult>>,
            const DriveKit::DKError &)> callback
    );

    int32_t ModifyRecords(std::shared_ptr<DriveKit::DKContext> context,
        std::vector<DriveKit::DKRecord> &records, DriveKit::DKDatabase::ModifyRecordsCallback callback
    );

    /* asset download */
    int32_t DownloadAssets(std::shared_ptr<DriveKit::DKContext> context,
        std::vector<DriveKit::DKDownloadAsset> &assetsToDownload, DriveKit::DKAssetPath downLoadPath,
        DriveKit::DKDownloadId &id,
        std::function<void(std::shared_ptr<DriveKit::DKContext>,
                       std::shared_ptr<const DriveKit::DKDatabase>,
                       const std::map<DriveKit::DKDownloadAsset, DriveKit::DKDownloadResult> &,
                       const DriveKit::DKError &)> resultCallback,
        std::function<void(std::shared_ptr<DriveKit::DKContext>, DriveKit::DKDownloadAsset, DriveKit::TotalSize,
                           DriveKit::DownloadSize)> progressCallback);
    int32_t DownloadAssets(DriveKit::DKDownloadAsset &assetsToDownload);
    int32_t DownloadAssets(const std::vector<DriveKit::DKDownloadAsset> &assetsToDownload,
                           std::vector<bool> &assetResultMap);

    int32_t CancelDownloadAssets(int32_t id);

    int32_t GetStartCursor(DriveKit::DKRecordType recordType, DriveKit::DKQueryCursor &cursor);

    std::shared_ptr<DriveKit::DKAssetReadSession> GetAssetReadSession(DriveKit::DKRecordType recordType,
                                                                      DriveKit::DKRecordId recordId,
                                                                      DriveKit::DKFieldKey assetKey,
                                                                      DriveKit::DKAssetPath assetPath);
    using SaveSubscriptionCallback = std::function<void(std::shared_ptr<DriveKit::DKContext>,
        std::shared_ptr<DriveKit::DKContainer>, DriveKit::DKSubscriptionResult &)>;
    using DelSubscriptionCallback = std::function<void(std::shared_ptr<DriveKit::DKContext>,
        const DriveKit::DKError &)>;
    using ChangesNotifyCallback = std::function<void(std::shared_ptr<DriveKit::DKContext>,
        const DriveKit::DKError &)>;
    int32_t SaveSubscription(SaveSubscriptionCallback callback);
    int32_t DeleteSubscription(DelSubscriptionCallback callback);
    int32_t ChangesNotify(ChangesNotifyCallback callback);
    void Release();
private:
    std::shared_ptr<DriveKit::DKContainer> container_;
    std::shared_ptr<DriveKit::DKDatabase> database_;
    std::shared_ptr<DriveKit::DKAssetsDownloader> downloader_;
};

struct SdkLock {
    DriveKit::DKLock lock = { 0 };
    std::mutex mtx;
    int count = 0;
    uint32_t timerId{0};
};
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
#endif // OHOS_CLOUD_SYNC_SERVICE_SDK_HELPER_H
