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

#include "data_syncer.h"

#include <functional>

#include "dfs_error.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
using namespace std;
using namespace placeholders;
using namespace DriveKit;

DataSyncer::DataSyncer(const std::string bundleName, const int32_t userId)
    : bundleName_(bundleName), userId_(userId), cloudPrefImpl_(userId, bundleName)
{
}

DataSyncer::~DataSyncer()
{
}

int32_t DataSyncer::AsyncRun(std::shared_ptr<TaskContext> context,
    void(DataSyncer::*f)(std::shared_ptr<TaskContext>))
{
    return E_OK;
}

template<typename T, typename RET, typename... ARGS>
function<RET(ARGS...)> DataSyncer::AsyncCallback(RET(T::*f)(ARGS...))
{
    return nullptr;
}

int32_t DataSyncer::StartSync(bool forceFlag, SyncTriggerType triggerType)
{
    return E_OK;
}

int32_t DataSyncer::StopSync(SyncTriggerType triggerType)
{
    return E_OK;
}

int32_t DataSyncer::StartDownloadFile(const std::string path, const int32_t userId)
{
    return E_OK;
}

int32_t DataSyncer::StopDownloadFile(const std::string path, const int32_t userId)
{
    return E_OK;
}

int32_t DataSyncer::RegisterDownloadFileCallback(const int32_t userId,
                                                 const sptr<ICloudDownloadCallback> downloadCallback)
{
    return E_OK;
}

int32_t DataSyncer::UnregisterDownloadFileCallback(const int32_t userId)
{
    return E_OK;
}

int32_t DataSyncer::Clean(const int action)
{
    return E_OK;
}

void DataSyncer::Abort()
{
}

void DataSyncer::SetSdkHelper(shared_ptr<SdkHelper> &sdkHelper)
{
}

int32_t DataSyncer::Pull(shared_ptr<DataHandler> handler)
{
    return E_OK;
}

void DataSyncer::PullRecords(shared_ptr<TaskContext> context)
{
}

void DataSyncer::PullDatabaseChanges(shared_ptr<TaskContext> context)
{
}

void DataSyncer::DownloadAssets(std::shared_ptr<TaskContext> context)
{
}

int32_t DataSyncer::CleanInner(std::shared_ptr<DataHandler> handler, const int action)
{
    return E_OK;
}

void DataSyncer::ClearCursor()
{
}

void EmptyDownLoadAssetsprogress(std::shared_ptr<DKContext>, DKDownloadAsset, TotalSize, DownloadSize)
{
    return;
}

int DataSyncer::HandleOnFetchRecords(const std::shared_ptr<DKContext> context,
    std::shared_ptr<const DKDatabase> database, std::shared_ptr<std::vector<DKRecord>> records)
{
    return E_OK;
}

void DataSyncer::OnFetchRecords(const std::shared_ptr<DKContext> context, std::shared_ptr<const DKDatabase> database,
    std::shared_ptr<std::vector<DKRecord>> records, DKQueryCursor nextCursor, const DKError &err)
{
}

int32_t DataSyncer::DownloadInner(std::shared_ptr<DataHandler> handler,
                                  const std::string path,
                                  const int32_t userId)
{
    return E_OK;
}

void DataSyncer::OnFetchDatabaseChanges(const std::shared_ptr<DKContext> context,
    std::shared_ptr<const DKDatabase> database,
    std::shared_ptr<std::vector<DriveKit::DKRecord>> records, DKQueryCursor nextCursor,
    bool hasMore, const DKError &err)
{
}

void DataSyncer::PullRetryRecords(shared_ptr<TaskContext> context)
{
}

void DataSyncer::OnFetchRetryRecord(shared_ptr<DKContext> context, shared_ptr<DKDatabase> database,
    DKRecordId recordId, const DKRecord &record, const DKError &error)
{
}

int32_t DataSyncer::Push(shared_ptr<DataHandler> handler)
{
    return E_OK;
}

void DataSyncer::CreateRecords(shared_ptr<TaskContext> context)
{
}

void DataSyncer::DeleteRecords(shared_ptr<TaskContext> context)
{
}

void DataSyncer::ModifyMdirtyRecords(shared_ptr<TaskContext> context)
{
}

void DataSyncer::ModifyFdirtyRecords(shared_ptr<TaskContext> context)
{
}

void DataSyncer::OnCreateRecords(shared_ptr<DKContext> context,
    shared_ptr<const DKDatabase> database,
    shared_ptr<const map<DKRecordId, DKRecordOperResult>> map, const DKError &err)
{
}

void DataSyncer::OnDeleteRecords(shared_ptr<DKContext> context,
    shared_ptr<const DKDatabase> database,
    shared_ptr<const map<DKRecordId, DKRecordOperResult>> map, const DKError &err)
{
}

void DataSyncer::OnModifyMdirtyRecords(shared_ptr<DKContext> context,
    shared_ptr<const DKDatabase> database,
    shared_ptr<const map<DKRecordId, DKRecordOperResult>> saveMap,
    shared_ptr<const map<DKRecordId, DKRecordOperResult>> deleteMap,
    const DKError &err)
{
}

void DataSyncer::OnModifyFdirtyRecords(shared_ptr<DKContext> context,
    shared_ptr<const DKDatabase> database,
    shared_ptr<const map<DKRecordId, DKRecordOperResult>> saveMap,
    shared_ptr<const map<DKRecordId, DKRecordOperResult>> deleteMap,
    const DKError &err)
{
}

void DataSyncer::BeginTransaction()
{
}

void DataSyncer::EndTransaction()
{
}

std::string DataSyncer::GetBundleName() const
{
    return bundleName_;
}

int32_t DataSyncer::GetUserId() const
{
    return userId_;
}

SyncState DataSyncer::GetSyncState() const
{
    if (bundleName_ == "com.ohos.photos") {
        return SyncState::SYNC_SUCCEED;
    }

    return SyncState::SYNC_FAILED;
}

void DataSyncer::CompletePull()
{
}

void DataSyncer::CompletePush()
{
}

void DataSyncer::CompleteAll()
{
}

void DataSyncer::SyncStateChangedNotify(const CloudSyncState state, const ErrorType error)
{
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
