/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include "rdb_assistant.h"

#include <dlfcn.h>

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

} // extern "C"
#endif