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
#include <thread>
#include <unistd.h>
#include <sys/ioctl.h>

#include "cloud_daemon_statistic.h"
#include "cloud_disk_inode.h"
#include "cloud_file_kit.h"
#include "clouddisk_type_const.h"
#include "datetime_ex.h"
#include "dfs_error.h"
#include "directory_ex.h"
#include "ffrt_inner.h"
#include "fuse_operations.h"
#include "meta_file.h"
#include "utils_log.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace FileManagement {
namespace CloudFile {
using namespace std;

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
static const unsigned int TWO_MB = 2 * 1024 * 1024;
static const unsigned int KEY_FRAME_SIZE = 8192;
static const unsigned int MAX_IDLE_THREADS = 10;
static const unsigned int HMDFS_IOC = 0xf2;
static const std::chrono::seconds READ_TIMEOUT_S = 20s;
static const std::chrono::seconds OPEN_TIMEOUT_S = 4s;

#define HMDFS_IOC_HAS_CACHE _IOW(HMDFS_IOC, 6, struct HmdfsHasCache)

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
    off_t offset{0xffffffff};
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

struct ReadArguments {
    size_t size{0};
    off_t offset{0};
    shared_ptr<int64_t> readResult{nullptr};
    shared_ptr<bool> readFinish{nullptr};
    shared_ptr<CloudFile::CloudError> ckError{nullptr};
    shared_ptr<ffrt::condition_variable> cond{nullptr};

    ReadArguments(size_t readSize, off_t readOffset) : size(readSize), offset(readOffset)
    {
        readResult = make_shared<int64_t>(-1);
        readFinish = make_shared<bool>(false);
        ckError = make_shared<CloudFile::CloudError>();
        cond = make_shared<ffrt::condition_variable>();
    }
};

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
    shared_ptr<CloudInode> ret;
    std::shared_lock<std::shared_mutex> rLock(data->cacheLock, std::defer_lock);
    rLock.lock();
    ret = data->inodeCache[path];
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
        LOGD("directory, ino:%s", ino->path.c_str());
    } else {
        stbuf->st_mode = S_IFREG | STAT_MODE_REG;
        stbuf->st_nlink = STAT_NLINK_REG;
        stbuf->st_size = static_cast<decltype(stbuf->st_size)>(ino->mBase->size);
        LOGD("regular file, ino:%s, size: %lld", ino->path.c_str(), (long long)stbuf->st_size);
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

    LOGD("parent: %{private}s, name: %s", parentName.c_str(), name);

    child = FindNode(data, childName);
    if (!child) {
        child = make_shared<CloudInode>();
        create = true;
        LOGD("new child %s", child->path.c_str());
    }
    MetaBase mBase(name);
    err = MetaFile(data->userId, GetCloudInode(data, parent)->path).DoLookup(mBase);
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
    LOGD("lookup success, child: %{private}s, refCount: %lld", child->path.c_str(),
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
    LOGD("%s, put num: %lld,  current refCount: %d", node->path.c_str(), (long long)num,  node->refCount.load());
    if (node->refCount == 0) {
        LOGD("node released: %s", node->path.c_str());
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
    if (!node) {
        fuse_reply_err(req, ENOMEM);
        return;
    }
    LOGD("forget %s, nlookup: %lld", node->path.c_str(), (long long)nlookup);
    PutNode(data, node, nlookup);
    fuse_reply_none(req);
}

static void CloudGetAttr(fuse_req_t req, fuse_ino_t ino,
                         struct fuse_file_info *fi)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    struct stat buf;
    struct FuseData *data = static_cast<struct FuseData *>(fuse_req_userdata(req));
    (void) fi;

    LOGD("getattr, %s", CloudPath(data, ino).c_str());
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
         cInode->mBase->fileType, parentPath.string().c_str(), cInode->path.c_str());
    return path;
}

static void fuse_inval(fuse_session *se, fuse_ino_t parentIno, fuse_ino_t childIno, const string &childName)
{
    if (fuse_lowlevel_notify_inval_entry(se, parentIno, childName.c_str(), childName.size())) {
        fuse_lowlevel_notify_inval_inode(se, childIno, 0, 0);
    }
}

static int CloudOpenOnLocal(struct FuseData *data, shared_ptr<CloudInode> cInode, struct fuse_file_info *fi)
{
    string localPath = GetLocalPath(data->userId, cInode->path);
    string tmpPath = GetLocalTmpPath(data->userId, cInode->path);
    char resolvedPath[PATH_MAX] = {'\0'};
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
    MetaFile(data->userId, GetCloudInode(data, cInode->parent)->path).DoRemove(*(cInode->mBase));
    cInode->mBase->hasDownloaded = true;
    fi->fh = static_cast<uint64_t>(fd);
    return 0;
}

static int HandleOpenResult(CloudFile::CloudError ckError, struct FuseData *data,
    shared_ptr<CloudInode> cInode, struct fuse_file_info *fi)
{
    auto ret = HandleCloudError(ckError);
    if (ret < 0) {
        cInode->readSession = nullptr;
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
    *err = cInode->readSession->InitSession();
    {
        unique_lock lck(cInode->openLock);
        *openFinish = true;
    }
    cond->notify_one();
    LOGI("download done, path: %{public}s", cInode->path.c_str());
    return;
}

static int DoCloudOpen(shared_ptr<CloudInode> cInode, struct fuse_file_info *fi, struct FuseData *data)
{
    auto error = make_shared<CloudError>();
    auto openFinish = make_shared<bool>(false);
    auto cond = make_shared<ffrt::condition_variable>();
    ffrt::thread(DownloadThmOrLcd, cInode, error, openFinish, cond).detach();
    unique_lock lck(cInode->openLock);
    auto waitStatus = cond->wait_for(lck, OPEN_TIMEOUT_S, [openFinish] {
        return *openFinish;
    });
    if (!waitStatus) {
        LOGE("download %{public}s timeout", cInode->path.c_str());
        return -ENETUNREACH;
    }
    return HandleOpenResult(*error, data, cInode, fi);
}

static void CloudOpen(fuse_req_t req, fuse_ino_t ino,
                      struct fuse_file_info *fi)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    struct FuseData *data = static_cast<struct FuseData *>(fuse_req_userdata(req));
    fi->fh = UINT64_MAX;
    shared_ptr<CloudInode> cInode = GetCloudInode(data, ino);
    string recordId = MetaFileMgr::GetInstance().CloudIdToRecordId(cInode->mBase->cloudId);
    shared_ptr<CloudFile::CloudDatabase> database = GetDatabase(data);
    std::unique_lock<std::shared_mutex> wSesLock(cInode->sessionLock, std::defer_lock);

    LOGI("%{public}d open %{public}s", req->ctx.pid, CloudPath(data, ino).c_str());
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
        cInode->readSession = database->NewAssetReadSession("media", recordId,
                                                            GetAssetKey(cInode->mBase->fileType),
                                                            GetAssetPath(cInode, data));
        if (cInode->readSession) {
            auto ret = DoCloudOpen(cInode, fi, data);
            if (ret == 0) {
                fuse_reply_open(req, fi);
            } else {
                fuse_inval(data->se, cInode->parent, ino, cInode->mBase->name);
                fuse_reply_err(req, -ret);
            }
            uint64_t endTime = UTCTimeMilliSeconds();
            CloudDaemonStatistic &readStat = CloudDaemonStatistic::GetInstance();
            readStat.UpdateOpenSizeStat(cInode->mBase->size);
            readStat.UpdateOpenTimeStat(cInode->mBase->fileType, (endTime > startTime) ? (endTime - startTime) : 0);
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
    std::unique_lock<std::shared_mutex> wSesLock(cInode->sessionLock, std::defer_lock);
    LOGI("%{public}d release %{public}s, sessionRefCount: %{public}d", req->ctx.pid,
        CloudPath(data, ino).c_str(), cInode->sessionRefCount.load());
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
    LOGE("readdir %s, not support", CloudPath(data, ino).c_str());
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
            fuse_reply_err(req, ENOMEM);
            return;
        }
        LOGD("forget (i=%zu) %s, nlookup: %lld", i, node->path.c_str(), (long long)forgets[i].nlookup);
        PutNode(data, node, forgets[i].nlookup);
    }
    fuse_reply_none(req);
}

static void HasCache(fuse_req_t req, fuse_ino_t ino, const void *inBuf)
{
    struct FuseData *data = static_cast<struct FuseData *>(fuse_req_userdata(req));
    shared_ptr<CloudInode> cInode = GetCloudInode(data, ino);
    if (!cInode->readSession) {
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

static void CloudIoctl(fuse_req_t req, fuse_ino_t ino, int cmd, void *arg, struct fuse_file_info *fi,
                       unsigned flags, const void *inBuf, size_t inBufsz, size_t outBufsz)
{
    if (static_cast<unsigned int>(cmd) == HMDFS_IOC_HAS_CACHE) {
        HasCache(req, ino, inBuf);
    } else {
        fuse_reply_err(req, ENOTTY);
    }
}

static bool PrepareForRead(fuse_req_t req, shared_ptr<char> &buf, size_t size, shared_ptr<CloudInode> cInode,
				shared_ptr<CloudFile::CloudAssetReadSession> readSession)
{
    if (size > MAX_READ_SIZE) {
        fuse_reply_err(req, EINVAL);
        return false;
    }
    buf.reset(new char[size], [](char* ptr) {
        delete[] ptr;
    });
    if (!buf) {
        fuse_reply_err(req, ENOMEM);
        LOGE("buffer is null");
        return false;
    }

    if (!readSession) {
        fuse_reply_err(req, EPERM);
        LOGE("read fail, readsession is null");
        return false;
    }

    return true;
}

static void CloudReadOnCloudFile(shared_ptr<ReadArguments> readArgs, shared_ptr<char> buf,
    shared_ptr<CloudInode> cInode, shared_ptr<CloudFile::CloudAssetReadSession> readSession)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    LOGI("PRead cloud file, path: %{public}s, size: %{public}zd, off: %{public}lu", cInode->path.c_str(),
        readArgs->size, static_cast<unsigned long>(readArgs->offset));
    uint64_t startTime = UTCTimeMilliSeconds();
    *readArgs->readResult = readSession->PRead(readArgs->offset, readArgs->size, buf.get(), *readArgs->ckError);
    uint64_t endTime = UTCTimeMilliSeconds();
    uint64_t readTime = (endTime > startTime) ? (endTime - startTime) : 0;
    CloudDaemonStatistic &readStat = CloudDaemonStatistic::GetInstance();
    readStat.UpdateReadSizeStat(readArgs->size);
    readStat.UpdateReadTimeStat(readArgs->size, readTime);
    {
        unique_lock lck(cInode->readLock);
        *readArgs->readFinish = true;
    }
    readArgs->cond->notify_one();
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

static void CloudRead(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off,
                      struct fuse_file_info *fi)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    shared_ptr<char> buf = nullptr;
    struct FuseData *data = static_cast<struct FuseData *>(fuse_req_userdata(req));
    shared_ptr<CloudInode> cInode = GetCloudInode(data, ino);
    LOGI("%{public}s, size=%{public}zd, off=%{public}lu", CloudPath(data, ino).c_str(), size, (unsigned long)off);
    auto dkReadSession = cInode->readSession;
    size_t originalSize = size;
    if (size <= MAX_READ_SIZE) {
        if (off <= cInode->offset || off >= cInode->offset + MAX_READ_SIZE - size) {
            size = MAX_READ_SIZE;
            cInode->offset = off;
        }
    }
    if (!PrepareForRead(req, buf, size, cInode, dkReadSession)) {
        return;
    }
    if (cInode->mBase->hasDownloaded) {
        CloudReadOnLocalFile(req, buf, originalSize, off, fi);
        return;
    }
    shared_ptr<ReadArguments> readArgs = make_shared<ReadArguments>(size, off);
    ffrt::thread(CloudReadOnCloudFile, readArgs, buf, cInode, dkReadSession).detach();
    unique_lock lck(cInode->readLock);
    auto waitStatus = readArgs->cond->wait_for(lck, READ_TIMEOUT_S, [readArgs] {
        return *readArgs->readFinish;
    });
    if (!waitStatus) {
        LOGE("Pread timeout %{public}s, size=%{public}zd, off=%{public}lu", CloudPath(data, ino).c_str(), size,
            (unsigned long)off);
        fuse_reply_err(req, ENETUNREACH);
        return;
    }
    if (*readArgs->readResult < 0) {
        LOGE("readSize: %{public}lld", static_cast<long long>(*readArgs->readResult));
        fuse_reply_err(req, ENOMEM);
        return;
    }
    auto ret = HandleCloudError(*readArgs->ckError);
    if (ret < 0) {
        fuse_reply_err(req, -ret);
        return;
    }
    LOGD("read success");
    fuse_reply_buf(req, buf.get(), min(originalSize, static_cast<size_t>(*readArgs->readResult)));
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
    std::string PATH_PREFIX = "/mnt/data/";
    if (realPath.rfind(PATH_PREFIX, 0) != 0) {
        return false;
    }

    size_t userIdBeginPos = PATH_PREFIX.length();
    size_t userIdEndPos = realPath.find("/", userIdBeginPos);
    const std::string userId = realPath.substr(userIdBeginPos, userIdEndPos - userIdBeginPos);
    if (userId.find_first_not_of("0123456789") != std::string::npos) {
        return false;
    }

    size_t suffixBeginPos = userIdEndPos + 1;
    const std::string PATH_SUFFIX1 = "cloud";
    const std::string PATH_SUFFIX2 = "cloud_fuse";
    if (realPath.rfind(PATH_SUFFIX1) == suffixBeginPos &&
        suffixBeginPos + PATH_SUFFIX1.length() == realPath.length()) {
        return true;
    }
    if (realPath.rfind(PATH_SUFFIX2) == suffixBeginPos &&
        suffixBeginPos + PATH_SUFFIX2.length() == realPath.length()) {
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
        se = fuse_session_new(&args, &cloudMediaFuseOps,
                              sizeof(cloudMediaFuseOps), &data);
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
