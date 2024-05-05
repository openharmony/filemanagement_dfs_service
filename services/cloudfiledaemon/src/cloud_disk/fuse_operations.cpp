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
#include "file_operations_helper.h"
#include "file_operations_local.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudDisk {
using namespace std;

static std::shared_ptr<CloudDiskInode> GetCloudDiskInode(fuse_req_t req, fuse_ino_t ino)
{
    struct CloudDiskFuseData *data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    return FileOperationsHelper::FindCloudDiskInode(data, static_cast<int64_t>(ino));
}

void FuseOperations::Lookup(fuse_req_t req, fuse_ino_t parent, const char *name)
{
    if (parent == FUSE_ROOT_ID) {
        auto opsPtr = make_shared<FileOperationsLocal>();
        opsPtr->Lookup(req, parent, name);
        return;
    }
    auto inoPtr = GetCloudDiskInode(req, parent);
    if (inoPtr == nullptr) {
        fuse_reply_err(req, EINVAL);
        LOGE("parent inode not found");
        return;
    }
    inoPtr->ops->Lookup(req, parent, name);
}

void FuseOperations::Access(fuse_req_t req, fuse_ino_t ino, int mask)
{
    if (ino == FUSE_ROOT_ID) {
        auto opsPtr = make_shared<FileOperationsLocal>();
        opsPtr->Access(req, ino, mask);
        return;
    }
    auto inoPtr = GetCloudDiskInode(req, ino);
    if (inoPtr == nullptr) {
        fuse_reply_err(req, EINVAL);
        LOGE("inode not found");
        return;
    }
    inoPtr->ops->Access(req, ino, mask);
}

void FuseOperations::GetAttr(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi)
{
    if (ino == FUSE_ROOT_ID) {
        auto opsPtr = make_shared<FileOperationsLocal>();
        opsPtr->GetAttr(req, ino, fi);
        return;
    }
    auto inoPtr = GetCloudDiskInode(req, ino);
    if (inoPtr == nullptr) {
        fuse_reply_err(req, EINVAL);
        LOGE("inode not found");
        return;
    }
    inoPtr->ops->GetAttr(req, ino, fi);
}

void FuseOperations::Open(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi)
{
    if (ino == FUSE_ROOT_ID) {
        auto opsPtr = make_shared<FileOperationsLocal>();
        opsPtr->Open(req, ino, fi);
        return;
    }
    auto inoPtr = GetCloudDiskInode(req, ino);
    if (inoPtr == nullptr) {
        fuse_reply_err(req, EINVAL);
        LOGE("inode not found");
        return;
    }
    inoPtr->ops->Open(req, ino, fi);
}

void FuseOperations::Forget(fuse_req_t req, fuse_ino_t ino, uint64_t nLookup)
{
    if (ino == FUSE_ROOT_ID) {
        auto opsPtr = make_shared<FileOperationsLocal>();
        opsPtr->Forget(req, ino, nLookup);
        return;
    }
    auto inoPtr = GetCloudDiskInode(req, ino);
    if (inoPtr == nullptr) {
        fuse_reply_err(req, EINVAL);
        LOGE("inode not found");
        return;
    }
    inoPtr->ops->Forget(req, ino, nLookup);
}

void FuseOperations::ForgetMulti(fuse_req_t req, size_t count, struct fuse_forget_data *forgets)
{
    if (count == 0 || forgets[0].ino == FUSE_ROOT_ID) {
        return;
    }
    auto inoPtr = GetCloudDiskInode(req, forgets[0].ino);
    if (inoPtr == nullptr) {
        fuse_reply_err(req, EINVAL);
        LOGE("inode not found");
        return;
    }
    inoPtr->ops->ForgetMulti(req, count, forgets);
}

void FuseOperations::MkNod(fuse_req_t req, fuse_ino_t parent, const char *name,
                           mode_t mode, dev_t rdev)
{
    if (parent == FUSE_ROOT_ID) {
        auto opsPtr = make_shared<FileOperationsLocal>();
        opsPtr->MkNod(req, parent, name, mode, rdev);
        return;
    }
    auto inoPtr = GetCloudDiskInode(req, parent);
    if (inoPtr == nullptr) {
        fuse_reply_err(req, EINVAL);
        LOGE("parent inode not found");
        return;
    }
    inoPtr->ops->MkNod(req, parent, name, mode, rdev);
}

void FuseOperations::Create(fuse_req_t req, fuse_ino_t parent, const char *name,
                            mode_t mode, struct fuse_file_info *fi)
{
    if (parent == FUSE_ROOT_ID) {
        auto opsPtr = make_shared<FileOperationsLocal>();
        opsPtr->Create(req, parent, name, mode, fi);
        return;
    }
    auto inoPtr = GetCloudDiskInode(req, parent);
    if (inoPtr == nullptr) {
        fuse_reply_err(req, EINVAL);
        LOGE("parent inode not found");
        return;
    }
    inoPtr->ops->Create(req, parent, name, mode, fi);
}

void FuseOperations::ReadDir(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off,
                             struct fuse_file_info *fi)
{
    (void) fi;
    if (ino == FUSE_ROOT_ID) {
        auto opsPtr = make_shared<FileOperationsLocal>();
        opsPtr->ReadDir(req, ino, size, off, fi);
        return;
    }
    auto inoPtr = GetCloudDiskInode(req, ino);
    if (inoPtr == nullptr) {
        fuse_reply_err(req, EINVAL);
        LOGE("inode not found");
        return;
    }
    inoPtr->ops->ReadDir(req, ino, size, off, fi);
}

void FuseOperations::SetXattr(fuse_req_t req, fuse_ino_t ino, const char *name,
                              const char *value, size_t size, int flags)
{
    if (ino == FUSE_ROOT_ID) {
        auto opsPtr = make_shared<FileOperationsLocal>();
        opsPtr->SetXattr(req, ino, name, value, size, flags);
        return;
    }
    auto inoPtr = GetCloudDiskInode(req, ino);
    if (inoPtr == nullptr) {
        fuse_reply_err(req, EINVAL);
        LOGE("inode not found");
        return;
    }
    inoPtr->ops->SetXattr(req, ino, name, value, size, flags);
}

void FuseOperations::GetXattr(fuse_req_t req, fuse_ino_t ino, const char *name,
                              size_t size)
{
    if (ino == FUSE_ROOT_ID) {
        auto opsPtr = make_shared<FileOperationsLocal>();
        opsPtr->GetXattr(req, ino, name, size);
        return;
    }
    auto inoPtr = GetCloudDiskInode(req, ino);
    if (inoPtr == nullptr) {
        fuse_reply_err(req, EINVAL);
        LOGE("inode not found");
        return;
    }
    inoPtr->ops->GetXattr(req, ino, name, size);
}

void FuseOperations::MkDir(fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode)
{
    if (parent == FUSE_ROOT_ID) {
        auto opsPtr = make_shared<FileOperationsLocal>();
        opsPtr->MkDir(req, parent, name, mode);
        return;
    }
    auto inoPtr = GetCloudDiskInode(req, parent);
    if (inoPtr == nullptr) {
        fuse_reply_err(req, EINVAL);
        LOGE("parent inode not found");
        return;
    }
    inoPtr->ops->MkDir(req, parent, name, mode);
}

void FuseOperations::RmDir(fuse_req_t req, fuse_ino_t parent, const char *name)
{
    if (parent == FUSE_ROOT_ID) {
        auto opsPtr = make_shared<FileOperationsLocal>();
        opsPtr->RmDir(req, parent, name);
        return;
    }
    auto inoPtr = GetCloudDiskInode(req, parent);
    if (inoPtr == nullptr) {
        fuse_reply_err(req, EINVAL);
        LOGE("parent inode not found");
        return;
    }
    inoPtr->ops->RmDir(req, parent, name);
}

