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
#include <mutex>
#include <string>
#include <vector>
#include <sys/stat.h>

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
    int32_t DoRename(const MetaBase &oldBase, const std::string &newName);
    int32_t DoLookup(MetaBase &base);

private:
    std::mutex mtx_{};
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
    uint32_t mode{S_IFREG};
    std::string name{};
    std::string cloudId{};

    int refcnt{0};
    std::mutex refMtx;
};

struct BitOps {
    static const int BIT_PER_BYTE = 8;
    static int TestBit(int nr, const uint8_t addr[])
    {
        return 1 & (addr[nr / BIT_PER_BYTE] >> (nr & (BIT_PER_BYTE - 1)));
    }

    static void ClearBit(int nr, uint8_t addr[])
    {
        addr[nr / BIT_PER_BYTE] &= ~(1UL << ((nr) % BIT_PER_BYTE));
    }

    static void SetBit(int nr, uint8_t addr[])
    {
        addr[nr / BIT_PER_BYTE] |= (1UL << ((nr) % BIT_PER_BYTE));
    }

    static int FindNextBit(const uint8_t addr[], int maxSlots, int start)
    {
        while (start < maxSlots) {
            if (BitOps::TestBit(start, addr)) {
                return start;
            }
            start++;
        }
        return maxSlots;
    }

    static int FindNextZeroBit(const uint8_t addr[], int maxSlots, int start)
    {
        while (start < maxSlots) {
            if (!BitOps::TestBit(start, addr)) {
                return start;
            }
            start++;
        }
        return maxSlots;
    }
};

} // namespace FileManagement
} // namespace OHOS

#endif // META_FILE_H
