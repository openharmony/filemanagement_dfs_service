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

#include "meta_file.h"

#include <ctime>
#include <fcntl.h>
#include <iomanip>
#include <sstream>
#include <sys/stat.h>

#include "cloud_file_utils.h"
#include "dfs_error.h"
#include "file_utils.h"
#include "securec.h"
#include "string_ex.h"
#include "sys/xattr.h"
#include "utils_directory.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
constexpr uint32_t DENTRYGROUP_SIZE = 4096;
constexpr uint32_t DENTRY_NAME_LEN = 8;
constexpr uint32_t DENTRY_RESERVED_LENGTH = 4;
constexpr uint32_t DENTRY_PER_GROUP = 52;
constexpr uint32_t DENTRY_BITMAP_LENGTH = 7;
constexpr uint32_t DENTRY_GROUP_RESERVED = 32;
constexpr uint32_t CLOUD_RECORD_ID_LEN = 33;
constexpr uint32_t DENTRYGROUP_HEADER = 4096;
constexpr uint32_t MAX_BUCKET_LEVEL = 63;
constexpr uint32_t BUCKET_BLOCKS = 2;
constexpr uint32_t BITS_PER_BYTE = 8;
constexpr uint32_t HMDFS_SLOT_LEN_BITS = 3;
constexpr uint32_t FILE_TYPE_OFFSET = 2;
constexpr uint32_t NO_UPLOAD_OFFSET = 4;

#pragma pack(push, 1)
struct HmdfsDentry {
    uint32_t hash{0};
    uint16_t mode{0};
    uint16_t namelen{0};
    uint64_t size{0};
    uint64_t mtime{0};
    uint64_t atime{0};
    uint8_t recordId[CLOUD_RECORD_ID_LEN]{0};
    uint8_t flags{0};
    /* reserved bytes for long term extend, total 60 bytes */
    uint8_t reserved[DENTRY_RESERVED_LENGTH];
};

struct HmdfsDentryGroup {
    uint8_t dentryVersion;
    uint8_t bitmap[DENTRY_BITMAP_LENGTH];
    struct HmdfsDentry nsl[DENTRY_PER_GROUP];
    uint8_t fileName[DENTRY_PER_GROUP][DENTRY_NAME_LEN];
    uint8_t reserved[DENTRY_GROUP_RESERVED];
};
static_assert(sizeof(HmdfsDentryGroup) == DENTRYGROUP_SIZE);

struct HmdfsDcacheHeader {
    uint64_t dcacheCrtime{0};
    uint64_t dcacheCrtimeNsec{0};

    uint64_t dentryCtime{0};
    uint64_t dentryCtimeNsec{0};

    uint64_t dentryCount{0};
};
#pragma pack(pop)

void MetaHelper::SetFileType(struct HmdfsDentry *de, uint8_t fileType)
{
    de->flags &= 0x13;
    de->flags |= (fileType << FILE_TYPE_OFFSET);
}

void MetaHelper::SetPosition(struct HmdfsDentry *de, uint8_t position)
{
    de->flags &= 0xFC;
    de->flags |= position;
}

void MetaHelper::SetNoUpload(struct HmdfsDentry *de, uint8_t noUpload)
{
    de->flags &= 0xEF;
    de->flags |= (noUpload << NO_UPLOAD_OFFSET);
}

uint8_t MetaHelper::GetNoUpload(const struct HmdfsDentry *de)
{
    return (de->flags & 0x10) >> NO_UPLOAD_OFFSET;
}

uint8_t MetaHelper::GetFileType(const struct HmdfsDentry *de)
{
    return (de->flags & 0xC) >> FILE_TYPE_OFFSET;
}

uint8_t MetaHelper::GetPosition(const struct HmdfsDentry *de)
{
    return de->flags & 0x3;
}

static std::string GetCloudDiskDentryFileByPath(uint32_t userId, const std::string &bundleName,
    const std::string &cloudId)
{
    std::string cacheDir =
        "/data/service/el2/" + std::to_string(userId) +
        "/hmdfs/cloud/data/" + bundleName + "/" +
        std::to_string(CloudDisk::CloudFileUtils::GetBucketId(cloudId)) + "/";
    std::string dentryFileName = MetaFileMgr::GetInstance().CloudIdToRecordId(cloudId);
    Storage::DistributedFile::Utils::ForceCreateDirectory(cacheDir, STAT_MODE_DIR);
    return cacheDir + dentryFileName;
}

CloudDiskMetaFile::CloudDiskMetaFile(uint32_t userId, const std::string &bundleName, const std::string &cloudId)
{
    userId_ = userId;
    bundleName_ = bundleName;
    cloudId_ = cloudId;
    cacheFile_ = GetCloudDiskDentryFileByPath(userId_, bundleName_, cloudId_);
    fd_ = UniqueFd{open(cacheFile_.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)};
    LOGD("CloudDiskMetaFile cloudId=%{public}s, path=%{public}s", cloudId_.c_str(), GetAnonyString(cacheFile_).c_str());
    LOGD("CloudDiskMetaFile fd=%{public}d, errno :%{public}d", fd_.Get(), errno);

    HmdfsDcacheHeader header{};
    (void)FileUtils::ReadFile(fd_, 0, sizeof(header), &header);
}

std::string CloudDiskMetaFile::GetDentryFilePath()
{
    return cacheFile_;
}

int32_t CloudDiskMetaFile::DoLookupAndUpdate(const std::string &name, CloudDiskMetaFileCallBack callback)
{
    MetaBase m(name);
    /* lookup and create in parent */
    int32_t ret = DoLookup(m);
    if (ret != E_OK) {
        callback(m);
        ret = DoCreate(m);
        if (ret != E_OK) {
            LOGE("create dentry file failed, ret %{public}d", ret);
            return ret;
        }
        return ret;
    } else {
        callback(m);
        ret = DoUpdate(m);
        if (ret != E_OK) {
            LOGE("update dentry file failed, ret %{public}d", ret);
            return ret;
        }
    }
    return E_OK;
}

int32_t CloudDiskMetaFile::DoChildUpdate(const std::string &name, CloudDiskMetaFileCallBack callback)
{
    MetaBase m(name);
    /* lookup and update */
    int32_t ret = DoLookup(m);
    if (ret != E_OK) {
        LOGE("lookup dentry file failed, ret %{public}d", ret);
        return ret;
    } else {
        callback(m);
        ret = DoUpdate(m);
        if (ret != E_OK) {
            LOGE("update dentry file failed, ret %{public}d", ret);
            return ret;
        }
    }
    return E_OK;
}

int32_t CloudDiskMetaFile::DoLookupAndRemove(MetaBase &metaBase)
{
    /* lookup and remove in parent */
    int32_t ret = DoLookup(metaBase);
    if (ret == E_OK) {
        ret = DoRemove(metaBase);
        if (ret != E_OK) {
            LOGE("remove dentry file failed, ret %{public}d", ret);
            return ret;
        }
        return E_OK;
    }
    return E_OK;
}

CloudDiskMetaFile::~CloudDiskMetaFile()
{
}

static inline uint32_t GetDentrySlots(size_t nameLen)
{
    return static_cast<uint32_t>((nameLen + BITS_PER_BYTE - 1) >> HMDFS_SLOT_LEN_BITS);
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
        return 0;
    }
    uint64_t buckets = (1ULL << (level + 1)) - 1;
    return static_cast<uint32_t>(buckets);
}

static size_t GetDcacheFileSize(uint32_t level)
{
    size_t buckets = GetOverallBucket(level);
    return buckets * DENTRYGROUP_SIZE * BUCKET_BLOCKS + DENTRYGROUP_HEADER;
}

static uint32_t GetBucketaddr(uint32_t level, uint32_t buckoffset)
{
    if (level >= MAX_BUCKET_LEVEL) {
        return 0;
    }

    uint64_t curLevelMaxBucks = (1ULL << level);
    if (buckoffset >= curLevelMaxBucks) {
        return 0;
    }

    return static_cast<uint32_t>(curLevelMaxBucks) + buckoffset - 1;
}

static uint32_t GetBucketByLevel(uint32_t level)
{
    if (level >= MAX_BUCKET_LEVEL) {
        LOGD("level = %{public}d overflow", level);
        return 0;
    }

    uint64_t buckets = (1ULL << level);
    return static_cast<uint32_t>(buckets);
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
    return 0;
}

static void UpdateDentry(HmdfsDentryGroup &d, const MetaBase &base, uint32_t nameHash, uint32_t bitPos)
{
    HmdfsDentry *de;
    const std::string name = base.name;
    uint32_t slots = GetDentrySlots(name.length());

    de = &d.nsl[bitPos];
    de->hash = nameHash;
    de->namelen = name.length();
    auto ret = memcpy_s(d.fileName[bitPos], slots * DENTRY_NAME_LEN, name.c_str(), name.length());
    if (ret != 0) {
        LOGE("memcpy_s failed, dstLen = %{public}d, srcLen = %{public}zu", slots * DENTRY_NAME_LEN, name.length());
    }
    de->atime = base.atime;
    de->mtime = base.mtime;
    de->size = base.size;
    de->mode = base.mode;
    MetaHelper::SetPosition(de, base.position);
    MetaHelper::SetFileType(de, base.fileType);
    MetaHelper::SetNoUpload(de, base.noUpload);
    (void) memset_s(de->recordId, CLOUD_RECORD_ID_LEN, 0, CLOUD_RECORD_ID_LEN);
    ret = memcpy_s(de->recordId, CLOUD_RECORD_ID_LEN, base.cloudId.c_str(), base.cloudId.length());
    if (ret != 0) {
        LOGE("memcpy_s failed, dstLen = %{public}d, srcLen = %{public}zu", CLOUD_RECORD_ID_LEN, base.cloudId.length());
    }

    for (uint32_t i = 0; i < slots; i++) {
        BitOps::SetBit(bitPos + i, d.bitmap);
        if (i) {
            (de + i)->namelen = 0;
        }
    }
}

int32_t CloudDiskMetaFile::HandleFileByFd(unsigned long &endBlock, uint32_t &level)
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

static unsigned long GetBidxFromLevel(uint32_t level, uint32_t namehash)
{
    uint32_t bucket = GetBucketByLevel(level);
    if (bucket == 0) {
        return 0;
    }
    return BUCKET_BLOCKS * GetBucketaddr(level, namehash % bucket);
}

int32_t CloudDiskMetaFile::DoCreate(const MetaBase &base)
{
    if (fd_ < 0) {
        LOGE("bad metafile fd");
        return EINVAL;
    }
    off_t pos = 0;
    uint32_t level = 0;
    uint32_t bitPos = 0;
    uint32_t namehash = 0;
    unsigned long bidx = 0;
    HmdfsDentryGroup dentryBlk = {0};
    std::unique_lock<std::mutex> lock(mtx_);
    FileRangeLock fileLock(fd_, 0, 0);
    namehash = CloudDisk::CloudFileUtils::DentryHash(base.name);
    bool found = false;
    while (!found) {
        if (level == MAX_BUCKET_LEVEL) {
            return ENOSPC;
        }
        bidx = GetBidxFromLevel(level, namehash);
        unsigned long endBlock = bidx + BUCKET_BLOCKS;
        int32_t ret = HandleFileByFd(endBlock, level);
        if (ret != E_OK) {
            return ret;
        }
        for (; bidx < endBlock; bidx++) {
            pos = GetDentryGroupPos(bidx);
            int size = FileUtils::ReadFile(fd_, pos, DENTRYGROUP_SIZE, &dentryBlk);
            if (size != DENTRYGROUP_SIZE) {
                return ENOENT;
            }
            bitPos = RoomForFilename(dentryBlk.bitmap, GetDentrySlots(base.name.length()), DENTRY_PER_GROUP);
            if (bitPos < DENTRY_PER_GROUP) {
                found = true;
                break;
            }
        }
        ++level;
    }
    pos = GetDentryGroupPos(bidx);
    UpdateDentry(dentryBlk, base, namehash, bitPos);
    int size = FileUtils::WriteFile(fd_, &dentryBlk, pos, DENTRYGROUP_SIZE);
    if (size != DENTRYGROUP_SIZE) {
        LOGD("WriteFile failed, size %{public}d != %{public}d", size, DENTRYGROUP_SIZE);
        return EINVAL;
    }
    return E_OK;
}

struct DcacheLookupCtx {
    int fd{-1};
    std::string name{};
    uint32_t hash{0};
    uint32_t bidx{0};
    std::unique_ptr<HmdfsDentryGroup> page{nullptr};
};

static void InitDcacheLookupCtx(DcacheLookupCtx *ctx, const MetaBase &base, int fd)
{
    ctx->fd = fd;
    ctx->name = base.name;
    ctx->bidx = 0;
    ctx->page = nullptr;
    ctx->hash = CloudDisk::CloudFileUtils::DentryHash(ctx->name);
}

static std::unique_ptr<HmdfsDentryGroup> FindDentryPage(uint64_t index, DcacheLookupCtx *ctx)
{
    auto dentryBlk = std::make_unique<HmdfsDentryGroup>();

    off_t pos = GetDentryGroupPos(index);
    ssize_t size = FileUtils::ReadFile(ctx->fd, pos, DENTRYGROUP_SIZE, dentryBlk.get());
    if (size != DENTRYGROUP_SIZE) {
        return nullptr;
    }
    return dentryBlk;
}

static HmdfsDentry *FindInBlock(HmdfsDentryGroup &dentryBlk, uint32_t namehash, const std::string &name)
{
    int maxLen = 0;
    uint32_t bitPos = 0;
    HmdfsDentry *de = nullptr;

    while (bitPos < DENTRY_PER_GROUP) {
        if (!BitOps::TestBit(bitPos, dentryBlk.bitmap)) {
            bitPos++;
            maxLen++;
            continue;
        }
        de = &dentryBlk.nsl[bitPos];
        if (!de->namelen) {
            bitPos++;
            continue;
        }

        if (de->hash == namehash && de->namelen == name.length() &&
            !memcmp(name.c_str(), dentryBlk.fileName[bitPos], de->namelen)) {
            return de;
        }
        maxLen = 0;
        bitPos += GetDentrySlots(de->namelen);
    }

    return nullptr;
}

static HmdfsDentry *InLevel(uint32_t level, DcacheLookupCtx *ctx)
    __attribute__((no_sanitize("unsigned-integer-overflow")))
{
    HmdfsDentry *de = nullptr;

    uint32_t nbucket = GetBucketByLevel(level);
    if (nbucket == 0) {
        return de;
    }

    uint32_t bidx = GetBucketaddr(level, ctx->hash % nbucket) * BUCKET_BLOCKS;
    uint32_t endBlock = bidx + BUCKET_BLOCKS;

    for (; bidx < endBlock; bidx++) {
        auto dentryBlk = FindDentryPage(bidx, ctx);
        if (dentryBlk == nullptr) {
            break;
        }

        de = FindInBlock(*dentryBlk, ctx->hash, ctx->name);
        if (de != nullptr) {
            ctx->page = std::move(dentryBlk);
            break;
        }
    }
    ctx->bidx = bidx;
    return de;
}

static HmdfsDentry *FindDentry(DcacheLookupCtx *ctx)
{
    for (uint32_t level = 0; level < MAX_BUCKET_LEVEL; level++) {
        HmdfsDentry *de = InLevel(level, ctx);
        if (de != nullptr) {
            return de;
        }
    }
    return nullptr;
}

int32_t CloudDiskMetaFile::DoRemove(const MetaBase &base)
{
    if (fd_ < 0) {
        LOGE("bad metafile fd");
        return EINVAL;
    }

    std::unique_lock<std::mutex> lock(mtx_);
    FileRangeLock fileLock(fd_, 0, 0);
    DcacheLookupCtx ctx;
    InitDcacheLookupCtx(&ctx, base, fd_);
    HmdfsDentry *de = FindDentry(&ctx);
    if (de == nullptr) {
        LOGE("find dentry failed");
        return ENOENT;
    }

    uint32_t bitPos = (de - ctx.page->nsl);
    uint32_t slots = GetDentrySlots(de->namelen);
    for (uint32_t i = 0; i < slots; i++) {
        BitOps::ClearBit(bitPos + i, ctx.page->bitmap);
    }

    off_t ipos = GetDentryGroupPos(ctx.bidx);
    ssize_t size = FileUtils::WriteFile(fd_, ctx.page.get(), ipos, sizeof(HmdfsDentryGroup));
    if (size != sizeof(HmdfsDentryGroup)) {
        LOGE("WriteFile failed!, ret = %{public}zd", size);
        return EIO;
    }

    return E_OK;
}

int32_t CloudDiskMetaFile::DoLookup(MetaBase &base)
{
    if (fd_ < 0) {
        LOGE("bad metafile fd");
        return EINVAL;
    }

    std::unique_lock<std::mutex> lock(mtx_);
    FileRangeLock fileLock(fd_, 0, 0);
    struct DcacheLookupCtx ctx;
    InitDcacheLookupCtx(&ctx, base, fd_);
    struct HmdfsDentry *de = FindDentry(&ctx);
    if (de == nullptr) {
        LOGD("find dentry failed");
        return ENOENT;
    }

    base.size = de->size;
    base.atime = de->atime;
    base.mtime = de->mtime;
    base.mode = de->mode;
    base.position = MetaHelper::GetPosition(de);
    base.fileType = MetaHelper::GetFileType(de);
    base.noUpload = MetaHelper::GetNoUpload(de);
    base.cloudId = std::string(reinterpret_cast<const char *>(de->recordId), CLOUD_RECORD_ID_LEN);
    return E_OK;
}

int32_t CloudDiskMetaFile::DoUpdate(const MetaBase &base)
{
    if (fd_ < 0) {
        LOGE("bad metafile fd");
        return EINVAL;
    }

    std::unique_lock<std::mutex> lock(mtx_);
    FileRangeLock fileLock(fd_, 0, 0);
    struct DcacheLookupCtx ctx;
    InitDcacheLookupCtx(&ctx, base, fd_);
    struct HmdfsDentry *de = FindDentry(&ctx);
    if (de == nullptr) {
        LOGD("find dentry failed");
        return ENOENT;
    }

    de->atime = base.atime;
    de->mtime = base.mtime;
    de->size = base.size;
    de->mode = base.mode;
    MetaHelper::SetPosition(de, base.position);
    MetaHelper::SetFileType(de, base.fileType);
    MetaHelper::SetNoUpload(de, base.noUpload);
    auto ret = memcpy_s(de->recordId, CLOUD_RECORD_ID_LEN, base.cloudId.c_str(), base.cloudId.length());
    if (ret != 0) {
        LOGE("memcpy_s failed, dstLen = %{public}d, srcLen = %{public}zu", CLOUD_RECORD_ID_LEN, base.cloudId.length());
    }

    off_t ipos = GetDentryGroupPos(ctx.bidx);
    ssize_t size = FileUtils::WriteFile(fd_, ctx.page.get(), ipos, sizeof(struct HmdfsDentryGroup));
    if (size != sizeof(struct HmdfsDentryGroup)) {
        LOGE("write failed, ret = %{public}zd", size);
        return EIO;
    }
    return E_OK;
}

