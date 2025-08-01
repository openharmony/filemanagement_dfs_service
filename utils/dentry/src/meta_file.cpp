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

#include "meta_file.h"

#include <ctime>
#include <fcntl.h>
#include <iomanip>
#include <sstream>
#include <sys/stat.h>

#include "cloud_file_utils.h"
#include "dfs_error.h"
#include "directory_ex.h"
#include "file_utils.h"
#include "securec.h"
#include "string_ex.h"
#include "sys/xattr.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
constexpr uint32_t DENTRYGROUP_SIZE = 4096;
constexpr uint32_t DENTRY_NAME_LEN = 8;
constexpr uint32_t DENTRY_RESERVED_LENGTH = 3;
constexpr uint32_t DENTRY_PER_GROUP = 60;
constexpr uint32_t DENTRY_BITMAP_LENGTH = 8;
constexpr uint32_t DENTRY_GROUP_RESERVED = 7;
constexpr uint32_t CLOUD_RECORD_ID_LEN = 33;
constexpr uint32_t DENTRYGROUP_HEADER = 4096;
constexpr uint32_t MAX_BUCKET_LEVEL = 63;
constexpr uint32_t BUCKET_BLOCKS = 2;
constexpr uint32_t BITS_PER_BYTE = 8;
constexpr uint32_t HMDFS_SLOT_LEN_BITS = 3;
constexpr uint32_t DIR_SIZE = 4096;
constexpr uint32_t EVEN_NUM_FLAG = 2;
const char HDC_ID_START = '0';
const std::string HDC_ID_END = "ff";

#pragma pack(push, 1)
struct HmdfsDentry {
    uint32_t hash{0};
    uint16_t mode{0};
    uint16_t namelen{0};
    uint64_t size{0};
    uint64_t mtime{0};
    uint8_t recordId[CLOUD_RECORD_ID_LEN]{0};
    /* reserved bytes for long term extend, total 60 bytes */
    union {
        struct {
            uint8_t fileType : 2;
        };
        uint8_t reserved[DENTRY_RESERVED_LENGTH];
    };
};

struct HmdfsDentryGroup {
    uint8_t dentryVersion;
    uint8_t bitmap[DENTRY_BITMAP_LENGTH];
    HmdfsDentry nsl[DENTRY_PER_GROUP];
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

static uint64_t PathHash(const std::string &path, bool caseSense)
{
    uint64_t res = 0;
    const char *kp = path.c_str();

    while (*kp) {
        char c = *kp;
        if (!caseSense) {
            c = tolower(c);
        }
        res = (res << 5) - res + static_cast<uint64_t>(c); /* hash shift width 5 */
        kp++;
    }
    return res;
}

static std::string GetDentryfileName(const std::string &path, bool caseSense)
{
    // path should be like "/", "/dir/", "/dir/dir/" ...
    constexpr uint32_t fileNameLen = 32;
    char buf[fileNameLen + 1] = {0};
    uint64_t fileHash = PathHash(path, caseSense);
    int ret = snprintf_s(buf, fileNameLen + 1, fileNameLen, "cloud_%016llx", fileHash);
    if (ret < 0) {
        LOGE("filename failer fileHash ret :%{public}d", ret);
    }
    return buf;
}

static std::string GetDentryfileByPath(uint32_t userId, const std::string &path, bool caseSense = false)
{
    std::string cacheDir =
        "/data/service/el2/" + std::to_string(userId) + "/hmdfs/cache/account_cache/dentry_cache/cloud/";
    std::string dentryFileName = GetDentryfileName(path, caseSense);

    return cacheDir + dentryFileName;
}

std::string MetaFile::GetParentDir(const std::string &path)
{
    if ((path == "/") || (path == "")) {
        return "";
    }

    auto pos = path.find_last_of('/');
    if ((pos == std::string::npos) || (pos == 0)) {
        return "/";
    }

    return path.substr(0, pos);
}

std::string MetaFile::GetFileName(const std::string &path)
{
    if ((path == "/") || (path == "")) {
        return "";
    }

    auto pos = path.find_last_of('/');
    if (pos == std::string::npos) {
        return "";
    }

    return path.substr(pos + 1);
}

static std::shared_ptr<MetaFile> GetParentMetaFile(uint32_t userId, const std::string &path)
{
    std::string parentPath = MetaFile::GetParentDir(path);
    if (parentPath == "") {
        return nullptr;
    }

    return MetaFileMgr::GetInstance().GetMetaFile(userId, parentPath);
}

MetaFile::MetaFile(uint32_t userId, const std::string &path)
{
    path_ = path;
    cacheFile_ = GetDentryfileByPath(userId, path);
    fd_ = UniqueFd{open(cacheFile_.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)};
    if (fd_ < 0) {
        LOGE("fd=%{public}d, errno :%{public}d", fd_.Get(), errno);
        return;
    }

    int ret = fsetxattr(fd_, "user.hmdfs_cache", path.c_str(), path.size(), 0);
    if (ret != 0) {
        LOGE("setxattr failed, errno %{public}d, cacheFile_ %s", errno, GetAnonyString(cacheFile_).c_str());
    }

    /* lookup and create in parent */
    parentMetaFile_ = GetParentMetaFile(userId, path);
    std::string dirName = GetFileName(path);
    if ((parentMetaFile_ == nullptr) || (dirName == "")) {
        return;
    }
    MetaBase m(dirName, std::to_string(PathHash(path, false)) + std::to_string(std::time(nullptr)));
    ret = parentMetaFile_->DoLookup(m);
    if (ret != E_OK) {
        m.mode = S_IFDIR;
        m.size = DIR_SIZE;
        ret = parentMetaFile_->DoCreate(m);
        if (ret != E_OK) {
            LOGE("create parent failed, ret %{public}d", ret);
        }
    }
}

MetaFile::~MetaFile()
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
        LOGI("level = %{public}d overflow", level);
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
        LOGI("level = %{public}d overflow", level);
        return 0;
    }

    uint64_t buckets = (1ULL << level);
    return static_cast<uint32_t>(buckets);
}

