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

GalleryDataSyncer::GalleryDataSyncer(const std::string bundleName, const int32_t userId)
    : DataSyncer(bundleName, userId)
{
    /* rdb config */
    NativeRdb::RdbStoreConfig config(DATABASE_NAME);
    config.SetPath(DATA_APP_EL2 + to_string(userId) + DATABASE_DIR + DATABASE_NAME);
    config.SetBundleName(BUNDLE_NAME);
    config.SetReadConSize(CONNECT_SIZE);
    config.SetSecurityLevel(NativeRdb::SecurityLevel::S3);
    config.SetScalarFunction("cloud_sync_func", 0, CloudSyncTriggerFunc);

    /*
     * Just pass in any value but zero for parameter @version in GetRdbStore,
     * since RdbCallback always return 0 in its callbacks.
     */
    int32_t err;
    RdbCallback cb;
    auto rdb_ = NativeRdb::RdbHelper::GetRdbStore(config, 1, cb, err);
    if (rdb_ == nullptr) {
        LOGE("gallyer data syncer init rdb fail");
        exit(0);
    }

    /* init handler */
    fileHandler_ = make_shared<FileDataHandler>(userId, bundleName, rdb_);
}

void GalleryDataSyncer::Schedule()
{
    LOGI("schedule to stage %{public}d", ++stage_);

    int32_t ret = E_OK;
    switch (stage_) {
        case PREPARE : {
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
        case UPLOADALBUM: {
            ret = UploadAlbum();
            break;
        }
        case UPLOADFILE: {
            ret = UploadFile();
            break;
        }
        case WAIT: {
            ret = Wait();
            break;
        }
        case END: {
            /* reset stage in case one restarts a sync */
            stage_ = BEGIN;
            ret = Complete();
            break;
        }
        default: {
            ret = E_SCHEDULE;
            LOGE("schedule fail %{public}d", ret);
            break;
        }
    }
}

int32_t GalleryDataSyncer::Prepare()
{
    LOGI("gallery data syncer prepare");
    /* restart a sync might need to update offset, etc */
    fileHandler_->Reset();
    Schedule();
    return E_OK;
}

int32_t GalleryDataSyncer::DownloadAlbum()
{
    LOGI("gallery data sycner download album");
    Schedule();
    return E_OK;
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
    LOGI("gallery data sycner upload album");
    Schedule();
    return E_OK;
}

int32_t GalleryDataSyncer::UploadFile()
{
    LOGI("gallery data sycner upload file");
    int ret = Push(fileHandler_);
    if (ret != E_OK) {
        LOGE("gallery data syncer push file err %{public}d", ret);
    }
    return ret;
}

/* wait for file download and upload */
int32_t GalleryDataSyncer::Wait()
{
    LOGI("gallery data syncer wait");
    Schedule();
    return E_OK;
}

int32_t GalleryDataSyncer::Complete()
{
    LOGI("gallery data syncer complete all");
    CompleteAll(E_OK, SyncType::ALL);
    return E_OK;
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
