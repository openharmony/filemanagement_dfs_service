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

#include "library_func_mock.h"

using namespace OHOS::Storage::DistributedFile;
void *dlopen(const char *file, int mode)
{
    if (LibraryFunc::libraryFunc_ == nullptr) {
        return nullptr;
    }
    return LibraryFunc::libraryFunc_->dlopen(file, mode);
}

int dlclose(void *handle)
{
    if (LibraryFunc::libraryFunc_ == nullptr) {
        return -1;
    }
    return LibraryFunc::libraryFunc_->dlclose(handle);
}

void *dlsym(void *__restrict handle, const char *__restrict name)
{
    if (LibraryFunc::libraryFunc_ == nullptr) {
        return nullptr;
    }
    return LibraryFunc::libraryFunc_->dlsym(handle, name);
}

char *realpath(const char *__restrict path, char *__restrict resolved_path)
{
    if (LibraryFunc::libraryFunc_ == nullptr) {
        return nullptr;
    }
    return LibraryFunc::libraryFunc_->realpath(path, resolved_path);
}

