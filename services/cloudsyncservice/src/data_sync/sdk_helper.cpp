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

#include "sdk_helper.h"

#include <thread>

#include "dfs_error.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
using namespace std;

static const uint64_t INTERVAL = 30 * 24;
int32_t SdkHelper::Init(const int32_t userId, const std::string &bundleName)
{
    auto driveKit = DriveKit::DriveKitNative::GetInstance(userId, DriveKit::DKCloudSyncDemon::DK_CLOUD_FILE);
    if (driveKit == nullptr) {
        LOGE("sdk helper get drive kit instance fail");
        return E_CLOUD_SDK;
    }

    container_ = driveKit->GetDefaultContainer(bundleName);
    if (container_ == nullptr) {
        LOGE("sdk helper get drive kit container fail");
        return E_CLOUD_SDK;
    }

    database_ = container_->GetPrivateDatabase();
    if (database_ == nullptr) {
        LOGE("sdk helper get drive kit database fail");
        return E_CLOUD_SDK;
    }
    if (!downloader_) {
        downloader_ = database_->GetAssetsDownloader();
        if (downloader_ == nullptr) {
            LOGE("sdk helper get drivekit downloader_ fail");
            return E_CLOUD_SDK;
        }
    }
    return E_OK;
}

int32_t SdkHelper::GetLock(DriveKit::DKLock &lock)
{
    auto err = database_->GetLock(lock);
    if (err.HasError()) {
        LOGE("get sdk lock err, server err %{public}d and dk errcor %{public}d", err.serverErrorCode, err.dkErrorCode);
        if (static_cast<DriveKit::DKServerErrorCode>(err.serverErrorCode) ==
            DriveKit::DKServerErrorCode::NETWORK_ERROR) {
            return E_SYNC_FAILED_NETWORK_NOT_AVAILABLE;
        }
        return E_CLOUD_SDK;
    }
    return E_OK;
}

void SdkHelper::DeleteLock(DriveKit::DKLock &lock)
{
    database_->DeleteLock(lock);
}

int32_t SdkHelper::FetchRecords(std::shared_ptr<DriveKit::DKContext> context, FetchCondition &cond,
    DriveKit::DKQueryCursor cursor, FetchRecordsCallback callback)
{
    auto err = database_->FetchRecords(context, cond.recordType, cond.desiredKeys, cond.limitRes, cursor, callback);
    if (err != DriveKit::DKLocalErrorCode::NO_ERROR) {
        LOGE("drivekit fetch records err %{public}d", err);
        return E_CLOUD_SDK;
    }
    return E_OK;
}

int32_t SdkHelper::FetchRecordWithId(std::shared_ptr<DriveKit::DKContext> context, FetchCondition &cond,
    DriveKit::DKRecordId recordId, FetchRecordCallback callback)
{
    auto err = database_->FetchRecordWithId(context, cond.recordType, recordId, cond.fullKeys, callback);
    if (err != DriveKit::DKLocalErrorCode::NO_ERROR) {
        LOGE("drivekit fetch records err %{public}d", err);
        return E_CLOUD_SDK;
    }
    return E_OK;
}

int32_t SdkHelper::FetchRecordWithIds(std::shared_ptr<DriveKit::DKContext> context, FetchCondition &cond,
    std::vector<DriveKit::DKRecord> &&records, FetchRecordIdsCallback callback)
{
    auto err = database_->FetchRecordWithIds(context, std::move(records), cond.fullKeys, callback);
    if (err != DriveKit::DKLocalErrorCode::NO_ERROR) {
        LOGE("drivekit fetch patch records err %{public}d", err);
        return E_CLOUD_SDK;
    }
    return E_OK;
}

int32_t SdkHelper::FetchDatabaseChanges(std::shared_ptr<DriveKit::DKContext> context, FetchCondition &cond,
    DriveKit::DKQueryCursor cursor, FetchDatabaseChangesCallback callback)
{
    auto err = database_->FetchDatabaseChanges(context, cond.recordType, cond.desiredKeys, cond.limitRes, cursor,
        callback);
    if (err != DriveKit::DKLocalErrorCode::NO_ERROR) {
        LOGE("drivekit fetch records err %{public}d", err);
        return E_CLOUD_SDK;
    }
    return E_OK;
}

int32_t SdkHelper::CreateRecords(shared_ptr<DriveKit::DKContext> context,
    vector<DriveKit::DKRecord> &records,
    std::function<void(std::shared_ptr<DriveKit::DKContext>, std::shared_ptr<const DriveKit::DKDatabase>,
        std::shared_ptr<const std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult>>,
        const DriveKit::DKError &)> callback)
{
    auto errCode = database_->SaveRecords(context, move(records),
        DriveKit::DKSavePolicy::DK_SAVE_IF_UNCHANGED, callback);
    if (errCode != DriveKit::DKLocalErrorCode::NO_ERROR) {
        LOGE("drivekit save records err %{public}d", errCode);
        return E_CLOUD_SDK;
    }
    return E_OK;
}

int32_t SdkHelper::DeleteRecords(shared_ptr<DriveKit::DKContext> context,
    vector<DriveKit::DKRecord> &records,
    std::function<void(std::shared_ptr<DriveKit::DKContext>, std::shared_ptr<const DriveKit::DKDatabase>,
        std::shared_ptr<const std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult>>,
        const DriveKit::DKError &)> callback)
{
    auto err = database_->DeleteRecords(context, move(records),
        DriveKit::DKSavePolicy::DK_SAVE_IF_UNCHANGED, callback);
    if (err != DriveKit::DKLocalErrorCode::NO_ERROR) {
        LOGE("drivekit deletes records err %{public}d", err);
        return E_CLOUD_SDK;
    }
    return E_OK;
}

