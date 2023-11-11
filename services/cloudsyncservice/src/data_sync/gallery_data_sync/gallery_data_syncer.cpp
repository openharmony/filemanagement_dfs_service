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

#include "dfs_error.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
using namespace std;

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
    return E_OK;
}

std::shared_ptr<NativeRdb::RdbStore> GalleryDataSyncer::RdbInit(const std::string &bundleName, const int32_t userId)
{
    /* rdb config */
    NativeRdb::RdbStoreConfig config(DATABASE_NAME);
    config.SetPath(DATA_APP_EL2 + to_string(userId) + DATABASE_DIR + DATABASE_NAME);
    config.SetBundleName(BUNDLE_NAME);
    config.SetReadConSize(CONNECT_SIZE);
    config.SetSecurityLevel(NativeRdb::SecurityLevel::S3);
    config.SetScalarFunction("cloud_sync_func", 0, CloudSyncTriggerFunc);
    config.SetScalarFunction("is_caller_self_func", 0, IsCallerSelfFunc);

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

int32_t GalleryDataSyncer::GetHandler()
{
    lock_guard<mutex> lock{handleInitMutex_};
    if (!fileHandler_) {
        auto rdb = RdbInit(bundleName_, userId_);
        if (!rdb) {
            return E_RDB;
        }
        /* init handler */
        fileHandler_ = make_shared<FileDataHandler>(userId_, bundleName_, rdb);
        albumHandler_ = make_shared<AlbumDataHandler>(userId_, bundleName_, rdb);
    }
    dataHandlerRefCount_++;
    LOGD("get handler, refCount: %{public}d", dataHandlerRefCount_);
    return E_OK;
}

void GalleryDataSyncer::PutHandler()
{
    lock_guard<mutex> lock{handleInitMutex_};
    LOGD("put handler, refCount: %{public}d", dataHandlerRefCount_);
    if (dataHandlerRefCount_ > 0) {
        dataHandlerRefCount_--;
    }
    if ((fileHandler_ != nullptr) && (dataHandlerRefCount_ == 0)) {
        fileHandler_ = nullptr;
        albumHandler_ = nullptr;
        LOGI("handler destroyed");
    }
}

int32_t GalleryDataSyncer::Clean(const int action)
{
    LOGD("gallery data sycner Clean");
    
    /* start clean */
    BeginClean();

    int32_t ret = GetHandler();
    if (ret != E_OK) {
        return ret;
    }

    /* file */
    ret = CleanInner(fileHandler_, action);
    if (ret != E_OK) {
        LOGE("gallery data syncer file clean err %{public}d", ret);
    }
    /* album */
    ret = CleanInner(albumHandler_, action);
    if (ret != E_OK) {
        LOGE("gallery data syncer album clean err %{public}d", ret);
    }
    
    DeleteSubscription();
    PutHandler();

    /* complete clean */
    CompleteClean();

    return ret;
}

int32_t GalleryDataSyncer::StartDownloadFile(const std::string path, const int32_t userId)
{
    int32_t ret = GetHandler();
    if (ret != E_OK) {
        return ret;
    }
    ret = DownloadInner(fileHandler_, path, userId);
    PutHandler();
    return ret;
}

int32_t GalleryDataSyncer::StopDownloadFile(const std::string path, const int32_t userId)
{
    int32_t ret = GetHandler();
    if (ret != E_OK) {
        return ret;
    }
    ret = DataSyncer::StopDownloadFile(path, userId);
    PutHandler();
    return ret;
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

void GalleryDataSyncer::Reset()
{
    /* release some resources after last sycn is completed */
    if (stage_ != BEGIN) {
        /* release DataHandler */
        PutHandler();
    }
    /* reset stage in case of stop or restart */
    stage_ = BEGIN;
}

int32_t GalleryDataSyncer::Prepare()
{
    /* call putHandler in the Reset function when sync is completed */
    int32_t ret = GetHandler();
    if (ret != E_OK) {
        LOGE("Get handler failed, may be rdb init failed");
        return ret;
    }
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

int32_t GalleryDataSyncer::Complete()
{
    LOGI("gallery data syncer complete all");
    Unlock();
    CompleteAll();
    return E_OK;
}

int32_t GalleryDataSyncer::OptimizeStorage(const int32_t agingDays)
{
    LOGD("gallery data sycner FileAging");

    int32_t ret = GetHandler();
    if (ret != E_OK) {
        return ret;
    }
    
    ret = fileHandler_->OptimizeStorage(agingDays);

    PutHandler();

    return ret;
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
