/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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
    TYPE_RETRY
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
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS

#endif // OHOS_CLOUD_DISK_SERVICE_TYPE_CONST_H