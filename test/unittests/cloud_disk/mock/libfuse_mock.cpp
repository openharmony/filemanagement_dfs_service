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

#include "file_operations_base.h"

#include "assistant.h"

using namespace OHOS::FileManagement::CloudDisk;
void *fuse_req_userdata(fuse_req_t req)
{
    return Assistant::ins->fuse_req_userdata(req);
}

int fuse_reply_err(fuse_req_t req, int err)
{
    return Assistant::ins->fuse_reply_err(req, err);
}

int fuse_reply_open(fuse_req_t req, const struct fuse_file_info *fi)
{
    return Assistant::ins->fuse_reply_open(req, fi);
}

void fuse_reply_none(fuse_req_t req)
{
    return Assistant::ins->fuse_reply_none(req);
}

int fuse_reply_data(fuse_req_t req, struct fuse_bufvec *bufv, enum fuse_buf_copy_flags flags)
{
    return Assistant::ins->fuse_reply_data(req, bufv, flags);
}

size_t fuse_buf_size(const struct fuse_bufvec *bufv)
{
    return Assistant::ins->fuse_buf_size(bufv);
}

ssize_t fuse_buf_copy(struct fuse_bufvec *dst, struct fuse_bufvec *src, enum fuse_buf_copy_flags flags)
{
    return Assistant::ins->fuse_buf_copy(dst, src, flags);
}

int fuse_reply_write(fuse_req_t req, size_t count)
{
    return Assistant::ins->fuse_reply_write(req, count);
}

int fuse_reply_lseek(fuse_req_t req, off_t off)
{
    return Assistant::ins->fuse_reply_lseek(req, off);
}

int fuse_reply_buf(fuse_req_t req, const char *buf, size_t size)
{
    return Assistant::ins->fuse_reply_buf(req, buf, size);
}

int fuse_reply_xattr(fuse_req_t req, size_t count)
{
    return Assistant::ins->fuse_reply_xattr(req, count);
}