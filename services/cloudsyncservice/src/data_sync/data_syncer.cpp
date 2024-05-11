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

#include "data_syncer.h"

#include <functional>
#include <memory>

#include "data_sync_const.h"
#include "data_sync_notifier.h"
#include "data_syncer_rdb_store.h"
#include "dfs_error.h"
#include "ipc/cloud_sync_callback_manager.h"
#include "sdk_helper.h"
#include "sync_rule/battery_status.h"
#include "sync_rule/network_status.h"
#include "sync_rule/screen_status.h"
#include "sync_rule/cloud_status.h"
#include "sync_rule/system_load.h"
#include "task_state_manager.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
using namespace std;
using namespace placeholders;
using namespace DriveKit;
using ChangeType = AAFwk::ChangeInfo::ChangeType;

DataSyncer::DataSyncer(const std::string bundleName, const int32_t userId)
    : bundleName_(bundleName), userId_(userId)
{
    /* alloc task runner */
    taskRunner_ = DelayedSingleton<TaskManager>::GetInstance()->AllocRunner(userId,
        bundleName, bind(&DataSyncer::Schedule, this));
    taskRunner_->SetStopFlag(stopFlag_);
    downloadCallbackMgr_.SetBundleName(bundleName);
}

DataSyncer::~DataSyncer()
{
    /* release task runner */
    DelayedSingleton<TaskManager>::GetInstance()->ReleaseRunner(userId_, bundleName_);
}

int32_t DataSyncer::AsyncRun(std::shared_ptr<TaskContext> context,
    void(DataSyncer::*f)(std::shared_ptr<TaskContext>))
{
    if (forceFlag_ || SystemLoadStatus::IsLoadStatusOkay()) {
        return taskRunner_->AsyncRun<DataSyncer>(context, f, this);
    }
    return E_SYSTEM_LOAD_OVER;
}

template<typename T, typename RET, typename... ARGS>
function<RET(ARGS...)> DataSyncer::AsyncCallback(RET(T::*f)(ARGS...))
{
    if (forceFlag_ || SystemLoadStatus::IsLoadStatusOkay()) {
        return taskRunner_->AsyncCallback<DataSyncer>(f, this);
    }
    return nullptr;
}

int32_t DataSyncer::StartSync(bool forceFlag, SyncTriggerType triggerType)
{
    LOGI("%{private}d %{public}s starts sync, isforceSync %{public}d, triggerType %{public}d",
        userId_, bundleName_.c_str(), forceFlag, triggerType);

    forceFlag_ = forceFlag;
    triggerType_ = triggerType;

    /* only one specific data sycner running at a time */
    if (syncStateManager_.CheckAndSetPending(forceFlag, triggerType)) {
        LOGI("syncing, pending sync");
        return E_PENDING;
    }

    startTime_ = GetCurrentTimeStamp();
    int32_t ret = InitSysEventData();
    if (ret != E_OK) {
        return E_DATA;
    }

    TaskStateManager::GetInstance().StartTask(bundleName_, TaskType::SYNC_TASK);
    /* start data sync */
    errorCode_ = E_OK;
    ScheduleByType(triggerType);

    return E_OK;
}

int32_t DataSyncer::StopSync(SyncTriggerType triggerType)
{
    LOGI("%{private}d %{public}s stops sync, trigger stop sync, type:%{public}d",
        userId_, bundleName_.c_str(), triggerType);
    syncStateManager_.SetStopSyncFlag();
    StopUploadAssets();
    Abort();
    return E_OK;
}

int32_t DataSyncer::Lock()
{
    return E_OK;
}

void DataSyncer::Unlock() {}

void DataSyncer::ForceUnlock() {}

int32_t DataSyncer::StartDownloadFile(const std::string path, const int32_t userId)
{
    return E_OK;
}

int32_t DataSyncer::StopDownloadFile(const std::string path, const int32_t userId)
{
    DownloadProgressObj download;
    if (downloadCallbackMgr_.StopDonwload(path, userId, download)) {
        int64_t downloadId = download.downloadId;
        int32_t res = sdkHelper_->CancelDownloadAssets(downloadId);
        LOGD("CancelDownloadAssets result: %d", res);
        if (res != NO_ERROR) {
            LOGE("CancelDownloadAssets fail %{public}d", res);
            return res;
        }
        downloadCallbackMgr_.NotifyProcessStop(download);
    }
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
        taskRunner_->ReleaseTask();
        /* call the syncer manager's callback for notification */
        Complete();
    }).detach();
}

void DataSyncer::SetSdkHelper(shared_ptr<SdkHelper> &sdkHelper)
{
    sdkHelper_ = sdkHelper;
}

int32_t DataSyncer::Pull(shared_ptr<DataHandler> handler)
{
    LOGI("%{private}d %{private}s pull", userId_, bundleName_.c_str());

    shared_ptr<TaskContext> context = make_shared<DownloadTaskContext>(handler, handler->GetBatchNo());

    RetryDownloadRecords(context);
    vector<DKRecordId> records;
    int32_t ret = handler->GetRetryRecords(records);
    if (ret == E_OK) {
        DataSyncer::PullRecordsWithId(context, records, true);
    }

    /* Full synchronization and incremental synchronization */
    if (handler->IsPullRecords()) {
        SetFullSyncSysEvent();
        if (handler->GetCheckFlag()) {
            SetCheckSysEvent();
        }
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

    SdkHelper::FetchRecordsCallback callback = nullptr;
    if (handler->GetCheckFlag()) {
        callback = AsyncCallback(&DataSyncer::OnFetchCheckRecords);
    } else {
        callback = AsyncCallback(&DataSyncer::OnFetchRecords);
    }
    if (callback == nullptr) {
        LOGE("wrap on fetch records fail");
        return;
    }

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

struct DownloadContext {
    std::shared_ptr<DKContext> context;
    vector<DKDownloadAsset> assets;
    DriveKit::DKDownloadId id;
    std::function<void(std::shared_ptr<DriveKit::DKContext>,
                       std::shared_ptr<const DriveKit::DKDatabase>,
                       const std::map<DriveKit::DKDownloadAsset, DriveKit::DKDownloadResult> &,
                       const DriveKit::DKError &)>
        resultCallback;
    std::function<
        void(std::shared_ptr<DriveKit::DKContext>, DriveKit::DKDownloadAsset, TotalSize, DriveKit::DownloadSize)>
        progressCallback;
};

void DataSyncer::DownloadAssets(DownloadContext &ctx)
{
    if (ctx.resultCallback == nullptr) {
        LOGE("resultCallback nullptr");
        return;
    }

    if (ctx.progressCallback == nullptr) {
        LOGE("progressCallback nullptr");
        return;
    }

    if (ctx.assets.size() == 0) {
        LOGE("no assets to download");
        return;
    }

    int32_t ret = sdkHelper_->DownloadAssets(ctx.context, ctx.assets, {}, ctx.id,
        ctx.resultCallback, ctx.progressCallback);
    if (ret != E_OK) {
        LOGE("sdk download assets error %{public}d", ret);
        /*
         * 1. remove previous added tasks
         * 2. invoke callback here to do some statistics in handler
         * 3. set assets back to handler for its info
         */
        shared_ptr<TaskContext> tctx = static_pointer_cast<TaskContext>(ctx.context);
        tctx->SetAssets(ctx.assets);
        ctx.resultCallback(ctx.context, nullptr, {}, {});
    }
}

void DataSyncer::DownloadThumbAssets(DownloadContext ctx)
{
    if (ctx.resultCallback == nullptr) {
        LOGE("resultCallback nullptr");
        return;
    }

    if (ctx.progressCallback == nullptr) {
        LOGE("progressCallback nullptr");
        return;
    }

    if (ctx.assets.size() == 0) {
        LOGE("no assets to download");
        return;
    }

    int32_t ret = sdkHelper_->DownloadAssets(ctx.context, ctx.assets, {}, ctx.id,
        ctx.resultCallback, ctx.progressCallback);
    if (ret != E_OK) {
        LOGE("sdk download assets error %{public}d", ret);
        shared_ptr<TaskContext> tctx = static_pointer_cast<TaskContext>(ctx.context);
        tctx->SetAssets(ctx.assets);
        ctx.resultCallback(ctx.context, nullptr, {}, {});
    }
}

void DataSyncer::FetchRecordsDownloadCallback(shared_ptr<DKContext> context,
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

    auto ctx = static_pointer_cast<TaskContext>(context);
    auto handler = ctx->GetHandler();
    if (resultMap.size() != 0) {
        handler->OnDownloadAssets(resultMap);
    } else {
        /* error account */
        (void)handler->OnDownloadAssetsFailure(ctx->GetAssets());
    }
}

static void FetchRecordsDownloadProgress(shared_ptr<DKContext> context,
                                         DKDownloadAsset asset,
                                         TotalSize total,
                                         DownloadSize download)
{
    LOGD("record %s %{public}s download progress", asset.recordId.c_str(), asset.fieldKey.c_str());
    if (total == download) {
        auto ctx = static_pointer_cast<TaskContext>(context);
        auto handler = ctx->GetHandler();
        handler->OnDownloadAssets(asset);
    }
    /* account error in result callback but not here */
}

int DataSyncer::HandleOnFetchRecords(const std::shared_ptr<DownloadTaskContext> context,
    std::shared_ptr<const DKDatabase> database, std::shared_ptr<std::vector<DKRecord>> records, bool checkOrRetry)
{
    if (records->size() == 0) {
        LOGI("no records to handle");
        return E_OK;
    }

    OnFetchParams onFetchParams;
    auto ctx = static_pointer_cast<TaskContext>(context);
    auto handler = ctx->GetHandler();
    if (handler == nullptr) {
        LOGE("context get handler err");
        return E_CONTEXT;
    }

    if (handler->IsPullRecords() && !checkOrRetry) {
        onFetchParams.totalPullCount = context->GetBatchNo() * handler->GetRecordSize();
    }

    int32_t ret = handler->OnFetchRecords(records, onFetchParams);
    if (!onFetchParams.assetsToDownload.empty()) {
        auto resultCallback = AsyncCallback(&DataSyncer::FetchRecordsDownloadCallback);
        if (resultCallback != nullptr) {
            DownloadContext dctx = {.context = context,
                                    .assets = onFetchParams.assetsToDownload,
                                    .id = 0,
                                    .resultCallback = resultCallback,
                                    .progressCallback = FetchRecordsDownloadProgress};
            DownloadAssets(dctx);
        }
    }

    if (ret != E_OK) {
        LOGE("handler on fetch records err %{public}d", ret);
    } else {
        SyncStateChangedNotify(CloudSyncState::DOWNLOADING, ErrorType::NO_ERROR);
    }
    if (!checkOrRetry) {
        handler->FinishPull(context->GetBatchNo());
    }
    return ret;
}

void DataSyncer::OnFetchRecords(const std::shared_ptr<DKContext> context, std::shared_ptr<const DKDatabase> database,
    std::shared_ptr<std::vector<DKRecord>> records, DKQueryCursor nextCursor, const DKError &err)
{
    if (err.HasError()) {
        LOGE("OnFetchRecords server err %{public}d and dk errcor %{public}d", err.serverErrorCode, err.dkErrorCode);
        if (static_cast<DKServerErrorCode>(err.serverErrorCode) == DKServerErrorCode::NETWORK_ERROR) {
            SetErrorCodeMask(ErrorType::NETWORK_UNAVAILABLE);
        } else if (!err.errorDetails.empty() && static_cast<DKDetailErrorCode>(err.errorDetails[0].detailCode) ==
                   DKDetailErrorCode::FORBIDDEN_USER) {
            SetErrorCodeMask(ErrorType::PERMISSION_NOT_ALLOW);
        }
        return;
    }

    LOGI("%{private}d %{private}s on fetch records", userId_, bundleName_.c_str());
    auto ctx = static_pointer_cast<DownloadTaskContext>(context);
    auto handler = ctx->GetHandler();
    if (handler == nullptr) {
        LOGE("context get handler err");
        return;
    }
    if (ctx->GetBatchNo() == 0) {
        handler->SetRecordSize(records->size());
    }
    /* pull more */
    if (nextCursor.empty()) {
        LOGI("no more records");
        handler->GetTempStartCursor(nextCursor);
        handler->SetTempNextCursor(nextCursor, true);
        if (records->size() == 0) {
            handler->FinishPull(ctx->GetBatchNo());
        }
    } else {
        handler->SetTempNextCursor(nextCursor, false);
        shared_ptr<DownloadTaskContext> nexCtx = make_shared<DownloadTaskContext>(handler, handler->GetBatchNo());
        int ret = AsyncRun(nexCtx, &DataSyncer::PullRecords);
        if (ret != E_OK) {
            LOGE("asyn run pull records err %{public}d", ret);
        }
    }

    if (HandleOnFetchRecords(ctx, database, records, false) != E_OK) {
        LOGE("HandleOnFetchRecords failed");
    }
}

int32_t DataSyncer::DownloadInner(std::shared_ptr<DataHandler> handler,
                                  const std::string path,
                                  const int32_t userId)
{
    auto ctx = std::make_shared<DentryContext>();
    std::vector<DKDownloadAsset> assetsToDownload;
    int32_t ret = handler->GetDownloadAsset(path, assetsToDownload, ctx);
    if (ret != E_OK) {
        LOGE("handler on fetch records err %{public}d", ret);
        return ret;
    }

    if (downloadCallbackMgr_.FindDownload(path)) {
        LOGI("download exist %{public}s", path.c_str());
        return E_OK;
    }

    auto downloadResultCallback = [this, path, assetsToDownload, handler](
                                      std::shared_ptr<DriveKit::DKContext> context,
                                      std::shared_ptr<const DriveKit::DKDatabase> database,
                                      const std::map<DriveKit::DKDownloadAsset, DriveKit::DKDownloadResult> &results,
                                      const DriveKit::DKError &err) {
        this->downloadCallbackMgr_.OnDownloadedResult(path, assetsToDownload, handler, context, database, results, err);
    };
    auto downloadProcessCallback = [this, path](std::shared_ptr<DKContext> context, DKDownloadAsset asset,
                                                     TotalSize totalSize, DownloadSize downloadSize) {
        this->downloadCallbackMgr_.OnDownloadProcess(path, context, asset, totalSize, downloadSize);
    };
    DownloadContext dctx = {.context = ctx,
                            .assets = assetsToDownload,
                            .id = 0,
                            .resultCallback = downloadResultCallback,
                            .progressCallback = downloadProcessCallback};
    DownloadAssets(dctx);
    downloadCallbackMgr_.StartDonwload(path, userId, dctx.id);
    return E_OK;
}

void DataSyncer::OnFetchDatabaseChanges(const std::shared_ptr<DKContext> context,
    std::shared_ptr<const DKDatabase> database,
    std::shared_ptr<std::vector<DriveKit::DKRecord>> records, DKQueryCursor nextCursor,
    bool hasMore, const DKError &err)
{
    LOGI("%{private}d %{private}s on fetch database changes", userId_, bundleName_.c_str());
    auto ctx = static_pointer_cast<DownloadTaskContext>(context);

    auto handler = ctx->GetHandler();
    if (handler == nullptr) {
        LOGE("context get handler err");
        return;
    }

    if (err.HasError()) {
        LOGE("OnFetchDatabaseChanges server err %{public}d and dk errcor %{public}d", err.serverErrorCode,
            err.dkErrorCode);
        if (static_cast<DKServerErrorCode>(err.serverErrorCode) == DKServerErrorCode::NETWORK_ERROR) {
            SetErrorCodeMask(ErrorType::NETWORK_UNAVAILABLE);
        } else if (!err.errorDetails.empty()) {
            DKDetailErrorCode detailCode = static_cast<DKDetailErrorCode>(err.errorDetails[0].detailCode);
            if (detailCode == DKDetailErrorCode::FORBIDDEN_USER) {
                SetErrorCodeMask(ErrorType::PERMISSION_NOT_ALLOW);
            }
            if (detailCode == DKDetailErrorCode::PARAM_INVALID || detailCode == DKDetailErrorCode::CURSOR_EXPIRED) {
                handler->SetChecking();
                Pull(handler);
            }
        }
        return;
    }

    /* pull more */
    if (!hasMore) {
        LOGI("no more records");
        handler->SetTempNextCursor(nextCursor, true);
        if (records->size() == 0) {
            handler->FinishPull(ctx->GetBatchNo());
        }
    } else {
        handler->SetTempNextCursor(nextCursor, false);
        shared_ptr<DownloadTaskContext> nexCtx = make_shared<DownloadTaskContext>(handler, handler->GetBatchNo());
        int ret = AsyncRun(nexCtx, &DataSyncer::PullDatabaseChanges);
        if (ret != E_OK) {
            LOGE("asyn run pull database changes err %{public}d", ret);
        }
    }

    if (HandleOnFetchRecords(ctx, database, records, false) != E_OK) {
        LOGE("HandleOnFetchRecords failed");
        return;
    }
}

void DataSyncer::OnFetchCheckRecords(const shared_ptr<DKContext> context,
    shared_ptr<const DKDatabase> database, shared_ptr<std::vector<DKRecord>> records,
    DKQueryCursor nextCursor, const DKError &err)
{
    if (err.HasError()) {
        LOGE("OnFetchCheckRecords server err %{public}d and dk errcor %{public}d", err.serverErrorCode,
             err.dkErrorCode);
        if (static_cast<DKServerErrorCode>(err.serverErrorCode) == DKServerErrorCode::NETWORK_ERROR) {
            SetErrorCodeMask(ErrorType::NETWORK_UNAVAILABLE);
        } else if (!err.errorDetails.empty() && static_cast<DKDetailErrorCode>(err.errorDetails[0].detailCode) ==
                   DKDetailErrorCode::FORBIDDEN_USER) {
            SetErrorCodeMask(ErrorType::PERMISSION_NOT_ALLOW);
        }

        return;
    }
    LOGI("%{private}d %{private}s on fetch records", userId_, bundleName_.c_str());
    auto ctx = static_pointer_cast<DownloadTaskContext>(context);
    auto handler = ctx->GetHandler();
    if (handler == nullptr) {
        LOGE("context get handler err");
        return;
    }

    int32_t ret = E_OK;
    /* pull more */
    if (nextCursor.empty()) {
        LOGI("no more records");
        handler->GetTempStartCursor(nextCursor);
        handler->SetTempNextCursor(nextCursor, true);
    } else {
        handler->SetTempNextCursor(nextCursor, false);
        shared_ptr<DownloadTaskContext> nexCtx = make_shared<DownloadTaskContext>(handler, handler->GetBatchNo());
        ret = AsyncRun(nexCtx, &DataSyncer::PullRecords);
        if (ret != E_OK) {
            LOGE("asyn run pull records err %{public}d", ret);
        }
    }

    std::vector<DriveKit::DKRecordId> checkRecords;
    ret = handler->GetCheckRecords(checkRecords, records);
    if (ret != E_OK) {
        LOGE("handler get check records err %{public}d", ret);
        return;
    }

    DataSyncer::PullRecordsWithId(ctx, checkRecords, false);
}

void DataSyncer::PullRecordsWithId(shared_ptr<TaskContext> context, const std::vector<DriveKit::DKRecordId> &records,
    bool retry)
{
    auto ctx = static_pointer_cast<DownloadTaskContext>(context);
    auto handler = ctx->GetHandler();
    if (handler == nullptr) {
        LOGE("context get handler err");
        return;
    }
    if (records.empty()) {
        if (!retry) {
            handler->FinishPull(ctx->GetBatchNo());
        }
        LOGI("PullRecordsWithId records is empty");
        return;
    }

    FetchCondition cond;
    handler->GetFetchCondition(cond);
    LOGI("retry records count: %{public}u", static_cast<uint32_t>(records.size()));

    std::vector<DKRecord> recordsToFetch;
    for (const auto& it : records) {
        DKRecord record;
        record.SetRecordId(it);
        record.SetRecordType(cond.recordType);
        recordsToFetch.push_back(record);
    }
    auto callback = AsyncCallback(&DataSyncer::OnFetchRecordWithIds);
    if (callback == nullptr) {
        LOGE("wrap on fetch batch records fail");
        return;
    }
    int32_t ret = sdkHelper_->FetchRecordWithIds(context, cond, std::move(recordsToFetch), callback);
    if (ret != E_OK) {
        LOGE("sdk fetch batch records err %{public}d", ret);
    }
    if (!retry) {
        handler->FinishPull(ctx->GetBatchNo());
    }
}

void DataSyncer::OnFetchRecordWithIds(shared_ptr<DKContext> context, shared_ptr<DKDatabase> database,
    shared_ptr<map<DKRecordId, DKRecordOperResult>> recordMap, const DKError &error)
{
    if (error.HasError()) {
        LOGE("OnFetchRecordWithIds err, localErr: %{public}d, serverErr: %{public}d",
            static_cast<int>(error.dkErrorCode), error.serverErrorCode);
    } else {
        LOGI("OnFetchRecordWithIds ok");
    }
    auto records = make_shared<std::vector<DKRecord>>();
    for (const auto &it : *recordMap) {
        auto recordId = it.first;
        if (!it.second.IsSuccess()) {
            LOGE("has error, recordId:%s, convert to delete record", recordId.c_str());
            DKRecord deleteRecord;
            deleteRecord.SetRecordId(recordId);
            deleteRecord.SetDelete(true);
            records->push_back(deleteRecord);
        } else {
            LOGI("handle retry record : %s", recordId.c_str());
            auto record = it.second.GetDKRecord();
            records->push_back(record);
        }
    }

    auto ctx = static_pointer_cast<DownloadTaskContext>(context);
    HandleOnFetchRecords(ctx, database, records, true);
}

void DataSyncer::OnFetchRecordWithId(shared_ptr<DKContext> context, shared_ptr<DKDatabase> database,
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
    auto ctx = static_pointer_cast<DownloadTaskContext>(context);
    HandleOnFetchRecords(ctx, database, records, true);
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
    if (assetsToDownload.empty()) {
        return;
    }

    LOGI("assetsToDownload count: %{public}zu", assetsToDownload.size());
        auto resultCallback = AsyncCallback(&DataSyncer::FetchRecordsDownloadCallback);
        if (resultCallback != nullptr) {
            DownloadContext dctx = {.context = ctx,
                                    .assets = assetsToDownload,
                                    .id = 0,
                                    .resultCallback = resultCallback,
                                    .progressCallback = FetchRecordsDownloadProgress};
            DownloadAssets(dctx);
        }
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

int32_t DataSyncer::Init(const std::string bundleName, const int32_t userId)
{
    return E_OK;
}

int32_t DataSyncer::Clean(const int action)
{
    return E_OK;
}

int32_t DataSyncer::DisableCloud()
{
    return E_OK;
}

int32_t DataSyncer::CleanInner(std::shared_ptr<DataHandler> handler, const int action)
{
    int ret = 0;
    LOGD("Enter function DataSyncer::CleanInner");
    handler->ClearCursor();
    ret = handler->Clean(action);
    if (ret != E_OK) {
        LOGE("Clean file failed res:%{public}d", ret);
        return ret;
    }
    return ret;
}

int32_t DataSyncer::CancelDownload(std::shared_ptr<DataHandler> handler)
{
    int32_t ret = 0;
    std::vector<int64_t> downloadIds = downloadCallbackMgr_.StopAllDownloads(userId_);
    if (!HasSdkHelper()) {
        LOGW(" sdk helper is null skip cancel download");
        return ret;
    }
    for (const auto &id : downloadIds) {
        ret = sdkHelper_->CancelDownloadAssets(id);
        if (ret != NO_ERROR) {
            LOGE("CancelDownloadAssets fail %{public}d", ret);
            return ret;
        }
    }
    return ret;
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
        SetErrorCodeMask(ErrorType::BATTERY_LEVEL_LOW);
        return;
    }

    if ((NetworkStatus::GetNetConnStatus() != NetworkStatus::WIFI_CONNECT)) {
        LOGE("network status abnormal, abort upload");
        SetErrorCodeMask(ErrorType::WIFI_UNAVAILABLE);
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

    if ((NetworkStatus::GetNetConnStatus() != NetworkStatus::WIFI_CONNECT)) {
        LOGE("network status abnormal, abort upload");
        SetErrorCodeMask(ErrorType::WIFI_UNAVAILABLE);
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

    if ((NetworkStatus::GetNetConnStatus() != NetworkStatus::WIFI_CONNECT)) {
        LOGE("network status abnormal, abort upload");
        SetErrorCodeMask(ErrorType::WIFI_UNAVAILABLE);
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

    if ((NetworkStatus::GetNetConnStatus() != NetworkStatus::WIFI_CONNECT)) {
        LOGE("network status abnormal, abort upload");
        SetErrorCodeMask(ErrorType::WIFI_UNAVAILABLE);
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
        UpdateErrorCode(ret);
        return;
    } else {
        SyncStateChangedNotify(CloudSyncState::UPLOADING, ErrorType::NO_ERROR);
        isDataChanged_ = true;
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
        UpdateErrorCode(ret);
        return;
    } else {
        isDataChanged_ = true;
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
        UpdateErrorCode(ret);
        return;
    } else {
        isDataChanged_ = true;
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
        UpdateErrorCode(ret);
        return;
    } else {
        isDataChanged_ = true;
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

int32_t DataSyncer::CompletePull()
{
    LOGI("%{private}d %{private}s completes pull", userId_, bundleName_.c_str());
    /* call syncer manager callback */
    auto error = GetErrorType();
    if (error) {
        LOGE("pull failed, errorType:%{public}d", error);
        SyncStateChangedNotify(CloudSyncState::DOWNLOAD_FAILED, error);
        Complete(false);
    } else {
        /* schedule to next stage */
        Schedule();
    }
    return E_OK;
}

int32_t DataSyncer::CompletePush()
{
    LOGI("%{private}d %{public}s completes push", userId_, bundleName_.c_str());
    /* call syncer manager callback */
    auto error = GetErrorType();
    if (error) {
        LOGE("pull failed, errorType:%{public}d", error);
        SyncStateChangedNotify(CloudSyncState::UPLOAD_FAILED, error);
        Complete(false);
    } else {
        /* schedule to next stage */
        Schedule();
    }
    return E_OK;
}

void DataSyncer::CompleteAll(bool isNeedNotify)
{
    LOGI("%{private}d %{private}s completes all", userId_, bundleName_.c_str());

    /* guarantee: unlock if locked */
    ForceUnlock();

    /* reset internal status */
    Reset();

    SyncState syncState = SyncState::SYNC_SUCCEED;
    if (errorCode_ != E_OK) {
        syncState = SyncState::SYNC_FAILED;
    }

    /* sys event report and free */
    ReportSysEvent(errorCode_);
    FreeSysEventData();

    CloudSyncState notifyState = CloudSyncState::COMPLETED;
    if (syncStateManager_.GetStopSyncFlag()) {
        notifyState = CloudSyncState::STOPPED;
    } else {
        SaveSubscription();
    }

    auto nextAction = syncStateManager_.UpdateSyncState(syncState);
    DataSyncerRdbStore::GetInstance().UpdateSyncState(userId_, bundleName_, syncState);

    if (nextAction == Action::START) {
        /* Retrigger sync, clear errorcode */
        errorCode_ = E_OK;
        StartSync(false, SyncTriggerType::PENDING_TRIGGER);
        return;
    } else if (nextAction == Action::FORCE_START) {
        /* Retrigger sync, clear errorcode */
        errorCode_ = E_OK;
        StartSync(true, SyncTriggerType::PENDING_TRIGGER);
        return;
    } else if (nextAction == Action::CHECK) {
        errorCode_ = E_OK;
        StartSync(false, SyncTriggerType::TASK_TRIGGER);
        return;
    } else {
        TaskStateManager::GetInstance().CompleteTask(bundleName_, TaskType::SYNC_TASK);
    }

    /* notify sync state */
    if (isNeedNotify) {
        SyncStateChangedNotify(notifyState, ErrorType::NO_ERROR);
    }
    /* clear errorcode */
    errorCode_ = E_OK;
}

void DataSyncer::BeginClean()
{
    *stopFlag_ = true;
    /* stop all the tasks and wait for tasks' termination */
    taskRunner_->ReleaseTask();
    TaskStateManager::GetInstance().StartTask(bundleName_, TaskType::CLEAN_TASK);
    /* set cleaning  state */
    (void)syncStateManager_.SetCleaningFlag();
}

void DataSyncer::CompleteClean()
{
    DeleteSubscription();
    TaskStateManager::GetInstance().CompleteTask(bundleName_, TaskType::CLEAN_TASK);
    if (syncStateManager_.GetSyncState() != SyncState::DISABLE_CLOUD) {
        DataSyncerRdbStore::GetInstance().UpdateSyncState(userId_, bundleName_, SyncState::CLEAN_SUCCEED);
        auto nextAction = syncStateManager_.UpdateSyncState(SyncState::CLEAN_SUCCEED);
        if (nextAction != Action::STOP) {
            /* Retrigger sync, clear errorcode */
            if (!CloudStatus::IsCloudStatusOkay(bundleName_, userId_)) {
                LOGE("cloud status is not OK");
                return;
            }
            StartSync(false, SyncTriggerType::PENDING_TRIGGER);
        }
    }
    *stopFlag_ = false;
}

void DataSyncer::BeginDisableCloud()
{
    /* stop all the tasks and wait for tasks' termination */
    TaskStateManager::GetInstance().StartTask(bundleName_, TaskType::DISABLE_CLOUD_TASK);
    /* set disable cloud  state */
    (void)syncStateManager_.SetDisableCloudFlag();
}

void DataSyncer::CompleteDisableCloud()
{
    DataSyncerRdbStore::GetInstance().UpdateSyncState(userId_, bundleName_, SyncState::DISABLE_CLOUD_SUCCEED);
    TaskStateManager::GetInstance().CompleteTask(bundleName_, TaskType::DISABLE_CLOUD_TASK);
    auto nextAction = syncStateManager_.UpdateSyncState(SyncState::DISABLE_CLOUD_SUCCEED);
    if (nextAction != Action::STOP) {
         /* Retrigger sync, clear errorcode */
        if (!CloudStatus::IsCloudStatusOkay(bundleName_, userId_)) {
            LOGE("cloud status is not OK");
            return;
        }
        StartSync(false, SyncTriggerType::PENDING_TRIGGER);
    }
}

void DataSyncer::SyncStateChangedNotify(const CloudSyncState state, const ErrorType error)
{
    CurrentSyncState_ = state;
    CurrentErrorType_ = error;
    CloudSyncCallbackManager::GetInstance().NotifySyncStateChanged(bundleName_, state, error);
}

void DataSyncer::NotifyCurrentSyncState()
{
    CloudSyncCallbackManager::GetInstance().NotifySyncStateChanged(bundleName_, CurrentSyncState_, CurrentErrorType_);
}

void DataSyncer::UpdateErrorCode(int32_t code)
{
    switch (code) {
        case E_SYNC_FAILED_NETWORK_NOT_AVAILABLE:
            SetErrorCodeMask(ErrorType::NETWORK_UNAVAILABLE);
            break;
        case E_LOCAL_STORAGE_FULL:
            SetErrorCodeMask(ErrorType::LOCAL_STORAGE_FULL);
            break;
        case E_CLOUD_STORAGE_FULL:
            SetErrorCodeMask(ErrorType::CLOUD_STORAGE_FULL);
            break;
        case E_SYNC_FAILED_BATTERY_LOW:
            SetErrorCodeMask(ErrorType::BATTERY_LEVEL_LOW);
            break;
        default:
            LOGI("ignore errcode, errcode: %{public}d", code);
            break;
    }
}

void DataSyncer::SetErrorCodeMask(ErrorType errorType)
{
    errorCode_ |= 1 << static_cast<uint32_t>(errorType);
}

ErrorType DataSyncer::GetErrorType()
{
    if (errorCode_ == E_OK) {
        return ErrorType::NO_ERROR;
    }

    std::vector<ErrorType> errorTypes = {NETWORK_UNAVAILABLE, WIFI_UNAVAILABLE,   BATTERY_LEVEL_WARNING,
                                         BATTERY_LEVEL_LOW,   CLOUD_STORAGE_FULL, LOCAL_STORAGE_FULL};
    for (const auto &errorType : errorTypes) {
        if (errorCode_ & (1 << static_cast<uint32_t>(errorType))) {
            return errorType;
        }
    }
    LOGE("errorcode unexpected, errcode: %{public}u", errorCode_);
    return ErrorType::NO_ERROR;
}

void DataSyncer::SaveSubscription()
{
    sdkHelper_->SaveSubscription([] (auto, shared_ptr<DriveKit::DKContainer>,
        DriveKit::DKSubscriptionResult & res) {
        if (!res.IsSuccess()) {
            auto err = res.GetDKError();
            LOGE("drivekit save subscription server err %{public}d and dk errcor %{public}d", err.serverErrorCode,
                err.dkErrorCode);
        }
    });
}

void DataSyncer::DeleteSubscription()
{
    if (!HasSdkHelper()) {
        LOGW(" sdk helper is null skip delete subscription");
        return;
    }

    sdkHelper_->DeleteSubscription([] (auto, DriveKit::DKError err) {
        if (err.HasError()) {
            LOGE("drivekit delete subscription server err %{public}d and dk errcor %{public}d", err.serverErrorCode,
                err.dkErrorCode);
        }
    });
}

int32_t DataSyncer::OptimizeStorage(const int32_t agingDays)
{
    return E_OK;
}

bool DataSyncer::HasSdkHelper()
{
    if (sdkHelper_ == nullptr) {
        return false;
    }
    return true;
}

// download the thumb and lcd of file when screenoff and charging
int32_t DataSyncer::DownloadThumb(int32_t type)
{
    return E_OK;
}

int32_t DataSyncer::DownloadThumbInner(std::shared_ptr<DataHandler> handler)
{
    if (syncStateManager_.GetSyncState() == SyncState::CLEANING ||
        syncStateManager_.GetSyncState() == SyncState::DISABLE_CLOUD) {
        LOGI("downloading or cleaning, not to trigger thumb downloading");
        return E_STOP;
    }

    if (!TaskStateManager::GetInstance().HasTask(bundleName_, TaskType::DOWNLOAD_THUMB_TASK)) {
        LOGI("stop download thumb");
        return E_STOP;
    }
    vector<DriveKit::DKDownloadAsset> assetsToDownload;
    int32_t ret = handler->GetThumbToDownload(assetsToDownload);
    if (ret != E_OK) {
        LOGE("get assetsToDownload err %{public}d", ret);
        return E_STOP;
    }
    if (assetsToDownload.empty()) {
        return E_STOP;
    }
    LOGI("assetsToDownload count: %{public}zu", assetsToDownload.size());
    auto ctx = std::make_shared<TaskContext>(handler);
    auto callback = [this] (shared_ptr<DKContext> context,
                            shared_ptr<const DKDatabase> database,
                            const map<DKDownloadAsset, DKDownloadResult> &resultMap,
                            const DKError &err) {
        DataSyncer::FetchThumbDownloadCallback(context, database, resultMap, err);
    };
    DownloadContext dctx = {.context = ctx,
                            .assets = assetsToDownload,
                            .id = 0,
                            .resultCallback = callback,
                            .progressCallback = FetchRecordsDownloadProgress};
    std::thread([this, dctx]() { this->DownloadThumbAssets(dctx); }).detach();
    return E_OK;
}

void DataSyncer::FetchThumbDownloadCallback(shared_ptr<DKContext> context,
                                            shared_ptr<const DKDatabase> database,
                                            const map<DKDownloadAsset, DKDownloadResult> &resultMap,
                                            const DKError &err)
{
    auto ctx = static_pointer_cast<TaskContext>(context);
    auto handler = ctx->GetHandler();
    handler->OnDownloadAssets(resultMap);
    if (err.HasError()) {
        LOGE("DKAssetsDownloader err, localErr: %{public}d, serverErr: %{public}d", static_cast<int>(err.dkErrorCode),
             err.serverErrorCode);
        StopDownloadThumb();
        return;
    }
    LOGI("DKAssetsDownloader ok");
    if (handler->GetDownloadType() == DataHandler::DownloadThmType::SCREENOFF_TRIGGER) {
        if (!CheckScreenAndWifi()) {
            LOGI("download thumb condition is not met");
            StopDownloadThumb();
            return;
        }
    }
    if (DownloadThumbInner(handler) == E_STOP) {
        StopDownloadThumb();
    }
}

bool DataSyncer::CheckScreenAndWifi()
{
    if (NetworkStatus::GetNetConnStatus() == NetworkStatus::WIFI_CONNECT
        && !ScreenStatus::IsScreenOn()) {
        return true;
    }
    return false;
}

int32_t DataSyncer::CleanCache(const string &uri)
{
    return E_OK;
}

void DataSyncer::StopDownloadThumb()
{
    return;
}

void DataSyncer::StopUploadAssets()
{
    sdkHelper_->Release();
}

int32_t DataSyncer::InitSysEventData()
{
    return E_OK;
}

void DataSyncer::FreeSysEventData()
{
}

void DataSyncer::ReportSysEvent(uint32_t code)
{
}

void DataSyncer::SetFullSyncSysEvent()
{
}

void DataSyncer::SetCheckSysEvent()
{
}

void DataSyncer::RemoveCycleTaskFile()
{
    string fullPath = CLOUDFILE_DIR + to_string(userId_) + "/" + bundleName_ + "/CycleTask";
    int32_t ret = unlink(fullPath.c_str());
    if (ret != E_OK) {
        LOGI("unlink cycle Task file fail, errno is %{public}d", errno);
    }
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
