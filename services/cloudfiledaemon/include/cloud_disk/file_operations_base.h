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

#ifndef CLOUD_FILE_DAEMON_FILE_OPERATIONS_BASE_H
#define CLOUD_FILE_DAEMON_FILE_OPERATIONS_BASE_H

#define FUSE_USE_VERSION 34

#include <fuse.h>
#include <fuse_i.h>
#include <fuse_lowlevel.h>

namespace OHOS {
namespace FileManagement {
namespace CloudDisk {
class FileOperationsBase {
public:
    virtual void Lookup(fuse_req_t req, fuse_ino_t parent, const char *name);
    virtual void Access(fuse_req_t req, fuse_ino_t ino, int mask);
    virtual void GetAttr(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi);
    virtual void Open(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi);
    virtual void Forget(fuse_req_t req, fuse_ino_t ino, uint64_t nLookup);
    virtual void ForgetMulti(fuse_req_t req, size_t count, struct fuse_forget_data *forgets);
    virtual void MkNod(fuse_req_t req, fuse_ino_t parent, const char *name,
                       mode_t mode, dev_t rdev);
    virtual void Create(fuse_req_t req, fuse_ino_t parent, const char *name,
                        mode_t mode, struct fuse_file_info *fi);
    virtual void ReadDir(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off,
                         struct fuse_file_info *fi);
    virtual void SetXattr(fuse_req_t req, fuse_ino_t ino, const char *name,
                          const char *value, size_t size, int flags);
    virtual void GetXattr(fuse_req_t req, fuse_ino_t ino, const char *name,
                          size_t size);
};
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS
#endif // CLOUD_FILE_DAEMON_FILE_OPERATIONS_BASE_H
