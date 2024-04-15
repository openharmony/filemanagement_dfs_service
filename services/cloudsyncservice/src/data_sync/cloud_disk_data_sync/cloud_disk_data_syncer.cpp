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

#include "cloud_disk_data_syncer.h"
#include "clouddisk_rdbstore.h"
#include "dfs_error.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
using namespace std;
using namespace OHOS::FileManagement::CloudDisk;
CloudDiskDataSyncer::CloudDiskDataSyncer(const std::string bundleName, const int32_t userId)
    : DataSyncer(bundleName, userId) {}

int32_t CloudDiskDataSyncer::Init(const std::string bundleName, const int32_t userId)
{
    CloudDisk::CloudDiskRdbStore cloudDiskRdbStore(bundleName_, userId_);
    auto rdb = cloudDiskRdbStore.GetRaw();
    if (!rdb) {
        LOGE("clouddisk rdb init fail");
        return E_RDB;
    }
    /* init handler */
    cloudDiskHandler_ = make_shared<CloudDiskDataHandler>(userId_, bundleName_, rdb, stopFlag_);
    return E_OK;
}

int32_t CloudDiskDataSyncer::Clean(const int action)
{
    LOGD("cloud disk data sycner Clean");
    BeginClean();
    int32_t ret = CancelDownload(cloudDiskHandler_);
    if (ret != E_OK) {
        LOGE("CloudDisk data syncer file cancel download err %{public}d", ret);
    }
    ret = CleanInner(cloudDiskHandler_, action);
    if (ret != E_OK) {
        LOGE("disk data syncer file clean err %{public}d", ret);
    }
    RemoveCycleTaskFile();
    CompleteClean();
    return ret;
}

int32_t CloudDiskDataSyncer::StartDownloadFile(const std::string path, const int32_t userId)
{
    return DownloadInner(cloudDiskHandler_, path, userId);
}

int32_t CloudDiskDataSyncer::StopDownloadFile(const std::string path, const int32_t userId)
{
    return DataSyncer::StopDownloadFile(path, userId);
}

int32_t CloudDiskDataSyncer::CleanCache(const string &uri)
{
    return cloudDiskHandler_->CleanCache(uri);
}

void CloudDiskDataSyncer::Schedule()
{
    LOGI("schedule to stage %{public}d", ++stage_);
    int32_t ret = E_OK;
    switch (stage_) {
        case DOWNLOADFILE: {
            ret = DownloadFile();
            break;
        }
        case COMPLETEPULL: {
            ret = CompletePull();
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

void CloudDiskDataSyncer::Reset()
{
    stage_ = BEGIN;
    cloudDiskHandler_->Reset();
}

int32_t CloudDiskDataSyncer::DownloadFile()
{
    LOGI("cloud disk data sycner download file");
    int ret = Pull(cloudDiskHandler_);
    if (ret != E_OK) {
        LOGE("disk data syncer pull file err %{public}d", ret);
    }
    return ret;
}

int32_t CloudDiskDataSyncer::UploadFile()
{
    LOGI("cloud disk data sycner upload file");
    int32_t ret = Push(cloudDiskHandler_);
    if (ret != E_OK) {
        LOGE("disk data syncer push file err %{public}d", ret);
    }
    return ret;
}

int32_t CloudDiskDataSyncer::Complete(bool isNeedNotify)
{
    LOGI("cloud disk data syncer complete all");
    Unlock();
    CompleteAll(isNeedNotify);
    if (isDataChanged_) {
        ChangesNotify();
        isDataChanged_ = false;
    }
    return E_OK;
}

void CloudDiskDataSyncer::ChangesNotify()
{
    sdkHelper_->ChangesNotify([] (auto, DriveKit::DKError err) {
        if (err.HasError()) {
            LOGE("drivekit changes notify server err %{public}d and dk error %{public}d", err.serverErrorCode,
                err.dkErrorCode);
        }
    });
}

void CloudDiskDataSyncer::ScheduleByType(SyncTriggerType syncTriggerType)
{
    Schedule();
    return;
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS