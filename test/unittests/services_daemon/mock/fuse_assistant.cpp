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

#include "fuse_assistant.h"

#include <dlfcn.h>

namespace OHOS {
namespace FileManagement {
namespace CloudFile {

void FuseAssistantMock::EnableMock()
{
    FuseAssistant::mockable = true;
}

void FuseAssistantMock::DisableMock()
{
    ins = nullptr;
    FuseAssistant::mockable = false;
}

bool FuseAssistantMock::IsMockable()
{
    return FuseAssistant::mockable;
}

} // namespace CloudFile
} // namespace FileManagement
} // namespace OHOS

#ifdef __cplusplus
extern "C" {
using namespace OHOS::FileManagement::CloudFile;

int mkdir(const char *path, mode_t mode)
{
    if (FuseAssistantMock::IsMockable()) {
        return FuseAssistant::ins->mkdir(path, mode);
    }

    static int (*realMkdir)() = []() {
        auto func = (int (*)())dlsym(RTLD_NEXT, "mkdir");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real mkdir: " << dlerror();
        }
        return func;
    }();

    if (!realMkdir) {
        return -1;
    }

    return realMkdir();
}

} // extern "C"
#endif