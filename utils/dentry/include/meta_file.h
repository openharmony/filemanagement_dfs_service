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

#include <atomic>
#include <memory>
#include <mutex>
#include <map>
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
    int32_t HandleFileByFd(unsigned long &endBlock, uint32_t &level);
    int32_t DoRemove(const MetaBase &base);
    int32_t DoUpdate(const MetaBase &base);
    int32_t DoRename(const MetaBase &oldBase, const std::string &newName);
    int32_t DoLookup(MetaBase &base);
    int32_t LoadChildren(std::vector<MetaBase> &bases);

    static std::string GetParentDir(const std::string &path);
    static std::string GetFileName(const std::string &path);

private:
    std::mutex mtx_{};
    std::string path_{};
    std::string cacheFile_{};
    UniqueFd fd_{};
    uint64_t dentryCount_{0};
    std::shared_ptr<MetaFile> parentMetaFile_{nullptr};
};

enum {
    FILE_TYPE_CONTENT = 0,
    FILE_TYPE_THUMBNAIL,
    FILE_TYPE_LCD,
};

class MetaFileMgr {
public:
    static MetaFileMgr& GetInstance();
    /* recordId is hex string of 256 bits, convert to u8 cloudId[32] to kernel */
    static std::string RecordIdToCloudId(const std::string hexStr);
    static std::string CloudIdToRecordId(const std::string cloudId);
    std::shared_ptr<MetaFile> GetMetaFile(uint32_t userId, const std::string &path);
    void ClearAll();
private:
    MetaFileMgr() = default;
    ~MetaFileMgr() = default;
    MetaFileMgr(const MetaFileMgr &m) = delete;
    const MetaFileMgr &operator=(const MetaFileMgr &m) = delete;

    std::recursive_mutex mtx_{};
    std::map<std::pair<uint32_t, std::string>, std::shared_ptr<MetaFile>> metaFiles_;
};

struct MetaBase {
    MetaBase(const std::string &name) : name(name) {}
    MetaBase(const std::string &name, const std::string &cloudId) : name(name), cloudId(cloudId) {}
    MetaBase() = default;
    inline bool operator!=(const MetaBase &other) const
    {
        return !operator==(other);
    }
    inline bool operator==(const MetaBase &other) const
    {
        return other.cloudId == cloudId && other.name == name && other.size == size;
    }
    uint64_t mtime{0};
    uint64_t size{0};
    uint32_t mode{S_IFREG};
    uint32_t fileType{FILE_TYPE_CONTENT};
    std::string name{};
    std::string cloudId{};
};

struct BitOps {
    static const uint8_t BIT_PER_BYTE = 8;
    static int TestBit(uint32_t nr, const uint8_t addr[])
    {
        return 1 & (addr[nr / BIT_PER_BYTE] >> (nr & (BIT_PER_BYTE - 1)));
    }

    static void ClearBit(uint32_t nr, uint8_t addr[])
    {
        addr[nr / BIT_PER_BYTE] &= ~(1UL << ((nr) % BIT_PER_BYTE));
    }

    static void SetBit(uint32_t nr, uint8_t addr[])
    {
        addr[nr / BIT_PER_BYTE] |= (1UL << ((nr) % BIT_PER_BYTE));
    }

    static uint32_t FindNextBit(const uint8_t addr[], uint32_t maxSlots, uint32_t start)
    {
        while (start < maxSlots) {
            if (BitOps::TestBit(start, addr)) {
                return start;
            }
            start++;
        }
        return maxSlots;
    }

    static uint32_t FindNextZeroBit(const uint8_t addr[], uint32_t maxSlots, uint32_t start)
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
