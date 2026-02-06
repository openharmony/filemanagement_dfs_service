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
#include "cloud_file_fault_event.h"
#include "cloud_file_kit.h"
#include "cloud_file_utils.h"
#include "clouddisk_rdb_transaction.h"
#include "clouddisk_rdb_utils.h"
#include "clouddisk_notify.h"
#include "database_manager.h"
#include "directory_ex.h"
#include "ffrt_inner.h"
#include "parameter.h"
#include "parameters.h"
#include "file_operations_helper.h"
#include "fuse_ioctl.h"
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
    IS_FAVORITE,
    HAS_THM
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
    static const string CONFLICT_PATH = ".cloudFileVersionConflict";
    static const uint64_t UNKNOWN_INODE_ID = 0;
    static const std::string FILEMANAGER_KEY = "persist.kernel.bundle_name.filemanager";
    static const string LOCAL_PATH_DATA_STORAGE = "/data/storage/el2/cloud/";
    static const unsigned int MAX_READ_SIZE = 4 * 1024 * 1024;
    static const unsigned int CATCH_TIMEOUT_S = 4;
    static const std::chrono::seconds READ_TIMEOUT_S = 16s;
    static const std::chrono::seconds OPEN_TIMEOUT_S = 4s;
    static const uint64_t INVAL_TIMEOUT_MS = 5;
    static const uint32_t SERVICE_UID = 1009;
}

const int32_t MAX_SIZE = 4096;

struct CloudDiskCopy {
    char destPath[MAX_SIZE];
};

struct SessionInitParams {
    std::shared_ptr<CloudDiskFile> filePtr;
    std::shared_ptr<CloudFile::CloudError> error;
    std::shared_ptr<bool> openFinish;
};

struct FuseInvalData {
    fuse_session *se;
    fuse_ino_t parentIno;
    fuse_ino_t childIno;
    string childName;

    FuseInvalData(fuse_session *seVal, fuse_ino_t parentInoVal, fuse_ino_t childInoVal, const string &childNameVal)
        : se(seVal), parentIno(parentInoVal), childIno(childInoVal), childName(childNameVal) {}
};

struct ReadCompletionParams {
    shared_ptr<ffrt::condition_variable> cond;
    shared_ptr<bool> readFinish;
    shared_ptr<CloudError> error;
    shared_ptr<int64_t> readSize;
    shared_ptr<char> buf;
};

struct CloudReadParams {
    off_t offset;
    size_t size;
    shared_ptr<char> buf;
    string path;
};

static void CallBack(void *data)
{
    FuseInvalData *fuseInvalData = static_cast<FuseInvalData *>(data);
    if (fuse_lowlevel_notify_inval_entry(fuseInvalData->se, fuseInvalData->parentIno,
                                         fuseInvalData->childName.c_str(), fuseInvalData->childName.size())) {
        fuse_lowlevel_notify_inval_inode(fuseInvalData->se, fuseInvalData->childIno, 0, 0);
    }

    delete fuseInvalData;
}

static void fuse_inval(fuse_session *se, fuse_ino_t parentIno, fuse_ino_t childIno, const string &childName)
{
    FuseInvalData *fuseInvalData = new (nothrow) FuseInvalData(se, parentIno, childIno, childName);
    if (fuseInvalData == nullptr) {
        return;
    }
    ffrt_timer_start(ffrt_qos_background, INVAL_TIMEOUT_MS, static_cast<void *>(fuseInvalData), CallBack, false);
}

static void InitInodeAttr(struct CloudDiskFuseData *data, fuse_ino_t parent,
    struct CloudDiskInode *childInode, const MetaBase &metaBase, const int64_t &inodeId)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    auto parentInode = FileOperationsHelper::FindCloudDiskInode(data,
        static_cast<int64_t>(parent));
    if (parentInode == nullptr) {
        LOGE("parent inode not found");
        return;
    }
    childInode->stat = parentInode->stat;
    childInode->stat.st_ino = static_cast<uint64_t>(inodeId);
    childInode->stat.st_mtime = metaBase.mtime / MILLISECOND_TO_SECONDS_TIMES;
    childInode->stat.st_atime = metaBase.atime / MILLISECOND_TO_SECONDS_TIMES;

    childInode->bundleName = parentInode->bundleName;
    childInode->fileName = metaBase.name;
    childInode->layer = FileOperationsHelper::GetNextLayer(parentInode, parent);
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
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
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
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
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
    child->refCount++;
    e->ino = static_cast<fuse_ino_t>(RECYCLE_LOCAL_ID);
    FileOperationsHelper::GetInodeAttr(child, &e->attr);
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
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    MetaBase metaBase(name);
    auto metaFile = MetaFileMgr::GetInstance().GetCloudDiskMetaFile(data->userId, bundleName,
        RECYCLE_CLOUD_ID);
    int ret = metaFile->DoLookup(metaBase);
    if (ret != 0) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{"", CloudFile::FaultOperation::LOOKUP,
            CloudFile::FaultType::FILE, ret, "file " + GetAnonyString(name) + " not found in recyclebin"});
        return ret;
    }
    int64_t inodeId = static_cast<int64_t>(CloudFileUtils::DentryHash(metaBase.cloudId));
    auto inoPtr = FileOperationsHelper::FindCloudDiskInode(data, inodeId);
    string nameStr = name;
    size_t lastSlash = nameStr.find_last_of("_");
    metaBase.name = nameStr.substr(0, lastSlash);
    inoPtr = UpdateChildCache(data, inodeId, inoPtr);
    inoPtr->refCount++;
    InitInodeAttr(data, RECYCLE_LOCAL_ID, inoPtr.get(), metaBase, inodeId);
    inoPtr->parent = UNKNOWN_INODE_ID;
    e->ino = static_cast<fuse_ino_t>(inodeId);
    FileOperationsHelper::GetInodeAttr(inoPtr, &e->attr);
    return 0;
}

static int32_t DoCloudLookup(fuse_req_t req, fuse_ino_t parent, const char *name,
                             struct fuse_entry_param *e)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    if (parent == FUSE_ROOT_ID) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{"", CloudFile::FaultOperation::LOOKUP,
            CloudFile::FaultType::WARNING, EINVAL, "cloud file operations should not get a fuse root inode"});
        return EINVAL;
    }

    auto parentInode = FileOperationsHelper::FindCloudDiskInode(data,
        static_cast<int64_t>(parent));
    if (parentInode == nullptr) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{"", CloudFile::FaultOperation::LOOKUP,
            CloudFile::FaultType::INODE_FILE, EINVAL, "fail to find parent inode"});
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
        LOGE("lookup dentry failed, name:%{public}s, ret = %{public}d", GetAnonyString(name).c_str(), ret);
        return ENOENT;
    }
    string key = std::to_string(parent) + name;
    int64_t inodeId = static_cast<int64_t>(CloudFileUtils::DentryHash(metaBase.cloudId));
    auto inoPtr = FileOperationsHelper::FindCloudDiskInode(data, inodeId);
    // if inoPtr is nullptr, UpdateChildCache will create it
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
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    struct fuse_entry_param e;
    e.attr_timeout = 1.0;
    e.entry_timeout = 1.0;
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
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    auto inoPtr = FileOperationsHelper::FindCloudDiskInode(data, static_cast<int64_t>(ino));
    if (inoPtr == nullptr) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{"", CloudFile::FaultOperation::GETATTR,
            CloudFile::FaultType::INODE_FILE, EINVAL, "inode not found"});
        fuse_reply_err(req, EINVAL);
        return;
    }
    fuse_reply_attr(req, &inoPtr->stat, 0);
}

static int32_t HandleCloudError(fuse_req_t req, CloudError error)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    int32_t ret = 0;
    switch (error) {
        case CloudError::CK_NO_ERROR:
            ret = 0;
            break;
        case CloudError::CK_NETWORK_ERROR:
            ret = ENOTCONN;
            CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{"", CloudFile::FaultOperation::SESSION,
                CloudFile::FaultType::WARNING, ENOTCONN, "network error"});
            break;
        case CloudError::CK_SERVER_ERROR:
            ret = EIO;
            CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{"", CloudFile::FaultOperation::SESSION,
                CloudFile::FaultType::WARNING, EIO, "server error"});
            break;
        case CloudError::CK_LOCAL_ERROR:
            ret = EINVAL;
            CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{"", CloudFile::FaultOperation::SESSION,
                CloudFile::FaultType::WARNING, EINVAL, "local error"});
            break;
        default:
            ret = EIO;
            CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{"", CloudFile::FaultOperation::SESSION,
                CloudFile::FaultType::WARNING, EIO, "unknown error"});
            break;
    }
    return ret;
}

static shared_ptr<CloudDatabase> GetDatabase(int32_t userId, const string &bundleName)
{
    auto instance = CloudFile::CloudFileKit::GetInstance();
    if (instance == nullptr) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{"", CloudFile::FaultOperation::OPEN,
            CloudFile::FaultType::DRIVERKIT, EINVAL, "get cloud file helper instance failed"});
        return nullptr;
    }

    if (AccountStatus::IsNeedCleanCache()) {
        auto ret = instance->CleanCloudUserInfo(userId, bundleName);
        if (ret != 0) {
            return nullptr;
        }
        LOGI("execute clean cloud user info success");
    }

    auto database = instance->GetCloudDatabase(userId, bundleName);
    if (database == nullptr) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{"", CloudFile::FaultOperation::READ,
            CloudFile::FaultType::QUERY_DATABASE, EINVAL, "get cloud file kit database fail"});
        return nullptr;
    }
    return database;
}

static unsigned int GetFileOpenFlags(int32_t fileFlags)
{
    unsigned int flags = static_cast<unsigned int>(fileFlags);
    if ((flags & O_ACCMODE) & O_WRONLY) {
        flags &= ~O_WRONLY;
        flags |= O_RDWR;
    }
    if (flags & O_APPEND) {
        flags &= ~O_APPEND;
    }
    if (flags & O_DIRECT) {
        flags &= ~O_DIRECT;
    }
    return flags;
}

static int32_t CheckBucketPath(const string &cloudId, const string &bundleName, int32_t userId, string tmpPath)
{
    string baseDir = CloudFileUtils::GetLocalBaseDir(bundleName, userId);
    string bucketPath = CloudFileUtils::GetLocalBucketPath(cloudId, bundleName, userId);
    if (access(baseDir.c_str(), F_OK) != 0) {
        LOGE("bucket path's parent directory not exits, errno=%{public}d", errno);
        auto accessErrno = errno;
        if (unlink(tmpPath.c_str()) != 0) {
            LOGE("unlink tmpPath failed, errno=%{public}d", errno);
            return errno;
        }
        return accessErrno;
    }
    if (access(bucketPath.c_str(), F_OK) != 0) {
        if (mkdir(bucketPath.c_str(), STAT_MODE_DIR) != 0) {
            LOGE("create bucket path directory failed, errno=%{public}d", errno);
            auto mkdirErrno = errno;
            if (unlink(tmpPath.c_str()) != 0) {
                LOGE("unlink tmpPath failed, errno=%{public}d", errno);
                return errno;
            }
            return mkdirErrno;
        }
        LOGW("mkdir bucketPath success");
        CloudFileUtils::ChangeUid(userId, bundleName, STAT_MODE_DIR, bucketPath);
    }
    return EOK;
}

static int32_t HandleCloudOpenSuccess(struct fuse_file_info *fi, struct CloudDiskFuseData *data,
    shared_ptr<CloudDiskInode> inoPtr, CloudOpenParams cloudOpenParams)
{
    auto metaBase = cloudOpenParams.metaBase;
    auto metaFile = cloudOpenParams.metaFile;
    auto filePtr = cloudOpenParams.filePtr;
    if (metaBase.fileType != FILE_TYPE_CONTENT) {
        string path = CloudFileUtils::GetLocalFilePath(inoPtr->cloudId, inoPtr->bundleName, data->userId);
        string tmpPath = CloudFileUtils::GetLocalDKCachePath(inoPtr->cloudId, inoPtr->bundleName, data->userId);
        auto ret = CheckBucketPath(inoPtr->cloudId, inoPtr->bundleName, data->userId, tmpPath);
        if (ret != EOK) {
            LOGE("check bucketPath failed, ret = %{public}d", ret);
            return ret;
        }
        if (access(path.c_str(), F_OK) != 0) {
            ret = rename(tmpPath.c_str(), path.c_str());
            if (ret == EOK) {
                DatabaseManager &databaseManager = DatabaseManager::GetInstance();
                auto rdbstore = databaseManager.GetRdbStore(inoPtr->bundleName, data->userId);
                rdbstore->UpdateTHMStatus(metaFile, metaBase, CloudSync::DOWNLOADED_THM, path);
            } else {
                LOGE("path rename failed, tmpPath:%{public}s, errno:%{public}d",
                    GetAnonyString(tmpPath).c_str(), errno);
                return errno;
            }
            CloudFileUtils::ChangeUid(data->userId, inoPtr->bundleName, STAT_MODE_REG, path);
        }
        unsigned int flags = GetFileOpenFlags(fi->flags);
        int32_t fd = open(path.c_str(), flags);
        if (fd < 0) {
            LOGE("failed to open local file, errno: %{public}d", errno);
            return errno;
        }
        filePtr->type = CLOUD_DISK_FILE_TYPE_LOCAL;
        filePtr->fd = fd;
    } else {
        filePtr->type = CLOUD_DISK_FILE_TYPE_CLOUD;
    }
    return EOK;
}

static void DoSessionInit(SessionInitParams sessionInitParams, shared_ptr<ffrt::condition_variable> cond,
    CloudOpenParams cloudOpenParams, const std::string &dirPath)
{
    ffrt::submit([sessionInitParams, cond, cloudOpenParams, dirPath] {
        HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
        auto session = sessionInitParams.filePtr->readSession;
        if (!session) {
            LOGE("readSession is nullptr");
            {
                unique_lock lck(sessionInitParams.filePtr->openLock);
                *sessionInitParams.openFinish = true;
            }
            *sessionInitParams.error = CloudError::CK_LOCAL_ERROR;
            cond->notify_one();
            return;
        }
        *sessionInitParams.error = session->InitSession();
        if (*sessionInitParams.error == CloudError::CK_NO_ERROR) {
            if (cloudOpenParams.metaBase.fileType != FILE_TYPE_CONTENT) {
                session->Catch(*sessionInitParams.error, CATCH_TIMEOUT_S);
            }
        }
        bool isSetUidSuccess = session->ChownUidForSyncDisk(true, dirPath);
        if (!isSetUidSuccess) {
            LOGE("SetUid failed: %{public}s", dirPath.c_str());
        }
        {
            unique_lock lck(sessionInitParams.filePtr->openLock);
            *sessionInitParams.openFinish = true;
        }
        cond->notify_one();
        LOGI("download done");
        return;
    });
}

static int32_t DoCloudOpen(fuse_req_t req, struct fuse_file_info *fi,
    shared_ptr<CloudDiskInode> inoPtr, struct CloudDiskFuseData *data, CloudOpenParams cloudOpenParams)
{
    auto error = make_shared<CloudError>();
    auto openFinish = make_shared<bool>(false);
    auto cond = make_shared<ffrt::condition_variable>();
    auto filePtr = cloudOpenParams.filePtr;
    SessionInitParams sessionInitParams = {filePtr, error, openFinish};
    string bundlePath = CloudFileUtils::GetLocalBaseDir(inoPtr->bundleName, data->userId);
    DoSessionInit(sessionInitParams, cond, cloudOpenParams, bundlePath);
    unique_lock lck(filePtr->openLock);
    auto waitStatus = cond->wait_for(lck, OPEN_TIMEOUT_S, [openFinish] {
        return *openFinish;
    });
    if (!waitStatus) {
        LOGE("init session timeout");
        return ENETUNREACH;
    }
    auto ret = HandleCloudError(req, *error);
    if (!ret) {
        auto cloudOpenRet = HandleCloudOpenSuccess(fi, data, inoPtr, cloudOpenParams);
        if (cloudOpenRet) {
            CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{inoPtr->bundleName, CloudFile::FaultOperation::OPEN,
                CloudFile::FaultType::FILE, cloudOpenRet, "cloud open failed"});
            return cloudOpenRet;
        }
        return 0;
    } else {
        LOGE("cloud open failed");
        return ret;
    }
}

static void ErasePathCache(string path, CloudDiskFuseData *data)
{
    std::unique_lock<std::shared_mutex> lck(data->readSessionLock);
    if (data->readSessionCache.find(path) != data->readSessionCache.end()) {
        data->readSessionCache.erase(path);
    }
}

static void HandleNewSession(struct CloudDiskFuseData *data, const struct SessionCountParams &sessionParam,
    shared_ptr<CloudDiskFile> filePtr, shared_ptr<CloudDatabase> database)
{
    string path = sessionParam.path;
    std::unique_lock<std::shared_mutex> rwLock(data->readSessionLock, std::defer_lock);
    rwLock.lock();
    if (data->readSessionCache.find(path) != data->readSessionCache.end()) {
        filePtr->readSession = data->readSessionCache[path];
        rwLock.unlock();
        return;
    }
    rwLock.unlock();
    string cloudId;
    if (database->Convert2CloudId(sessionParam.cloudId, cloudId) != 0) {
        LOGE("Failed to get cloudId");
        return;
    }
    filePtr->readSession = database->NewAssetReadSession(data->userId, "file", cloudId, sessionParam.assets, path);
    rwLock.lock();
    if (filePtr->readSession) {
        data->readSessionCache[path] = filePtr->readSession;
    }
    rwLock.unlock();
}

static int32_t GetNewSession(shared_ptr<CloudDiskInode> inoPtr,
    string &path, struct CloudDiskFuseData *data, shared_ptr<CloudDatabase> database,
    CloudOpenParams cloudOpenParams)
{
    string cloudId = inoPtr->cloudId;
    string assets = "content";
    auto metaBase = cloudOpenParams.metaBase;
    auto filePtr = cloudOpenParams.filePtr;
    if (metaBase.fileType == FILE_TYPE_THUMBNAIL ||
        metaBase.fileType == FILE_TYPE_LCD) {
        assets = (metaBase.fileType == FILE_TYPE_THUMBNAIL) ? "thumbnail" : "lcd";
        DatabaseManager &databaseManager = DatabaseManager::GetInstance();
        auto rdbStore = databaseManager.GetRdbStore(inoPtr->bundleName, data->userId);
        auto ret = rdbStore->GetSrcCloudId(inoPtr->cloudId, cloudId);
        if (ret) {
            LOGE("get %{public}s cloudId failed", assets.c_str());
            return ret;
        }
    }
    LOGD("cloudId %s", cloudId.c_str());
    struct SessionCountParams sessionParam = {path, cloudId, assets};
    HandleNewSession(data, sessionParam, filePtr, database);
    return EOK;
}

static void HandleOpenFail(HandleOpenErrorParams params, string path, CloudDiskFuseData *data,
    fuse_file_info *fi)
{
    ErasePathCache(path, data);
    params.filePtr->readSession = nullptr;
    params.filePtr->refCount--;
    FileOperationsHelper::PutCloudDiskFile(data, params.filePtr, fi->fh);
    fuse_inval(data->se, params.inoPtr->parent, params.ino, params.inoPtr->fileName);
}

static void HandleSessionNull(HandleOpenErrorParams params, CloudDiskFuseData *data,
    fuse_file_info *fi)
{
    CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{"", CloudFile::FaultOperation::OPEN,
            CloudFile::FaultType::DRIVERKIT, EPERM, "readSession is null"});
    params.filePtr->refCount--;
    FileOperationsHelper::PutCloudDiskFile(data, params.filePtr, fi->fh);
    fuse_inval(data->se, params.inoPtr->parent, params.ino, params.inoPtr->fileName);
}

static void HandleCloudReopen(struct fuse_file_info *fi, struct CloudDiskFuseData *data,
    shared_ptr<CloudDiskInode> inoPtr, CloudOpenParams cloudOpenParams, fuse_req_t req)
{
    auto metaBase = cloudOpenParams.metaBase;
    auto filePtr = cloudOpenParams.filePtr;
    if (metaBase.fileType != FILE_TYPE_CONTENT) {
        string path = CloudFileUtils::GetLocalFilePath(inoPtr->cloudId, inoPtr->bundleName, data->userId);
        unsigned int flags = GetFileOpenFlags(fi->flags);
        int32_t fd = open(path.c_str(), flags);
        if (fd < 0) {
            LOGE("failed to open local file, errno: %{public}d", errno);
            fuse_reply_err(req, errno);
            return;
        }
        filePtr->type = CLOUD_DISK_FILE_TYPE_LOCAL;
        filePtr->fd = fd;
    } else {
        filePtr->type = CLOUD_DISK_FILE_TYPE_CLOUD;
        filePtr->readSession->sessionCount++;
    }
    fuse_reply_open(req, fi);
}

static void CloudOpen(fuse_req_t req, shared_ptr<CloudDiskInode> inoPtr,
    struct fuse_file_info *fi, string path, fuse_ino_t ino)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    auto database = GetDatabase(data->userId, inoPtr->bundleName);
    if (!database) {
        LOGE("database is null");
        fuse_inval(data->se, inoPtr->parent, ino, inoPtr->fileName);
        return (void) fuse_reply_err(req, EPERM);
    }
    auto parentInode = FileOperationsHelper::FindCloudDiskInode(data, static_cast<int64_t>(inoPtr->parent));
    if (parentInode == nullptr) {
        LOGE("parent inode not found");
        fuse_inval(data->se, inoPtr->parent, ino, inoPtr->fileName);
        return (void) fuse_reply_err(req, EPERM);
    }
    auto metaFile = MetaFileMgr::GetInstance().GetCloudDiskMetaFile(data->userId,
        parentInode->bundleName, parentInode->cloudId);
    MetaBase metaBase(inoPtr->fileName);
    auto ret = metaFile->DoLookup(metaBase);
    if (ret != EOK) {
        fuse_inval(data->se, inoPtr->parent, ino, inoPtr->fileName);
        return (void) fuse_reply_err(req, EPERM);
    }
    auto filePtr = InitFileAttr(data, fi);
    CloudOpenParams cloudOpenParams = {metaBase, metaFile, filePtr};
    std::unique_lock<std::shared_mutex> lck(inoPtr->sessionLock);
    if (GetNewSession(inoPtr, path, data, database, cloudOpenParams)) {
        filePtr->refCount--;
        FileOperationsHelper::PutCloudDiskFile(data, filePtr, fi->fh);
        fuse_inval(data->se, inoPtr->parent, ino, inoPtr->fileName);
        return (void) fuse_reply_err(req, EPERM);
    }
    HandleOpenErrorParams handleOpenErrorParams = {filePtr, inoPtr, ino};
    if (filePtr->readSession) {
        if (filePtr->readSession->sessionCount > 0) {
            return HandleCloudReopen(fi, data, inoPtr, cloudOpenParams, req);
        }
        auto ret = DoCloudOpen(req, fi, inoPtr, data, cloudOpenParams);
        if (!ret) {
            filePtr->readSession->sessionCount++;
            fuse_reply_open(req, fi);
        } else {
            HandleOpenFail(handleOpenErrorParams, path, data, fi);
            return (void) fuse_reply_err(req, ret);
        }
    } else {
        HandleSessionNull(handleOpenErrorParams, data, fi);
        return (void) fuse_reply_err(req, EPERM);
    }
}

void FileOperationsCloud::Open(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi)
{
    if (WaitParameter("persist.kernel.move.finish", "true", MOVE_FILE_TIME_DAEMON) != 0) {
        LOGE("wait move error");
        return (void) fuse_reply_err(req, EBUSY);
    }
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    std::unique_lock<std::shared_mutex> wLock(data->fileIdLock, std::defer_lock);
    wLock.lock();
    data->fileId++;
    fi->fh = static_cast<uint64_t>(data->fileId);
    wLock.unlock();
    auto inoPtr = FileOperationsHelper::FindCloudDiskInode(data, static_cast<int64_t>(ino));
    if (inoPtr == nullptr) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{"", CloudFile::FaultOperation::OPEN,
            CloudFile::FaultType::INODE_FILE, EINVAL, "inode not found"});
        fuse_reply_err(req, EINVAL);
        return;
    }
    string path = CloudFileUtils::GetLocalFilePath(inoPtr->cloudId, inoPtr->bundleName, data->userId);
    unsigned int flags = GetFileOpenFlags(fi->flags);
    if (access(path.c_str(), F_OK) == 0) {
        int32_t fd = open(path.c_str(), flags);
        if (fd < 0) {
            CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{"", CloudFile::FaultOperation::OPEN,
                CloudFile::FaultType::FILE, errno, "open file failed path:" + GetAnonyString(path) +
                " errno: " + std::to_string(errno)});
            fuse_inval(data->se, inoPtr->parent, ino, inoPtr->fileName);
            return (void) fuse_reply_err(req, errno);
        }
        struct stat statInfo {};
        if (fstat(fd, &statInfo) != 0) {
            LOGE("fstat path %{public}s failed, errno %{public}d", GetAnonyString(path).c_str(), errno);
            close(fd);
            return (void) fuse_reply_err(req, errno);
        }
        auto filePtr = InitFileAttr(data, fi);
        filePtr->type = CLOUD_DISK_FILE_TYPE_LOCAL;
        filePtr->fd = fd;
        filePtr->isWriteOpen = (flags & O_RDWR) | (flags & O_WRONLY);
        filePtr->mtime = static_cast<uint64_t>(CloudFileUtils::Timespec2Milliseconds(statInfo.st_mtim));
        fuse_reply_open(req, fi);
    } else {
        path = CloudFileUtils::GetLocalDKCachePath(inoPtr->cloudId, inoPtr->bundleName, data->userId);
        CloudOpen(req, inoPtr, fi, path, ino);
    }
}

static int32_t CreateLocalFile(const string &cloudId, const string &bundleName, int32_t userId, mode_t mode)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    string bucketPath = CloudFileUtils::GetLocalBucketPath(cloudId, bundleName, userId);
    string path = CloudFileUtils::GetLocalFilePath(cloudId, bundleName, userId);
    if (access(bucketPath.c_str(), F_OK) != 0) {
        if (mkdir(bucketPath.c_str(), STAT_MODE_DIR) != 0) {
            CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{"", CloudFile::FaultOperation::MKNOD,
                CloudFile::FaultType::FILE, errno, "mkdir bucketpath failed :" + GetAnonyString(bucketPath) +
                " err: " + std::to_string(errno)});
            return -errno;
        }
        CloudFileUtils::ChangeUid(userId, bundleName, STAT_MODE_DIR, bucketPath);
    }
    int32_t fd = open(path.c_str(), (mode & O_NOFOLLOW) | O_CREAT | O_RDWR, STAT_MODE_REG);
    if (fd < 0) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{"", CloudFile::FaultOperation::MKNOD,
            CloudFile::FaultType::FILE, errno, "create file failed :" + GetAnonyString(path) +
            " err: " + std::to_string(errno)});
        return -errno;
    }
    CloudFileUtils::ChangeUid(userId, bundleName, STAT_MODE_REG, path);
    return fd;
}

void RemoveLocalFile(const string &path)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    int32_t err = remove(path.c_str());
    if (err != 0) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{"", CloudFile::FaultOperation::MKNOD,
            CloudFile::FaultType::WARNING, errno, "remove file " + GetAnonyString(path) + " failed, "+
            "error: " + std::to_string(errno)});
    }
}

int32_t GenerateCloudId(int32_t userId, string &cloudId, const string &bundleName)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    vector<std::string> ids;
    int32_t ret = 0;
    if (bundleName == system::GetParameter(FILEMANAGER_KEY, "")) {
        auto dkDatabasePtr = GetDatabase(userId, bundleName);
        if (dkDatabasePtr == nullptr) {
            LOGE("Failed to get database");
            return ENOSYS;
        }
        ret = dkDatabasePtr->GenerateIds(1, ids);
        if (ret != 0 || ids.size() == 0) {
            return ENOSYS;
        }
        cloudId = ids[0];
        return 0;
    }
    auto instance = CloudFile::CloudFileKit::GetInstance();
    if (instance == nullptr) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{"", CloudFile::FaultOperation::OPEN,
            CloudFile::FaultType::DRIVERKIT, EINVAL, "get cloud file helper instance failed"});
        return ENOSYS;
    }
    ret = instance->GenerateLocalIds(bundleName, 1, ids);
    if (ret != 0 || ids.size() == 0) {
        return ENOSYS;
    }
    cloudId = ids[0];
    return 0;
}

static int32_t GetParentUpload(shared_ptr<CloudDiskInode> parentInode, struct CloudDiskFuseData *data,
                               bool &parentNoUpload)
{
    auto grandparentInode = FileOperationsHelper::FindCloudDiskInode(data, static_cast<int64_t>(parentInode->parent));
    if (grandparentInode == nullptr) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{"", CloudFile::FaultOperation::MKNOD,
            CloudFile::FaultType::INODE_FILE, EINVAL, "grandparentInode not found"});
        return EINVAL;
    }
    auto metaFile = MetaFileMgr::GetInstance().GetCloudDiskMetaFile(data->userId,
        grandparentInode->bundleName, grandparentInode->cloudId);
    MetaBase metaBase(parentInode->fileName);
    auto ret = metaFile->DoLookup(metaBase);
    if (ret != 0) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{grandparentInode->bundleName,
            CloudFile::FaultOperation::MKNOD, CloudFile::FaultType::DENTRY_FILE, ret, "file " +
            GetAnonyString(parentInode->fileName) + " not found"});
        return ret;
    }
    parentNoUpload = (metaBase.noUpload == NO_UPLOAD);
    return 0;
}

int32_t DoCreatFile(fuse_req_t req, fuse_ino_t parent, const char *name,
                    mode_t mode, struct fuse_entry_param &e)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    struct CloudDiskFuseData *data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    auto parentInode = FileOperationsHelper::FindCloudDiskInode(data, static_cast<int64_t>(parent));
    if (parentInode == nullptr) {
        LOGE("parent inode not found");
        return -EINVAL;
    }
    string cloudId;
    int32_t err = GenerateCloudId(data->userId, cloudId, parentInode->bundleName);
    if (err != 0) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{"", CloudFile::FaultOperation::MKNOD,
            CloudFile::FaultType::FILE, err, "Failed to generate cloud id"});
        return -err;
    }
    int32_t fd = CreateLocalFile(cloudId, parentInode->bundleName, data->userId, mode);
    if (fd < 0) {
        LOGD("Create local file failed error:%{public}d", fd);
        return fd;
    }
    string path = CloudFileUtils::GetLocalFilePath(cloudId, parentInode->bundleName, data->userId);
    bool noNeedUpload = false;
    if (parentInode->cloudId != ROOT_CLOUD_ID) {
        err = GetParentUpload(parentInode, data, noNeedUpload);
        if (err != 0) {
            CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{"", CloudFile::FaultOperation::MKNOD,
                CloudFile::FaultType::FILE, err, "Failed to get parent no upload"});
            close(fd);
            RemoveLocalFile(path);
            return -err;
        }
    }
    DatabaseManager &databaseManager = DatabaseManager::GetInstance();
    shared_ptr<CloudDiskRdbStore> rdbStore =
        databaseManager.GetRdbStore(parentInode->bundleName, data->userId);
    err = rdbStore->Create(cloudId, parentInode->cloudId, name, noNeedUpload);
    if (err != 0) {
        close(fd);
        RemoveLocalFile(path);
        return -EINVAL;
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
    if (WaitParameter("persist.kernel.move.finish", "true", MOVE_FILE_TIME_DAEMON) != 0) {
        LOGE("wait move error");
        return (void) fuse_reply_err(req, EBUSY);
    }
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
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
    if (WaitParameter("persist.kernel.move.finish", "true", MOVE_FILE_TIME_DAEMON) != 0) {
        LOGE("wait move error");
        return (void) fuse_reply_err(req, EBUSY);
    }
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
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
    fi->fh = static_cast<uint64_t>(data->fileId);
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
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
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
    size_t startPos = CloudSeekDir<T>(req, ino, off, childInfos);
    string buf;
    buf.resize(size);
    if (childInfos.empty() || startPos == childInfos.size()) {
        LOGW("empty buffer replied");
        return (void)fuse_reply_buf(req, buf.c_str(), 0);
    }

    size_t nextOff = 0;
    size_t remain = size;
    static const struct stat statInfoDir = { .st_mode = S_IFDIR | STAT_MODE_DIR };
    static const struct stat statInfoReg = { .st_mode = S_IFREG | STAT_MODE_REG };
    for (size_t i = startPos; i < childInfos.size(); i++) {
        size_t alignSize = CloudDiskRdbUtils::FuseDentryAlignSize(childInfos[i].name.c_str());
        if (alignSize > remain) {
            break;
        }
        alignSize = fuse_add_direntry(req, &buf[nextOff], alignSize, childInfos[i].name.c_str(),
            childInfos[i].mode != S_IFREG ? &statInfoDir : &statInfoReg,
            off + static_cast<off_t>(nextOff) + static_cast<off_t>(alignSize));
        nextOff += alignSize;
        remain -= alignSize;
    }
    (void)fuse_reply_buf(req, buf.c_str(), size - remain);
}

static void ReadDirForRecycle(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off,
                              struct fuse_file_info *fi)
{
    int32_t err = -1;
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    auto inode = FileOperationsHelper::FindCloudDiskInode(data, static_cast<int64_t>(ino));
    if (inode == nullptr) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{"", CloudFile::FaultOperation::READDIR,
            CloudFile::FaultType::INODE_FILE, EINVAL, "inode not found"});
        fuse_reply_err(req, EINVAL);
        return;
    }
    auto metaFile = MetaFileMgr::GetInstance().GetCloudDiskMetaFile(data->userId,
        inode->bundleName, RECYCLE_NAME);
    std::vector<MetaBase> childInfos;
    err = metaFile->LoadChildren(childInfos);
    if (err != 0) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{inode->bundleName, CloudFile::FaultOperation::READDIR,
            CloudFile::FaultType::DENTRY_FILE, err, "load children failed, err = " + std::to_string(err)});
        fuse_reply_err(req, EINVAL);
        return;
    }
    size_t nextOff = 0;
    for (size_t i = 0; i < childInfos.size(); ++i) {
        size_t alignSize = CloudDiskRdbUtils::FuseDentryAlignSize(childInfos[i].name.c_str());
        nextOff += alignSize;
        childInfos[i].nextOff = static_cast<off_t>(nextOff);
    }
    AddDirEntryToBuf(req, ino, size, off, childInfos);
}

void FileOperationsCloud::ReadDir(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off,
                                  struct fuse_file_info *fi)
{
    if (WaitParameter("persist.kernel.move.finish", "true", MOVE_FILE_TIME_DAEMON) != 0) {
        LOGE("wait move error");
        return (void) fuse_reply_err(req, EBUSY);
    }
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    if (ino == RECYCLE_LOCAL_ID) {
        ReadDirForRecycle(req, ino, size, off, fi);
        return;
    }

    vector<CloudDiskFileInfo> childInfos;
    int32_t err = GetChildInfos(req, ino, childInfos);
    if (err != 0) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{"", CloudFile::FaultOperation::READDIR,
            CloudFile::FaultType::QUERY_DATABASE, err, "failed to get child infos, err = " + std::to_string(err)});
        return (void)fuse_reply_err(req, EINVAL);
    }
    AddDirEntryToBuf(req, ino, size, off, childInfos);
}

int32_t CheckXattr(const char *name)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    LOGD("start CheckXattr name is:%{public}s", name);
    if (CloudFileUtils::CheckIsHmdfsPermission(name)) {
        return HMDFS_PERMISSION;
    } else if (CloudFileUtils::CheckIsCloudLocation(name)) {
        return CLOUD_LOCATION;
    } else if (CloudFileUtils::CheckIsCloudRecycle(name)) {
        return CLOUD_RECYCLE;
    } else if (CloudFileUtils::CheckIsFavorite(name)) {
        return IS_FAVORITE;
    } else if (CloudFileUtils::CheckIsHasLCD(name) || CloudFileUtils::CheckIsHasTHM(name)) {
        return HAS_THM;
    } else {
        LOGD("no definition Xattr name:%{public}s", name);
        return ERROR_CODE;
    }
}

void HandleCloudLocation(fuse_req_t req, fuse_ino_t ino, const char *name,
                         const char *value)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
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

void HandleCloudRecycle(fuse_req_t req, fuse_ino_t ino, const char *name,
                        const char *value)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    auto inoPtr = FileOperationsHelper::FindCloudDiskInode(data, static_cast<int64_t>(ino));
    if (inoPtr == nullptr) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{"", CloudFile::FaultOperation::SETEXTATTR,
            CloudFile::FaultType::INODE_FILE, EINVAL, "inode not found"});
        fuse_reply_err(req, EINVAL);
        return;
    }
    string parentCloudId;
    DatabaseManager &databaseManager = DatabaseManager::GetInstance();
    auto parentInode = FileOperationsHelper::FindCloudDiskInode(data, static_cast<int64_t>(inoPtr->parent));
    auto rdbStore = databaseManager.GetRdbStore(inoPtr->bundleName, data->userId);
    if (parentInode == nullptr) {
        int32_t ret = rdbStore->GetParentCloudId(inoPtr->cloudId, parentCloudId);
        if (ret != 0) {
            CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{"", CloudFile::FaultOperation::SETEXTATTR,
                CloudFile::FaultType::DATABASE, ret, "fail to get parentCloudId"});
            fuse_reply_err(req, EINVAL);
            return;
        }
    } else {
        parentCloudId = parentInode->cloudId;
    }
    int32_t ret = MetaFileMgr::GetInstance().CreateRecycleDentry(data->userId, inoPtr->bundleName);
    if (ret != 0) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{inoPtr->bundleName,
            CloudFile::FaultOperation::SETEXTATTR, CloudFile::FaultType::DENTRY_FILE, ret,
            "create recycle dentry failed"});
        fuse_reply_err(req, ret);
        return;
    }
    ret = rdbStore->SetXAttr(inoPtr->cloudId, CLOUD_CLOUD_RECYCLE_XATTR, value,
        inoPtr->fileName, parentCloudId);
    if (ret != 0) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{inoPtr->bundleName,
            CloudFile::FaultOperation::SETEXTATTR, CloudFile::FaultType::MODIFY_DATABASE, ret,
            "set cloud recycle xattr fail, ret = " + std::to_string(ret)});
        fuse_reply_err(req, EINVAL);
        return;
    }
    int32_t val = std::stoi(value);
    CloudDiskNotify::GetInstance().TryNotify({data, FileOperationsHelper::FindCloudDiskInode,
        val == 0 ? NotifyOpsType::DAEMON_RESTORE : NotifyOpsType::DAEMON_RECYCLE, inoPtr});
    fuse_reply_err(req, 0);
}

void HandleFavorite(fuse_req_t req, fuse_ino_t ino, const char *name,
                    const char *value)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    auto inoPtr = FileOperationsHelper::FindCloudDiskInode(data, static_cast<int64_t>(ino));
    if (inoPtr == nullptr) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{"", CloudFile::FaultOperation::SETEXTATTR,
            CloudFile::FaultType::INODE_FILE, EINVAL, "inode not found"});
        fuse_reply_err(req, EINVAL);
        return;
    }
    DatabaseManager &databaseManager = DatabaseManager::GetInstance();
    auto rdbStore = databaseManager.GetRdbStore(inoPtr->bundleName, data->userId);
    string nameStr = name;
    int32_t err = rdbStore->SetXAttr(inoPtr->cloudId, IS_FAVORITE_XATTR, value, nameStr);
    if (err != 0) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{inoPtr->bundleName, CloudFile::FaultOperation::SETEXTATTR,
            CloudFile::FaultType::MODIFY_DATABASE, err, "set cloud is favorite xattr fail, err = " +
            std::to_string(err)});
        fuse_reply_err(req, EINVAL);
        return;
    }
    CloudDiskNotify::GetInstance().TryNotify({data, FileOperationsHelper::FindCloudDiskInode,
        NotifyOpsType::DAEMON_SETXATTR, inoPtr});
    fuse_reply_err(req, 0);
}

void HandleHasTHM(fuse_req_t req, fuse_ino_t ino, const char *name,
                  const char *value)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    auto inoPtr = FileOperationsHelper::FindCloudDiskInode(data, static_cast<int64_t>(ino));
    if (inoPtr == nullptr) {
        fuse_reply_err(req, EINVAL);
        std::string errMsg = "inode not found";
            CLOUD_FILE_FAULT_REPORT(CloudFileFaultInfo{"", CloudFile::FaultOperation::SETATTR,
                CloudFile::FaultType::DENTRY_FILE, EINVAL, errMsg});
        return;
    }
    DatabaseManager &databaseManager = DatabaseManager::GetInstance();
    auto rdbStore = databaseManager.GetRdbStore(inoPtr->bundleName, data->userId);
    string nameStr = name;
    int32_t err = rdbStore->SetXAttr(inoPtr->cloudId, name, value, nameStr);
    if (err != 0) {
        std::string errMsg = "set has thm fail " + to_string(err);
            CLOUD_FILE_FAULT_REPORT(CloudFileFaultInfo{inoPtr->bundleName, CloudFile::FaultOperation::SETATTR,
                CloudFile::FaultType::DENTRY_FILE, EINVAL, errMsg});
        fuse_reply_err(req, EINVAL);
        return;
    }
    fuse_reply_err(req, 0);
}

void HandleExtAttribute(fuse_req_t req, fuse_ino_t ino, const char *name, const char *value)
{
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    auto inoPtr = FileOperationsHelper::FindCloudDiskInode(data, static_cast<int64_t>(ino));
    if (inoPtr == nullptr) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{"", CloudFile::FaultOperation::SETEXTATTR,
            CloudFile::FaultType::INODE_FILE, EINVAL, "inode not found"});
        fuse_reply_err(req, EINVAL);
        return;
    }
    DatabaseManager &databaseManager = DatabaseManager::GetInstance();
    auto rdbStore = databaseManager.GetRdbStore(inoPtr->bundleName, data->userId);
    string nameStr = name;
    int32_t err = rdbStore->SetXAttr(inoPtr->cloudId, CLOUD_EXT_ATTR, value, nameStr);
    if (err != 0) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{inoPtr->bundleName, CloudFile::FaultOperation::SETEXTATTR,
            CloudFile::FaultType::MODIFY_DATABASE, err, "set cloud ext attr fail, err = " + std::to_string(err)});
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
    if (WaitParameter("persist.kernel.move.finish", "true", MOVE_FILE_TIME_DAEMON) != 0) {
        LOGE("wait move error");
        return (void) fuse_reply_err(req, EBUSY);
    }
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
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
        case HAS_THM:
            HandleHasTHM(req, ino, name, value);
            break;
        default:
            HandleExtAttribute(req, ino, name, value);
            break;
    }
}

string GetIsFavorite(fuse_req_t req, shared_ptr<CloudDiskInode> inoPtr)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    string favorite;
    DatabaseManager &databaseManager = DatabaseManager::GetInstance();
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    auto rdbStore = databaseManager.GetRdbStore(inoPtr->bundleName, data->userId);
    int res = rdbStore->GetXAttr(inoPtr->cloudId, IS_FAVORITE_XATTR, favorite);
    if (res != 0) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{"", CloudFile::FaultOperation::GETEXTATTR,
            CloudFile::FaultType::QUERY_DATABASE, res, "local file get isFavorite fail"});
        return "null";
    }
    return favorite;
}

static string GetFileStatus(fuse_req_t req, struct CloudDiskInode *inoPtr)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    string fileStatus;
    if (inoPtr == nullptr) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{"", CloudFile::FaultOperation::GETEXTATTR,
            CloudFile::FaultType::INODE_FILE, EINVAL, "inoPtr is null"});
        return "null";
    }
    DatabaseManager &databaseManager = DatabaseManager::GetInstance();
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    auto rdbStore = databaseManager.GetRdbStore(inoPtr->bundleName, data->userId);
    int res = rdbStore->GetXAttr(inoPtr->cloudId, IS_FILE_STATUS_XATTR, fileStatus);
    if (res != 0) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{"", CloudFile::FaultOperation::GETEXTATTR,
            CloudFile::FaultType::QUERY_DATABASE, res, "local file get file_status fail"});
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
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{"", CloudFile::FaultOperation::GETEXTATTR,
            CloudFile::FaultType::INODE_FILE, EINVAL, "parent inode not found"});
        return "null";
    }
    CacheNode newNode = {.parentCloudId = parentInode->cloudId, .fileName = inoPtr->fileName};
    int res = rdbStore->GetXAttr(inoPtr->cloudId, CLOUD_FILE_LOCATION, location, newNode);
    if (res != 0) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{"", CloudFile::FaultOperation::GETEXTATTR,
            CloudFile::FaultType::QUERY_DATABASE, res, "local file get location fail"});
        return "null";
    }
    return location;
}

string GetTimeRecycled(fuse_req_t req, shared_ptr<CloudDiskInode> inoPtr)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    string timeRecycled;
    DatabaseManager &databaseManager = DatabaseManager::GetInstance();
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    auto rdbStore = databaseManager.GetRdbStore(inoPtr->bundleName, data->userId);
    int res = rdbStore->GetXAttr(inoPtr->cloudId, CLOUD_TIME_RECYCLED, timeRecycled);
    if (res != 0) {
        LOGE("local file get time recycled fail");
        return "null";
    }
    return timeRecycled;
}

static string GetRecyclePath(fuse_req_t req, shared_ptr<CloudDiskInode> inoPtr)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    DatabaseManager &databaseManager = DatabaseManager::GetInstance();
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    auto rdbStore = databaseManager.GetRdbStore(inoPtr->bundleName, data->userId);
    int64_t rowId;
    int res = rdbStore->GetRowId(inoPtr->cloudId, rowId);
    if (res != 0) {
        LOGE("local file get recycle path fail");
        auto parentInode = FileOperationsHelper::FindCloudDiskInode(data, static_cast<int64_t>(inoPtr->parent));
        if (parentInode == nullptr) {
            LOGE("parent inode not found");
            return "null";
        }
        int32_t ret = rdbStore->GenerateNewRowId(inoPtr->cloudId, inoPtr->fileName, rowId, parentInode->cloudId);
        if (ret != 0) {
            LOGE("GenerateRowId failed , ret is %{public}d", ret);
            return "null";
        }
    }
    string recyclePath = LOCAL_PATH_DATA_STORAGE + RECYCLE_NAME + "/" +
        inoPtr->fileName + "_" + to_string(rowId);
    return recyclePath;
}

string GetExtAttr(fuse_req_t req, shared_ptr<CloudDiskInode> inoPtr, const char *extAttrKey)
{
    string extAttr;
    if (inoPtr == nullptr) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{"", CloudFile::FaultOperation::GETEXTATTR,
            CloudFile::FaultType::INODE_FILE, EINVAL, "get ext attr inoPtr is null"});
        return "null";
    }

    DatabaseManager &databaseManager = DatabaseManager::GetInstance();
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    auto rdbStore = databaseManager.GetRdbStore(inoPtr->bundleName, data->userId);
    CacheNode newNode = {};
    int res = rdbStore->GetXAttr(inoPtr->cloudId, CLOUD_EXT_ATTR, extAttr, newNode, extAttrKey);
    if (res != 0) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{"", CloudFile::FaultOperation::GETEXTATTR,
            CloudFile::FaultType::FILE, res, "get ext attr is null"});
        return "null";
    }
    return extAttr;
}

void FileOperationsCloud::GetXattr(fuse_req_t req, fuse_ino_t ino, const char *name,
                                   size_t size)
{
    if (WaitParameter("persist.kernel.move.finish", "true", MOVE_FILE_TIME_DAEMON) != 0) {
        LOGE("wait move error");
        return (void) fuse_reply_err(req, EBUSY);
    }
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    auto inoPtr = FileOperationsHelper::FindCloudDiskInode(data, static_cast<int64_t>(ino));
    if (inoPtr == nullptr) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{"", CloudFile::FaultOperation::GETEXTATTR,
            CloudFile::FaultType::INODE_FILE, EINVAL, "inode not found"});
        fuse_reply_err(req, EINVAL);
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
    } else if (CloudFileUtils::CheckIsCloudLocation(name)) {
        buf = GetLocation(req, inoPtr);
    } else if (CloudFileUtils::CheckIsTimeRecycled(name)) {
        buf = GetTimeRecycled(req, inoPtr);
    } else if (CloudFileUtils::CheckIsRecyclePath(name)) {
        buf = GetRecyclePath(req, inoPtr);
    } else {
        buf = GetExtAttr(req, inoPtr, name);
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
    if (WaitParameter("persist.kernel.move.finish", "true", MOVE_FILE_TIME_DAEMON) != 0) {
        LOGE("wait move error");
        return (void) fuse_reply_err(req, EBUSY);
    }
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    auto parentInode = FileOperationsHelper::FindCloudDiskInode(data, static_cast<int64_t>(parent));
    if (parentInode == nullptr) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{"", CloudFile::FaultOperation::MKDIR,
            CloudFile::FaultType::INNER_ERROR, EINVAL, "parent inode not found"});
        return (void) fuse_reply_err(req, EINVAL);
    }
    string fileName = name;
    bool noNeedUpload;
    if ((fileName == ".cloudthumbnails" || fileName == CONFLICT_PATH) && parentInode->cloudId == ROOT_CLOUD_ID) {
        noNeedUpload = true;
    } else if (parentInode->cloudId != ROOT_CLOUD_ID) {
        int32_t err = GetParentUpload(parentInode, data, noNeedUpload);
        if (err != 0) {
            LOGE("Failed to get parent no upload");
            return (void) fuse_reply_err(req, err);
        }
    }
    string cloudId;
    int32_t err = GenerateCloudId(data->userId, cloudId, parentInode->bundleName);
    if (err != 0) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{parentInode->bundleName,
            CloudFile::FaultOperation::MKDIR, CloudFile::FaultType::FILE, err, "Failed to generate cloud id"});
        return (void) fuse_reply_err(req, err);
    }

    DatabaseManager &databaseManager = DatabaseManager::GetInstance();
    shared_ptr<CloudDiskRdbStore> rdbStore = databaseManager.GetRdbStore(parentInode->bundleName, data->userId);
    err = rdbStore->MkDir(cloudId, parentInode->cloudId, name, noNeedUpload);
    if (err != 0) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{"", CloudFile::FaultOperation::MKDIR,
            CloudFile::FaultType::DATABASE, err, "Failed to mkdir to DB err:" + std::to_string(err)});
        return (void) fuse_reply_err(req, EINVAL);
    }

    struct fuse_entry_param e;
    err = DoCloudLookup(req, parent, name, &e);
    if (err != 0) {
        LOGE("Failed to find dir %{private}s", GetAnonyString(name).c_str());
        fuse_reply_err(req, err);
    } else {
        fuse_reply_entry(req, &e);
    }
    CloudDiskNotify::GetInstance().TryNotify({data, FileOperationsHelper::FindCloudDiskInode,
        NotifyOpsType::DAEMON_MKDIR, parentInode, parent, name});
}

void RDBUnlinkAsync(shared_ptr<CloudDiskRdbStore> rdbStore, const string& cloudId, int32_t noUpload)
{
    function<void()> rdbUnlink = [rdbStore, cloudId, noUpload] {
        if (rdbStore->Unlink(cloudId, noUpload) != 0) {
            CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{"",
                CloudFile::FaultOperation::UNLINK, CloudFile::FaultType::DATABASE, EINVAL,
                "Failed to unlink DB cloudId: " + cloudId});
        }
    };
    ffrt::thread(rdbUnlink).detach();
}

int32_t DoCloudUnlink(fuse_req_t req, fuse_ino_t parent, const char *name)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    auto parentInode = FileOperationsHelper::FindCloudDiskInode(data, static_cast<int64_t>(parent));
    if (parentInode == nullptr) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{"", CloudFile::FaultOperation::UNLINK,
            CloudFile::FaultType::INODE_FILE, EINVAL, "parent inode not found"});
        return EINVAL;
    }
    DatabaseManager &databaseManager = DatabaseManager::GetInstance();
    shared_ptr<CloudDiskRdbStore> rdbStore = databaseManager.GetRdbStore(parentInode->bundleName, data->userId);
    MetaBase metaBase(name);
    auto metaFile = MetaFileMgr::GetInstance().GetCloudDiskMetaFile(data->userId,
        parentInode->bundleName, parentInode->cloudId);
    if (int32_t ret = metaFile->DoLookup(metaBase); ret != 0) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{parentInode->bundleName,
            CloudFile::FaultOperation::UNLINK, CloudFile::FaultType::DENTRY_FILE, ret,
            "lookup denty failed, name: " + GetAnonyString(name)});
        return ret;
    }
    string cloudId = metaBase.cloudId;
    int32_t isDirectory = S_ISDIR(metaBase.mode);
    int32_t position = metaBase.position;
    int32_t noUpload = metaBase.noUpload;
    if (int32_t ret = metaFile->DoRemove(metaBase); ret != 0) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{parentInode->bundleName,
            CloudFile::FaultOperation::UNLINK, CloudFile::FaultType::DENTRY_FILE, ret, "remove dentry failed, ret = " +
            std::to_string(ret)});
        return ret;
    }
    LOGD("doUnlink, dentry file has been deleted");
    if (isDirectory == FILE && position != CLOUD) {
        string localPath = CloudFileUtils::GetLocalFilePath(cloudId, parentInode->bundleName, data->userId);
        LOGI("unlink %{public}s", GetAnonyString(localPath).c_str());
        int32_t ret = unlink(localPath.c_str());
        if (ret != 0 && errno == ENOENT) {
            std::string errMsg = "doCloudUnlink, unlink local file ret ENOENT.";
            CLOUD_FILE_FAULT_REPORT(CloudFileFaultInfo{parentInode->bundleName, CloudFile::FaultOperation::UNLINK,
                CloudFile::FaultType::WARNING, errno, errMsg});
        } else if (ret != 0) {
            CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{parentInode->bundleName,
                CloudFile::FaultOperation::UNLINK, CloudFile::FaultType::FILE, errno,
                "Failed to unlink cloudId:" + cloudId + ", errno: " + std::to_string(errno)});
            (void)metaFile->DoCreate(metaBase);
            return errno;
        }
    }
    RDBUnlinkAsync(rdbStore, cloudId, noUpload);
    return 0;
}

std::shared_ptr<CloudDiskMetaFile> LoadMetaFileChildren(const CloudDiskFuseData* data, const MetaBase& metaBase,
    shared_ptr<CloudDiskInode> parentInode, const fuse_req_t& req)
{
    auto metaFile = MetaFileMgr::GetInstance().GetCloudDiskMetaFile(data->userId,
        parentInode->bundleName, metaBase.cloudId);
    std::vector<MetaBase> bases;
    if (int err = metaFile->LoadChildren(bases); err != 0) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{parentInode->bundleName,
            CloudFile::FaultOperation::RMDIR, CloudFile::FaultType::DENTRY_FILE, err, "load children failed, err = " +
            std::to_string(err)});
        fuse_reply_err(req, EINVAL);
        return nullptr;
    }
    if (!bases.empty()) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{parentInode->bundleName,
            CloudFile::FaultOperation::RMDIR, CloudFile::FaultType::DENTRY_FILE, ENOTEMPTY, "Directory not empty"});
        fuse_reply_err(req, ENOTEMPTY);
        return nullptr;
    }
    return metaFile;
}

void FileOperationsCloud::RmDir(fuse_req_t req, fuse_ino_t parent, const char *name)
{
    if (WaitParameter("persist.kernel.move.finish", "true", MOVE_FILE_TIME_DAEMON) != 0) {
        LOGE("wait move error");
        return (void) fuse_reply_err(req, EBUSY);
    }
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);

    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    auto parentInode = FileOperationsHelper::FindCloudDiskInode(data, static_cast<int64_t>(parent));
    if (parentInode == nullptr) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{"", CloudFile::FaultOperation::RMDIR,
            CloudFile::FaultType::INODE_FILE, EINVAL, "parent inode not found"});
        return (void) fuse_reply_err(req, EINVAL);
    }
    auto parentMetaFile = MetaFileMgr::GetInstance().GetCloudDiskMetaFile(data->userId,
        parentInode->bundleName, parentInode->cloudId);
    MetaBase metaBase(name);
    int32_t err = parentMetaFile->DoLookup(metaBase);
    if (err != 0) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{parentInode->bundleName,
            CloudFile::FaultOperation::RMDIR, CloudFile::FaultType::DENTRY_FILE, err, "lookup dir failed, err = " +
            std::to_string(err)});
        return (void) fuse_reply_err(req, EINVAL);
    }

    std::shared_ptr<CloudDiskMetaFile> metaFile = LoadMetaFileChildren(data, metaBase, parentInode, req);
    if (metaFile == nullptr) {
        return;
    }

    err = DoCloudUnlink(req, parent, name);
    if (err != 0) {
        fuse_reply_err(req, err);
        return;
    }
    MetaFileMgr::GetInstance().Clear(static_cast<uint32_t>(data->userId), parentInode->bundleName, metaBase.cloudId);
    if (unlink(metaFile->GetDentryFilePath().c_str()) != 0) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{parentInode->bundleName,
            CloudFile::FaultOperation::RMDIR, CloudFile::FaultType::WARNING, errno, "fail to delete dentry: " +
            std::to_string(errno)});
    }
    CloudDiskNotify::GetInstance().TryNotify({data, FileOperationsHelper::FindCloudDiskInode,
        NotifyOpsType::DAEMON_RMDIR, nullptr, parent, name});
    return (void) fuse_reply_err(req, 0);
}

void FileOperationsCloud::Unlink(fuse_req_t req, fuse_ino_t parent, const char *name)
{
    if (WaitParameter("persist.kernel.move.finish", "true", MOVE_FILE_TIME_DAEMON) != 0) {
        LOGE("wait move error");
        return (void) fuse_reply_err(req, EBUSY);
    }
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
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
    if (WaitParameter("persist.kernel.move.finish", "true", MOVE_FILE_TIME_DAEMON) != 0) {
        LOGE("wait move error");
        return (void) fuse_reply_err(req, EBUSY);
    }
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    if (flags) {
        LOGE("Fuse failed to support flag");
        return (void) fuse_reply_err(req, EINVAL);
    }
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    auto parentInode = FileOperationsHelper::FindCloudDiskInode(data, static_cast<int64_t>(parent));
    auto newParentInode = FileOperationsHelper::FindCloudDiskInode(data, static_cast<int64_t>(newParent));
    if (!parentInode || !newParentInode) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{"", CloudFile::FaultOperation::RENAME,
            CloudFile::FaultType::INODE_FILE, EINVAL, "rename old or new parent not found"});
        return (void) fuse_reply_err(req, EINVAL);
    }
    bool noNeedUpload = false;
    if (newParentInode->cloudId != ROOT_CLOUD_ID) {
        int32_t err = GetParentUpload(newParentInode, data, noNeedUpload);
        if (err != 0) {
            LOGE("Failed to get parent no upload");
            return (void) fuse_reply_err(req, err);
        }
    }
    bool sync = req->ctx.uid != SERVICE_UID;
    DatabaseManager &databaseManager = DatabaseManager::GetInstance();
    shared_ptr<CloudDiskRdbStore> rdbStore = databaseManager.GetRdbStore(parentInode->bundleName, data->userId);
    int32_t err = rdbStore->Rename(parentInode->cloudId, name, newParentInode->cloudId, newName, noNeedUpload, sync);
    if (err != 0) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{parentInode->bundleName,
            CloudFile::FaultOperation::RENAME, CloudFile::FaultType::DATABASE, err,
            "Failed to Rename DB name: " + GetAnonyString(name) + "err:" + std::to_string(err)});
        return (void) fuse_reply_err(req, EINVAL);
    }
    bool isDir = false;
    int64_t localId = FileOperationsHelper::FindLocalId(data, std::to_string(parent) + name);
    auto inoPtr = FileOperationsHelper::FindCloudDiskInode(data, localId);
    if (inoPtr != nullptr) {
        inoPtr->fileName = newName;
        inoPtr->parent = newParent;
        isDir = S_ISDIR(inoPtr->stat.st_mode);
    }
    if (sync || noNeedUpload) {
        CloudDiskNotify::GetInstance().TryNotify({data, FileOperationsHelper::FindCloudDiskInode,
            NotifyOpsType::DAEMON_RENAME, nullptr, parent, name, newParent, newName}, {FileStatus::UNKNOW, isDir});
    }
    return (void) fuse_reply_err(req, 0);
}

static int64_t ReadFileToBuf(const std::string& path, char* buf, size_t size, off_t off)
{
    int fd = open(path.c_str(), O_RDONLY);
    if (fd < 0) {
        LOGE("open file %{public}s failed!, fd: %d", GetAnonyStringStrictly(path).c_str(), fd);
        return -1;
    }
 
    ssize_t n;
    size_t readLen = 0;
    while (readLen < size) {
        n = pread(fd, buf + readLen, size - readLen, off + readLen);
        if (n > 0) {
            readLen += n;
        } else if (n == 0) {
            break;
        } else {
            LOGE("pread failed, return: %{public}zd", n);
            break;
        }
    }
 
    close(fd);
    int64_t result = static_cast<int64_t>(readLen);
    return result;
}

static void HandleReadCompletion(fuse_req_t req, shared_ptr<CloudDiskFile> filePtr, ReadCompletionParams params)
{
    unique_lock lck(filePtr->readLock);
    auto waitStatus = params.cond->wait_for(lck, READ_TIMEOUT_S, [params] {
        return *params.readFinish;
    });
    if (!waitStatus) {
        LOGE("PRead timeout");
        fuse_reply_err(req, ENETUNREACH);
        return;
    }
    if (*params.readSize < 0) {
        LOGE("read fail");
        fuse_reply_err(req, EIO);
        return;
    }

    auto ret = HandleCloudError(req, *params.error);
    if (ret == 0) {
        filePtr->type = CLOUD_DISK_FILE_TYPE_CLOUD;
        fuse_reply_buf(req, params.buf.get(), *params.readSize);
    } else {
        LOGE("read fail");
        fuse_reply_err(req, ret);
    }
    return;
}

static void DoCloudRead(fuse_req_t req, shared_ptr<CloudDiskFile> filePtr, CloudReadParams params)
{
    auto readSize = make_shared<int64_t>();
    auto error = make_shared<CloudError>();
    auto readFinish = make_shared<bool>(false);
    auto cond = make_shared<ffrt::condition_variable>();

    ffrt::submit([filePtr, error, readFinish, cond, params, readSize] {
        HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
        auto session = filePtr->readSession;
        if (!session) {
            LOGE("readSession is nullptr");
            {
                unique_lock lck(filePtr->readLock);
                *readFinish = true;
            }
            *error = CloudError::CK_LOCAL_ERROR;
            cond->notify_one();
            return;
        }
        *readSize = session->PRead(params.offset, params.size, *error);
        {
            unique_lock lck(filePtr->readLock);
            *readFinish = true;
        }
        if (*readSize == 0) {
            *readSize = ReadFileToBuf(params.path, params.buf.get(), params.size, params.offset);
        } else {
            LOGE("read cloud disk file failed,read result: %{public}" PRId64 ",error: %{public}d", *readSize, *error);
        }
        cond->notify_one();
        LOGI("download done");
        return;
    });

    ReadCompletionParams readParams = {cond, readFinish, error, readSize, params.buf};
    HandleReadCompletion(req, filePtr, readParams);
}

void FileOperationsCloud::Read(fuse_req_t req, fuse_ino_t ino, size_t size,
                               off_t offset, struct fuse_file_info *fi)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    if (size > MAX_READ_SIZE) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{"", CloudFile::FaultOperation::READ,
            CloudFile::FaultType::WARNING, EINVAL, "Read size is larger than the kernel pre-read window"});
        fuse_reply_err(req, EINVAL);
        return;
    }
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    auto inoPtr = FileOperationsHelper::FindCloudDiskInode(data, static_cast<int64_t>(ino));
    if (inoPtr == nullptr) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{"", CloudFile::FaultOperation::OPEN,
            CloudFile::FaultType::INODE_FILE, EINVAL, "inode not found"});
        fuse_reply_err(req, EINVAL);
        return;
    }
    auto filePtr = FileOperationsHelper::FindCloudDiskFile(data, fi->fh);
    if (filePtr == nullptr) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{"", CloudFile::FaultOperation::READ,
            CloudFile::FaultType::FILE, EINVAL, "file not found"});
        fuse_reply_err(req, EINVAL);
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

    shared_ptr<char> buf = nullptr;
    buf.reset(new char[size], [](char* ptr) {
        delete[] ptr;
    });

    if (!buf) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{"", CloudFile::FaultOperation::READ,
            CloudFile::FaultType::FILE, ENOMEM, "buffer is null"});
        fuse_reply_err(req, ENOMEM);
        return;
    }
    string path = CloudFileUtils::GetLocalDKCachePath(inoPtr->cloudId, inoPtr->bundleName, data->userId);
    CloudReadParams readParams = {offset, size, buf, path};
    DoCloudRead(req, filePtr, readParams);
}

static void UpdateCloudDiskInode(shared_ptr<CloudDiskRdbStore> rdbStore, shared_ptr<CloudDiskInode> inoPtr)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    CloudDiskFileInfo childInfo;
    int32_t err = rdbStore->GetAttr(inoPtr->cloudId, childInfo);
    if (err != 0) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{"", CloudFile::FaultOperation::GETATTR,
            CloudFile::FaultType::QUERY_DATABASE, err, "update file fail"});
        return;
    }
    inoPtr->stat.st_size = childInfo.size;
    inoPtr->stat.st_mtime = childInfo.mtime / MILLISECOND_TO_SECONDS_TIMES;
}

static void UpdateCloudStore(CloudDiskFuseData *data, const std::string &fileName, const std::string &parentCloudId,
    shared_ptr<CloudDiskInode> inoPtr, bool isWrite)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    DatabaseManager &databaseManager = DatabaseManager::GetInstance();
    auto rdbStore = databaseManager.GetRdbStore(inoPtr->bundleName, data->userId);
    int32_t dirtyType;
    int res = rdbStore->GetDirtyType(inoPtr->cloudId, dirtyType);
    if (res != 0) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{"", CloudFile::FaultOperation::RELEASE,
            CloudFile::FaultType::QUERY_DATABASE, res, "get file status fail, err: " + std::to_string(res)});
    }
    res = rdbStore->Write(fileName, parentCloudId, inoPtr->cloudId);
    if (res != 0) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{"", CloudFile::FaultOperation::RELEASE,
            CloudFile::FaultType::MODIFY_DATABASE, res, "write file fail"});
    }
    CloudDiskNotify::GetInstance().TryNotify({data, FileOperationsHelper::FindCloudDiskInode,
        NotifyOpsType::DAEMON_WRITE, inoPtr}, {dirtyType, false, isWrite});
    UpdateCloudDiskInode(rdbStore, inoPtr);
}

static int32_t UpdateCacheDentrySize(CloudDiskFuseData *data, fuse_ino_t ino)
{
    auto inoPtr = FileOperationsHelper::FindCloudDiskInode(data, static_cast<int64_t>(ino));
    if (inoPtr == nullptr) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{"", CloudFile::FaultOperation::WRITE,
            CloudFile::FaultType::INODE_FILE, EINVAL, "inode not found"});
        return EINVAL;
    }
    string filePath = CloudFileUtils::GetLocalFilePath(inoPtr->cloudId, inoPtr->bundleName, data->userId);
    struct stat statInfo {};
    int32_t ret = stat(filePath.c_str(), &statInfo);
    if (ret != 0) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{inoPtr->bundleName, CloudFile::FaultOperation::WRITE,
            CloudFile::FaultType::FILE, errno, "filePath " + GetAnonyString(filePath) + " is invalid"});
        return errno;
    }
    MetaBase metaBase(inoPtr->fileName);
    metaBase.mtime = static_cast<uint64_t>(CloudFileUtils::Timespec2Milliseconds(statInfo.st_mtim));
    metaBase.size = static_cast<uint64_t>(statInfo.st_size);
    auto callback = [&metaBase] (MetaBase &m) {
        m.size = metaBase.size;
        m.mtime = metaBase.mtime;
    };
    auto parentInode = FileOperationsHelper::FindCloudDiskInode(data,
        static_cast<int64_t>(inoPtr->parent));
    if (parentInode == nullptr) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{"", CloudFile::FaultOperation::WRITE,
            CloudFile::FaultType::INODE_FILE, EINVAL, "fail to find parent inode"});
        return EINVAL;
    }
    string parentCloudId = parentInode->cloudId;
    auto metaFile = MetaFileMgr::GetInstance().GetCloudDiskMetaFile(data->userId, inoPtr->bundleName, parentCloudId);
    ret = metaFile->DoChildUpdate(inoPtr->fileName, callback);
    if (ret != 0) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{"", CloudFile::FaultOperation::WRITE,
            CloudFile::FaultType::DENTRY_FILE, ret, "update new dentry failed, ret = " + std::to_string(ret)});
        return ret;
    }
    inoPtr->stat.st_size = static_cast<decltype(inoPtr->stat.st_size)>(metaBase.size);
    inoPtr->stat.st_mtime =
        static_cast<decltype(inoPtr->stat.st_mtime)>(metaBase.mtime / MILLISECOND_TO_SECONDS_TIMES);
    return 0;
}

void FileOperationsCloud::WriteBuf(fuse_req_t req, fuse_ino_t ino, struct fuse_bufvec *bufv,
                                   off_t off, struct fuse_file_info *fi)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    struct fuse_bufvec out_buf = FUSE_BUFVEC_INIT(fuse_buf_size(bufv));
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    auto filePtr = FileOperationsHelper::FindCloudDiskFile(data, fi->fh);
    if (filePtr == nullptr) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{"", CloudFile::FaultOperation::WRITE,
            CloudFile::FaultType::DRIVERKIT, EINVAL, "file not found"});
        fuse_reply_err(req, EINVAL);
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
        int32_t ret = UpdateCacheDentrySize(data, ino);
        if (ret != 0) {
            LOGE("write size in cache and dentry fail, ret = %{public}d", ret);
        }
        fuse_reply_write(req, (size_t) res);
    }
}

static void UploadLocalFile(CloudDiskFuseData *data, const std::string &fileName, const std::string &parentCloudId,
    shared_ptr<CloudDiskInode> inoPtr, bool isWrite)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    MetaBase metaBase(fileName);
    auto metaFile = MetaFileMgr::GetInstance().GetCloudDiskMetaFile(data->userId, inoPtr->bundleName, parentCloudId);
    int32_t ret = metaFile->DoLookup(metaBase);
    if (ret != 0) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{inoPtr->bundleName, CloudFile::FaultOperation::RELEASE,
            CloudFile::FaultType::DENTRY_FILE, ret, "local file get location from dentryfile fail, ret = " +
            std::to_string(ret)});
    } else if (metaBase.position == LOCAL || isWrite) {
        DatabaseManager &databaseManager = DatabaseManager::GetInstance();
        auto rdbStore = databaseManager.GetRdbStore(inoPtr->bundleName, data->userId);
        int32_t dirtyType;
        ret = rdbStore->GetDirtyType(inoPtr->cloudId, dirtyType);
        if (ret != 0) {
            CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{inoPtr->bundleName,
                CloudFile::FaultOperation::RELEASE, CloudFile::FaultType::DRIVERKIT_DATABASE, ret,
                "get file status fail, err: " + std::to_string(ret)});
        }
        ret = rdbStore->Write(fileName, parentCloudId, inoPtr->cloudId);
        if (ret != 0) {
            CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{"", CloudFile::FaultOperation::RELEASE,
                CloudFile::FaultType::DRIVERKIT_DATABASE, ret, "write file fail"});
        }
        CloudDiskNotify::GetInstance().TryNotify({data, FileOperationsHelper::FindCloudDiskInode,
            NotifyOpsType::DAEMON_WRITE, inoPtr}, {dirtyType, false, isWrite});
        UpdateCloudDiskInode(rdbStore, inoPtr);
    }
}

static int32_t HandleLocalClose(fuse_req_t req, shared_ptr<CloudDiskInode> inoPtr,
    shared_ptr<CloudDiskFile> filePtr, const string &parentCloudId)
{
    struct stat statInfo {};
    if (fstat(filePtr->fd, &statInfo) != 0) {
        LOGE("fstat failed, errno %{public}d", errno);
        filePtr->refCount++;
        return errno;
    }
    close(filePtr->fd);
    filePtr->readSession = nullptr;
    auto editedTime = static_cast<uint64_t>(CloudFileUtils::Timespec2Milliseconds(statInfo.st_mtim));
    bool isWrite = ((filePtr->fileDirty == CLOUD_DISK_FILE_WRITE) || (editedTime > filePtr->mtime));
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    if (filePtr->fileDirty != CLOUD_DISK_FILE_UNKNOWN) {
        UpdateCloudStore(data, inoPtr->fileName, parentCloudId, inoPtr, isWrite);
    } else if (filePtr->isWriteOpen) {
        UploadLocalFile(data, inoPtr->fileName, parentCloudId, inoPtr, isWrite);
    }
    return 0;
}

static void HandleRelease(std::shared_ptr<CloudDiskFile> filePtr, std::shared_ptr<CloudDiskInode> inoPtr,
    CloudDiskFuseData *data)
{
    std::unique_lock<std::shared_mutex> lck(inoPtr->sessionLock);
    filePtr->readSession->sessionCount--;
    if (filePtr->readSession->sessionCount == 0) {
        bool res = filePtr->readSession->Close(false);
        if (!res) {
            LOGE("close error");
        }
        string path = CloudFileUtils::GetLocalDKCachePath(inoPtr->cloudId, inoPtr->bundleName, data->userId);
        ErasePathCache(path, data);
        filePtr->readSession = nullptr;
        LOGD("readSession released");
    }
}

void FileOperationsCloud::Release(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    auto inoPtr = FileOperationsHelper::FindCloudDiskInode(data, static_cast<int64_t>(ino));
    if (inoPtr == nullptr) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{"", CloudFile::FaultOperation::RELEASE,
            CloudFile::FaultType::INODE_FILE, EINVAL, "inode not found"});
        return (void)fuse_reply_err(req, EINVAL);
    }
    auto parentInode = FileOperationsHelper::FindCloudDiskInode(data,
        static_cast<int64_t>(inoPtr->parent));
    if (parentInode == nullptr) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{"", CloudFile::FaultOperation::RELEASE,
            CloudFile::FaultType::INODE_FILE, EINVAL, "fail to find parent inode"});
        return (void)fuse_reply_err(req, EINVAL);
    }
    string parentCloudId = parentInode->cloudId;
    auto filePtr = FileOperationsHelper::FindCloudDiskFile(data, fi->fh);
    if (filePtr == nullptr) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{parentInode->bundleName,
            CloudFile::FaultOperation::RELEASE, CloudFile::FaultType::DRIVERKIT, EINVAL, "file not found"});
        return (void)fuse_reply_err(req, EINVAL);
    }
    filePtr->refCount--;
    if (filePtr->refCount == 0) {
        if (filePtr->type == CLOUD_DISK_FILE_TYPE_LOCAL) {
            auto ret = HandleLocalClose(req, inoPtr, filePtr, parentCloudId);
            if (ret) {
                return (void)fuse_reply_err(req, ret);
            }
        } else if (filePtr->type == CLOUD_DISK_FILE_TYPE_CLOUD && filePtr->readSession != nullptr) {
            HandleRelease(filePtr, inoPtr, data);
        }
        FileOperationsHelper::PutCloudDiskFile(data, filePtr, fi->fh);
    }
    fuse_inval(data->se, inoPtr->parent, ino, inoPtr->fileName);
    fuse_reply_err(req, 0);
}

void HandleTruncateError(shared_ptr<CloudDiskInode> inoPtr, shared_ptr<CloudDiskRdbStore> rdbStore,
    shared_ptr<CloudDiskInode> parentInode, fuse_req_t req)
{
    CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{inoPtr->bundleName,
        CloudFile::FaultOperation::SETATTR, CloudFile::FaultType::FILE, errno, "truncate failed, err: " +
        std::to_string(errno)});
    struct stat *statPtr = &inoPtr->stat;
    int res = rdbStore->SetAttr(inoPtr->fileName, parentInode->cloudId, inoPtr->cloudId, statPtr, FUSE_SET_ATTR_SIZE);
    if (res != 0) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{inoPtr->bundleName,
            CloudFile::FaultOperation::SETATTR, CloudFile::FaultType::DATABASE, res,
            "update rdb size failed, res: " + std::to_string(res)});
        fuse_reply_err(req, EINVAL);
    } else {
        fuse_reply_err(req, errno);
    }
}

std::optional<int32_t> FuseFileTruncate(struct fuse_file_info* fi, struct CloudDiskFuseData* data,
    shared_ptr<CloudDiskInode> inoPtr, fuse_req_t req, struct stat *attr)
{
    int32_t res = -1;
    if (fi) {
        auto filePtr = FileOperationsHelper::FindCloudDiskFile(data, fi->fh);
        if (filePtr == nullptr) {
            CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{inoPtr->bundleName,
                CloudFile::FaultOperation::SETATTR, CloudFile::FaultType::DRIVERKIT, EINVAL, "file not found"});
            fuse_reply_err(req, EINVAL);
            return std::nullopt;
        }
        res = ftruncate(filePtr->fd, attr->st_size);
    } else {
        string path = CloudFileUtils::GetLocalFilePath(inoPtr->cloudId, inoPtr->bundleName, data->userId);
        res = truncate(path.c_str(), attr->st_size);
    }
    return res;
}

int32_t HandleSize(shared_ptr<CloudDiskRdbStore> rdbStore, struct CloudDiskFuseData* data,
    shared_ptr<CloudDiskInode> inoPtr, shared_ptr<CloudDiskInode> parentInode, struct stat *attr)
{
    int32_t res = rdbStore->SetAttr(inoPtr->fileName, parentInode->cloudId, inoPtr->cloudId, attr, FUSE_SET_ATTR_SIZE);
    if (res != 0) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{inoPtr->bundleName,
            CloudFile::FaultOperation::SETATTR, CloudFile::FaultType::DATABASE, res,
            "update rdb size failed, res: " + std::to_string(res)});
    }
    return res;
}

int32_t HandleMtime(shared_ptr<CloudDiskRdbStore> rdbStore, struct CloudDiskFuseData* data,
    shared_ptr<CloudDiskInode> inoPtr, shared_ptr<CloudDiskInode> parentInode, struct stat *attr)
{
    int32_t res = rdbStore->SetAttr(inoPtr->fileName, parentInode->cloudId, inoPtr->cloudId, attr, FUSE_SET_ATTR_MTIME);
    if (res != 0) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{inoPtr->bundleName,
            CloudFile::FaultOperation::SETATTR, CloudFile::FaultType::DATABASE, res,
            "update rdb mtime failed, res: " + std::to_string(res)});
    }
    return res;
}

void FileOperationsCloud::SetAttr(fuse_req_t req, fuse_ino_t ino, struct stat *attr,
                                  int valid, struct fuse_file_info *fi)
{
    if (WaitParameter("persist.kernel.move.finish", "true", MOVE_FILE_TIME_DAEMON) != 0) {
        LOGE("wait move error");
        return (void) fuse_reply_err(req, EBUSY);
    }
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    LOGD("SetAttr begin valid:%{public}d", valid);
    auto data = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
    auto inoPtr = FileOperationsHelper::FindCloudDiskInode(data, static_cast<int64_t>(ino));
    if (inoPtr == nullptr) {
        CLOUD_FILE_FAULT_REPORT(CloudFile::CloudFileFaultInfo{"", CloudFile::FaultOperation::SETATTR,
            CloudFile::FaultType::INODE_FILE, EINVAL, "get an invalid inode!"});
        return (void) fuse_reply_err(req, EINVAL);
    }
    auto parentInode = FileOperationsHelper::FindCloudDiskInode(data, static_cast<int64_t>(inoPtr->parent));
    if (parentInode == nullptr) {
        LOGE("parent inode not found");
        fuse_reply_err(req, EINVAL);
        return;
    }
    DatabaseManager &databaseManager = DatabaseManager::GetInstance();
    auto rdbStore = databaseManager.GetRdbStore(inoPtr->bundleName, data->userId);
    if (static_cast<unsigned int>(valid) & FUSE_SET_ATTR_SIZE) {
        int32_t res = HandleSize(rdbStore, data, inoPtr, parentInode, attr);
        if (res != 0) {
            return (void) fuse_reply_err(req, EINVAL);
        }
        auto ret = FuseFileTruncate(fi, data, inoPtr, req, attr);
        if (!ret.has_value()) {
            return;
        }
        if (ret.value() == -1) {
            HandleTruncateError(inoPtr, rdbStore, parentInode, req);
            return;
        }
    }
    if (static_cast<unsigned int>(valid) & FUSE_SET_ATTR_MTIME) {
        int32_t res = HandleMtime(rdbStore, data, inoPtr, parentInode, attr);
        if (res != 0) {
            return (void) fuse_reply_err(req, EINVAL);
        }
    }
    UpdateCloudDiskInode(rdbStore, inoPtr);
    CloudDiskNotify::GetInstance().TryNotify({data, FileOperationsHelper::FindCloudDiskInode,
        NotifyOpsType::DAEMON_SETATTR, inoPtr});
    fuse_reply_attr(req, &inoPtr->stat, 0);
}

void FileOperationsCloud::Lseek(fuse_req_t req, fuse_ino_t ino, off_t off, int whence,
                                struct fuse_file_info *fi)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
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

void FileOperationsCloud::Ioctl(fuse_req_t req, fuse_ino_t ino, int cmd, void *arg, struct fuse_file_info *fi,
                                unsigned flags, const void *inBuf, size_t inBufsz, size_t outBufsz)
{
    if (static_cast<unsigned int>(cmd) == HMDFS_IOC_COPY_FILE) {
        auto dataReq = reinterpret_cast<struct CloudDiskFuseData *>(fuse_req_userdata(req));
        auto inoPtr = FileOperationsHelper::FindCloudDiskInode(dataReq, static_cast<int64_t>(ino));
        if (inoPtr == nullptr) {
            LOGE("Failed to find inode for cloud disk");
            fuse_reply_err(req, EINVAL);
            return;
        }
        if (S_ISDIR(inoPtr->stat.st_mode)) {
            LOGE("Dir is not supported");
            fuse_reply_err(req, ENOSYS);
            return;
        }

        const struct CloudDiskCopy *dataCopy = reinterpret_cast<const struct CloudDiskCopy *>(inBuf);
        std::string dest(dataCopy->destPath);
        std::string destPath = CloudFileUtils::GetRealPath(dest);
        DatabaseManager &databaseManager = DatabaseManager::GetInstance();
        shared_ptr<CloudDiskRdbStore> rdbStore =
            databaseManager.GetRdbStore(inoPtr->bundleName, dataReq->userId);

        std::string destCloudId;
        int32_t ret = GenerateCloudId(dataReq->userId, destCloudId, inoPtr->bundleName);
        if (ret != 0) {
            LOGE("Failed generate cloud id, ret: %{public}d", ret);
            fuse_reply_err(req, ENOSYS);
            return;
        }
        if ((rdbStore->CopyFile(inoPtr->cloudId, destCloudId, inoPtr->bundleName, dataReq->userId, destPath) != 0)) {
            fuse_reply_err(req, EIO);
            return;
        }
        fuse_reply_ioctl(req, 0, NULL, 0);
    } else {
        LOGE("Invalid argument, cmd is not supported");
        fuse_reply_err(req, EINVAL);
        return;
    }
}
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS
