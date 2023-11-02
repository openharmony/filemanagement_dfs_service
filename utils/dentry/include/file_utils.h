/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_FILEMGMT_DENTRY_FILE_UTILS_H
#define OHOS_FILEMGMT_DENTRY_FILE_UTILS_H

#include <string>
#include <sys/stat.h>

namespace OHOS {
namespace FileManagement {
class FileUtils {
public:
    FileUtils() = delete;
    ~FileUtils() = delete;

    static int64_t ReadFile(int fd, off_t offset, size_t size, void *data);
    static int64_t WriteFile(int fd, const void *data, off_t offset, size_t size);
};

class FileRangeLock {
public:
    FileRangeLock(int fd, off_t offset, size_t size);
    ~FileRangeLock();

private:
    int fd_{-1};
    off_t offset_{0};
    size_t size_{0};
    bool lockFailed_{false};
    int xcollieId_{0};
};
} // namespace FileManagement
} // namespace OHOS

#endif
