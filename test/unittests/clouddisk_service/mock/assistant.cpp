/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "assistant.h"
#include <dlfcn.h>
#include <sys/fanotify.h>

#include "file_utils.h"

namespace OHOS {
namespace FileManagement {
namespace CloudDiskService {

void AssistantMock::EnableMock()
{
    Assistant::mockable = true;
}

void AssistantMock::DisableMock()
{
    ins = nullptr;
    Assistant::mockable = false;
}

bool AssistantMock::IsMockable()
{
    return Assistant::mockable;
}

} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS

#ifdef __cplusplus
extern "C" {
using namespace OHOS::FileManagement::CloudDiskService;

int access(const char* path, int mode)
{
    if (AssistantMock::IsMockable()) {
        return Assistant::ins->access(path, mode);
    }

    static int (*realAccess)(const char* path, int mode) = []() {
        auto func = (int (*)(const char* path, int mode))dlsym(RTLD_NEXT, "access");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real access: " << dlerror();
        }
        return func;
    }();

    if (!realAccess) {
        return -1;
    }

    return realAccess(path, mode);
}

int stat(const char* path, struct stat* buf)
{
    if (AssistantMock::IsMockable()) {
        return Assistant::ins->stat(path, buf);
    }

    static int (*realStat)(const char* path, struct stat* buf) = []() {
        auto func = (int (*)(const char* path, struct stat* buf))dlsym(RTLD_NEXT, "stat");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real stat: " << dlerror();
        }
        return func;
    }();

    if (!realStat) {
        return -1;
    }

    return realStat(path, buf);
}

ssize_t readlink(const char *pathname, char *buf, size_t bufsiz)
{
    if (AssistantMock::IsMockable()) {
        return Assistant::ins->readlink(pathname, buf, bufsiz);
    }

    static ssize_t (*realReadlink)(const char *, char *, size_t) = []() {
        auto func = (ssize_t (*)(const char *, char *, size_t))dlsym(RTLD_NEXT, "readlink");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real readlink: " << dlerror();
        }
        return func;
    }();

    if (!realReadlink) {
        return -1;
    }

    return realReadlink(pathname, buf, bufsiz);
}

int fanotify_init(unsigned flags, unsigned event_f_flags)
{
    if (AssistantMock::IsMockable()) {
        return Assistant::ins->fanotify_init(flags, event_f_flags);
    }

    static int (*realFanotifyInit)(unsigned, unsigned) = []() {
        auto func = (int (*)(unsigned, unsigned))dlsym(RTLD_NEXT, "fanotify_init");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real fanotify_init: " << dlerror();
        }
        return func;
    }();

    if (!realFanotifyInit) {
        return -1;
    }

    return realFanotifyInit(flags, event_f_flags);
}

int fanotify_mark(int fanotify_fd, unsigned flags, unsigned long long mask, int dfd, const char *pathname)
{
    if (AssistantMock::IsMockable()) {
        return Assistant::ins->fanotify_mark(fanotify_fd, flags, mask, dfd, pathname);
    }

    static int (*realFanotifyMark)(int, unsigned, unsigned long long, int, const char *) = []() {
        auto func = (int (*)(int, unsigned, unsigned long long, int, const char *))dlsym(RTLD_NEXT, "fanotify_mark");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real fanotify_mark: " << dlerror();
        }
        return func;
    }();

    if (!realFanotifyMark) {
        return -1;
    }

    return realFanotifyMark(fanotify_fd, flags, mask, dfd, pathname);
}

DIR *opendir(const char *path)
{
    if (AssistantMock::IsMockable()) {
        return Assistant::ins->opendir(path);
    }

    static DIR *(*realOpendir)(const char *) = []() {
        auto func = (DIR *(*)(const char *))dlsym(RTLD_NEXT, "opendir");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real opendir: " << dlerror();
        }
        return func;
    }();

    if (!realOpendir) {
        return nullptr;
    }

    return realOpendir(path);
}

int dirfd(DIR *d)
{
    if (AssistantMock::IsMockable()) {
        return Assistant::ins->dirfd(d);
    }

    static int (*realDirfd)(DIR *) = []() {
        auto func = (int (*)(DIR *))dlsym(RTLD_NEXT, "dirfd");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real dirfd: " << dlerror();
        }
        return func;
    }();

    if (!realDirfd) {
        return -1;
    }

    return realDirfd(d);
}

int setxattr(const char *path, const char *name, const void *value, size_t size, int flags)
{
    if (AssistantMock::IsMockable()) {
        return Assistant::ins->setxattr(path, name, value, size, flags);
    }

    static int (*realSetxattr)(const char *, const char *, const void *, size_t, int) = []() {
        auto func = (int (*)(const char *, const char *, const void *, size_t, int))dlsym(RTLD_NEXT, "setxattr");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real setxattr: " << dlerror();
        }
        return func;
    }();

    if (!realSetxattr) {
        return -1;
    }

    return realSetxattr(path, name, value, size, flags);
}

int fstat(int fd, struct stat *buf)
{
    if (AssistantMock::IsMockable()) {
        return Assistant::ins->fstat(fd, buf);
    }

    static int (*realFstat)(int, struct stat *) = []() {
        auto func = (int (*)(int, struct stat *))dlsym(RTLD_NEXT, "fstat");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real fstat: " << dlerror();
        }
        return func;
    }();

    if (!realFstat) {
        return -1;
    }

    return realFstat(fd, buf);
}

int ftruncate(int fd, off_t length)
{
    if (AssistantMock::IsMockable()) {
        return Assistant::ins->ftruncate(fd, length);
    }

    static int (*realFtruncate)(int, off_t) = []() {
        auto func = (int (*)(int, off_t))dlsym(RTLD_NEXT, "ftruncate");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real ftruncate: " << dlerror();
        }
        return func;
    }();

    if (!realFtruncate) {
        return -1;
    }

    return realFtruncate(fd, length);
}

int removexattr(const char *path, const char *name)
{
    if (AssistantMock::IsMockable()) {
        return Assistant::ins->removexattr(path, name);
    }

    static int (*realRemovexattr)(const char *, const char *) = []() {
        auto func = (int (*)(const char *, const char *))dlsym(RTLD_NEXT, "removexattr");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real removexattr: " << dlerror();
        }
        return func;
    }();

    if (!realRemovexattr) {
        return -1;
    }

    return realRemovexattr(path, name);
}

struct dirent* readdir(DIR* d)
{
    if (AssistantMock::IsMockable()) {
        return Assistant::ins->readdir(d);
    }

    static struct dirent* (*realReaddir)(DIR*) = []() {
        auto func = (struct dirent* (*)(DIR*))dlsym(RTLD_NEXT, "readdir");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real readdir: " << dlerror();
        }
        return func;
    }();

    if (!realReaddir) {
        return nullptr;
    }

    return realReaddir(d);
}

int lstat(const char *path, struct stat *buf)
{
    if (AssistantMock::IsMockable()) {
        return Assistant::ins->lstat(path, buf);
    }

    static int (*realLstat)(const char *, struct stat *) = []() {
        auto func = (int (*)(const char *, struct stat *))dlsym(RTLD_NEXT, "lstat");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real lstat: " << dlerror();
        }
        return func;
    }();

    if (!realLstat) {
        return -1;
    }

    return realLstat(path, buf);
}

} // extern "C"
#endif

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