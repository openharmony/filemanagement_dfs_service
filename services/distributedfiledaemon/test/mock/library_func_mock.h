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

#ifndef FILEMANAGEMENT_DFS_SERVICE_LIBRARY_FUNC_MOCK_H
#define FILEMANAGEMENT_DFS_SERVICE_LIBRARY_FUNC_MOCK_H

#include <cstdlib>
#include <dlfcn.h>
#include <memory>

#include "gmock/gmock.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
class LibraryFunc {
public:
    virtual ~LibraryFunc() = default;

    virtual void *dlopen(const char *file, int mode) = 0;
    virtual int dlclose(void *handle) = 0;
    virtual void *dlsym(void *__restrict handle, const char *__restrict name) = 0;
    virtual char *realpath(const char *__restrict path, char *__restrict resolved_path) = 0;

public:
    static inline std::shared_ptr<LibraryFunc> libraryFunc_ = nullptr;
};

class LibraryFuncMock : public LibraryFunc {
public:
    MOCK_METHOD2(dlopen, void *(const char *file, int mode));
    MOCK_METHOD1(dlclose, int(void *handle));
    MOCK_METHOD2(dlsym, void *(void *__restrict handle, const char *__restrict name));
    MOCK_METHOD2(realpath, char *(const char *__restrict path, char *__restrict resolved_path));
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS

#endif // FILEMANAGEMENT_DFS_SERVICE_LIBRARY_FUNC_MOCK_H
