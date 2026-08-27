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

#ifdef DECOMPRESS_UNIT_TEST
#error "decompress_mock.cpp must not be compiled with DECOMPRESS_UNIT_TEST defined"
#endif

#include <cstdarg>
#include <cerrno>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dlfcn.h>
#include <fcntl.h>
#include <securec.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>

namespace {
constexpr int IOCTL_MAX_RETRY_COUNT = 2;
}

namespace DecompressMock {

State g_mockState;

void Reset()
{
    g_mockState = State();
}

} // namespace DecompressMock

int MockOpen(const char *pathname, int flags, ...)
{
    auto& s = DecompressMock::g_mockState;
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
    return open(pathname, flags, mode);
}

int MockClose(int fd)
{
    auto& s = DecompressMock::g_mockState;
    if (s.closeEnabled) {
        return s.closeReturnVal;
    }
    return close(fd);
}

int MockIoctl(int fd, unsigned long request, ...)
{
    auto& s = DecompressMock::g_mockState;
    if (s.ioctlEnabled) {
        s.ioctlCallCount++;
        if (s.ioctlSuccessAfterCount >= 0 && s.ioctlCallCount > s.ioctlSuccessAfterCount) {
            return 0;
        }
        if (s.ioctlRetryErrno != 0 && s.ioctlCallCount <= IOCTL_MAX_RETRY_COUNT) {
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
    return ioctl(fd, request, arg);
}

char *MockRealpath(const char *path, char *resolved)
{
    auto& s = DecompressMock::g_mockState;
    if (s.realpathEnabled) {
        if (s.realpathFail) {
            errno = ENOENT;
            return nullptr;
        }
        if (resolved) {
            errno_t ret = strncpy_s(resolved, PATH_MAX, path, PATH_MAX - 1);
            if (ret != EOK) {
                return nullptr;
            }
        }
        return resolved;
    }

    return realpath(path, resolved);
}

void *MockDlopen(const char *filename, int flags)
{
    auto& s = DecompressMock::g_mockState;
    if (s.dlopenEnabled) {
        return s.dlopenSucceed ? s.dlopenHandle : nullptr;
    }
    return dlopen(filename, flags);
}

void *MockDlsym(void *handle, const char *symbol)
{
    auto& s = DecompressMock::g_mockState;
    if (s.dlsymEnabled) {
        if (!s.dlsymSucceed) {
            return nullptr;
        }
        if (strcmp(symbol, "CloudSync_GetDecompressUnsupportedList") == 0) {
            static int32_t (*fn)(std::vector<std::string> *) =
                +[](std::vector<std::string> *list) -> int32_t {
                if (list) {
                    *list = DecompressMock::g_mockState.unsupportedList;
                }
                return DecompressMock::g_mockState.unsupportedListRet;
            };
            return reinterpret_cast<void *>(fn);
        }
        if (strcmp(symbol, "CloudSync_GetDecompressSystemFeature") == 0) {
            static int32_t (*fn)(bool *) =
                +[](bool *feature) -> int32_t {
                if (feature) {
                    *feature = DecompressMock::g_mockState.systemFeatureValue;
                }
                return DecompressMock::g_mockState.systemFeatureRet;
            };
            return reinterpret_cast<void *>(fn);
        }
        return nullptr;
    }
    return dlsym(handle, symbol);
}

int MockStat(const char *pathname, struct stat *statbuf)
{
    auto& s = DecompressMock::g_mockState;
    if (s.statEnabled) {
        s.statCallCount++;
        if (s.statFail) {
            errno = ENOENT;
            return -1;
        }
        if (statbuf) {
            errno_t ret = memset_s(statbuf, sizeof(struct stat), 0, sizeof(struct stat));
            if (ret != EOK) {
                return -1;
            }
            statbuf->st_dev = (s.statCallCount == 1) ? s.statHapDev : s.statDstDev;
        }
        return 0;
    }
    return stat(pathname, statbuf);
}

FILE *MockFopen(const char *pathname, const char *mode)
{
    auto& s = DecompressMock::g_mockState;
    if (s.fopenEnabled) {
        return s.fopenSucceed ? s.fopenHandle : nullptr;
    }
    return fopen(pathname, mode);
}

int MockFclose(FILE *stream)
{
    auto& s = DecompressMock::g_mockState;
    if (s.fopenEnabled) {
        return 0;
    }
    return fclose(stream);
}

ssize_t MockGetline(char **lineptr, size_t *n, FILE *stream)
{
    auto& s = DecompressMock::g_mockState;
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
    return getline(lineptr, n, stream);
}
