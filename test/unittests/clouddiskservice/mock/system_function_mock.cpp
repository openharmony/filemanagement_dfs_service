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
#include "assistant.h"

#include <cerrno>
#include <cstdarg>
#include <dlfcn.h>
#include <fcntl.h>
#include <sys/fanotify.h>
#include <sys/ioctl.h>

#include "file_utils.h"

using namespace OHOS::FileManagement::CloudDiskService;

ssize_t readlink(const char *pathname, char *buf, size_t bufsiz)
{
    return Assistant::ins->readlink(pathname, buf, bufsiz);
}

int fanotify_init(unsigned flags, unsigned event_f_flags)
{
    return Assistant::ins->fanotify_init(flags, event_f_flags);
}

int fanotify_mark(int fanotify_fd,
                  unsigned flags,
                  unsigned long long mask,
                  int dfd,
                  const char *pathname)
{
    return Assistant::ins->fanotify_mark(fanotify_fd, flags, mask, dfd, pathname);
}

DIR* opendir(const char* path)
{
    return Assistant::ins->opendir(path);
}

int dirfd(DIR *d)
{
    return Assistant::ins->dirfd(d);
}

extern "C" {
int setxattr(const char *path, const char *name, const void *value, size_t size, int flags)
{
    return Assistant::ins->setxattr(path, name, value, size, flags);
}

int open(const char *path, int flags, ...)
{
    mode_t mode = 0;
    if ((flags & O_CREAT) != 0) {
        va_list args;
        va_start(args, flags);
        mode = static_cast<mode_t>(va_arg(args, int));
        va_end(args);
    }
    if (Assistant::mockFdApi) {
        if (Assistant::ins == nullptr) {
            errno = ENOENT;
            return -1;
        }
        errno = Assistant::mockErrno;
        return Assistant::ins->Open(path, flags, mode);
    }

    static int (*realOpen)(const char *, int, ...) = []() {
        return reinterpret_cast<int (*)(const char *, int, ...)>(dlsym(RTLD_NEXT, "open"));
    }();
    if (realOpen == nullptr) {
        return -1;
    }
    if ((flags & O_CREAT) != 0) {
        return realOpen(path, flags, mode);
    }
    return realOpen(path, flags);
}

int openat(int dirfd, const char *path, int flags, ...)
{
    mode_t mode = 0;
    if ((flags & O_CREAT) != 0) {
        va_list args;
        va_start(args, flags);
        mode = static_cast<mode_t>(va_arg(args, int));
        va_end(args);
    }
    if (Assistant::mockFdApi) {
        if (Assistant::ins == nullptr) {
            errno = ENOENT;
            return -1;
        }
        errno = Assistant::mockErrno;
        return Assistant::ins->OpenAt(dirfd, path, flags, mode);
    }

    static int (*realOpenAt)(int, const char *, int, ...) = []() {
        return reinterpret_cast<int (*)(int, const char *, int, ...)>(dlsym(RTLD_NEXT, "openat"));
    }();
    if (realOpenAt == nullptr) {
        return -1;
    }
    if ((flags & O_CREAT) != 0) {
        return realOpenAt(dirfd, path, flags, mode);
    }
    return realOpenAt(dirfd, path, flags);
}

int unlink(const char *path)
{
    if (Assistant::mockFdApi) {
        if (Assistant::ins == nullptr) {
            errno = ENOENT;
            return -1;
        }
        errno = Assistant::mockErrno;
        return Assistant::ins->Unlink(path);
    }

    static int (*realUnlink)(const char *) = []() {
        return reinterpret_cast<int (*)(const char *)>(dlsym(RTLD_NEXT, "unlink"));
    }();
    if (realUnlink == nullptr) {
        return -1;
    }
    return realUnlink(path);
}

int unlinkat(int dirfd, const char *path, int flags)
{
    if (Assistant::mockFdApi) {
        if (Assistant::ins == nullptr) {
            errno = ENOENT;
            return -1;
        }
        errno = Assistant::mockErrno;
        return Assistant::ins->UnlinkAt(dirfd, path, flags);
    }

    static int (*realUnlinkAt)(int, const char *, int) = []() {
        return reinterpret_cast<int (*)(int, const char *, int)>(dlsym(RTLD_NEXT, "unlinkat"));
    }();
    if (realUnlinkAt == nullptr) {
        return -1;
    }
    return realUnlinkAt(dirfd, path, flags);
}

int ioctl(int fd, int request, ...)
{
    va_list args;
    va_start(args, request);
    void *arg = va_arg(args, void *);
    va_end(args);

    if (Assistant::mockFdApi) {
        if (Assistant::ins == nullptr) {
            errno = ENOENT;
            return -1;
        }
        errno = Assistant::mockErrno;
        return Assistant::ins->Ioctl(fd, request, arg);
    }

    static int (*realIoctl)(int, int, ...) = []() {
        return reinterpret_cast<int (*)(int, int, ...)>(dlsym(RTLD_NEXT, "ioctl"));
    }();
    if (realIoctl == nullptr) {
        return -1;
    }
    return realIoctl(fd, request, arg);
}
}

int fstat(int fd, struct stat *buf)
{
    return Assistant::ins->fstat(fd, buf);
}

int ftruncate(int fd, off_t length)
{
    return Assistant::ins->ftruncate(fd, length);
}

struct dirent* readdir(DIR* d)
{
    return Assistant::ins->readdir(d);
}

namespace OHOS::FileManagement {
int64_t FileUtils::ReadFile(int fd, off_t offset, size_t size, void *data)
{
    return Assistant::ins->ReadFile(fd, offset, size, data);
}

int64_t FileUtils::WriteFile(int fd, const void *data, off_t offset, size_t size)
{
    return Assistant::ins->WriteFile(fd, data, offset, size);
}
} // namespace OHOS::FileManagement
