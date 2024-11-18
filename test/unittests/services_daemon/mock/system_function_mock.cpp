/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "file_operations_base.h"
#include "fuse_assistant.h"
#include "securec.h"

using namespace OHOS::FileManagement::CloudFile;

int mkdir(const char *path, mode_t mode)
{
    return FuseAssistant::ins->mkdir(path, mode);
}

int chown(const char *path, uid_t owner, gid_t group)
{
    return FuseAssistant::ins->chown(path, owner, group);
}

int chmod(const char *path, mode_t mode)
{
    return FuseAssistant::ins->chmod(path, mode);
}

int AccessMock(const char *name, int type)
{
    if (name == nullptr) {
        return 0;
    }
    const char *dentryPath = "/data/service/el2/100/hmdfs/cache/account_cache/dentry_cache/cloud";
    if (strcmp(name, dentryPath) == 0) {
        return -1;
    }
    return 0;
}

char* StrdupMock(const char *path)
{
    const char *str1 = "/mnt/data/234/cloud";
    const char *str2 = "/mnt/data/234/cloud_fuse";
    if (path == nullptr) {
        return nullptr;
    }
    if (strcmp(path, str1) == 0 || strcmp(path, str2) == 0) {
        return nullptr;
    }
    size_t len = strlen(path);
    if (len <= 0) {
        return nullptr;
    }
    char* dstStr = static_cast<char*>(malloc(len + 1));
    if (dstStr == nullptr) {
        return nullptr;
    }
    if (memcpy_s(dstStr, len + 1, path, len + 1) != 0) {
        free(dstStr);
        dstStr = nullptr;
        return nullptr;
    }
    return dstStr;
}