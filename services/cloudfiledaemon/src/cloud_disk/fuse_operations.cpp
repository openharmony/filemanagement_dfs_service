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
#include "fuse_operations.h"

#include "cloud_disk_inode.h"
#include "file_operations_local.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudDisk {
using namespace std;
void FuseOperations::Lookup(fuse_req_t req, fuse_ino_t parent, const char *name)
{
    struct CloudDiskInode *inoPtr = nullptr;
    if (parent == FUSE_ROOT_ID) {
        auto opsPtr = make_shared<FileOperationsLocal>();
        opsPtr->Lookup(req, parent, name);
        return;
    }
    inoPtr= reinterpret_cast<struct CloudDiskInode*>(parent);
    inoPtr->ops->Lookup(req, parent, name);
}

void FuseOperations::Access(fuse_req_t req, fuse_ino_t ino, int mask)
{
    struct CloudDiskInode *inoPtr = nullptr;
    if (ino == FUSE_ROOT_ID) {
        auto opsPtr = make_shared<FileOperationsLocal>();
        opsPtr->Access(req, ino, mask);
        return;
    }
    inoPtr = reinterpret_cast<struct CloudDiskInode*>(ino);
    inoPtr->ops->Access(req, ino, mask);
}

void FuseOperations::GetAttr(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi)
{
    struct CloudDiskInode *inoPtr = nullptr;
    if (ino == FUSE_ROOT_ID) {
        auto opsPtr = make_shared<FileOperationsLocal>();
        opsPtr->GetAttr(req, ino, fi);
        return;
    }
    inoPtr = reinterpret_cast<struct CloudDiskInode*>(ino);
    inoPtr->ops->GetAttr(req, ino, fi);
}

void FuseOperations::Open(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi)
{
    struct CloudDiskInode *inoPtr = nullptr;
    if (ino == FUSE_ROOT_ID) {
        auto opsPtr = make_shared<FileOperationsLocal>();
        opsPtr->Open(req, ino, fi);
        return;
    }
    inoPtr = reinterpret_cast<struct CloudDiskInode*>(ino);
    inoPtr->ops->Open(req, ino, fi);
}

void FuseOperations::Forget(fuse_req_t req, fuse_ino_t ino, uint64_t nLookup)
{
    struct CloudDiskInode *inoPtr = nullptr;
    if (ino == FUSE_ROOT_ID) {
        auto opsPtr = make_shared<FileOperationsLocal>();
        opsPtr->Forget(req, ino, nLookup);
        return;
    }
    inoPtr = reinterpret_cast<struct CloudDiskInode*>(ino);
    inoPtr->ops->Forget(req, ino, nLookup);
}

void FuseOperations::ForgetMulti(fuse_req_t req, size_t count, struct fuse_forget_data *forgets)
{
    if (count == 0 || forgets[0].ino == FUSE_ROOT_ID) {
        return;
    }
    struct CloudDiskInode *inoPtr = reinterpret_cast<struct CloudDiskInode*>(forgets[0].ino);
    inoPtr->ops->ForgetMulti(req, count, forgets);
}

void FuseOperations::MkNod(fuse_req_t req, fuse_ino_t parent, const char *name,
                           mode_t mode, dev_t rdev)
{
    struct CloudDiskInode *inoPtr = nullptr;
    if (parent == FUSE_ROOT_ID) {
        auto opsPtr = make_shared<FileOperationsLocal>();
        opsPtr->MkNod(req, parent, name, mode, rdev);
        return;
    }
    inoPtr = reinterpret_cast<struct CloudDiskInode*>(parent);
    inoPtr->ops->MkNod(req, parent, name, mode, rdev);
}

void FuseOperations::Create(fuse_req_t req, fuse_ino_t parent, const char *name,
                            mode_t mode, struct fuse_file_info *fi)
{
    struct CloudDiskInode *inoPtr = nullptr;
    if (parent == FUSE_ROOT_ID) {
        auto opsPtr = make_shared<FileOperationsLocal>();
        opsPtr->Create(req, parent, name, mode, fi);
        return;
    }
    inoPtr = reinterpret_cast<struct CloudDiskInode*>(parent);
    inoPtr->ops->Create(req, parent, name, mode, fi);
}

void FuseOperations::ReadDir(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off,
                             struct fuse_file_info *fi)
{
    struct CloudDiskInode *inoPtr = nullptr;
    (void) fi;
    if (ino == FUSE_ROOT_ID) {
        auto opsPtr = make_shared<FileOperationsLocal>();
        opsPtr->ReadDir(req, ino, size, off, fi);
        return;
    }
    inoPtr = reinterpret_cast<struct CloudDiskInode*>(ino);
    inoPtr->ops->ReadDir(req, ino, size, off, fi);
}

void FuseOperations::SetXattr(fuse_req_t req, fuse_ino_t ino, const char *name,
                              const char *value, size_t size, int flags)
{
    struct CloudDiskInode *inoPtr = nullptr;
    if (ino == FUSE_ROOT_ID) {
        auto opsPtr = make_shared<FileOperationsLocal>();
        opsPtr->SetXattr(req, ino, name, value, size, flags);
        return;
    }
    inoPtr = reinterpret_cast<struct CloudDiskInode*>(ino);
    inoPtr->ops->SetXattr(req, ino, name, value, size, flags);
}

void FuseOperations::GetXattr(fuse_req_t req, fuse_ino_t ino, const char *name,
                              size_t size)
{
    struct CloudDiskInode *inoPtr = nullptr;
    if (ino == FUSE_ROOT_ID) {
        auto opsPtr = make_shared<FileOperationsLocal>();
        opsPtr->GetXattr(req, ino, name, size);
        return;
    }
    inoPtr = reinterpret_cast<struct CloudDiskInode*>(ino);
    inoPtr->ops->GetXattr(req, ino, name, size);
}
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS
