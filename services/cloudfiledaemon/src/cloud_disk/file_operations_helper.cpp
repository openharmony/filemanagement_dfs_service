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
#include "file_operations_helper.h"

#include <cinttypes>
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
    static const int32_t BUNDLE_NAME_OFFSET = 1000000000;
    static const int32_t STAT_MODE_DIR = 0771;
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
    statBuf->st_ino = ino->stat.st_ino;
    statBuf->st_uid = ino->stat.st_uid;
    statBuf->st_gid = ino->stat.st_gid;
    statBuf->st_mtime = ino->stat.st_mtime;
    statBuf->st_ctime = ino->stat.st_ctime;
    statBuf->st_atime = ino->stat.st_atime;
    statBuf->st_mode = ino->stat.st_mode;
    statBuf->st_nlink = ino->stat.st_nlink;
    if (statBuf->st_mode & S_IFREG) {
        statBuf->st_size = ino->stat.st_size;
    }
}

int32_t FileOperationsHelper::GetNextLayer(std::shared_ptr<CloudDiskInode> inoPtr, fuse_ino_t ino)
{
    if (ino == FUSE_ROOT_ID) {
        return CLOUD_DISK_INODE_ZERO_LAYER;
    }
    if (inoPtr->layer >= CLOUD_DISK_INODE_OTHER_LAYER) {
        return CLOUD_DISK_INODE_OTHER_LAYER;
    } else {
        return inoPtr->layer + 1;
    }
}

int32_t FileOperationsHelper::GetFixedLayerRootId(int32_t layer)
{
    if (layer == CLOUD_DISK_INODE_ZERO_LAYER) {
        return CLOUD_DISK_INODE_ZERO_LAYER_LOCALID;
    } else if (layer == CLOUD_DISK_INODE_FIRST_LAYER) {
        return CLOUD_DISK_INODE_FIRST_LAYER_LOCALID;
    }
    return CLOUD_DISK_INODE_LAYER_LOCALID_UNKNOWN;
}

shared_ptr<CloudDiskInode> FileOperationsHelper::FindCloudDiskInode(struct CloudDiskFuseData *data,
                                                                    int64_t key)
{
    shared_ptr<CloudDiskInode> ret;
    shared_lock<shared_mutex> rLock(data->cacheLock, std::defer_lock);
    rLock.lock();
    auto it = data->inodeCache.find(key);
    if (it != data->inodeCache.end()) {
        ret = it->second;
    } else {
        ret = nullptr;
    }
    rLock.unlock();
    return ret;
}

shared_ptr<CloudDiskFile> FileOperationsHelper::FindCloudDiskFile(struct CloudDiskFuseData *data,
                                                                  int64_t key)
{
    shared_ptr<CloudDiskFile> ret;
    shared_lock<shared_mutex> rLock(data->fileLock, std::defer_lock);
    rLock.lock();
    auto it = data->fileCache.find(key);
    if (it != data->fileCache.end()) {
        ret = it->second;
    } else {
        ret = nullptr;
    }
    rLock.unlock();
    return ret;
}

int64_t FileOperationsHelper::FindLocalId(struct CloudDiskFuseData *data, const std::string &key)
{
    int64_t ret = -1;
    shared_lock<shared_mutex> rLock(data->localIdLock, std::defer_lock);
    rLock.lock();
    auto it = data->localIdCache.find(key);
    if (it != data->localIdCache.end()) {
        ret = it->second;
    } else {
        ret = -1;
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
                                                                        fuse_ino_t parent,
                                                                        const string &fileName,
                                                                        const string &path)
{
    std::unique_lock<std::shared_mutex> cWLock(data->cacheLock, std::defer_lock);
    std::unique_lock<std::shared_mutex> lWLock(data->localIdLock, std::defer_lock);
    shared_ptr<CloudDiskInode> child = make_shared<CloudDiskInode>();
    int32_t err = stat(path.c_str(), &child->stat);
    if (err != 0) {
        LOGE("GenerateCloudDiskInode %{public}s error, err: %{public}d", path.c_str(), errno);
        return nullptr;
    }
    child->stat.st_mode |= STAT_MODE_DIR;
    auto parentInode = FindCloudDiskInode(data, parent);
    if (parentInode == nullptr) {
        LOGE("parent inode not found");
        return nullptr;
    }
    child->refCount++;
    child->parent = parent;
    child->path = path;
    child->layer = GetNextLayer(parentInode, parent);
    int64_t localId = GetFixedLayerRootId(child->layer);
    if (child->layer >= CLOUD_DISK_INODE_FIRST_LAYER) {
        std::lock_guard<std::shared_mutex> bWLock(data->bundleNameIdLock);
        data->bundleNameId++;
        localId = data->bundleNameId + BUNDLE_NAME_OFFSET;
    }
    child->stat.st_ino = static_cast<uint64_t>(localId);
    child->ops = make_shared<FileOperationsLocal>();
    cWLock.lock();
    data->inodeCache[localId] = child;
    cWLock.unlock();
    lWLock.lock();
    data->localIdCache[path] = localId;
    lWLock.unlock();
    if (child->layer == CLOUD_DISK_INODE_FIRST_LAYER) {
        child->bundleName = fileName;
        child->ops = make_shared<FileOperationsCloud>();
    }
    return child;
}

void FileOperationsHelper::PutCloudDiskInode(struct CloudDiskFuseData *data,
                                             shared_ptr<CloudDiskInode> inoPtr, uint64_t num, int64_t key)
{
    std::unique_lock<std::shared_mutex> wLock(data->cacheLock, std::defer_lock);
    if (inoPtr == nullptr) {
        LOGD("Get an invalid inode!");
        return;
    }
    inoPtr->refCount -= num;
    if (inoPtr->refCount == 0) {
        LOGD("node released: %{public}" PRId64 "", key);
        wLock.lock();
        data->inodeCache.erase(key);
        wLock.unlock();
    }
}

void FileOperationsHelper::PutCloudDiskFile(struct CloudDiskFuseData *data,
                                            shared_ptr<CloudDiskFile> filePtr, int64_t key)
{
    std::unique_lock<std::shared_mutex> wLock(data->fileLock, std::defer_lock);
    if (filePtr == nullptr) {
        LOGD("Get an invalid file!");
        return;
    }
    if (filePtr->refCount == 0) {
        LOGD("file released: %{public}" PRId64 "", key);
        wLock.lock();
        data->fileCache.erase(key);
        wLock.unlock();
    }
}

void FileOperationsHelper::PutLocalId(struct CloudDiskFuseData *data,
                                      std::shared_ptr<CloudDiskInode> inoPtr,
                                      uint64_t num, const std::string &key)
{
    std::unique_lock<std::shared_mutex> wLock(data->localIdLock, std::defer_lock);
    if (inoPtr == nullptr) {
        LOGD("Get an invalid inode!");
        return;
    }
    inoPtr->refCount -= num;
    if (inoPtr->refCount == 0) {
        LOGD("node released: %{public}s", key.c_str());
        wLock.lock();
        data->localIdCache.erase(key);
        wLock.unlock();
    }
}
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS
