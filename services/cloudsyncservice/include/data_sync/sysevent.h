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

#include "hisysevent.h"

#include "dfs_error.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {

#define CloudSyncSysEvent(eventName, type, ...)    \
    HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::FILEMANAGEMENT, eventName,   \
                    type, ##__VA_ARGS__)    \

class SysEventData {
public:
    virtual void Report() = 0;
};

class SyncData : public SysEventData {
public:
    SyncData() : upload_meta_(6, 0), download_meta_(5, 0), download_thumb_(5, 0),
        download_lcd_(5, 0), upload_album_(4, 0), download_album_(4, 0)
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

    void SetSyncReason(uint32_t reason)
    {
        sync_reason_ = reason;
    }

    void SetStopReason(uint32_t reason)
    {
        stop_reason_ = reason;
    }

    void SetStartTime(uint64_t time)
    {
        start_time_ = time;
    }

    void SetDuration(uint64_t time)
    {
        duration_ = time - start_time_;
    }

protected:
    bool isFullSync_;

    uint32_t sync_reason_;
    uint32_t stop_reason_;
    uint64_t start_time_;
    uint64_t duration_;
    std::vector<uint64_t> upload_meta_;
    std::vector<uint64_t> download_meta_;
    std::vector<uint64_t> download_thumb_;
    std::vector<uint64_t> download_lcd_;
    std::vector<uint64_t> upload_album_;
    std::vector<uint64_t> download_album_;
    std::string sync_info_;
};

class IncSyncData : public SyncData {
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
        if (isFullSync_ != true) {
            return;
        }

        int32_t ret = CloudSyncSysEvent("CLOUD_SYNC_INC_STAT",
            HiviewDFX::HiSysEvent::EventType::STATISTIC,
            "upload_meta", upload_meta_,
            "download_meta", download_meta_,
            "download_thumb", download_thumb_,
            "download_lcd", download_lcd_,
            "upload_album", upload_album_,
            "download_album", download_album_,
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

class FullSyncData : public SyncData {
public:
    FullSyncData()
    {
        isFullSync_ = true;
    }

    ~FullSyncData() {}

    void Report() override
    {
        int32_t ret = CloudSyncSysEvent("CLOUD_SYNC_FULL_STAT",
            HiviewDFX::HiSysEvent::EventType::STATISTIC,
            "sync_reason", sync_reason_,
            "stop_reason", stop_reason_,
            "start_time", start_time_,
            "duration", duration_,
            "upload_meta", upload_meta_,
            "download_meta", download_meta_,
            "download_thumb", download_thumb_,
            "download_lcd", download_lcd_,
            "upload_album", upload_album_,
            "download_album", download_album_,
            "sync_info", sync_info_
        );
        if (ret != E_OK) {
            LOGE("report CLOUD_SYNC_FULL_STAT error %{public}d", ret);
        }
    }
};
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
#endif // OHOS_CLOUD_SYNC_SERVICE_SYSEVENT_H
