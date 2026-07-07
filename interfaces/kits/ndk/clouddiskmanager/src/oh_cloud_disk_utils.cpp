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
#include <sys/stat.h>
#include <sys/xattr.h>
#include <vector>

#include <securec.h>

#include "dfsu_access_token_helper.h"
#include "utils_log.h"

namespace {
constexpr const char *SANDBOX_PATH_PREFIX = "/storage/Users/currentUser";
constexpr const char *HMDFS_MNT_PATH_PREFIX = "/mnt/hmdfs/";
constexpr const char *HMDFS_MNT_PATH_SUFFIX = "/account/device_view/local/files/Docs";
constexpr const char *PLACEHOLDER_XATTR_KEY = "user.clouddisk.placeholder";

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

CloudDisk_ErrorCode CheckCloudDiskPermission()
{
    return CheckPermissions(OHOS::FileManagement::PERM_CLOUD_DISK_SERVICE, true);
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

    // 同步根本身允许通过；子路径必须以同步根为前缀，并且边界字符必须是 '/'。
    // 这样可以允许 "/sync/a.txt"，同时拒绝 "/sync2/a.txt" 这类同名前缀目录。
    if (filePath == syncFolderPath) {
        return true;
    }
    if (filePath.size() <= syncFolderPath.size() ||
        filePath.compare(0, syncFolderPath.size(), syncFolderPath) != 0) {
        return false;
    }
    return syncFolderPath.back() == '/' || filePath[syncFolderPath.size()] == '/';
}

CloudDisk_ErrorCode PathToMntPathBySandboxPath(const std::string &path, std::string &realPath)
{
    int32_t userId = OHOS::FileManagement::DfsuAccessTokenHelper::GetUserId();
    std::string replacementPath = std::string(HMDFS_MNT_PATH_PREFIX) + std::to_string(userId) + HMDFS_MNT_PATH_SUFFIX;
    return ReplacePathPrefix(SANDBOX_PATH_PREFIX, replacementPath, path, realPath);
}

CloudDisk_ErrorCode QueryPlaceholderByXattr(const std::string &realFilePath, bool &isPlaceholder)
{
    struct stat statInfo = {};
    errno = 0;
    int statRet = lstat(realFilePath.c_str(), &statInfo);
    int statErr = errno;
    LOGI("Placeholder xattr lstat ret=%{public}d errno=%{public}d mode=%{public}o",
         statRet, statErr, statInfo.st_mode);
    if (statRet != 0) {
        return ConvertXattrErrno(statErr);
    }
    if (!S_ISREG(statInfo.st_mode)) {
        LOGE("Placeholder xattr query rejected non-regular file");
        return CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG;
    }

    errno = 0;
    ssize_t xattrRet = getxattr(realFilePath.c_str(), PLACEHOLDER_XATTR_KEY, nullptr, 0);
    int xattrErr = errno;
    LOGI("Placeholder xattr size key=%{public}s ret=%{public}zd errno=%{public}d",
         PLACEHOLDER_XATTR_KEY, xattrRet, xattrErr);
    if (xattrRet <= 0) {
        return ConvertXattrErrno(xattrErr);
    }

    std::vector<char> value(static_cast<size_t>(xattrRet));
    errno = 0;
    xattrRet = getxattr(realFilePath.c_str(), PLACEHOLDER_XATTR_KEY, value.data(), value.size());
    xattrErr = errno;
    LOGI("Placeholder xattr read key=%{public}s ret=%{public}zd errno=%{public}d",
         PLACEHOLDER_XATTR_KEY, xattrRet, xattrErr);
    if (xattrRet <= 0) {
        return ConvertXattrErrno(xattrErr);
    }

    isPlaceholder = (xattrRet == 1);
    LOGI("Placeholder xattr query success, isPlaceholder=%{public}d", isPlaceholder);
    return CloudDisk_ErrorCode::CLOUD_DISK_OK;
}
