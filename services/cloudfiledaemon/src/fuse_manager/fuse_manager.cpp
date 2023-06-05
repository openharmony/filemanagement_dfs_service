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

#define FUSE_USE_VERSION 34

#include "fuse_manager/fuse_manager.h"

#include <atomic>
#include <cassert>
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
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

#include <fuse.h>
#include <fuse_i.h>
#include <fuse_lowlevel.h> /* for fuse_cmdline_opts */

#include "dfs_error.h"
#include "directory_ex.h"
#include "dk_database.h"
#include "dk_asset_read_session.h"
#include "drive_kit.h"
#include "meta_file.h"
#include "sdk_helper.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudFile {
using namespace std;

const string PHOTOS_BUNDLE_NAME = "com.ohos.photos";
const unsigned int STAT_NLINK_REG = 1;
const unsigned int STAT_NLINK_DIR = 2;

struct CloudInode {
    shared_ptr<MetaBase> mBase{nullptr};
    shared_ptr<MetaFile> mFile{nullptr};
    string path;
    fuse_ino_t parent{0};
    atomic<int> refCount{0};
    shared_ptr<DriveKit::DKAssetReadSession> readSession{nullptr};
    atomic<int> sessionRefCount{0};
};

struct FuseData {
    int userId;
    shared_ptr<CloudInode> rootNode{nullptr};
    map<string, shared_ptr<CloudInode>> inodeCache;
    std::shared_mutex cacheLock;
    shared_ptr<DriveKit::DKDatabase> database;
    struct fuse_session *se;
};

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
        LOGD("create rootNode");
    }
    data->rootNode->mFile = MetaFileMgr::GetInstance().GetMetaFile(data->userId, "/");
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
    if (ino->mBase->mode & S_IFDIR) {
        stbuf->st_mode = S_IFDIR;
        stbuf->st_nlink = STAT_NLINK_DIR;
        LOGD("directory, ino:%s", ino->path.c_str());
    } else {
        stbuf->st_mode = S_IFREG;
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
    struct FuseData *data = (struct FuseData *)fuse_req_userdata(req);
    string childName = (parent == FUSE_ROOT_ID) ? CloudPath(data, parent) + name :
                                                  CloudPath(data, parent) + "/" + name;
    std::unique_lock<std::shared_mutex> wLock(data->cacheLock, std::defer_lock);

    LOGD("parent: %{private}s, name: %s", CloudPath(data, parent).c_str(), name);

    child = FindNode(data, childName);
    if (!child) {
        child = make_shared<CloudInode>();
        LOGD("new child %s", child->path.c_str());
    }
    child->mBase = make_shared<MetaBase>(name);
    child->path = childName;
    child->refCount++;
    err = GetCloudInode(data, parent)->mFile->DoLookup(*(child->mBase));
    if (err) {
        LOGE("lookup %s error, err: %{public}d", childName.c_str(), err);
        return err;
    }
    if (child->mBase->mode & S_IFDIR) {
        child->mFile = MetaFileMgr::GetInstance().GetMetaFile(data->userId,
                                                              childName);
    }
    child->parent = parent;
    wLock.lock();
    data->inodeCache[child->path] = child;
    wLock.unlock();
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
    struct FuseData *data = (struct FuseData *)fuse_req_userdata(req);
    shared_ptr<CloudInode> node = GetCloudInode(data, ino);
    LOGD("forget %s, nlookup: %lld", node->path.c_str(), (long long)nlookup);
    PutNode(data, node, nlookup);
    fuse_reply_none(req);
}

static void CloudGetAttr(fuse_req_t req, fuse_ino_t ino,
                         struct fuse_file_info *fi)
{
    struct stat buf;
    struct FuseData *data = (struct FuseData *)fuse_req_userdata(req);
    (void) fi;

    LOGD("getattr, %s", CloudPath(data, ino).c_str());
    GetMetaAttr(GetCloudInode(data, ino), &buf);

    fuse_reply_attr(req, &buf, 0);
}

static string GetParentDir(const string &path)
{
    if ((path == "/") || (path == "")) {
        return "";
    }

    auto pos = path.find_last_of('/');
    if ((pos == string::npos) || (pos == 0)) {
        return "/";
    }

    return path.substr(0, pos);
}

static void CloudOpen(fuse_req_t req, fuse_ino_t ino,
                      struct fuse_file_info *fi)
{
    struct FuseData *data = (struct FuseData *)fuse_req_userdata(req);
    shared_ptr<CloudInode> mInode = GetCloudInode(data, ino);
    string recordId = MetaFileMgr::GetInstance().CloudIdToRecordId(mInode->mBase->cloudId);
    shared_ptr<DriveKit::DKDatabase> database = GetDatabase(data);

    LOGD("open %s", CloudPath(data, ino).c_str());
    if (!database) {
        fuse_reply_err(req, EPERM);
        return;
    }
    if (!mInode->readSession) {
        string path = "/data/service/el2/" + to_string(data->userId) +
               "/hmdfs/fuse" + GetParentDir(mInode->path);
        ForceCreateDirectory(path);
        LOGD("recordId: %s, create dir: %s, filename: %s",
             recordId.c_str(), path.c_str(), mInode->mBase->name.c_str());
        /*
         * 'recordType' is fixed to "fileType" now
         * 'assetKey' is one of "content"/"lcd"/"thumbnail"
         */
        mInode->readSession = database->NewAssetReadSession("fileType",
                                                            recordId,
                                                            "content",
                                                            path + "/" + mInode->mBase->name);
    }

    if (!mInode->readSession) {
        fuse_reply_err(req, EPERM);
    } else {
        mInode->sessionRefCount++;
        LOGD("open success, sessionRefCount: %d", mInode->sessionRefCount.load());
        fuse_reply_open(req, fi);
    }
}

static void CloudRelease(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi)
{
    struct FuseData *data = (struct FuseData *)fuse_req_userdata(req);
    shared_ptr<CloudInode> mInode = GetCloudInode(data, ino);

    LOGD("%s, sessionRefCount: %d", CloudPath(data, ino).c_str(), mInode->sessionRefCount.load());
    mInode->sessionRefCount--;
    if (mInode->sessionRefCount == 0) {
        LOGD("readSession released");
        mInode->readSession = nullptr;
    }

    fuse_reply_err(req, 0);
}

static void CloudReadDir(fuse_req_t req, fuse_ino_t ino, size_t size,
                         off_t off, struct fuse_file_info *fi)
{
    struct FuseData *data = (struct FuseData *)fuse_req_userdata(req);
    LOGE("readdir %s, not support", CloudPath(data, ino).c_str());
    fuse_reply_err(req, ENOENT);
}

static void CloudForgetMulti(fuse_req_t req, size_t count,
				struct fuse_forget_data *forgets)
{
    struct FuseData *data = (struct FuseData *)fuse_req_userdata(req);
    LOGD("forget_multi");
    for (size_t i = 0; i < count; i++) {
        shared_ptr<CloudInode> node = GetCloudInode(data, forgets[i].ino);
        LOGD("forget (i=%zu) %s, nlookup: %lld", i, node->path.c_str(), (long long)forgets[i].nlookup);
        PutNode(data, node, forgets[i].nlookup);
    }
    fuse_reply_none(req);
}

static void CloudRead(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off,
                      struct fuse_file_info *fi)
{
    int64_t readSize;
    DriveKit::DKError dkError;
    shared_ptr<char> buf = nullptr;
    struct FuseData *data = (struct FuseData *)fuse_req_userdata(req);

    LOGD("%s, size=%zd, off=%lu", CloudPath(data, ino).c_str(), size, (unsigned long)off);

    buf.reset(new char[size], [](char* ptr) {
        delete[] ptr;
    });
    if (!buf) {
        fuse_reply_err(req, ENOMEM);
        return;
    }

    if (!GetCloudInode(data, ino)->readSession) {
        fuse_reply_err(req, EPERM);
        return;
    }

    readSize = GetCloudInode(data, ino)->readSession->PRead(off, size, buf.get(), dkError);
    if (dkError.HasError()) {
        LOGE("read error");
        fuse_reply_err(req, EIO);
        return;
    }

    LOGD("read %s success, %lld bytes", CloudPath(data, ino).c_str(), static_cast<long long>(readSize));
    fuse_reply_buf(req, buf.get(), readSize);
}

static const struct fuse_lowlevel_ops cloudFuseOps = {
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
    struct FuseData data;
    struct fuse_session *se = nullptr;
    int ret;

    if (fuse_opt_add_arg(&args, path.c_str())) {
        LOGE("Mount path invalid");
        return -EINVAL;
    }

    se = fuse_session_new(&args, &cloudFuseOps,
                          sizeof(cloudFuseOps), &data);
    if (se == nullptr) {
        LOGE("fuse_session_new error");
        return -EINVAL;
    }

    data.userId = userId;
    data.se = se;

    LOGI("fuse_session_new success, userId: %{public}d", userId);
    se->fd = devFd;
    se->mountpoint = strdup(path.c_str());
    sessions_[userId] = se;

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
