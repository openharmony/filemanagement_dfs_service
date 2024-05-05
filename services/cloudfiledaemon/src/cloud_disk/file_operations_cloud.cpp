/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "account_status.h"
#include "cloud_disk_inode.h"
#include "cloud_file_utils.h"
#include "clouddisk_rdb_utils.h"
#include "clouddisk_notify.h"
#include "database_manager.h"
#include "directory_ex.h"
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
enum XATTR_CODE {
    ERROR_CODE = -1,
    HMDFS_PERMISSION,
    CLOUD_LOCATION,
    CLOUD_RECYCLE,
    IS_FAVORITE
};
namespace {
    static const uint32_t STAT_NLINK_REG = 1;
    static const uint32_t STAT_NLINK_DIR = 2;
    static const uint32_t CLOUD_FILE_LAYER = 2;
    static const uint32_t STAT_MODE_REG = 0660;
    static const uint32_t STAT_MODE_DIR = 0771;
    static const uint32_t MILLISECOND_TO_SECONDS_TIMES = 1000;
    static const string FILE_LOCAL = "1";
    static const float LOOKUP_TIMEOUT = 60.0;
}

static void InitInodeAttr(struct CloudDiskFuseData *data, shared_ptr<CloudDiskInode> parentInode,
    struct CloudDiskInode *childInode, const CloudDiskFileInfo &childInfo)
{
    childInode->stat = parentInode->stat;
    childInode->stat.st_mtime = childInfo.mtime / MILLISECOND_TO_SECONDS_TIMES;
    childInode->stat.st_ctime = childInfo.ctime / MILLISECOND_TO_SECONDS_TIMES;
    childInode->stat.st_atime = childInfo.atime / MILLISECOND_TO_SECONDS_TIMES;

    childInode->bundleName = parentInode->bundleName;
    childInode->fileName = childInfo.fileName;
    childInode->layer = FileOperationsHelper::GetNextLayer(parentInode, parentInode->parent);
    childInode->parent = parentInode->parent;
    childInode->cloudId = childInfo.cloudId;
    childInode->ops = make_shared<FileOperationsCloud>();
    childInode->stat.st_ino = childInfo.localId;

    if (childInfo.IsDirectory) {
        childInode->stat.st_mode = S_IFDIR | STAT_MODE_DIR;
        childInode->stat.st_nlink = STAT_NLINK_DIR;
    } else {
        childInode->stat.st_mode = S_IFREG | STAT_MODE_REG;
        childInode->stat.st_nlink = STAT_NLINK_REG;
        childInode->stat.st_size = childInfo.size;
    }
}

static shared_ptr<CloudDiskFile> InitFileAttr(struct CloudDiskFuseData *data, struct fuse_file_info *fi)
{
    std::unique_lock<std::shared_mutex> wLock(data->fileLock, std::defer_lock);
    shared_ptr<CloudDiskFile> filePtr = FileOperationsHelper::FindCloudDiskFile(data, fi->fh);
    if (filePtr == nullptr) {
        filePtr = make_shared<CloudDiskFile>();
        wLock.lock();
        data->fileCache[fi->fh] = filePtr;
        wLock.unlock();
    }
    filePtr->refCount++;
    return filePtr;
}

static void InitLocalIdCache(struct CloudDiskFuseData *data, const string &key, int64_t val)
{
    std::unique_lock<std::shared_mutex> wLock(data->localIdLock, std::defer_lock);
    int64_t localId = FileOperationsHelper::FindLocalId(data, key);
    if (localId == -1) {
        wLock.lock();
        data->localIdCache[key] = val;
        wLock.unlock();
    }
}

static int32_t DoCloudLookup(fuse_req_t req, fuse_ino_t parent, const char *name,
                             struct fuse_entry_param *e)
{
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    if (parent == FUSE_ROOT_ID) {
        LOGE("cloud file operations should not get a fuse root inode");
        return ENOENT;
    }

    auto parentInode = FileOperationsHelper::FindCloudDiskInode(data, static_cast<int64_t>(parent));
    if (parentInode == nullptr) {
        LOGE("parent inode not found");
        return EINVAL;
    }
    DatabaseManager &databaseManager = DatabaseManager::GetInstance();
    shared_ptr<CloudDiskRdbStore> rdbStore =
        databaseManager.GetRdbStore(parentInode->bundleName, data->userId);
    CloudDiskFileInfo childInfo;
    int32_t err = rdbStore->LookUp(parentInode->cloudId, name, childInfo);
    if (err != 0) {
        LOGE("lookup %{public}s error, err: %{public}d", name, err);
        return ENOENT;
    }

    string key = std::to_string(parent) + name;
    std::unique_lock<std::shared_mutex> cWLock(data->cacheLock, std::defer_lock);
    std::unique_lock<std::shared_mutex> lWLock(data->localIdLock, std::defer_lock);
    auto child = FileOperationsHelper::FindCloudDiskInode(data, childInfo.localId);
    if (child == nullptr) {
        child = make_shared<CloudDiskInode>();
        cWLock.lock();
        data->inodeCache[childInfo.localId] = child;
        cWLock.unlock();
    } else {
        string oldKey = std::to_string(child->parent) + child->fileName;
        lWLock.lock();
        data->localIdCache.erase(oldKey);
        lWLock.unlock();
    }
    child->refCount++;
    InitInodeAttr(data, parentInode, child.get(), childInfo);
    InitLocalIdCache(data, key, childInfo.localId);
    e->ino = static_cast<fuse_ino_t>(childInfo.localId);
    FileOperationsHelper::GetInodeAttr(child, &e->attr);
    return 0;
}

void FileOperationsCloud::Lookup(fuse_req_t req, fuse_ino_t parent, const char *name)
{
    struct fuse_entry_param e;
    e.attr_timeout = LOOKUP_TIMEOUT;
    e.entry_timeout = LOOKUP_TIMEOUT;
    int32_t err = DoCloudLookup(req, parent, name, &e);
    if (err) {
        fuse_reply_err(req, err);
    } else {
        fuse_reply_entry(req, &e);
    }
}

void FileOperationsCloud::Access(fuse_req_t req, fuse_ino_t ino, int mask)
{
    LOGI("Access operation is not supported!");
    fuse_reply_err(req, ENOSYS);
}

void FileOperationsCloud::GetAttr(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi)
{
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    auto inoPtr = FileOperationsHelper::FindCloudDiskInode(data, static_cast<int64_t>(ino));
    if (inoPtr == nullptr) {
        LOGE("inode not found");
        fuse_reply_err(req, EINVAL);
        return;
    }
    fuse_reply_attr(req, &inoPtr->stat, 0);
}

static bool HandleDkError(fuse_req_t req, DriveKit::DKError dkError)
{
    if (!dkError.HasError()) {
        return false;
    }
    if ((dkError.serverErrorCode == static_cast<int>(DriveKit::DKServerErrorCode::NETWORK_ERROR))
        || dkError.dkErrorCode == DriveKit::DKLocalErrorCode::DOWNLOAD_REQUEST_ERROR) {
        LOGE("network error");
        fuse_reply_err(req, ENOTCONN);
    } else if (dkError.isServerError) {
        LOGE("server errorCode is: %{public}d", dkError.serverErrorCode);
        fuse_reply_err(req, EIO);
    } else if (dkError.isLocalError) {
        LOGE("local errorCode is: %{public}d", dkError.dkErrorCode);
        fuse_reply_err(req, EINVAL);
    }
    return true;
}

static shared_ptr<DriveKit::DKDatabase> GetDatabase(int32_t userId, string bundleName)
{
    auto driveKit = DriveKit::DriveKitNative::GetInstance(userId);
    if (driveKit == nullptr) {
        LOGE("sdk helper get drive kit instance fail");
        return nullptr;
    }

    if (AccountStatus::IsNeedCleanCache()) {
        driveKit->CleanCloudUserInfo();
        LOGI("execute clean cloud user info success");
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

static void CloudOpen(fuse_req_t req, shared_ptr<CloudDiskInode> inoPtr, struct fuse_file_info *fi, string path)
{
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    auto filePtr = FileOperationsHelper::FindCloudDiskFile(data, data->fileId.load());
    if (filePtr == nullptr) {
        filePtr = InitFileAttr(data, fi);
    }
    shared_ptr<DriveKit::DKDatabase> database = GetDatabase(data->userId, inoPtr->bundleName);

    if (!database) {
        fuse_reply_err(req, EPERM);
        LOGE("database is null");
        return;
    }

    if (filePtr->readSession) {
        filePtr->type = CLOUD_DISK_FILE_TYPE_CLOUD;
        fuse_reply_open(req, fi);
        return;
    }

    string cloudId = inoPtr->cloudId;
    LOGD("cloudId: %s", cloudId.c_str());
    filePtr->readSession = database->NewAssetReadSession("file", cloudId, "content", path);
    if (filePtr->readSession) {
        DriveKit::DKError dkError = filePtr->readSession->InitSession();
        if (!HandleDkError(req, dkError)) {
            filePtr->type = CLOUD_DISK_FILE_TYPE_CLOUD;
            fuse_reply_open(req, fi);
        } else {
            filePtr->readSession = nullptr;
            fuse_reply_err(req, EPERM);
            LOGE("open fail");
        }
    } else {
        fuse_reply_err(req, EPERM);
        LOGE("readSession is null");
    }
    return;
}

void FileOperationsCloud::Open(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi)
{
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    data->fileId++;
    fi->fh = data->fileId.load();
    auto inoPtr = FileOperationsHelper::FindCloudDiskInode(data, static_cast<int64_t>(ino));
    if (inoPtr == nullptr) {
        LOGE("inode not found");
        fuse_reply_err(req, EINVAL);
        return;
    }
    string path = CloudFileUtils::GetLocalFilePath(inoPtr->cloudId, inoPtr->bundleName, data->userId);
    if (access(path.c_str(), F_OK) == 0) {
        if ((fi->flags & O_ACCMODE) & O_WRONLY) {
            fi->flags &= ~O_WRONLY;
            fi->flags |= O_RDWR;
        }
        if (fi->flags & O_APPEND) {
            fi->flags &= ~O_APPEND;
        }
        if (fi->flags & O_DIRECT) {
            fi->flags &= ~O_DIRECT;
        }
        int32_t fd = open(path.c_str(), fi->flags);
        if (fd < 0) {
            LOGE("open file failed path:%{public}s errno:%{public}d", path.c_str(), errno);
            return (void) fuse_reply_err(req, errno);
        }
        auto filePtr = InitFileAttr(data, fi);
        filePtr->type = CLOUD_DISK_FILE_TYPE_LOCAL;
        filePtr->fd = fd;
        fuse_reply_open(req, fi);
    } else {
        CloudOpen(req, inoPtr, fi, path);
    }
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
    auto parentInode = FileOperationsHelper::FindCloudDiskInode(data, static_cast<int64_t>(parent));
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
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    struct fuse_entry_param e;
    int32_t err = DoCreatFile(req, parent, name, mode, e);
    if (err < 0) {
        fuse_reply_err(req, -err);
        return;
    }
    auto filePtr = InitFileAttr(data, fi);
    data->fileId++;
    fi->fh = data->fileId;
    filePtr->fd = err;
    filePtr->type = CLOUD_DISK_FILE_TYPE_LOCAL;
    filePtr->fileDirty = CLOUD_DISK_FILE_CREATE;
    fuse_reply_create(req, &e, fi);
}

static size_t FindNextPos(const vector<CloudDiskFileInfo> &childInfos, off_t off)
{
    for (size_t i = 0; i < childInfos.size(); i++) {
        /* Find the first valid offset beyond @off */
        if (childInfos[i].nextOff > off) {
            return i + 1;
        }
    }
    /* If @off is beyond all valid offset, then return the index after the last info */
    if (!childInfos.empty() && childInfos.back().nextOff < off) {
        return childInfos.size();
    }
    return 0;
}

static int32_t GetChildInfos(fuse_req_t req, fuse_ino_t ino, vector<CloudDiskFileInfo> &childInfos)
{
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    auto inoPtr = FileOperationsHelper::FindCloudDiskInode(data, static_cast<int64_t>(ino));
    if (inoPtr == nullptr) {
        LOGE("inode not found");
        return EINVAL;
    }
    string parentCloudId = inoPtr->cloudId;

    DatabaseManager &databaseManager = DatabaseManager::GetInstance();
    shared_ptr<CloudDiskRdbStore> rdbStore = databaseManager.GetRdbStore(inoPtr->bundleName, data->userId);
    int32_t err = rdbStore->ReadDir(parentCloudId, childInfos);
    if (err != 0) {
        LOGE("Readdir failed cloudId:%{public}s err:%{public}d", parentCloudId.c_str(), err);
        return err;
    }
    return 0;
}

static size_t CloudSeekDir(fuse_req_t req, fuse_ino_t ino, off_t off,
                           const vector<CloudDiskFileInfo> &childInfos)
{
    if (off == 0 || childInfos.empty()) {
        return 0;
    }

    size_t i = 0;
    for (; i < childInfos.size(); i++) {
        if (childInfos[i].nextOff == off) {
            /* Start position should be the index of next entry */
            return i + 1;
        }
    }
    if (i == childInfos.size()) {
        /* The directory may changed recently, find the next valid index for this offset */
        return FindNextPos(childInfos, off);
    }

    return 0;
}

void FileOperationsCloud::ReadDir(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off,
                                  struct fuse_file_info *fi)
{
    vector<CloudDiskFileInfo> childInfos;
    int32_t err = GetChildInfos(req, ino, childInfos);
    if (err != 0) {
        return (void)fuse_reply_err(req, err);
    }

    size_t startPos = CloudSeekDir(req, ino, off, childInfos);
    string buf;
    buf.resize(size);
    if (childInfos.empty() || startPos == childInfos.size()) {
        return (void)fuse_reply_buf(req, buf.c_str(), 0);
    }

    size_t nextOff = 0;
    size_t remain = size;
    static const struct stat STAT_INFO_DIR = { .st_mode = S_IFDIR | STAT_MODE_DIR };
    static const struct stat STAT_INFO_REG = { .st_mode = S_IFREG | STAT_MODE_REG };
    for (size_t i = startPos; i < childInfos.size(); i++) {
        size_t alignSize = CloudDiskRdbUtils::FuseDentryAlignSize(childInfos[i].fileName.c_str());
        if (alignSize > remain) {
            break;
        }
        alignSize = fuse_add_direntry(req, &buf[nextOff], alignSize, childInfos[i].fileName.c_str(),
            childInfos[i].IsDirectory ? &STAT_INFO_DIR : &STAT_INFO_REG,
            off + static_cast<off_t>(nextOff) + static_cast<off_t>(alignSize));
        nextOff += alignSize;
        remain -= alignSize;
    }
    (void)fuse_reply_buf(req, buf.c_str(), size - remain);
}

int32_t CheckXattr(const char *name)
{
    LOGD("start CheckXattr name is:%{public}s", name);
    if (CloudFileUtils::CheckIsHmdfsPermission(name)) {
        return HMDFS_PERMISSION;
    } else if (CloudFileUtils::CheckIsCloudLocation(name)) {
        return CLOUD_LOCATION;
    } else if (CloudFileUtils::CheckIsCloudRecycle(name)) {
        return CLOUD_RECYCLE;
    } else if (CloudFileUtils::CheckIsFavorite(name)) {
        return IS_FAVORITE;
    } else {
        LOGD("no definition Xattr name:%{public}s", name);
        return ERROR_CODE;
    }
}

void HandleCloudLocation(fuse_req_t req, fuse_ino_t ino, const char *name,
                         const char *value)
{
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    auto inoPtr = FileOperationsHelper::FindCloudDiskInode(data, static_cast<int64_t>(ino));
    if (inoPtr == nullptr) {
        fuse_reply_err(req, EINVAL);
        LOGE("inode not found");
        return;
    }
    DatabaseManager &databaseManager = DatabaseManager::GetInstance();
    auto rdbStore = databaseManager.GetRdbStore(inoPtr->bundleName, data->userId);
    int32_t err = rdbStore->SetXAttr(inoPtr->cloudId, CLOUD_FILE_LOCATION, value);
    if (err != 0) {
        LOGE("set cloud id fail %{public}d", err);
        fuse_reply_err(req, EINVAL);
        return;
    }
    CloudDiskNotify::GetInstance().TryNotify({data, FileOperationsHelper::FindCloudDiskInode,
        NotifyOpsType::DAEMON_SETXATTR, inoPtr});
    fuse_reply_err(req, 0);
}

void HandleCloudRecycle(fuse_req_t req, fuse_ino_t ino, const char *name,
                        const char *value)
{
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    auto inoPtr = FileOperationsHelper::FindCloudDiskInode(data, static_cast<int64_t>(ino));
    if (inoPtr == nullptr) {
        fuse_reply_err(req, EINVAL);
        LOGE("inode not found");
        return;
    }
    DatabaseManager &databaseManager = DatabaseManager::GetInstance();
    auto rdbStore = databaseManager.GetRdbStore(inoPtr->bundleName, data->userId);
    int32_t err = rdbStore->SetXAttr(inoPtr->cloudId, CLOUD_CLOUD_RECYCLE_XATTR, value);
    if (err != 0) {
        LOGE("set cloud id fail %{public}d", err);
        fuse_reply_err(req, EINVAL);
        return;
    }
    CloudDiskNotify::GetInstance().TryNotify({data, FileOperationsHelper::FindCloudDiskInode,
        NotifyOpsType::DAEMON_SETXATTR, inoPtr});
    fuse_reply_err(req, 0);
}

void HandleFavorite(fuse_req_t req, fuse_ino_t ino, const char *name,
                    const char *value)
{
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    auto inoPtr = FileOperationsHelper::FindCloudDiskInode(data, static_cast<int64_t>(ino));
    if (inoPtr == nullptr) {
        fuse_reply_err(req, EINVAL);
        LOGE("inode not found");
        return;
    }
    DatabaseManager &databaseManager = DatabaseManager::GetInstance();
    auto rdbStore = databaseManager.GetRdbStore(inoPtr->bundleName, data->userId);
    int32_t err = rdbStore->SetXAttr(inoPtr->cloudId, IS_FAVORITE_XATTR, value);
    if (err != 0) {
        LOGE("set cloud id fail %{public}d", err);
        fuse_reply_err(req, EINVAL);
        return;
    }
    CloudDiskNotify::GetInstance().TryNotify({data, FileOperationsHelper::FindCloudDiskInode,
        NotifyOpsType::DAEMON_SETXATTR, inoPtr});
    fuse_reply_err(req, 0);
}

void FileOperationsCloud::SetXattr(fuse_req_t req, fuse_ino_t ino, const char *name,
                                   const char *value, size_t size, int flags)
{
    LOGD("Setxattr begin name:%{public}s", name);
    int32_t checknum = CheckXattr(name);
    switch (checknum) {
        case HMDFS_PERMISSION:
            fuse_reply_err(req, 0);
            break;
        case CLOUD_LOCATION:
            HandleCloudLocation(req, ino, name, value);
            break;
        case CLOUD_RECYCLE:
            HandleCloudRecycle(req, ino, name, value);
            break;
        case IS_FAVORITE:
            HandleFavorite(req, ino, name, value);
            break;
        default:
            fuse_reply_err(req, EINVAL);
            break;
    }
}

string GetIsFavorite(fuse_req_t req, struct CloudDiskInode *inoPtr)
{
    string favorite;
    DatabaseManager &databaseManager = DatabaseManager::GetInstance();
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    auto rdbStore = databaseManager.GetRdbStore(inoPtr->bundleName, data->userId);
    int res = rdbStore->GetXAttr(inoPtr->cloudId, IS_FAVORITE_XATTR, favorite);
    if (res != 0) {
        LOGE("local file get location fail");
        return "null";
    }
    return favorite;
}

static string GetFileStatus(fuse_req_t req, struct CloudDiskInode *inoPtr)
{
    string fileStatus;
    if (inoPtr == nullptr) {
        LOGE("inoPtr is null");
        return "null";
    }
    DatabaseManager &databaseManager = DatabaseManager::GetInstance();
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    auto rdbStore = databaseManager.GetRdbStore(inoPtr->bundleName, data->userId);
    int res = rdbStore->GetXAttr(inoPtr->cloudId, IS_FILE_STATUS_XATTR, fileStatus);
    if (res != 0) {
        LOGE("local file get file_status fail");
        return "null";
    }
    return fileStatus;
}

void FileOperationsCloud::GetXattr(fuse_req_t req, fuse_ino_t ino, const char *name,
                                   size_t size)
{
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    auto inoPtr = FileOperationsHelper::FindCloudDiskInode(data, static_cast<int64_t>(ino));
    if (inoPtr == nullptr) {
        fuse_reply_err(req, EINVAL);
        LOGE("inode not found");
        return;
    }
    string buf;
    if (CloudFileUtils::CheckIsHmdfsPermission(name)) {
        buf = to_string(inoPtr->layer + CLOUD_FILE_LAYER);
    } else if (CloudFileUtils::CheckIsCloud(name)) {
        buf = inoPtr->cloudId;
    } else if (CloudFileUtils::CheckIsFavorite(name)) {
        buf = GetIsFavorite(req, inoPtr.get());
    } else if (CloudFileUtils::CheckFileStatus(name)) {
        buf = GetFileStatus(req, inoPtr.get());
    } else {
        fuse_reply_err(req, EINVAL);
        return;
    }
    if (buf == "null") {
        fuse_reply_err(req, ENODATA);
        return;
    }
    if (size == 0) {
        fuse_reply_xattr(req, buf.size());
        return;
    }
    if (buf.size() > size) {
        fuse_reply_err(req, ERANGE);
        return;
    }
    if (buf.size() > 0) {
        fuse_reply_buf(req, buf.c_str(), buf.size());
    } else {
        fuse_reply_err(req, 0);
    }
}

void FileOperationsCloud::MkDir(fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode)
{
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    auto parentInode = FileOperationsHelper::FindCloudDiskInode(data, static_cast<int64_t>(parent));
    if (parentInode == nullptr) {
        fuse_reply_err(req, EINVAL);
        LOGE("parent inode not found");
        return;
    }
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
        LOGE("Failed to find dir %{private}s", name);
        fuse_reply_err(req, err);
    } else {
        fuse_reply_entry(req, &e);
    }
    CloudDiskNotify::GetInstance().TryNotify({data, FileOperationsHelper::FindCloudDiskInode,
        NotifyOpsType::DAEMON_MKDIR, parentInode, parent, name});
}

int32_t DoCloudUnlink(fuse_req_t req, fuse_ino_t parent, const char *name)
{
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    auto parentInode = FileOperationsHelper::FindCloudDiskInode(data, static_cast<int64_t>(parent));
    if (parentInode == nullptr) {
        LOGE("parent inode not found");
        return EINVAL;
    }
    DatabaseManager &databaseManager = DatabaseManager::GetInstance();
    shared_ptr<CloudDiskRdbStore> rdbStore = databaseManager.GetRdbStore(parentInode->bundleName,
                                                                         data->userId);
    string unlinkCloudId = "";
    int32_t err = rdbStore->Unlink(parentInode->cloudId, name, unlinkCloudId);
    if (err != 0) {
        LOGE("Failed to unlink DB cloudId:%{private}s err:%{public}d", unlinkCloudId.c_str(), err);
        return ENOSYS;
    }
    if (unlinkCloudId.empty()) {
        return 0;
    }
    err = unlink(CloudFileUtils::GetLocalFilePath(unlinkCloudId, parentInode->bundleName, data->userId).c_str());
    if (err != 0) {
        LOGE("Failed to unlink cloudId:%{private}s err:%{public}d", unlinkCloudId.c_str(), errno);
        return errno;
    }
    return 0;
}

static int32_t GetHasChild(fuse_req_t req, fuse_ino_t parent, bool &hasChild)
{
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    auto parentInode = FileOperationsHelper::FindCloudDiskInode(data, static_cast<int64_t>(parent));
    if (parentInode == nullptr) {
        LOGE("inode not found");
        return EINVAL;
    }
    string parentCloudId = parentInode->cloudId;
    DatabaseManager &databaseManager = DatabaseManager::GetInstance();
    shared_ptr<CloudDiskRdbStore> rdbStore = databaseManager.GetRdbStore(parentInode->bundleName,
                                                                         data->userId);
    int32_t err = rdbStore->GetHasChild(parentCloudId, hasChild);
    if (err != NativeRdb::E_OK) {
        LOGE("rdbStore get hasChild err:%{public}d", err);
        return err;
    }
    return 0;
}

void FileOperationsCloud::RmDir(fuse_req_t req, fuse_ino_t parent, const char *name)
{
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    string key = std::to_string(parent) + name;
    int64_t localId = FileOperationsHelper::FindLocalId(data, key);
    bool hasChild = false;
    int32_t err = GetHasChild(req, static_cast<fuse_ino_t>(localId), hasChild);
    if (err != 0) {
        fuse_reply_err(req, err);
        LOGE("failed to get hasChild");
        return;
    }
    if (hasChild) {
        fuse_reply_err(req, EPERM);
        LOGE("Directory not empty");
        return;
    }
    err = DoCloudUnlink(req, parent, name);
    if (err != 0) {
        fuse_reply_err(req, err);
        return;
    }
    CloudDiskNotify::GetInstance().TryNotify({data, FileOperationsHelper::FindCloudDiskInode,
        NotifyOpsType::DAEMON_RMDIR, nullptr, parent, name});
    return (void) fuse_reply_err(req, 0);
}

void FileOperationsCloud::Unlink(fuse_req_t req, fuse_ino_t parent, const char *name)
{
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    int32_t err = DoCloudUnlink(req, parent, name);
    if (err != 0) {
        fuse_reply_err(req, err);
        return;
    }
    CloudDiskNotify::GetInstance().TryNotify({data, FileOperationsHelper::FindCloudDiskInode,
        NotifyOpsType::DAEMON_UNLINK, nullptr, parent, name});
    return (void) fuse_reply_err(req, 0);
}

void FileOperationsCloud::Rename(fuse_req_t req, fuse_ino_t parent, const char *name,
                                 fuse_ino_t newParent, const char *newName, unsigned int flags)
{
    if (flags) {
        LOGE("Fuse failed to support flag");
        fuse_reply_err(req, EINVAL);
        return;
    }
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    auto parentInode = FileOperationsHelper::FindCloudDiskInode(data, static_cast<int64_t>(parent));
    auto newParentInode = FileOperationsHelper::FindCloudDiskInode(data, static_cast<int64_t>(newParent));
    if (!parentInode || !newParentInode) {
        fuse_reply_err(req, EINVAL);
        LOGE("rename old parent or new parent not found");
        return;
    }

    DatabaseManager &databaseManager = DatabaseManager::GetInstance();
    shared_ptr<CloudDiskRdbStore> rdbStore = databaseManager.GetRdbStore(parentInode->bundleName,
                                                                         data->userId);
    int32_t err = rdbStore->Rename(parentInode->cloudId, name, newParentInode->cloudId, newName);
    if (err != 0) {
        fuse_reply_err(req, err);
        LOGE("Failed to Rename DB name:%{private}s err:%{public}d", name, err);
        return;
    }
    int32_t isDir;
    err = rdbStore->GetIsDirectory(newParentInode->cloudId, newName, isDir);
    if (err != 0) {
        LOGE("get isDir fail, err: %{public}d", err);
    }
    CloudDiskNotify::GetInstance().TryNotify({data, FileOperationsHelper::FindCloudDiskInode,
        NotifyOpsType::DAEMON_RENAME, nullptr, parent, name, newParent, newName}, {FileStatus::UNKNOW, isDir});
    return (void) fuse_reply_err(req, 0);
}

void FileOperationsCloud::Read(fuse_req_t req, fuse_ino_t ino, size_t size,
                               off_t offset, struct fuse_file_info *fi)
{
    const size_t MAX_SIZE = 2 * 1024 * 1024;
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    auto filePtr = FileOperationsHelper::FindCloudDiskFile(data, data->fileId.load());
    if (filePtr == nullptr) {
        fuse_reply_err(req, EINVAL);
        LOGE("file not found");
        return;
    }
    if (filePtr->type == CLOUD_DISK_FILE_TYPE_LOCAL) {
        struct fuse_bufvec buf = FUSE_BUFVEC_INIT(size);

        buf.buf[0].flags = static_cast<fuse_buf_flags> (FUSE_BUF_IS_FD | FUSE_BUF_FD_SEEK);
        buf.buf[0].fd = filePtr->fd;
        buf.buf[0].pos = offset;

        fuse_reply_data(req, &buf, static_cast<fuse_buf_copy_flags> (0));
        return;
    }

    int64_t readSize;
    DriveKit::DKError dkError;
    shared_ptr<char> buf = nullptr;

    if (size > MAX_SIZE) {
        LOGE("Size too large");
        return;
    }

    buf.reset(new char[size], [](char* ptr) {
        delete[] ptr;
    });

    if (!buf) {
        fuse_reply_err(req, ENOMEM);
        LOGE("buffer is null");
        return;
    }

    readSize = filePtr->readSession->PRead(offset, size, buf.get(), dkError);
    if (!HandleDkError(req, dkError)) {
        LOGD("read success, %lld bytes", static_cast<long long>(readSize));
        fuse_reply_buf(req, buf.get(), readSize);
    } else {
        LOGE("read fail");
    }
}

static void UpdateCloudDiskInode(shared_ptr<CloudDiskRdbStore> rdbStore, shared_ptr<CloudDiskInode> inoPtr)
{
    CloudDiskFileInfo childInfo;
    int32_t err = rdbStore->GetAttr(inoPtr->cloudId, childInfo);
    if (err != 0) {
        LOGE("update file fail");
        return;
    }
    inoPtr->stat.st_size = childInfo.size;
    inoPtr->stat.st_mtime = childInfo.mtime / MILLISECOND_TO_SECONDS_TIMES;
}

static void UpdateCloudStore(struct CloudDiskFuseData *data, shared_ptr<CloudDiskInode> inoPtr)
{
    int32_t userId = data->userId;
    DatabaseManager &databaseManager = DatabaseManager::GetInstance();
    auto rdbStore = databaseManager.GetRdbStore(inoPtr->bundleName, userId);
    int32_t dirtyType;
    int res = rdbStore->GetDirtyType(inoPtr->cloudId, dirtyType);
    if (res != 0) {
        LOGE("get file status fail, err: %{public}d", res);
    }
    res = rdbStore->Write(inoPtr->cloudId);
    if (res != 0) {
        LOGE("write file fail");
    }
    CloudDiskNotify::GetInstance().TryNotify({data, FileOperationsHelper::FindCloudDiskInode,
        NotifyOpsType::DAEMON_WRITE, inoPtr}, {dirtyType});
    UpdateCloudDiskInode(rdbStore, inoPtr);
}

void FileOperationsCloud::WriteBuf(fuse_req_t req, fuse_ino_t ino, struct fuse_bufvec *bufv,
                                   off_t off, struct fuse_file_info *fi)
{
    struct fuse_bufvec out_buf = FUSE_BUFVEC_INIT(fuse_buf_size(bufv));
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    auto filePtr = FileOperationsHelper::FindCloudDiskFile(data, fi->fh);
    if (filePtr == nullptr) {
        fuse_reply_err(req, EINVAL);
        LOGE("file not found");
        return;
    }
    if (filePtr->type != CLOUD_DISK_FILE_TYPE_LOCAL) {
        fuse_reply_err(req, EINVAL);
        LOGE("write on cloud file not supported");
        return;
    }
    out_buf.buf[0].flags = (fuse_buf_flags)(FUSE_BUF_IS_FD | FUSE_BUF_FD_SEEK);
    out_buf.buf[0].fd = filePtr->fd;
    out_buf.buf[0].pos = off;
    int res = fuse_buf_copy(&out_buf, bufv, (fuse_buf_copy_flags)(0));
    if (res < 0) {
        fuse_reply_err(req, -res);
    } else {
        if (filePtr != nullptr) { filePtr->fileDirty = CLOUD_DISK_FILE_WRITE; }
        fuse_reply_write(req, (size_t) res);
    }
}

static void UploadLocalFile(struct CloudDiskFuseData *data, shared_ptr<CloudDiskInode> inoPtr)
{
    string location;
    int32_t userId = data->userId;
    DatabaseManager &databaseManager = DatabaseManager::GetInstance();
    auto rdbStore = databaseManager.GetRdbStore(inoPtr->bundleName, userId);
    int res = rdbStore->GetXAttr(inoPtr->cloudId, CLOUD_FILE_LOCATION, location);
    if (res != 0) {
        LOGE("local file get location fail");
    } else if (location == FILE_LOCAL) {
        int32_t dirtyType;
        res = rdbStore->GetDirtyType(inoPtr->cloudId, dirtyType);
        if (res != 0) {
            LOGE("get file status fail, err: %{public}d", res);
        }
        res = rdbStore->Write(inoPtr->cloudId);
        if (res != 0) {
            LOGE("write file fail");
        }
        CloudDiskNotify::GetInstance().TryNotify({data, FileOperationsHelper::FindCloudDiskInode,
            NotifyOpsType::DAEMON_WRITE, inoPtr}, {dirtyType});
        UpdateCloudDiskInode(rdbStore, inoPtr);
    }
}

void FileOperationsCloud::Release(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi)
{
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    auto inoPtr = FileOperationsHelper::FindCloudDiskInode(data, static_cast<int64_t>(ino));
    if (inoPtr == nullptr) {
        fuse_reply_err(req, EINVAL);
        LOGE("inode not found");
        return;
    }
    auto filePtr = FileOperationsHelper::FindCloudDiskFile(data, fi->fh);
    if (filePtr == nullptr) {
        fuse_reply_err(req, EINVAL);
        LOGE("file not found");
        return;
    }
    filePtr->refCount--;
    if (filePtr->refCount == 0) {
        if (filePtr->type == CLOUD_DISK_FILE_TYPE_LOCAL) {
            close(filePtr->fd);
            if (filePtr->fileDirty != CLOUD_DISK_FILE_UNKNOWN) {
                UpdateCloudStore(data, inoPtr);
            } else {
                UploadLocalFile(data, inoPtr);
            }
        } else if (filePtr->type == CLOUD_DISK_FILE_TYPE_CLOUD &&
            filePtr->readSession != nullptr) {
            bool res = filePtr->readSession->Close(false);
            if (!res) {
                fuse_reply_err(req, ENOSYS);
                LOGE("close error");
                return;
            }
            filePtr->readSession = nullptr;
            LOGD("readSession released");
        }
        FileOperationsHelper::PutCloudDiskFile(data, filePtr, fi->fh);
    }
    fuse_reply_err(req, 0);
}

void FileOperationsCloud::SetAttr(fuse_req_t req, fuse_ino_t ino, struct stat *attr,
                                  int valid, struct fuse_file_info *fi)
{
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    auto inoPtr = FileOperationsHelper::FindCloudDiskInode(data, static_cast<int64_t>(ino));
    if (inoPtr == nullptr) {
        LOGE("get an invalid inode!");
        fuse_reply_err(req, EINVAL);
        return;
    }

    if (static_cast<unsigned int>(valid) & FUSE_SET_ATTR_SIZE) {
        auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
        DatabaseManager &databaseManager = DatabaseManager::GetInstance();
        auto rdbStore = databaseManager.GetRdbStore(inoPtr->bundleName, data->userId);

        int res = rdbStore->SetAttr(inoPtr->cloudId, attr->st_size);
        if (res != 0) {
            LOGE("update rdb size failed, res: %{public}d", res);
            fuse_reply_err(req, ENOSYS);
            return;
        }

        if (fi) {
            auto filePtr = FileOperationsHelper::FindCloudDiskFile(data, fi->fh);
            if (filePtr == nullptr) {
                fuse_reply_err(req, EINVAL);
                LOGE("file not found");
                return;
            }
            res = ftruncate(filePtr->fd, attr->st_size);
        } else {
            string path = CloudFileUtils::GetLocalFilePath(inoPtr->cloudId, inoPtr->bundleName, data->userId);
            res = truncate(path.c_str(), attr->st_size);
        }
        if (res == -1) {
            LOGE("truncate failed, err: %{public}d", errno);
            res = rdbStore->SetAttr(inoPtr->cloudId, inoPtr->stat.st_size);
            if (res != 0) {
                LOGE("update rdb size failed, res: %{public}d", res);
                fuse_reply_err(req, ENOSYS);
            } else {
                fuse_reply_err(req, errno);
            }
            return;
        }
        UpdateCloudDiskInode(rdbStore, inoPtr);
    }
    CloudDiskNotify::GetInstance().TryNotify({data, FileOperationsHelper::FindCloudDiskInode,
        NotifyOpsType::DAEMON_SETATTR, inoPtr});
    fuse_reply_attr(req, &inoPtr->stat, 0);
}

void FileOperationsCloud::Lseek(fuse_req_t req, fuse_ino_t ino, off_t off, int whence,
                                struct fuse_file_info *fi)
{
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    auto filePtr = FileOperationsHelper::FindCloudDiskFile(data, fi->fh);
    if (filePtr == nullptr) {
        fuse_reply_err(req, EINVAL);
        LOGE("file not found");
        return;
    }
    if (filePtr->type != CLOUD_DISK_FILE_TYPE_LOCAL) {
        fuse_reply_err(req, ENOSYS);
        LOGE("lseek on cloud file not supported");
        return;
    }
    off_t res = lseek(filePtr->fd, off, whence);
    if (res != -1)
        fuse_reply_lseek(req, res);
    else
        fuse_reply_err(req, errno);
}
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS
