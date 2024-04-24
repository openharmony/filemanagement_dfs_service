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

#ifndef OHOS_CLOUD_DISK_SERVICE_TYPE_CONST_H
#define OHOS_CLOUD_DISK_SERVICE_TYPE_CONST_H

namespace OHOS {
namespace FileManagement {
namespace CloudDisk {
enum class DirtyType : int32_t {
    TYPE_SYNCED,
    TYPE_NEW,
    TYPE_MDIRTY,
    TYPE_FDIRTY,
    TYPE_DELETED,
    TYPE_RETRY,
    TYPE_NO_NEED_UPLOAD
};

enum class FileType : int32_t {
    FILE_TYPE_APPLICATION = 1,
    FILE_TYPE_AUDIO,
    FILE_TYPE_IMAGE,
    FILE_TYPE_VIDEO,
    FILE_TYPE_TEXT
};

enum class OperationType : int64_t {
    UNKNOWN_TYPE = 0,
    NEW,
    UPDATE,
    COPY,
    SAVE,
    DELETE,
    RESTORE,
    MOVE,
    RENAME,
    OPEN
};

enum FileStatus {
    TO_BE_UPLOADED = 0,
    UPLOADING = 1,
    UPLOAD_FAILURE = 2,
    UPLOAD_SUCCESS = 3,
    UNKNOW = 4
};

enum class NotifyType : uint32_t {
    NOTIFY_ADDED = 0,
    NOTIFY_MODIFIED,
    NOTIFY_DELETED,
    NOTIFY_RENAMED,
    NOTIFY_NONE,
};

constexpr int64_t SECOND_TO_MILLISECOND = 1e3;
constexpr int64_t MILLISECOND_TO_NANOSECOND = 1e6;
constexpr int32_t LOCAL = 1;
constexpr int32_t CLOUD = 2;
constexpr int32_t LOCAL_AND_CLOUD = 3;
constexpr int32_t FILE = 0;
constexpr int32_t DIRECTORY = 1;
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS

#endif // OHOS_CLOUD_DISK_SERVICE_TYPE_CONST_H