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
#include "file_operations_helper.h"

#include <unistd.h>

#include "file_operations_cloud.h"
#include "file_operations_local.h"
#include "securec.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudDisk {
using namespace std;
namespace {
    static const string LOCAL_PATH_DATA_SERVICE_EL2 = "/data/service/el2/";
    static const string LOCAL_PATH_HMDFS_CLOUD_DATA = "/hmdfs/cloud/data/";
    static const string LOCAL_PATH_HMDFS_CLOUD = "/hmdfs/cloud/";
}

string FileOperationsHelper::GetCloudDiskRootPath(int32_t userId)
{
    return LOCAL_PATH_DATA_SERVICE_EL2 + to_string(userId) + LOCAL_PATH_HMDFS_CLOUD;
}

string FileOperationsHelper::GetCloudDiskLocalPath(int32_t userId, string fileName)
{
    if (fileName == "data") {
        return LOCAL_PATH_DATA_SERVICE_EL2 + to_string(userId) +
               LOCAL_PATH_HMDFS_CLOUD_DATA;
    } else if (fileName == "/") {
        return GetCloudDiskRootPath(userId);
    } else {
        return LOCAL_PATH_DATA_SERVICE_EL2 + to_string(userId) +
               LOCAL_PATH_HMDFS_CLOUD_DATA + fileName;
    }
}

void FileOperationsHelper::GetInodeAttr(shared_ptr<CloudDiskInode> ino, struct stat *statBuf)
{
    statBuf->st_ino = reinterpret_cast<fuse_ino_t>(ino.get());
    statBuf->st_uid = ino->stat.st_uid;
    statBuf->st_gid = ino->stat.st_gid;
    statBuf->st_mtime = ino->stat.st_mtime;
    statBuf->st_mode = ino->stat.st_mode;
    statBuf->st_nlink = ino->stat.st_nlink;
    if (statBuf->st_mode & S_IFREG) {
        statBuf->st_size = ino->stat.st_size;
    }
}

int32_t FileOperationsHelper::GetNextLayer(fuse_ino_t ino)
{
    if (ino == FUSE_ROOT_ID) {
        return CLOUD_DISK_INODE_ZERO_LAYER;
    }
    struct CloudDiskInode *inoPtr = reinterpret_cast<struct CloudDiskInode *>(ino);
    if (inoPtr->layer >= CLOUD_DISK_INODE_OTHER_LAYER) {
        return CLOUD_DISK_INODE_OTHER_LAYER;
    } else {
        return inoPtr->layer + 1;
    }
}

shared_ptr<CloudDiskInode> FileOperationsHelper::FindCloudDiskInode(struct CloudDiskFuseData *data,
                                                                    const string &key)
{
    shared_ptr<CloudDiskInode> ret;
    shared_lock<shared_mutex> rLock(data->cacheLock, std::defer_lock);
    rLock.lock();
    if (data->inodeCache.find(key) != data->inodeCache.end()) {
        ret = data->inodeCache[key];
    } else {
        ret = nullptr;
    }
    rLock.unlock();
    return ret;
}

void FileOperationsHelper::AddDirEntry(fuse_req_t req, std::string &buf, size_t &size, const char *name,
                                       std::shared_ptr<CloudDiskInode> ino)
{
    size_t oldSize = size;
    size += fuse_add_direntry(req, NULL, 0, name, NULL, 0);
    buf.resize(size);
    fuse_add_direntry(req, &buf[oldSize], size - oldSize, name, &ino->stat, size);
}

void FileOperationsHelper::FuseReplyLimited(fuse_req_t req, const char *buf, size_t bufSize,
                                            off_t off, size_t maxSize)
{
    if (off < bufSize) {
        size_t size = (bufSize - off) < maxSize ? (bufSize - off) : maxSize;
        fuse_reply_buf(req, buf + off, size);
    } else {
        fuse_reply_buf(req, NULL, 0);
    }
}

shared_ptr<CloudDiskInode> FileOperationsHelper::GenerateCloudDiskInode(struct CloudDiskFuseData *data,
                                                                        struct CloudDiskInode *parent,
                                                                        const string &fileName,
                                                                        const string &path)
{
    std::unique_lock<std::shared_mutex> wLock(data->cacheLock, std::defer_lock);
    shared_ptr<CloudDiskInode> child = make_shared<CloudDiskInode>();
    int32_t err = stat(path.c_str(), &child->stat);
    if (err != 0) {
        LOGE("GenerateCloudDiskInode %{public}s error, err: %{public}d", path.c_str(), errno);
        return nullptr;
    }

    child->refCount++;
    child->parent = reinterpret_cast<fuse_ino_t>(parent);
    child->path = path;
    child->layer = GetNextLayer(child->parent);
    child->stat.st_ino = reinterpret_cast<fuse_ino_t>(child.get());
    child->ops = make_shared<FileOperationsLocal>();
    wLock.lock();
    data->inodeCache[path] = child;
    wLock.unlock();
    if (child->layer == CLOUD_DISK_INODE_FIRST_LAYER) {
        child->bundleName = fileName;
        child->ops = make_shared<FileOperationsCloud>();
    }
    return child;
}

void FileOperationsHelper::PutCloudDiskInode(struct CloudDiskFuseData *data,
                                             shared_ptr<CloudDiskInode> inoPtr, uint64_t num)
{
    std::unique_lock<std::shared_mutex> wLock(data->cacheLock, std::defer_lock);
    if (inoPtr == nullptr) {
        LOGD("Get an invalid inode!");
        return;
    }
    inoPtr->refCount -= num;
    if (inoPtr->refCount == 0) {
        LOGD("node released: %{public}s", inoPtr->path.c_str());
        wLock.lock();
        data->inodeCache.erase(inoPtr->path);
        wLock.unlock();
    }
}
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS
