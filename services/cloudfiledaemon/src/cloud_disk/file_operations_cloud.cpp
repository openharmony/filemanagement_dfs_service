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
#include <sys/types.h>
#include <sys/xattr.h>

#include "cloud_disk_inode.h"
#include "cloud_file_utils.h"
#include "database_manager.h"
#include "dk_database.h"
#include "drive_kit.h"
#include "file_operations_helper.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudDisk {
using namespace std;
using namespace DriveKit;
namespace {
    static const uint32_t STAT_MODE_REG = 0770;
    static const uint32_t STAT_MODE_DIR = 0771;
}

static int32_t DoCloudLookup(fuse_req_t req, fuse_ino_t parent, const char *name,
                             struct fuse_entry_param *e)
{
    return 0;
}

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
    struct CloudDiskInode *inoPtr = reinterpret_cast<struct CloudDiskInode *>(ino);
    fuse_reply_attr(req, &inoPtr->stat, 0);
}

void FileOperationsCloud::Open(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi)
{
    int32_t fd = -1;
    struct CloudDiskFuseData *data =
        reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    struct CloudDiskInode *inoPtr = reinterpret_cast<struct CloudDiskInode *>(ino);

    string path = CloudFileUtils::GetLocalFilePath(
        inoPtr->cloudId, inoPtr->bundleName, data->userId);
    fd = open(path.c_str(), fi->flags & O_NOFOLLOW);
    if (fd < 0) {
        LOGE("open file failed path:%{public}s errno:%{public}d", path.c_str(), errno);
        return (void) fuse_reply_err(req, errno);
    }
    fi->fh = fd;
    fuse_reply_open(req, fi);
}

void FileOperationsCloud::Forget(fuse_req_t req, fuse_ino_t ino, uint64_t nLookup)
{
    struct CloudDiskFuseData *data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    if (ino == FUSE_ROOT_ID) {
        LOGD("Cloud file operation should not get a root inode");
        return (void) fuse_reply_none(req);
    }
    struct CloudDiskInode *inoPtr = reinterpret_cast<struct CloudDiskInode *>(ino);
    shared_ptr<CloudDiskInode> node = FileOperationsHelper::FindCloudDiskInode(data, inoPtr->cloudId);
    fuse_reply_none(req);
}

void FileOperationsCloud::ForgetMulti(fuse_req_t req, size_t count, struct fuse_forget_data *forgets)
{
    struct CloudDiskFuseData *data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    for (size_t i = 0; i < count; i++) {
        if (forgets[i].ino == FUSE_ROOT_ID) {
            LOGD("Cloud file operation should not get a root inode");
            return (void) fuse_reply_none(req);
        }
        struct CloudDiskInode *inoPtr = reinterpret_cast<struct CloudDiskInode *>(forgets[i].ino);
        shared_ptr<CloudDiskInode> node = FileOperationsHelper::FindCloudDiskInode(data, inoPtr->cloudId);
        FileOperationsHelper::PutCloudDiskInode(data, node, forgets[i].nlookup);
    }
    fuse_reply_none(req);
}

static shared_ptr<DriveKit::DKDatabase> GetDatabase(int32_t userId, string bundleName)
{
    auto driveKit = DriveKit::DriveKitNative::GetInstance(userId);
    if (driveKit == nullptr) {
        LOGE("sdk helper get drive kit instance fail");
        return nullptr;
    }

    auto container = driveKit->GetDefaultContainer(bundleName);
    if (container == nullptr) {
        LOGE("sdk helper get drive kit container fail");
        return nullptr;
    }

    shared_ptr<DriveKit::DKDatabase> database = container->GetPrivateDatabase();
    if (database == nullptr) {
        LOGE("sdk helper get drive kit database fail");
        return nullptr;
    }
    return database;
}

static int32_t CreateLocalFile(const string &cloudId, const string &bundleName, int32_t userId, mode_t mode)
{
    string bucketPath = CloudFileUtils::GetLocalBucketPath(cloudId, bundleName, userId);
    string path = CloudFileUtils::GetLocalFilePath(cloudId, bundleName, userId);
    if (access(bucketPath.c_str(), F_OK) != 0) {
        if (mkdir(bucketPath.c_str(), STAT_MODE_DIR) != 0) {
            LOGE("mkdir bucketpath failed :%{public}s err:%{public}d", bucketPath.c_str(), errno);
            return -errno;
        }
    }
    int32_t fd = open(path.c_str(), (mode & O_NOFOLLOW) | O_CREAT, STAT_MODE_REG);
    if (fd < 0) {
        LOGE("create file failed :%{public}s err:%{public}d", path.c_str(), errno);
        return -errno;
    }
    return fd;
}

void RemoveLocalFile(const string &path)
{
    int32_t err = remove(path.c_str());
    if (err != 0) {
        LOGE("remove file %{public}s failed, error:%{public}d", path.c_str(), errno);
    }
}

int32_t GenerateCloudId(int32_t userId, const string &bundleName, string &cloudId)
{
    shared_ptr<DKDatabase> dkDatabasePtr = GetDatabase(userId, bundleName);
    if (dkDatabasePtr == nullptr) {
        LOGE("Failed to get database");
        return ENOSYS;
    }

    vector<DKRecordId> ids;
    DKError dkErr = dkDatabasePtr->GenerateIds(1, ids);
    if (dkErr.dkErrorCode != DKLocalErrorCode::NO_ERROR || ids.size() != 1) {
        return ENOSYS;
    }
    cloudId = ids[0];
    return 0;
}

int32_t DoCreatFile(fuse_req_t req, fuse_ino_t parent, const char *name,
                    mode_t mode, struct fuse_entry_param &e)
{
    int32_t err = 0;
    string cloudId;
    struct CloudDiskFuseData *data =
        reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    struct CloudDiskInode *parentInode = reinterpret_cast<struct CloudDiskInode *>(parent);

    err = GenerateCloudId(data->userId, parentInode->bundleName, cloudId);
    if (err != 0) {
        LOGE("Failed to generate cloud id");
        return -err;
    }

    int32_t fd = CreateLocalFile(cloudId, parentInode->bundleName, data->userId, mode);
    if (fd < 0) {
        LOGD("Create local file failed error:%{public}d", fd);
        return fd;
    }

    DatabaseManager &databaseManager = DatabaseManager::GetInstance();
    string path = CloudFileUtils::GetLocalFilePath(cloudId, parentInode->bundleName, data->userId);
    shared_ptr<CloudDiskRdbStore> rdbStore =
        databaseManager.GetRdbStore(parentInode->bundleName, data->userId);
    err = rdbStore->Create(cloudId, parentInode->cloudId, name);
    if (err != 0) {
        close(fd);
        RemoveLocalFile(path);
        return -err;
    }

    err = DoCloudLookup(req, parent, name, &e);
    if (err != 0) {
        close(fd);
        RemoveLocalFile(path);
        return -err;
    }
    return fd;
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
    struct fuse_entry_param e;
    int32_t err = 0;

    err = DoCreatFile(req, parent, name, mode, e);
    if (err < 0) {
        fuse_reply_err(req, -err);
        return;
    }
    fi->fh = err;
    fuse_reply_create(req, &e, fi);
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
