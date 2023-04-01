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

#include <fuse_i.h>
#include <fuse.h>
#include <fuse_lowlevel.h>  /* for fuse_cmdline_opts */

#include <cerrno>
#include <fcntl.h>
#include <cassert>
#include <cstddef>
#include <unistd.h>
#include <pthread.h>
#include <stdexcept>

#include <exception>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <map>
#include <mutex>
#include <atomic>

#include "utils_log.h"
namespace OHOS {
namespace FileManagement {
namespace CloudFile {
using namespace std;

#define FAKE_ROOT "/data/service/el2/100/hmdfs/non_account/fake_cloud/"

struct FakeNode {
    string path;
    int fd;
    ino_t ino;
    dev_t dev;
    atomic_int64_t refCount;
};

FakeNode g_rootNode;

mutex g_cacheLock;
map<unsigned long long, FakeNode *> fakeCache;

unsigned long long GlobalNode(ino_t ino, dev_t dev)
{
    unsigned long long gid = ino + (static_cast<unsigned long long>(dev) << 32);
    return gid;
}

static struct FakeNode* GetFakeNode(fuse_ino_t ino)
{
    if (ino == FUSE_ROOT_ID) {
        return &g_rootNode;
    } else {
        return (struct FakeNode *) (uintptr_t) ino;
    }
}

static int FakeFd(fuse_ino_t ino)
{
    return GetFakeNode(ino)->fd;
}

static string FakePath(fuse_ino_t ino)
{
    return GetFakeNode(ino)->path;
}

static struct FakeNode* FindNode(struct stat *st)
{
    if (st->st_dev == g_rootNode.dev && st->st_ino == g_rootNode.ino) {
        return &g_rootNode;
    } else {
        return fakeCache[GlobalNode(st->st_dev, st->st_ino)];
    }
}

FuseManager::FuseManager()
{
}

static int FakeDoLookup(fuse_req_t req, fuse_ino_t parent, const char *name,
                        struct fuse_entry_param *e)
{
    int childFd;
    int res;
    struct FakeNode *child;
    unsigned long long gid;
    string tmpName = name;

    *e = {0};
    childFd = openat(FakeFd(parent), name, O_PATH | O_NOFOLLOW);
    if (childFd < 0) {
        return errno;
    }

    res = fstatat(childFd, "", &e->attr, AT_EMPTY_PATH | AT_SYMLINK_NOFOLLOW);
    if (res == -1) {
        close(childFd);
        return errno;
    }

    gid = GlobalNode(e->attr.st_dev, e->attr.st_ino);
    child = FindNode(&e->attr);
    if (child) {
        close(childFd);
        child->refCount++;
        childFd = -1;
    } else {
        child = new FakeNode();
        child->fd = childFd;
        child->ino = e->attr.st_ino;
        child->dev = e->attr.st_dev;
        child->path = FakePath(parent) + tmpName;
        child->refCount = 1;
        g_cacheLock.lock();
        fakeCache[gid] = child;
        g_cacheLock.unlock();
    }
    e->ino = reinterpret_cast<fuse_ino_t>(child);
    return 0;
}

static void FakeLookup(fuse_req_t req, fuse_ino_t parent,
                       const char *name)
{
    LOGI("lookup");
    struct fuse_entry_param e;
    int err;

    err = FakeDoLookup(req, parent, name, &e);
    if (err) {
        fuse_reply_err(req, err);
    } else {
        fuse_reply_entry(req, &e);
    }
}

static void PutNode(struct FakeNode *node, uint64_t num)
{
    node->refCount -= num;
    if (node->refCount == 0) {
        g_cacheLock.lock();
        fakeCache.erase(GlobalNode(node->dev, node->ino));
        close(node->fd);
        delete node;
        node = nullptr;
        g_cacheLock.unlock();
    }
}

static void FakeForget(fuse_req_t req, fuse_ino_t ino,
                       uint64_t nlookup)
{
    LOGI("forget");
    FakeNode *node = GetFakeNode(ino);
    if (node) {
        PutNode(node, nlookup);
    }
    fuse_reply_none(req);
}

static void FakeGetAttr(fuse_req_t req, fuse_ino_t ino,
                        struct fuse_file_info *fi)
{
    int res;
    struct stat buf;
    (void) fi;

    res = fstatat(FakeFd(ino), "", &buf, AT_EMPTY_PATH | AT_SYMLINK_NOFOLLOW);
    if (res == -1) {
        return (void) fuse_reply_err(req, errno);
    }

    fuse_reply_attr(req, &buf, 0);
}

static void FakeOpen(fuse_req_t req, fuse_ino_t ino,
                     struct fuse_file_info *fi)
{
    int fd;
    string path;

    LOGI("open %s", FakePath(ino).c_str());
    path = "/proc/self/fd/" + std::to_string(FakeFd(ino));
    fd = open(path.c_str(), fi->flags & ~O_NOFOLLOW);
    if (fd == -1)
            return (void) fuse_reply_err(req, errno);
    fi->fh = fd;
    fuse_reply_open(req, fi);
}

static void FakeFlush(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi)
{
    int res;

    LOGI("Close %s", FakePath(ino).c_str());
    res = close(dup(fi->fh));
    fuse_reply_err(req, res == -1 ? errno : 0);
}

static void FakeRelease(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi)
{
    LOGI("release %s", FakePath(ino).c_str());

    close(fi->fh);
    fuse_reply_err(req, 0);
}

static void FakeReadDir(fuse_req_t req, fuse_ino_t ino, size_t size,
                        off_t off, struct fuse_file_info *fi)
{
    LOGI("readdir");
    fuse_reply_err(req, ENOENT);
}

static void FakeForgetMulti(fuse_req_t req, size_t count,
				struct fuse_forget_data *forgets)
{
    for (int i = 0; i < count; i++) {
        FakeNode *node = GetFakeNode(forgets[i].ino);
        PutNode(node, forgets[i].nlookup);
    }
    fuse_reply_none(req);
}

static const struct fuse_lowlevel_ops fakeOps = {
    .lookup 		= FakeLookup,
    .forget     	= FakeForget,
    .getattr   		= FakeGetAttr,
    .open   		= FakeOpen,
    .flush  		= FakeFlush,
    .release    	= FakeRelease,
    .readdir    	= FakeReadDir,
    .forget_multi 	= FakeForgetMulti,
};

int32_t FuseManager::StartFuse(int32_t devFd, const string &path)
{
    struct fuse_session *se;
    struct fuse_loop_config config;
    struct fuse_args args = FUSE_ARGS_INIT(0, nullptr);
    int ret;
    struct stat stat;

    if (fuse_opt_add_arg(&args, path.c_str())) {
        LOGE("Mount path invalid");
        return -EINVAL;
    }
    g_rootNode.fd = -1;
    g_rootNode.path = FAKE_ROOT;
    ret = lstat(FAKE_ROOT, &stat);
    if (ret == -1) {
        LOGE("root is empty");
    } else {
        g_rootNode.dev = stat.st_dev;
        g_rootNode.ino = stat.st_ino;
    }
    g_rootNode.fd = open(FAKE_ROOT, O_PATH);
    if (g_rootNode.fd < 0) {
        LOGE("root is empty");
    }

    se = fuse_session_new(&args, &fakeOps,
                          sizeof(fakeOps), NULL);
    if (se == NULL) {
        return -EINVAL;
    }

    if (fuse_set_signal_handlers(se) != 0) {
        fuse_session_destroy(se);
        return -EINVAL;
    }
    se->fd = devFd;
    se->mountpoint = strdup(path.c_str());

    fuse_daemonize(false);
    config.max_idle_threads = 1;
    ret = fuse_session_loop_mt(se, &config);

    fuse_session_unmount(se);
    if (g_rootNode.fd > 0) {
        close(g_rootNode.fd);
    }
    if (se->mountpoint) {
        free(se->mountpoint);
        se->mountpoint = nullptr;
    }

    fuse_remove_signal_handlers(se);
    fuse_session_destroy(se);
    return 0;
}

void FuseManager::Stop()
{
    LOGI("Stop finished successfully");
}

} // namespace CloudFile
} // namespace FileManagement
} // namespace OHOS
