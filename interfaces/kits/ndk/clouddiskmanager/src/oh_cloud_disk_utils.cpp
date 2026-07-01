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

#include <cerrno>
#include <strings.h>

#include <securec.h>

#include "utils_log.h"

namespace {
constexpr const char *PLACEHOLDER_XATTR_KEY = "user.hmdfs.placeholder";
constexpr size_t PLACEHOLDER_XATTR_BUFFER_SIZE = 64;

bool IsAsciiFalseValue(const char *value, ssize_t length)
{
    if (length == 1 && value[0] == '0') {
        return true;
    }
    return length == 5 && strncasecmp(value, "false", 5) == 0;
}
} // namespace

CloudDisk_ErrorCode ConvertToErrorCode(int32_t innerErrorCode)
{
    auto iter = innerToNErrTable.find(innerErrorCode);
    if (iter != innerToNErrTable.end()) {
        return iter->second;
    } else {
        return CloudDisk_ErrorCode::CLOUD_DISK_TRY_AGAIN;
    }
}

CloudDisk_ErrorCode ConvertXattrErrno(int err)
{
    if (err == ENOENT) {
        return CloudDisk_ErrorCode::CLOUD_DISK_SYNC_FOLDER_PATH_NOT_EXIST;
    }
    if (err == EACCES || err == EPERM) {
        return CloudDisk_ErrorCode::CLOUD_DISK_PERMISSION_DENIED;
    }
    if (err == ENOTSUP || err == EOPNOTSUPP) {
        return CloudDisk_ErrorCode::CLOUD_DISK_NOT_SUPPORTED;
    }
    return CloudDisk_ErrorCode::CLOUD_DISK_TRY_AGAIN;
}

const char *GetPlaceholderXattrKey()
{
    return PLACEHOLDER_XATTR_KEY;
}

size_t GetPlaceholderXattrBufferSize()
{
    return PLACEHOLDER_XATTR_BUFFER_SIZE;
}

bool ParsePlaceholderXattrValue(const char *value, ssize_t length)
{
    if (length <= 0) {
        return true;
    }
    if (IsAsciiFalseValue(value, length)) {
        return false;
    }
    if (length == 1 && value[0] == '1') {
        return true;
    }
    if (length == 4 && strncasecmp(value, "true", 4) == 0) {
        return true;
    }
    return value[0] != 0;
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
