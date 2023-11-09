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

#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudDisk {
void FileOperationsLocal::Lookup(fuse_req_t req, fuse_ino_t parent, const char *name)
{
    LOGE("Lookup operation is not supported!");
    fuse_reply_err(req, ENOSYS);
}

void FileOperationsLocal::GetAttr(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi)
{
    LOGE("GetAttr operation is not supported!");
    fuse_reply_err(req, ENOSYS);
}

void FileOperationsLocal::Forget(fuse_req_t req, fuse_ino_t ino, uint64_t nLookup)
{
    LOGE("Forget operation is not supported!");
    fuse_reply_err(req, ENOSYS);
}

void FileOperationsLocal::ForgetMulti(fuse_req_t req, size_t count, struct fuse_forget_data *forgets)
{
    LOGE("ForgetMulti operation is not supported!");
    fuse_reply_err(req, ENOSYS);
}

void FileOperationsLocal::ReadDir(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off,
                                  struct fuse_file_info *fi)
{
    LOGE("ReadDir operation is not supported!");
    fuse_reply_err(req, ENOSYS);
}
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS
