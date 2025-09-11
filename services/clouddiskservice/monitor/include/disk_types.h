/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_FILEMANAGEMENT_DISK_TYPES_H
#define OHOS_FILEMANAGEMENT_DISK_TYPES_H
#include <ctime>
#include <filesystem>
#include <string>

#include "cloud_disk_common.h"

namespace OHOS::FileManagement::CloudDiskService {
constexpr int64_t SECOND_TO_MILLISECOND = 1e3;
constexpr int64_t MILLISECOND_TO_NANOSECOND = 1e6;

static uint64_t UTCTimeMilliSeconds()
{
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    return t.tv_sec * SECOND_TO_MILLISECOND + t.tv_nsec / MILLISECOND_TO_NANOSECOND;
}

struct EventInfo {
    int32_t userId{-1};
    uint32_t syncFolderIndex{0};
    std::string path;
    std::string name;
    OperationType operateType{OperationType::OPERATION_MAX};
    uint64_t timestamp{0};

    EventInfo() {}
    EventInfo(int32_t uid, int32_t syncFolder, OperationType type, const std::string &filePath)
        : userId(uid), syncFolderIndex(syncFolder), operateType(type)
    {
        path = std::filesystem::path(filePath).parent_path().string();
        name = std::filesystem::path(filePath).filename().string();
        timestamp = UTCTimeMilliSeconds();
    }

    void Reset()
    {
        userId = -1;
        syncFolderIndex = 0;
        path = "";
        name = "";
        operateType = OperationType::OPERATION_MAX;
        timestamp = 0;
    }
};
} // namespace OHOS::FileManagement::CloudDiskService
#endif
