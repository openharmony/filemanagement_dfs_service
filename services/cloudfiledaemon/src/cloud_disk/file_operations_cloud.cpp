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
#include "file_operations_cloud.h"

#include <cerrno>

#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudDisk {
void FileOperationsCloud::Lookup(fuse_req_t req, fuse_ino_t parent, const char *name)
{
    LOGE("Lookup operation is not supported!");
    fuse_reply_err(req, ENOSYS);
}

void FileOperationsCloud::Access(fuse_req_t req, fuse_ino_t ino, int mask)
{
    LOGE("Access operation is not supported!");
    fuse_reply_err(req, ENOSYS);
}

void FileOperationsCloud::GetAttr(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi)
{
    LOGE("GetAttr operation is not supported!");
    fuse_reply_err(req, ENOSYS);
}

void FileOperationsCloud::Open(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi)
{
    LOGE("Open operation is not supported!");
    fuse_reply_err(req, ENOSYS);
}

void FileOperationsCloud::Forget(fuse_req_t req, fuse_ino_t ino, uint64_t nLookup)
{
    LOGE("Forget operation is not supported!");
    fuse_reply_err(req, ENOSYS);
}

void FileOperationsCloud::ForgetMulti(fuse_req_t req, size_t count, struct fuse_forget_data *forgets)
{
    LOGE("ForgetMulti operation is not supported!");
    fuse_reply_err(req, ENOSYS);
}

void FileOperationsCloud::MkNod(fuse_req_t req, fuse_ino_t parent, const char *name,
                                mode_t mode, dev_t rdev)
{
    LOGE("MkNod operation is not supported!");
    fuse_reply_err(req, ENOSYS);
}

void FileOperationsCloud::Create(fuse_req_t req, fuse_ino_t parent, const char *name,
                                 mode_t mode, struct fuse_file_info *fi)
{
    LOGE("Create operation is not supported!");
    fuse_reply_err(req, ENOSYS);
}

void FileOperationsCloud::ReadDir(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off,
                                  struct fuse_file_info *fi)
{
    LOGE("ReadDir operation is not supported!");
    fuse_reply_err(req, ENOSYS);
}

void FileOperationsCloud::SetXattr(fuse_req_t req, fuse_ino_t ino, const char *name,
                                   const char *value, size_t size, int flags)
{
    LOGE("SetXattr operation is not supported!");
    fuse_reply_err(req, ENOSYS);
}

void FileOperationsCloud::GetXattr(fuse_req_t req, fuse_ino_t ino, const char *name,
                                   size_t size)
{
    LOGE("GetXattr operation is not supported!");
    fuse_reply_err(req, ENOSYS);
}
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS
