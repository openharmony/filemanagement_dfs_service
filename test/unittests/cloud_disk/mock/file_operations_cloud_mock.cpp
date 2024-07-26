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
#include "cloud_file_kit.h"
#include "cloud_file_utils.h"
#include "clouddisk_rdb_transaction.h"
#include "clouddisk_rdb_utils.h"
#include "clouddisk_notify.h"
#include "database_manager.h"
#include "directory_ex.h"
#include "ffrt_inner.h"
#include "parameters.h"
#include "file_operations_helper.h"
#include "hitrace_meter.h"
#include "securec.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudDisk {
using namespace std;
using namespace CloudFile;
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
    static const float LOOKUP_TIMEOUT = 60.0;
    static const uint64_t UNKNOWN_INODE_ID = 0;
    static const std::string FILEMANAGER_KEY = "persist.kernel.bundle_name.filemanager";
    static const unsigned int MAX_READ_SIZE = 4 * 1024 * 1024;
}

static void InitInodeAttr(struct CloudDiskFuseData *data, fuse_ino_t parent,
    struct CloudDiskInode *childInode, const MetaBase &metaBase, const int64_t &inodeId)
{
    return;
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
    return;
}

static void LookUpRecycleBin(struct CloudDiskFuseData *data, fuse_ino_t parent,
    shared_ptr<CloudDiskInode> parentInode, struct fuse_entry_param *e)
{
    return;
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

static int32_t LookupRecycledFile(struct CloudDiskFuseData *data, const char *name,
    const std::string bundleName, struct fuse_entry_param *e)
{
    MetaBase metaBase(name);
    auto metaFile = MetaFileMgr::GetInstance().GetCloudDiskMetaFile(data->userId, bundleName,
        RECYCLE_CLOUD_ID);
    int ret = metaFile->DoLookup(metaBase);
    if (ret != 0) {
        LOGE("file %{public}s not found in recyclebin", name);
        return EINVAL;
    }
    int64_t inodeId = static_cast<int64_t>(CloudFileUtils::DentryHash(metaBase.cloudId));
    auto inoPtr = FileOperationsHelper::FindCloudDiskInode(data, inodeId);
    if (inoPtr == nullptr) {
        string nameStr = name;
        size_t lastSlash = nameStr.find_last_of("_");
        metaBase.name = nameStr.substr(0, lastSlash);
        inoPtr = UpdateChildCache(data, inodeId, inoPtr);
        inoPtr->refCount++;
        InitInodeAttr(data, RECYCLE_LOCAL_ID, inoPtr.get(), metaBase, inodeId);
        inoPtr->parent = UNKNOWN_INODE_ID;
    }
    e->ino = static_cast<fuse_ino_t>(inodeId);
    FileOperationsHelper::GetInodeAttr(inoPtr, &e->attr);
    return 0;
}

static int32_t DoCloudLookup(fuse_req_t req, fuse_ino_t parent, const char *name,
                             struct fuse_entry_param *e)
{
    return 1;
}

void FileOperationsCloud::Lookup(fuse_req_t req, fuse_ino_t parent, const char *name)
{
    return;
}

void FileOperationsCloud::Access(fuse_req_t req, fuse_ino_t ino, int mask)
{
    LOGI("Access operation is not supported!");
    fuse_reply_err(req, ENOSYS);
}

void FileOperationsCloud::GetAttr(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi)
{
    return;
}

static bool HandleCloudError(fuse_req_t req, CloudError error)
{
    if (error == CloudError::CK_NO_ERROR) {
        return false;
    }
    if (error == CloudError::CK_NETWORK_ERROR) {
        LOGE("network error");
        fuse_reply_err(req, ENOTCONN);
    } else if (error == CloudError::CK_SERVER_ERROR) {
        LOGE("server error");
        fuse_reply_err(req, EIO);
    } else if (error == CloudError::CK_LOCAL_ERROR) {
        LOGE("local error");
        fuse_reply_err(req, EINVAL);
    } else {
        LOGE("Unknow error");
        fuse_reply_err(req, EIO);
    }
    return true;
}

static shared_ptr<CloudDatabase> GetDatabase(int32_t userId, const string &bundleName)
{
    auto instance = CloudFile::CloudFileKit::GetInstance();
    if (instance == nullptr) {
        LOGE("get cloud file helper instance failed");
        return nullptr;
    }

    if (AccountStatus::IsNeedCleanCache()) {
        auto ret = instance->CleanCloudUserInfo(userId);
        if (ret != 0) {
            return nullptr;
        }
        LOGI("execute clean cloud user info success");
    }

    auto database = instance->GetCloudDatabase(userId, bundleName);
    if (database == nullptr) {
        LOGE("get cloud file kit database fail");
        return nullptr;
    }
    return database;
}

static void CloudOpen(fuse_req_t req,
    shared_ptr<CloudDiskInode> inoPtr, struct fuse_file_info *fi, string path)
{
    return;
}

void FileOperationsCloud::Open(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {}

static int32_t CreateLocalFile(const string &cloudId, const string &bundleName, int32_t userId, mode_t mode)
{
    string bucketPath = CloudFileUtils::GetLocalBucketPath(cloudId, bundleName, userId);
    string path = CloudFileUtils::GetLocalFilePath(cloudId, bundleName, userId);
    if (access(bucketPath.c_str(), F_OK) != 0) {
        if (mkdir(bucketPath.c_str(), STAT_MODE_DIR) != 0) {
            LOGE("mkdir bucketpath failed :%{public}s err:%{public}d", GetAnonyString(bucketPath).c_str(), errno);
            return -errno;
        }
    }
    int32_t fd = open(path.c_str(), (mode & O_NOFOLLOW) | O_CREAT | O_RDWR, STAT_MODE_REG);
    if (fd < 0) {
        LOGE("create file failed :%{public}s err:%{public}d", GetAnonyString(path).c_str(), errno);
        return -errno;
    }
    return fd;
}

void RemoveLocalFile(const string &path)
{
    int32_t err = remove(path.c_str());
    if (err != 0) {
        LOGE("remove file %{public}s failed, error:%{public}d", GetAnonyString(path).c_str(), errno);
    }
}

int32_t GenerateCloudId(int32_t userId, string &cloudId, const string &bundleName)
{
    auto dkDatabasePtr = GetDatabase(userId, bundleName);
    if (dkDatabasePtr == nullptr) {
        LOGE("Failed to get database");
        return ENOSYS;
    }

    vector<std::string> ids;
    auto ret = dkDatabasePtr->GenerateIds(1, ids);
    if (ret != 0 || ids.size() == 0) {
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
    int32_t err = GenerateCloudId(data->userId, cloudId, parentInode->bundleName);
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
    return;
}

void FileOperationsCloud::Create(fuse_req_t req, fuse_ino_t parent, const char *name,
                                 mode_t mode, struct fuse_file_info *fi)
{
    return;
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

static size_t FindNextPos(const vector<MetaBase> &childInfos, off_t off)
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

template<typename T>
static size_t CloudSeekDir(fuse_req_t req, fuse_ino_t ino, off_t off,
                           const std::vector<T> &childInfos)
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

template<typename T>
static void AddDirEntryToBuf(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off,
    const std::vector<T> &childInfos)
{
    return;
}

static void ReadDirForRecycle(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off,
                              struct fuse_file_info *fi)
{
    return;
}

void FileOperationsCloud::ReadDir(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off,
                                  struct fuse_file_info *fi)
{
    return;
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
    return;
}

void HandleCloudRecycle(fuse_req_t req, fuse_ino_t ino, const char *name,
                        const char *value)
{
    return;
}

void HandleFavorite(fuse_req_t req, fuse_ino_t ino, const char *name,
                    const char *value)
{
    return;
}

void HandleExtAttribute(fuse_req_t req, fuse_ino_t ino, const char *name, const char *value)
{
    return;
}

void FileOperationsCloud::SetXattr(fuse_req_t req, fuse_ino_t ino, const char *name,
                                   const char *value, size_t size, int flags)
{
    return;
}

string GetIsFavorite(fuse_req_t req, shared_ptr<CloudDiskInode> inoPtr)
{
    string favorite;
    DatabaseManager &databaseManager = DatabaseManager::GetInstance();
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    auto rdbStore = databaseManager.GetRdbStore(inoPtr->bundleName, data->userId);
    int res = rdbStore->GetXAttr(inoPtr->cloudId, IS_FAVORITE_XATTR, favorite);
    if (res != 0) {
        LOGE("local file get isFavorite fail");
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

string GetLocation(fuse_req_t req, shared_ptr<CloudDiskInode> inoPtr)
{
    string location;
    DatabaseManager &databaseManager = DatabaseManager::GetInstance();
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    auto rdbStore = databaseManager.GetRdbStore(inoPtr->bundleName, data->userId);
    auto parentInode = FileOperationsHelper::FindCloudDiskInode(data, static_cast<int64_t>(inoPtr->parent));
    if (parentInode == nullptr) {
        LOGE("parent inode not found");
        return "null";
    }
    CacheNode newNode = {.parentCloudId = parentInode->cloudId, .fileName = inoPtr->fileName};
    int res = rdbStore->GetXAttr(inoPtr->cloudId, CLOUD_FILE_LOCATION, location, newNode);
    if (res != 0) {
        LOGE("local file get location fail");
        return "null";
    }
    return location;
}

string GetExtAttr(fuse_req_t req, shared_ptr<CloudDiskInode> inoPtr, const char *extAttrKey)
{
    string extAttr;
    if (inoPtr == nullptr) {
        LOGE("get ext attr inoPtr is null");
        return "null";
    }

    DatabaseManager &databaseManager = DatabaseManager::GetInstance();
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    auto rdbStore = databaseManager.GetRdbStore(inoPtr->bundleName, data->userId);
    CacheNode newNode = {};
    int res = rdbStore->GetXAttr(inoPtr->cloudId, CLOUD_EXT_ATTR, extAttr, newNode, extAttrKey);
    if (res != 0) {
        LOGE("get ext attr is null");
        return "null";
    }
    return extAttr;
}

void FileOperationsCloud::GetXattr(fuse_req_t req, fuse_ino_t ino, const char *name,
                                   size_t size)
{
    return;
}

void FileOperationsCloud::MkDir(fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode)
{
    return;
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
    LOGD("doUnlink, dentry file has been deleted");
    if (isDirectory == FILE && position != CLOUD) {
        string localPath = CloudFileUtils::GetLocalFilePath(cloudId, parentInode->bundleName, data->userId);
        LOGI("unlink %{public}s", GetAnonyString(localPath).c_str());
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
    return;
}

void FileOperationsCloud::Unlink(fuse_req_t req, fuse_ino_t parent, const char *name)
{
    return;
}

void FileOperationsCloud::Rename(fuse_req_t req, fuse_ino_t parent, const char *name,
                                 fuse_ino_t newParent, const char *newName, unsigned int flags)
{
    return;
}

void FileOperationsCloud::Read(fuse_req_t req, fuse_ino_t ino, size_t size,
                               off_t offset, struct fuse_file_info *fi)
{
    return;
}

static void UpdateCloudDiskInode(shared_ptr<CloudDiskRdbStore> rdbStore, shared_ptr<CloudDiskInode> inoPtr)
{
    return;
}

static void UpdateCloudStore(CloudDiskFuseData *data, const std::string &fileName, const std::string &parentCloudId,
    int32_t userId, shared_ptr<CloudDiskInode> inoPtr)
{
    return;
}

void FileOperationsCloud::WriteBuf(fuse_req_t req, fuse_ino_t ino, struct fuse_bufvec *bufv,
                                   off_t off, struct fuse_file_info *fi)
{
    return;
}

static void UploadLocalFile(CloudDiskFuseData *data, const std::string &fileName, const std::string &parentCloudId,
    int32_t userId, shared_ptr<CloudDiskInode> inoPtr)
{
    return;
}

void FileOperationsCloud::Release(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi)
{
    return;
}

void FileOperationsCloud::SetAttr(fuse_req_t req, fuse_ino_t ino, struct stat *attr,
                                  int valid, struct fuse_file_info *fi)
{
    return;
}

void FileOperationsCloud::Lseek(fuse_req_t req, fuse_ino_t ino, off_t off, int whence,
                                struct fuse_file_info *fi)
{
    return;
}
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS
