/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#ifndef TEST_UNITTEST_CLOUD_DISK_SERVICE_ASSISTANT_H
#define TEST_UNITTEST_CLOUD_DISK_SERVICE_ASSISTANT_H

#include <dirent.h>
#include <gmock/gmock.h>

namespace OHOS::FileManagement::CloudDiskService {
class Assistant {
public:
    static inline std::shared_ptr<Assistant> ins = nullptr;

    virtual ~Assistant() = default;
    virtual ssize_t readlink(const char *pathname, char *buf, size_t bufsiz) = 0;
    virtual int fanotify_init(unsigned flags, unsigned event_f_flags) = 0;
    virtual int fanotify_mark(int fanotify_fd, unsigned flags, unsigned long long mask, int dfd,
        const char *pathname) = 0;
    virtual DIR *opendir(const char *path) = 0;
    virtual int dirfd(DIR *d) = 0;
    virtual int setxattr(const char *path, const char *name, const void *value, size_t size, int flags) = 0;
    virtual int fstat(int fd, struct stat *buf) = 0;
    virtual int ftruncate(int fd, off_t length) = 0;

    // file_utils
    virtual int64_t ReadFile(int fd, off_t offset, size_t size, void *data) = 0;
    virtual int64_t WriteFile(int fd, const void *data, off_t offset, size_t size) = 0;
    virtual int FilePosLock(int fd, off_t offset, size_t size, int type) = 0;

    virtual int access(const char *name, int type) = 0;
    virtual struct dirent* readdir(DIR* d) = 0;
    virtual int MockStat(const char* path, struct stat* buf);
};

class AssistantMock : public Assistant {
public:
    MOCK_METHOD3(readlink, ssize_t(const char *, char *, size_t));
    MOCK_METHOD2(fanotify_init, int(unsigned, unsigned));
    MOCK_METHOD5(fanotify_mark, int(int, unsigned, unsigned long long, int, const char *));
    MOCK_METHOD1(opendir, DIR *(const char *));
    MOCK_METHOD1(dirfd, int(DIR *));
    MOCK_METHOD5(setxattr, int(const char *, const char *, const void *, size_t, int));
    MOCK_METHOD2(fstat, int(int, struct stat *));
    MOCK_METHOD2(ftruncate, int(int, off_t));

    // file_utils
    MOCK_METHOD4(ReadFile, int64_t(int fd, off_t offset, size_t size, void *data));
    MOCK_METHOD4(WriteFile, int64_t(int fd, const void *data, off_t offset, size_t size));
    MOCK_METHOD4(FilePosLock, int(int fd, off_t offset, size_t size, int type));

    MOCK_METHOD2(access, int(const char *, int));
    MOCK_METHOD1(readdir, struct dirent*(DIR*));
    MOCK_METHOD2(MockStat, int(const char*, struct stat*));
};
} // namespace OHOS::FileManagement::CloudDiskService

#endif // TEST_UNITTEST_CLOUD_DISK_SERVICE_ASSISTANT_H
