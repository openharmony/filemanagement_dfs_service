/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_CLOUD_SYNC_SERVICE_GALLERY_SYSEVENT_H
#define OHOS_CLOUD_SYNC_SERVICE_GALLERY_SYSEVENT_H

#include <atomic>

#include "hisysevent.h"

#include "dfs_error.h"
#include "sysevent.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {

/* stat length */
#define UPLOAD_META_LEN 6
#define DOWNLOAD_META_LEN 5
#define DOWNLOAD_THUMB_LEN 5
#define DOWNLOAD_LCD_LEN 5
#define UPLOAD_ALBUM_LEN 4
#define DOWNLOAD_ALBUM_LEN 4

/* stat index */
enum MetaStatIndex {
    /* download */
    INDEX_DL_META_SUCCESS,
    INDEX_DL_META_ERROR_SDK,
    INDEX_DL_META_ERROR_DATA,
    INDEX_DL_META_ERROR_IO,
    INDEX_DL_META_ERROR_RDB,

    /* upload */
    INDEX_UL_META_SUCCESS,
    INDEX_UL_META_ERROR_SDK,
    INDEX_UL_META_ERROR_DATA,
    INDEX_UL_META_ERROR_IO,
    INDEX_UL_META_ERROR_ATTACHMENT,
    INDEX_UL_META_ERROR_RDB,
};

enum AttachmentStatIndex {
    /* thumb */
    INDEX_THUMB_SUCCESS,
    INDEX_THUMB_ERROR_SDK,
    INDEX_THUMB_ERROR_DATA,
    INDEX_THUMB_ERROR_IO,
    INDEX_THUMB_ERROR_RDB,

    /* lcd */
    INDEX_LCD_SUCCESS,
    INDEX_LCD_ERROR_SDK,
    INDEX_LCD_ERROR_DATA,
    INDEX_LCD_ERROR_IO,
    INDEX_LCD_ERROR_RDB,

    /* context */
};

enum AlbumStatIndex {
    /* upload */
    INDEX_UL_ALBUM_SUCCESS,
    INDEX_UL_ALBUM_ERROR_SDK,
    INDEX_UL_ALBUM_ERROR_DATA,
    INDEX_UL_ALBUM_ERROR_RDB,

    /* download */
    INDEX_DL_ALBUM_SUCCESS,
    INDEX_DL_ALBUM_ERROR_SDK,
    INDEX_DL_ALBUM_ERROR_DATA,
    INDEX_DL_ALBUM_ERROR_RDB,
};

class GallerySyncStat : public SyncStat {
public:
    GallerySyncStat() : uploadMeta_(UPLOAD_META_LEN),
        downloadMeta_(DOWNLOAD_META_LEN),
        downloadThumb_(DOWNLOAD_THUMB_LEN),
        downloadLcd_(DOWNLOAD_LCD_LEN),
        uploadAlbum_(UPLOAD_ALBUM_LEN),
        downloadAlbum_(DOWNLOAD_ALBUM_LEN)
    {
    }

    virtual ~GallerySyncStat() {}

    void UpdateMetaStat(uint32_t index, uint64_t diff)
    {
        if (index < INDEX_UL_META_SUCCESS) {
            downloadMeta_[index].fetch_add(diff);
        } else {
            uploadMeta_[index - INDEX_UL_META_SUCCESS].fetch_add(diff);
        }
    }

    void UpdateAttachmentStat(uint32_t index, uint64_t diff)
    {
        if (index < INDEX_LCD_SUCCESS) {
            downloadThumb_[index].fetch_add(diff);
        } else {
            downloadLcd_[index - INDEX_LCD_SUCCESS].fetch_add(diff);
        }
    }

    void UpdateAlbumStat(uint32_t index, uint64_t diff)
    {
        if (index < INDEX_DL_ALBUM_SUCCESS) {
            downloadAlbum_[index].fetch_add(diff);
        } else {
            uploadAlbum_[index - INDEX_DL_ALBUM_SUCCESS].fetch_add(diff);
        }
    }

protected:
    std::vector<std::atomic<uint64_t>> uploadMeta_;
    std::vector<std::atomic<uint64_t>> downloadMeta_;
    std::vector<std::atomic<uint64_t>> downloadThumb_;
    std::vector<std::atomic<uint64_t>> downloadLcd_;
    std::vector<std::atomic<uint64_t>> uploadAlbum_;
    std::vector<std::atomic<uint64_t>> downloadAlbum_;
};

class GalleryFullSyncStat : public GallerySyncStat {
public:
    GalleryFullSyncStat()
    {
        isFullSync_ = true;
    }

    ~GalleryFullSyncStat() {}

    void Report() override
    {
        int32_t ret = CLOUD_SYNC_SYS_EVENT("CLOUD_SYNC_FULL_STAT",
            HiviewDFX::HiSysEvent::EventType::STATISTIC,
            "sync_reason", syncReason_,
            "stop_reason", stopReason_,
            "start_time", startTime_,
            "duration", duration_,
            "upload_meta", vector_atomic_to_origin<uint64_t>(uploadMeta_),
            "download_meta", vector_atomic_to_origin<uint64_t>(downloadMeta_),
            "download_thm", vector_atomic_to_origin<uint64_t>(downloadThumb_),
            "download_lcd", vector_atomic_to_origin<uint64_t>(downloadLcd_),
            "upload_album", vector_atomic_to_origin<uint64_t>(uploadAlbum_),
            "download_album", vector_atomic_to_origin<uint64_t>(downloadAlbum_),
            "sync_info", sync_info_);
        if (ret != E_OK) {
            LOGE("report CLOUD_SYNC_FULL_STAT error %{public}d", ret);
        }
    }
};

class GalleryIncSyncStat : public GalleryFullSyncStat {
public:
    GalleryIncSyncStat()
    {
    }

    ~GalleryIncSyncStat()
    {
        Dump();
    }

    void Report() override
    {
        if (isFullSync_ == true) {
            GalleryFullSyncStat::Report();
            return;
        }

        int32_t ret = CLOUD_SYNC_SYS_EVENT("CLOUD_SYNC_INC_STAT",
            HiviewDFX::HiSysEvent::EventType::STATISTIC,
            "upload_meta", vector_atomic_to_origin<uint64_t>(uploadMeta_),
            "download_meta", vector_atomic_to_origin<uint64_t>(downloadMeta_),
            "download_thm", vector_atomic_to_origin<uint64_t>(downloadThumb_),
            "download_lcd", vector_atomic_to_origin<uint64_t>(downloadLcd_),
            "upload_album", vector_atomic_to_origin<uint64_t>(uploadAlbum_),
            "download_album", vector_atomic_to_origin<uint64_t>(downloadAlbum_),
            "sync_info", sync_info_);
        if (ret != E_OK) {
            LOGE("report CLOUD_SYNC_INC_STAT error %{public}d", ret);
        }
    }

    /* read from disk */
    void Load()
    {
    }

    /* write into disk */
    void Dump()
    {
        Load();
    }
};

class GallerySyncStatContainer : public SyncStatContainer<GalleryIncSyncStat> {
public:
    void UpdateMetaStat(uint32_t index, uint64_t diff)
    {
        std::shared_ptr<GalleryIncSyncStat> stat = GetSyncStat();
        /* for sake of no-crash, might delete later */
        if (stat != nullptr) {
            stat->UpdateMetaStat(index, diff);
        }
    }

    void UpdateAttachmentStat(uint32_t index, uint64_t diff)
    {
        std::shared_ptr<GalleryIncSyncStat> stat = GetSyncStat();
        /* for sake of no-crash, might delete later */
        if (stat != nullptr) {
            stat->UpdateAttachmentStat(index, diff);
        }
    }

    void UpdateAlbumStat(uint32_t index, uint64_t diff)
    {
        std::shared_ptr<GalleryIncSyncStat> stat = GetSyncStat();
        /* for sake of no-crash, might delete later */
        if (stat != nullptr) {
            stat->UpdateAlbumStat(index, diff);
        }
    }
};

enum GalleryCheckStatIndex {
    /* foud */
    INDEX_CHECK_FOUND,
    /* fixed */
    INDEX_CHECK_FIXED,
};

#define GALLERY_CHECK_STAT_LEN 2

class GalleryCheckSatat : public SyncStat {
public:
    GalleryCheckSatat() : file_(GALLERY_CHECK_STAT_LEN),
        album_(GALLERY_CHECK_STAT_LEN),
        map_(GALLERY_CHECK_STAT_LEN),
        attachment_(GALLERY_CHECK_STAT_LEN)
    {
    }

    void Report() override
    {
        int32_t ret = CLOUD_SYNC_SYS_EVENT("CLOUD_SYNC_CHECK",
            HiviewDFX::HiSysEvent::EventType::STATISTIC,
            "start_time", startTime_,
            "duration", duration_,
            "file", file_,
            "album", album_,
            "map", map_,
            "attachment", attachment_,
            "check_info", checkInfo_);
        if (ret != E_OK) {
            LOGE("report CLOUD_SYNC_CHECK error %{public}d", ret);
        }
    }

    void UpdateFile(uint32_t index, uint64_t diff)
    {
        file_[index] += diff;
    }

    void UpdateAlbum(uint32_t index, uint64_t diff)
    {
        album_[index] += diff;
    }

    void UpdateMap(uint32_t index, uint64_t diff)
    {
        map_[index] += diff;
    }

    void UpdateAttachment(uint32_t index, uint64_t diff)
    {
        attachment_[index] += diff;
    }

private:
    std::vector<uint64_t> file_;
    std::vector<uint64_t> album_;
    std::vector<uint64_t> map_;
    std::vector<uint64_t> attachment_;
    std::string checkInfo_;
};

class GalleryCheckStatContainer : public CheckStatContainer<GalleryCheckSatat> {
public:
    void UpdateCheckFile(uint32_t index, uint64_t diff)
    {
        std::shared_ptr<GalleryCheckSatat> stat = GetCheckStat();
        if (stat != nullptr) {
            stat->UpdateFile(index, diff);
        }
    }

    void UpdateCheckAlbum(uint32_t index, uint64_t diff)
    {
        std::shared_ptr<GalleryCheckSatat> stat = GetCheckStat();
        if (stat != nullptr) {
            stat->UpdateAlbum(index, diff);
        }
    }

    void UpdateCheckMap(uint32_t index, uint64_t diff)
    {
        std::shared_ptr<GalleryCheckSatat> stat = GetCheckStat();
        if (stat != nullptr) {
            stat->UpdateMap(index, diff);
        }
    }

    void UpdateCheckAttachment(uint32_t index, uint64_t diff)
    {
        std::shared_ptr<GalleryCheckSatat> stat = GetCheckStat();
        if (stat != nullptr) {
            stat->UpdateAttachment(index, diff);
        }
    }
};
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
#endif // OHOS_CLOUD_SYNC_SERVICE_GALLERY_SYSEVENT_H
