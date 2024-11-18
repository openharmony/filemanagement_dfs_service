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

#ifndef TEST_UNITTESTS_CLOUD_DISK_ASSISTANT_H
#define TEST_UNITTESTS_CLOUD_DISK_ASSISTANT_H
#ifndef FUSE_USE_VERSION
#define FUSE_USE_VERSION 34
#endif

#include <fuse_lowlevel.h>
#include <fuse_opt.h>
#include <gmock/gmock.h>
#include <memory>

namespace OHOS::FileManagement::CloudDisk {
class Assistant {
public:
    static inline std::shared_ptr<Assistant> ins = nullptr;
public:
    virtual ~Assistant() = default;
    virtual void *fuse_req_userdata(fuse_req_t) = 0;
    virtual int fuse_reply_err(fuse_req_t, int) = 0;
    virtual int fuse_reply_open(fuse_req_t, const struct fuse_file_info *) = 0;
    virtual void fuse_reply_none(fuse_req_t) = 0;
    virtual int fuse_reply_data(fuse_req_t, struct fuse_bufvec *, enum fuse_buf_copy_flags) = 0;
    virtual size_t fuse_buf_size(const struct fuse_bufvec *) = 0;
    virtual ssize_t fuse_buf_copy(struct fuse_bufvec *, struct fuse_bufvec *, enum fuse_buf_copy_flags) = 0;
    virtual int fuse_reply_write(fuse_req_t, size_t) = 0;
    virtual int fuse_reply_lseek(fuse_req_t, off_t) = 0;
    virtual int fuse_reply_buf(fuse_req_t, const char *, size_t) = 0;
    virtual int fuse_reply_xattr(fuse_req_t, size_t) = 0;
    virtual int fuse_reply_attr(fuse_req_t, const struct stat *, double) = 0;
    virtual int fuse_reply_entry(fuse_req_t, const struct fuse_entry_param *) = 0;
    virtual int fuse_reply_create(fuse_req_t, const struct fuse_entry_param *, const struct fuse_file_info *) = 0;
    virtual int fuse_opt_add_arg(struct fuse_args *args, const char *arg) = 0;
    virtual struct fuse_session* fuse_session_new(struct fuse_args *args, const struct fuse_lowlevel_ops *op,
        size_t op_size, void *userdata) = 0;
    
    static int access(const char *name, int type)
    {
        if (strcmp(name, "mock") == 0) {
            return 0;
        }
        return 1;
    }
    static int open(const char *file, int oflag)
    {
        return 0;
    }
public:
    virtual off_t lseek(int, off_t, int) = 0;
};

class AssistantMock : public Assistant {
public:
    MOCK_METHOD1(fuse_req_userdata, void*(fuse_req_t));
    MOCK_METHOD2(fuse_reply_err, int(fuse_req_t, int));
    MOCK_METHOD2(fuse_reply_open, int(fuse_req_t, const struct fuse_file_info *));
    MOCK_METHOD1(fuse_reply_none, void(fuse_req_t));
    MOCK_METHOD3(fuse_reply_data, int(fuse_req_t, struct fuse_bufvec *, enum fuse_buf_copy_flags));
    MOCK_METHOD1(fuse_buf_size, size_t(const struct fuse_bufvec *));
    MOCK_METHOD3(fuse_buf_copy, ssize_t(struct fuse_bufvec *, struct fuse_bufvec *, enum fuse_buf_copy_flags));
    MOCK_METHOD2(fuse_reply_write, int(fuse_req_t, size_t));
    MOCK_METHOD2(fuse_reply_lseek, int(fuse_req_t, off_t));
    MOCK_METHOD3(fuse_reply_buf, int(fuse_req_t, const char *, size_t));
    MOCK_METHOD2(fuse_reply_xattr, int(fuse_req_t, size_t));
    MOCK_METHOD3(fuse_reply_attr, int(fuse_req_t, const struct stat *, double));
    MOCK_METHOD2(fuse_reply_entry, int(fuse_req_t, const struct fuse_entry_param *));
    MOCK_METHOD3(fuse_reply_create, int(fuse_req_t, const struct fuse_entry_param *, const struct fuse_file_info *));
    MOCK_METHOD2(fuse_opt_add_arg, int(struct fuse_args *args, const char *arg));
    MOCK_METHOD4(fuse_session_new, struct fuse_session*(struct fuse_args *args, const struct fuse_lowlevel_ops *op,
        size_t opSize, void *userData));
public:
    MOCK_METHOD3(lseek, off_t(int, off_t, int));
};
} // namespace OHOS::FileManagement::CloudDisk
#endif // TEST_UNITTESTS_CLOUD_DISK_ASSISTANT_H