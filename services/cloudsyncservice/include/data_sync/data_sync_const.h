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
#ifndef OHOS_CLOUD_SYNC_SERVICE_DATA_SYNC_CONST_H
#define OHOS_CLOUD_SYNC_SERVICE_DATA_SYNC_CONST_H

#include "medialibrary_db_const.h"
#include "medialibrary_type_const.h"
#include "photo_album_column.h"
#include "photo_map_column.h"
#include <set>
#include <string>
namespace OHOS::FileManagement::CloudSync {

enum CleanAction {
    RETAIN_DATA = 0,
    CLEAR_DATA
};

enum AlbumSource {
    ALBUM_FROM_LOCAL = 1,
    ALBUM_FROM_CLOUD = 2
};

enum class SyncTriggerType : int32_t {
    APP_TRIGGER,
    CLOUD_TRIGGER,
    PENDING_TRIGGER,
    BATTERY_OK_TRIGGER,
    NETWORK_AVAIL_TRIGGER,
    TASK_TRIGGER,
    SYSTEM_LOAD_TRIGGER,
};

enum class ThumbState : int32_t {
    DOWNLOADED,
    LCD_TO_DOWNLOAD,
    THM_TO_DOWNLOAD,
    TO_DOWNLOAD,
};

struct LocalInfo {
    int64_t mdirtyTime;
    int64_t fdirtyTime;
};

const int32_t POSITION_LOCAL = 1;
const int32_t POSITION_CLOUD = 2;
const int32_t POSITION_BOTH = 3;

const int32_t FILE = 0;
const int32_t DIRECTORY = 1;

const int32_t MILLISECOND_TO_SECOND = 1000;
const int32_t SECOND_TO_MILLISECOND = 1000;
const int64_t MILLISECOND_TO_NANOSECOND = 1e6;

const std::string GALLERY_BUNDLE_NAME = "com.ohos.photos";
const std::string MEDIALIBRARY_BUNDLE_NAME = "com.ohos.medialibrary.medialibrarydata";

static inline uint64_t GetCurrentTimeStamp()
{
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    return t.tv_sec * MILLISECOND_TO_NANOSECOND + t.tv_nsec / MILLISECOND_TO_NANOSECOND;
}
} // namespace OHOS::FileManagement::CloudSync
#endif // OHOS_CLOUD_SYNC_SERVICE_DATA_SYNC_CONST_H
