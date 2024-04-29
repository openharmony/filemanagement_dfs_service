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

#ifndef GALLERY_DATA_SYNCER_H
#define GALLERY_DATA_SYNCER_H

#include "functional"

#include "album_data_handler.h"
#include "data_syncer.h"
#include "file_data_handler.h"
#include "gallery_sysevent.h"
#include "rdb_data_handler.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
class GalleryDataSyncer : public DataSyncer, std::enable_shared_from_this<GalleryDataSyncer> {
public:
    GalleryDataSyncer(const std::string bundleName, const int32_t userId);
    virtual ~GalleryDataSyncer() = default;

    virtual void Schedule() override;
    virtual void ScheduleByType(SyncTriggerType syncTriggerType) override;
    virtual void Reset() override;

    virtual int32_t StartDownloadFile(const std::string path, const int32_t userId) override;
    virtual int32_t Init(const std::string bundleName, const int32_t userId) override;
    virtual int32_t Clean(const int action) override;
    virtual int32_t DisableCloud() override;
    virtual int32_t OptimizeStorage(const int32_t agingDays) override;
    virtual int32_t Lock() override;
    virtual void Unlock() override;
    virtual void ForceUnlock() override;
    virtual int32_t DownloadThumb(const int32_t type) override;
    virtual void StopDownloadThumb() override;
    virtual int32_t InitSysEventData() override;
    virtual void FreeSysEventData() override;
    virtual void ReportSysEvent(uint32_t code) override;
    virtual void SetFullSyncSysEvent() override;
    virtual void SetCheckSysEvent() override;
    virtual int32_t CompletePull() override;

private:
    enum {
        BEGIN,
        PREPARE,
        DOWNLOADALBUM,
        DOWNLOADFILE,
        COMPLETEPULL,
        UPLOADALBUM,
        UPLOADFILE,
        COMPLETEPUSH,
        END
    };

    int32_t Prepare();
    int32_t DownloadAlbum();
    int32_t DownloadFile();
    int32_t UploadAlbum();
    int32_t UploadFile();
    int32_t Complete(bool isNeedNotify = true) override;
    std::shared_ptr<NativeRdb::RdbStore> RdbInit(const std::string &bundleName, const int32_t userId);

    /* sync stat */
    void UpdateBasicEventStat(uint32_t code);

    /* stage */
    int32_t stage_ = BEGIN;

    /* rdb */
    const std::string DATA_APP_EL2 = "/data/app/el2/";
    const std::string DATABASE_DIR = "/database/com.ohos.medialibrary.medialibrarydata/rdb/";
    const std::string DATABASE_NAME = "media_library.db";
    const std::string BUNDLE_NAME = "com.ohos.medialibrary.medialibrarydata";
    const int32_t CONNECT_SIZE = 10;
    const int32_t EL2_PATH = 1;

    /* handler */
    std::shared_ptr<FileDataHandler> fileHandler_;
    std::shared_ptr<AlbumDataHandler> albumHandler_;

    /* sync stat */
    std::shared_ptr<GalleryIncSyncStat> syncStat_;
    /* check stat */
    std::shared_ptr<GalleryCheckSatat> checkStat_;
};

class RdbCallback : public NativeRdb::RdbOpenCallback {
public:
    virtual int32_t OnCreate(NativeRdb::RdbStore &r) override
    {
        return 0;
    }

    virtual int32_t OnUpgrade(NativeRdb::RdbStore &r, int32_t oldVersion,
        int32_t newVersion) override
    {
        return 0;
    }
};
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
#endif // GALLERY_DATA_SYNCER_H
