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
#include "sysevent.h"
#include "task.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
struct DownloadContext;

class DataSyncer {
public:
    enum CleanAction {
        RETAIN_DATA = 0,
        CLEAR_DATA
    };
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
    virtual int32_t CleanCache(const std::string &uri);
    /* properties */
    std::string GetBundleName() const;
    int32_t GetUserId() const;
    SyncState GetSyncState() const;

    /*clean*/
    virtual int32_t Init(const std::string bundleName, const int32_t userId);
    virtual int32_t Clean(const int action);
    virtual int32_t DisableCloud();

    /* sdk */
    bool HasSdkHelper();
    void SetSdkHelper(std::shared_ptr<SdkHelper> &sdkHelper);
    virtual int32_t Lock();
    virtual void Unlock();
    virtual void ForceUnlock();

    void NotifyCurrentSyncState();

    /* optimizestorage */
    virtual int32_t OptimizeStorage(const int32_t agingDays);
    virtual int32_t DownloadThumb(const int32_t type);
    virtual void StopDownloadThumb();

    void SaveSubscription();
    void DeleteSubscription();
    void UpdateErrorCode(int32_t code);
    /* cloud sync result */
    ErrorType GetErrorType();

protected:
    /* download */
    int32_t Pull(std::shared_ptr<DataHandler> handler);

    /* upload */
    int32_t Push(std::shared_ptr<DataHandler> handler);

    /*clean cloud files*/
    int32_t CleanInner(std::shared_ptr<DataHandler> handler, const int action);
    int32_t CancelDownload(std::shared_ptr<DataHandler> handler);

    int32_t DownloadThumbInner(std::shared_ptr<DataHandler> handler);
    void ClearCursor();
    void RemoveCycleTaskFile();

    /* schedule */
    virtual void Schedule() = 0;
    virtual void ScheduleByType(SyncTriggerType syncTriggerType) = 0;
    virtual void Reset() = 0;
    virtual int32_t Complete(bool isNeedNotify = true) = 0;
    void Abort();

    /* download source file */
    int32_t DownloadInner(std::shared_ptr<DataHandler> handler,
                          const std::string path,
                          const int32_t userId);

    /* notify */
    virtual int32_t CompletePull();
    int32_t CompletePush();
    void CompleteAll(bool isNeedNotify = true);

    void BeginClean();
    void CompleteClean();

    void BeginDisableCloud();
    void CompleteDisableCloud();

    void SyncStateChangedNotify(const CloudSyncState state, const ErrorType error);
    void SetErrorCodeMask(ErrorType errorType);

    /* sync rule to be supplemented and imporved */
    bool CheckScreenAndWifi();

    /* sys event */
    virtual int32_t InitSysEventData();
    virtual void FreeSysEventData();
    virtual void ReportSysEvent(uint32_t code);
    virtual void SetFullSyncSysEvent();
    virtual void SetCheckSysEvent();

    /* identifier */
    const std::string bundleName_;
    const int32_t userId_;

    /* sdk */
    std::shared_ptr<SdkHelper> sdkHelper_;
    SdkLock lock_;

    uint32_t errorCode_{0};

    bool isDataChanged_{false};

    uint64_t startTime_{0};
    SyncTriggerType triggerType_;
    std::shared_ptr<bool> stopFlag_ = std::make_shared<bool>(false);

private:
    /* download */
    void PullRecords(std::shared_ptr<TaskContext> context);
    void PullDatabaseChanges(std::shared_ptr<TaskContext> context);
    void PullRecordsWithId(std::shared_ptr<TaskContext> context, const std::vector<DriveKit::DKRecordId> &records,
        bool retry);
    void DownloadAssets(DownloadContext &ctx);
    void DownloadThumbAssets(DownloadContext ctx);
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
    void OnFetchRecordWithIds(std::shared_ptr<DriveKit::DKContext> context,
        std::shared_ptr<DriveKit::DKDatabase> database, std::shared_ptr<std::map<DriveKit::DKRecordId,
        DriveKit::DKRecordOperResult>> recordMap, const DriveKit::DKError &error);
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
    void FetchThumbDownloadCallback(std::shared_ptr<DriveKit::DKContext> context,
                                      std::shared_ptr<const DriveKit::DKDatabase> database,
                                      const std::map<DriveKit::DKDownloadAsset, DriveKit::DKDownloadResult> &resultMap,
                                      const DriveKit::DKError &err);

    /* upload */
    void CreateRecords(std::shared_ptr<TaskContext> context);
    void DeleteRecords(std::shared_ptr<TaskContext> context);
    void ModifyMdirtyRecords(std::shared_ptr<TaskContext> context);
    void ModifyFdirtyRecords(std::shared_ptr<TaskContext> context);
    void StopUploadAssets();

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
    bool forceFlag_ = false;
    /* state management */
    SyncStateManager syncStateManager_;

    /* task management */
    std::shared_ptr<TaskRunner> taskRunner_;

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
