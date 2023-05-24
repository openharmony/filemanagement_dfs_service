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
#include <stdexcept>
#include <string>
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

const int SINGLE_ACCOUNT_USERID = 100;
const string PHOTOS_BUNDLE_NAME = "com.ohos.photos";
const unsigned int STAT_NLINK_REG = 1;
const unsigned int STAT_NLINK_DIR = 2;

struct MetaInode {
    shared_ptr<MetaBase> mBase{nullptr};
    shared_ptr<MetaFile> mFile{nullptr};
    string path;
    atomic<int> refCount{0};
    shared_ptr<DriveKit::DKAssetReadSession> readSession{nullptr};
    atomic<int> sessionRefCount{0};
};

static struct MetaInode *g_rootNode = nullptr;
static mutex g_cacheLock;
static map<string, struct MetaInode *> g_inodeCache;
static shared_ptr<CloudSync::SdkHelper> g_sdkHelper = nullptr;

static shared_ptr<CloudSync::SdkHelper> GetSdkHelper(void)
{
    if (!g_sdkHelper) {
        g_sdkHelper = make_shared<CloudSync::SdkHelper>();
        if (g_sdkHelper->Init(SINGLE_ACCOUNT_USERID, PHOTOS_BUNDLE_NAME) != E_OK)
            g_sdkHelper = nullptr;
    }
    return g_sdkHelper;
}

static struct MetaInode *GetMetaInode(fuse_ino_t ino)
{
    if (ino == FUSE_ROOT_ID) {
        if (!g_rootNode) {
            g_rootNode = new MetaInode();
            g_rootNode->mFile = MetaFileMgr::GetInstance().GetMetaFile(SINGLE_ACCOUNT_USERID, "/");
            g_rootNode->path = "/";
            g_rootNode->mBase = make_shared<MetaBase>();
            g_rootNode->mBase->mode = S_IFDIR;
            g_rootNode->refCount = 1;
        }
        return g_rootNode;
    } else {
        return (struct MetaInode *) (uintptr_t)ino;
    }
}

static string CloudPath(fuse_ino_t ino)
{
    return GetMetaInode(ino)->path;
}

static struct MetaInode *FindNode(string path)
{
    return g_inodeCache[path];
}

FuseManager::FuseManager()
{
}

static void GetMetaAttr(struct MetaInode *ino, struct stat *stbuf)
{
    stbuf->st_ino = reinterpret_cast<fuse_ino_t>(ino);
    if (ino->mBase->mode & S_IFDIR) {
        stbuf->st_mode = S_IFDIR;
        stbuf->st_nlink = STAT_NLINK_DIR;
    } else {
        stbuf->st_mode = S_IFREG;
        stbuf->st_nlink = STAT_NLINK_REG;
        stbuf->st_size = static_cast<decltype(stbuf->st_size)>(ino->mBase->size);
    }
}

static int CloudDoLookup(fuse_req_t req, fuse_ino_t parent, const char *name,
                         struct fuse_entry_param *e)
{
    int err;
    struct MetaInode *child;
    string childName = (parent == FUSE_ROOT_ID) ? CloudPath(parent) + name :
                                                  CloudPath(parent) + "/" + name;
    LOGD("parent: %{private}s, name: %s", CloudPath(parent).c_str(), name);

    child = FindNode(childName);
    if (child) {
        child->refCount++;
        LOGD("child exist, refCount: %lld", static_cast<long long>(child->refCount));
    } else {
        child = new MetaInode();
        child->mBase = make_shared<MetaBase>(name);
        err = GetMetaInode(parent)->mFile->DoLookup(*(child->mBase));
        if (err) {
            delete child;
            LOGE("lookup error, %{public}d", err);
            return err;
        }
        if (child->mBase->mode & S_IFDIR)
            child->mFile = MetaFileMgr::GetInstance().GetMetaFile(SINGLE_ACCOUNT_USERID,
                                                                  childName);
        child->path = childName;
        child->refCount = 1;
        g_cacheLock.lock();
        g_inodeCache[childName] = child;
        g_cacheLock.unlock();
        LOGD("lookup success, child: %{private}s", child->path.c_str());
    }
    GetMetaAttr(child, &e->attr);
    e->ino = reinterpret_cast<fuse_ino_t>(child);
    return 0;
}

static void CloudLookup(fuse_req_t req, fuse_ino_t parent,
                        const char *name)
{
    LOGD("lookup");
    struct fuse_entry_param e;
    int err;

    err = CloudDoLookup(req, parent, name, &e);
    if (err) {
        fuse_reply_err(req, err);
    } else {
        fuse_reply_entry(req, &e);
    }
}

static void PutNode(struct MetaInode *node, uint64_t num)
{
    node->refCount -= num;
    if (node->refCount == 0) {
        g_cacheLock.lock();
        g_inodeCache.erase(node->path);
        delete node;
        g_cacheLock.unlock();
    }
}

static void CloudForget(fuse_req_t req, fuse_ino_t ino,
                        uint64_t nlookup)
{
    LOGD("forget");
    MetaInode *node = GetMetaInode(ino);
    if (node) {
        PutNode(node, nlookup);
    }
    fuse_reply_none(req);
}

static void CloudGetAttr(fuse_req_t req, fuse_ino_t ino,
                         struct fuse_file_info *fi)
{
    struct stat buf;
    (void) fi;

    LOGD("getattr, ino:%lld", static_cast<long long>(ino));
    GetMetaAttr(GetMetaInode(ino), &buf);

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
    string path;
    MetaInode *mInode = GetMetaInode(ino);
    string recordId = MetaFileMgr::GetInstance().CloudIdToRecordId(mInode->mBase->cloudId);
    shared_ptr <CloudSync::SdkHelper> sdkHelper = GetSdkHelper();

    LOGD("open %s", CloudPath(ino).c_str());
    if (!sdkHelper) {
        fuse_reply_err(req, EPERM);
        return;
    }
    /*
     * recordType is "fileType" now for debug
     */
    if (!mInode->readSession) {
        path = "/data/service/el2/" + to_string(SINGLE_ACCOUNT_USERID) +
               "/hmdfs/fuse" + GetParentDir(mInode->path);
        ForceCreateDirectory(path);
        LOGD("recordId: %s, create dir: %s, filename: %s",
             recordId.c_str(), path.c_str(), mInode->mBase->name.c_str());
        mInode->readSession = sdkHelper->GetAssetReadSession("fileType",
                                                             recordId,
                                                             mInode->path,
                                                             path + "/" + mInode->mBase->name);
    }

    if (!mInode->readSession) {
        fuse_reply_err(req, EPERM);
    } else {
        mInode->sessionRefCount++;
        fuse_reply_open(req, fi);
    }
}

static void CloudRelease(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi)
{
    LOGD("release %s", CloudPath(ino).c_str());
    MetaInode *mInode = GetMetaInode(ino);

    mInode->sessionRefCount--;
    if (mInode->sessionRefCount == 0) {
        mInode->readSession = nullptr;
    }

    fuse_reply_err(req, 0);
}

static void CloudReadDir(fuse_req_t req, fuse_ino_t ino, size_t size,
                         off_t off, struct fuse_file_info *fi)
{
    LOGD("readdir");
    fuse_reply_err(req, ENOENT);
}

static void CloudForgetMulti(fuse_req_t req, size_t count,
				struct fuse_forget_data *forgets)
{
    LOGD("forget_multi");
    for (size_t i = 0; i < count; i++) {
        MetaInode *node = GetMetaInode(forgets[i].ino);
        PutNode(node, forgets[i].nlookup);
    }
    fuse_reply_none(req);
}

static void CloudRead(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off,
                      struct fuse_file_info *fi)
{
    int64_t readSize;
    DriveKit::DKError dkError;
    shared_ptr<char> buf = nullptr;

    LOGD("read(ino=%lu, size=%zd, off=%lu",
         (unsigned long)ino, size, (unsigned long)off);

    buf.reset(new char[size], [](char* ptr) {
        delete[] ptr;
    });
    if (!buf) {
        fuse_reply_err(req, ENOMEM);
        return;
    }

    if (!GetMetaInode(ino)->readSession) {
        fuse_reply_err(req, EPERM);
        return;
    }

    readSize = GetMetaInode(ino)->readSession->PRead(off, size, buf.get(), dkError);
    if (dkError.HasError()) {
        fuse_reply_err(req, EIO);
        return;
    }

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

int32_t FuseManager::StartFuse(int32_t devFd, const string &path)
{
    struct fuse_session *se;
    struct fuse_loop_config config;
    struct fuse_args args = FUSE_ARGS_INIT(0, nullptr);
    int ret;

    if (fuse_opt_add_arg(&args, path.c_str())) {
        LOGE("Mount path invalid");
        return -EINVAL;
    }

    se = fuse_session_new(&args, &cloudFuseOps,
                          sizeof(cloudFuseOps), nullptr);
    if (se == nullptr) {
        return -EINVAL;
    }
    se->fd = devFd;
    se->mountpoint = strdup(path.c_str());

    fuse_daemonize(true);
    config.max_idle_threads = 1;
    ret = fuse_session_loop_mt(se, &config);

    fuse_session_unmount(se);
    if (se->mountpoint) {
        free(se->mountpoint);
        se->mountpoint = nullptr;
    }

    fuse_session_destroy(se);
    close(devFd);
    return ret;
}

void FuseManager::Stop()
{
    LOGD("Stop finished successfully");
}

} // namespace CloudFile
} // namespace FileManagement
} // namespace OHOS