int32_t CloudDiskMetaFile::DoRename(MetaBase &metaBase, const std::string &newName,
    std::shared_ptr<CloudDiskMetaFile> newMetaFile)
{
    std::string oldName = metaBase.name;
    metaBase.name = newName;
    int32_t ret = newMetaFile->DoCreate(metaBase);
    if (ret != E_OK) {
        LOGE("create dentry failed, ret = %{public}d", ret);
        return ret;
    }
    metaBase.name = oldName;
    ret = DoRemove(metaBase);
    if (ret != E_OK) {
        LOGE("remove dentry failed, ret = %{public}d", ret);
        metaBase.name = newName;
        (void)newMetaFile->DoRemove(metaBase);
        return ret;
    }
    return E_OK;
}

static int32_t DecodeDentrys(const HmdfsDentryGroup &dentryGroup, std::vector<MetaBase> &bases)
{
    for (uint32_t i = 0; i < DENTRY_PER_GROUP; i++) {
        int len = dentryGroup.nsl[i].namelen;
        if (!BitOps::TestBit(i, dentryGroup.bitmap) || len == 0 || len >= PATH_MAX) {
            continue;
        }

        std::string name(reinterpret_cast<const char *>(dentryGroup.fileName[i]), len);

        MetaBase base(name);
        base.mode = dentryGroup.nsl[i].mode;
        base.mtime = dentryGroup.nsl[i].mtime;
        base.size = dentryGroup.nsl[i].size;
        base.position = MetaHelper::GetPosition(&dentryGroup.nsl[i]);
        base.fileType = MetaHelper::GetFileType(&dentryGroup.nsl[i]);
        base.cloudId = std::string(reinterpret_cast<const char *>(dentryGroup.nsl[i].recordId), CLOUD_RECORD_ID_LEN);
        bases.emplace_back(base);
    }
    return 0;
}

int32_t CloudDiskMetaFile::LoadChildren(std::vector<MetaBase> &bases)
{
    if (fd_ < 0) {
        LOGE("bad metafile fd");
        return EINVAL;
    }

    std::lock_guard<std::mutex> lock(mtx_);
    FileRangeLock fileLock(fd_, 0, 0);
    struct stat fileStat;
    int ret = fstat(fd_, &fileStat);
    if (ret != E_OK) {
        return EINVAL;
    }

    uint64_t fileSize = static_cast<uint64_t>(fileStat.st_size);
    uint64_t groupCnt = GetDentryGroupCnt(fileSize);
    HmdfsDentryGroup dentryGroup;

    for (uint64_t i = 1; i < groupCnt + 1; i++) {
        uint64_t off = i * sizeof(HmdfsDentryGroup);
        FileUtils::ReadFile(fd_, off, sizeof(HmdfsDentryGroup), &dentryGroup);
        DecodeDentrys(dentryGroup, bases);
    }
    return E_OK;
}

void MetaFileMgr::Clear(uint32_t userId, const std::string &bundleName,
    const std::string &cloudId)
{
    std::lock_guard<std::mutex> lock(cloudDiskMutex_);
    cloudDiskMetaFile_.erase({userId, cloudId + bundleName});
}

void MetaFileMgr::CloudDiskClearAll()
{
    std::lock_guard<std::mutex> lock(cloudDiskMutex_);
    cloudDiskMetaFile_.clear();
}

std::shared_ptr<CloudDiskMetaFile> MetaFileMgr::GetCloudDiskMetaFile(uint32_t userId, const std::string &bundleName,
    const std::string &cloudId)
{
    std::shared_ptr<CloudDiskMetaFile> mFile = nullptr;
    std::lock_guard<std::mutex> lock(cloudDiskMutex_);
    if (cloudDiskMetaFile_.find({userId, cloudId + bundleName}) != cloudDiskMetaFile_.end()) {
        mFile = cloudDiskMetaFile_[{userId, cloudId + bundleName}];
    } else {
        mFile = std::make_shared<CloudDiskMetaFile>(userId, bundleName, cloudId);
        cloudDiskMetaFile_[{userId, cloudId + bundleName}] = mFile;
    }
    return mFile;
}

