/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "fuse_manager/fuse_manager.h"

#include <atomic>
#include <cassert>
#include <cerrno>
#include <chrono>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <exception>
#include <filesystem>
#include <fcntl.h>
#include <iostream>
#include <map>
#include <mutex>
#include <pthread.h>
#include <shared_mutex>
#include <stdexcept>
#include <string>
#include <sys/stat.h>
#include <sys/xattr.h>
#include <thread>
#include <unistd.h>
#include <sys/ioctl.h>
#include <vector>

#include "cloud_daemon_statistic.h"
#include "cloud_disk_inode.h"
#include "cloud_file_kit.h"
#include "clouddisk_type_const.h"
#include "datetime_ex.h"
#include "dfs_error.h"
#include "directory_ex.h"
#include "ffrt_inner.h"
#include "fuse_operations.h"
#include "hitrace_meter.h"
#include "meta_file.h"
#include "securec.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudFile {
using namespace std;
using PAGE_FLAG_TYPE = const uint32_t;

static const string LOCAL_PATH_DATA_SERVICE_EL2 = "/data/service/el2/";
static const string LOCAL_PATH_HMDFS_CLOUD_CACHE = "/hmdfs/cache/cloud_cache";
static const string CLOUD_CACHE_DIR = "/pread_cache";
static const string CLOUD_CACHE_XATTR_NAME = "user.cloud.cacheMap";
static const string VIDEO_TYPE_PREFIX = "VID_";
static const string HMDFS_PATH_PREFIX = "/mnt/hmdfs/";
static const string LOCAL_PATH_SUFFIX = "/account/device_view/local";
static const string CLOUD_MERGE_VIEW_PATH_SUFFIX = "/account/cloud_merge_view";
static const string PATH_TEMP_SUFFIX = ".temp.fuse";
static const string PHOTOS_BUNDLE_NAME = "com.ohos.photos";
static const unsigned int OID_USER_DATA_RW = 1008;
static const unsigned int STAT_NLINK_REG = 1;
static const unsigned int STAT_NLINK_DIR = 2;
static const unsigned int STAT_MODE_REG = 0770;
static const unsigned int STAT_MODE_DIR = 0771;
static const unsigned int MAX_READ_SIZE = 4 * 1024 * 1024;
static const unsigned int KEY_FRAME_SIZE = 8192;
static const unsigned int MAX_IDLE_THREADS = 10;
static const unsigned int READ_CACHE_SLEEP = 10 * 1000;
static const unsigned int CACHE_PAGE_NUM = 2;
static const unsigned int HMDFS_IOC = 0xf2;
static const std::chrono::seconds READ_TIMEOUT_S = 20s;
static const std::chrono::seconds OPEN_TIMEOUT_S = 4s;

#define HMDFS_IOC_HAS_CACHE _IOW(HMDFS_IOC, 6, struct HmdfsHasCache)
#define HMDFS_IOC_CANCEL_READ _IO(HMDFS_IOC, 8)
#define HMDFS_IOC_RESET_READ _IO(HMDFS_IOC, 9)
PAGE_FLAG_TYPE PG_READAHEAD = 0x00000001;
PAGE_FLAG_TYPE PG_UPTODATE = 0x00000002;
PAGE_FLAG_TYPE PG_REFERENCED = 0x00000004;
PAGE_FLAG_TYPE PG_NEEDBECLEANED = 0x00000020;

enum CLOUD_READ_STATUS {
    READING = 0,
    READ_FINISHED,
    READ_CANCELED,
};

enum CLOUD_CACHE_STATUS : int {
    NOT_CACHE = 0,
    HAS_CACHED,
};

struct ReadCacheInfo {
    std::mutex mutex;
    std::condition_variable cond{};
    uint32_t flags{0};
};

struct ReadSlice {
    size_t sizeHead{0};
    size_t sizeTail{0};
    off_t offHead{0};
    off_t offTail{0};
};

struct ReadArguments {
    size_t size{0};
    off_t offset{0};
    pid_t pid{0};
    shared_ptr<int64_t> readResult{nullptr};
    shared_ptr<CLOUD_READ_STATUS> readStatus{nullptr};
    shared_ptr<CloudFile::CloudError> ckError{nullptr};
    shared_ptr<ffrt::condition_variable> cond{nullptr};
    shared_ptr<char> buf{nullptr};

    ReadArguments(size_t readSize, off_t readOffset, pid_t readPid) : size(readSize), offset(readOffset), pid(readPid)
    {
        readResult = make_shared<int64_t>(-1);
        readStatus = make_shared<CLOUD_READ_STATUS>(READING);
        ckError = make_shared<CloudFile::CloudError>();
        cond = make_shared<ffrt::condition_variable>();
        if (0 < readSize && readSize <= MAX_READ_SIZE) {
            buf.reset(new char[readSize], [](char *ptr) { delete[] ptr; });
        }
    }
};

struct CloudInode {
    shared_ptr<MetaBase> mBase{nullptr};
    string path;
    fuse_ino_t parent{0};
    atomic<int> refCount{0};
    shared_ptr<CloudFile::CloudAssetReadSession> readSession{nullptr};
    atomic<int> sessionRefCount{0};
    std::shared_mutex sessionLock;
    ffrt::mutex readLock;
    ffrt::mutex openLock;
    std::mutex readArgsLock;
    off_t offset{0xffffffff};
    std::map<int64_t, std::shared_ptr<ReadCacheInfo>> readCacheMap;
    /* variable readArguments for cancel*/
    std::set<shared_ptr<ReadArguments>> readArgsSet;
    /* process's read status for ioctl, true when canceled */
    std::map<pid_t, bool> readCtlMap;
    std::unique_ptr<CLOUD_CACHE_STATUS[]> cacheFileIndex{nullptr};
    bool SetReadCacheFlag(int64_t index, PAGE_FLAG_TYPE flag)
    {
        std::shared_lock lock(sessionLock);
        auto it = readCacheMap.find(index);
        if (it != readCacheMap.end()) {
            std::unique_lock<std::mutex> flock(it->second->mutex);
            it->second->flags |= flag;
            return true;
        }
        return false;
    }
    bool IsReadAhead(int64_t index)
    {
        std::shared_lock lock(sessionLock);
        auto it = readCacheMap.find(index);
        if (it == readCacheMap.end()) {
            return false;
        }
        std::unique_lock<std::mutex> flock(it->second->mutex);
        return it->second->flags & PG_READAHEAD;
    }
};

struct DoCloudReadParams {
    shared_ptr<CloudInode> cInode{nullptr};
    shared_ptr<CloudFile::CloudAssetReadSession> readSession{nullptr};
    shared_ptr<ReadArguments> readArgsHead{nullptr};
    shared_ptr<ReadArguments> readArgsTail{nullptr};
};

struct HmdfsHasCache {
    int64_t offset;
    int64_t readSize;
};

struct FuseData {
    int userId;
    shared_ptr<CloudInode> rootNode{nullptr};
    /* store CloudInode by path */
    map<string, shared_ptr<CloudInode>> inodeCache;
    std::shared_mutex cacheLock;
    shared_ptr<CloudFile::CloudDatabase> database;
    struct fuse_session *se;
};

static void InsertReadArgs(shared_ptr<CloudInode> cInode, vector<shared_ptr<ReadArguments>> readArgsList)
{
    std::unique_lock<std::mutex> lock(cInode->readArgsLock);
    for (auto &it : readArgsList) {
        if (it) {
            cInode->readArgsSet.insert(it);
        }
    }
}

static void EraseReadArgs(shared_ptr<CloudInode> cInode, vector<shared_ptr<ReadArguments>> readArgsList)
{
    std::unique_lock<std::mutex> lock(cInode->readArgsLock);
    for (auto &it : readArgsList) {
        if (it) {
            auto readArgs = cInode->readArgsSet.find(it);
            if (readArgs != cInode->readArgsSet.end()) {
                cInode->readArgsSet.erase(readArgs);
            }
        }
    }
}

static string GetLocalPath(int32_t userId, const string &relativePath)
{
    return HMDFS_PATH_PREFIX + to_string(userId) + LOCAL_PATH_SUFFIX + relativePath;
}

static string GetLocalTmpPath(int32_t userId, const string &relativePath)
{
    return GetLocalPath(userId, relativePath) + PATH_TEMP_SUFFIX;
}

static int HandleCloudError(CloudError error)
{
    int ret = 0;
    switch (error) {
        case CloudError::CK_NO_ERROR:
            ret = 0;
            break;
        case CloudError::CK_NETWORK_ERROR:
            ret = -ENOTCONN;
            break;
        case CloudError::CK_SERVER_ERROR:
            ret = -EIO;
            break;
        case CloudError::CK_LOCAL_ERROR:
            ret = -EINVAL;
            break;
        default:
            ret = -EIO;
            break;
    }
    return ret;
}

static shared_ptr<CloudDatabase> GetDatabase(struct FuseData *data)
{
    if (!data->database) {
        auto instance = CloudFile::CloudFileKit::GetInstance();
        if (instance == nullptr) {
            LOGE("get cloud file helper instance failed");
            return nullptr;
        }

        data->database = instance->GetCloudDatabase(data->userId, PHOTOS_BUNDLE_NAME);
        if (data->database == nullptr) {
            LOGE("get cloud file kit database fail");
            return nullptr;
        }
    }
    return data->database;
}

static shared_ptr<CloudInode> FindNode(struct FuseData *data, string path)
{
    shared_ptr<CloudInode> ret = nullptr;
    std::shared_lock<std::shared_mutex> rLock(data->cacheLock, std::defer_lock);
    rLock.lock();
    if (data->inodeCache.count(path) != 0) {
        ret = data->inodeCache.at(path);
    }
    rLock.unlock();
    return ret;
}

static shared_ptr<CloudInode> GetRootInode(struct FuseData *data, fuse_ino_t ino)
{
    std::unique_lock<std::shared_mutex> wLock(data->cacheLock, std::defer_lock);
    shared_ptr<CloudInode> ret;

    wLock.lock();
    if (!data->rootNode) {
        data->rootNode = make_shared<CloudInode>();
        data->rootNode->path = "/";
        data->rootNode->refCount = 1;
        data->rootNode->mBase = make_shared<MetaBase>();
        data->rootNode->mBase->mode = S_IFDIR;
        data->rootNode->mBase->mtime = static_cast<uint64_t>(GetSecondsSince1970ToNow());
        LOGD("create rootNode");
    }
    ret = data->rootNode;
    wLock.unlock();

    return ret;
}

static shared_ptr<CloudInode> GetCloudInode(struct FuseData *data, fuse_ino_t ino)
{
    if (ino == FUSE_ROOT_ID) {
        return GetRootInode(data, ino);
    } else {
        struct CloudInode *inoPtr = reinterpret_cast<struct CloudInode *>(ino);
        if (inoPtr == nullptr) {
            LOGE("inoPtr is nullptr");
            return nullptr;
        }
        return FindNode(data, inoPtr->path);
    }
}

static string CloudPath(struct FuseData *data, fuse_ino_t ino)
{
    auto inode = GetCloudInode(data, ino);
    if (inode) {
        return inode->path;
    } else {
        LOGE("find node is nullptr");
        return "";
    }
}

static void GetMetaAttr(struct FuseData *data, shared_ptr<CloudInode> ino, struct stat *stbuf)
{
    stbuf->st_ino = reinterpret_cast<fuse_ino_t>(ino.get());
    stbuf->st_uid = OID_USER_DATA_RW;
    stbuf->st_gid = OID_USER_DATA_RW;
    stbuf->st_mtime = static_cast<int64_t>(ino->mBase->mtime);
    if (ino->mBase->mode & S_IFDIR) {
        stbuf->st_mode = S_IFDIR | STAT_MODE_DIR;
        stbuf->st_nlink = STAT_NLINK_DIR;
        LOGD("directory, ino:%s", GetAnonyString(ino->path).c_str());
    } else {
        stbuf->st_mode = S_IFREG | STAT_MODE_REG;
        stbuf->st_nlink = STAT_NLINK_REG;
        stbuf->st_size = static_cast<decltype(stbuf->st_size)>(ino->mBase->size);
        LOGD("regular file, ino:%s, size: %lld", GetAnonyString(ino->path).c_str(), (long long)stbuf->st_size);
    }
}

static int CloudDoLookup(fuse_req_t req, fuse_ino_t parent, const char *name,
                         struct fuse_entry_param *e)
{
    int err = 0;
    shared_ptr<CloudInode> child;
    bool create = false;
    struct FuseData *data = static_cast<struct FuseData *>(fuse_req_userdata(req));
    string parentName = CloudPath(data, parent);
    if (parentName == "") {
        LOGE("parent name is empty");
        return ENOENT;
    }
    string childName = (parent == FUSE_ROOT_ID) ? parentName + name :
                                                  parentName + "/" + name;
    std::unique_lock<std::shared_mutex> wLock(data->cacheLock, std::defer_lock);

    LOGD("parent: %{private}s, name: %s", GetAnonyString(parentName).c_str(), GetAnonyString(name).c_str());

    child = FindNode(data, childName);
    if (!child) {
        child = make_shared<CloudInode>();
        create = true;
        LOGD("new child %s", GetAnonyString(child->path).c_str());
    }
    MetaBase mBase(name);
    err = MetaFile(data->userId, parentName).DoLookup(mBase);
    if (err) {
        LOGE("lookup %s error, err: %{public}d", childName.c_str(), err);
        return err;
    }

    child->refCount++;
    if (create) {
        child->mBase = make_shared<MetaBase>(mBase);
        child->path = childName;
        child->parent = parent;
        wLock.lock();
        data->inodeCache[child->path] = child;
        wLock.unlock();
    } else if (*(child->mBase) != mBase) {
        LOGW("invalidate %s", childName.c_str());
        child->mBase = make_shared<MetaBase>(mBase);
    }
    LOGD("lookup success, child: %{private}s, refCount: %lld", GetAnonyString(child->path).c_str(),
         static_cast<long long>(child->refCount));
    GetMetaAttr(data, child, &e->attr);
    e->ino = reinterpret_cast<fuse_ino_t>(child.get());
    return 0;
}

static void CloudLookup(fuse_req_t req, fuse_ino_t parent,
                        const char *name)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    struct fuse_entry_param e;
    int err;

    err = CloudDoLookup(req, parent, name, &e);
    if (err) {
        fuse_reply_err(req, err);
    } else {
        fuse_reply_entry(req, &e);
    }
}

static void PutNode(struct FuseData *data, shared_ptr<CloudInode> node, uint64_t num)
{
    std::unique_lock<std::shared_mutex> wLock(data->cacheLock, std::defer_lock);
    node->refCount -= num;
    LOGD("%s, put num: %lld,  current refCount: %d",
         GetAnonyString(node->path).c_str(), (long long)num,  node->refCount.load());
    if (node->refCount == 0) {
        LOGD("node released: %s", GetAnonyString(node->path).c_str());
        wLock.lock();
        data->inodeCache.erase(node->path);
        wLock.unlock();
    }
}

static void CloudForget(fuse_req_t req, fuse_ino_t ino,
                        uint64_t nlookup)
{
    struct FuseData *data = static_cast<struct FuseData *>(fuse_req_userdata(req));
    shared_ptr<CloudInode> node = GetCloudInode(data, ino);
    if (node) {
        LOGD("forget %s, nlookup: %lld", GetAnonyString(node->path).c_str(), (long long)nlookup);
        PutNode(data, node, nlookup);
    }
    fuse_reply_none(req);
}

static void CloudGetAttr(fuse_req_t req, fuse_ino_t ino,
                         struct fuse_file_info *fi)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    struct stat buf;
    struct FuseData *data = static_cast<struct FuseData *>(fuse_req_userdata(req));
    (void) fi;

    LOGD("getattr, %s", GetAnonyString(CloudPath(data, ino)).c_str());
    shared_ptr<CloudInode> node = GetCloudInode(data, ino);
    if (!node || !node->mBase) {
        LOGE("Failed to get cloud inode.");
        fuse_reply_err(req, ENOMEM);
        return;
    }
    GetMetaAttr(data, node, &buf);

    fuse_reply_attr(req, &buf, 0);
}

static string GetAssetKey(int fileType)
{
    switch (fileType) {
        case FILE_TYPE_CONTENT:
            return "content";
        case FILE_TYPE_THUMBNAIL:
            return "thumbnail";
        case FILE_TYPE_LCD:
            return "lcd";
        default:
            LOGE("bad fileType %{public}d", fileType);
            return "";
    }
}

static string GetCloudMergeViewPath(int32_t userId, const string &relativePath)
{
    return HMDFS_PATH_PREFIX + to_string(userId) + CLOUD_MERGE_VIEW_PATH_SUFFIX + relativePath;
}

static string GetAssetPath(shared_ptr<CloudInode> cInode, struct FuseData *data)
{
    string path;
    filesystem::path parentPath;
    path = GetLocalTmpPath(data->userId, cInode->path);
    parentPath = filesystem::path(path).parent_path();
    ForceCreateDirectory(parentPath.string());
    LOGD("fileType: %d, create dir: %s, relative path: %s",
         cInode->mBase->fileType, GetAnonyString(parentPath.string()).c_str(), GetAnonyString(cInode->path).c_str());
    return path;
}

static void fuse_inval(fuse_session *se, fuse_ino_t parentIno, fuse_ino_t childIno, const string &childName)
{
    auto task = [se, parentIno, childIno, childName] {
        if (fuse_lowlevel_notify_inval_entry(se, parentIno, childName.c_str(), childName.size())) {
            fuse_lowlevel_notify_inval_inode(se, childIno, 0, 0);
        }
    };
    ffrt::submit(task, {}, {}, ffrt::task_attr().qos(ffrt_qos_background));
}

static int CloudOpenOnLocal(struct FuseData *data, shared_ptr<CloudInode> cInode, struct fuse_file_info *fi)
{
    string localPath = GetLocalPath(data->userId, cInode->path);
    string tmpPath = GetLocalTmpPath(data->userId, cInode->path);
    char resolvedPath[PATH_MAX + 1] = {'\0'};
    char *realPath = realpath(tmpPath.c_str(), resolvedPath);
    if (realPath == nullptr) {
        LOGE("Failed to realpath, errno: %{public}d", errno);
        return 0;
    }
    unsigned int flags = static_cast<unsigned int>(fi->flags);
    if (flags & O_DIRECT) {
        flags &= ~O_DIRECT;
    }
    auto fd = open(realPath, flags);
    if (fd < 0) {
        LOGE("Failed to open local file, errno: %{public}d", errno);
        return 0;
    }
    string cloudMergeViewPath = GetCloudMergeViewPath(data->userId, cInode->path);
    if (remove(cloudMergeViewPath.c_str()) < 0) {
        LOGE("Failed to update kernel dentry cache, errno: %{public}d", errno);
        close(fd);
        return 0;
    }

    filesystem::path parentPath = filesystem::path(localPath).parent_path();
    ForceCreateDirectory(parentPath.string());
    if (rename(tmpPath.c_str(), localPath.c_str()) < 0) {
        LOGE("Failed to rename tmpPath to localPath, errno: %{public}d", errno);
        close(fd);
        return -errno;
    }
    auto parentInode = GetCloudInode(data, cInode->parent);
    if (parentInode == nullptr) {
        LOGE("fail to find parent inode");
        close(fd);
        return -ENOMEM;
    }
    MetaFile(data->userId, parentInode->path).DoRemove(*(cInode->mBase));
    cInode->mBase->hasDownloaded = true;
    fi->fh = static_cast<uint64_t>(fd);
    return 0;
}

static int HandleOpenResult(CloudFile::CloudError ckError, struct FuseData *data,
    shared_ptr<CloudInode> cInode, struct fuse_file_info *fi)
{
    auto ret = HandleCloudError(ckError);
    if (ret < 0) {
        LOGE("open fali");
        return ret;
    }
    if (cInode->mBase->fileType != FILE_TYPE_CONTENT) {
        ret = CloudOpenOnLocal(data, cInode, fi);
        if (ret < 0) {
            return ret;
        }
    }
    cInode->sessionRefCount++;
    LOGI("open success, sessionRefCount: %{public}d", cInode->sessionRefCount.load());
    return 0;
}

static uint64_t UTCTimeMilliSeconds()
{
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    return t.tv_sec * CloudDisk::SECOND_TO_MILLISECOND + t.tv_nsec / CloudDisk::MILLISECOND_TO_NANOSECOND;
}

static void DownloadThmOrLcd(shared_ptr<CloudInode> cInode, shared_ptr<CloudError> err, shared_ptr<bool> openFinish,
    shared_ptr<ffrt::condition_variable> cond)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    auto session = cInode->readSession;
    if (!session) {
        LOGE("readSession is nullptr");
        return;
    }
    *err = session->InitSession();
    {
        unique_lock lck(cInode->openLock);
        *openFinish = true;
    }
    cond->notify_one();
    LOGI("download done, path: %{public}s", GetAnonyString(cInode->path).c_str());
    return;
}

static bool IsVideoType(const string &name)
{
    return name.find(VIDEO_TYPE_PREFIX) == 0;
}

static void LoadCacheFileIndex(shared_ptr<CloudInode> cInode, int32_t userId)
{
    string cachePath = LOCAL_PATH_DATA_SERVICE_EL2 + to_string(userId) + LOCAL_PATH_HMDFS_CLOUD_CACHE +
                       CLOUD_CACHE_DIR + cInode->path;
    int filePageSize = static_cast<int32_t>(cInode->mBase->size / MAX_READ_SIZE + 1);
    CLOUD_CACHE_STATUS *tmp = new CLOUD_CACHE_STATUS[filePageSize]();
    std::unique_ptr<CLOUD_CACHE_STATUS[]> mp(tmp);
    if (access(cachePath.c_str(), F_OK) != 0) {
        string parentPath = filesystem::path(cachePath).parent_path().string();
        if (!ForceCreateDirectory(parentPath)) {
            LOGE("failed to create parent dir");
            return;
        }
        int fd = open(cachePath.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
        if (fd < 0) {
            LOGE("failed to open cache file, ret: %{public}d", errno);
            return;
        }
        if (ftruncate(fd, cInode->mBase->size) == -1) {
            LOGE("failed to truncate file, ret: %{public}d", errno);
        }
        close(fd);
        cInode->cacheFileIndex = std::move(mp);
        return;
    }

    if (getxattr(cachePath.c_str(), CLOUD_CACHE_XATTR_NAME.c_str(), mp.get(),
                 sizeof(CLOUD_CACHE_STATUS[filePageSize])) < 0 &&
        errno != ENODATA) {
        LOGE("getxattr fail, err: %{public}d", errno);
        cInode->cacheFileIndex = std::move(mp);
        return;
    }
    for (int i = 0; i < filePageSize; i++) {
        if (mp.get()[i] == HAS_CACHED) {
            auto memInfo = std::make_shared<ReadCacheInfo>();
            memInfo->flags = PG_UPTODATE;
            cInode->readCacheMap[i] = memInfo;
        }
    }
    cInode->cacheFileIndex = std::move(mp);
}

static int DoCloudOpen(shared_ptr<CloudInode> cInode, struct fuse_file_info *fi, struct FuseData *data)
{
    auto error = make_shared<CloudError>();
    auto openFinish = make_shared<bool>(false);
    auto cond = make_shared<ffrt::condition_variable>();
    ffrt::submit([cInode, error, openFinish, cond, data] {
        if (IsVideoType(cInode->mBase->name)) {
            LoadCacheFileIndex(cInode, data->userId);
        }
        DownloadThmOrLcd(cInode, error, openFinish, cond);
    });
    unique_lock lck(cInode->openLock);
    auto waitStatus = cond->wait_for(lck, OPEN_TIMEOUT_S, [openFinish] {
        return *openFinish;
    });
    if (!waitStatus) {
        LOGE("download %{public}s timeout", GetAnonyString(cInode->path).c_str());
        return -ENETUNREACH;
    }
    return HandleOpenResult(*error, data, cInode, fi);
}

static void UpdateReadStat(shared_ptr<CloudInode> cInode, uint64_t startTime)
{
    uint64_t endTime = UTCTimeMilliSeconds();
    CloudDaemonStatistic &readStat = CloudDaemonStatistic::GetInstance();
    readStat.UpdateOpenSizeStat(cInode->mBase->size);
    readStat.UpdateOpenTimeStat(cInode->mBase->fileType, (endTime > startTime) ? (endTime - startTime) : 0);
}

static void CloudOpen(fuse_req_t req, fuse_ino_t ino,
                      struct fuse_file_info *fi)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    struct FuseData *data = static_cast<struct FuseData *>(fuse_req_userdata(req));
    fi->fh = UINT64_MAX;
    shared_ptr<CloudInode> cInode = GetCloudInode(data, ino);
    if (!cInode) {
        fuse_reply_err(req, ENOMEM);
        return;
    }
    string recordId = MetaFileMgr::GetInstance().CloudIdToRecordId(cInode->mBase->cloudId);
    shared_ptr<CloudFile::CloudDatabase> database = GetDatabase(data);
    std::unique_lock<std::shared_mutex> wSesLock(cInode->sessionLock, std::defer_lock);

    LOGI("%{public}d open %{public}s", req->ctx.pid, GetAnonyString(CloudPath(data, ino)).c_str());
    if (!database) {
        LOGE("database is null");
        fuse_inval(data->se, cInode->parent, ino, cInode->mBase->name);
        fuse_reply_err(req, EPERM);
        return;
    }
    wSesLock.lock();
    if (!cInode->readSession) {
        /*
         * 'recordType' is fixed to "media" now
         * 'assetKey' is one of "content"/"lcd"/"thumbnail"
         */
        LOGD("recordId: %s", recordId.c_str());
        uint64_t startTime = UTCTimeMilliSeconds();
        cInode->readSession = database->NewAssetReadSession("media", recordId, GetAssetKey(cInode->mBase->fileType),
                                                            GetAssetPath(cInode, data));
        if (cInode->readSession) {
            auto ret = DoCloudOpen(cInode, fi, data);
            if (ret == 0) {
                fuse_reply_open(req, fi);
            } else {
                fuse_inval(data->se, cInode->parent, ino, cInode->mBase->name);
                fuse_reply_err(req, -ret);
                cInode->readSession = nullptr;
            }
            UpdateReadStat(cInode, startTime);
            wSesLock.unlock();
            return;
        }
    }
    if (!cInode->readSession) {
        LOGE("readSession is null or fix size fail");
        fuse_inval(data->se, cInode->parent, ino, cInode->mBase->name);
        fuse_reply_err(req, EPERM);
    } else {
        cInode->sessionRefCount++;
        LOGI("open success, sessionRefCount: %{public}d", cInode->sessionRefCount.load());
        fuse_reply_open(req, fi);
    }
    wSesLock.unlock();
}

static void CloudRelease(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi)
{
    struct FuseData *data = static_cast<struct FuseData *>(fuse_req_userdata(req));
    shared_ptr<CloudInode> cInode = GetCloudInode(data, ino);
    if (!cInode) {
        fuse_reply_err(req, ENOMEM);
        return;
    }
    std::unique_lock<std::shared_mutex> wSesLock(cInode->sessionLock, std::defer_lock);
    LOGI("%{public}d release %{public}s, sessionRefCount: %{public}d", req->ctx.pid,
         GetAnonyString(cInode->path).c_str(), cInode->sessionRefCount.load());
    wSesLock.lock();
    cInode->sessionRefCount--;
    if (cInode->sessionRefCount == 0) {
        if (fi->fh != UINT64_MAX) {
            close(fi->fh);
        }
        if (cInode->mBase->fileType == FILE_TYPE_CONTENT && (!cInode->readSession->Close(false))) {
            LOGE("Failed to close readSession");
        }
        cInode->readSession = nullptr;
        cInode->readCacheMap.clear();
        {
            std::unique_lock<std::mutex> lock(cInode->readArgsLock);
            cInode->readCtlMap.clear();
        }
        if (cInode->cacheFileIndex) {
            string cachePath = LOCAL_PATH_DATA_SERVICE_EL2 + to_string(data->userId) + LOCAL_PATH_HMDFS_CLOUD_CACHE +
                               CLOUD_CACHE_DIR + cInode->path;
            if (setxattr(cachePath.c_str(), CLOUD_CACHE_XATTR_NAME.c_str(), cInode->cacheFileIndex.get(),
                         sizeof(int[cInode->mBase->size / MAX_READ_SIZE + 1]), 0) < 0) {
                LOGE("setxattr fail, err: %{public}d", errno);
            }
        }
        LOGD("readSession released");
    }
    wSesLock.unlock();

    LOGI("release end");
    fuse_inval(data->se, cInode->parent, ino, cInode->mBase->name);
    fuse_reply_err(req, 0);
}

static void CloudReadDir(fuse_req_t req, fuse_ino_t ino, size_t size,
                         off_t off, struct fuse_file_info *fi)
{
    struct FuseData *data = static_cast<struct FuseData *>(fuse_req_userdata(req));
    LOGE("readdir %s, not support", GetAnonyString(CloudPath(data, ino)).c_str());
    fuse_reply_err(req, ENOENT);
}

static void CloudForgetMulti(fuse_req_t req, size_t count,
				struct fuse_forget_data *forgets)
{
    struct FuseData *data = static_cast<struct FuseData *>(fuse_req_userdata(req));
    LOGD("forget_multi");
    for (size_t i = 0; i < count; i++) {
        shared_ptr<CloudInode> node = GetCloudInode(data, forgets[i].ino);
        if (!node) {
            continue;
        }
        LOGD("forget (i=%zu) %s, nlookup: %lld", i, GetAnonyString(node->path).c_str(), (long long)forgets[i].nlookup);
        PutNode(data, node, forgets[i].nlookup);
    }
    fuse_reply_none(req);
}

static void HasCache(fuse_req_t req, fuse_ino_t ino, const void *inBuf)
{
    struct FuseData *data = static_cast<struct FuseData *>(fuse_req_userdata(req));
    shared_ptr<CloudInode> cInode = GetCloudInode(data, ino);
    if (!cInode || !cInode->readSession) {
        fuse_reply_err(req, EPERM);
        return;
    }

    const struct HmdfsHasCache *ioctlData = reinterpret_cast<const struct HmdfsHasCache *>(inBuf);
    if (!ioctlData) {
        fuse_reply_err(req, EINVAL);
        return;
    }
    if (cInode->readSession->HasCache(ioctlData->offset, ioctlData->readSize)) {
        fuse_reply_ioctl(req, 0, NULL, 0);
    } else {
        fuse_reply_err(req, EIO);
    }
}

static bool CheckReadIsCanceled(pid_t pid, shared_ptr<CloudInode> cInode)
{
    std::unique_lock<std::mutex> lock(cInode->readArgsLock);
    return cInode->readCtlMap[pid];
}

static void CancelRead(fuse_req_t req, fuse_ino_t ino)
{
    struct FuseData *data = static_cast<struct FuseData *>(fuse_req_userdata(req));
    shared_ptr<CloudInode> cInode = GetCloudInode(data, ino);
    if (!cInode) {
        fuse_reply_err(req, ENOMEM);
        return;
    }
    LOGI("Cancel read for pid: %{public}d", req->ctx.pid);
    {
        std::unique_lock<std::mutex> lock(cInode->readArgsLock);
        cInode->readCtlMap[req->ctx.pid] = true;
        for (const auto &it : cInode->readArgsSet) {
            if (it->pid == req->ctx.pid) {
                {
                    std::unique_lock lck(cInode->readLock);
                    *it->readStatus = READ_CANCELED;
                }
                it->cond->notify_one();
            }
        }
    }
    std::unique_lock<std::shared_mutex> wSesLock(cInode->sessionLock, std::defer_lock);
    wSesLock.lock();
    for (auto &it : cInode->readCacheMap) {
        std::unique_lock<std::mutex> lock(it.second->mutex);
        if (!(it.second->flags & PG_UPTODATE)) {
            it.second->cond.notify_all();
        }
    }
    wSesLock.unlock();

    fuse_reply_ioctl(req, 0, NULL, 0);
}

static void ResetRead(fuse_req_t req, fuse_ino_t ino)
{
    struct FuseData *data = static_cast<struct FuseData *>(fuse_req_userdata(req));
    shared_ptr<CloudInode> cInode = GetCloudInode(data, ino);
    if (!cInode) {
        fuse_reply_err(req, ENOMEM);
        return;
    }
    LOGI("Reset read for pid: %{public}d", req->ctx.pid);
    {
        std::unique_lock<std::mutex> lock(cInode->readArgsLock);
        cInode->readCtlMap[req->ctx.pid] = false;
    }
    fuse_reply_ioctl(req, 0, NULL, 0);
}

static void CloudIoctl(fuse_req_t req, fuse_ino_t ino, int cmd, void *arg, struct fuse_file_info *fi,
                       unsigned flags, const void *inBuf, size_t inBufsz, size_t outBufsz)
{
    switch (static_cast<unsigned int>(cmd)) {
        case HMDFS_IOC_HAS_CACHE:
            HasCache(req, ino, inBuf);
            break;
        case HMDFS_IOC_CANCEL_READ:
            CancelRead(req, ino);
            break;
        case HMDFS_IOC_RESET_READ:
            ResetRead(req, ino);
            break;
        default:
            fuse_reply_err(req, ENOTTY);
    }
}

static bool CheckAndWait(pid_t pid, shared_ptr<CloudInode> cInode, off_t off)
{
    int64_t cacheIndex = off / MAX_READ_SIZE;
    if (cInode->IsReadAhead(cacheIndex)) {
        auto &it = cInode->readCacheMap[cacheIndex];
        std::unique_lock<std::mutex> lock(it->mutex);
        auto waitStatus = it->cond.wait_for(
            lock, READ_TIMEOUT_S, [&] { return (it->flags & PG_UPTODATE) || CheckReadIsCanceled(pid, cInode); });
        if (!waitStatus) {
            LOGE("CheckAndWait timeout: %{public}ld", static_cast<long>(cacheIndex));
            return false;
        }
    }
    /* true when read finish and not canceled */
    return !CheckReadIsCanceled(pid, cInode);
}

static void SaveCacheToFile(shared_ptr<ReadArguments> readArgs,
                            shared_ptr<CloudInode> cInode,
                            int64_t cacheIndex,
                            int32_t userId)
{
    string cachePath =
        LOCAL_PATH_DATA_SERVICE_EL2 + to_string(userId) + LOCAL_PATH_HMDFS_CLOUD_CACHE + CLOUD_CACHE_DIR + cInode->path;
    char *realPaths = realpath(cachePath.c_str(), nullptr);
    if (realPaths == nullptr) {
        LOGE("realpath failed");
        return;
    }
    int fd = open(realPaths, O_RDWR);
    free(realPaths);
    if (fd < 0) {
        LOGE("Failed to open cache file, err: %{public}d", errno);
        return;
    }
    if (cInode->cacheFileIndex.get()[cacheIndex] == NOT_CACHE &&
        pwrite(fd, readArgs->buf.get(), *readArgs->readResult, readArgs->offset) == *readArgs->readResult) {
        LOGI("Write to cache file, offset: %{public}ld*4M ", static_cast<long>(cacheIndex));
        cInode->cacheFileIndex.get()[cacheIndex] = HAS_CACHED;
    }
    close(fd);
}

static void CloudReadOnCloudFile(pid_t pid,
                                 int32_t userId,
                                 shared_ptr<ReadArguments> readArgs,
                                 shared_ptr<CloudInode> cInode,
                                 shared_ptr<CloudFile::CloudAssetReadSession> readSession)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    LOGI("PRead CloudFile, path: %{public}s, size: %{public}zd, off: %{public}lu", GetAnonyString(cInode->path).c_str(),
         readArgs->size, static_cast<unsigned long>(readArgs->offset));

    uint64_t startTime = UTCTimeMilliSeconds();
    bool isReading = true;
    int64_t cacheIndex = readArgs->offset / MAX_READ_SIZE;
    std::unique_lock<std::shared_mutex> wSesLock(cInode->sessionLock, std::defer_lock);

    wSesLock.lock();
    if (readArgs->offset % MAX_READ_SIZE == 0 && cInode->readCacheMap.find(cacheIndex) == cInode->readCacheMap.end()) {
        auto memInfo = std::make_shared<ReadCacheInfo>();
        memInfo->flags = PG_READAHEAD;
        cInode->readCacheMap[cacheIndex] = memInfo;
        isReading = false;
        LOGI("To do read cloudfile, offset: %{public}ld*4M", static_cast<long>(cacheIndex));
    }
    wSesLock.unlock();
    if (isReading && !CheckAndWait(pid, cInode, readArgs->offset)) {
        return;
    }

    *readArgs->readResult =
        readSession->PRead(readArgs->offset, readArgs->size, readArgs->buf.get(), *readArgs->ckError);

    uint64_t endTime = UTCTimeMilliSeconds();
    uint64_t readTime = (endTime > startTime) ? (endTime - startTime) : 0;
    CloudDaemonStatistic &readStat = CloudDaemonStatistic::GetInstance();
    readStat.UpdateReadSizeStat(readArgs->size);
    readStat.UpdateReadTimeStat(readArgs->size, readTime);

    {
        unique_lock lck(cInode->readLock);
        *readArgs->readStatus = READ_FINISHED;
    }
    readArgs->cond->notify_one();
    if (readArgs->offset % MAX_READ_SIZE == 0) {
        cInode->SetReadCacheFlag(cacheIndex, PG_UPTODATE);
        wSesLock.lock();
        if (cInode->readCacheMap.find(cacheIndex) != cInode->readCacheMap.end()) {
            cInode->readCacheMap[cacheIndex]->cond.notify_all();
            LOGI("Read cloudfile done and notify all waiting threads, offset: %{public}ld*4M",
                static_cast<long>(cacheIndex));
        }
        if (IsVideoType(cInode->mBase->name) && *readArgs->readResult > 0) {
            ffrt::submit(
                [userId, readArgs, cInode, cacheIndex] { SaveCacheToFile(readArgs, cInode, cacheIndex, userId); });
        }
        wSesLock.unlock();
    }
    return;
}

static void CloudReadOnCacheFile(shared_ptr<ReadArguments> readArgs,
                                 shared_ptr<CloudInode> cInode,
                                 shared_ptr<CloudFile::CloudAssetReadSession> readSession,
                                 int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    usleep(READ_CACHE_SLEEP);
    LOGI("Preread CloudFile, path: %{public}s, size: %{public}zd, off: %{public}lu",
        GetAnonyString(cInode->path).c_str(), readArgs->size, static_cast<unsigned long>(readArgs->offset));
    uint64_t startTime = UTCTimeMilliSeconds();
    int64_t cacheIndex = readArgs->offset / MAX_READ_SIZE;
    std::unique_lock<std::shared_mutex> wSesLock(cInode->sessionLock, std::defer_lock);

    wSesLock.lock();
    if (cInode->readCacheMap.find(cacheIndex) != cInode->readCacheMap.end()) {
        wSesLock.unlock();
        return;
    }
    auto memInfo = std::make_shared<ReadCacheInfo>();
    memInfo->flags = PG_READAHEAD;
    cInode->readCacheMap[cacheIndex] = memInfo;
    LOGI("To do preread cloudfile, offset: %{public}ld*4M", static_cast<long>(cacheIndex));
    wSesLock.unlock();

    readArgs->offset = cacheIndex * MAX_READ_SIZE;
    readArgs->buf.reset(new char[MAX_READ_SIZE], [](char *ptr) { delete[] ptr; });
    *readArgs->readResult =
        readSession->PRead(readArgs->offset, readArgs->size, readArgs->buf.get(), *readArgs->ckError);

    uint64_t endTime = UTCTimeMilliSeconds();
    uint64_t readTime = (endTime > startTime) ? (endTime - startTime) : 0;
    CloudDaemonStatistic &readStat = CloudDaemonStatistic::GetInstance();
    readStat.UpdateReadSizeStat(readArgs->size);
    readStat.UpdateReadTimeStat(readArgs->size, readTime);

    cInode->SetReadCacheFlag(cacheIndex, PG_UPTODATE);
    wSesLock.lock();
    if (cInode->readCacheMap.find(cacheIndex) != cInode->readCacheMap.end()) {
        cInode->readCacheMap[cacheIndex]->cond.notify_all();
        LOGI("Preread cloudfile done and notify all waiting threads, offset: %{public}ld*4M",
            static_cast<long>(cacheIndex));
    }
    if (IsVideoType(cInode->mBase->name) && *readArgs->readResult > 0) {
        ffrt::submit([readArgs, cInode, cacheIndex, userId] { SaveCacheToFile(readArgs, cInode, cacheIndex, userId); });
    }
    wSesLock.unlock();
    return;
}

static void CloudReadOnLocalFile(fuse_req_t req,  shared_ptr<char> buf, size_t size,
    off_t off, struct fuse_file_info *fi)
{
    auto readSize = pread(fi->fh, buf.get(), size, off);
    if (readSize < 0) {
        LOGE("Failed to read local file, errno: %{public}d", errno);
        fuse_reply_err(req, errno);
        return;
    }
    fuse_reply_buf(req, buf.get(), min(size, static_cast<size_t>(readSize)));
    return;
}

static void InitReadSlice(size_t size, off_t off, ReadSlice &readSlice)
{
    readSlice.offHead = off;
    readSlice.offTail = (off + static_cast<off_t>(size)) / MAX_READ_SIZE * MAX_READ_SIZE;
    if (readSlice.offTail > readSlice.offHead) {
        readSlice.sizeHead = static_cast<size_t>(readSlice.offTail - readSlice.offHead);
        readSlice.sizeTail = MAX_READ_SIZE;
    } else {
        readSlice.sizeHead = static_cast<size_t>(readSlice.offTail + MAX_READ_SIZE - readSlice.offHead);
    }
    if ((static_cast<off_t>(size) + off) % MAX_READ_SIZE == 0) {
        readSlice.offTail -= MAX_READ_SIZE;
        readSlice.sizeTail = 0;
    }
}

static bool FixData(fuse_req_t req, shared_ptr<char> buf, size_t size, size_t sizeDone,
    shared_ptr<ReadArguments> readArgs)
{
    if (*readArgs->readResult < 0) {
        LOGE("Pread failed, readResult: %{public}ld", static_cast<long>(*readArgs->readResult));
        fuse_reply_err(req, ENOMEM);
        return false;
    }

    auto ret = HandleCloudError(*readArgs->ckError);
    if (ret < 0) {
        LOGE("Pread failed, clouderror: %{public}d", -ret);
        fuse_reply_err(req, -ret);
        return false;
    }

    int32_t copyRet = memcpy_s(buf.get() + sizeDone, min(size - sizeDone, static_cast<size_t>(*readArgs->readResult)),
                               readArgs->buf.get(), min(size - sizeDone, static_cast<size_t>(*readArgs->readResult)));
    if (copyRet != 0) {
        LOGE("Parcel data copy failed, err=%{public}d", copyRet);
        fuse_reply_err(req, ENETUNREACH);
        return false;
    }
    return true;
}

static bool WaitData(fuse_req_t req, shared_ptr<CloudInode> cInode, shared_ptr<ReadArguments> readArgs)
{
    std::unique_lock lock(cInode->readLock);
    auto waitStatus = readArgs->cond->wait_for(lock, READ_TIMEOUT_S, [readArgs] { return *readArgs->readStatus; });
    if (*readArgs->readStatus == READ_CANCELED) {
        LOGI("read is cancelled");
        fuse_reply_err(req, EIO);
        return false;
    }
    if (!waitStatus) {
        std::unique_lock<std::shared_mutex> wSesLock(cInode->sessionLock, std::defer_lock);
        int64_t cacheIndex = readArgs->offset / MAX_READ_SIZE;
        cInode->SetReadCacheFlag(cacheIndex, PG_UPTODATE);
        wSesLock.lock();
        if (cInode->readCacheMap.find(cacheIndex) != cInode->readCacheMap.end()) {
            cInode->readCacheMap[cacheIndex]->cond.notify_all();
        }
        wSesLock.unlock();
        LOGE("Pread timeout, offset: %{public}ld*4M", static_cast<long>(cacheIndex));
        fuse_reply_err(req, ENETUNREACH);
        return false;
    }
    return true;
}

static void WaitAndFixData(fuse_req_t req,
                           shared_ptr<char> buf,
                           size_t size,
                           shared_ptr<CloudInode> cInode,
                           vector<shared_ptr<ReadArguments>> readArgsList)
{
    size_t sizeDone = 0;
    for (auto &it : readArgsList) {
        if (!it) {
            continue;
        }
        if (!WaitData(req, cInode, it)) {
            return;
        }
        if (!FixData(req, buf, size, sizeDone, it)) {
            return;
        }
        sizeDone += it->size;
    }
    fuse_reply_buf(req, buf.get(), min(size, sizeDone));
}

static ssize_t ReadCacheFile(shared_ptr<ReadArguments> readArgs, const string &path, int32_t userId)
{
    string cachePath =
        LOCAL_PATH_DATA_SERVICE_EL2 + to_string(userId) + LOCAL_PATH_HMDFS_CLOUD_CACHE + CLOUD_CACHE_DIR + path;
    char *realPaths = realpath(cachePath.c_str(), nullptr);
    if (realPaths == nullptr) {
        LOGE("realpath failed");
        return -1;
    }
    int fd = open(realPaths, O_RDONLY);
    free(realPaths);
    if (fd < 0) {
        return fd;
    }
    ssize_t bytesRead = pread(fd, readArgs->buf.get(), readArgs->size, readArgs->offset);
    close(fd);
    return bytesRead;
}

static bool DoReadSlice(fuse_req_t req,
                        shared_ptr<CloudInode> cInode,
                        shared_ptr<CloudFile::CloudAssetReadSession> readSession,
                        shared_ptr<ReadArguments> readArgs,
                        bool needCheck)
{
    if (!readArgs) {
        return true;
    }
    if (!readArgs->buf) {
        fuse_reply_err(req, ENOMEM);
        LOGE("buffer is null");
        return false;
    }
    int64_t cacheIndex = readArgs->offset / MAX_READ_SIZE;
    struct FuseData *data = static_cast<struct FuseData *>(fuse_req_userdata(req));
    if (IsVideoType(cInode->mBase->name) && cInode->cacheFileIndex.get()[cacheIndex] == HAS_CACHED) {
        LOGI("DoReadSlice from local: %{public}ld", static_cast<long>(cacheIndex));
        *readArgs->readResult = ReadCacheFile(readArgs, cInode->path, data->userId);
        if (*readArgs->readResult >= 0) {
            unique_lock lock(cInode->readLock);
            *readArgs->readStatus = READ_FINISHED;
            return true;
        } else {
            LOGI("read cache file failed, errno: %{public}d", errno);
        }
    }

    LOGI("DoReadSlice from cloud: %{public}ld", static_cast<long>(cacheIndex));
    if (needCheck && !CheckAndWait(req->ctx.pid, cInode, readArgs->offset)) {
        fuse_reply_err(req, ENETUNREACH);
        return false;
    }
    ffrt::submit([req, readArgs, cInode, readSession, data] {
        CloudReadOnCloudFile(req->ctx.pid, data->userId, readArgs, cInode, readSession);
    });
    return true;
}

static bool DoCloudRead(fuse_req_t req, int flags, DoCloudReadParams params)
{
    if (!DoReadSlice(req, params.cInode, params.readSession, params.readArgsHead, true)) {
        return false;
    }
    if (!DoReadSlice(req, params.cInode, params.readSession, params.readArgsTail, false)) {
        return false;
    }
    // no prefetch when contains O_NOFOLLOW
    unsigned int unflags = static_cast<unsigned int>(flags);
    if (unflags & O_NOFOLLOW) {
        return true;
    }

    struct FuseData *data = static_cast<struct FuseData *>(fuse_req_userdata(req));
    for (uint32_t i = 1; i <= CACHE_PAGE_NUM; i++) {
        int64_t cacheIndex = static_cast<int64_t>(
            (params.readArgsTail ? params.readArgsTail->offset : params.readArgsHead->offset) / MAX_READ_SIZE + i);
        if (IsVideoType(params.cInode->mBase->name) && params.cInode->cacheFileIndex.get()[cacheIndex] == HAS_CACHED) {
            continue;
        }
        auto readArgsCache = make_shared<ReadArguments>(0, cacheIndex * MAX_READ_SIZE, req->ctx.pid);
        if (!readArgsCache) {
            LOGE("Init readArgsCache failed");
            break;
        }
        ffrt::submit([readArgsCache, params, data] {
            CloudReadOnCacheFile(readArgsCache, params.cInode, params.readSession, data->userId);
        });
    }
    return true;
}

static void CloudRead(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off,
                      struct fuse_file_info *fi)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    shared_ptr<char> buf = nullptr;
    struct FuseData *data = static_cast<struct FuseData *>(fuse_req_userdata(req));
    shared_ptr<CloudInode> cInode = GetCloudInode(data, ino);
    if (!cInode) {
        fuse_reply_err(req, ENOMEM);
        return;
    }
    LOGI("CloudRead: %{public}s, size=%{public}zd, off=%{public}lu",
         GetAnonyString(CloudPath(data, ino)).c_str(), size, (unsigned long)off);

    if (CheckReadIsCanceled(req->ctx.pid, cInode)) {
        LOGI("read is cancelled");
        fuse_reply_err(req, EIO);
        return;
    }
    if (size > MAX_READ_SIZE) {
        LOGE("input size exceeds MAX_READ_SIZE");
        fuse_reply_err(req, EINVAL);
        return;
    }
    auto dkReadSession = cInode->readSession;
    if (!dkReadSession) {
        LOGE("readSession is nullptr");
        fuse_reply_err(req, EPERM);
        return;
    }
    buf.reset(new char[size], [](char *ptr) { delete[] ptr; });
    if (!buf) {
        fuse_reply_err(req, ENOMEM);
        LOGE("buffer is null");
        return;
    }
    if (cInode->mBase->hasDownloaded) {
        CloudReadOnLocalFile(req, buf, size, off, fi);
        return;
    }

    ReadSlice readSlice;
    /* slice read request into 4M pages */
    InitReadSlice(size, off, readSlice);
    /* init readArgs for current page, and next page if cross pages */
    auto readArgsHead = make_shared<ReadArguments>(readSlice.sizeHead, readSlice.offHead, req->ctx.pid);
    shared_ptr<ReadArguments> readArgsTail = nullptr;
    if (readSlice.sizeTail > 0) {
        readArgsTail = make_shared<ReadArguments>(readSlice.sizeTail, readSlice.offTail, req->ctx.pid);
    }
    if (!DoCloudRead(req, fi->flags, {cInode, dkReadSession, readArgsHead, readArgsTail})) {
        return;
    }
    /* wait and reply current req data */
    InsertReadArgs(cInode, {readArgsHead, readArgsTail});
    WaitAndFixData(req, buf, size, cInode, {readArgsHead, readArgsTail});
    EraseReadArgs(cInode, {readArgsHead, readArgsTail});
}

static const struct fuse_lowlevel_ops cloudDiskFuseOps = {
    .lookup             = CloudDisk::FuseOperations::Lookup,
    .forget             = CloudDisk::FuseOperations::Forget,
    .getattr            = CloudDisk::FuseOperations::GetAttr,
    .setattr            = CloudDisk::FuseOperations::SetAttr,
    .mknod              = CloudDisk::FuseOperations::MkNod,
    .mkdir              = CloudDisk::FuseOperations::MkDir,
    .unlink             = CloudDisk::FuseOperations::Unlink,
    .rmdir              = CloudDisk::FuseOperations::RmDir,
    .rename             = CloudDisk::FuseOperations::Rename,
    .open               = CloudDisk::FuseOperations::Open,
    .read               = CloudDisk::FuseOperations::Read,
    .release            = CloudDisk::FuseOperations::Release,
    .readdir            = CloudDisk::FuseOperations::ReadDir,
    .setxattr           = CloudDisk::FuseOperations::SetXattr,
    .getxattr           = CloudDisk::FuseOperations::GetXattr,
    .access             = CloudDisk::FuseOperations::Access,
    .create             = CloudDisk::FuseOperations::Create,
    .write_buf          = CloudDisk::FuseOperations::WriteBuf,
    .forget_multi       = CloudDisk::FuseOperations::ForgetMulti,
    .lseek              = CloudDisk::FuseOperations::Lseek,
};

static const struct fuse_lowlevel_ops cloudMediaFuseOps = {
    .lookup             = CloudLookup,
    .forget             = CloudForget,
    .getattr            = CloudGetAttr,
    .open               = CloudOpen,
    .read               = CloudRead,
    .release            = CloudRelease,
    .readdir            = CloudReadDir,
    .ioctl              = CloudIoctl,
    .forget_multi       = CloudForgetMulti,
};

static bool CheckPathForStartFuse(const string &path)
{
    char resolvedPath[PATH_MAX] = {'\0'};
    char* ret = realpath(path.c_str(), resolvedPath);
    if (ret == nullptr) {
        return false;
    }
    std::string realPath(resolvedPath);
    std::string pathPrefix = "/mnt/data/";
    if (realPath.rfind(pathPrefix, 0) != 0) {
        return false;
    }

    size_t userIdBeginPos = pathPrefix.length();
    size_t userIdEndPos = realPath.find("/", userIdBeginPos);
    const std::string userId = realPath.substr(userIdBeginPos, userIdEndPos - userIdBeginPos);
    if (userId.find_first_not_of("0123456789") != std::string::npos) {
        return false;
    }

    size_t suffixBeginPos = userIdEndPos + 1;
    const std::string pathSuffix1 = "cloud";
    const std::string pathSuffix2 = "cloud_fuse";
    if (realPath.rfind(pathSuffix1) == suffixBeginPos &&
        suffixBeginPos + pathSuffix1.length() == realPath.length()) {
        return true;
    }
    if (realPath.rfind(pathSuffix2) == suffixBeginPos &&
        suffixBeginPos + pathSuffix2.length() == realPath.length()) {
        return true;
    }
    return false;
}

int32_t FuseManager::StartFuse(int32_t userId, int32_t devFd, const string &path)
{
    LOGI("FuseManager::StartFuse entry");
    struct fuse_loop_config config;
    struct fuse_args args = FUSE_ARGS_INIT(0, nullptr);
    struct CloudDisk::CloudDiskFuseData cloudDiskData;
    struct FuseData data;
    struct fuse_session *se = nullptr;
    int ret;

    if (fuse_opt_add_arg(&args, path.c_str()) || !CheckPathForStartFuse(path)) {
        LOGE("Mount path invalid");
        return -EINVAL;
    }

    if (path.find("cloud_fuse") != string::npos) {
        se = fuse_session_new(&args, &cloudDiskFuseOps,
                              sizeof(cloudDiskFuseOps), &cloudDiskData);
        if (se == nullptr) {
            LOGE("cloud disk fuse_session_new error");
            return -EINVAL;
        }
        cloudDiskData.userId = userId;
        cloudDiskData.se = se;
        config.max_idle_threads = 1;
    } else {
        se = fuse_session_new(&args, &cloudMediaFuseOps, sizeof(cloudMediaFuseOps), &data);
        if (se == nullptr) {
            LOGE("cloud media fuse_session_new error");
            return -EINVAL;
        }
        data.userId = userId;
        data.se = se;
        config.max_idle_threads = MAX_IDLE_THREADS;
    }

    LOGI("fuse_session_new success, userId: %{public}d", userId);
    se->fd = devFd;
    se->mountpoint = strdup(path.c_str());
    if (se->mountpoint == nullptr) {
        return -ENOMEM;
    }

    fuse_daemonize(true);
    ret = fuse_session_loop_mt(se, &config);

    fuse_session_unmount(se);
    LOGI("fuse_session_unmount");
    if (se->mountpoint) {
        free(se->mountpoint);
        se->mountpoint = nullptr;
    }

    fuse_session_destroy(se);
    close(devFd);
    return ret;
}

FuseManager &FuseManager::GetInstance()
{
    static FuseManager instance_;
    return instance_;
}

} // namespace CloudFile
} // namespace FileManagement
} // namespace OHOS
