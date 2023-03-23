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

#include <fuse.h>
#include <fuse_lowlevel.h>  /* for fuse_cmdline_opts */

#include <errno.h>
#include <fcntl.h>
#include <assert.h>
#include <stddef.h>
#include <unistd.h>
#include <pthread.h>
#include <stdexcept>

#include <exception>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <map>

#include "utils_log.h"
namespace OHOS {
namespace Storage {
namespace CloudFile {
using namespace std;

#define FAKE_ROOT "/data/fake_cloud/"

struct fake_node {
	string path;
	int fd;
	ino_t ino;
	dev_t dev;
};

fake_node root_node;

map<unsigned long long, fake_node *> fake_cache;

unsigned long long gloabel_inode(ino_t ino, dev_t dev)
{
	unsigned long long gid = ino + ((unsigned long long)dev << 32);
	return gid;
}

static struct fake_node* __fake_node(fuse_ino_t ino)
{
	if (ino == FUSE_ROOT_ID)
		return &root_node;
	else
		return (struct fake_node *) (uintptr_t) ino;
}

static int fake_fd(fuse_ino_t ino)
{
    return __fake_node(ino)->fd;
}

static string fake_path(fuse_ino_t ino)
{
    return __fake_node(ino)->path;
}

static struct fake_node* find_node(struct stat *st)
{
	if(st->st_dev == root_node.dev && st->st_ino == root_node.ino)
		return &root_node;
	else
		return fake_cache[gloabel_inode(st->st_dev, st->st_ino)];

}

FuseManager::FuseManager()
{
}

static int fake_do_lookup(fuse_req_t req, fuse_ino_t parent, const char *name,
			 struct fuse_entry_param *e)
{
	int child_fd;
	int res;
	struct fake_node *child;
	unsigned long long gid;
	string tmp_name = name;


	memset(e, 0, sizeof(*e));

	child_fd = openat(fake_fd(parent), name, O_PATH | O_NOFOLLOW);
	if(child_fd < 0)
		goto out_err;

	res = fstatat(child_fd, "", &e->attr, AT_EMPTY_PATH | AT_SYMLINK_NOFOLLOW);
	if (res == -1)
		goto out_err;

	gid = gloabel_inode(e->attr.st_dev, e->attr.st_ino); 
	child = find_node(&e->attr);
	if(child) {
		close(child_fd);
		child_fd = -1;
	} else {
		child = new fake_node();
		child->fd = child_fd;
		child->ino = e->attr.st_ino;
		child->dev = e->attr.st_dev;
	        child->path = fake_path(parent) + tmp_name; 
		fake_cache[gid] = child;
	}
	
	e->ino = (uintptr_t) child;
	return 0;
out_err:
	if(child_fd > 0)
		close(child_fd);
	return errno;
}

static void cfs_lookup(fuse_req_t req, fuse_ino_t parent,
		       const char *name)
{
	LOGI("lookup");
	struct fuse_entry_param e;
	int err;

	err = fake_do_lookup(req, parent, name, &e);
	if (err)
		fuse_reply_err(req, err);
	else
		fuse_reply_entry(req, &e);
}

static void cfs_forget(fuse_req_t req, fuse_ino_t ino,
		       uint64_t nlookup)
{
	LOGI("forget");
	fuse_reply_none(req);
}

static void cfs_getattr(fuse_req_t req, fuse_ino_t ino,
			struct fuse_file_info *fi)
{
	int res;
	struct stat buf;
	(void) fi;
	LOGI("get attr %llu", ino);

	res = fstatat(fake_fd(ino), "", &buf, AT_EMPTY_PATH | AT_SYMLINK_NOFOLLOW);
	if (res == -1)
		return (void) fuse_reply_err(req, errno);

	fuse_reply_attr(req, &buf, 0);
}

static void cfs_open(fuse_req_t req, fuse_ino_t ino,
                     struct fuse_file_info *fi)
{
	LOGI("open %s", fake_path(ino).c_str());
	fuse_reply_err(req, ENOENT);
}

static void cfs_readdir(fuse_req_t req, fuse_ino_t ino, size_t size,
                        off_t off, struct fuse_file_info *fi)
{
	LOGI("readdir");
	fuse_reply_err(req, ENOENT);
}

static const struct fuse_lowlevel_ops cfs_oper = {
    .lookup	= cfs_lookup,
    .forget     = cfs_forget,
    .getattr	= cfs_getattr,
    .open	= cfs_open,
    .readdir	= cfs_readdir,
};


void FuseManager::Start(const string &mnt)
{
    struct fuse_session *se;
    struct fuse_cmdline_opts opts;
    struct fuse_loop_config config;
    struct fuse_args args = FUSE_ARGS_INIT(0, nullptr);
    int ret;
    struct stat stat;

    if(fuse_opt_add_arg(&args, mnt.c_str())) {
    	LOGE("Mount path invalid");
	return;
    }
    root_node.fd = -1;
    root_node.path = FAKE_ROOT;
    ret = lstat(FAKE_ROOT, &stat);
    if (ret == -1) {
	    LOGE("root is empty");
    } else {
          root_node.dev = stat.st_dev;
	  root_node.ino = stat.st_ino;
    }
    root_node.fd = open(FAKE_ROOT, O_PATH);
    if (root_node.fd < 0) {
	    LOGE("root is empty");
    }

    se = fuse_session_new(&args, &cfs_oper,
                          sizeof(cfs_oper), NULL);
    if (se == NULL)
        goto err_out1;

    if (fuse_set_signal_handlers(se) != 0)
        goto err_out2;

    if (fuse_session_mount(se, mnt.c_str()) != 0)
        goto err_out3;

    fuse_daemonize(false);
    config.max_idle_threads = 2;
    ret = fuse_session_loop_mt(se, &config);

    fuse_session_unmount(se);
    if (root_node.fd > 0) {
	    close(root_node.fd);
    }
err_out3:
    fuse_remove_signal_handlers(se);
err_out2:
    fuse_session_destroy(se);
err_out1:
    free(opts.mountpoint);
}

void FuseManager::Stop()
{
    LOGI("Stop finished successfully");
}

} // namespace CloudFile
} // namespace Storage
} // namespace OHOS
