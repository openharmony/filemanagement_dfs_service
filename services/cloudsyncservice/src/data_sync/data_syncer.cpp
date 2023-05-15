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
#include "ipc/cloud_sync_callback_manager.h"
#include "sdk_helper.h"
#include "sync_rule/battery_status.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
using namespace std;
using namespace placeholders;
using namespace DriveKit;

DataSyncer::DataSyncer(const std::string bundleName, const int32_t userId)
    : bundleName_(bundleName), userId_(userId), cloudPrefImpl_(userId, bundleName)
{
    cloudPrefImpl_.GetString(START_CURSOR, startCursor_);
    cloudPrefImpl_.GetString(NEXT_CURSOR, nextCursor_);
    taskManager_ = make_shared<TaskManager>(bind(&DataSyncer::Schedule, this));
}

int32_t DataSyncer::AsyncRun(std::shared_ptr<TaskContext> context,
    void(DataSyncer::*f)(std::shared_ptr<TaskContext>))
{
    shared_ptr<Task> task = make_shared<Task>(context,
        [this, f](shared_ptr<TaskContext> ctx) {
            (this->*f)(ctx);
        }
    );

    int32_t ret = CommitTask(task);
    if (ret != E_OK) {
        LOGE("async run commit task err %{public}d", ret);
        return ret;
    }

    return E_OK;
}

/*
 * About ARGS...
 * <1> async execute requires value-copy or shared_ptr like input parameters,
 *     but no reference for lifecycle consideration.
 * <2> In addition, [=] requires the wrapped function with const parameters.
 */
template<typename T, typename RET, typename... ARGS>
function<RET(ARGS...)> DataSyncer::AsyncCallback(RET(T::*f)(ARGS...))
{
    shared_ptr<Task> task = make_shared<Task>(nullptr, nullptr);

    int32_t ret = AddTask(task);
    if (ret != E_OK) {
        LOGE("async callback add task err %{public}d", ret);
        return nullptr;
    }

    return [this, f, task](ARGS... args) -> RET {
        int32_t ret = StartTask(task, [this, f, args...](shared_ptr<TaskContext>) {
            (this->*f)(args...);
        });
        if (ret != E_OK) {
            LOGE("async callback start task err %{public}d", ret);
        }
    };
}

int32_t DataSyncer::StartSync(bool forceFlag, SyncTriggerType triggerType)
{
    LOGI("%{private}d %{public}s starts sync, isforceSync %{public}d, triggerType %{public}d",
        userId_, bundleName_.c_str(), forceFlag, triggerType);

    /* only one specific data sycner running at a time */
    if (syncStateManager_.CheckAndSetPending(forceFlag)) {
        LOGI("syncing, pending sync");
        return E_PENDING;
    }

    /* notify sync state */
    SyncStateChangedNotify(SyncType::ALL, SyncPromptState::SYNC_STATE_SYNCING);

    /* lock: device-reentrant */
    int32_t ret = sdkHelper_->GetLock(lock_);
    if (ret != E_OK) {
        return ret;
    }

    /* start data sync */
    Schedule();

    return E_OK;
}

int32_t DataSyncer::StopSync(SyncTriggerType triggerType)
{
    LOGI("%{private}d %{public}s stops sync, trigger stop sync, type:%{public}d",
        userId_, bundleName_.c_str(), triggerType);

    syncStateManager_.SetStopSyncFlag();
    Abort();

    return E_OK;
}

void DataSyncer::Abort()
{
    LOGI("%{private}d %{private}s aborts", userId_, bundleName_.c_str());

    /* stop all the tasks and wait for tasks' termination */
    if (!taskManager_->StopAndWaitFor()) {
        LOGE("wait for tasks stop fail");
    }

    /* call the syncer manager's callback for notification */
}

void DataSyncer::SetSdkHelper(shared_ptr<SdkHelper> sdkHelper)
{
    sdkHelper_ = sdkHelper;
}

int32_t DataSyncer::Pull(shared_ptr<DataHandler> handler)
{
    LOGI("%{private}d %{private}s pull", userId_, bundleName_.c_str());

    shared_ptr<TaskContext> context = make_shared<TaskContext>(handler);

    DataSyncer::PullRetryRecords(context);

    /* Full synchronization and incremental synchronization */
    int32_t ret = E_OK;
    if (startCursor_.empty()) {
        ret = AsyncRun(context, &DataSyncer::PullRecords);
    } else {
        ret = AsyncRun(context, &DataSyncer::PullDatabaseChanges);
    }
    if (ret != E_OK) {
        LOGE("asyn run pull records err %{public}d", ret);
        return ret;
    }

    return E_OK;
}

/*
 * Although records from the cloud should be all pulled down before
 * uploading the local change, conflicts might be rare in most cases,
 * and the syncer would just move on.
 */
void DataSyncer::PullRecords(shared_ptr<TaskContext> context)
{
    LOGI("%{private}d %{private}s pull records", userId_, bundleName_.c_str());

    /* get query condition here */
    auto callback = AsyncCallback(&DataSyncer::OnFetchRecords);
    if (callback == nullptr) {
        LOGE("wrap on fetch records fail");
        return;
    }

    std::string tempStartCursor;
    sdkHelper_->GetStartCursor(context, tempStartCursor);
    cloudPrefImpl_.SetString(TEMP_START_CURSOR, tempStartCursor);

    int32_t ret = sdkHelper_->FetchRecords(context, nextCursor_, callback);
    if (ret != E_OK) {
        LOGE("sdk fetch records err %{public}d", ret);
    }
}

void DataSyncer::PullDatabaseChanges(shared_ptr<TaskContext> context)
{
    LOGI("%{private}d %{private}s pull database changes", userId_, bundleName_.c_str());

    auto callback = AsyncCallback(&DataSyncer::OnFetchDatabaseChanges);
    if (callback == nullptr) {
        LOGE("wrap on fetch records fail");
        return;
    }

    /* If nextCursor is empty, it is the first batch of incremental synchronization. Begin with the startCursor_ */
    if (nextCursor_.empty()) {
        nextCursor_ = startCursor_;
    }
    int32_t ret = sdkHelper_->FetchDatabaseChanges(context, nextCursor_, callback);
    if (ret != E_OK) {
        LOGE("sdk fetch records err %{public}d", ret);
    }
}

class DownloadContext : public TaskContext {
public:
    DownloadContext(std::shared_ptr<DataHandler> handler,
                    vector<DKDownloadAsset> assets,
                    DriveKit::DKDownloadId &id,
                    shared_ptr<std::function<void(std::shared_ptr<DriveKit::DKContext>,
                           std::shared_ptr<const DriveKit::DKDatabase>,
                           const std::map<DriveKit::DKDownloadAsset, DriveKit::DKDownloadResult> &,
                           const DriveKit::DKError &)>> resultCallback,
                    const std::shared_ptr<DKContext> context,
                    std::shared_ptr<const DKDatabase> database,
                    shared_ptr<std::function<void(std::shared_ptr<DriveKit::DKContext>,
                                                  DriveKit::DKDownloadAsset, TotalSize,
                                                  DriveKit::DownloadSize)>> progressCallback)
        : TaskContext(handler), assets(assets), id(id), resultCallback(resultCallback),
          context(context), database(database), progressCallback(progressCallback){};
    vector<DKDownloadAsset> assets;
    DriveKit::DKDownloadId &id;
    shared_ptr<std::function<void(std::shared_ptr<DriveKit::DKContext>,
                           std::shared_ptr<const DriveKit::DKDatabase>,
                           const std::map<DriveKit::DKDownloadAsset, DriveKit::DKDownloadResult> &,
                           const DriveKit::DKError &)>> resultCallback = nullptr;
    const std::shared_ptr<DKContext> context;
    std::shared_ptr<const DKDatabase> database;
    shared_ptr<std::function<void(std::shared_ptr<DriveKit::DKContext>,
                                  DriveKit::DKDownloadAsset,
                                  TotalSize, DriveKit::DownloadSize)>> progressCallback;
};

void DataSyncer::DownloadAssets(std::shared_ptr<TaskContext> context)
{
    auto ctx = static_pointer_cast<DownloadContext>(context);
    if (ctx->resultCallback == nullptr) {
        LOGE("resultCallback nullptr");
        return;
    }
    if (ctx->progressCallback == nullptr) {
        LOGE("progressCallback nullptr");
        return;
    }
    sdkHelper_->DownloadAssets(ctx->context, ctx->assets, {}, ctx->id,
        *(ctx->resultCallback), *(ctx->progressCallback));
}

void EmptyDownLoadAssetsprogress(std::shared_ptr<DKContext>, DKDownloadAsset, TotalSize, DownloadSize)
{
    return;
}

int DataSyncer::HandleOnFetchRecords(const std::shared_ptr<DKContext> context,
    std::shared_ptr<const DKDatabase> database, std::shared_ptr<std::vector<DKRecord>> records)
{
    if (records->size() == 0) {
        LOGI("no records to handle");
        return E_OK;
    }

    DKDownloadId id;
    vector<DKDownloadAsset> assetsToDownload;
    shared_ptr<std::function<void(std::shared_ptr<DriveKit::DKContext>,
                           std::shared_ptr<const DriveKit::DKDatabase>,
                           const std::map<DriveKit::DKDownloadAsset, DriveKit::DKDownloadResult> &,
                           const DriveKit::DKError &)>> downloadResultCallback = nullptr;
    auto downloadProcessCallback = 
        std::make_shared<std::function<void(std::shared_ptr<DKContext>, DKDownloadAsset, TotalSize, DownloadSize)>>(
            EmptyDownLoadAssetsprogress);

    auto ctx = static_pointer_cast<TaskContext>(context);
    auto handler = ctx->GetHandler();
    if (handler == nullptr) {
        LOGE("context get handler err");
        return E_CONTEXT;
    }
    int32_t ret = handler->OnFetchRecords(records, assetsToDownload, downloadResultCallback);
    if (ret != E_OK) {
        LOGE("handler on fetch records err %{public}d", ret);
        return ret;
    }

    auto dctx = make_shared<DownloadContext>(handler, assetsToDownload, id,
                                             downloadResultCallback, context, database, downloadProcessCallback);
    ret = AsyncRun(static_pointer_cast<TaskContext>(dctx), &DataSyncer::DownloadAssets);
    if (ret != E_OK) {
        LOGE("async run DownloadAssets err %{public}d", ret);
    }
    return E_OK;
}

void DataSyncer::OnFetchRecords(const std::shared_ptr<DKContext> context, std::shared_ptr<const DKDatabase> database,
    std::shared_ptr<std::vector<DKRecord>> records, DKQueryCursor nextCursor, const DKError &err)
{
    LOGI("%{private}d %{private}s on fetch records", userId_, bundleName_.c_str());
    auto ctx = static_pointer_cast<TaskContext>(context);
    if (HandleOnFetchRecords(context, database, records) != E_OK) {
        LOGE("HandleOnFetchRecords failed");
        return;
    }

    /* pull more */
    if (nextCursor.empty()) {
        LOGI("no more records");
        nextCursor_.clear();
        cloudPrefImpl_.SetString(NEXT_CURSOR, nextCursor_);
        cloudPrefImpl_.GetString(TEMP_START_CURSOR, startCursor_);
        cloudPrefImpl_.SetString(START_CURSOR, startCursor_);
        cloudPrefImpl_.Delete(TEMP_START_CURSOR);
        return;
    }
    nextCursor_ = nextCursor;
    cloudPrefImpl_.SetString(NEXT_CURSOR, nextCursor_);
    int ret = AsyncRun(ctx, &DataSyncer::PullRecords);
    if (ret != E_OK) {
        LOGE("asyn run pull records err %{public}d", ret);
    }
}

void DataSyncer::OnFetchDatabaseChanges(const std::shared_ptr<DKContext> context,
    std::shared_ptr<const DKDatabase> database,
    std::shared_ptr<std::vector<DriveKit::DKRecord>> records, DKQueryCursor nextCursor,
    bool hasMore, const DKError &err)
{
    LOGI("%{private}d %{private}s on fetch database changes", userId_, bundleName_.c_str());
    auto ctx = static_pointer_cast<TaskContext>(context);
    if (HandleOnFetchRecords(context, database, records) != E_OK) {
        LOGE("HandleOnFetchRecords failed");
        return;
    }

    /* pull more */
    if (!hasMore) {
        LOGI("no more records");
        startCursor_ = nextCursor;
        nextCursor_.clear();
        cloudPrefImpl_.SetString(NEXT_CURSOR, nextCursor_);
        cloudPrefImpl_.SetString(START_CURSOR, startCursor_);
        return;
    }
    nextCursor_ = nextCursor;
    cloudPrefImpl_.SetString(NEXT_CURSOR, nextCursor_);
    int ret = AsyncRun(ctx, &DataSyncer::PullDatabaseChanges);
    if (ret != E_OK) {
        LOGE("asyn run pull database changes err %{public}d", ret);
    }
}

void DataSyncer::PullRetryRecords(shared_ptr<TaskContext> context)
{
    auto ctx = static_pointer_cast<TaskContext>(context);
    auto handler = ctx->GetHandler();
    if (handler == nullptr) {
        LOGE("context get handler err");
        return;
    }

    vector<DKRecordId> records;
    int32_t ret = handler->GetRetryRecords(records);
    if (ret != E_OK) {
        LOGE("get retry records err %{public}d", ret);
        return;
    }

    LOGI("retry records count: %{public}u", static_cast<uint32_t>(records.size()));
    for (auto it : records) {
        auto callback = AsyncCallback(&DataSyncer::OnFetchRetryRecord);
        if (callback == nullptr) {
            LOGE("wrap on fetch records fail");
            continue;
        }
        int32_t ret = sdkHelper_->FetchRecordWithId(context, it, callback);
        if (ret != E_OK) {
            LOGE("sdk fetch records err %{public}d", ret);
        }
    }
}

void DataSyncer::OnFetchRetryRecord(shared_ptr<DKContext> context, shared_ptr<DKDatabase> database,
    DKRecordId recordId, const DKRecord &record, const DKError &error)
{
    auto records = make_shared<std::vector<DKRecord>>();
    if (error.HasError()) {
        LOGE("has error, recordId:%s, dkErrorCode :%{public}d, serverErrorCode:%{public}d", recordId.c_str(),
             error.dkErrorCode, error.serverErrorCode);
        LOGI("convert to delete record");
        DKRecord deleteRecord;
        deleteRecord.SetRecordId(recordId);
        deleteRecord.SetDelete(true);
        records->push_back(deleteRecord);
    } else {
        LOGI("handle retry record : %s", record.GetRecordId().c_str());
        records->push_back(record);
    }

    HandleOnFetchRecords(context, database, records);
}

int32_t DataSyncer::Push(shared_ptr<DataHandler> handler)
{
    /*
     * Although unlikely, if the first callback finds no more records available
     * and tries to schedule before following tasks commited, the data syncer
     * will directly schedule to the next stage, while following tasks would be
     * commited to the next stage mistakenly.
     * One possible solution: commit dummy task in the beginning and complete
     * dummy task in the end.
     */
    shared_ptr<TaskContext> context = make_shared<TaskContext>(handler);

    /* commit a dummy task */
    BeginTransaction();

    int32_t ret = AsyncRun(context, &DataSyncer::CreateRecords);
    if (ret != E_OK) {
        LOGE("async run create records err %{public}d", ret);
        return ret;
    }

    ret = AsyncRun(context, &DataSyncer::DeleteRecords);
    if (ret != E_OK) {
        LOGE("async run delete records err %{public}d", ret);
        return ret;
    }

    ret = AsyncRun(context, &DataSyncer::ModifyMdirtyRecords);
    if (ret != E_OK) {
        LOGE("async run modify mdirty records err %{public}d", ret);
        return ret;
    }

    ret = AsyncRun(context, &DataSyncer::ModifyFdirtyRecords);
    if (ret != E_OK) {
        LOGE("async run modify fdirty records err %{public}d", ret);
        return ret;
    }

    /* complete the dummy task */
    EndTransaction();

    return E_OK;
}

void DataSyncer::CreateRecords(shared_ptr<TaskContext> context)
{
    LOGI("%{private}d %{private}s creates records", userId_, bundleName_.c_str());

    auto handler = context->GetHandler();
    if (handler == nullptr) {
        LOGE("context get handler err");
        return;
    }

    /* query local */
    vector<DKRecord> records;
    int32_t ret = handler->GetCreatedRecords(records);
    if (ret != E_OK) {
        LOGE("handler get created records err %{public}d", ret);
        return;
    }

    /* no need upload */
    if (records.size() == 0) {
        return;
    }

    if (!BatteryStatus::IsAllowUpload(syncStateManager_.GetForceFlag())) {
        LOGE("battery status abnormal, abort upload");
        errorCode_ = E_SYNC_FAILED_BATTERY_LOW;
        return;
    }

    /* upload */
    auto callback = AsyncCallback(&DataSyncer::OnCreateRecords);
    if (callback == nullptr) {
        LOGE("wrap on create records fail");
        return;
    }
    ret = sdkHelper_->CreateRecords(context, records, callback);
    if (ret != E_OK) {
        LOGE("sdk create records err %{public}d", ret);
        return;
    }
}

void DataSyncer::DeleteRecords(shared_ptr<TaskContext> context)
{
    LOGI("%{private}d %{private}s deletes records", userId_, bundleName_.c_str());

    auto handler = context->GetHandler();
    if (handler == nullptr) {
        LOGE("context get handler err");
        return;
    }

    /* query local */
    vector<DKRecord> records;
    int32_t ret = handler->GetDeletedRecords(records);
    if (ret != E_OK) {
        LOGE("handler get deleted records err %{public}d", ret);
        return;
    }

    /* no need upload */
    if (records.size() == 0) {
        return;
    }

    /* upload */
    auto callback = AsyncCallback(&DataSyncer::OnDeleteRecords);
    if (callback == nullptr) {
        LOGE("wrap on delete records fail");
        return;
    }
    ret = sdkHelper_->DeleteRecords(context, records, callback);
    if (ret != E_OK) {
        LOGE("sdk delete records err %{public}d", ret);
    }
}

void DataSyncer::ModifyMdirtyRecords(shared_ptr<TaskContext> context)
{
    LOGI("%{private}d %{private}s modifies records", userId_, bundleName_.c_str());

    auto handler = context->GetHandler();
    if (handler == nullptr) {
        LOGE("context get handler err");
        return;
    }

    /* query local */
    vector<DKRecord> records;
    int32_t ret = handler->GetMetaModifiedRecords(records);
    if (ret != E_OK) {
        LOGE("handler get modified records err %{public}d", ret);
        return;
    }

    /* no need upload */
    if (records.size() == 0) {
        return;
    }

    /* upload */
    auto callback = AsyncCallback(&DataSyncer::OnModifyMdirtyRecords);
    if (callback == nullptr) {
        LOGE("wrap on modify records fail");
        return;
    }
    ret = sdkHelper_->ModifyRecords(context, records, callback);
    if (ret != E_OK) {
        LOGE("sdk modify records err %{public}d", ret);
    }
}

void DataSyncer::ModifyFdirtyRecords(shared_ptr<TaskContext> context)
{
    LOGI("%{private}d %{private}s modifies records", userId_, bundleName_.c_str());

    auto handler = context->GetHandler();
    if (handler == nullptr) {
        LOGE("context get handler err");
        return;
    }

    /* query local */
    vector<DKRecord> records;
    int32_t ret = handler->GetFileModifiedRecords(records);
    if (ret != E_OK) {
        LOGE("handler get modified records err %{public}d", ret);
        return;
    }

    /* no need upload */
    if (records.size() == 0) {
        return;
    }

    /* upload */
    auto callback = AsyncCallback(&DataSyncer::OnModifyFdirtyRecords);
    if (callback == nullptr) {
        LOGE("wrap on modify records fail");
        return;
    }
    ret = sdkHelper_->ModifyRecords(context, records, callback);
    if (ret != E_OK) {
        LOGE("sdk modify records err %{public}d", ret);
    }
}

void DataSyncer::OnCreateRecords(shared_ptr<DKContext> context,
    shared_ptr<const DKDatabase> database,
    shared_ptr<const map<DKRecordId, DKRecordOperResult>> map, const DKError &err)
{
    LOGI("%{private}d %{private}s on create records %{public}zu", userId_,
        bundleName_.c_str(), map->size());

    auto ctx = static_pointer_cast<TaskContext>(context);

    /* update local */
    auto handler = ctx->GetHandler();
    int32_t ret = handler->OnCreateRecords(*map);
    if (ret != E_OK) {
        LOGE("handler on create records err %{public}d", ret);
        return;
    }

    /* push more */
    ret = AsyncRun(ctx, &DataSyncer::CreateRecords);
    if (ret != E_OK) {
        LOGE("async run create records err %{public}d", ret);
        return;
    }
}

void DataSyncer::OnDeleteRecords(shared_ptr<DKContext> context,
    shared_ptr<const DKDatabase> database,
    shared_ptr<const map<DKRecordId, DKRecordOperResult>> map, const DKError &err)
{
    LOGI("%{private}d %{private}s on delete records %{public}zu", userId_,
        bundleName_.c_str(), map->size());

    auto ctx = static_pointer_cast<TaskContext>(context);

    /* update local */
    auto handler = ctx->GetHandler();
    int32_t ret = handler->OnDeleteRecords(*map);
    if (ret != E_OK) {
        LOGE("handler on delete records err %{public}d", ret);
        return;
    }

    /* push more */
    ret = AsyncRun(ctx, &DataSyncer::DeleteRecords);
    if (ret != E_OK) {
        LOGE("async run delete records err %{public}d", ret);
        return;
    }
}

void DataSyncer::OnModifyMdirtyRecords(shared_ptr<DKContext> context,
    shared_ptr<const DKDatabase> database,
    shared_ptr<const map<DKRecordId, DKRecordOperResult>> saveMap,
    shared_ptr<const map<DKRecordId, DKRecordOperResult>> deleteMap,
    const DKError &err)
{
    LOGI("%{private}d %{private}s on modify mdirty records %{public}zu", userId_,
        bundleName_.c_str(), saveMap->size());

    auto ctx = static_pointer_cast<TaskContext>(context);

    /* update local */
    auto handler = ctx->GetHandler();
    int32_t ret = handler->OnModifyMdirtyRecords(*saveMap);
    if (ret != E_OK) {
        LOGE("handler on modify records err %{public}d", ret);
        return;
    }

    /* push more */
    ret = AsyncRun(ctx, &DataSyncer::ModifyMdirtyRecords);
    if (ret != E_OK) {
        LOGE("async run modify records err %{public}d", ret);
        return;
    }
}

void DataSyncer::OnModifyFdirtyRecords(shared_ptr<DKContext> context,
    shared_ptr<const DKDatabase> database,
    shared_ptr<const map<DKRecordId, DKRecordOperResult>> saveMap,
    shared_ptr<const map<DKRecordId, DKRecordOperResult>> deleteMap,
    const DKError &err)
{
    LOGI("%{private}d %{private}s on modify fdirty records %{public}zu", userId_,
        bundleName_.c_str(), saveMap->size());

    auto ctx = static_pointer_cast<TaskContext>(context);

    /* update local */
    auto handler = ctx->GetHandler();
    int32_t ret = handler->OnModifyFdirtyRecords(*saveMap);
    if (ret != E_OK) {
        LOGE("handler on modify records err %{public}d", ret);
        return;
    }

    /* push more */
    ret = AsyncRun(ctx, &DataSyncer::ModifyFdirtyRecords);
    if (ret != E_OK) {
        LOGE("async run modify records err %{public}d", ret);
        return;
    }
}

int32_t DataSyncer::CommitTask(shared_ptr<Task> t)
{
    return taskManager_->CommitTask(t);
}

int32_t DataSyncer::AddTask(shared_ptr<Task> t)
{
    return taskManager_->AddTask(t);
}

int32_t DataSyncer::StartTask(shared_ptr<Task> t, TaskAction action)
{
    return taskManager_->StartTask(t, action);
}

void DataSyncer::CompleteTask(shared_ptr<Task> t)
{
    taskManager_->CompleteTask(t->GetId());
}

void DataSyncer::BeginTransaction()
{
    taskManager_->CommitDummyTask();
}

void DataSyncer::EndTransaction()
{
    taskManager_->CompleteDummyTask();
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
    return syncStateManager_.GetSyncState();
}

void DataSyncer::CompletePull()
{
    LOGI("%{private}d %{private}s completes pull", userId_, bundleName_.c_str());
    /* call syncer manager callback */
}

void DataSyncer::CompletePush()
{
    LOGI("%{private}d %{public}s completes push", userId_, bundleName_.c_str());
    /* call syncer manager callback */
}

void DataSyncer::CompleteAll(int32_t code, const SyncType type)
{
    LOGI("%{private}d %{private}s completes all", userId_, bundleName_.c_str());

    /* unlock */
    sdkHelper_->DeleteLock(lock_);

    if (errorCode_ == E_SYNC_FAILED_BATTERY_LOW) {
        code = errorCode_;
    }

    SyncState syncState;
    if (code == E_OK) {
        syncState = SyncState::SYNC_SUCCEED;
    } else {
        syncState = SyncState::SYNC_FAILED;
    }

    auto nextAction = syncStateManager_.UpdateSyncState(syncState);
    if (nextAction == Action::START) {
        StartSync(false, SyncTriggerType::PENDING_TRIGGER);
        return;
    }
    if (nextAction == Action::FORCE_START) {
        StartSync(true, SyncTriggerType::PENDING_TRIGGER);
        return;
    }

    auto state = GetSyncPromptState(code);
    if (code == E_OK) {
        CloudSyncCallbackManager::GetInstance().NotifySyncStateChanged(SyncType::ALL, state);
    } else {
        CloudSyncCallbackManager::GetInstance().NotifySyncStateChanged(type, state);
    }
}

void DataSyncer::SyncStateChangedNotify(const SyncType type, const SyncPromptState state)
{
    CloudSyncCallbackManager::GetInstance().NotifySyncStateChanged(SyncType::ALL, state);
}

SyncPromptState DataSyncer::GetSyncPromptState(const int32_t code)
{
    if (code == E_OK) {
        return SyncPromptState::SYNC_STATE_DEFAULT;
    }
    SyncPromptState state(SyncPromptState::SYNC_STATE_DEFAULT);
    auto capacityLevel = BatteryStatus::GetCapacityLevel();
    if (capacityLevel == BatteryStatus::LEVEL_LOW) {
        state = SyncPromptState::SYNC_STATE_PAUSED_FOR_BATTERY;
    } else if (capacityLevel == BatteryStatus::LEVEL_TOO_LOW) {
        state = SyncPromptState::SYNC_STATE_BATTERY_TOO_LOW;
    }

    return state;
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
