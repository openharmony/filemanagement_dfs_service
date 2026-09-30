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

#include "CloudDiskFuseControllerMock.h"

#include <cerrno>
#include <cstdlib>
#include <memory>
#include <string>

#include "cloud_disk_fuse_mount_adapter.h"
#include "cloud_disk_fuse_operations.h"
#include "cloud_disk_service_logfile.h"
#include "cloud_disk_service_metafile.h"
#include "cloud_disk_sync_folder.h"
#include "utils_directory.h"

namespace OHOS::FileManagement::CloudDiskService::Test {

CloudDiskFuseControllerMock::State &CloudDiskFuseControllerMock::GetState()
{
    static State state;
    return state;
}

void CloudDiskFuseControllerMock::Reset()
{
    GetState() = State{};
}

int32_t CloudDiskFuseControllerMock::NextUnmountResult()
{
    auto &state = GetState();
    ++state.unmountCalls;
    if (state.unmountResults.empty()) {
        return state.defaultUnmountResult;
    }
    int32_t result = state.unmountResults.front();
    state.unmountResults.pop_front();
    return result;
}

} // namespace OHOS::FileManagement::CloudDiskService::Test

namespace OHOS::FileManagement::CloudDiskService {

using Test::CloudDiskFuseControllerMock;

CloudDiskSyncFolder &CloudDiskSyncFolder::GetInstance()
{
    static CloudDiskSyncFolder instance;
    return instance;
}

bool CloudDiskSyncFolder::PathToMntPathByPhysicalPath(const std::string &path,
                                                      const std::string &userId,
                                                      std::string &realPath)
{
    (void)path;
    (void)userId;
    auto &state = CloudDiskFuseControllerMock::GetState();
    ++state.convertCalls;
    if (state.convertResult) {
        realPath = state.convertedPath;
    }
    return state.convertResult;
}

LogFileMgr &LogFileMgr::GetInstance()
{
    static LogFileMgr instance;
    return instance;
}

void LogFileMgr::ScheduleFillChildForDir(const int32_t userId, const uint32_t syncFolderIndex, const std::string &path)
{
    (void)userId;
    (void)syncFolderIndex;
    (void)path;
    ++CloudDiskFuseControllerMock::GetState().scheduleFillCalls;
}

int32_t LogFileMgr::WarmupSyncFolder(const int32_t userId,
                                     const uint32_t syncFolderIndex,
                                     const std::string &path,
                                     const WarmupCancelCallback &shouldCancel)
{
    (void)userId;
    (void)syncFolderIndex;
    (void)path;
    auto &state = CloudDiskFuseControllerMock::GetState();
    ++state.warmupCalls;
    state.warmupCancelValue = shouldCancel ? shouldCancel() : false;
    return state.warmupResult;
}

CloudDiskServiceMetaFile::CloudDiskServiceMetaFile(const int32_t userId,
                                                   const uint32_t syncFolderIndex,
                                                   const uint64_t inode)
    : syncFolderIndex_(std::to_string(syncFolderIndex)), userId_(userId)
{
    (void)inode;
}

CloudDiskServiceMetaFile::CloudDiskServiceMetaFile(const int32_t userId,
                                                   const uint32_t syncFolderIndex,
                                                   const uint64_t inode,
                                                   bool createIfMissing)
    : CloudDiskServiceMetaFile(userId, syncFolderIndex, inode)
{
    (void)createIfMissing;
}

int32_t CloudDiskServiceMetaFile::DoLookupByName(MetaBase &base)
{
    uint32_t syncFolderIndex = static_cast<uint32_t>(std::strtoul(syncFolderIndex_.c_str(), nullptr, 10));
    const auto &behaviors = CloudDiskFuseControllerMock::GetState().metaBehaviors;
    auto behavior = behaviors.find(syncFolderIndex);
    if (behavior == behaviors.end()) {
        return ENOENT;
    }
    base.mode = behavior->second.mode;
    base.placeholder = behavior->second.placeholder;
    return behavior->second.lookupResult;
}

MetaFileMgr &MetaFileMgr::GetInstance()
{
    static MetaFileMgr instance;
    return instance;
}

std::shared_ptr<CloudDiskServiceMetaFile> MetaFileMgr::GetCloudDiskServiceMetaFileIfExists(
    const int32_t userId, const uint32_t syncFolderIndex, const uint64_t inode)
{
    auto &state = CloudDiskFuseControllerMock::GetState();
    const auto &behaviors = state.metaBehaviors;
    auto behavior = behaviors.find(syncFolderIndex);
    std::shared_ptr<CloudDiskServiceMetaFile> metaFile;
    if (behavior != behaviors.end() && behavior->second.exists) {
        metaFile = std::make_shared<CloudDiskServiceMetaFile>(userId, syncFolderIndex, inode, false);
    }
    if (state.metaFileLookupHook) {
        state.metaFileLookupHook();
    }
    return metaFile;
}

int32_t MetaFileMgr::GetRelativePathIfExists(const std::shared_ptr<CloudDiskServiceMetaFile> metaFile,
                                             std::string &path)
{
    if (metaFile == nullptr) {
        return ENOENT;
    }
    uint32_t syncFolderIndex = static_cast<uint32_t>(std::strtoul(metaFile->syncFolderIndex_.c_str(), nullptr, 10));
    const auto &behaviors = CloudDiskFuseControllerMock::GetState().metaBehaviors;
    auto behavior = behaviors.find(syncFolderIndex);
    if (behavior == behaviors.end()) {
        return ENOENT;
    }
    path = behavior->second.relativePath;
    return behavior->second.relativePathResult;
}

int32_t CloudDiskFuseMountAdapter::Mount(int32_t userId, const std::string &mountPoint, int &fuseFd)
{
    (void)userId;
    (void)mountPoint;
    auto &state = CloudDiskFuseControllerMock::GetState();
    ++state.mountCalls;
    fuseFd = state.mountFd;
    return state.mountResult;
}

int32_t CloudDiskFuseMountAdapter::Unmount(int32_t userId, const std::string &mountPoint)
{
    (void)userId;
    (void)mountPoint;
    return CloudDiskFuseControllerMock::NextUnmountResult();
}

void CloudDiskFuseOperations::Lookup(fuse_req_t req, fuse_ino_t parent, const char *name)
{
    (void)req;
    (void)parent;
    (void)name;
}

void CloudDiskFuseOperations::Forget(fuse_req_t req, fuse_ino_t ino, uint64_t nlookup)
{
    (void)req;
    (void)ino;
    (void)nlookup;
}

void CloudDiskFuseOperations::GetAttr(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi)
{
    (void)req;
    (void)ino;
    (void)fi;
}

void CloudDiskFuseOperations::Open(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi)
{
    (void)req;
    (void)ino;
    (void)fi;
}

void CloudDiskFuseOperations::Release(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi)
{
    (void)req;
    (void)ino;
    (void)fi;
}

} // namespace OHOS::FileManagement::CloudDiskService

namespace OHOS::Storage::DistributedFile::Utils {

void ForceCreateDirectory(const std::string &path, mode_t mode)
{
    auto &state = OHOS::FileManagement::CloudDiskService::Test::CloudDiskFuseControllerMock::GetState();
    ++state.forceCreateDirectoryCalls;
    state.lastDirectory = path;
    state.lastDirectoryMode = mode;
}

} // namespace OHOS::Storage::DistributedFile::Utils
