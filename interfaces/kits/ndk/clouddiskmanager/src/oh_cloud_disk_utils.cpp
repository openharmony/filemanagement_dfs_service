/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "oh_cloud_disk_utils.h"

#include <securec.h>

#include "utils_log.h"

CloudDisk_ErrorCode ConvertToErrorCode(int32_t innerErrorCode)
{
    auto iter = innerToNErrTable.find(innerErrorCode);
    if (iter != innerToNErrTable.end()) {
        return iter->second;
    } else {
        return CloudDisk_ErrorCode::CLOUD_DISK_TRY_AGAIN;
    }
}

char *AllocField(const char *value, size_t length)
{
    char *field = new(std::nothrow) char[length + 1];
    if (field == nullptr) {
        LOGE("new failed");
        return nullptr;
    }

    if (memcpy_s(field, length + 1, value, length) != 0) {
        LOGE("memcpy_s failed");
        delete[] field;
        return nullptr;
    }
    field[length] = '\0';
    return field;
}

bool IsValidPathInfo(const char *path, size_t length)
{
    if (path == nullptr) {
        LOGE("path is nullptr");
        return false;
    }
    if (length == 0) {
        LOGE("length is invalid: %{public}zu", length);
        return false;
    }
    size_t actualLen = strnlen(path, length + 1);
    if (actualLen != length) {
        LOGE("length not equal to actual string length: %{public}zu", actualLen);
        return false;
    }
    return true;
}