void FuseOperations::Unlink(fuse_req_t req, fuse_ino_t parent, const char *name)
{
    if (parent == FUSE_ROOT_ID) {
        auto opsPtr = make_shared<FileOperationsLocal>();
        opsPtr->Unlink(req, parent, name);
        return;
    }
    auto inoPtr = GetCloudDiskInode(req, parent);
    if (inoPtr == nullptr) {
        fuse_reply_err(req, EINVAL);
        LOGE("parent inode not found");
        return;
    }
    inoPtr->ops->Unlink(req, parent, name);
}

void FuseOperations::Rename(fuse_req_t req, fuse_ino_t parent, const char *name,
                            fuse_ino_t newParent, const char *newName, unsigned int flags)
{
    if (parent == FUSE_ROOT_ID) {
        auto opsPtr = make_shared<FileOperationsLocal>();
        opsPtr->Rename(req, parent, name, newParent, newName, flags);
        return;
    }
    auto inoPtr = GetCloudDiskInode(req, parent);
    if (inoPtr == nullptr) {
        fuse_reply_err(req, EINVAL);
        LOGE("parent inode not found");
        return;
    }
    inoPtr->ops->Rename(req, parent, name, newParent, newName, flags);
}

void FuseOperations::Read(fuse_req_t req, fuse_ino_t ino, size_t size,
                          off_t offset, struct fuse_file_info *fi)
{
    if (ino == FUSE_ROOT_ID) {
        auto opsPtr = make_shared<FileOperationsLocal>();
        opsPtr->Read(req, ino, size, offset, fi);
        return;
    }
    auto inoPtr = GetCloudDiskInode(req, ino);
    if (inoPtr == nullptr) {
        fuse_reply_err(req, EINVAL);
        LOGE("inode not found");
        return;
    }
    inoPtr->ops->Read(req, ino, size, offset, fi);
}

void FuseOperations::WriteBuf(fuse_req_t req, fuse_ino_t ino, struct fuse_bufvec *bufv,
                              off_t off, struct fuse_file_info *fi)
{
    if (ino == FUSE_ROOT_ID) {
        auto opsPtr = make_shared<FileOperationsLocal>();
        opsPtr->WriteBuf(req, ino, bufv, off, fi);
        return;
    }
    auto inoPtr = GetCloudDiskInode(req, ino);
    if (inoPtr == nullptr) {
        fuse_reply_err(req, EINVAL);
        LOGE("inode not found");
        return;
    }
    inoPtr->ops->WriteBuf(req, ino, bufv, off, fi);
}

void FuseOperations::Release(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi)
{
    if (ino == FUSE_ROOT_ID) {
        auto opsPtr = make_shared<FileOperationsLocal>();
        opsPtr->Release(req, ino, fi);
        return;
    }
    auto inoPtr = GetCloudDiskInode(req, ino);
    if (inoPtr == nullptr) {
        fuse_reply_err(req, EINVAL);
        LOGE("inode not found");
        return;
    }
    inoPtr->ops->Release(req, ino, fi);
}
void FuseOperations::SetAttr(fuse_req_t req, fuse_ino_t ino, struct stat *attr,
                             int valid, struct fuse_file_info *fi)
{
    if (ino == FUSE_ROOT_ID) {
        auto opsPtr = make_shared<FileOperationsLocal>();
        opsPtr->SetAttr(req, ino, attr, valid, fi);
        return;
    }
    auto inoPtr = GetCloudDiskInode(req, ino);
    if (inoPtr == nullptr) {
        fuse_reply_err(req, EINVAL);
        LOGE("inode not found");
        return;
    }
    inoPtr->ops->SetAttr(req, ino, attr, valid, fi);
}
void FuseOperations::Lseek(fuse_req_t req, fuse_ino_t ino, off_t off, int whence,
                           struct fuse_file_info *fi)
{
    if (ino == FUSE_ROOT_ID) {
        auto opsPtr = make_shared<FileOperationsLocal>();
        opsPtr->Lseek(req, ino, off, whence, fi);
        return;
    }
    auto inoPtr = GetCloudDiskInode(req, ino);
    if (inoPtr == nullptr) {
        fuse_reply_err(req, EINVAL);
        LOGE("inode not found");
        return;
    }
    inoPtr->ops->Lseek(req, ino, off, whence, fi);
}
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS
