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
#include <cstdlib>
#include <limits.h>
#include <string>
#include <strings.h>

#include <securec.h>

#include "dfsu_access_token_helper.h"
#include "utils_log.h"

namespace {
constexpr const char *PLACEHOLDER_XATTR_KEY = "user.hmdfs.placeholder";
constexpr size_t PLACEHOLDER_XATTR_BUFFER_SIZE = 64;
constexpr const char *SANDBOX_PATH_PREFIX = "/storage/Users/currentUser";
constexpr const char *HMDFS_MNT_PATH_PREFIX = "/mnt/hmdfs/";
constexpr const char *HMDFS_MNT_PATH_SUFFIX = "/account/device_view/local/files/Docs";
constexpr ssize_t ASCII_TRUE_LENGTH = 4;
constexpr ssize_t ASCII_FALSE_LENGTH = 5;

bool IsAsciiFalseValue(const char *value, ssize_t length)
{
    if (length == 1 && value[0] == '0') {
        return true;
    }
    return length == ASCII_FALSE_LENGTH && strncasecmp(value, "false", ASCII_FALSE_LENGTH) == 0;
}

std::string TrimTrailingSlashes(const std::string &path)
{
    if (path.empty()) {
        return path;
    }
    size_t end = path.size();
    while (end > 1 && path[end - 1] == '/') {
        --end;
    }
    return path.substr(0, end);
}

CloudDisk_ErrorCode ReplacePathPrefix(const std::string &oldPrefix, const std::string &newPrefix,
                                      const std::string &inputPath, std::string &outputPath)
{
    if ((inputPath.size() < oldPrefix.size()) || (inputPath.substr(0, oldPrefix.size()) != oldPrefix)) {
        LOGE("Invalid path prefix");
        return CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG;
    }

    std::string newPath = newPrefix + inputPath.substr(oldPrefix.size());
    char realPathBuffer[PATH_MAX + 1] = {'\0'};
    errno = 0;
    if (realpath(newPath.c_str(), realPathBuffer) == nullptr) {
        LOGE("Realpath error: %{public}d", errno);
        return (errno == ENOENT) ? CloudDisk_ErrorCode::CLOUD_DISK_SYNC_FOLDER_PATH_NOT_EXIST
                                 : CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG;
    }

    std::string newRealPath(realPathBuffer);
    if (newRealPath.substr(0, newPrefix.size()) != newPrefix) {
        LOGE("Path traversal prefix");
        return CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG;
    }

    outputPath = newRealPath;
    return CloudDisk_ErrorCode::CLOUD_DISK_OK;
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

CloudDisk_ErrorCode CheckPermissions(const std::string &permission, bool isSystemApp)
{
    if (!permission.empty() && !OHOS::FileManagement::DfsuAccessTokenHelper::CheckCallerPermission(permission)) {
        LOGE("permission denied");
        return CloudDisk_ErrorCode::CLOUD_DISK_PERMISSION_DENIED;
    }
    if (isSystemApp && !OHOS::FileManagement::DfsuAccessTokenHelper::IsSystemApp()) {
        LOGE("caller hap is not system hap");
        return CloudDisk_ErrorCode::CLOUD_DISK_PERMISSION_DENIED;
    }
    return CloudDisk_ErrorCode::CLOUD_DISK_OK;
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
    if (length == ASCII_TRUE_LENGTH && strncasecmp(value, "true", ASCII_TRUE_LENGTH) == 0) {
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

bool IsPathInSyncFolder(const std::string &syncFolderPath, const std::string &filePath)
{
    if (syncFolderPath.empty() || filePath.empty()) {
        return false;
    }

    std::string normalizedSyncFolder = TrimTrailingSlashes(syncFolderPath);
    if (filePath == normalizedSyncFolder) {
        return true;
    }
    if (filePath.size() <= normalizedSyncFolder.size()) {
        return false;
    }
    if (filePath.compare(0, normalizedSyncFolder.size(), normalizedSyncFolder) != 0) {
        return false;
    }
    return normalizedSyncFolder.back() == '/' || filePath[normalizedSyncFolder.size()] == '/';
}

CloudDisk_ErrorCode PathToMntPathBySandboxPath(const std::string &path, int32_t userId, std::string &realPath)
{
    std::string replacementPath = std::string(HMDFS_MNT_PATH_PREFIX) + std::to_string(userId) + HMDFS_MNT_PATH_SUFFIX;
    return ReplacePathPrefix(SANDBOX_PATH_PREFIX, replacementPath, path, realPath);
}
