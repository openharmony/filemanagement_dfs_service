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
    auto rdb_ = NativeRdb::RdbHelper::GetRdbStore(config, Media::MEDIA_RDB_VERSION, cb, err);
    if (rdb_ == nullptr) {
        LOGE("gallyer data syncer init rdb fail");
        return E_RDB;
    }

    /* init handler */
    fileHandler_ = make_shared<FileDataHandler>(userId, bundleName, rdb_);
    albumHandler_ = make_shared<AlbumDataHandler>(userId, bundleName, rdb_);
    return E_OK;
}

int32_t GalleryDataSyncer::Clean(const int action)
{
    LOGD("gallery data sycner Clean");
    /* file */
    int32_t ret = CleanInner(fileHandler_, action);
    if (ret != E_OK) {
        LOGE("gallery data syncer file clean err %{public}d", ret);
    }
    /* album */
    ret = CleanInner(albumHandler_, action);
    if (ret != E_OK) {
        LOGE("gallery data syncer album clean err %{public}d", ret);
    }
    return ret;
}

int32_t GalleryDataSyncer::StartDownloadFile(const std::string path, const int32_t userId)
{
    return DownloadInner(fileHandler_, path, userId);
}

int32_t GalleryDataSyncer::StopDownloadFile(const std::string path, const int32_t userId)
{
    return DataSyncer::StopDownloadFile(path, userId);
}

void GalleryDataSyncer::Schedule()
{
    LOGI("schedule to stage %{public}d", ++stage_);

    int32_t ret = E_OK;
    switch (stage_) {
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
    /* reset stage in case of stop or restart */
    stage_ = BEGIN;
    /* restart a sync might need to update offset, etc */
    fileHandler_->Reset();
    albumHandler_->Reset();
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
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
