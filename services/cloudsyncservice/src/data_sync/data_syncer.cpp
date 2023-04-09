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
#include "utils_log.h"
#include "sdk_helper.h"
#include "ipc/cloud_sync_callback_manager.h"
#include "sync_rule/battery_status.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
using namespace std;
using namespace placeholders;
using namespace DriveKit;

DataSyncer::DataSyncer(const std::string bundleName, const int32_t userId)
    : bundleName_(bundleName), userId_(userId), sdkHelper_(userId, bundleName)
{
    taskManager_ = make_shared<TaskManager>(bind(&DataSyncer::Schedule, this));
    if (taskManager_ == nullptr) {
        LOGE("init task manager err");
    }
}

int32_t DataSyncer::AsyncRun(std::shared_ptr<TaskContext> context,
    void(DataSyncer::*f)(std::shared_ptr<TaskContext>))
{
    shared_ptr<Task> task = make_shared<Task>(context,
        [this, f](shared_ptr<TaskContext> ctx) {
            (this->*f)(ctx);
        }
    );
    if (task == nullptr) {
        LOGE("async run alloc task fail");
        return E_MEMORY;
    }

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
    if (task == nullptr) {
        LOGE("async callback alloc task err");
        return nullptr;
    }

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

int32_t DataSyncer::Pull(shared_ptr<DataHandler> handler)
{
    LOGI("%{private}d %{private}s pull", userId_, bundleName_.c_str());

    shared_ptr<TaskContext> context = make_shared<TaskContext>(handler);
    if (context == nullptr) {
        LOGE("pull alloc context fail");
        return E_MEMORY;
    }

    int32_t ret = AsyncRun(context, &DataSyncer::PullRecords);
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
    int32_t ret = sdkHelper_.FetchRecords(context, callback);
    if (ret != E_OK) {
        LOGE("sdk fetch records err %{public}d", ret);
    }
}

void DataSyncer::OnFetchRecords(const shared_ptr<DKContext> context,
    const shared_ptr<vector<DKRecord>> records)
{
    LOGI("%{private}d %{private}s on fetch records", userId_, bundleName_.c_str());

    auto ctx = static_pointer_cast<TaskContext>(context);

    /* no more records */
    if (records->size() == 0) {
        return;
    }

    /* update local */
    auto handler = ctx->GetHandler();
    if (handler == nullptr) {
        LOGE("context get handler err");
        return;
    }
    int32_t ret = handler->OnFetchRecords(*records);
    if (ret != E_OK) {
        LOGE("handler on fetch records err %{public}d", ret);
        return;
    }

    /* pull more */
    ret = AsyncRun(ctx, &DataSyncer::PullRecords);
    if (ret != E_OK) {
        LOGE("asyn run pull records err %{public}d", ret);
        return;
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
    if (context == nullptr) {
        LOGE("alloc context fail");
        return E_MEMORY;
    }

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

    ret = AsyncRun(context, &DataSyncer::ModifyRecords);
    if (ret != E_OK) {
        LOGE("async run modify records err %{public}d", ret);
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
    ret = sdkHelper_.CreateRecords(context, records, callback);
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
    ret = sdkHelper_.DeleteRecords(context, records, callback);
    if (ret != E_OK) {
        LOGE("sdk delete records err %{public}d", ret);
    }
}

void DataSyncer::ModifyRecords(shared_ptr<TaskContext> context)
{
    LOGI("%{private}d %{private}s modifies records", userId_, bundleName_.c_str());

    auto handler = context->GetHandler();
    if (handler == nullptr) {
        LOGE("context get handler err");
        return;
    }

    /* query local */
    vector<DKRecord> records;
    int32_t ret = handler->GetModifiedRecords(records);
    if (ret != E_OK) {
        LOGE("handler get modified records err %{public}d", ret);
        return;
    }

    /* no need upload */
    if (records.size() == 0) {
        return;
    }

    /* upload */
    auto callback = AsyncCallback(&DataSyncer::OnModifyRecords);
    if (callback == nullptr) {
        LOGE("wrap on modify records fail");
        return;
    }
    ret = sdkHelper_.ModifyRecords(context, records, callback);
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
    LOGI("%{private}d %{private}s on create records %{public}zu", userId_,
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

void DataSyncer::OnModifyRecords(shared_ptr<DKContext> context,
    shared_ptr<const DKDatabase> database,
    shared_ptr<const map<DKRecordId, DKRecordOperResult>> map, const DKError &err)
{
    LOGI("%{private}d %{private}s on create records %{public}zu", userId_,
        bundleName_.c_str(), map->size());

    auto ctx = static_pointer_cast<TaskContext>(context);

    /* update local */
    auto handler = ctx->GetHandler();
    int32_t ret = handler->OnModifyRecords(*map);
    if (ret != E_OK) {
        LOGE("handler on modify records err %{public}d", ret);
        return;
    }

    /* push more */
    ret = AsyncRun(ctx, &DataSyncer::ModifyRecords);
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
        CloudSyncCallbackManager::GetInstance().NotifySyncStateChanged(
            bundleName_, userId_, SyncType::ALL, state);
    } else {
        CloudSyncCallbackManager::GetInstance().NotifySyncStateChanged(
            bundleName_, userId_, type, state);
    }
}

void DataSyncer::SyncStateChangedNotify(const SyncType type, const SyncPromptState state)
{
    CloudSyncCallbackManager::GetInstance().NotifySyncStateChanged(bundleName_,
        userId_, SyncType::ALL, state);
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