int32_t SdkHelper::ModifyRecords(shared_ptr<DriveKit::DKContext> context,
    vector<DriveKit::DKRecord> &records, DriveKit::DKDatabase::ModifyRecordsCallback callback)
{
    vector<DriveKit::DKRecord> null;
    auto err = database_->ModifyRecords(context, move(records), move(null),
        DriveKit::DKSavePolicy::DK_SAVE_IF_UNCHANGED, false, callback);
    if (err != DriveKit::DKLocalErrorCode::NO_ERROR) {
        LOGE("drivekit modifies records err %{public}d", err);
        return E_CLOUD_SDK;
    }
    return E_OK;
}

int32_t SdkHelper::DownloadAssets(shared_ptr<DriveKit::DKContext> context,
    std::vector<DriveKit::DKDownloadAsset> &assetsToDownload, DriveKit::DKAssetPath downLoadPath,
    DriveKit::DKDownloadId &id,
    std::function<void(std::shared_ptr<DriveKit::DKContext>,
                       std::shared_ptr<const DriveKit::DKDatabase>,
                       const std::map<DriveKit::DKDownloadAsset, DriveKit::DKDownloadResult> &,
                       const DriveKit::DKError &)> resultCallback,
    std::function<void(std::shared_ptr<DriveKit::DKContext>, DriveKit::DKDownloadAsset,
                       DriveKit::TotalSize, DriveKit::DownloadSize)> progressCallback)
{
    bool isPriority = false;
    if (assetsToDownload.front().fieldKey == "content") {
        isPriority = true;
    }
    auto result =
        downloader_->DownLoadAssets(context, assetsToDownload, downLoadPath, id,
                                    resultCallback, progressCallback, isPriority);
    if (result != DriveKit::DKLocalErrorCode::NO_ERROR) {
        LOGE("DownLoadAssets fail");
        return E_CLOUD_SDK;
    }
    return E_OK;
}

int32_t SdkHelper::DownloadAssets(DriveKit::DKDownloadAsset &assetsToDownload)
{
    auto result = downloader_->DownLoadAssets(assetsToDownload);
    if (result != DriveKit::DKLocalErrorCode::NO_ERROR) {
        LOGE("DownLoadAssets fail ret %{public}d", static_cast<int>(result));
        return E_CLOUD_SDK;
    }
    return E_OK;
}

int32_t SdkHelper::DownloadAssets(const std::vector<DriveKit::DKDownloadAsset> &assetsToDownload,
                                  std::vector<bool> &assetResultMap)
{
    std::vector<DriveKit::DKDownloadResult> result;
    auto ret = downloader_->DownLoadAssets(assetsToDownload, result);
    for (const auto &it : result) {
        assetResultMap.emplace_back(it.IsSuccess());
    }
    if (ret != DriveKit::DKLocalErrorCode::NO_ERROR) {
        LOGE("DownLoadAssets fail ret %{public}d", static_cast<int>(ret));
        return E_CLOUD_SDK;
    }
    return E_OK;
}

int32_t SdkHelper::CancelDownloadAssets(int32_t id)
{
    return static_cast<int32_t>(downloader_->CancelDownloadAssets(id, true));
}

int32_t SdkHelper::GetStartCursor(DriveKit::DKRecordType recordType, DriveKit::DKQueryCursor &cursor)
{
    auto err = database_->GetStartCursor(recordType, cursor);
    if (err.HasError()) {
        LOGE("drivekit get start cursor server err %{public}d and dk errcor %{public}d", err.serverErrorCode,
            err.dkErrorCode);
        return E_CLOUD_SDK;
    }
    return E_OK;
}

std::shared_ptr<DriveKit::DKAssetReadSession> SdkHelper::GetAssetReadSession(DriveKit::DKRecordType recordType,
                                                                             DriveKit::DKRecordId recordId,
                                                                             DriveKit::DKFieldKey assetKey,
                                                                             DriveKit::DKAssetPath assetPath)
{
    if (!database_)
        return nullptr;
    return database_->NewAssetReadSession(recordType, recordId, assetKey, assetPath);
}

int32_t SdkHelper::SaveSubscription(SaveSubscriptionCallback callback)
{
    auto expireTime = std::chrono::duration_cast<std::chrono::milliseconds>
        ((std::chrono::system_clock::now() + std::chrono::hours(INTERVAL)).time_since_epoch()).count();
    DriveKit::DKSubscription subscription{expireTime};
    auto err = container_->SaveSubscription(nullptr, subscription, callback);
    if (err != DriveKit::DKLocalErrorCode::NO_ERROR) {
        LOGE("SaveSubscription fail ret %{public}d", err);
        return E_CLOUD_SDK;
    }
    return E_OK;
}

int32_t SdkHelper::DeleteSubscription(DelSubscriptionCallback callback)
{
    auto err = container_->DeleteSubscription(nullptr, "", callback);
    if (err != DriveKit::DKLocalErrorCode::NO_ERROR) {
        LOGE("DeleteSubscription fail ret %{public}d", err);
        return E_CLOUD_SDK;
    }
    return E_OK;
}

int32_t SdkHelper::ChangesNotify(ChangesNotifyCallback callback)
{
    auto err = container_->ChangesNotify(nullptr, "", callback);
    if (err != DriveKit::DKLocalErrorCode::NO_ERROR) {
        LOGE("ChangesNotify fail ret %{public}d", err);
        return E_CLOUD_SDK;
    }
    return E_OK;
}
void SdkHelper::Release()
{
    LOGD("stop upload asset");
    database_->Release();
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
