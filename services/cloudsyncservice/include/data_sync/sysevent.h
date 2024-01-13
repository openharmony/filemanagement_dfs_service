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

#ifndef OHOS_CLOUD_SYNC_SERVICE_SYSEVENT_H
#define OHOS_CLOUD_SYNC_SERVICE_SYSEVENT_H

#include <atomic>

#include "hisysevent.h"

#include "dfs_error.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {

#define CLOUD_SYNC_SYS_EVENT(eventName, type, ...)    \
    HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::FILEMANAGEMENT, eventName,   \
                    type, ##__VA_ARGS__)    \

class SysEventData {
public:
    virtual void Report() = 0;
};

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

template <typename T>
static inline std::vector<T> vector_atomic_to_origin(std::vector<std::atomic<T>> &v)
{
    std::vector<T> ret;
    ret.reserve(v.size());
    for (auto &atomic_value : v) {
        ret.push_back(atomic_value.load());
    }
    return ret;
}

class SyncData : public SysEventData {
public:
    SyncData() : uploadMeta_(UPLOAD_META_LEN),
        downloadMeta_(DOWNLOAD_META_LEN),
        downloadThumb_(DOWNLOAD_THUMB_LEN),
        downloadLcd_(DOWNLOAD_LCD_LEN),
        uploadAlbum_(UPLOAD_ALBUM_LEN),
        downloadAlbum_(DOWNLOAD_ALBUM_LEN)
    {
    }

    virtual ~SyncData() {}

    void AppendSyncInfo(std::string &info)
    {
        sync_info_.append(info);
    }

    bool IsFullSync()
    {
        return isFullSync_;
    }

    void SetFullSync()
    {
        isFullSync_ = true;
    }

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

    void SetSyncReason(uint32_t reason)
    {
        syncReason_ = reason;
    }

    void SetStopReason(uint32_t reason)
    {
        stopReason_ = reason;
    }

    void SetStartTime(uint64_t time)
    {
        startTime_ = time;
    }

    void SetDuration(uint64_t time)
    {
        duration_ = time - startTime_;
    }

protected:
    bool isFullSync_;

    uint32_t syncReason_;
    uint32_t stopReason_;
    uint64_t startTime_;
    uint64_t duration_;
    std::vector<std::atomic<uint64_t>> uploadMeta_;
    std::vector<std::atomic<uint64_t>> downloadMeta_;
    std::vector<std::atomic<uint64_t>> downloadThumb_;
    std::vector<std::atomic<uint64_t>> downloadLcd_;
    std::vector<std::atomic<uint64_t>> uploadAlbum_;
    std::vector<std::atomic<uint64_t>> downloadAlbum_;
    std::string sync_info_;
};

class FullSyncData : public SyncData {
public:
    FullSyncData()
    {
        isFullSync_ = true;
    }

    ~FullSyncData() {}

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
            "download_thumb", vector_atomic_to_origin<uint64_t>(downloadThumb_),
            "download_lcd", vector_atomic_to_origin<uint64_t>(downloadLcd_),
            "upload_album", vector_atomic_to_origin<uint64_t>(uploadAlbum_),
            "download_album", vector_atomic_to_origin<uint64_t>(downloadAlbum_),
            "sync_info", sync_info_
        );
        if (ret != E_OK) {
            LOGE("report CLOUD_SYNC_FULL_STAT error %{public}d", ret);
        }
    }
};

class IncSyncData : public FullSyncData {
public:
    IncSyncData()
    {
        isFullSync_ = false;
    }

    ~IncSyncData()
    {
        Dump();
    }

    void Report() override
    {
        if (isFullSync_ == true) {
            FullSyncData::Report();
            return;
        }

        int32_t ret = CLOUD_SYNC_SYS_EVENT("CLOUD_SYNC_INC_STAT",
            HiviewDFX::HiSysEvent::EventType::STATISTIC,
            "upload_meta", vector_atomic_to_origin<uint64_t>(uploadMeta_),
            "download_meta", vector_atomic_to_origin<uint64_t>(downloadMeta_),
            "download_thumb", vector_atomic_to_origin<uint64_t>(downloadThumb_),
            "download_lcd", vector_atomic_to_origin<uint64_t>(downloadLcd_),
            "upload_album", vector_atomic_to_origin<uint64_t>(uploadAlbum_),
            "download_album", vector_atomic_to_origin<uint64_t>(downloadAlbum_),
            "sync_info", sync_info_
        );
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
    }
};
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
#endif // OHOS_CLOUD_SYNC_SERVICE_SYSEVENT_H
