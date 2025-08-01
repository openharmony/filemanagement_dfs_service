/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <sys/stat.h>
#include <vector>

#include "unique_fd.h"

namespace OHOS {
namespace FileManagement {

const std::string RECYCLE_NAME = ".trash";
const std::string RECYCLE_CLOUD_ID = ".trash";
const std::string ROOT_CLOUD_ID = "rootId";
const unsigned int STAT_MODE_DIR = 0771;
constexpr int32_t LOCAL = 1;
constexpr uint32_t MAX_META_FILE_NUM = 100;
constexpr uint32_t MAX_CLOUDDISK_META_FILE_NUM = 150;

struct MetaBase;
struct RestoreInfo;
class MetaFile {
public:
    MetaFile() = delete;
    ~MetaFile();
    using CloudDiskMetaFileCallBack = std::function<void(MetaBase &)>;
    explicit MetaFile(uint32_t userId, const std::string &path);
    explicit MetaFile(uint32_t userId, const std::string &bundleName, const std::string &parentCloudId);
    int32_t DoLookupAndUpdate(const std::string &name, CloudDiskMetaFileCallBack updateFunc);
    int32_t DoLookupAndRemove(MetaBase &metaBase);
    int32_t DoCreate(const MetaBase &base);
    int32_t HandleFileByFd(unsigned long &endBlock, uint32_t &level);
    int32_t DoRemove(const MetaBase &base);
    int32_t DoUpdate(const MetaBase &base);
    int32_t DoRename(const MetaBase &oldBase, const std::string &newName);
    int32_t DoRename(MetaBase &metaBase, const std::string &newName, std::shared_ptr<MetaFile> newMetaFile);
    int32_t DoLookup(MetaBase &base);
    int32_t LoadChildren(std::vector<MetaBase> &bases);

    static std::string GetParentDir(const std::string &path);
    static std::string GetFileName(const std::string &path);

private:
    std::mutex mtx_{};
    std::string path_{};
    std::string cacheFile_{};
    std::string bundleName_{};
    std::string cloudId_{};
    std::string name_{};
    UniqueFd fd_{};
    uint32_t userId_{};
    std::shared_ptr<MetaFile> parentMetaFile_{nullptr};
};

class CloudDiskMetaFile {
public:
    CloudDiskMetaFile() = delete;
    ~CloudDiskMetaFile();
    using CloudDiskMetaFileCallBack = std::function<void(MetaBase &)>;
    explicit CloudDiskMetaFile(uint32_t userId, const std::string &bundleName, const std::string &cloudId);

    int32_t DoLookupAndCreate(const std::string &name, CloudDiskMetaFileCallBack metaFileCallBack);
    int32_t DoLookupAndUpdate(const std::string &name, CloudDiskMetaFileCallBack updateFunc);
    int32_t DoChildUpdate(const std::string &name, CloudDiskMetaFileCallBack updateFunc);
    int32_t DoLookupAndRemove(MetaBase &metaBase);
    int32_t DoCreate(const MetaBase &base);
    int32_t HandleFileByFd(unsigned long &endBlock, uint32_t &level);
    int32_t DoRemove(const MetaBase &base);
    int32_t DoUpdate(const MetaBase &base);
    int32_t DoRename(MetaBase &metaBase, const std::string &newName,
        std::shared_ptr<CloudDiskMetaFile> newMetaFile);
    int32_t DoLookup(MetaBase &base);
    int32_t LoadChildren(std::vector<MetaBase> &bases);
    std::string GetDentryFilePath();

private:
    int32_t GetCreateInfo(const MetaBase &base, uint32_t &bitPos, uint32_t &namehash,
        unsigned long &bidx, struct HmdfsDentryGroup &dentryBlk);
    std::mutex mtx_{};
    std::string path_{};
    std::string cacheFile_{};
    std::string bundleName_{};
    std::string cloudId_{};
    std::string name_{};
    UniqueFd fd_{};
    uint32_t userId_{};
    std::shared_ptr<MetaFile> parentMetaFile_{nullptr};
};

enum {
    NEED_UPLOAD = 0,
    NO_UPLOAD,
};

enum {
    FILE_TYPE_CONTENT = 0,
    FILE_TYPE_THUMBNAIL,
    FILE_TYPE_LCD,
};

enum {
    POSITION_UNKNOWN = 0,
    POSITION_LOCAL = 0x01,
    POSITION_CLOUD = 0x02,
    POSITION_LOCAL_AND_CLOUD = POSITION_LOCAL | POSITION_CLOUD,
};

typedef std::pair<uint32_t, std::string> MetaFileKey;
typedef std::pair<MetaFileKey, std::shared_ptr<CloudDiskMetaFile>> CloudDiskMetaFileListEle;
typedef std::pair<MetaFileKey, std::shared_ptr<MetaFile>> MetaFileListEle;

class MetaFileMgr {
public:
    static MetaFileMgr& GetInstance();
    /* recordId is hex string of 256 bits, convert to u8 cloudId[32] to kernel */
    static std::string RecordIdToCloudId(const std::string hexStr, bool isHdc = false);
    static std::string CloudIdToRecordId(const std::string cloudId, bool isHdc = false);
    std::shared_ptr<MetaFile> GetMetaFile(uint32_t userId, const std::string &path);
    std::shared_ptr<CloudDiskMetaFile> GetCloudDiskMetaFile(uint32_t userId, const std::string &bundleName,
        const std::string &cloudId);
    void ClearAll();
    void CloudDiskClearAll();
    void Clear(uint32_t userId, const std::string &bundleName, const std::string &cloudId);
    int32_t CreateRecycleDentry(uint32_t userId, const std::string &bundleName);
    int32_t MoveIntoRecycleDentryfile(uint32_t userId, const std::string &bundleName,
        const struct RestoreInfo &restoreInfo);
    int32_t RemoveFromRecycleDentryfile(uint32_t userId, const std::string &bundleName,
        const struct RestoreInfo &restoreinfo);
    int32_t GetNewName(std::shared_ptr<CloudDiskMetaFile> metaFile,
        const std::string &oldName, std::string &newName);
    int32_t CheckMetaFileSize();
private:
    MetaFileMgr() = default;
    ~MetaFileMgr() = default;
    MetaFileMgr(const MetaFileMgr &m) = delete;
    const MetaFileMgr &operator=(const MetaFileMgr &m) = delete;

    std::recursive_mutex mtx_{};
    std::mutex cloudDiskMutex_{};
    std::list<MetaFileListEle> metaFileList_;
    std::map<MetaFileKey, std::list<MetaFileListEle>::iterator> metaFiles_;
    std::list<CloudDiskMetaFileListEle> cloudDiskMetaFileList_;
    std::map<MetaFileKey, std::list<CloudDiskMetaFileListEle>::iterator> cloudDiskMetaFile_;
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
    uint64_t atime{0};
    uint64_t mtime{0};
    uint64_t size{0};
    uint32_t mode{S_IFREG};
    uint8_t position{POSITION_LOCAL};
    uint8_t fileType{FILE_TYPE_CONTENT};
    uint8_t noUpload{NEED_UPLOAD};
    std::string name{};
    std::string cloudId{};
    off_t nextOff{0};
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

struct MetaHelper {
    static void SetFileType(struct HmdfsDentry *de, uint8_t fileType);
    static void SetPosition(struct HmdfsDentry *de, uint8_t position);
    static void SetNoUpload(struct HmdfsDentry *de, uint8_t noUpload);
    static uint8_t GetFileType(const struct HmdfsDentry *de);
    static uint8_t GetPosition(const struct HmdfsDentry *de);
    static uint8_t GetNoUpload(const struct HmdfsDentry *de);
};

struct RestoreInfo {
    std::string oldName;
    std::string parentCloudId;
    std::string newName;
    int64_t rowId = 0;
};
} // namespace FileManagement
} // namespace OHOS

#endif // META_FILE_H
