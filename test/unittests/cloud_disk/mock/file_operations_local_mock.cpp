/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include "file_operations_local.h"

#include <cerrno>
#include <dirent.h>

#include "file_operations_cloud.h"
#include "file_operations_helper.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudDisk {
using namespace std;
static const int32_t BUNDLE_NAME_OFFSET = 1000000000;
static const int32_t STAT_MODE_DIR = 0771;
static const float LOOKUP_TIMEOUT = 60.0;

void FileOperationsLocal::Lookup(fuse_req_t req, fuse_ino_t parent, const char *name) {}

void FileOperationsLocal::GetAttr(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi)
{
    struct CloudDiskFuseData *data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    if (ino == FUSE_ROOT_ID) {
        string path = FileOperationsHelper::GetCloudDiskRootPath(data->userId);

        struct stat statBuf;
        int err = stat(path.c_str(), &statBuf);
        if (err != 0) {
            LOGE("lookup %{public}s error, err: %{public}d", path.c_str(), err);
            fuse_reply_err(req, err);
            return;
        }
        fuse_reply_attr(req, &statBuf, 0);
        return;
    }
    auto inoPtr = FileOperationsHelper::FindCloudDiskInode(data, static_cast<int64_t>(ino));
    if (inoPtr == nullptr) {
        fuse_reply_err(req, EINVAL);
        LOGE("inode not found");
        return;
    }
    fuse_reply_attr(req, &inoPtr->stat, 0);
}

void FileOperationsLocal::ReadDir(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off,
                                  struct fuse_file_info *fi)
{
    (void) fi;
    string path;
    struct CloudDiskFuseData *data =
        reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    if (ino == FUSE_ROOT_ID) {
        path = FileOperationsHelper::GetCloudDiskRootPath(data->userId);
    } else {
        auto inoPtr = FileOperationsHelper::FindCloudDiskInode(data, static_cast<int64_t>(ino));
        if (inoPtr == nullptr) {
            fuse_reply_err(req, EINVAL);
            LOGE("inode not found");
            return;
        }
        path = inoPtr->path;
    }
    DIR* dir = opendir(path.c_str());
    if (dir == NULL) {
        LOGE("opendir error %{public}d, path:%{public}s", errno, path.c_str());
        return;
    }

    struct dirent *entry;
    string entryData;
    size_t len = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        string childPath = FileOperationsHelper::GetCloudDiskLocalPath(data->userId,
            entry->d_name);
        int64_t key = FileOperationsHelper::FindLocalId(data, std::to_string(ino) +
            entry->d_name);
        auto childPtr = FileOperationsHelper::FindCloudDiskInode(data, key);
        if (childPtr == nullptr) {
            childPtr = FileOperationsHelper::GenerateCloudDiskInode(data, ino,
                entry->d_name, childPath.c_str());
        }
        if (childPtr == nullptr) {
            continue;
        }
        FileOperationsHelper::AddDirEntry(req, entryData, len, entry->d_name, childPtr);
    }
    FileOperationsHelper::FuseReplyLimited(req, entryData.c_str(), len, off, size);
    closedir(dir);
    return;
}

void FileOperationsLocal::Ioctl(fuse_req_t req, fuse_ino_t ino, int cmd, void *arg, struct fuse_file_info *fi,
                                unsigned flags, const void *inBuf, size_t inBufsz, size_t outBufsz)
{
    fuse_reply_ioctl(req, 0, NULL, 0);
    return;
}
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS
