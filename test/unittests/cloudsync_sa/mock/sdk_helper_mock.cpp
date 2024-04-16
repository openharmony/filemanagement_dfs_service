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

#include <gtest/gtest.h>

#include "dfs_error.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
using namespace std;

constexpr int TEST_NUM = 100;

int32_t SdkHelper::Init(const int32_t userId, const std::string &bundleName)
{
    return E_OK;
}

int32_t SdkHelper::GetLock(DriveKit::DKLock &lock)
{
    GTEST_LOG_(INFO) << "GetLock Start";
    if (lock.lockInterval == TEST_NUM) {
        return 1;
    }
    return E_OK;
}

void SdkHelper::DeleteLock(DriveKit::DKLock &lock)
{
    return;
}

int32_t SdkHelper::FetchRecords(std::shared_ptr<DriveKit::DKContext> context,
                                FetchCondition &cond,
                                DriveKit::DKQueryCursor,
                                FetchRecordsCallback callback)
{
    if (context->data_ != nullptr) {
        return 1;
    }
    return E_OK;
}

int32_t SdkHelper::FetchRecordWithId(std::shared_ptr<DriveKit::DKContext> context,
                                     FetchCondition &cond,
                                     DriveKit::DKRecordId recordId,
                                     FetchRecordCallback callback)
{
    return E_OK;
}

int32_t SdkHelper::FetchRecordWithIds(std::shared_ptr<DriveKit::DKContext> context,
                                      FetchCondition &cond,
                                      std::vector<DriveKit::DKRecord> &&records,
                                      FetchRecordIdsCallback callback)
{
    return E_OK;
}

int32_t SdkHelper::FetchDatabaseChanges(std::shared_ptr<DriveKit::DKContext> context,
                                        FetchCondition &cond,
                                        DriveKit::DKQueryCursor cursor,
                                        FetchDatabaseChangesCallback callback)
{
    if (context->data_ != nullptr) {
        return 1;
    }
    return E_OK;
}

int32_t SdkHelper::CreateRecords(
    shared_ptr<DriveKit::DKContext> context,
    vector<DriveKit::DKRecord> &records,
    std::function<void(std::shared_ptr<DriveKit::DKContext>,
                       std::shared_ptr<const DriveKit::DKDatabase>,
                       std::shared_ptr<const std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult>>,
                       const DriveKit::DKError &)> callback)
{
    return E_OK;
}
int32_t SdkHelper::DeleteRecords(
    shared_ptr<DriveKit::DKContext> context,
    vector<DriveKit::DKRecord> &records,
    std::function<void(std::shared_ptr<DriveKit::DKContext>,
                       std::shared_ptr<const DriveKit::DKDatabase>,
                       std::shared_ptr<const std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult>>,
                       const DriveKit::DKError &)> callback)
{
    return E_OK;
}

int32_t SdkHelper::ModifyRecords(shared_ptr<DriveKit::DKContext> context,
                                 vector<DriveKit::DKRecord> &records,
                                 DriveKit::DKDatabase::ModifyRecordsCallback callback)
{
    return E_OK;
}

int32_t SdkHelper::DownloadAssets(
    shared_ptr<DriveKit::DKContext> context,
    std::vector<DriveKit::DKDownloadAsset> &assetsToDownload,
    DriveKit::DKAssetPath downLoadPath,
    DriveKit::DKDownloadId &id,
    std::function<void(std::shared_ptr<DriveKit::DKContext>,
                       std::shared_ptr<const DriveKit::DKDatabase>,
                       const std::map<DriveKit::DKDownloadAsset, DriveKit::DKDownloadResult> &,
                       const DriveKit::DKError &)> resultCallback,
    std::function<void(std::shared_ptr<DriveKit::DKContext>,
                       DriveKit::DKDownloadAsset,
                       DriveKit::TotalSize,
                       DriveKit::DownloadSize)> progressCallback)
{
    return E_OK;
}

int32_t SdkHelper::DownloadAssets(DriveKit::DKDownloadAsset &assetsToDownload)
{
    return E_OK;
}

int32_t SdkHelper::CancelDownloadAssets(int32_t id)
{
    return E_OK;
}

int32_t SdkHelper::GetStartCursor(DriveKit::DKRecordType recordType, DriveKit::DKQueryCursor &cursor)
{
    return E_OK;
}

std::shared_ptr<DriveKit::DKAssetReadSession> SdkHelper::GetAssetReadSession(DriveKit::DKRecordType recordType,
                                                                             DriveKit::DKRecordId recordId,
                                                                             DriveKit::DKFieldKey assetKey,
                                                                             DriveKit::DKAssetPath assetPath)
{
    return nullptr;
}

int32_t SdkHelper::SaveSubscription(SaveSubscriptionCallback callback)
{
    return E_OK;
}

int32_t SdkHelper::DeleteSubscription(DelSubscriptionCallback callback)
{
    return E_OK;
}

int32_t SdkHelper::ChangesNotify(ChangesNotifyCallback callback)
{
    return E_OK;
}

void SdkHelper::Release()
{
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS