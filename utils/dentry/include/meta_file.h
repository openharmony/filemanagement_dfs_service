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

#ifndef OHOS_FILEMGMT_DENTRY_META_FILE_H
#define OHOS_FILEMGMT_DENTRY_META_FILE_H

#include <memory>
#include <shared_mutex>
#include <string>
#include <vector>

#include "unique_fd.h"

namespace OHOS {
namespace FileManagement {

struct MetaBase;
class MetaFile {
public:
    MetaFile() = delete;
    ~MetaFile();
    explicit MetaFile(uint32_t userId, const std::string &path);

    int32_t DoCreate(const MetaBase &base);
    int32_t DoRemove(const MetaBase &base);
    int32_t DoUpdate(const MetaBase &base);
    int32_t DoRename(MetaBase &oldbase, MetaBase &newbase);
    int32_t DoLookup(MetaBase &base);

private:
    mutable std::shared_mutex mtx_{};
    std::string path_{};
    std::string cacheFile_{};
    UniqueFd fd_{};
    uint64_t dentryCount_{0};
    std::shared_ptr<MetaFile> parentMetaFile_{nullptr};
};

struct MetaBase {
    MetaBase(const std::string &name) : name(name) {}
    MetaBase(const std::string &name, const std::string &cloudId) : name(name), cloudId(cloudId) {}
    MetaBase() = default;
    uint64_t mtime{0};
    uint64_t size{0};
    uint32_t mode{0};
    std::string name{};
    std::string cloudId{};

    int refcnt{0};
    std::mutex refMtx;
};
} // namespace FileManagement
} // namespace OHOS

#endif // META_FILE_H
