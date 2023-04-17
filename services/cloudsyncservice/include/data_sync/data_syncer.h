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

#ifndef OHOS_CLOUD_SYNC_SERVICE_DATA_SYNCER_H
#define OHOS_CLOUD_SYNC_SERVICE_DATA_SYNCER_H

#include <string>
#include <vector>
#include <list>
#include <memory>

#include "sdk_helper.h"
#include "data_handler.h"
#include "task.h"
#include "cloud_sync_constants.h"
#include "data_sync/sync_state_manager.h"
#include "cloud_pref_impl.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
enum class SyncTriggerType : int32_t {
    APP_TRIGGER,
    CLOUD_TRIGGER,
    PENDING_TRIGGER,
    BATTERY_OK_TRIGGER,
};

class DataSyncer {
public:
    DataSyncer(const std::string bundleName, const int32_t userId);
    virtual ~DataSyncer() = default;

    /* sync */
    virtual int32_t StartSync(bool forceFlag, SyncTriggerType triggerType);
    virtual int32_t StopSync(SyncTriggerType triggerType);

    /* properties */
    std::string GetBundleName() const;
    int32_t GetUserId() const;
    SyncState GetSyncState() const;

protected:
    /* download */
    int32_t Pull(std::shared_ptr<DataHandler> handler);

    /* upload */
    int32_t Push(std::shared_ptr<DataHandler> handler);

    /* schedule */
    virtual void Schedule() = 0;
    void Abort();

    /* notify */
    void CompletePull();
    void CompletePush();
    void CompleteAll(int32_t code, const SyncType type);

    void SyncStateChangedNotify(const SyncType type, const SyncPromptState state);

private:
    /* download */
    void PullRecords(std::shared_ptr<TaskContext> context);
    void PullDatabaseChanges(std::shared_ptr<TaskContext> context);
    /* dowload callback */
    void OnFetchRecords(const std::shared_ptr<DriveKit::DKContext>, std::shared_ptr<const DriveKit::DKDatabase>,
        std::shared_ptr<const std::map<DriveKit::DKRecordId, DriveKit::DKRecord>>, DriveKit::DKQueryCursor,
        const DriveKit::DKError &);

    void OnFetchDatabaseChanges(const std::shared_ptr<DriveKit::DKContext>,
        std::shared_ptr<const DriveKit::DKDatabase>,
        std::shared_ptr<const std::map<DriveKit::DKRecordId, DriveKit::DKRecord>>, DriveKit::DKQueryCursor,
        bool, const DriveKit::DKError &);

    /* upload */
    void CreateRecords(std::shared_ptr<TaskContext> context);
    void DeleteRecords(std::shared_ptr<TaskContext> context);
    void ModifyRecords(std::shared_ptr<TaskContext> context);

    /* upload callback */
    void OnCreateRecords(std::shared_ptr<DriveKit::DKContext>,
        std::shared_ptr<const DriveKit::DKDatabase>,
        std::shared_ptr<const std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult>>,
        const DriveKit::DKError &);
    void OnDeleteRecords(std::shared_ptr<DriveKit::DKContext>,
        std::shared_ptr<const DriveKit::DKDatabase>,
        std::shared_ptr<const std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult>>,
        const DriveKit::DKError &);
    void OnModifyRecords(std::shared_ptr<DriveKit::DKContext>,
        std::shared_ptr<const DriveKit::DKDatabase>,
        std::shared_ptr<const std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult>>,
        const DriveKit::DKError &);

    /* task */
    int32_t CommitTask(std::shared_ptr<Task> t);
    int32_t AddTask(std::shared_ptr<Task> t);
    int32_t StartTask(std::shared_ptr<Task> t, TaskAction action);
    void CompleteTask(std::shared_ptr<Task> t);

    void BeginTransaction();
    void EndTransaction();

    /* async task wrapper */
    int32_t AsyncRun(std::shared_ptr<TaskContext> context,
        void(DataSyncer::*f)(std::shared_ptr<TaskContext>));
    template<typename T, typename RET, typename... ARGS>
    std::function<RET(ARGS...)> AsyncCallback(RET(T::*f)(ARGS...));

    /* prompt state */
    SyncPromptState GetSyncPromptState(const int32_t code);

    /* identifier */
    const std::string bundleName_;
    const int32_t userId_;

    /* state management */
    SyncStateManager syncStateManager_;

    /* task management */
    std::shared_ptr<TaskManager> taskManager_;

    /* sdk */
    SdkHelper sdkHelper_;

    int32_t errorCode_{0};

    /* cursor*/
    DriveKit::DKQueryCursor startCursor_;
    DriveKit::DKQueryCursor nextCursor_;

    /* cloud preference impl */
    CloudPrefImpl cloudPrefImpl_;
};
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
#endif // OHOS_CLOUD_SYNC_SERVICE_DATA_SYNCER_H
