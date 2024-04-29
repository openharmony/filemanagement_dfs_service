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

#include "gallery_data_syncer.h"

#include "data_syncer.h"
#include "dfs_error.h"
#include "dfsu_timer.h"
#include "medialibrary_rdb_utils.h"
#include "parameters.h"
#include "sync_rule/network_status.h"
#include "sync_rule/screen_status.h"
#include "task_state_manager.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
using namespace std;
using ChangeType = AAFwk::ChangeInfo::ChangeType;
const std::string MEDIA_LIBRARY_STARTUP_PARAM_PREFIX = "multimedia.medialibrary.startup.";

const std::string CloudSyncTriggerFunc(const std::vector<std::string> &args)
{
    return "";
}

const std::string IsCallerSelfFunc(const std::vector<std::string> &args)
{
    return "false";
}

GalleryDataSyncer::GalleryDataSyncer(const std::string bundleName, const int32_t userId)
    : DataSyncer(bundleName, userId)
{
}

int32_t GalleryDataSyncer::Init(const std::string bundleName, const int32_t userId)
{
    auto rdb = RdbInit(bundleName_, userId_);
    if (!rdb) {
        return E_RDB;
    }
    /* init handler */
    fileHandler_ = make_shared<FileDataHandler>(userId_, bundleName_, rdb, stopFlag_);
    albumHandler_ = make_shared<AlbumDataHandler>(userId_, bundleName_, rdb, stopFlag_);
    return E_OK;
}

std::shared_ptr<NativeRdb::RdbStore> GalleryDataSyncer::RdbInit(const std::string &bundleName, const int32_t userId)
{
    std::string startupParam = MEDIA_LIBRARY_STARTUP_PARAM_PREFIX + to_string(userId);
    auto rdbInitFlag = system::GetBoolParameter(startupParam, false);
    if (!rdbInitFlag) {
        LOGE("media library db upgrade not completed, startupParam:%{public}s", startupParam.c_str());
        return nullptr;
    }
    LOGI("media library db upgrade completed");
    /* rdb config */
    NativeRdb::RdbStoreConfig config(DATABASE_NAME);
    config.SetPath(DATA_APP_EL2 + to_string(userId) + DATABASE_DIR + DATABASE_NAME);
    config.SetBundleName(BUNDLE_NAME);
    config.SetReadConSize(CONNECT_SIZE);
    config.SetSecurityLevel(NativeRdb::SecurityLevel::S3);
    config.SetScalarFunction("cloud_sync_func", 0, CloudSyncTriggerFunc);
    config.SetScalarFunction("is_caller_self_func", 0, IsCallerSelfFunc);
    config.SetArea(EL2_PATH);

    /*
     * Just pass in any value but zero for parameter @version in GetRdbStore,
     * since RdbCallback always return 0 in its callbacks.
     */
    int32_t err;
    RdbCallback cb;
    auto rdb = NativeRdb::RdbHelper::GetRdbStore(config, Media::MEDIA_RDB_VERSION, cb, err);
    if (rdb == nullptr) {
        LOGE("gallyer data syncer init rdb fail");
    }

    return rdb;
}

int32_t GalleryDataSyncer::Clean(const int action)
{
    LOGI("gallery data sycner Clean");
    /* start clean */
    BeginClean();
    /* file */
    int32_t ret = CancelDownload(fileHandler_);
    if (ret != E_OK) {
        LOGE("gallery data syncer file cancel download err %{public}d", ret);
    }
    ret = fileHandler_->MarkClean(action);
    if (ret != E_OK) {
        LOGE("gallery photos clean err %{public}d", ret);
    }
    ret = CleanInner(albumHandler_, CleanAction::RETAIN_DATA);
    if (ret != E_OK) {
        LOGE("gallery data syncer album clean err %{public}d", ret);
    }
    fileHandler_->ClearCursor();
    albumHandler_->ClearCursor();
    RemoveCycleTaskFile();
    CompleteClean();
    return ret;
}

int32_t GalleryDataSyncer::DisableCloud()
{
    LOGI("gallery data sycner disable");
    /* start clean */
    BeginDisableCloud();
    int32_t ret = CleanInner(fileHandler_, CleanAction::RETAIN_DATA);
    if (ret != E_OK) {
        LOGE("gallery data syncer file disable cloud err %{public}d", ret);
    }
    CompleteDisableCloud();
    return ret;
}

int32_t GalleryDataSyncer::StartDownloadFile(const std::string path, const int32_t userId)
{
    return DownloadInner(fileHandler_, path, userId);
}

void GalleryDataSyncer::Schedule()
{
    LOGI("schedule to stage %{public}d", ++stage_);

    int32_t ret = E_OK;
    switch (stage_) {
        case PREPARE: {
            ret = Prepare();
            break;
        }
        case DOWNLOADALBUM: {
            ret = DownloadAlbum();
            break;
        }
        case DOWNLOADFILE: {
            ret = DownloadFile();
            break;
        }
        case COMPLETEPULL: {
            ret = CompletePull();
            break;
        }
        case UPLOADALBUM: {
            ret = UploadAlbum();
            break;
        }
        case UPLOADFILE: {
            ret = UploadFile();
            break;
        }
        case COMPLETEPUSH: {
            ret = CompletePush();
            break;
        }
        case END: {
            ret = Complete();
            break;
        }
        default: {
            ret = E_SCHEDULE;
            LOGE("schedule fail %{public}d", ret);
            break;
        }
    }

    /* if error takes place while schedule, just abort it */
    if (ret != E_OK) {
        Abort();
    }
}

void GalleryDataSyncer::ScheduleByType(SyncTriggerType syncTriggerType)
{
    LOGI("schedule to stage %{public}d", ++stage_);
    /* call putHandler in the Reset function when sync is completed */
    if (syncTriggerType == SyncTriggerType::TASK_TRIGGER) {
        fileHandler_->SetChecking();
    }
    Schedule();
}

void GalleryDataSyncer::Reset()
{
    if (fileHandler_ != nullptr) {
        fileHandler_->Reset();
    }
    if (albumHandler_ != nullptr) {
        albumHandler_->Reset();
    }
    /* reset stage in case of stop or restart */
    stage_ = BEGIN;
}

int32_t GalleryDataSyncer::Prepare()
{
    /* schedule to next stage */
    Schedule();
    return E_OK;
}

int32_t GalleryDataSyncer::DownloadAlbum()
{
    SyncStateChangedNotify(CloudSyncState::DOWNLOADING, ErrorType::NO_ERROR);

    LOGI("gallery data sycner download album");
    int32_t ret = Pull(albumHandler_);
    if (ret != E_OK) {
        LOGE("gallery data syncer pull album err %{public}d", ret);
    }
    return ret;
}

int32_t GalleryDataSyncer::DownloadFile()
{
    LOGI("gallery data sycner download file");
    int ret = Pull(fileHandler_);
    if (ret != E_OK) {
        LOGE("gallery data syncer pull file err %{public}d", ret);
    }
    return ret;
}

int32_t GalleryDataSyncer::UploadAlbum()
{
    int32_t ret = Lock();
    if (ret != E_OK) {
        LOGE("gallery data syncer lock err %{public}d", ret);
        return E_CLOUD_SDK;
    }

    SyncStateChangedNotify(CloudSyncState::UPLOADING, ErrorType::NO_ERROR);

    LOGI("gallery data sycner upload album");
    ret = Push(albumHandler_);
    if (ret != E_OK) {
        LOGE("gallery data syncer push album err %{public}d", ret);
    }

    return ret;
}

int32_t GalleryDataSyncer::UploadFile()
{
    LOGI("gallery data sycner upload file");
    int32_t ret = Push(fileHandler_);
    if (ret != E_OK) {
        LOGE("gallery data syncer push file err %{public}d", ret);
    }
    return ret;
}

int32_t GalleryDataSyncer::Complete(bool isNeedNotify)
{
    LOGI("gallery data syncer complete all");
    Unlock();
    if (!TaskStateManager::GetInstance().HasTask(bundleName_, TaskType::DOWNLOAD_THUMB_TASK)) {
        fileHandler_->StopUpdataFiles();
    }
    int32_t ret = fileHandler_->CleanRemainRecord();
    if (ret != E_OK) {
        LOGW("clean remain record failed");
        return ret;
    }

    if (checkStat_ != nullptr) {
        fileHandler_->SetCheckReportStatus();
    }

    DataSyncer::CompleteAll(isNeedNotify);
    return E_OK;
}

int32_t GalleryDataSyncer::OptimizeStorage(const int32_t agingDays)
{
    LOGI("gallery data sycner FileAging");

    return fileHandler_->OptimizeStorage(agingDays);
}

int32_t GalleryDataSyncer::Lock()
{
    lock_guard<mutex> lock(lock_.mtx);
    if (lock_.count > 0) {
        lock_.count++;
        return E_OK;
    }

    /* lock: device-reentrant */
    int32_t ret = sdkHelper_->GetLock(lock_.lock);
    if (ret != E_OK) {
        LOGE("sdk helper get lock err %{public}d", ret);
        lock_.lock = {0};
        if (ret == E_SYNC_FAILED_NETWORK_NOT_AVAILABLE) {
            SetErrorCodeMask(ErrorType::NETWORK_UNAVAILABLE);
        }
        return ret;
    }

    auto timerCallback = [this]() {
        lock_guard<mutex> lock(lock_.mtx);
        if (lock_.lock.lockSessionId.empty()) {
            LOGE("session is unlocked, please lock first");
            return;
        }
        auto ret = sdkHelper_->GetLock(lock_.lock);
        if (ret != E_OK) {
            LOGE("sdk helper get lock err %{public}d", ret);
            return;
        }
        LOGD("timer trigger, lockSessionId:%{public}s", lock_.lock.lockSessionId.c_str());
    };

    const uint32_t KEEP_ALIVE_PERIOD_COEF = 3;
    uint32_t period = (uint32_t)(lock_.lock.lockInterval) * (uint32_t)(SECOND_TO_MILLISECOND) / KEEP_ALIVE_PERIOD_COEF;
    LOGD("period %{public}d", period);
    DfsuTimer::GetInstance().Register(timerCallback, lock_.timerId, period);

    lock_.count++;

    return ret;
}

void GalleryDataSyncer::Unlock()
{
    lock_guard<mutex> lock(lock_.mtx);
    lock_.count--;
    if (lock_.count > 0) {
        return;
    }

    DfsuTimer::GetInstance().Unregister(lock_.timerId);
    /* sdk unlock */
    sdkHelper_->DeleteLock(lock_.lock);
    /* reset sdk lock */
    lock_.lock = { 0 };
    lock_.timerId = 0;
}

void GalleryDataSyncer::ForceUnlock()
{
    lock_guard<mutex> lock(lock_.mtx);
    if (lock_.count == 0) {
        return;
    }
    DfsuTimer::GetInstance().Unregister(lock_.timerId);
    sdkHelper_->DeleteLock(lock_.lock);
    lock_.lock = { 0 };
    lock_.count = 0;
    lock_.timerId = 0;
}

int32_t GalleryDataSyncer::DownloadThumb(int32_t type)
{
    LOGI("Begin download thumbnails");
    if (TaskStateManager::GetInstance().HasTask(bundleName_, TaskType::DOWNLOAD_THUMB_TASK)) {
        LOGI("it's already downloading thumb");
        return E_STOP;
    }
    if (type == DataHandler::DownloadThmType::SCREENOFF_TRIGGER) {
        if (!CheckScreenAndWifi()) {
            LOGI("download thumb condition is not met");
            return E_STOP;
        }
    }
    TaskStateManager::GetInstance().StartTask(bundleName_, TaskType::DOWNLOAD_THUMB_TASK);
    fileHandler_->SetDownloadType(type);
    int32_t ret = DataSyncer::DownloadThumbInner(fileHandler_);
    if (ret == E_STOP) {
        StopDownloadThumb();
    }
    return ret;
}

void GalleryDataSyncer::StopDownloadThumb()
{
    TaskStateManager::GetInstance().CompleteTask(bundleName_, TaskType::DOWNLOAD_THUMB_TASK);
    if (!TaskStateManager::GetInstance().HasTask(bundleName_, TaskType::SYNC_TASK)) {
        fileHandler_->StopUpdataFiles();
    }
}

int32_t GalleryDataSyncer::InitSysEventData()
{
    /* bind sync data to handler */
    syncStat_ = make_shared<GalleryIncSyncStat>();
    fileHandler_->SetSyncStat(syncStat_);
    albumHandler_->SetSyncStat(syncStat_);

    return E_OK;
}

void GalleryDataSyncer::FreeSysEventData()
{
    if (syncStat_ != nullptr) {
        /* dec ref to sync data */
        fileHandler_->PutSyncStat();
        albumHandler_->PutSyncStat();
        syncStat_ = nullptr;
    }

    if (checkStat_ != nullptr) {
        fileHandler_->PutCheckStat();
        checkStat_ = nullptr;
    }
}

void GalleryDataSyncer::SetFullSyncSysEvent()
{
    if (syncStat_ != nullptr) {
        syncStat_->SetFullSync();
    }
}

void GalleryDataSyncer::SetCheckSysEvent()
{
    if (checkStat_ == nullptr) {
        /* bind check data to handler */
        checkStat_ = make_shared<GalleryCheckSatat>();
        fileHandler_->SetCheckStat(checkStat_);
    }
}

int32_t GalleryDataSyncer::CompletePull()
{
    fileHandler_->UpdateAlbumInternal();
    DataSyncNotifier::GetInstance().TryNotify(PHOTO_URI_PREFIX, ChangeType::INSERT, "");
    DataSyncNotifier::GetInstance().FinalNotify();
    return DataSyncer::CompletePull();
}

void GalleryDataSyncer::ReportSysEvent(uint32_t code)
{
    if (syncStat_ != nullptr) {
        if (syncStat_->IsFullSync()) {
            UpdateBasicEventStat(code);
            syncStat_->Report();
        } else {
            /* inc sync report */
        }
    }

    if (checkStat_ != nullptr) {
        checkStat_->Report();
    }
}

void GalleryDataSyncer::UpdateBasicEventStat(uint32_t code)
{
    syncStat_->SetSyncReason(static_cast<uint32_t>(triggerType_));
    syncStat_->SetStopReason(code);
    syncStat_->SetStartTime(startTime_);
    syncStat_->SetDuration(GetCurrentTimeStamp());
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
