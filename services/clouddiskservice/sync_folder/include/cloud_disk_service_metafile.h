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

#ifndef CLOUD_DISK_SERVICE_METAFILE_H
#define CLOUD_DISK_SERVICE_METAFILE_H

#include <fcntl.h>
#include <list>
#include <map>
#include <sstream>
#include <sys/stat.h>

#include "unique_fd.h"

namespace OHOS::FileManagement::CloudDiskService {

constexpr uint8_t VALIDATE = 0x01;
constexpr uint8_t INVALIDATE = 0x02;
constexpr uint32_t DENTRYGROUP_SIZE = 4096;
constexpr uint32_t DENTRY_NAME_LEN = 16;
constexpr uint32_t DENTRY_RESERVED_LENGTH = 3;
constexpr uint32_t DENTRY_PER_GROUP = 60;
constexpr uint32_t DENTRY_BITMAP_LENGTH = 8;
constexpr uint32_t DENTRY_GROUP_RESERVED = 7;
constexpr uint32_t DENTRYGROUP_HEADER = 4096;
constexpr uint32_t MAX_BUCKET_LEVEL = 63;
constexpr uint32_t BUCKET_BLOCKS = 2;
constexpr uint32_t BYTE_PER_SLOT = 16;
constexpr uint32_t HMDFS_SLOT_LEN_BITS = 4;
constexpr uint32_t RECORD_ID_LEN = 16;
constexpr uint32_t DENTRYFILE_NAME_LEN = 16;
const std::string ROOT_PARENTDENTRYFILE = std::string(DENTRYFILE_NAME_LEN, '.');

#pragma pack(push, 1)
struct CloudDiskServiceDentry {
    uint8_t revalidate;
    uint8_t recordId[RECORD_ID_LEN];
    uint32_t hash;
    uint16_t mode;
    uint16_t namelen;
    uint64_t size;
    uint64_t mtime;
    uint64_t atime;
    /* reserved bytes for long term extend, total 52 bytes */
    uint8_t reserved[DENTRY_RESERVED_LENGTH];
};

struct CloudDiskServiceDentryGroup {
    uint8_t dentryVersion;
    uint8_t bitmap[DENTRY_BITMAP_LENGTH];
    struct CloudDiskServiceDentry nsl[DENTRY_PER_GROUP];
    uint8_t fileName[DENTRY_PER_GROUP][DENTRY_NAME_LEN];
    uint8_t reserved[DENTRY_GROUP_RESERVED];
};
static_assert(sizeof(CloudDiskServiceDentryGroup) == DENTRYGROUP_SIZE);

struct CloudDiskServiceDcacheHeader {
    uint8_t parentDentryfile[DENTRYFILE_NAME_LEN];
    uint8_t selfRecordId[RECORD_ID_LEN];
    uint32_t selfHash;
};
#pragma pack(pop)

struct MetaBase {
    MetaBase(const std::string &name) : name(name) {}
    MetaBase(const std::string &recordId, const uint32_t hash) : hash(hash), recordId(recordId) {}
    MetaBase(const std::string &name, const std::string &recordId) : name(name), recordId(recordId) {}
    MetaBase() = default;

    uint32_t mode{0};
    uint32_t hash{0};
    uint64_t atime{0};
    uint64_t mtime{0};
    uint64_t size{0};
    std::string name{};
    std::string recordId{};
};

class CloudDiskServiceMetaFile {
public:
    CloudDiskServiceMetaFile() = delete;
    ~CloudDiskServiceMetaFile() = default;
    using CloudDiskServiceMetaFileCallBack = std::function<void(MetaBase &)>;
    explicit CloudDiskServiceMetaFile(const int32_t userId, const uint32_t syncFolderIndex, const uint64_t inode);

    int32_t DoCreate(const MetaBase &base, unsigned long &bidx, uint32_t &bitPos);
    int32_t DoRemove(const MetaBase &base, std::string &recordId, unsigned long &bidx, uint32_t &bitPos);
    int32_t DoFlush(const MetaBase &base, std::string &recordId, unsigned long &bidx, uint32_t &bitPos);
    int32_t DoUpdate(const MetaBase &base, std::string &recordId, unsigned long &bidx, uint32_t &bitPos);
    int32_t DoRenameOld(const MetaBase &base, std::string &recordId, unsigned long &bidx, uint32_t &bitPos);
    int32_t DoRenameNew(const MetaBase &base, std::string &recordId, unsigned long &bidx, uint32_t &bitPos);
    int32_t DoRename(MetaBase &metaBase, const std::string &newName,
        std::shared_ptr<CloudDiskServiceMetaFile> newMetaFile);
    int32_t DoLookupByName(MetaBase &base);
    int32_t DoLookupByRecordId(MetaBase &base, uint8_t revalidate);
    int32_t DoLookupByOffset(MetaBase &base, const unsigned long bidx, const uint32_t bitPos);

    int32_t DecodeDentryHeader();
    int32_t GenericDentryHeader();

    std::string parentDentryFile_{};
    std::string selfRecordId_{};
    std::string selfInode_{};
    uint32_t selfHash_{0};

    std::string syncFolderIndex_{};
    int32_t userId_{0};

private:
    int32_t GetCreateInfo(const MetaBase &base, uint32_t &bitPos, uint32_t &namehash, unsigned long &bidx,
                          struct CloudDiskServiceDentryGroup &dentryBlk);
    int32_t HandleFileByFd(unsigned long &endBlock, uint32_t level);
    std::mutex mtx_{};
    UniqueFd fd_{};
    std::string cacheFile_{};
};

typedef std::pair<uint32_t, std::string> MetaFileKey;
typedef std::pair<MetaFileKey, std::shared_ptr<CloudDiskServiceMetaFile>> MetaFileListEle;

class MetaFileMgr {
public:
    static MetaFileMgr& GetInstance();
    std::shared_ptr<CloudDiskServiceMetaFile> GetCloudDiskServiceMetaFile(const int32_t userId,
        const uint32_t syncFolderIndex, const uint64_t inode);

    int32_t GetRelativePath(const std::shared_ptr<CloudDiskServiceMetaFile> metaFile, std::string &path);
    void CloudDiskServiceClearAll();
private:
    MetaFileMgr() = default;
    ~MetaFileMgr() = default;
    MetaFileMgr(const MetaFileMgr &m) = delete;
    const MetaFileMgr &operator=(const MetaFileMgr &m) = delete;

    std::mutex mtx_{};
    std::list<MetaFileListEle> metaFileList_;
    std::map<MetaFileKey, std::list<MetaFileListEle>::iterator> metaFiles_;
};

} // namespace OHOS::FileManagement::CloudDiskService

#endif // CLOUD_DISK_SERVICE_METAFILE_H