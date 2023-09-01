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

#include <list>
#include <memory>
#include <string>
#include <vector>

#include "cloud_pref_impl.h"
#include "cloud_sync_constants.h"
#include "data_handler.h"
#include "data_sync/sync_state_manager.h"
#include "ipc/cloud_download_callback_manager.h"
#include "i_cloud_download_callback.h"
#include "sdk_helper.h"
#include "task.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
enum class SyncTriggerType : int32_t {
    APP_TRIGGER,
    CLOUD_TRIGGER,
    PENDING_TRIGGER,
    BATTERY_OK_TRIGGER,
    NETWORK_AVAIL_TRIGGER,
};
struct DownloadContext;

class DataSyncer {
public:
    DataSyncer(const std::string bundleName, const int32_t userId);
    virtual ~DataSyncer();

    /* sync */
    virtual int32_t StartSync(bool forceFlag, SyncTriggerType triggerType);
    virtual int32_t StopSync(SyncTriggerType triggerType);
    virtual int32_t StartDownloadFile(const std::string path, const int32_t userId);
    virtual int32_t StopDownloadFile(const std::string path, const int32_t userId);
    int32_t RegisterDownloadFileCallback(const int32_t userId,
                                         const sptr<ICloudDownloadCallback> downloadCallback);
    int32_t UnregisterDownloadFileCallback(const int32_t userId);
    /* properties */
    std::string GetBundleName() const;
    int32_t GetUserId() const;
    SyncState GetSyncState() const;

    /*clean*/
    virtual int32_t Init(const std::string bundleName, const int32_t userId);
    virtual int32_t Clean(const int action);

    /* sdk */
    void SetSdkHelper(std::shared_ptr<SdkHelper> &sdkHelper);
    int32_t Lock();
    void Unlock();
    void ForceUnlock();

    void NotifyCurrentSyncState();

protected:
    /* download */
    int32_t Pull(std::shared_ptr<DataHandler> handler);

    /* upload */
    int32_t Push(std::shared_ptr<DataHandler> handler);

    /*clean cloud files*/
    int32_t CleanInner(std::shared_ptr<DataHandler> handler, const int action);

    void ClearCursor();

    void SaveSubscription();
    void DeleteSubscription();
    /* schedule */
    virtual void Schedule() = 0;
    virtual void Reset() = 0;
    void Abort();

    /* download source file */
    int32_t DownloadInner(std::shared_ptr<DataHandler> handler,
                          const std::string path,
                          const int32_t userId);

    /* notify */
    int32_t CompletePull();
    int32_t CompletePush();
    void CompleteAll(bool isNeedNotify = true);

    void BeginClean();
    void CompleteClean();

    void SyncStateChangedNotify(const CloudSyncState state, const ErrorType error);

    /* identifier */
    const std::string bundleName_;
    const int32_t userId_;

private:
    /* download */
    void PullRecords(std::shared_ptr<TaskContext> context);
    void PullDatabaseChanges(std::shared_ptr<TaskContext> context);
    void PullRecordsWithId(std::shared_ptr<TaskContext> context, const std::vector<DriveKit::DKRecordId> &records,
        bool retry);
    void DownloadAssets(DownloadContext &ctx);
    void RetryDownloadRecords(std::shared_ptr<TaskContext> context);
    /* dowload callback */
    void OnFetchRecords(const std::shared_ptr<DriveKit::DKContext>, std::shared_ptr<const DriveKit::DKDatabase>,
        std::shared_ptr<std::vector<DriveKit::DKRecord>>, DriveKit::DKQueryCursor,
        const DriveKit::DKError &);
    void OnFetchCheckRecords(const std::shared_ptr<DriveKit::DKContext>, std::shared_ptr<const DriveKit::DKDatabase>,
        std::shared_ptr<std::vector<DriveKit::DKRecord>>, DriveKit::DKQueryCursor,
        const DriveKit::DKError &);
    void OnFetchRecordWithId(std::shared_ptr<DriveKit::DKContext>, std::shared_ptr<DriveKit::DKDatabase>,
        DriveKit::DKRecordId, const DriveKit::DKRecord &, const DriveKit::DKError &);
    int HandleOnFetchRecords(const std::shared_ptr<DownloadTaskContext> context,
        std::shared_ptr<const DriveKit::DKDatabase> database,
        std::shared_ptr<std::vector<DriveKit::DKRecord>> records, bool checkOrRetry);
    void OnFetchDatabaseChanges(const std::shared_ptr<DriveKit::DKContext>,
        std::shared_ptr<const DriveKit::DKDatabase>,
        std::shared_ptr<std::vector<DriveKit::DKRecord>>, DriveKit::DKQueryCursor,
        bool, const DriveKit::DKError &);
    void FetchRecordsDownloadCallback(std::shared_ptr<DriveKit::DKContext> context,
                                      std::shared_ptr<const DriveKit::DKDatabase> database,
                                      const std::map<DriveKit::DKDownloadAsset, DriveKit::DKDownloadResult> &resultMap,
                                      const DriveKit::DKError &err);

    /* upload */
    void CreateRecords(std::shared_ptr<TaskContext> context);
    void DeleteRecords(std::shared_ptr<TaskContext> context);
    void ModifyMdirtyRecords(std::shared_ptr<TaskContext> context);
    void ModifyFdirtyRecords(std::shared_ptr<TaskContext> context);

    /* upload callback */
    void OnCreateRecords(std::shared_ptr<DriveKit::DKContext>,
        std::shared_ptr<const DriveKit::DKDatabase>,
        std::shared_ptr<const std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult>>,
        const DriveKit::DKError &);
    void OnDeleteRecords(std::shared_ptr<DriveKit::DKContext>,
        std::shared_ptr<const DriveKit::DKDatabase>,
        std::shared_ptr<const std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult>>,
        const DriveKit::DKError &);
    void OnModifyMdirtyRecords(std::shared_ptr<DriveKit::DKContext>,
        std::shared_ptr<const DriveKit::DKDatabase>,
        std::shared_ptr<const std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult>>,
        std::shared_ptr<const std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult>>,
        const DriveKit::DKError &);
    void OnModifyFdirtyRecords(std::shared_ptr<DriveKit::DKContext>,
        std::shared_ptr<const DriveKit::DKDatabase>,
        std::shared_ptr<const std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult>>,
        std::shared_ptr<const std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult>>,
        const DriveKit::DKError &);

    /* transaction */
    void BeginTransaction();
    void EndTransaction();

    /* async task wrapper */
    int32_t AsyncRun(std::shared_ptr<TaskContext> context,
        void(DataSyncer::*f)(std::shared_ptr<TaskContext>));
    template<typename T, typename RET, typename... ARGS>
    std::function<RET(ARGS...)> AsyncCallback(RET(T::*f)(ARGS...));

    void UpdateErrorCode(int32_t code);
    void SetErrorCodeMask(ErrorType errorType);
    /* cloud sync result */
    ErrorType GetErrorType();

    /* state management */
    SyncStateManager syncStateManager_;

    /* task management */
    std::shared_ptr<TaskRunner> taskRunner_;

    /* sdk */
    std::shared_ptr<SdkHelper> sdkHelper_;
    SdkLock lock_;

    uint32_t errorCode_{0};

    /* download callback manager*/
    CloudDownloadCallbackManager downloadCallbackMgr_;

    /* Current sync state */
    CloudSyncState CurrentSyncState_{CloudSyncState::COMPLETED};

    /* Current error type */
    ErrorType CurrentErrorType_{ErrorType::NO_ERROR};
};
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
#endif // OHOS_CLOUD_SYNC_SERVICE_DATA_SYNCER_H
