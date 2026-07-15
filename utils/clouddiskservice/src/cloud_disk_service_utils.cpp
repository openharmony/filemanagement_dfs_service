/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "cloud_disk_service_utils.h"

#include <cerrno>

#include "cloud_disk_service_error.h"

namespace OHOS::FileManagement::CloudDiskService {

bool IsPathInSyncFolder(const std::string &syncFolder, const std::string &filePath)
{
    if (syncFolder.empty()) {
        return false;
    }
    std::string prefix = syncFolder;
    if (prefix.back() != '/') {
        prefix += "/";
    }
    return filePath.size() > prefix.size() && filePath.compare(0, prefix.size(), prefix) == 0;
}

bool HasInvalidRelativePathSegment(const std::string &path)
{
    if (path.empty()) {
        return true;
    }
    size_t start = 0;
    while (start < path.size()) {
        size_t end = path.find('/', start);
        std::string segment = path.substr(start, end == std::string::npos ? end : end - start);
        if (segment == "." || segment == "..") {
            return true;
        }
        if (end == std::string::npos) {
            break;
        }
        start = end + 1;
    }
    return false;
}

std::string JoinSyncFolderAndRelativePath(const std::string &syncFolderPath, const std::string &relativePath)
{
    std::string filePath = syncFolderPath;
    if (!filePath.empty() && filePath.back() != '/') {
        filePath += "/";
    }
    filePath += relativePath;
    return filePath;
}

int32_t ConvertErrnoToCloudDiskError(int32_t err)
{
    switch (err) {
        case EEXIST:
            return E_FILE_ALREADY_EXISTS;
        case ENOTDIR:
            return E_NOT_A_DIRECTORY;
        case ENOSPC:
        case EDQUOT:
            return E_NO_SPACE_LEFT;
        case EINVAL:
        case EISDIR:
        case ELOOP:
        case ENODATA:
        case ERANGE:
        case ENAMETOOLONG:
            return E_INVALID_ARG;
        case ENOENT:
            return E_SYNC_FOLDER_PATH_NOT_EXIST;
        case EACCES:
        case EPERM:
            return E_PERMISSION_DENIED;
        case ENOTTY:
        case EOPNOTSUPP:
            return E_NOT_SUPPORTED;
        case EIO:
        default:
            return E_TRY_AGAIN;
    }
}

} // namespace OHOS::FileManagement::CloudDiskService
