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

#include "assistant.h"

#include <dlfcn.h>

namespace OHOS {
namespace FileManagement {
namespace CloudFile {
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

} // namespace CloudFile
} // namespace FileManagement
} // namespace OHOS

#ifdef __cplusplus
extern "C" {
using namespace OHOS::FileManagement::CloudFile;

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

int chmod(const char *pathname, mode_t mode)
{
    if (AssistantMock::IsMockable()) {
        return Assistant::ins->chmod(pathname, mode);
    }

    static int (*realChmod)() = []() {
        auto func = (int (*)())dlsym(RTLD_NEXT, "chmod");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real chmod: " << dlerror();
        }
        return func;
    }();

    if (!realChmod) {
        return -1;
    }

    return realChmod();
}
} // extern "C"
#endif
