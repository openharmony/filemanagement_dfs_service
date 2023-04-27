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
#include "task.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
using namespace std;

SdkHelper::SdkHelper(const int32_t userId, const std::string bundleName)
{
    auto driveKit = DriveKit::DriveKitNative::GetInstance(userId);
    if (driveKit == nullptr) {
        LOGE("sdk helper get drive kit instance fail");
        return;
    }

    auto container = driveKit->GetDefaultContainer(bundleName);
    if (container == nullptr) {
        LOGE("sdk helper get drive kit container fail");
        return;
    }

    database_ = container->GetPrivateDatabase();
    if (database_ == nullptr) {
        LOGE("sdk helper get drive kit database fail");
    }
}

int32_t SdkHelper::GetLock(DriveKit::DKLock &lock)
{
    auto err = database_->GetLock(lock);
    if (err.HasError()) {
        LOGE("get sdk lock err");
        return E_CLOUD_SDK;
    }
    return E_OK;
}

void SdkHelper::DeleteLock(DriveKit::DKLock &lock)
{
    database_->DeleteLock(lock);
}

int32_t SdkHelper::FetchRecords(shared_ptr<DriveKit::DKContext> context, DriveKit::DKQueryCursor cursor,
    function<void(std::shared_ptr<DriveKit::DKContext>, std::shared_ptr<const DriveKit::DKDatabase>,
        std::shared_ptr<std::vector<DriveKit::DKRecord>>, DriveKit::DKQueryCursor,
        const DriveKit::DKError &)> callback)
{
    auto ctx = static_pointer_cast<TaskContext>(context);
    auto handler = ctx->GetHandler();
    if (handler == nullptr) {
        LOGE("context get handler err");
        return E_INVAL_ARG;
    }
    FetchCondition cond;
    handler->GetFetchCondition(cond);
    auto err = database_->FetchRecords(context, cond.recordType, cond.desiredKeys, cond.limitRes, cursor, callback);
    if (err != DriveKit::DKLocalErrorCode::NO_ERROR) {
        LOGE("drivekit fetch records err %{public}d", err);
        return E_CLOUD_SDK;
    }
    return E_OK;
}

int32_t SdkHelper::FetchDatabaseChanges(std::shared_ptr<DriveKit::DKContext> context, DriveKit::DKQueryCursor cursor,
    std::function<void(const std::shared_ptr<DriveKit::DKContext>, std::shared_ptr<const DriveKit::DKDatabase>,
        std::shared_ptr<std::vector<DriveKit::DKRecord>>, DriveKit::DKQueryCursor,
        bool, const DriveKit::DKError &)> callback)
{
    auto ctx = static_pointer_cast<TaskContext>(context);
    auto handler = ctx->GetHandler();
    if (handler == nullptr) {
        LOGE("context get handler err");
        return E_INVAL_ARG;
    }
    FetchCondition cond;
    handler->GetFetchCondition(cond);
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
        DriveKit::DKSavePolicy::DK_SAVE_IF_UNCHANGED, true, callback);
    if (err != DriveKit::DKLocalErrorCode::NO_ERROR) {
        LOGE("drivekit modifies records err %{public}d", err);
        return E_CLOUD_SDK;
    }
    return E_OK;
}

int32_t SdkHelper::DownloadAssets(shared_ptr<DriveKit::DKContext> context,
    vector<DriveKit::DKDownloadAsset> recordIds, int32_t id,
    function<void(shared_ptr<DriveKit::DKContext> context, shared_ptr<vector<DriveKit::DKDownloadAsset>>)> callback)
{
    thread ([=]() { callback(context, nullptr); }).detach();
    return E_OK;
}

int32_t SdkHelper::CancelDownloadAssets(int32_t id)
{
    return E_OK;
}

int32_t SdkHelper::GetStartCursor(shared_ptr<DriveKit::DKContext> context, DriveKit::DKQueryCursor &cursor)
{
    auto ctx = static_pointer_cast<TaskContext>(context);
    auto handler = ctx->GetHandler();
    if (handler == nullptr) {
        LOGE("context get handler err");
        return E_INVAL_ARG;
    }
    FetchCondition cond;
    handler->GetFetchCondition(cond);
    auto err = database_->GetStartCursor(cond.recordType, cursor);
    if (err.HasError()) {
        LOGE("drivekit get start cursor server err %{public}d and dk errcor %{public}d", err.serverErrorCode,
            err.dkErrorCode);
        return E_CLOUD_SDK;
    }
    return E_OK;
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
