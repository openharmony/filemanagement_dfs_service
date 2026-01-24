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

#include <dlfcn.h>
#include <fcntl.h>

namespace OHOS {
namespace FileManagement {
namespace CloudDisk {
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
using namespace OHOS::FileManagement::CloudDisk;

constexpr int32_t TEST_ERR = 2;

off_t lseek(int fd, off_t offset, int whence)
{
    if (AssistantMock::IsMockable()) {
        return Assistant::ins->lseek(fd, offset, whence);
    }

    static off_t (*realLseek)() = []() {
        auto func = (off_t (*)())dlsym(RTLD_NEXT, "lseek");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real lseek: " << dlerror();
        }
        return func;
    }();

    if (!realLseek) {
        return -1;
    }

    return realLseek();
}

int access(const char *name, int type)
{
    if (AssistantMock::IsMockable()) {
        return Assistant::ins->access(name, type);
    }

    static int (*realAccess)() = []() {
        auto func = (int (*)())dlsym(RTLD_NEXT, "access");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real access: " << dlerror();
        }
        return func;
    }();

    if (!realAccess) {
        return -1;
    }

    return realAccess();
}

int close(int fd)
{
    if (AssistantMock::IsMockable()) {
        return Assistant::ins->close(fd);
    }

    static int (*realClose)() = []() {
        auto func = (int (*)())dlsym(RTLD_NEXT, "close");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real close: " << dlerror();
        }
        return func;
    }();

    if (!realClose) {
        return -1;
    }

    return realClose();
}

int fstat(int fd, struct stat *buf)
{
    if (AssistantMock::IsMockable()) {
        errno = TEST_ERR;
        return Assistant::ins->fstat(fd, buf);
    }

    static int (*realFstat)() = []() {
        auto func = (int (*)())dlsym(RTLD_NEXT, "fstat");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real fstat: " << dlerror();
        }
        return func;
    }();

    if (!realFstat) {
        return -1;
    }

    return realFstat();
}

int fcntl(int fd, int op, ...)
{
    if (AssistantMock::IsMockable()) {
        errno = TEST_ERR;
        return Assistant::ins->fcntl(fd, op);
    }

    static int (*realFcntl)() = []() {
        auto func = (int (*)())dlsym(RTLD_NEXT, "fcntl");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real fcntl: " << dlerror();
        }
        return func;
    }();

    if (!realFcntl) {
        return -1;
    }

    return realFcntl();
}

int ftruncate(int fd, off_t length)
{
    if (AssistantMock::IsMockable()) {
        return Assistant::ins->ftruncate(fd, length);
    }

    static int (*realFtruncate)() = []() {
        auto func = (int (*)())dlsym(RTLD_NEXT, "ftruncate");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real ftruncate: " << dlerror();
        }
        return func;
    }();

    if (!realFtruncate) {
        return -1;
    }

    return realFtruncate();
}

int MyOpen(const char *file, int oflag, mode_t mode)
{
    if (AssistantMock::IsMockable()) {
        return Assistant::ins->MyOpen(file, oflag, mode);
    }

    return 0;
}

ssize_t MyPread(int fd, void *buf, size_t count, off_t offset)
{
    if (AssistantMock::IsMockable()) {
        return Assistant::ins->MyPread(fd, buf, count, offset);
    }

    static int (*realpread)() = []() {
        auto func = (int (*)())dlsym(RTLD_NEXT, "pread");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real pread: " << dlerror();
        }
        return func;
    }();

    if (!realpread) {
        return -1;
    }

    return realpread();
}

} // extern "C"
#endif