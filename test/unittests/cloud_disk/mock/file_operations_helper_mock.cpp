/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
    static const int32_t NULL_PTR = -1;
    static const int32_t MOCK0 = 0;
    static const int32_t MOCK1 = 1;
    static const int32_t MOCK2 = 2;
    static const int32_t MOCK3 = 3;
    static const int32_t MOCK4 = 4;
    static const int32_t MOCK5 = 5;
    static const int32_t MOCK6 = 6;
    static const int32_t MOCK7 = 7;
    static const int32_t MOCKUSERID0 = 100;
    static const int32_t MOCKUSERID1 = 1;
    static const int32_t STATSIZE = 1;
}

string FileOperationsHelper::GetCloudDiskRootPath(int32_t userId)
{
    if (userId == MOCKUSERID0 || userId == MOCKUSERID1) {
        return "/data";
    }
    return "";
}

string FileOperationsHelper::GetCloudDiskLocalPath(int32_t userId, string fileName)
{
    if (userId == MOCKUSERID0) {
        return "/data";
    }
    return "";
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
    shared_ptr<CloudDiskInode> ptr = make_shared<CloudDiskInode>();
    if (key == NULL_PTR) {
        ptr = nullptr;
    } else if (key == MOCK0) {
        ptr->parent = -1;
        ptr->layer = CLOUD_DISK_INODE_FIRST_LAYER;
        ptr->path = "";
    } else if (key == MOCK1) {
        ptr->parent = 0;
    } else if (key == MOCK2) {
        ptr->parent = 0;
        ptr->bundleName = "com.ohos.photos";
        ptr->cloudId = "mock";
        ptr->fileName = "mock";
    } else if (key == MOCK3) {
        ptr->parent = 0;
        ptr->cloudId = "mock";
        ptr->fileName = "test";
    } else if (key == MOCK4) {
        ptr->bundleName = "com.ohos.photos";
        ptr->parent = 0;
        ptr->cloudId = "";
        ptr->fileName = "test";
    } else if (key == MOCK5) {
        ptr = nullptr;
    } else if (key == MOCK6) {
        ptr->parent = MOCK5;
    } else if (key == MOCK7) {
        ptr->parent = MOCK7;
        ptr->stat.st_size = STATSIZE;
    }
    return ptr;
}

shared_ptr<CloudDiskFile> FileOperationsHelper::FindCloudDiskFile(struct CloudDiskFuseData *data,
                                                                  int64_t key)
{
    shared_ptr<CloudDiskFile> ptr = make_shared<CloudDiskFile>();
    ptr -> type = CLOUD_DISK_FILE_TYPE_LOCAL;
    ptr -> refCount = 1;
    ptr -> fileDirty = CLOUD_DISK_FILE_CREATE;
    if (key == NULL_PTR) {
        ptr = nullptr;
    } else if (key == MOCK0) {
        ptr -> fileDirty = CLOUD_DISK_FILE_UNKNOWN;
    } else if (key == MOCK1) {
        string recordType = "";
        string recordId = "";
        string assetKey = "";
        string assetPath = "";
        ptr -> type = CLOUD_DISK_FILE_TYPE_CLOUD;
        ptr -> readSession =
        make_shared<CloudFile::CloudAssetReadSession>(recordType, recordId, assetKey, assetPath);
    } else if (key == MOCK3) {
        ptr -> refCount = 0;
    } else if (key == MOCK4) {
        ptr = nullptr;
    }
    return ptr;
}

int64_t FileOperationsHelper::FindLocalId(struct CloudDiskFuseData *data, const std::string &key)
{
    if (key == "1mock" || key == "0mock") {
        return -1;
    }
    if (data->userId == 1) {
        return -1;
    }
    return 0;
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
    shared_ptr<CloudDiskInode> child = make_shared<CloudDiskInode>();
    if (data->userId == 1) {
        return nullptr;
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