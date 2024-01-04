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

static int32_t DoLocalLookup(fuse_req_t req, fuse_ino_t parent, const char *name,
                             struct fuse_entry_param *e)
{
    int32_t err = 0;
    bool createFlag = false;
    shared_ptr<CloudDiskInode> child;
    struct CloudDiskFuseData *data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    string path = FileOperationsHelper::GetCloudDiskLocalPath(data->userId, name);
    std::unique_lock<std::shared_mutex> wLock(data->cacheLock, std::defer_lock);

    child = FileOperationsHelper::FindCloudDiskInode(data, path);
    if (child == nullptr) {
        child = make_shared<CloudDiskInode>();
        createFlag = true;
        LOGD("new child %{public}s", name);
    }

    err = stat(path.c_str(), &child->stat);
    if (err != 0) {
        LOGE("lookup %{public}s error, err: %{public}d", path.c_str(), errno);
        return errno;
    }

    child->refCount++;
    if (createFlag) {
        child->parent = parent;
        child->path = path;
        child->layer = FileOperationsHelper::GetNextLayer(parent);
        child->stat.st_ino = reinterpret_cast<fuse_ino_t>(child.get());
        child->ops = make_shared<FileOperationsLocal>();
        wLock.lock();
        data->inodeCache[path] = child;
        wLock.unlock();
    }
    if (child->layer >= CLOUD_DISK_INODE_FIRST_LAYER) {
        child->bundleName = name;
        child->ops = make_shared<FileOperationsCloud>();
    }
    e->ino = reinterpret_cast<fuse_ino_t>(child.get());
    FileOperationsHelper::GetInodeAttr(child, &e->attr);
    return 0;
}

void FileOperationsLocal::Lookup(fuse_req_t req, fuse_ino_t parent, const char *name)
{
    struct fuse_entry_param e;
    int32_t err;

    err = DoLocalLookup(req, parent, name, &e);
    if (err) {
        fuse_reply_err(req, err);
    } else {
        fuse_reply_entry(req, &e);
    }
}

void FileOperationsLocal::GetAttr(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi)
{
    if (ino == FUSE_ROOT_ID) {
        struct CloudDiskFuseData *data =
            reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
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
    struct CloudDiskInode *inoPtr = reinterpret_cast<struct CloudDiskInode *>(ino);
    fuse_reply_attr(req, &inoPtr->stat, 0);
}

void FileOperationsLocal::Forget(fuse_req_t req, fuse_ino_t ino, uint64_t nLookup)
{
    struct CloudDiskFuseData *data =
        reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    struct CloudDiskInode *inoPtr = reinterpret_cast<struct CloudDiskInode *>(ino);
    shared_ptr<CloudDiskInode> node = FileOperationsHelper::FindCloudDiskInode(data, inoPtr->path);
    FileOperationsHelper::PutCloudDiskInode(data, node, nLookup, inoPtr->path);
    fuse_reply_none(req);
}

void FileOperationsLocal::ForgetMulti(fuse_req_t req, size_t count, struct fuse_forget_data *forgets)
{
    struct CloudDiskFuseData *data =
        reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    for (size_t i = 0; i < count; i++) {
        struct CloudDiskInode *inoPtr =
            reinterpret_cast<struct CloudDiskInode *>(forgets[i].ino);
        shared_ptr<CloudDiskInode> node = FileOperationsHelper::FindCloudDiskInode(data, inoPtr->path);
        FileOperationsHelper::PutCloudDiskInode(data, node, forgets[i].nlookup, inoPtr->path);
    }
    fuse_reply_none(req);
}

void FileOperationsLocal::ReadDir(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off,
                                  struct fuse_file_info *fi)
{
    (void) fi;
    string path;
    struct CloudDiskInode *inoPtr = reinterpret_cast<struct CloudDiskInode *>(ino);
    struct CloudDiskFuseData *data =
        reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    shared_ptr<CloudDiskInode> child;
    if (ino == FUSE_ROOT_ID) {
        path = FileOperationsHelper::GetCloudDiskRootPath(data->userId);
    } else {
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

        string childPath = FileOperationsHelper::GetCloudDiskLocalPath(data->userId, entry->d_name);
        shared_ptr<CloudDiskInode> childPtr = FileOperationsHelper::FindCloudDiskInode(
            data, childPath.c_str());
        if (childPtr == nullptr) {
            childPtr = FileOperationsHelper::GenerateCloudDiskInode(data, inoPtr,
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
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS
