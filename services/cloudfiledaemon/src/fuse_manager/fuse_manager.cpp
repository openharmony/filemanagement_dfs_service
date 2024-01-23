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

#include "fuse_manager/fuse_manager.h"

#include <atomic>
#include <cassert>
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
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
#include <thread>
#include <unistd.h>

#include "cloud_disk_inode.h"
#include "datetime_ex.h"
#include "dfs_error.h"
#include "directory_ex.h"
#include "dk_database.h"
#include "dk_asset_read_session.h"
#include "drive_kit.h"
#include "fuse_operations.h"
#include "meta_file.h"
#include "sdk_helper.h"
#include "utils_log.h"

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
static const unsigned int READ_TIMEOUT_MS = 4000;
static const unsigned int MAX_READ_SIZE = 2 * 1024 * 1024;

struct CloudInode {
    shared_ptr<MetaBase> mBase{nullptr};
    string path;
    fuse_ino_t parent{0};
    atomic<int> refCount{0};
    shared_ptr<DriveKit::DKAssetReadSession> readSession{nullptr};
    atomic<int> sessionRefCount{0};
    std::shared_mutex sessionLock;
    std::shared_mutex readLock;
};

struct FuseData {
    int userId;
    shared_ptr<CloudInode> rootNode{nullptr};
    /* store CloudInode by path */
    map<string, shared_ptr<CloudInode>> inodeCache;
    std::shared_mutex cacheLock;
    shared_ptr<DriveKit::DKDatabase> database;
    struct fuse_session *se;
};

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
    } else {
        LOGE("Unknow error");
        fuse_reply_err(req, EIO);
    }
    return true;
}

static shared_ptr<DriveKit::DKDatabase> GetDatabase(struct FuseData *data)
{
    if (!data->database) {
        auto driveKit = DriveKit::DriveKitNative::GetInstance(data->userId);
        if (driveKit == nullptr) {
            LOGE("sdk helper get drive kit instance fail");
            return nullptr;
        }

        auto container = driveKit->GetDefaultContainer(PHOTOS_BUNDLE_NAME);
        if (container == nullptr) {
            LOGE("sdk helper get drive kit container fail");
            return nullptr;
        }

        data->database = container->GetPrivateDatabase();
        if (data->database == nullptr) {
            LOGE("sdk helper get drive kit database fail");
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
    return GetCloudInode(data, ino)->path;
}

static void GetMetaAttr(shared_ptr<CloudInode> ino, struct stat *stbuf)
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
    string childName = (parent == FUSE_ROOT_ID) ? CloudPath(data, parent) + name :
                                                  CloudPath(data, parent) + "/" + name;
    std::unique_lock<std::shared_mutex> wLock(data->cacheLock, std::defer_lock);

    LOGD("parent: %{private}s, name: %s", CloudPath(data, parent).c_str(), name);

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
    GetMetaAttr(child, &e->attr);
    e->ino = reinterpret_cast<fuse_ino_t>(child.get());
    return 0;
}

static void CloudLookup(fuse_req_t req, fuse_ino_t parent,
                        const char *name)
{
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
    LOGD("forget %s, nlookup: %lld", node->path.c_str(), (long long)nlookup);
    PutNode(data, node, nlookup);
    fuse_reply_none(req);
}

static void CloudGetAttr(fuse_req_t req, fuse_ino_t ino,
                         struct fuse_file_info *fi)
{
    struct stat buf;
    struct FuseData *data = static_cast<struct FuseData *>(fuse_req_userdata(req));
    (void) fi;

    LOGD("getattr, %s", CloudPath(data, ino).c_str());
    GetMetaAttr(GetCloudInode(data, ino), &buf);

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

static string GetLocalPath(int32_t userId, const string &relativePath)
{
    return HMDFS_PATH_PREFIX + to_string(userId) + LOCAL_PATH_SUFFIX + relativePath;
}

static string GetLocalTmpPath(int32_t userId, const string &relativePath)
{
    return GetLocalPath(userId, relativePath) + PATH_TEMP_SUFFIX;
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

static void CloudOpen(fuse_req_t req, fuse_ino_t ino,
                      struct fuse_file_info *fi)
{
    struct FuseData *data = static_cast<struct FuseData *>(fuse_req_userdata(req));
    shared_ptr<CloudInode> cInode = GetCloudInode(data, ino);
    string recordId = MetaFileMgr::GetInstance().CloudIdToRecordId(cInode->mBase->cloudId);
    shared_ptr<DriveKit::DKDatabase> database = GetDatabase(data);
    std::unique_lock<std::shared_mutex> wSesLock(cInode->sessionLock, std::defer_lock);

    LOGI("%{public}d open %{public}s", req->ctx.pid, CloudPath(data, ino).c_str());
    if (!database) {
        fuse_reply_err(req, EPERM);
        LOGE("database is null");
        return;
    }
    wSesLock.lock();
    if (!cInode->readSession) {
        /*
         * 'recordType' is fixed to "media" now
         * 'assetKey' is one of "content"/"lcd"/"thumbnail"
         */
        LOGD("recordId: %s", recordId.c_str());
        cInode->readSession = database->NewAssetReadSession("media",
                                                            recordId,
                                                            GetAssetKey(cInode->mBase->fileType),
                                                            GetAssetPath(cInode, data));
        if (cInode->readSession) {
            DriveKit::DKError dkError = cInode->readSession->InitSession();
            if (!HandleDkError(req, dkError)) {
                cInode->sessionRefCount++;
                LOGD("open success, sessionRefCount: %d", cInode->sessionRefCount.load());
                fuse_reply_open(req, fi);
            } else {
                cInode->readSession = nullptr;
                LOGE("open fali");
            }
            wSesLock.unlock();
            return;
        }
    }
    if (!cInode->readSession) {
        fuse_reply_err(req, EPERM);
        LOGE("readSession is null");
    } else {
        cInode->sessionRefCount++;
        LOGD("open success, sessionRefCount: %d", cInode->sessionRefCount.load());
        fuse_reply_open(req, fi);
    }
    wSesLock.unlock();
}

static void CloudRelease(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi)
{
    struct FuseData *data = static_cast<struct FuseData *>(fuse_req_userdata(req));
    shared_ptr<CloudInode> cInode = GetCloudInode(data, ino);
    std::unique_lock<std::shared_mutex> wSesLock(cInode->sessionLock, std::defer_lock);
    string cloudMergeViewPath = GetCloudMergeViewPath(data->userId, cInode->path);
    string localPath = GetLocalPath(data->userId, cInode->path);
    string tmpPath = GetLocalTmpPath(data->userId, cInode->path);

    wSesLock.lock();
    LOGI("%{public}d release %{public}s, sessionRefCount: %{public}d", req->ctx.pid,
        CloudPath(data, ino).c_str(), cInode->sessionRefCount.load());
    cInode->sessionRefCount--;
    if (cInode->sessionRefCount == 0) {
        bool needRemain = false;
        if (cInode->mBase->fileType != FILE_TYPE_CONTENT) {
            needRemain = true;
        }
        bool res = cInode->readSession->Close(needRemain);
        if (!res) {
            LOGE("close error, needRemain: %{public}d", needRemain);
        }
        if (needRemain && res) {
            MetaFile(data->userId, GetCloudInode(data, cInode->parent)->path).DoRemove(*(cInode->mBase));
            LOGD("remove from dentryfile");

            /*
             * after removing file item from dentryfile, we should delete file
             * of cloud merge view to update kernel dentry cache.
             */
            if (remove(cloudMergeViewPath.c_str()) != 0) {
                LOGE("update kernel dentry cache fail, errno: %{public}d, cloudMergeViewPath: %{public}s",
                     errno, cloudMergeViewPath.c_str());
            }

            filesystem::path parentPath = filesystem::path(localPath).parent_path();
            ForceCreateDirectory(parentPath.string());
            if (rename(tmpPath.c_str(), localPath.c_str()) != 0) {
                LOGE("err rename tmpPath to localPath, errno: %{public}d", errno);
            }
        }
        cInode->readSession = nullptr;
        LOGD("readSession released");
    }
    wSesLock.unlock();

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
        LOGD("forget (i=%zu) %s, nlookup: %lld", i, node->path.c_str(), (long long)forgets[i].nlookup);
        PutNode(data, node, forgets[i].nlookup);
    }
    fuse_reply_none(req);
}

static void ChangeReadAvailable(shared_ptr<CloudInode> cInode, shared_ptr<bool> readAvailable)
{
    unique_lock<shared_mutex> wLock(cInode->readLock, defer_lock);
    wLock.lock();
    *readAvailable = false;
    wLock.unlock();
}

static void CloudRead(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off,
                      struct fuse_file_info *fi)
{
    shared_ptr<DriveKit::DKError> dkError = make_shared<DriveKit::DKError>();
    shared_ptr<char> buf = nullptr;
    struct FuseData *data = static_cast<struct FuseData *>(fuse_req_userdata(req));
    shared_ptr<CloudInode> cInode = GetCloudInode(data, ino);
    LOGD("%s, size=%zd, off=%lu", CloudPath(data, ino).c_str(), size, (unsigned long)off);

    if (size > MAX_READ_SIZE) {
        fuse_reply_err(req, EINVAL);
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

    if (!cInode->readSession) {
        fuse_reply_err(req, EPERM);
        LOGE("read fail, readsession is null");
        return;
    }

    shared_ptr<int64_t> readSize = make_shared<int64_t>(-1);
    shared_ptr<bool> readAvailable = make_shared<bool>(true);
    condition_variable readConVar;
    thread([=, &readConVar]() {
        *readSize = cInode->readSession->PRead(off, size, buf.get(), *dkError);
        LOGD("read %s result, %lld bytes", CloudPath(data, ino).c_str(), static_cast<long long>(*readSize));
        unique_lock<shared_mutex> wLock(cInode->readLock, defer_lock);
        wLock.lock();
        if (*readAvailable) {
        readConVar.notify_one();
        }
        wLock.unlock();
        }).detach();

    mutex readMutex;
    unique_lock<mutex> lock(readMutex);
    auto waitStatus = readConVar.wait_for(lock, chrono::milliseconds(READ_TIMEOUT_MS));
    if (waitStatus == cv_status::timeout) {
        LOGE("Pread timeout");
        ChangeReadAvailable(cInode, readAvailable);
        fuse_reply_err(req, ENOTCONN);
        return;
    }

    if (!HandleDkError(req, *dkError)) {
        LOGD("read success");
        fuse_reply_buf(req, buf.get(), *readSize);
    }
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
    .forget_multi       = CloudForgetMulti,
};

int32_t FuseManager::StartFuse(int32_t userId, int32_t devFd, const string &path)
{
    struct fuse_loop_config config;
    struct fuse_args args = FUSE_ARGS_INIT(0, nullptr);
    struct CloudDisk::CloudDiskFuseData cloudDiskData;
    struct FuseData data;
    struct fuse_session *se = nullptr;
    int ret;

    if (fuse_opt_add_arg(&args, path.c_str())) {
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
    } else {
        se = fuse_session_new(&args, &cloudMediaFuseOps,
                              sizeof(cloudMediaFuseOps), &data);
        if (se == nullptr) {
            LOGE("cloud media fuse_session_new error");
            return -EINVAL;
        }
        data.userId = userId;
        data.se = se;
    }

    LOGI("fuse_session_new success, userId: %{public}d", userId);
    se->fd = devFd;
    se->mountpoint = strdup(path.c_str());

    fuse_daemonize(true);
    config.max_idle_threads = 1;
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
