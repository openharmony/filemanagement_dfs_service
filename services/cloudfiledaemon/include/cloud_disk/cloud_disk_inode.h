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
#ifndef CLOUD_FILE_DAEMON_CLOUD_DISK_INODE_H
#define CLOUD_FILE_DAEMON_CLOUD_DISK_INODE_H

#include <atomic>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <unordered_map>

#include "dk_asset_read_session.h"
#include "dk_database.h"
#include "file_operations_base.h"

namespace OHOS {
namespace FileManagement {
namespace CloudDisk {

enum CLOUD_DISK_INODE_LAYER {
    CLOUD_DISK_INODE_ZERO_LAYER = 0, // data
    CLOUD_DISK_INODE_FIRST_LAYER,    // bundleName
    CLOUD_DISK_INODE_OTHER_LAYER     // others
};

enum CLOUD_DISK_INODE_LAYER_LOCALID {
    CLOUD_DISK_INODE_LAYER_LOCALID_UNKNOWN = 0, // placeholder
    CLOUD_DISK_INODE_ROOT_LAYER_LOCALID,        // /
    CLOUD_DISK_INODE_ZERO_LAYER_LOCALID,        // data
    CLOUD_DISK_INODE_FIRST_LAYER_LOCALID        // bundleName
};

enum CLOUD_DISK_FILE_DIRTY {
    CLOUD_DISK_FILE_UNKNOWN = 0,
    CLOUD_DISK_FILE_CREATE,
    CLOUD_DISK_FILE_WRITE
};

enum CLOUD_DISK_FILE_TYPE {
    CLOUD_DISK_FILE_TYPE_UNKNOWN = 0,
    CLOUD_DISK_FILE_TYPE_LOCAL,
    CLOUD_DISK_FILE_TYPE_CLOUD
};

struct CloudDiskInode {
    int layer{CLOUD_DISK_INODE_ZERO_LAYER};
    struct stat stat;
    std::string cloudId{"rootId"};
    std::string bundleName;
    std::string fileName;
    fuse_ino_t parent{0};
    std::atomic<int> refCount{0};
    std::string path; // just used in local file operation

    /* ops means file operation that uses local or database */
    std::shared_ptr<FileOperationsBase> ops{nullptr};
};

struct CloudDiskFile {
    int type{CLOUD_DISK_FILE_TYPE_UNKNOWN};
    int fileDirty{CLOUD_DISK_FILE_UNKNOWN};
    int32_t fd{-1};
    std::atomic<int> refCount{0};
    std::shared_ptr<DriveKit::DKAssetReadSession> readSession{nullptr};
};

struct CloudDiskFuseData {
    int userId;
    int64_t bundleNameId{1};
    int64_t fileId{0};
    std::shared_ptr<CloudDiskInode> rootNode{nullptr};
    std::unordered_map<int64_t, std::shared_ptr<CloudDiskInode>> inodeCache;
    std::unordered_map<int64_t, std::shared_ptr<CloudDiskFile>> fileCache;
    std::unordered_map<std::string, int64_t> localIdCache;
    std::shared_mutex bundleNameIdLock;
    std::shared_mutex cacheLock;
    std::shared_mutex fileLock;
    std::shared_mutex fileIdLock;
    std::shared_mutex localIdLock;
    struct fuse_session *se;
};
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS
#endif // CLOUD_FILE_DAEMON_CLOUD_DISK_INODE_H
