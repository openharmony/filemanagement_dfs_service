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

#ifndef DISTRIBUTED_FILE_SIZE_UTILS_H
#define DISTRIBUTED_FILE_SIZE_UTILS_H

#include <dirent.h>
#include <memory>
#include <string>


namespace OHOS {
namespace Storage {
namespace DistributedFile {

struct NameList {
    struct dirent **namelist = { nullptr };
    int direntNum = 0;
};

class FileSizeUtils {
public:
    static int32_t GetSize(const std::string &uri, bool isSrcUri, uint64_t &size);
    static int32_t IsDirectory(const std::string &uri, bool isSrcUri, bool &isDirectory);

    static std::string GetPathFromUri(const std::string &uri, bool isSrcUri);
    static int32_t GetFileSize(const std::string &path, uint64_t &size);
    static int32_t GetDirSize(const std::string &path, uint64_t &size);
    static void Deleter(struct NameList *arg);
    static std::unique_ptr<struct NameList, decltype(FileSizeUtils::Deleter) *> GetDirNameList(const std::string &path);

private:
    static int FilterFunc(const struct dirent *filename);
    static std::string GetRealPath(const std::string &path);
    static int32_t IsFile(const std::string &path, bool &result);
    static int32_t IsDirectory(const std::string &path, bool &result);
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // DISTRIBUTED_FILE_SIZE_UTILS_H
