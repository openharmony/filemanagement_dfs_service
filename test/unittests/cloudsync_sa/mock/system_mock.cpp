/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "system_mock.h"

#include <cerrno>
#include <dlfcn.h>

namespace OHOS::FileManagement::CloudSync {
void SystemMock::EnableMock()
{
    System::mockable = true;
    System::exited = false;
}

void SystemMock::DisableMock()
{
    ins = nullptr;
    System::exited = true;
}

bool SystemMock::IsMockable()
{
    return System::mockable;
}
} // namespace OHOS::FileManagement::CloudSync

extern "C" {
using namespace OHOS::FileManagement::CloudSync;

using RealpathFunc = char *(*)(const char *, char *);
using GetxattrFunc = int (*)(const char *, const char *, void *, size_t);

static RealpathFunc realRealpath = nullptr;
static GetxattrFunc realGetxattr = nullptr;

static void EnsureRealFuncsInitialized()
{
    if (!realRealpath) {
        realRealpath = reinterpret_cast<RealpathFunc>(dlsym(RTLD_NEXT, "realpath"));
        if (!realRealpath) {
            GTEST_LOG_(ERROR) << "Failed to resolve realpath: " << dlerror();
        }
    }
    if (!realGetxattr) {
        realGetxattr = reinterpret_cast<GetxattrFunc>(dlsym(RTLD_NEXT, "getxattr"));
        if (!realGetxattr) {
            GTEST_LOG_(ERROR) << "Failed to resolve getxattr: " << dlerror();
        }
    }
}

char *realpath(const char *path, char *resolved_path)
{
    if (System::exited || System::ins == nullptr) {
        errno = EFAULT;
        return nullptr;
    }

    if (SystemMock::IsMockable()) {
        return System::ins->realpath(path, resolved_path);
    }

    EnsureRealFuncsInitialized();
    if (!realRealpath) {
        errno = EFAULT;
        return nullptr;
    }

    return realRealpath(path, resolved_path);
}

int getxattr(const char *path, const char *name, void *value, size_t size)
{
    if (System::exited || System::ins == nullptr) {
        errno = EFAULT;
        return -1;
    }

    if (SystemMock::IsMockable()) {
        return System::ins->getxattr(path, name, value, size);
    }

    EnsureRealFuncsInitialized();
    if (!realGetxattr) {
        errno = EFAULT;
        return -1;
    }

    return realGetxattr(path, name, value, size);
}
}