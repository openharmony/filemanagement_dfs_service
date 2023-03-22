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

#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace CloudFile {
using namespace std;

FuseManager::FuseManager()
{
}

static void cfs_lookup(fuse_req_t req, fuse_ino_t parent,
		       const char *name)
{
	LOGI("lookup");
	fuse_reply_err(req, ENOENT);
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
	fuse_reply_err(req, ENOENT);
}

static void cfs_open(fuse_req_t req, fuse_ino_t ino,
                     struct fuse_file_info *fi)
{
	LOGI("open");
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

    if(fuse_opt_add_arg(&args, mnt.c_str())) {
    	LOGE("Mount path invalid");
	return;
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
