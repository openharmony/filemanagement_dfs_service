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

#include "cloud_disk_service_metafile.h"

#include <string>
#include <memory>
#include <securec.h>
#include <sys/types.h>
#include <unistd.h>
#include <uuid/uuid.h>

#include "bit_ops.h"
#include "cloud_disk_service_error.h"
#include "cloud_file_utils.h"
#include "convertor.h"
#include "file_utils.h"
#include "utils_directory.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudDiskService {
using namespace OHOS::FileManagement;

constexpr uint32_t MAX_META_FILE_NUM = 150;
const unsigned int STAT_MODE_DIR = 0771;

struct DcacheLookupCtx {
    int fd{-1};
    std::string name{};
    std::string recordId{};
    uint32_t hash{0};
    unsigned long bidx{0};
    uint32_t bitPos{0};
    std::unique_ptr<CloudDiskServiceDentryGroup> page{nullptr};
};

static inline uint32_t GetDentrySlots(size_t namelen)
{
    return static_cast<uint32_t>((namelen + BYTE_PER_SLOT - 1) >> HMDFS_SLOT_LEN_BITS);
}

static inline off_t GetDentryGroupPos(size_t bidx)
{
    return bidx * DENTRYGROUP_SIZE + DENTRYGROUP_HEADER;
}

static inline uint64_t GetDentryGroupCnt(uint64_t size)
{
    return (size >= DENTRYGROUP_HEADER) ? ((size - DENTRYGROUP_HEADER) / DENTRYGROUP_SIZE) : 0;
}

static uint32_t GetOverallBucket(uint32_t level)
{
    if (level >= MAX_BUCKET_LEVEL) {
        LOGD("level = %{public}d overflow", level);
        return E_OK;
    }
    uint64_t buckets = (1ULL << (level + 1)) - 1;
    return static_cast<uint32_t>(buckets);
}

static size_t GetDcacheFileSize(uint32_t level)
{
    size_t buckets = GetOverallBucket(level);
    return buckets * DENTRYGROUP_SIZE * BUCKET_BLOCKS + DENTRYGROUP_HEADER;
}

static uint32_t GetBucketByLevel(uint32_t level)
{
    if (level >= MAX_BUCKET_LEVEL) {
        LOGD("level = %{public}d overflow", level);
        return E_OK;
    }

    uint64_t buckets = (1ULL << level);
    return static_cast<uint32_t>(buckets);
}

static uint32_t GetBucketaddr(uint32_t level, uint32_t buckoffset)
{
    if (level >= MAX_BUCKET_LEVEL) {
        return E_OK;
    }

    uint64_t curLevelMaxBucks = (1ULL << level);
    if (buckoffset >= curLevelMaxBucks) {
        return E_OK;
    }

    return static_cast<uint32_t>(curLevelMaxBucks) + buckoffset - 1;
}

static unsigned long GetBidxFromLevel(uint32_t level, uint32_t namehash)
{
    uint32_t bucket = GetBucketByLevel(level);
    if (bucket == 0) {
        return E_OK;
    }
    return BUCKET_BLOCKS * GetBucketaddr(level, namehash % bucket);
}

static std::unique_ptr<CloudDiskServiceDentryGroup> FindDentryPage(uint64_t index, DcacheLookupCtx *ctx)
{
    auto dentryBlk = std::make_unique<CloudDiskServiceDentryGroup>();

    off_t pos = GetDentryGroupPos(index);
    auto ret = FileRangeLock::FilePosLock(ctx->fd, pos, DENTRYGROUP_SIZE, F_WRLCK);
    if (ret) {
        return nullptr;
    }
    ssize_t size = FileUtils::ReadFile(ctx->fd, pos, DENTRYGROUP_SIZE, dentryBlk.get());
    if (size != DENTRYGROUP_SIZE) {
        (void)FileRangeLock::FilePosLock(ctx->fd, pos, DENTRYGROUP_SIZE, F_UNLCK);
        return nullptr;
    }
    return dentryBlk;
}

static CloudDiskServiceDentry *FindInBlockById(CloudDiskServiceDentryGroup &dentryBlk, DcacheLookupCtx *ctx,
                                               uint8_t revalidate)
{
    uint32_t bitPos = 0;
    CloudDiskServiceDentry *de = nullptr;

    while (bitPos < DENTRY_PER_GROUP) {
        if (!BitOps::TestBit(bitPos, dentryBlk.bitmap)) {
            bitPos++;
            continue;
        }
        de = &dentryBlk.nsl[bitPos];
        if (!de->namelen) {
            bitPos++;
            continue;
        }

        if ((de->revalidate & revalidate) != 0 &&
            !memcmp(ctx->recordId.c_str(), de->recordId, ctx->recordId.length())) {
            ctx->bitPos = bitPos;
            return de;
        }
        bitPos += GetDentrySlots(de->namelen);
    }

    return nullptr;
}

static CloudDiskServiceDentry *FindInBlock(CloudDiskServiceDentryGroup &dentryBlk, DcacheLookupCtx *ctx,
                                           uint8_t revalidate)
{
    uint32_t bitPos = 0;
    CloudDiskServiceDentry *de = nullptr;

    while (bitPos < DENTRY_PER_GROUP) {
        if (!BitOps::TestBit(bitPos, dentryBlk.bitmap)) {
            bitPos++;
            continue;
        }
        de = &dentryBlk.nsl[bitPos];
        if (!de->namelen) {
            bitPos++;
            continue;
        }

        if ((de->revalidate & revalidate) != 0 && de->hash == ctx->hash && de->namelen == ctx->name.length() &&
            !memcmp(ctx->name.c_str(), dentryBlk.fileName[bitPos], de->namelen)) {
            ctx->bitPos = bitPos;
            return de;
        }
        bitPos += GetDentrySlots(de->namelen);
    }

    return nullptr;
}

static CloudDiskServiceDentry *InLevel(uint32_t level, DcacheLookupCtx *ctx, bool byId, uint8_t revalidate)
    __attribute__((no_sanitize("unsigned-integer-overflow")))
{
    CloudDiskServiceDentry *de = nullptr;

    uint32_t nbucket = GetBucketByLevel(level);
    if (nbucket == 0) {
        return de;
    }

    unsigned long bidx = GetBucketaddr(level, ctx->hash % nbucket) * BUCKET_BLOCKS;
    uint32_t endBlock = bidx + BUCKET_BLOCKS;

    for (; bidx < endBlock; bidx++) {
        auto dentryBlk = FindDentryPage(bidx, ctx);
        if (dentryBlk == nullptr) {
            break;
        }

        if (byId) {
            de = FindInBlockById(*dentryBlk, ctx, revalidate);
        } else {
            de = FindInBlock(*dentryBlk, ctx, revalidate);
        }
        if (de != nullptr) {
            ctx->page = std::move(dentryBlk);
            break;
        }
        off_t pos = GetDentryGroupPos(bidx);
        (void)FileRangeLock::FilePosLock(ctx->fd, pos, DENTRYGROUP_SIZE, F_UNLCK);
    }
    ctx->bidx = bidx;
    return de;
}

static CloudDiskServiceDentry *FindDentry(DcacheLookupCtx *ctx, bool byId = false, uint8_t revalidate = VALIDATE)
{
    for (uint32_t level = 0; level < MAX_BUCKET_LEVEL; level++) {
        CloudDiskServiceDentry *de = InLevel(level, ctx, byId, revalidate);
        if (de != nullptr) {
            return de;
        }
    }
    return nullptr;
}

static void InitDcacheLookupCtx(DcacheLookupCtx *ctx, const MetaBase &base, uint32_t hash, int fd)
{
    ctx->fd = fd;
    ctx->name = base.name;
    ctx->bidx = 0;
    ctx->page = nullptr;
    ctx->hash = hash;
}

static uint32_t RoomForFilename(const uint8_t bitmap[], size_t slots, uint32_t maxSlots)
{
    uint32_t bitStart = 0;
    bool loopFlag = true;
    while (loopFlag) {
        uint32_t zeroStart = BitOps::FindNextZeroBit(bitmap, maxSlots, bitStart);
        if (zeroStart >= maxSlots) {
            return maxSlots;
        }

        uint32_t zeroEnd = BitOps::FindNextBit(bitmap, maxSlots, zeroStart);
        if (zeroEnd - zeroStart >= slots) {
            return zeroStart;
        }

        bitStart = zeroEnd + 1;
        if (zeroEnd + 1 >= maxSlots) {
            return maxSlots;
        }
    }
    return E_OK;
}

static bool CreateDentry(CloudDiskServiceDentryGroup &d, const MetaBase &base, uint32_t nameHash, uint32_t bitPos,
                         std::string recordId)
{
    CloudDiskServiceDentry *de;
    const std::string name = base.name;
    uint32_t slots = GetDentrySlots(name.length());

    de = &d.nsl[bitPos];
    de->hash = nameHash;
    de->namelen = name.length();
    auto ret = memcpy_s(d.fileName[bitPos], slots * DENTRY_NAME_LEN, name.c_str(), name.length());
    if (ret != 0) {
        LOGE("memcpy_s failed, dstLen = %{public}d, srcLen = %{public}zu", slots * DENTRY_NAME_LEN, name.length());
        return false;
    }
    de->revalidate = VALIDATE;
    de->atime = base.atime;
    de->mtime = base.mtime;
    de->size = base.size;
    de->mode = base.mode;
    (void) memset_s(de->recordId, RECORD_ID_LEN, 0, RECORD_ID_LEN);
    ret = memcpy_s(de->recordId, RECORD_ID_LEN, recordId.c_str(), recordId.length());
    if (ret != 0) {
        LOGE("memcpy_s failed, dstLen = %{public}d, srcLen = %{public}zu", RECORD_ID_LEN, recordId.length());
        return false;
    }

    for (uint32_t i = 0; i < slots; i++) {
        BitOps::SetBit(bitPos + i, d.bitmap);
        if (i) {
            (de + i)->namelen = 0;
        }
    }
    return true;
}

static std::string GetDentryFileByPath(const int32_t userId, const std::string &syncFolderIndex,
                                       const std::string &inode)
{
    std::string cacheDir =
        "/data/service/el2/" + std::to_string(userId) +
        "/hmdfs/cache/account_cache/dentry_cache/clouddisk_service_cache/" + syncFolderIndex + "/" +
        std::to_string(CloudDisk::CloudFileUtils::GetBucketId(inode)) + "/";
    Storage::DistributedFile::Utils::ForceCreateDirectory(cacheDir, STAT_MODE_DIR);
    return cacheDir + inode;
}

CloudDiskServiceMetaFile::CloudDiskServiceMetaFile(const int32_t userId, const uint32_t syncFolderIndex,
                                                   const uint64_t inode)
{
    userId_ = userId;
    syncFolderIndex_ = Convertor::ConvertToHex(syncFolderIndex);
    selfInode_ = Convertor::ConvertToHex(inode);
    cacheFile_ = GetDentryFileByPath(userId_, syncFolderIndex_, selfInode_);
    if (access(cacheFile_.c_str(), F_OK) == 0) {
        fd_ = UniqueFd{open(cacheFile_.c_str(), O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)};
        DecodeDentryHeader();
    } else {
        fd_ = UniqueFd{open(cacheFile_.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)};
    }
}

int32_t CloudDiskServiceMetaFile::DecodeDentryHeader()
{
    auto ret = FileRangeLock::FilePosLock(fd_, 0, DENTRYGROUP_HEADER, F_WRLCK);
    if (ret) {
        return ret;
    }
    struct CloudDiskServiceDcacheHeader header;
    ssize_t size = FileUtils::ReadFile(fd_, 0, sizeof(CloudDiskServiceDcacheHeader), &header);
    (void)FileRangeLock::FilePosLock(fd_, 0, DENTRYGROUP_HEADER, F_UNLCK);
    if (size != sizeof(CloudDiskServiceDcacheHeader)) {
        return size;
    }

    parentDentryFile_ = std::string(reinterpret_cast<const char *>(header.parentDentryfile), DENTRYFILE_NAME_LEN);
    selfRecordId_ = std::string(reinterpret_cast<const char *>(header.selfRecordId), RECORD_ID_LEN);
    selfHash_ = header.selfHash;

    return E_OK;
}

int32_t CloudDiskServiceMetaFile::GenericDentryHeader()
{
    struct stat fileStat;
    int err = fstat(fd_, &fileStat);
    if (err < 0) {
        return EINVAL;
    }
    if (fileStat.st_size < DENTRYGROUP_HEADER && ftruncate(fd_, DENTRYGROUP_HEADER)) {
        return ENOENT;
    }

    struct CloudDiskServiceDcacheHeader header;
    auto ret = memcpy_s(header.parentDentryfile, DENTRYFILE_NAME_LEN, parentDentryFile_.c_str(), DENTRYFILE_NAME_LEN);
    ret += memcpy_s(header.selfRecordId, RECORD_ID_LEN, selfRecordId_.c_str(), RECORD_ID_LEN);
    if (ret != 0) {
        LOGE("memcpy_s failed errno=%{public}d", errno);
        return errno;
    }
    header.selfHash = selfHash_;

    ret = FileRangeLock::FilePosLock(fd_, 0, DENTRYGROUP_HEADER, F_WRLCK);
    if (ret) {
        return ret;
    }
    ssize_t size = FileUtils::WriteFile(fd_, &header, 0, sizeof(CloudDiskServiceDcacheHeader));
    (void)FileRangeLock::FilePosLock(fd_, 0, DENTRYGROUP_HEADER, F_UNLCK);
    if (size != sizeof(CloudDiskServiceDcacheHeader)) {
        return size;
    }

    return E_OK;
}

int32_t CloudDiskServiceMetaFile::DoCreate(const MetaBase &base, unsigned long &bidx, uint32_t &bitPos)
{
    if (fd_ < 0) {
        LOGE("bad metafile fd");
        return EINVAL;
    }

    // validate the length of name, maximum length is 52*8 byte
    uint32_t slots = GetDentrySlots(base.name.length());
    if (slots > DENTRY_PER_GROUP) {
        LOGE("name is too long");
        return ENAMETOOLONG;
    }

    std::unique_lock<std::mutex> lock(mtx_);
    DcacheLookupCtx ctx;
    InitDcacheLookupCtx(&ctx, base, CloudDisk::CloudFileUtils::DentryHash(base.name), fd_);
    CloudDiskServiceDentry *de = FindDentry(&ctx);
    if (de != nullptr) {
        LOGE("this name dentry is exist");
        (void)FileRangeLock::FilePosLock(fd_, GetDentryGroupPos(ctx.bidx), DENTRYGROUP_SIZE, F_UNLCK);
        return EEXIST;
    }
    CloudDiskServiceDentryGroup dentryBlk = {0};
    uint32_t namehash = 0;
    auto ret = GetCreateInfo(base, bitPos, namehash, bidx, dentryBlk);
    if (ret) {
        return ret;
    }
    off_t pos = GetDentryGroupPos(bidx);
    if (!CreateDentry(dentryBlk, base, namehash, bitPos, base.recordId)) {
        LOGI("CreateDentry fail, stop write.");
        (void)FileRangeLock::FilePosLock(fd_, pos, DENTRYGROUP_SIZE, F_UNLCK);
        return EINVAL;
    }
    int size = FileUtils::WriteFile(fd_, &dentryBlk, pos, DENTRYGROUP_SIZE);
    if (size != DENTRYGROUP_SIZE) {
        LOGD("WriteFile failed, size %{public}d != %{public}d", size, DENTRYGROUP_SIZE);
        (void)FileRangeLock::FilePosLock(fd_, pos, DENTRYGROUP_SIZE, F_UNLCK);
        return EINVAL;
    }
    ret = FileRangeLock::FilePosLock(fd_, pos, DENTRYGROUP_SIZE, F_UNLCK);
    if (ret) {
        return ret;
    }
    return E_OK;
}

int32_t CloudDiskServiceMetaFile::DoRemove(const MetaBase &base, std::string &recordId, unsigned long &bidx,
                                           uint32_t &bitPos)
{
    if (fd_ < 0) {
        LOGE("bad metafile fd");
        return EINVAL;
    }

    std::unique_lock<std::mutex> lock(mtx_);
    struct DcacheLookupCtx ctx;
    InitDcacheLookupCtx(&ctx, base, CloudDisk::CloudFileUtils::DentryHash(base.name), fd_);
    struct CloudDiskServiceDentry *de = FindDentry(&ctx);
    if (de == nullptr) {
        LOGD("find dentry failed");
        return ENOENT;
    }

    de->revalidate = INVALIDATE;
    recordId = std::string(reinterpret_cast<const char *>(de->recordId), RECORD_ID_LEN);

    off_t ipos = GetDentryGroupPos(ctx.bidx);
    ssize_t size = FileUtils::WriteFile(fd_, ctx.page.get(), ipos, sizeof(struct CloudDiskServiceDentryGroup));
    if (size != sizeof(struct CloudDiskServiceDentryGroup)) {
        LOGE("write failed, ret = %{public}zd", size);
        (void)FileRangeLock::FilePosLock(fd_, ipos, DENTRYGROUP_SIZE, F_UNLCK);
        return EIO;
    }
    auto ret = FileRangeLock::FilePosLock(fd_, ipos, DENTRYGROUP_SIZE, F_UNLCK);
    if (ret) {
        return ret;
    }
    bidx = ctx.bidx;
    bitPos = ctx.bitPos;
    return E_OK;
}

int32_t CloudDiskServiceMetaFile::DoFlush(const MetaBase &base, std::string &recordId, unsigned long &bidx,
                                          uint32_t &bitPos)
{
    if (fd_ < 0) {
        LOGE("bad metafile fd");
        return EINVAL;
    }

    std::unique_lock<std::mutex> lock(mtx_);
    DcacheLookupCtx ctx;
    InitDcacheLookupCtx(&ctx, base, CloudDisk::CloudFileUtils::DentryHash(base.name), fd_);
    CloudDiskServiceDentry *de = FindDentry(&ctx, false, INVALIDATE);
    if (de == nullptr) {
        LOGE("find dentry failed");
        return ENOENT;
    }

    uint32_t slots = GetDentrySlots(de->namelen);
    for (uint32_t i = 0; i < slots; i++) {
        BitOps::ClearBit(ctx.bitPos + i, ctx.page->bitmap);
    }

    off_t ipos = GetDentryGroupPos(ctx.bidx);
    ssize_t size = FileUtils::WriteFile(fd_, ctx.page.get(), ipos, sizeof(CloudDiskServiceDentryGroup));
    if (size != sizeof(CloudDiskServiceDentryGroup)) {
        LOGE("WriteFile failed!, ret = %{public}zd", size);
        (void)FileRangeLock::FilePosLock(fd_, ipos, DENTRYGROUP_SIZE, F_UNLCK);
        return EIO;
    }
    auto ret = FileRangeLock::FilePosLock(fd_, ipos, DENTRYGROUP_SIZE, F_UNLCK);
    if (ret) {
        return ret;
    }
    bidx = ctx.bidx;
    bitPos = ctx.bitPos;
    return E_OK;
}

int32_t CloudDiskServiceMetaFile::DoUpdate(const MetaBase &base, std::string &recordId, unsigned long &bidx,
                                           uint32_t &bitPos)
{
    if (fd_ < 0) {
        LOGE("bad metafile fd");
        return EINVAL;
    }

    std::unique_lock<std::mutex> lock(mtx_);
    struct DcacheLookupCtx ctx;
    InitDcacheLookupCtx(&ctx, base, CloudDisk::CloudFileUtils::DentryHash(base.name), fd_);
    struct CloudDiskServiceDentry *de = FindDentry(&ctx);
    if (de == nullptr) {
        LOGD("find dentry failed");
        return ENOENT;
    }

    de->atime = base.atime;
    de->mtime = base.mtime;
    de->size = base.size;
    de->mode = base.mode;
    recordId = std::string(reinterpret_cast<const char *>(de->recordId), RECORD_ID_LEN);

    off_t ipos = GetDentryGroupPos(ctx.bidx);
    ssize_t size = FileUtils::WriteFile(fd_, ctx.page.get(), ipos, sizeof(struct CloudDiskServiceDentryGroup));
    if (size != sizeof(struct CloudDiskServiceDentryGroup)) {
        LOGE("write failed, ret = %{public}zd", size);
        (void)FileRangeLock::FilePosLock(fd_, ipos, DENTRYGROUP_SIZE, F_UNLCK);
        return EIO;
    }
    auto ret = FileRangeLock::FilePosLock(fd_, ipos, DENTRYGROUP_SIZE, F_UNLCK);
    if (ret) {
        return ret;
    }
    bidx = ctx.bidx;
    bitPos = ctx.bitPos;
    return E_OK;
}

int32_t CloudDiskServiceMetaFile::DoRenameOld(const MetaBase &base, std::string &recordId, unsigned long &bidx,
                                              uint32_t &bitPos)
{
    if (fd_ < 0) {
        LOGE("bad metafile fd");
        return EINVAL;
    }

    std::unique_lock<std::mutex> lock(mtx_);
    struct DcacheLookupCtx ctx;
    InitDcacheLookupCtx(&ctx, base, CloudDisk::CloudFileUtils::DentryHash(base.name), fd_);
    struct CloudDiskServiceDentry *de = FindDentry(&ctx);
    if (de == nullptr) {
        LOGD("find dentry failed");
        return ENOENT;
    }

    de->revalidate = INVALIDATE;
    recordId = std::string(reinterpret_cast<const char *>(de->recordId), RECORD_ID_LEN);

    off_t ipos = GetDentryGroupPos(ctx.bidx);
    ssize_t size = FileUtils::WriteFile(fd_, ctx.page.get(), ipos, sizeof(struct CloudDiskServiceDentryGroup));
    if (size != sizeof(struct CloudDiskServiceDentryGroup)) {
        LOGE("write failed, ret = %{public}zd", size);
        (void)FileRangeLock::FilePosLock(fd_, ipos, DENTRYGROUP_SIZE, F_UNLCK);
        return EIO;
    }
    auto ret = FileRangeLock::FilePosLock(fd_, ipos, DENTRYGROUP_SIZE, F_UNLCK);
    if (ret) {
        return ret;
    }
    bidx = ctx.bidx;
    bitPos = ctx.bitPos;
    return E_OK;
}

int32_t CloudDiskServiceMetaFile::DoRenameNew(const MetaBase &base, std::string &recordId, unsigned long &bidx,
                                              uint32_t &bitPos)
{
    if (fd_ < 0) {
        LOGE("bad metafile fd");
        return EINVAL;
    }

    // validate the length of name, maximum length is 52*8 byte
    uint32_t slots = GetDentrySlots(base.name.length());
    if (slots > DENTRY_PER_GROUP) {
        LOGE("name is too long");
        return ENAMETOOLONG;
    }

    std::unique_lock<std::mutex> lock(mtx_);
    DcacheLookupCtx ctx;
    InitDcacheLookupCtx(&ctx, base, CloudDisk::CloudFileUtils::DentryHash(base.name), fd_);
    CloudDiskServiceDentry *de = FindDentry(&ctx);
    if (de != nullptr) {
        LOGE("this name dentry is exist");
        (void)FileRangeLock::FilePosLock(fd_, GetDentryGroupPos(ctx.bidx), DENTRYGROUP_SIZE, F_UNLCK);
        return EEXIST;
    }
    CloudDiskServiceDentryGroup dentryBlk = {0};
    uint32_t namehash = 0;
    auto ret = GetCreateInfo(base, bitPos, namehash, bidx, dentryBlk);
    if (ret) {
        return ret;
    }
    off_t pos = GetDentryGroupPos(bidx);
    if (!CreateDentry(dentryBlk, base, namehash, bitPos, recordId)) {
        LOGI("CreateDentry fail, stop write.");
        (void)FileRangeLock::FilePosLock(fd_, pos, DENTRYGROUP_SIZE, F_UNLCK);
        return EINVAL;
    }
    int size = FileUtils::WriteFile(fd_, &dentryBlk, pos, DENTRYGROUP_SIZE);
    if (size != DENTRYGROUP_SIZE) {
        LOGD("WriteFile failed, size %{public}d != %{public}d", size, DENTRYGROUP_SIZE);
        (void)FileRangeLock::FilePosLock(fd_, pos, DENTRYGROUP_SIZE, F_UNLCK);
        return EINVAL;
    }
    ret = FileRangeLock::FilePosLock(fd_, pos, DENTRYGROUP_SIZE, F_UNLCK);
    if (ret) {
        return ret;
    }
    return E_OK;
}

int32_t CloudDiskServiceMetaFile::DoRename(MetaBase &metaBase, const std::string &newName,
    std::shared_ptr<CloudDiskServiceMetaFile> newMetaFile)
{
    std::string oldName = metaBase.name;
    metaBase.name = newName;
    unsigned long bidx;
    uint32_t bitPos;
    auto ret = newMetaFile->DoCreate(metaBase, bidx, bitPos);
    if (ret != 0) {
        LOGE("create dentry failed, ret = %{public}d", ret);
        return ret;
    }
    metaBase.name = oldName;
    std::string childRecordId;
    ret = DoRemove(metaBase, childRecordId, bidx, bitPos);
    if (ret != 0) {
        LOGE("remove dentry failed, ret = %{public}d", ret);
        metaBase.name = newName;
        (void)newMetaFile->DoFlush(metaBase, childRecordId, bidx, bitPos);
        return ret;
    }
    return E_OK;
}

int32_t CloudDiskServiceMetaFile::DoLookupByName(MetaBase &base)
{
    if (fd_ < 0) {
        LOGE("bad metafile fd");
        return EINVAL;
    }

    std::unique_lock<std::mutex> lock(mtx_);
    struct DcacheLookupCtx ctx;
    InitDcacheLookupCtx(&ctx, base, CloudDisk::CloudFileUtils::DentryHash(base.name), fd_);
    struct CloudDiskServiceDentry *de = FindDentry(&ctx);
    if (de == nullptr) {
        LOGD("find dentry failed");
        return ENOENT;
    }
    (void)FileRangeLock::FilePosLock(fd_, GetDentryGroupPos(ctx.bidx), DENTRYGROUP_SIZE, F_UNLCK);

    base.mode = de->mode;
    base.hash = de->hash;
    base.size = de->size;
    base.atime = de->atime;
    base.mtime = de->mtime;
    base.recordId = std::string(reinterpret_cast<const char *>(de->recordId), RECORD_ID_LEN);
    return E_OK;
}

int32_t CloudDiskServiceMetaFile::DoLookupByRecordId(MetaBase &base, uint8_t revalidate)
{
    if (fd_ < 0) {
        LOGE("bad metafile fd");
        return EINVAL;
    }

    std::unique_lock<std::mutex> lock(mtx_);
    struct DcacheLookupCtx ctx;
    InitDcacheLookupCtx(&ctx, base, base.hash, fd_);
    ctx.recordId = base.recordId;
    struct CloudDiskServiceDentry *de = ((revalidate & VALIDATE) == 0) ? nullptr : FindDentry(&ctx, true, VALIDATE);
    if (de == nullptr) {
        de = ((revalidate & INVALIDATE) == 0) ? nullptr : FindDentry(&ctx, true, INVALIDATE);
        if (de == nullptr) {
            LOGD("find dentry by id failed");
            return ENOENT;
        }
    }
    (void)FileRangeLock::FilePosLock(fd_, GetDentryGroupPos(ctx.bidx), DENTRYGROUP_SIZE, F_UNLCK);

    base.mode = de->mode;
    base.hash = de->hash;
    base.size = de->size;
    base.atime = de->atime;
    base.mtime = de->mtime;
    base.name = std::string(reinterpret_cast<const char *>(ctx.page->fileName[ctx.bitPos]), de->namelen);
    return E_OK;
}

int32_t CloudDiskServiceMetaFile::DoLookupByOffset(MetaBase &base, const unsigned long bidx, const uint32_t bitPos)
{
    if (fd_ < 0) {
        LOGE("bad metafile fd");
        return EINVAL;
    }

    std::unique_lock<std::mutex> lock(mtx_);
    struct DcacheLookupCtx ctx;
    ctx.fd = fd_;
    auto dentryBlk = FindDentryPage(bidx, &ctx);
    if (dentryBlk == nullptr) {
        return ENOENT;
    }
    if (bitPos >= DENTRY_PER_GROUP) {
        return EINVAL;
    }
    CloudDiskServiceDentry *de = &(dentryBlk->nsl[bitPos]);

    base.mode = de->mode;
    base.hash = de->hash;
    base.size = de->size;
    base.atime = de->atime;
    base.mtime = de->mtime;
    base.name = std::string(reinterpret_cast<const char *>(dentryBlk->fileName[bitPos]), de->namelen);
    base.recordId = std::string(reinterpret_cast<const char *>(de->recordId), RECORD_ID_LEN);
    return E_OK;
}

int32_t CloudDiskServiceMetaFile::GetCreateInfo(const MetaBase &base, uint32_t &bitPos, uint32_t &namehash,
    unsigned long &bidx, struct CloudDiskServiceDentryGroup &dentryBlk)
{
    uint32_t level = 0;
    namehash = CloudDisk::CloudFileUtils::DentryHash(base.name);
    bool found = false;
    while (!found) {
        if (level == MAX_BUCKET_LEVEL) {
            return ENOSPC;
        }
        bidx = GetBidxFromLevel(level, namehash);
        unsigned long endBlock = bidx + BUCKET_BLOCKS;
        int32_t ret = HandleFileByFd(endBlock, level);
        if (ret != 0) {
            return ret;
        }
        for (; bidx < endBlock; bidx++) {
            off_t pos = GetDentryGroupPos(bidx);
            ret = FileRangeLock::FilePosLock(fd_, pos, DENTRYGROUP_SIZE, F_WRLCK);
            if (ret) {
                return ret;
            }
            if (FileUtils::ReadFile(fd_, pos, DENTRYGROUP_SIZE, &dentryBlk) != DENTRYGROUP_SIZE) {
                (void)FileRangeLock::FilePosLock(fd_, pos, DENTRYGROUP_SIZE, F_UNLCK);
                return ENOENT;
            }
            bitPos = RoomForFilename(dentryBlk.bitmap, GetDentrySlots(base.name.length()), DENTRY_PER_GROUP);
            if (bitPos < DENTRY_PER_GROUP) {
                found = true;
                break;
            }
            (void)FileRangeLock::FilePosLock(fd_, pos, DENTRYGROUP_SIZE, F_UNLCK);
        }
        ++level;
    }
    return E_OK;
}

int32_t CloudDiskServiceMetaFile::HandleFileByFd(unsigned long &endBlock, uint32_t level)
{
    struct stat fileStat;
    int err = fstat(fd_, &fileStat);
    if (err < 0) {
        return EINVAL;
    }
    if ((endBlock > GetDentryGroupCnt(fileStat.st_size)) &&
        ftruncate(fd_, GetDcacheFileSize(level))) {
        return ENOENT;
    }
    return E_OK;
}

MetaFileMgr& MetaFileMgr::GetInstance()
{
    static MetaFileMgr instance_;
    return instance_;
}

std::shared_ptr<CloudDiskServiceMetaFile> MetaFileMgr::GetCloudDiskServiceMetaFile(int32_t userId,
    const uint32_t syncFolderIndex, const uint64_t inode)
{
    std::shared_ptr<CloudDiskServiceMetaFile> mFile = nullptr;
    std::lock_guard<std::mutex> lock(mtx_);
    MetaFileKey key(userId, Convertor::ConvertToHex(syncFolderIndex) + Convertor::ConvertToHex(inode));
    auto it = metaFiles_.find(key);
    if (it != metaFiles_.end()) {
        metaFileList_.splice(metaFileList_.begin(), metaFileList_, it->second);
        mFile = it->second->second;
    } else {
        if (metaFiles_.size() == MAX_META_FILE_NUM) {
            auto deleteKey = metaFileList_.back().first;
            metaFiles_.erase(deleteKey);
            metaFileList_.pop_back();
        }
        mFile = std::make_shared<CloudDiskServiceMetaFile>(userId, syncFolderIndex, inode);
        metaFileList_.emplace_front(key, mFile);
        metaFiles_[key] = metaFileList_.begin();
    }
    return mFile;
}

int32_t MetaFileMgr::GetRelativePath(const std::shared_ptr<CloudDiskServiceMetaFile> metaFile, std::string &path)
{
    auto pMetaFile = metaFile;
    int32_t userId = metaFile->userId_;
    uint32_t syncFolderIndex = Convertor::ConvertFromHex(metaFile->syncFolderIndex_);
    while (pMetaFile->parentDentryFile_ != ROOT_PARENTDENTRYFILE) {
        uint64_t inode = Convertor::ConvertFromHex(pMetaFile->parentDentryFile_);
        auto parentMetaFile = GetCloudDiskServiceMetaFile(userId, syncFolderIndex, inode);

        MetaBase mBase(pMetaFile->selfRecordId_, pMetaFile->selfHash_);
        int32_t ret = parentMetaFile->DoLookupByRecordId(mBase, VALIDATE);
        if (ret != 0) {
            path = "";
            return E_PATH_NOT_EXIST;
        }

        path = mBase.name + "/" + path;
        pMetaFile = parentMetaFile;
    }
    path = "/" + path;
    return E_OK;
}

void MetaFileMgr::CloudDiskServiceClearAll()
{
    std::lock_guard<std::mutex> lock(mtx_);
    metaFiles_.clear();
    metaFileList_.clear();
}

} // namespace OHOS::FileManagement::CloudDiskService
