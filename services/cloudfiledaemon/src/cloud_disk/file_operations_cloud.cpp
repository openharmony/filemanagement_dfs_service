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
#include "securec.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudDisk {
using namespace std;
using namespace DriveKit;
namespace {
    static const uint32_t STAT_NLINK_REG = 1;
    static const uint32_t STAT_NLINK_DIR = 2;
    static const uint32_t STAT_MODE_REG = 0770;
    static const uint32_t STAT_MODE_DIR = 0771;
    static const string HMDFS_PERMISSION_XATTR = "user.hmdfs.perm";
    static const uint32_t MILLISECOND_TO_SECONDS_TIMES = 1000;
}

static void InitInodeAttr(fuse_ino_t parent, struct CloudDiskInode *childInode,
                          const CloudDiskFileInfo &childInfo)
{
    struct CloudDiskInode *parentInode = reinterpret_cast<struct CloudDiskInode *>(parent);
    childInode->stat = parentInode->stat;
    childInode->stat.st_mtime = childInfo.mtime / MILLISECOND_TO_SECONDS_TIMES;
    childInode->stat.st_ctime = childInfo.ctime / MILLISECOND_TO_SECONDS_TIMES;
    childInode->stat.st_atime = childInfo.atime ? childInfo.atime / MILLISECOND_TO_SECONDS_TIMES :
                                childInode->stat.st_mtime;

    childInode->bundleName = parentInode->bundleName;
    childInode->layer = FileOperationsHelper::GetNextLayer(parent);
    childInode->parent = parent;
    childInode->cloudId = childInfo.cloudId;
    childInode->ops = make_shared<FileOperationsCloud>();
    childInode->stat.st_ino = reinterpret_cast<fuse_ino_t>(childInode);

    if (childInfo.IsDirectory) {
        childInode->stat.st_mode = S_IFDIR | STAT_MODE_DIR;
        childInode->stat.st_nlink = STAT_NLINK_DIR;
    } else {
        childInode->stat.st_mode = S_IFREG | STAT_MODE_REG;
        childInode->stat.st_nlink = STAT_NLINK_REG;
        childInode->stat.st_size = childInfo.size;
    }
}

static int32_t DoCloudLookup(fuse_req_t req, fuse_ino_t parent, const char *name,
                             struct fuse_entry_param *e)
{
    bool createFlag = false;
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    std::unique_lock<std::shared_mutex> wLock(data->cacheLock, std::defer_lock);

    if (parent == FUSE_ROOT_ID) {
        LOGE("cloud file operations should not get a fuse root inode");
        return ENOENT;
    }

    auto parentInode = reinterpret_cast<struct CloudDiskInode *>(parent);
    DatabaseManager &databaseManager = DatabaseManager::GetInstance();
    shared_ptr<CloudDiskRdbStore> rdbStore =
        databaseManager.GetRdbStore(parentInode->bundleName, data->userId);
    CloudDiskFileInfo childInfo;
    int32_t err = rdbStore->LookUp(parentInode->cloudId, name, childInfo);
    if (err != 0) {
        LOGE("lookup %{public}s error, err: %{public}d", name, err);
        return ENOENT;
    }

    shared_ptr<CloudDiskInode> child = FileOperationsHelper::FindCloudDiskInode(data, childInfo.cloudId);
    if (child == nullptr) {
        child = make_shared<CloudDiskInode>();
        createFlag = true;
    }

    child->refCount++;
    if (createFlag) {
        InitInodeAttr(parent, child.get(), childInfo);
        wLock.lock();
        data->inodeCache[childInfo.cloudId] = child;
        wLock.unlock();
    }
    e->ino = reinterpret_cast<fuse_ino_t>(child.get());
    FileOperationsHelper::GetInodeAttr(child, &e->attr);
    return 0;
}

void FileOperationsCloud::Lookup(fuse_req_t req, fuse_ino_t parent, const char *name)
{
    struct fuse_entry_param e;

    int32_t err = DoCloudLookup(req, parent, name, &e);
    if (err) {
        fuse_reply_err(req, err);
    } else {
        fuse_reply_entry(req, &e);
    }
}

void FileOperationsCloud::Access(fuse_req_t req, fuse_ino_t ino, int mask)
{
    LOGE("Access operation is not supported!");
    fuse_reply_err(req, ENOSYS);
}

void FileOperationsCloud::GetAttr(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi)
{
    auto inoPtr = reinterpret_cast<struct CloudDiskInode *>(ino);
    fuse_reply_attr(req, &inoPtr->stat, 0);
}

void FileOperationsCloud::Open(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi)
{
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    auto inoPtr = reinterpret_cast<struct CloudDiskInode *>(ino);

    string path = CloudFileUtils::GetLocalFilePath(inoPtr->cloudId, inoPtr->bundleName, data->userId);
    int32_t fd = open(path.c_str(), fi->flags & O_NOFOLLOW);
    if (fd < 0) {
        LOGE("open file failed path:%{public}s errno:%{public}d", path.c_str(), errno);
        return (void) fuse_reply_err(req, errno);
    }
    fi->fh = fd;
    fuse_reply_open(req, fi);
}

void FileOperationsCloud::Forget(fuse_req_t req, fuse_ino_t ino, uint64_t nLookup)
{
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    if (ino == FUSE_ROOT_ID) {
        LOGD("Cloud file operation should not get a root inode");
        return (void) fuse_reply_none(req);
    }
    auto inoPtr = reinterpret_cast<struct CloudDiskInode *>(ino);
    string key = inoPtr->cloudId;
    if (inoPtr->layer != CLOUD_DISK_INODE_OTHER_LAYER) {
        key = inoPtr->path;
    }
    shared_ptr<CloudDiskInode> node = FileOperationsHelper::FindCloudDiskInode(data, key);
    FileOperationsHelper::PutCloudDiskInode(data, node, nLookup);
    fuse_reply_none(req);
}

void FileOperationsCloud::ForgetMulti(fuse_req_t req, size_t count, struct fuse_forget_data *forgets)
{
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    for (size_t i = 0; i < count; i++) {
        if (forgets[i].ino == FUSE_ROOT_ID) {
            LOGD("Cloud file operation should not get a root inode");
            return (void) fuse_reply_none(req);
        }
        auto inoPtr = reinterpret_cast<struct CloudDiskInode *>(forgets[i].ino);
        string key = inoPtr->cloudId;
        if (inoPtr->layer != CLOUD_DISK_INODE_OTHER_LAYER) {
            key = inoPtr->path;
        }
        shared_ptr<CloudDiskInode> node = FileOperationsHelper::FindCloudDiskInode(data, key);
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
    int32_t fd = open(path.c_str(), (mode & O_NOFOLLOW) | O_CREAT | O_RDWR, STAT_MODE_REG);
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
    if (dkErr.dkErrorCode != DKLocalErrorCode::NO_ERROR || ids.size() == 0) {
        return ENOSYS;
    }
    cloudId = ids[0];
    return 0;
}

int32_t DoCreatFile(fuse_req_t req, fuse_ino_t parent, const char *name,
                    mode_t mode, struct fuse_entry_param &e)
{
    struct CloudDiskFuseData *data =
        reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    struct CloudDiskInode *parentInode = reinterpret_cast<struct CloudDiskInode *>(parent);

    string cloudId;
    int32_t err = GenerateCloudId(data->userId, parentInode->bundleName, cloudId);
    if (err != 0) {
        LOGE("Failed to generate cloud id");
        return -err;
    }

    int32_t fd = CreateLocalFile(cloudId, parentInode->bundleName, data->userId, mode);
    if (fd < 0) {
        LOGD("Create local file failed error:%{public}d", fd);
        return fd;
    }

    int32_t num = write(fd, cloudId.c_str(), cloudId.size());
    if (num != cloudId.size()) {
        LOGD("Write cloud id failed!");
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
    struct fuse_entry_param e;
    int32_t err = DoCreatFile(req, parent, name, mode, e);
    if (err < 0) {
        fuse_reply_err(req, -err);
        return;
    }
    close(err);
    fuse_reply_entry(req, &e);
}

void FileOperationsCloud::Create(fuse_req_t req, fuse_ino_t parent, const char *name,
                                 mode_t mode, struct fuse_file_info *fi)
{
    struct fuse_entry_param e;
    int32_t err = DoCreatFile(req, parent, name, mode, e);
    if (err < 0) {
        fuse_reply_err(req, -err);
        return;
    }
    fi->fh = err;
    fuse_reply_create(req, &e, fi);
}

static shared_ptr<CloudDiskInode> GenerateCloudDiskInode(struct CloudDiskFuseData *data, fuse_ino_t parent,
                                                         const CloudDiskFileInfo &childInfo)
{
    std::unique_lock<std::shared_mutex> wLock(data->cacheLock, std::defer_lock);
    shared_ptr<CloudDiskInode> child = make_shared<CloudDiskInode>();
    child->refCount++;
    InitInodeAttr(parent, child.get(), childInfo);
    wLock.lock();
    data->inodeCache[childInfo.cloudId] = child;
    wLock.unlock();
    return child;
}

void FileOperationsCloud::ReadDir(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off,
                                  struct fuse_file_info *fi)
{
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    auto inoPtr = reinterpret_cast<struct CloudDiskInode *>(ino);
    string parentCloudId = inoPtr->cloudId;

    vector<CloudDiskFileInfo> childInfos;
    DatabaseManager &databaseManager = DatabaseManager::GetInstance();
    shared_ptr<CloudDiskRdbStore> rdbStore = databaseManager.GetRdbStore(inoPtr->bundleName, data->userId);
    int32_t err = rdbStore->ReadDir(parentCloudId, childInfos);
    if (err != 0) {
        LOGE("Readdir failed cloudId:%{public}s err:%{public}d", parentCloudId.c_str(), err);
        return (void) fuse_reply_err(req, err);
    }

    string entryData;
    size_t len = 0;
    for (size_t i = 0; i < childInfos.size(); i++) {
        shared_ptr<CloudDiskInode> childPtr = FileOperationsHelper::FindCloudDiskInode(data, childInfos[i].cloudId);
        if (childPtr == nullptr) {
            childPtr = GenerateCloudDiskInode(data, ino, childInfos[i]);
        }
        FileOperationsHelper::AddDirEntry(req, entryData, len, childInfos[i].fileName.c_str(), childPtr);
    }
    FileOperationsHelper::FuseReplyLimited(req, entryData.c_str(), len, off, size);
    return;
}

static bool CheckIsHmdfsPermission(const string &key)
{
    return key == HMDFS_PERMISSION_XATTR;
}

void FileOperationsCloud::SetXattr(fuse_req_t req, fuse_ino_t ino, const char *name,
                                   const char *value, size_t size, int flags)
{
    LOGD("Setxattr begin name:%{public}s", name);
    if (CheckIsHmdfsPermission(name)) {
        fuse_reply_err(req, 0);
    } else {
        fuse_reply_err(req, ENOSYS);
    }
}

void FileOperationsCloud::GetXattr(fuse_req_t req, fuse_ino_t ino, const char *name,
                                   size_t size)
{
    LOGD("GetXattr begin name:%{public}s", name);
    if (!CheckIsHmdfsPermission(name)) {
        return (void) fuse_reply_err(req, ENOSYS);
    }
    auto inoPtr = reinterpret_cast<struct CloudDiskInode *>(ino);
    shared_ptr<char> value = make_shared<char>(size);
    string hmdfsLayer = to_string(inoPtr->layer + 2);
    memcpy_s(value.get(), size, hmdfsLayer.c_str(), hmdfsLayer.size());
    fuse_reply_buf(req, value.get(), 0);
    fuse_reply_err(req, 0);
}

void FileOperationsCloud::MkDir(fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode)
{
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    auto parentInode = reinterpret_cast<struct CloudDiskInode *>(parent);
    string cloudId;
    int32_t err = GenerateCloudId(data->userId, parentInode->bundleName, cloudId);
    if (err != 0) {
        LOGE("Failed to generate cloud id");
        return (void) fuse_reply_err(req, err);
    }

    DatabaseManager &databaseManager = DatabaseManager::GetInstance();
    shared_ptr<CloudDiskRdbStore> rdbStore = databaseManager.GetRdbStore(parentInode->bundleName,
                                                                         data->userId);
    err = rdbStore->MkDir(cloudId, parentInode->cloudId, name);
    if (err != 0) {
        LOGE("Failed to mkdir to DB err:%{public}d", err);
        return (void) fuse_reply_err(req, ENOSYS);
    }

    struct fuse_entry_param e;
    err = DoCloudLookup(req, parent, name, &e);
    if (err != 0) {
        LOGE("Faile to find dir %{private}s", name);
        return (void) fuse_reply_err(req, err);
    }
    fuse_reply_entry(req, &e);
    fuse_reply_err(req, 0);
}

int32_t DoCloudUnlink(fuse_req_t req, fuse_ino_t parent, const char *name)
{
    struct fuse_entry_param e;
    int32_t err = DoCloudLookup(req, parent, name, &e);
    if (err != 0) {
        return ENOENT;
    }

    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    auto parentInode = reinterpret_cast<struct CloudDiskInode *>(parent);
    DatabaseManager &databaseManager = DatabaseManager::GetInstance();
    shared_ptr<CloudDiskRdbStore> rdbStore = databaseManager.GetRdbStore(parentInode->bundleName,
                                                                         data->userId);
    err = rdbStore->Unlink(parentInode->cloudId, name);
    if (err != 0) {
        LOGE("Failed to unlink DB name:%{private}s err:%{public}d", name, err);
        return ENOSYS;
    }
    return 0;
}

void FileOperationsCloud::RmDir(fuse_req_t req, fuse_ino_t parent, const char *name)
{
    int32_t err = DoCloudUnlink(req, parent, name);
    if (err != 0) {
        fuse_reply_err(req, err);
        return;
    }
    return (void) fuse_reply_err(req, 0);
}

void FileOperationsCloud::Unlink(fuse_req_t req, fuse_ino_t parent, const char *name)
{
    int32_t err = DoCloudUnlink(req, parent, name);
    if (err != 0) {
        fuse_reply_err(req, err);
        return;
    }
    return (void) fuse_reply_err(req, 0);
}
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS
