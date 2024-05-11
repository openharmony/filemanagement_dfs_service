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
#ifndef CLOUD_FILE_DAEMON_FILE_OPERATIONS_HELPER_H
#define CLOUD_FILE_DAEMON_FILE_OPERATIONS_HELPER_H

#include "cloud_disk_inode.h"

namespace OHOS {
namespace FileManagement {
namespace CloudDisk {
class FileOperationsHelper {
public:
    static int32_t GetNextLayer(std::shared_ptr<CloudDiskInode> inoPtr, fuse_ino_t parent);
    static int32_t GetFixedLayerRootId(int32_t layer);
    static std::string GetCloudDiskRootPath(int32_t userId);
    static std::string GetCloudDiskLocalPath(int32_t userId, std::string fileName);
    static void GetInodeAttr(std::shared_ptr<CloudDiskInode> ino, struct stat *statBuf);
    static std::shared_ptr<CloudDiskInode> FindCloudDiskInode(struct CloudDiskFuseData *data,
                                                              int64_t key);
    static std::shared_ptr<CloudDiskFile> FindCloudDiskFile(struct CloudDiskFuseData *data,
                                                            int64_t key);
    static int64_t FindLocalId(struct CloudDiskFuseData *data, const std::string &key);
    static void AddDirEntry(fuse_req_t req, std::string &buf, size_t &size, const char *name,
                            std::shared_ptr<CloudDiskInode> ino);
    static void FuseReplyLimited(fuse_req_t req, const char *buf, size_t bufSize,
                                 off_t off, size_t maxSize);
    static void PutCloudDiskInode(struct CloudDiskFuseData *data,
                                  std::shared_ptr<CloudDiskInode> inoPtr, uint64_t num, int64_t key);
    static void PutCloudDiskFile(struct CloudDiskFuseData *data,
                                 std::shared_ptr<CloudDiskFile> filePtr, int64_t key);
    static void PutLocalId(struct CloudDiskFuseData *data,
                           std::shared_ptr<CloudDiskInode> inoPtr, uint64_t num, const std::string &key);
    static std::shared_ptr<CloudDiskInode> GenerateCloudDiskInode(struct CloudDiskFuseData *data,
                                                                  fuse_ino_t parent,
                                                                  const std::string &fileName,
                                                                  const std::string &path);
};
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS
#endif // CLOUD_FILE_DAEMON_FILE_OPERATIONS_HELPER_H
