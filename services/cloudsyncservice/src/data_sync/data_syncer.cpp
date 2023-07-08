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
    : bundleName_(bundleName), userId_(userId)
{
    /* alloc task runner */
    taskRunner_ = DelayedSingleton<TaskManager>::GetInstance()->AllocRunner(userId,
        bundleName, bind(&DataSyncer::Schedule, this));
}

DataSyncer::~DataSyncer()
{
    /* release task runner */
    DelayedSingleton<TaskManager>::GetInstance()->ReleaseRunner(userId_, bundleName_);
}

int32_t DataSyncer::AsyncRun(std::shared_ptr<TaskContext> context,
    void(DataSyncer::*f)(std::shared_ptr<TaskContext>))
{
    return taskRunner_->AsyncRun<DataSyncer>(context, f, this);
}

template<typename T, typename RET, typename... ARGS>
function<RET(ARGS...)> DataSyncer::AsyncCallback(RET(T::*f)(ARGS...))
{
    return taskRunner_->AsyncCallback<DataSyncer>(f, this);
}

int32_t DataSyncer::StartSync(bool forceFlag, SyncTriggerType triggerType)
{
    LOGI("%{private}d %{public}s starts sync, isforceSync %{public}d, triggerType %{public}d",
        userId_, bundleName_.c_str(), forceFlag, triggerType);

    sdkHelper_->SaveSubscription();
    /* only one specific data sycner running at a time */
    if (syncStateManager_.CheckAndSetPending(forceFlag)) {
        LOGI("syncing, pending sync");
        return E_PENDING;
    }

    /* lock: device-reentrant */
    sdkHelper_->ResetLock(lock_);
    int32_t ret = sdkHelper_->GetLock(lock_);
    if (ret != E_OK) {
        LOGE("sdk helper get lock err %{public}d", ret);
        errorCode_ = ret;
        CompleteAll();
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

    SyncStateChangedNotify(CloudSyncState::STOPPED, ErrorType::NO_ERROR);

    return E_OK;
}

int32_t DataSyncer::StartDownloadFile(const std::string path, const int32_t userId)
{
    return E_OK;
}

int32_t DataSyncer::StopDownloadFile(const std::string path, const int32_t userId)
{
    downloadCallbackMgr_.StopDonwload(path, userId);
    return E_OK;
}

int32_t DataSyncer::RegisterDownloadFileCallback(const int32_t userId,
                                                 const sptr<ICloudDownloadCallback> downloadCallback)
{
    downloadCallbackMgr_.RegisterCallback(userId, downloadCallback);
    return E_OK;
}

int32_t DataSyncer::UnregisterDownloadFileCallback(const int32_t userId)
{
    downloadCallbackMgr_.UnregisterCallback(userId);
    return E_OK;
}

void DataSyncer::Abort()
{
    LOGI("%{private}d %{private}s aborts", userId_, bundleName_.c_str());
    thread ([this]() {
        /* stop all the tasks and wait for tasks' termination */
        if (!taskRunner_->StopAndWaitFor()) {
            LOGE("wait for tasks stop fail");
        }
        /* call the syncer manager's callback for notification */
        CompleteAll();
    }).detach();
}

void DataSyncer::SetSdkHelper(shared_ptr<SdkHelper> &sdkHelper)
{
    sdkHelper_ = sdkHelper;
}

int32_t DataSyncer::Pull(shared_ptr<DataHandler> handler)
{
    LOGI("%{private}d %{private}s pull", userId_, bundleName_.c_str());

    shared_ptr<TaskContext> context = make_shared<TaskContext>(handler);

    RetryDownloadRecords(context);
    PullRetryRecords(context);

    /* Full synchronization and incremental synchronization */
    int32_t ret = E_OK;
    if (handler->IsPullRecords()) {
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

    auto handler = context->GetHandler();
    if (handler == nullptr) {
        LOGE("context get handler err");
        return;
    }

    FetchCondition cond;
    handler->GetFetchCondition(cond);

    DKQueryCursor tempStartCursor;
    handler->GetTempStartCursor(tempStartCursor);
    if (tempStartCursor.empty()) {
        sdkHelper_->GetStartCursor(cond.recordType, tempStartCursor);
        handler->SetTempStartCursor(tempStartCursor);
    }

    DKQueryCursor nextCursor;
    handler->GetNextCursor(nextCursor);

    int32_t ret = sdkHelper_->FetchRecords(context, cond, nextCursor, callback);
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

    auto handler = context->GetHandler();
    if (handler == nullptr) {
        LOGE("context get handler err");
        return;
    }
    DKQueryCursor nextCursor;
    handler->GetNextCursor(nextCursor);

    FetchCondition cond;
    handler->GetFetchCondition(cond);
    int32_t ret = sdkHelper_->FetchDatabaseChanges(context, cond, nextCursor, callback);
    if (ret != E_OK) {
        LOGE("sdk fetch records err %{public}d", ret);
    }
}

class DownloadContext : public TaskContext {
public:
    DownloadContext(std::shared_ptr<DataHandler> handler,
                    const vector<DKDownloadAsset> &assets,
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


static void ThumbDownloadCallback(shared_ptr<DKContext> context,
                                  shared_ptr<const DKDatabase> database,
                                  const map<DKDownloadAsset, DKDownloadResult> &resultMap,
                                  const DKError &err)
{
    if (err.HasError()) {
        LOGE("DKAssetsDownloader err, localErr: %{public}d, serverErr: %{public}d", static_cast<int>(err.dkErrorCode),
             err.serverErrorCode);
    } else {
        LOGI("DKAssetsDownloader ok");
    }

    for (const auto &it : resultMap) {
        if (it.second.IsSuccess()) {
            LOGI("record %s %{public}s download success", it.first.recordId.c_str(), it.first.fieldKey.c_str());
            if (it.first.fieldKey == "thumbnail") {
                auto ctx = static_pointer_cast<TaskContext>(context);
                auto handler = ctx->GetHandler();
                handler->OnDownloadThumbSuccess(it.first);
            }
        } else {
            LOGE("record %s %{public}s download failed, localErr: %{public}d, serverErr: %{public}d",
                 it.first.recordId.c_str(), it.first.fieldKey.c_str(),
                 static_cast<int>(it.second.GetDKError().dkErrorCode), it.second.GetDKError().serverErrorCode);
        }
    }
}

int DataSyncer::HandleOnFetchRecords(const std::shared_ptr<DKContext> context,
    std::shared_ptr<const DKDatabase> database, std::shared_ptr<std::vector<DKRecord>> records)
{
    if (records->size() == 0) {
        LOGI("no records to handle");
        return E_OK;
    }

    DKDownloadId id;
    OnFetchParams onFetchParams;
    auto resultCallback = make_shared<std::function<void(
        std::shared_ptr<DriveKit::DKContext>, std::shared_ptr<const DriveKit::DKDatabase>,
        const std::map<DriveKit::DKDownloadAsset, DriveKit::DKDownloadResult> &, const DriveKit::DKError &)>>(
        ThumbDownloadCallback);
    auto downloadProcessCallback =
        std::make_shared<std::function<void(std::shared_ptr<DKContext>, DKDownloadAsset, TotalSize, DownloadSize)>>(
            EmptyDownLoadAssetsprogress);
    /* for pull records, judge whether to download thumb and lcds */

    auto ctx = static_pointer_cast<TaskContext>(context);
    auto handler = ctx->GetHandler();
    if (handler == nullptr) {
        LOGE("context get handler err");
        return E_CONTEXT;
    }

    int32_t totalPullCount = 0;
    int32_t downloadThumbLimit = 0;
    if (handler->IsPullRecords()) {
        handler->GetPullCount(totalPullCount, downloadThumbLimit);
        totalPullCount += records->size();
        onFetchParams.fetchThumbs = totalPullCount <= downloadThumbLimit;
        LOGI("fetchThumbs :%{public}d, totalPulls %{public}d, limit %{public}d",
             onFetchParams.fetchThumbs, totalPullCount, downloadThumbLimit);
    }

    int32_t ret = handler->OnFetchRecords(records, onFetchParams);
    auto dctx = make_shared<DownloadContext>(handler, onFetchParams.assetsToDownload, id, resultCallback,
                                             context, database, downloadProcessCallback);
    AsyncRun(static_pointer_cast<TaskContext>(dctx), &DataSyncer::DownloadAssets);
    if (ret != E_OK) {
        LOGE("handler on fetch records err %{public}d", ret);
        return ret;
    }

    if (handler->IsPullRecords()) {
        handler->SetTotalPullCount(totalPullCount);
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
    auto handler = ctx->GetHandler();
    if (handler == nullptr) {
        LOGE("context get handler err");
        return;
    }

    /* pull more */
    if (nextCursor.empty()) {
        LOGI("no more records");
        handler->FinishPull(nextCursor);
        return;
    }
    handler->SetNextCursor(nextCursor);
    int ret = AsyncRun(ctx, &DataSyncer::PullRecords);
    if (ret != E_OK) {
        LOGE("asyn run pull records err %{public}d", ret);
    }
}

int32_t DataSyncer::DownloadInner(std::shared_ptr<DataHandler> handler,
                                  const std::string path,
                                  const int32_t userId)
{
    DKDownloadId id;
    auto ctx = std::make_shared<TaskContext>(handler);
    std::vector<DKDownloadAsset> assetsToDownload;
    int32_t ret = handler->GetDownloadAsset(path, assetsToDownload);
    if (ret != E_OK) {
        LOGE("handler on fetch records err %{public}d", ret);
        return ret;
    }

    downloadCallbackMgr_.StartDonwload(path, userId);

    auto downloadResultCallback = [this, path, assetsToDownload, handler](
                                      std::shared_ptr<DriveKit::DKContext> context,
                                      std::shared_ptr<const DriveKit::DKDatabase> database,
                                      const std::map<DriveKit::DKDownloadAsset, DriveKit::DKDownloadResult> &results,
                                      const DriveKit::DKError &err) {
        this->downloadCallbackMgr_.OnDownloadedResult(path, assetsToDownload, handler, context, database, results, err);
    };
    auto downloadResultPtr = std::make_shared<std::function<void(
        std::shared_ptr<DriveKit::DKContext>, std::shared_ptr<const DriveKit::DKDatabase>,
        const std::map<DriveKit::DKDownloadAsset, DriveKit::DKDownloadResult> &, const DriveKit::DKError &)>>(
        downloadResultCallback);
    auto downloadProcessCallback = [this, path](std::shared_ptr<DKContext> context, DKDownloadAsset asset,
                                                     TotalSize totalSize, DownloadSize downloadSize) {
        this->downloadCallbackMgr_.OnDownloadProcess(path, context, asset, totalSize, downloadSize);
    };
    auto downloadProcessPtr =
        std::make_shared<std::function<void(std::shared_ptr<DKContext>, DKDownloadAsset, TotalSize, DownloadSize)>>(
            downloadProcessCallback);
    auto dctx = make_shared<DownloadContext>(handler, assetsToDownload, id, downloadResultPtr, ctx, nullptr,
                                             downloadProcessPtr);
    DownloadAssets(static_pointer_cast<TaskContext>(dctx));
    return E_OK;
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
    auto handler = ctx->GetHandler();
    if (handler == nullptr) {
        LOGE("context get handler err");
        return;
    }

    /* pull more */
    if (!hasMore) {
        LOGI("no more records");
        handler->FinishPull(nextCursor);
        return;
    }
    handler->SetNextCursor(nextCursor);
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

    FetchCondition cond;
    handler->GetFetchCondition(cond);
    LOGI("retry records count: %{public}u", static_cast<uint32_t>(records.size()));
    for (auto it : records) {
        auto callback = AsyncCallback(&DataSyncer::OnFetchRetryRecord);
        if (callback == nullptr) {
            LOGE("wrap on fetch records fail");
            continue;
        }
        ret = sdkHelper_->FetchRecordWithId(context, cond, it, callback);
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

void DataSyncer::RetryDownloadRecords(shared_ptr<TaskContext> context)
{
    auto ctx = static_pointer_cast<TaskContext>(context);
    auto handler = ctx->GetHandler();
    if (handler == nullptr) {
        LOGE("context get handler err");
        return;
    }

    vector<DriveKit::DKDownloadAsset> assetsToDownload;
    int32_t ret = handler->GetAssetsToDownload(assetsToDownload);
    if (ret != E_OK) {
        LOGE("get assetsToDownload err %{public}d", ret);
        return;
    }
    LOGI("assetsToDownload count: %{public}zu", assetsToDownload.size());
    if (assetsToDownload.empty()) {
        return;
    }

    DKDownloadId id;
    auto resultCallback = make_shared<std::function<void(
        std::shared_ptr<DriveKit::DKContext>, std::shared_ptr<const DriveKit::DKDatabase>,
        const std::map<DriveKit::DKDownloadAsset, DriveKit::DKDownloadResult> &, const DriveKit::DKError &)>>(
        ThumbDownloadCallback);
    auto downloadProcessCallback =
        std::make_shared<std::function<void(std::shared_ptr<DKContext>, DKDownloadAsset, TotalSize, DownloadSize)>>(
            EmptyDownLoadAssetsprogress);

    auto dctx = make_shared<DownloadContext>(handler, assetsToDownload, id, resultCallback, context, nullptr,
                                             downloadProcessCallback);
    AsyncRun(static_pointer_cast<TaskContext>(dctx), &DataSyncer::DownloadAssets);
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

int32_t DataSyncer::Clean(const int action)
{
    return E_OK;
}

int32_t DataSyncer::CleanInner(std::shared_ptr<DataHandler> handler, const int action)
{
    LOGD("Enter function DataSyncer::CleanInner");
    int res = handler->Clean(action);
    if (res != E_OK) {
        LOGE("Clean file failed res:%{public}d", res);
        return res;
    }
    handler->ClearCursor();
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

void DataSyncer::BeginTransaction()
{
    taskRunner_->CommitDummyTask();
}

void DataSyncer::EndTransaction()
{
    taskRunner_->CompleteDummyTask();
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
    auto error = GetErrorType(errorCode_);
    if (error) {
        SyncStateChangedNotify(CloudSyncState::DOWNLOAD_FAILED, error);
    }
}

void DataSyncer::CompletePush()
{
    LOGI("%{private}d %{public}s completes push", userId_, bundleName_.c_str());
    /* call syncer manager callback */
    auto error = GetErrorType(errorCode_);
    if (error) {
        SyncStateChangedNotify(CloudSyncState::UPLOAD_FAILED, error);
    }
}

void DataSyncer::CompleteAll()
{
    LOGI("%{private}d %{private}s completes all", userId_, bundleName_.c_str());

    /* unlock */
    sdkHelper_->DeleteLock(lock_);

    /* reset internal status */
    Reset();

    SyncState syncState;
    if (errorCode_ == E_OK) {
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

    /* notify sync state */
    auto error = GetErrorType(errorCode_);
    SyncStateChangedNotify(CloudSyncState::COMPLETED, error);
    errorCode_ = E_OK;
}

void DataSyncer::SyncStateChangedNotify(const CloudSyncState state, const ErrorType error)
{
    CloudSyncCallbackManager::GetInstance().NotifySyncStateChanged(state, error);
}

ErrorType DataSyncer::GetErrorType(const int32_t code)
{
    if (code == E_OK) {
        return ErrorType::NO_ERROR;
    }
    ErrorType error(ErrorType::NO_ERROR);
    auto capacityLevel = BatteryStatus::GetCapacityLevel();
    if (capacityLevel == BatteryStatus::LEVEL_LOW) {
        error = ErrorType::BATTERY_LEVEL_LOW;
    } else if (capacityLevel == BatteryStatus::LEVEL_TOO_LOW) {
        error = ErrorType::BATTERY_LEVEL_WARNING;
    }

    return error;
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