int32_t MetaFileMgr::CreateRecycleDentry(uint32_t userId, const std::string &bundleName)
{
    MetaBase metaBase(RECYCLE_NAME);
    auto metaFile = MetaFileMgr::GetInstance().GetCloudDiskMetaFile(userId, bundleName, ROOT_CLOUD_ID);
    int32_t ret = metaFile->DoLookup(metaBase);
    if (ret != 0) {
        metaBase.cloudId = RECYCLE_CLOUD_ID;
        metaBase.mode = S_IFDIR | STAT_MODE_DIR;
        metaBase.position = static_cast<uint8_t>(LOCAL);
        ret = metaFile->DoCreate(metaBase);
        if (ret != 0) {
            return ret;
        }
    }
    return 0;
}

int32_t MetaFileMgr::MoveIntoRecycleDentryfile(uint32_t userId, const std::string &bundleName, const std::string &name,
    const std::string &parentCloudId, int64_t rowId)
{
    MetaBase metaBase(name);
    auto srcMetaFile = MetaFileMgr::GetInstance().GetCloudDiskMetaFile(userId, bundleName, parentCloudId);
    auto dstMetaFile = MetaFileMgr::GetInstance().GetCloudDiskMetaFile(userId, bundleName, RECYCLE_CLOUD_ID);
    std::string uniqueName = name + "_" + std::to_string(rowId);
    int32_t ret = srcMetaFile->DoLookup(metaBase);
    if (ret != E_OK) {
        LOGE("lookup src metafile failed, ret = %{public}d", ret);
        return ret;
    }
    metaBase.name = uniqueName;
    ret = dstMetaFile->DoCreate(metaBase);
    if (ret != E_OK) {
        LOGE("lookup and remove dentry failed, ret = %{public}d", ret);
        return ret;
    }
    metaBase.name = name;
    ret = srcMetaFile->DoLookupAndRemove(metaBase);
    if (ret != E_OK) {
        LOGE("lookup and remove dentry failed, ret = %{public}d", ret);
        metaBase.name = uniqueName;
        (void)dstMetaFile->DoLookupAndRemove(metaBase);
        return ret;
    }
    return E_OK;
}

int32_t MetaFileMgr::RemoveFromRecycleDentryfile(uint32_t userId, const std::string &bundleName,
    const std::string &name, const std::string &parentCloudId, int64_t rowId)
{
    auto srcMetaFile = MetaFileMgr::GetInstance().GetCloudDiskMetaFile(userId, bundleName, RECYCLE_CLOUD_ID);
    auto dstMetaFile = MetaFileMgr::GetInstance().GetCloudDiskMetaFile(userId, bundleName, parentCloudId);
    std::string uniqueName = name + "_" + std::to_string(rowId);
    MetaBase metaBase(uniqueName);
    int32_t ret = srcMetaFile->DoLookup(metaBase);
    if (ret != E_OK) {
        LOGE("lookup and update dentry failed, ret = %{public}d", ret);
        return ret;
    }
    metaBase.name = name;
    ret = dstMetaFile->DoCreate(metaBase);
    if (ret != E_OK) {
        LOGE("lookup and remove dentry failed, ret = %{public}d", ret);
        return ret;
    }
    metaBase.name = uniqueName;
    ret = srcMetaFile->DoLookupAndRemove(metaBase);
    if (ret != E_OK) {
        LOGE("lookup and remove dentry failed, ret = %{public}d", ret);
        metaBase.name = name;
        (void)dstMetaFile->DoLookupAndRemove(metaBase);
        return ret;
    }
    return E_OK;
}
} // namespace FileManagement
} // namespace OHOS
