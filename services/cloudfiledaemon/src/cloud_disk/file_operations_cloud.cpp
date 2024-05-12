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
#include <sstream>
#include <sys/types.h>
#include <sys/xattr.h>
#include <functional>

#include "account_status.h"
#include "cloud_disk_inode.h"
#include "cloud_file_utils.h"
#include "clouddisk_rdb_transaction.h"
#include "clouddisk_rdb_utils.h"
#include "clouddisk_notify.h"
#include "database_manager.h"
#include "directory_ex.h"
#include "dk_database.h"
#include "drive_kit.h"
#include "ffrt_inner.h"
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
    static const uint32_t USER_LOCAL_ID_OFFSET = 100;
    static const uint32_t STAT_MODE_REG = 0660;
    static const uint32_t STAT_MODE_DIR = 0771;
    static const uint32_t MILLISECOND_TO_SECONDS_TIMES = 1000;
    static const uint32_t RECYCLE_LOCAL_ID = 4;
    static const string FILE_LOCAL = "1";
    static const string ROOT_CLOUD_ID = "rootId";
    static const string RECYCLE_NAME = ".trash";
    static const uint64_t DELTA_DISK = 0x9E3779B9;
    static const uint64_t HMDFS_HASH_COL_BIT_DISK = (0x1ULL) << 63;
    static const float LOOKUP_TIMEOUT = 60.0;
    static const int32_t LEFT_SHIFT = 4;
    static const int32_t RIGHT_SHIFT = 5;
}

static void InitInodeAttr(struct CloudDiskFuseData *data, fuse_ino_t parent,
    struct CloudDiskInode *childInode, const MetaBase &metaBase, const int64_t &inodeId)
{
    auto parentInode = FileOperationsHelper::FindCloudDiskInode(data,
        static_cast<int64_t>(parent));
    childInode->stat = parentInode->stat;
    childInode->stat.st_ino = static_cast<uint64_t>(inodeId);
    childInode->stat.st_mtime = metaBase.mtime / MILLISECOND_TO_SECONDS_TIMES;
    childInode->stat.st_atime = metaBase.atime / MILLISECOND_TO_SECONDS_TIMES;

    childInode->bundleName = parentInode->bundleName;
    childInode->fileName = metaBase.name;
    childInode->layer = FileOperationsHelper::GetNextLayer(parentInode, parentInode->parent);
    childInode->parent = parent;
    childInode->cloudId = metaBase.cloudId;
    childInode->ops = make_shared<FileOperationsCloud>();

    if (S_ISDIR(metaBase.mode)) {
        childInode->stat.st_mode = S_IFDIR | STAT_MODE_DIR;
        childInode->stat.st_nlink = STAT_NLINK_DIR;
    } else {
        childInode->stat.st_mode = S_IFREG | STAT_MODE_REG;
        childInode->stat.st_nlink = STAT_NLINK_REG;
        childInode->stat.st_size = metaBase.size;
    }
}

static void Str2HashBuf(const char *msg, size_t len, uint32_t *buf, int num)
{
    const int32_t shift8 = 8;
    const int32_t shift16 = 16;
    const int32_t three = 3;
    const int32_t mod = 4;
    uint32_t pad = static_cast<uint32_t>(len) | (static_cast<uint32_t>(len) << shift8);
    pad |= pad << shift16;

    uint32_t val = pad;
    len = std::min(len, static_cast<size_t>(num * sizeof(int)));
    for (uint32_t i = 0; i < len; i++) {
        if ((i % sizeof(int)) == 0) {
            val = pad;
        }
        uint8_t c = static_cast<uint8_t>(tolower(msg[i]));
        val = c + (val << shift8);
        if ((i % mod) == three) {
            *buf++ = val;
            val = pad;
            num--;
        }
    }
    if (--num >= 0) {
        *buf++ = val;
    }
    while (--num >= 0) {
        *buf++ = pad;
    }
}

static void TeaTransform(uint32_t buf[4], uint32_t const in[]) __attribute__((no_sanitize("unsigned-integer-overflow")))
{
    int n = 16;
    uint32_t a = in[0];
    uint32_t b = in[1];
    uint32_t c = in[2];
    uint32_t d = in[3];
    uint32_t b0 = buf[0];
    uint32_t b1 = buf[1];
    uint32_t sum = 0;

    do {
        sum += DELTA_DISK;
        b0 += ((b1 << LEFT_SHIFT) + a) ^ (b1 + sum) ^ ((b1 >> RIGHT_SHIFT) + b);
        b1 += ((b0 << LEFT_SHIFT) + c) ^ (b0 + sum) ^ ((b0 >> RIGHT_SHIFT) + d);
    } while (--n);

    buf[0] += b0;
    buf[1] += b1;
}

static uint32_t DentryHash(const std::string &cloudId)
{
    constexpr int inLen = 8;
    constexpr int bufLen = 4;
    uint32_t in[inLen];
    uint32_t buf[bufLen] = {0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476};
    auto len = cloudId.length();
    constexpr decltype(len) hashWidth = 16;
    const char *p = cloudId.c_str();

    bool loopFlag = true;
    while (loopFlag) {
        Str2HashBuf(p, len, in, bufLen);
        TeaTransform(buf, in);

        if (len <= hashWidth) {
            break;
        }
        p += hashWidth;
        len -= hashWidth;
    };
    uint32_t hash = buf[0];
    uint32_t hmdfsHash = hash & ~HMDFS_HASH_COL_BIT_DISK;
    return hmdfsHash;
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

static void InitLocalIdCache(struct CloudDiskFuseData *data, const std::string &key, const int64_t val)
{
    std::unique_lock<std::shared_mutex> wLock(data->localIdLock, std::defer_lock);
    int64_t localId = FileOperationsHelper::FindLocalId(data, key);
    if (localId == -1) {
        wLock.lock();
        data->localIdCache[key] = val;
        wLock.unlock();
    }
}

static void LookUpRecycleBin(struct CloudDiskFuseData *data, fuse_ino_t parent,
    shared_ptr<CloudDiskInode> parentInode, struct fuse_entry_param *e)
{
    std::unique_lock<std::shared_mutex> cacheWLock(data->cacheLock, std::defer_lock);
    auto child = FileOperationsHelper::FindCloudDiskInode(data, RECYCLE_LOCAL_ID);
    if (child == nullptr) {
        child = make_shared<CloudDiskInode>();
        child->stat = parentInode->stat;
        child->stat.st_ino = RECYCLE_LOCAL_ID;
        child->bundleName = parentInode->bundleName;
        child->fileName = RECYCLE_NAME;
        child->layer = FileOperationsHelper::GetNextLayer(parentInode, parent);
        child->parent = parent;
        child->cloudId = RECYCLE_CLOUD_ID;
        child->ops = make_shared<FileOperationsCloud>();
        child->stat.st_mode = S_IFDIR | STAT_MODE_DIR;
        child->stat.st_nlink = STAT_NLINK_DIR;
        cacheWLock.lock();
        data->inodeCache[RECYCLE_LOCAL_ID] = child;
        cacheWLock.unlock();
    }
    e->ino = static_cast<fuse_ino_t>(RECYCLE_LOCAL_ID);
    FileOperationsHelper::GetInodeAttr(child, &e->attr);
}

static int32_t LookupRecycledFile(struct CloudDiskFuseData *data, const char *name,
    const std::string bundleName, struct fuse_entry_param *e)
{
    MetaBase metaBase(name);
    auto metaFile = MetaFileMgr::GetInstance().GetCloudDiskMetaFile(data->userId, bundleName,
        RECYCLE_CLOUD_ID);
    int ret = metaFile->DoLookup(metaBase);
    if (ret != NativeRdb::E_OK) {
        LOGE("file %{public}s not found in recyclebin", name);
        return EINVAL;
    }
    int64_t inodeId = static_cast<int64_t>(DentryHash(metaBase.cloudId));
    auto inoPtr = FileOperationsHelper::FindCloudDiskInode(data, inodeId);
    if (inoPtr == nullptr) {
        LOGE("inode not found");
        return EINVAL;
    }
    e->ino = static_cast<fuse_ino_t>(inodeId);
    FileOperationsHelper::GetInodeAttr(inoPtr, &e->attr);
    return 0;
}

static shared_ptr<CloudDiskInode> UpdateChildCache(struct CloudDiskFuseData *data, int64_t localId,
    shared_ptr<CloudDiskInode> child)
{
    std::unique_lock<std::shared_mutex> cacheWLock(data->cacheLock, std::defer_lock);
    std::unique_lock<std::shared_mutex> localIdWLock(data->localIdLock, std::defer_lock);
    if (child == nullptr) {
        child = make_shared<CloudDiskInode>();
        cacheWLock.lock();
        data->inodeCache[localId] = child;
        cacheWLock.unlock();
    } else {
        auto old_key = std::to_string(child->parent) + child->fileName;
        localIdWLock.lock();
        data->localIdCache.erase(old_key);
        localIdWLock.unlock();
    }
    return child;
}

static int32_t DoCloudLookup(fuse_req_t req, fuse_ino_t parent, const char *name,
                             struct fuse_entry_param *e)
{
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    if (parent == FUSE_ROOT_ID) {
        LOGE("cloud file operations should not get a fuse root inode");
        return EINVAL;
    }

    auto parentInode = FileOperationsHelper::FindCloudDiskInode(data,
        static_cast<int64_t>(parent));
    if (parentInode == nullptr) {
        LOGE("fail to find parent inode");
        return EINVAL;
    }
    if (name == RECYCLE_NAME) {
        LookUpRecycleBin(data, parent, parentInode, e);
        return 0;
    } else if (parent == RECYCLE_LOCAL_ID) {
        int32_t ret = LookupRecycledFile(data, name, parentInode->bundleName, e);
        if (ret != 0) {
            LOGE("fail to lookup recycledfile");
            return ret;
        }
        return 0;
    }
    MetaBase metaBase(name);
    auto metaFile = MetaFileMgr::GetInstance().GetCloudDiskMetaFile(data->userId, parentInode->bundleName,
        parentInode->cloudId);
    int32_t ret = metaFile->DoLookup(metaBase);
    if (ret != 0) {
        LOGE("lookup dnetry failed, ret = %{public}d", ret);
        return EINVAL;
    }
    string key = std::to_string(parent) + name;
    int64_t inodeId = static_cast<int64_t>(DentryHash(metaBase.cloudId));
    auto inoPtr = FileOperationsHelper::FindCloudDiskInode(data, inodeId);
    auto child = UpdateChildCache(data, inodeId, inoPtr);
    child->refCount++;
    InitInodeAttr(data, parent, child.get(), metaBase, inodeId);
    InitLocalIdCache(data, key, inodeId);
    e->ino = static_cast<fuse_ino_t>(inodeId);
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
        LOGE("server errorCode is: %d", dkError.serverErrorCode);
        fuse_reply_err(req, EIO);
    } else if (dkError.isLocalError) {
        LOGE("local errorCode is: %d", dkError.dkErrorCode);
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

static void CloudOpen(fuse_req_t req,
    shared_ptr<CloudDiskInode> inoPtr, struct fuse_file_info *fi, string path)
{
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    auto filePtr = FileOperationsHelper::FindCloudDiskFile(data, fi->fh);
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
    std::unique_lock<std::shared_mutex> wLock(data->fileIdLock, std::defer_lock);
    wLock.lock();
    data->fileId++;
    fi->fh = data->fileId;
    wLock.unlock();
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
    auto parentInode = FileOperationsHelper::FindCloudDiskInode(data,
        static_cast<int64_t>(parent));

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
    std::unique_lock<std::shared_mutex> wLock(data->fileIdLock, std::defer_lock);
    wLock.lock();
    data->fileId++;
    fi->fh = data->fileId;
    wLock.unlock();
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
    auto parentInode = FileOperationsHelper::FindCloudDiskInode(data, static_cast<int64_t>(inoPtr->parent));
    if (parentInode == nullptr) {
        LOGE("parent inode not found");
        return (void) fuse_reply_err(req, EINVAL);
    }
    DatabaseManager &databaseManager = DatabaseManager::GetInstance();
    auto rdbStore = databaseManager.GetRdbStore(inoPtr->bundleName, data->userId);
    int32_t err = rdbStore->SetXAttr(inoPtr->cloudId, CLOUD_FILE_LOCATION, value, inoPtr->fileName,
        parentInode->cloudId);
    if (err != 0) {
        LOGE("set cloud id fail %{public}d", err);
        fuse_reply_err(req, EINVAL);
        return;
    }
    CloudDiskNotify::GetInstance().TryNotify({data, FileOperationsHelper::FindCloudDiskInode,
        NotifyOpsType::DAEMON_SETXATTR, inoPtr});
    fuse_reply_err(req, 0);
}

static int32_t CreateRecycleDentry(uint32_t userId, const std::string &bundleName)
{
    MetaBase metaBase(RECYCLE_NAME);
    auto metaFile = MetaFileMgr::GetInstance().GetCloudDiskMetaFile(userId, bundleName, ROOT_CLOUD_ID);
    int32_t ret = metaFile->DoLookup(metaBase);
    if (ret != NativeRdb::E_OK) {
        metaBase.cloudId = RECYCLE_CLOUD_ID;
        metaBase.mode = S_IFDIR | STAT_MODE_DIR;
        metaBase.position = static_cast<uint8_t>(LOCAL);
        ret = metaFile->DoCreate(metaBase);
        if (ret != NativeRdb::E_OK) {
            return ret;
        }
    }
    return NativeRdb::E_OK;
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
    auto parentInode = FileOperationsHelper::FindCloudDiskInode(data, static_cast<int64_t>(inoPtr->parent));
    if (parentInode == nullptr) {
        fuse_reply_err(req, EINVAL);
        LOGE("parent inode not found");
        return;
    }
    int32_t ret = CreateRecycleDentry(data->userId, inoPtr->bundleName);
    if (ret != NativeRdb::E_OK) {
        LOGE("create recycle dentry failed");
        return;
    }
    DatabaseManager &databaseManager = DatabaseManager::GetInstance();
    auto rdbStore = databaseManager.GetRdbStore(inoPtr->bundleName, data->userId);
    ret = rdbStore->SetXAttr(inoPtr->cloudId, CLOUD_CLOUD_RECYCLE_XATTR, value,
        inoPtr->fileName, parentInode->cloudId);
    if (ret != 0) {
        LOGE("set cloud id fail %{public}d", ret);
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

string GetIsFavorite(fuse_req_t req, shared_ptr<CloudDiskInode> inoPtr)
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
        buf = GetIsFavorite(req, inoPtr);
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
        LOGE("parent inode not found");
        return (void) fuse_reply_err(req, EINVAL);
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
        return ENOSYS;
    }
    DatabaseManager &databaseManager = DatabaseManager::GetInstance();
    shared_ptr<CloudDiskRdbStore> rdbStore = databaseManager.GetRdbStore(parentInode->bundleName,
                                                                         data->userId);
    MetaBase metaBase(name);
    auto metaFile = MetaFileMgr::GetInstance().GetCloudDiskMetaFile(data->userId,
        parentInode->bundleName, parentInode->cloudId);
    int32_t ret = metaFile->DoLookup(metaBase);
    if (ret != 0) {
        LOGE("lookup denty failed, name:%{public}s", name);
        return EINVAL;
    }
    string cloudId = metaBase.cloudId;
    int32_t isDirectory = S_ISDIR(metaBase.mode);
    int32_t position = metaBase.position;
    ret = metaFile->DoRemove(metaBase);
    if (ret != 0) {
        LOGE("remove dentry failed, ret = %{public}d", ret);
        return ret;
    }
    if (isDirectory == FILE && position != CLOUD) {
        string localPath = CloudFileUtils::GetLocalFilePath(cloudId, parentInode->bundleName, data->userId);
        LOGI("unlink %{public}s", localPath.c_str());
        ret = unlink(localPath.c_str());
        if (ret != 0) {
            LOGE("Failed to unlink cloudId:%{private}s, errno:%{public}d", cloudId.c_str(), errno);
            (void)metaFile->DoCreate(metaBase);
            return ret;
        }
    }
    function<void()> rdbUnlink = [rdbStore, cloudId, position] {
        int32_t err = rdbStore->Unlink(cloudId, position);
        if (err != 0) {
            LOGE("Failed to unlink DB cloudId:%{private}s err:%{public}d", cloudId.c_str(), err);
        }
    };
    ffrt::thread(rdbUnlink).detach();
    return 0;
}

void FileOperationsCloud::RmDir(fuse_req_t req, fuse_ino_t parent, const char *name)
{
    int32_t err = -1;
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    auto parentInode = FileOperationsHelper::FindCloudDiskInode(data, static_cast<int64_t>(parent));
    if (parentInode == nullptr) {
        LOGE("parent inode not found");
        fuse_reply_err(req, EINVAL);
        return;
    }
    auto parentMetaFile = MetaFileMgr::GetInstance().GetCloudDiskMetaFile(data->userId,
        parentInode->bundleName, parentInode->cloudId);
    MetaBase metaBase(name);
    err = parentMetaFile->DoLookup(metaBase);
    if (err != 0) {
        LOGE("lookup dir failed, err=%{public}d", err);
        fuse_reply_err(req, EINVAL);
        return;
    }
    auto metaFile = MetaFileMgr::GetInstance().GetCloudDiskMetaFile(data->userId,
        parentInode->bundleName, metaBase.cloudId);
    std::vector<MetaBase> bases;
    err = metaFile->LoadChildren(bases);
    if (err != 0) {
        LOGE("load children failed, err=%{public}d", err);
        fuse_reply_err(req, EINVAL);
        return;
    }
    if (!bases.empty()) {
        LOGE("Directory not empty");
        fuse_reply_err(req, ENOTEMPTY);
        return;
    }
    err = DoCloudUnlink(req, parent, name);
    if (err != 0) {
        fuse_reply_err(req, err);
        return;
    }
    CloudDiskNotify::GetInstance().TryNotify({data, FileOperationsHelper::FindCloudDiskInode,
        NotifyOpsType::DAEMON_RMDIR, nullptr, parent, name});
    bool deleteFlag = false;
    if (deleteFlag) {
        string dentryfilePath = metaFile->GetDentryFilePath();
        MetaFileMgr::GetInstance().Clear(metaBase.cloudId);
        (void)unlink(dentryfilePath.c_str());
    }
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
        LOGE("rename old or new parent not found");
        fuse_reply_err(req, EINVAL);
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
    bool isDir = false;
    string key = std::to_string(parent) + name;
    int64_t localId = FileOperationsHelper::FindLocalId(data, key);
    auto inoPtr = FileOperationsHelper::FindCloudDiskInode(data, localId);
    if (inoPtr != nullptr) {
        isDir = S_ISDIR(inoPtr->stat.st_mode);
    }
    CloudDiskNotify::GetInstance().TryNotify({data, FileOperationsHelper::FindCloudDiskInode,
        NotifyOpsType::DAEMON_RENAME, nullptr, parent, name, newParent, newName},
        {FileStatus::UNKNOW, isDir});
    return (void) fuse_reply_err(req, 0);
}

void FileOperationsCloud::Read(fuse_req_t req, fuse_ino_t ino, size_t size,
                               off_t offset, struct fuse_file_info *fi)
{
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    auto filePtr = FileOperationsHelper::FindCloudDiskFile(data, fi->fh);
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

static void UpdateCloudStore(CloudDiskFuseData *data, const std::string &fileName, const std::string &parentCloudId,
    int32_t userId, shared_ptr<CloudDiskInode> inoPtr)
{
    DatabaseManager &databaseManager = DatabaseManager::GetInstance();
    auto rdbStore = databaseManager.GetRdbStore(inoPtr->bundleName, userId);
    int32_t dirtyType;
    int res = rdbStore->GetDirtyType(inoPtr->cloudId, dirtyType);
    if (res != 0) {
        LOGE("get file status fail, err: %{public}d", res);
    }
    res = rdbStore->Write(fileName, parentCloudId, inoPtr->cloudId);
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

static void UploadLocalFile(CloudDiskFuseData *data, const std::string &fileName, const std::string &parentCloudId,
    int32_t userId, shared_ptr<CloudDiskInode> inoPtr)
{
    MetaBase metaBase(fileName);
    auto metaFile = MetaFileMgr::GetInstance().GetCloudDiskMetaFile(userId, inoPtr->bundleName, parentCloudId);
    int32_t ret = metaFile->DoLookup(metaBase);
    if (ret != NativeRdb::E_OK) {
        LOGE("local file get location from dentryfile fail, ret = %{public}d", ret);
    } else if (metaBase.position == LOCAL) {
        DatabaseManager &databaseManager = DatabaseManager::GetInstance();
        auto rdbStore = databaseManager.GetRdbStore(inoPtr->bundleName, userId);
        int32_t dirtyType;
        ret = rdbStore->GetDirtyType(inoPtr->cloudId, dirtyType);
        if (ret != 0) {
            LOGE("get file status fail, err: %{public}d", ret);
        }
        ret = rdbStore->Write(fileName, parentCloudId, inoPtr->cloudId);
        if (ret != 0) {
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
        LOGE("inode not found");
        fuse_reply_err(req, EINVAL);
        return;
    }
    auto parentInode = FileOperationsHelper::FindCloudDiskInode(data,
        static_cast<int64_t>(inoPtr->parent));
    if (parentInode == nullptr) {
        LOGE("fail to find parent inode");
        return;
    }
    string parentCloudId = parentInode->cloudId;
    shared_ptr<CloudDiskFile> filePtr = FileOperationsHelper::FindCloudDiskFile(data, fi->fh);
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
                UpdateCloudStore(data, inoPtr->fileName, parentCloudId, data->userId, inoPtr);
            } else {
                UploadLocalFile(data, inoPtr->fileName, parentCloudId, data->userId, inoPtr);
            }
        } else if (filePtr->type == CLOUD_DISK_FILE_TYPE_CLOUD &&
            filePtr->readSession != nullptr) {
            bool res = filePtr->readSession->Close(false);
            if (!res) {
                LOGE("close error");
                fuse_reply_err(req, ENOSYS);
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
    auto parentInode = FileOperationsHelper::FindCloudDiskInode(data,
        static_cast<int64_t>(inoPtr->parent));
    if (static_cast<unsigned int>(valid) & FUSE_SET_ATTR_SIZE) {
        DatabaseManager &databaseManager = DatabaseManager::GetInstance();
        auto rdbStore = databaseManager.GetRdbStore(inoPtr->bundleName, data->userId);
        int32_t res = rdbStore->SetAttr(inoPtr->fileName, parentInode->cloudId, inoPtr->cloudId, attr->st_size);
        if (res != 0) {
            LOGE("update rdb size failed, res: %{public}d", res);
            fuse_reply_err(req, ENOSYS);
            return;
        }
        if (fi) {
            auto filePtr = FileOperationsHelper::FindCloudDiskFile(data, fi->fh);
            if (filePtr == nullptr) {
                LOGE("file not found");
                return (void) fuse_reply_err(req, EINVAL);
            }
            res = ftruncate(filePtr->fd, attr->st_size);
        } else {
            string path = CloudFileUtils::GetLocalFilePath(inoPtr->cloudId, inoPtr->bundleName, data->userId);
            res = truncate(path.c_str(), attr->st_size);
        }
        if (res == -1) {
            LOGE("truncate failed, err: %{public}d", errno);
            res = rdbStore->SetAttr(inoPtr->fileName, parentInode->cloudId, inoPtr->cloudId,
                inoPtr->stat.st_size);
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
    auto inoPtr = FileOperationsHelper::FindCloudDiskInode(data, static_cast<int64_t>(ino));
    if (inoPtr == nullptr) {
        LOGE("get an invalid inode!");
        fuse_reply_err(req, EINVAL);
        return;
    }
    shared_ptr<CloudDiskFile> filePtr = FileOperationsHelper::FindCloudDiskFile(data, fi->fh);
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
