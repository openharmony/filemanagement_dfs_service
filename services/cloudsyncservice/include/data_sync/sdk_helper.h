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

class SdkHelper {
public:
    SdkHelper(const int32_t userId, const std::string bundleName);
    ~SdkHelper() = default;

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

    int32_t FetchRecords(std::shared_ptr<DriveKit::DKContext> context, DriveKit::DKQueryCursor,
        FetchRecordsCallback callback);

    int32_t FetchDatabaseChanges(std::shared_ptr<DriveKit::DKContext> context, DriveKit::DKQueryCursor,
        FetchDatabaseChangesCallback callback);

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

    int32_t CancelDownloadAssets(int32_t id);

    int32_t GetStartCursor(std::shared_ptr<DriveKit::DKContext> context, DriveKit::DKQueryCursor &cursor);

private:
    std::shared_ptr<DriveKit::DKDatabase> database_;
};
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
#endif // OHOS_CLOUD_SYNC_SERVICE_SDK_HELPER_H