static uint32_t RoomForFilename(const uint8_t bitmap[], size_t slots, uint32_t maxSlots)
{
    uint32_t bitStart = 0;

    while (1) {
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

static bool UpdateDentry(HmdfsDentryGroup &d, const MetaBase &base, uint32_t nameHash, uint32_t bitPos)
{
    HmdfsDentry *de;
    const std::string name = base.name;
    uint32_t slots = GetDentrySlots(name.length());

    de = &d.nsl[bitPos];
    de->hash = nameHash;
    de->namelen = name.length();
    errno_t ret = memcpy_s(d.fileName[bitPos], slots * DENTRY_NAME_LEN, name.c_str(), name.length());
    if (ret != EOK) {
        LOGE("memcpy_s failed, dstLen = %{public}d, srcLen = %{public}zu", slots * DENTRY_NAME_LEN, name.length());
        return false;
    }
    de->mtime = base.mtime;
    de->fileType = base.fileType;
    de->size = base.size;
    de->mode = base.mode;
    (void)memset_s(de->recordId, CLOUD_RECORD_ID_LEN, 0, CLOUD_RECORD_ID_LEN);
    ret = memcpy_s(de->recordId, CLOUD_RECORD_ID_LEN, base.cloudId.c_str(), base.cloudId.length());
    if (ret != EOK) {
        LOGE("memcpy_s failed, dstLen = %{public}d, srcLen = %{public}zu", CLOUD_RECORD_ID_LEN, base.cloudId.length());
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

int32_t MetaFile::HandleFileByFd(unsigned long &endBlock, uint32_t &level)
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

int32_t MetaFile::DoCreate(const MetaBase &base)
{
    if (fd_ < 0) {
        LOGE("bad metafile fd");
        return EINVAL;
    }

    off_t pos = 0;
    uint32_t level = 0;
    uint32_t bitPos = 0;
    unsigned long bidx;
    HmdfsDentryGroup dentryBlk = {0};

    std::unique_lock<std::mutex> lock(mtx_);
    uint32_t namehash = CloudDisk::CloudFileUtils::DentryHash(base.name);

    bool found = false;
    while (!found) {
        if (level == MAX_BUCKET_LEVEL) {
            return ENOSPC;
        }
        bidx = BUCKET_BLOCKS * GetBucketaddr(level, namehash % GetBucketByLevel(level));
        unsigned long endBlock = bidx + BUCKET_BLOCKS;

        int32_t ret = MetaFile::HandleFileByFd(endBlock, level);
        if (ret != E_OK) {
            return ret;
        }

        for (; bidx < endBlock; bidx++) {
            pos = GetDentryGroupPos(bidx);
            if (FileUtils::ReadFile(fd_, pos, DENTRYGROUP_SIZE, &dentryBlk) != DENTRYGROUP_SIZE) {
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
    if (!UpdateDentry(dentryBlk, base, namehash, bitPos)) {
        LOGI("UpdateDentry fail, stop write.");
        return EINVAL;
    }
    int size = FileUtils::WriteFile(fd_, &dentryBlk, pos, DENTRYGROUP_SIZE);
    if (size != DENTRYGROUP_SIZE) {
        LOGI("WriteFile failed, size %{public}d != %{public}d", size, DENTRYGROUP_SIZE);
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
    if (!nbucket) {
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

static uint32_t GetMaxLevel(int32_t fd)
{
    struct stat st;
    if (fstat(fd, &st) == -1) {
        return MAX_BUCKET_LEVEL;
    }
    uint32_t blkNum = static_cast<uint32_t>(st.st_size) / DENTRYGROUP_SIZE + 1;
    uint32_t maxLevel = 0;
    blkNum >>= 1;
    while (blkNum > 1) {
        blkNum >>= 1;
        maxLevel++;
    }
    return maxLevel;
}

static HmdfsDentry *FindDentry(DcacheLookupCtx *ctx)
{
    uint32_t maxLevel = GetMaxLevel(ctx->fd);
    for (uint32_t level = 0; level < maxLevel; level++) {
        HmdfsDentry *de = InLevel(level, ctx);
        if (de != nullptr) {
            return de;
        }
    }
    return nullptr;
}

int32_t MetaFile::DoRemove(const MetaBase &base)
{
    if (fd_ < 0) {
        LOGE("bad metafile fd");
        return EINVAL;
    }

    std::unique_lock<std::mutex> lock(mtx_);
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

int32_t MetaFile::DoLookup(MetaBase &base)
{
    if (fd_ < 0) {
        LOGE("bad metafile fd");
        return EINVAL;
    }

    std::unique_lock<std::mutex> lock(mtx_);
    struct DcacheLookupCtx ctx;
    InitDcacheLookupCtx(&ctx, base, fd_);
    struct HmdfsDentry *de = FindDentry(&ctx);
    if (de == nullptr) {
        LOGD("find dentry failed");
        return ENOENT;
    }

    base.size = de->size;
    base.mtime = de->mtime;
    base.mode = de->mode;
    base.fileType = de->fileType;
    base.cloudId = std::string(reinterpret_cast<const char *>(de->recordId), CLOUD_RECORD_ID_LEN - 1);

    return E_OK;
}

int32_t MetaFile::DoUpdate(const MetaBase &base)
{
    if (fd_ < 0) {
        LOGE("bad metafile fd");
        return EINVAL;
    }

    std::unique_lock<std::mutex> lock(mtx_);
    struct DcacheLookupCtx ctx;
    InitDcacheLookupCtx(&ctx, base, fd_);
    struct HmdfsDentry *de = FindDentry(&ctx);
    if (de == nullptr) {
        LOGI("find dentry failed");
        return ENOENT;
    }

    de->mtime = base.mtime;
    de->size = base.size;
    de->mode = base.mode;
    de->fileType = base.fileType;
    if (memcpy_s(de->recordId, CLOUD_RECORD_ID_LEN, base.cloudId.c_str(), base.cloudId.length())) {
        LOGE("memcpy_s failed, dstLen = %{public}d, srcLen = %{public}zu", CLOUD_RECORD_ID_LEN, base.cloudId.length());
    }

    off_t ipos = GetDentryGroupPos(ctx.bidx);
    ssize_t size = FileUtils::WriteFile(fd_, ctx.page.get(), ipos, sizeof(struct HmdfsDentryGroup));
    if (size != sizeof(struct HmdfsDentryGroup)) {
        LOGI("write failed, ret = %{public}zd", size);
        return EIO;
    }
    return E_OK;
}

int32_t MetaFile::DoRename(const MetaBase &oldBase, const std::string &newName)
{
    MetaBase base{oldBase.name};
    int32_t ret = DoLookup(base);
    if (ret) {
        LOGE("ret = %{public}d, lookup %s failed", ret, GetAnonyString(base.name).c_str());
        return ret;
    }

    base.name = newName;
    ret = DoCreate(base);
    if (ret) {
        LOGE("ret = %{public}d, create %s failed", ret, GetAnonyString(base.name).c_str());
        return ret;
    }

    base.name = oldBase.name;
    ret = DoRemove(oldBase);
    if (ret) {
        LOGE("ret = %{public}d, remove %s failed", ret, GetAnonyString(oldBase.name).c_str());
        base.name = newName;
        (void)DoRemove(base);
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
        base.fileType = dentryGroup.nsl[i].fileType;
        base.cloudId = std::string(reinterpret_cast<const char *>(dentryGroup.nsl[i].recordId), CLOUD_RECORD_ID_LEN);
        bases.emplace_back(base);
    }
    return 0;
}

int32_t MetaFile::LoadChildren(std::vector<MetaBase> &bases)
{
    if (fd_ < 0) {
        LOGE("bad metafile fd");
        return EINVAL;
    }

    std::lock_guard<std::mutex> lock(mtx_);
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

MetaFileMgr& MetaFileMgr::GetInstance()
{
    static MetaFileMgr instance_;
    return instance_;
}

std::shared_ptr<MetaFile> MetaFileMgr::GetMetaFile(uint32_t userId, const std::string &path)
{
    std::shared_ptr<MetaFile> mFile = nullptr;
    std::lock_guard<std::recursive_mutex> lock(mtx_);

    MetaFileKey key(userId, path);
    auto it = metaFiles_.find(key);
    if (it != metaFiles_.end()) {
        metaFileList_.splice(metaFileList_.begin(), metaFileList_, it->second);
        mFile = it->second->second;
    } else {
        mFile = std::make_shared<MetaFile>(userId, path);
        while (metaFiles_.size() >= MAX_META_FILE_NUM) {
            auto deleteKey = metaFileList_.back().first;
            metaFiles_.erase(deleteKey);
            metaFileList_.pop_back();
        }
        metaFileList_.emplace_front(key, mFile);
        metaFiles_[key] = metaFileList_.begin();
    }
    return mFile;
}

int32_t MetaFileMgr::CheckMetaFileSize()
{
    std::lock_guard<std::recursive_mutex> lock(mtx_);
    if (metaFiles_.size() != metaFileList_.size()) {
        return -1;
    }
    return metaFileList_.size();
}

void MetaFileMgr::ClearAll()
{
    std::lock_guard<std::recursive_mutex> lock(mtx_);
    metaFiles_.clear();
    metaFileList_.clear();
}

std::string MetaFileMgr::RecordIdToCloudId(const std::string hexStr, bool isHdc)
{
    std::string srcStr = hexStr;
    if (isHdc) {
        if (srcStr.size() % EVEN_NUM_FLAG != 0) {
            srcStr = HDC_ID_START + srcStr + HDC_ID_END;
        } else {
            srcStr = srcStr + HDC_ID_END;
        }
    }
    std::string result;
    constexpr std::size_t offset = 2;
    constexpr int changeBase = 16;
    for (std::size_t i = 0; i < srcStr.length(); i += offset) {
        std::string hexByte = srcStr.substr(i, offset);
        char *endPtr;
        unsigned long hexValue = std::strtoul(hexByte.c_str(), &endPtr, changeBase);

        if (endPtr != hexByte.c_str() + hexByte.length()) {
            LOGE("Invalid hexadecimal string: %{public}s", srcStr.c_str());
            return "";
        }
        result += static_cast<char>(hexValue);
    }
    if (result.size() > CLOUD_RECORD_ID_LEN) {
        LOGE("Invalid result length %{public}zu", result.size());
        return "";
    }

    return result;
}

/**
 * @brief convert cloudId to recordId
 *
 * @param cloudId
 * @param isHdc is homedatacenter's cloudId
 * @return string recordId
 * @example recordId: 012345678ff -> 12345678 when isHdc=true
 */
std::string MetaFileMgr::CloudIdToRecordId(const std::string cloudId, bool isHdc)
{
    std::stringstream result;
    constexpr int width = 2;
    for (std::size_t i = 0; i < cloudId.length(); i++) {
        uint8_t u8Byte = cloudId[i];
        result << std::setw(width) << std::setfill('0') << std::hex << static_cast<int>(u8Byte);
    }
    if (!isHdc) {
        return result.str();
    }
    std::string recordId = result.str();
    if (recordId.at(0) == HDC_ID_START) {
        recordId = recordId.substr(1);
    }
    auto pos = recordId.find(HDC_ID_END);
    if (pos == std::string::npos) {
        return recordId;
    }
    return recordId.substr(0, pos);
}

} // namespace FileManagement
} // namespace OHOS
