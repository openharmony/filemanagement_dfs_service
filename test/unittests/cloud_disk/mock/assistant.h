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

#include <fuse_lowlevel.h>
#include <memory>
#include <gmock/gmock.h>

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
public:
    virtual int access(const char *, int) = 0;
    virtual int open(const char *, int) = 0;
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

public:
    MOCK_METHOD2(access, int(const char *, int));
    MOCK_METHOD2(open, int(const char *, int));
    MOCK_METHOD3(lseek, off_t(int, off_t, int));
};
} // namespace OHOS::FileManagement::CloudDisk
#endif // TEST_UNITTESTS_CLOUD_DISK_ASSISTANT_H