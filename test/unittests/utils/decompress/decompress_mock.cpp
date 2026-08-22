/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "decompress_mock.h"

#include <cstdarg>
#include <cerrno>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dlfcn.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

namespace DecompressMock {

State g;

void Reset()
{
    g = State();
}

} // namespace DecompressMock

extern "C" {
int __real_open(const char *pathname, int flags, ...);

int __wrap_open(const char *pathname, int flags, ...)
{
    auto& s = DecompressMock::g;
    if (s.openEnabled) {
        s.openCallCount++;
        if (s.openFailAfterCount >= 0 && s.openCallCount > s.openFailAfterCount) {
            errno = s.openErrno;
            return -1;
        }
        if (s.openReturnFd >= 0) {
            return s.openReturnFd;
        }
        errno = s.openErrno;
        return -1;
    }
    mode_t mode = 0;
    if (flags & (O_CREAT | O_TMPFILE)) {
        va_list ap;
        va_start(ap, flags);
        mode = va_arg(ap, int);
        va_end(ap);
    }
    return __real_open(pathname, flags, mode);
}

int __real_close(int fd);

int __wrap_close(int fd)
{
    auto& s = DecompressMock::g;
    if (s.closeEnabled) {
        return s.closeReturnVal;
    }
    return __real_close(fd);
}

int __real_ioctl(int fd, unsigned long request, ...);

int __wrap_ioctl(int fd, unsigned long request, ...)
{
    auto& s = DecompressMock::g;
    if (s.ioctlEnabled) {
        s.ioctlCallCount++;
        if (s.ioctlSuccessAfterCount >= 0 && s.ioctlCallCount > s.ioctlSuccessAfterCount) {
            return 0;
        }
        if (s.ioctlRetryErrno != 0 && s.ioctlCallCount <= 2) {
            errno = s.ioctlRetryErrno;
        } else {
            errno = s.ioctlErrno;
        }
        return s.ioctlReturnVal;
    }
    va_list ap;
    va_start(ap, request);
    void *arg = va_arg(ap, void *);
    va_end(ap);
    return __real_ioctl(fd, request, arg);
}

char *__real_realpath(const char *path, char *resolved);

char* __wrap_realpath(const char *path, char *resolved)
{
    auto& s = DecompressMock::g;
    if (s.realpathEnabled) {
        if (s.realpathFail) {
            errno = ENOENT;
            return nullptr;
        }
        if (resolved) {
            strncpy(resolved, path, PATH_MAX - 1);
            resolved[PATH_MAX - 1] = '\0';
        }
        return resolved;
    }

    return __real_realpath(path, resolved);
}

void *__real_dlopen(const char *filename, int flags);

void* __wrap_dlopen(const char *filename, int flags)
{
    auto& s = DecompressMock::g;
    if (s.dlopenEnabled) {
        return s.dlopenSucceed ? s.dlopenHandle : nullptr;
    }
    return __real_dlopen(filename, flags);
}

void *__real_dlsym(void *handle, const char *symbol);

void* __wrap_dlsym(void *handle, const char *symbol)
{
    auto& s = DecompressMock::g;
    if (s.dlsymEnabled) {
        if (!s.dlsymSucceed) {
            return nullptr;
        }
        if (strcmp(symbol, "CloudSync_GetDecompressUnsupportedList") == 0) {
            static int32_t (*fn)(std::vector<std::string> *) =
                +[](std::vector<std::string> *list) -> int32_t {
                if (list) {
                    *list = DecompressMock::g.unsupportedList;
                }
                return DecompressMock::g.unsupportedListRet;
            };
            return reinterpret_cast<void *>(fn);
        }
        if (strcmp(symbol, "CloudSync_GetDecompressSystemFeature") == 0) {
            static int32_t (*fn)(bool *) =
                +[](bool *feature) -> int32_t {
                if (feature) {
                    *feature = DecompressMock::g.systemFeatureValue;
                }
                return DecompressMock::g.systemFeatureRet;
            };
            return reinterpret_cast<void *>(fn);
        }
        return nullptr;
    }
    return __real_dlsym(handle, symbol);
}

FILE *__real_fopen(const char *pathname, const char *mode);

FILE* __wrap_fopen(const char *pathname, const char *mode)
{
    auto& s = DecompressMock::g;
    if (s.fopenEnabled) {
        return s.fopenSucceed ? s.fopenHandle : nullptr;
    }
    return __real_fopen(pathname, mode);
}

int __real_fclose(FILE *stream);

int __wrap_fclose(FILE *stream)
{
    auto& s = DecompressMock::g;
    if (s.fopenEnabled) {
        return 0;
    }
    return __real_fclose(stream);
}

ssize_t __real_getline(char **lineptr, size_t *n, FILE *stream);

ssize_t __wrap_getline(char **lineptr, size_t *n, FILE *stream)
{
    auto& s = DecompressMock::g;
    if (s.getlineEnabled) {
        if (s.getlineSucceed) {
            if (lineptr) {
                *lineptr = strdup(s.getlineContent);
            }
            if (n) {
                *n = strlen(s.getlineContent) + 1;
            }
            return s.getlineRetVal;
        }
        return -1;
    }
    return __real_getline(lineptr, n, stream);
}

} // extern "C"