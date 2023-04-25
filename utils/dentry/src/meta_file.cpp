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

#include <fcntl.h>
#include <sstream>
#include <sys/stat.h>

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
constexpr uint64_t DELTA = 0x9E3779B9; /* Hashing code copied from f2fs */
constexpr uint64_t HMDFS_HASH_COL_BIT = (0x1ULL) << 63;

#pragma pack(push, 1)
struct HmdfsDentry {
    uint32_t hash{0};
    uint16_t mode{0};
    uint16_t namelen{0};
    uint64_t size{0};
    uint64_t mtime{0};
    uint8_t recordId[CLOUD_RECORD_ID_LEN]{0};
    /* reserved bytes for long term extend, total 60 bytes */
    uint8_t reserved[DENTRY_RESERVED_LENGTH];
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
    char c;

    while (*kp) {
        c = *kp;
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
    int ret = snprintf_s(buf, fileNameLen, fileNameLen, "cloud_%016llx", fileHash);
    if (ret) {
        LOGE("filename failer ret :%{public}d", ret);
    }
    return buf;
}

static std::string GetDentryfileByPath(uint32_t userId, const std::string &path, bool caseSense = false)
{
    std::string cacheDir =
        "/data/service/el2/" + std::to_string(userId) + "/hmdfs/cache/cloud_cache/dentry_cache/cloud/";
    ForceCreateDirectory(cacheDir);
    std::string dentryFileName = GetDentryfileName(path, caseSense);

    return cacheDir + dentryFileName;
}

MetaFile::MetaFile(uint32_t userId, const std::string &path)
{
    cacheFile_ = GetDentryfileByPath(userId, path);
    fd_ = UniqueFd{open(cacheFile_.c_str(), O_RDWR | O_CREAT)};
    LOGD("fd=%{public}d, errno :%{public}d", fd_.Get(), errno);

    struct stat statBuf {};
    int ret = fstat(fd_, &statBuf);
    LOGD("fd=%{public}d, size=%{public}u, cacheFile_=%s", fd_.Get(), (uint32_t)statBuf.st_size, cacheFile_.c_str());

    ret = fsetxattr(fd_, "user.hmdfs_cache", path.c_str(), path.size(), 0);
    if (ret != 0) {
        LOGE("setxattr failed, errno %{public}d, cacheFile_ %s", errno, cacheFile_.c_str());
    }
}

MetaFile::~MetaFile()
{
    if ((dentryCount_ == 0) && (path_ != "/")) {
        unlink(cacheFile_.c_str());
        // parentMetaFile_->DoRemove()
        return;
    }
}

static bool IsDotDotdot(const std::string &name)
{
    return name == "." || name == "..";
}

static void Str2HashBuf(const char *msg, size_t len, uint32_t *buf, int num)
{
    uint32_t pad = static_cast<uint32_t>(len) | (static_cast<uint32_t>(len) << 8);
    pad |= pad << 16; /* hash pad length 16 */

    uint32_t val = pad;
    len = std::min(len, static_cast<size_t>(num * sizeof(int)));
    for (uint32_t i = 0; i < len; i++) {
        if ((i % sizeof(int)) == 0) {
            val = pad;
        }
        uint8_t c = tolower(msg[i]);
        val = c + (val << 8); /* hash shift size 8 */
        if ((i % 4) == 3) {   /* msg size 4, shift when 3 */
            *buf++ = val;
            val = pad;
            num--;
        }
    }
    if (--num >= 0) {
        *buf++ = val;
    }
    while (--num >= 0) {
        *buf++ = pad;
    }
}

static void TeaTransform(uint32_t buf[4], uint32_t const in[])
{
    int n = 16;           /* transform total rounds 16 */
    uint32_t a = in[0];   /* transform input pos 0 */
    uint32_t b = in[1];   /* transform input pos 1 */
    uint32_t c = in[2];   /* transform input pos 2 */
    uint32_t d = in[3];   /* transform input pos 3 */
    uint32_t b0 = buf[0]; /* buf pos 0 */
    uint32_t b1 = buf[1]; /* buf pos 1 */
    uint32_t sum = 0;

    do {
        sum += DELTA;
        b0 += ((b1 << 4) + a) ^ (b1 + sum) ^ ((b1 >> 5) + b); /* tea transform width 4 and 5 */
        b1 += ((b0 << 4) + c) ^ (b0 + sum) ^ ((b0 >> 5) + d); /* tea transform width 4 and 5 */
    } while (--n);

    buf[0] += b0;
    buf[1] += b1;
}

static uint32_t DentryHash(const std::string &name)
{
    if (IsDotDotdot(name)) {
        return 0;
    }

    constexpr int inLen = 8;      /* hash input buf size 8 */
    constexpr int bufLen = 4;     /* hash output buf size 4 */
    constexpr int hashWidth = 16; /* hash operation width 4 */
    uint32_t in[inLen], buf[bufLen];
    int len = name.length();
    const char *p = name.c_str();

    buf[0] = 0x67452301; /* hash magic 1 */
    buf[1] = 0xefcdab89; /* hash magic 2 */
    buf[2] = 0x98badcfe; /* hash magic 3 */
    buf[3] = 0x10325476; /* hash magic 4 */

    do {
        Str2HashBuf(p, len, in, bufLen);
        TeaTransform(buf, in);
        p += hashWidth;
        len -= hashWidth;
    } while (len > 0);
    uint32_t hash = buf[0];
    uint32_t hmdfsHash = hash & ~HMDFS_HASH_COL_BIT;

    return hmdfsHash;
}

static inline int GetDentrySlots(size_t nameLen)
{
    return (nameLen + BITS_PER_BYTE - 1) >> HMDFS_SLOT_LEN_BITS;
}

static inline off_t GetDentryGroupPos(size_t bidx)
{
    return static_cast<off_t>(bidx) * DENTRYGROUP_SIZE + DENTRYGROUP_HEADER;
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
    uint32_t buckets = (1U << (level + 1)) - 1;
    return buckets;
}

static size_t GetDcacheFileSize(uint32_t level)
{
    size_t buckets = GetOverallBucket(level);
    return buckets * DENTRYGROUP_SIZE * BUCKET_BLOCKS + DENTRYGROUP_HEADER;
}

static uint32_t GetBucketaddr(uint32_t level, uint32_t buckoffset)
{
    if (level >= MAX_BUCKET_LEVEL) {
        LOGI("level = %{public}d overflow", level);
        return 0;
    }

    uint32_t curLevelMaxBucks = (1U << level);
    if (buckoffset >= curLevelMaxBucks) {
        LOGI("buckoffset %{public}d overflow, level %{public}d has %{public}d buckets max", buckoffset, level,
             curLevelMaxBucks);
        return 0;
    }

    return curLevelMaxBucks + buckoffset - 1;
}

static uint32_t GetBucketByLevel(uint32_t level)
{
    if (level >= MAX_BUCKET_LEVEL) {
        LOGI("level = %{public}d overflow", level);
        return 0;
    }

    uint32_t buckets = (1U << level);
    return buckets;
}

static int RoomForFilename(const uint8_t bitmap[], int slots, int maxSlots)
{
    int bitStart = 0;

    while (1) {
        int zeroStart = BitOps::FindNextZeroBit(bitmap, maxSlots, bitStart);
        if (zeroStart >= maxSlots) {
            return maxSlots;
        }

        int zeroEnd = BitOps::FindNextBit(bitmap, maxSlots, zeroStart);
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
    int slots = GetDentrySlots(name.length());

    de = &d.nsl[bitPos];
    de->hash = nameHash;
    de->namelen = name.length();
    if (memcpy_s(d.fileName[bitPos], slots * DENTRY_NAME_LEN, name.c_str(), name.length())) {
        LOGE("memcpy_s failed, dstLen = %{public}d, srcLen = %{public}zu", slots * DENTRY_NAME_LEN, name.length());
    }
    de->mtime = base.mtime;
    de->size = base.size;
    de->mode = base.mode;
    if (memcpy_s(de->recordId, CLOUD_RECORD_ID_LEN, base.cloudId.c_str(), base.cloudId.length())) {
        LOGE("memcpy_s failed, dstLen = %{public}d, srcLen = %{public}zu", CLOUD_RECORD_ID_LEN, base.cloudId.length());
    }

    for (int i = 0; i < slots; i++) {
        BitOps::SetBit(bitPos + i, d.bitmap);
        if (i) {
            (de + i)->namelen = 0;
        }
    }
}

int32_t MetaFile::DoCreate(const MetaBase &base)
{
    if (fd_ < 0) {
        LOGE("bad metafile fd");
        return EINVAL;
    }

    off_t pos = 0;
    uint32_t level = 0;
    uint32_t bitPos, namehash;
    unsigned long bidx, endBlock;
    HmdfsDentryGroup dentryBlk = {0};

    namehash = DentryHash(base.name);

    bool found = false;
    while (!found) {
        if (level == MAX_BUCKET_LEVEL) {
            return ENOSPC;
        }
        bidx = BUCKET_BLOCKS * GetBucketaddr(level, namehash % GetBucketByLevel(level));
        endBlock = bidx + BUCKET_BLOCKS;

        struct stat fileStat;
        int err = fstat(fd_, &fileStat);
        if (err < 0) {
            return EINVAL;
        }
        if ((endBlock > GetDentryGroupCnt(fileStat.st_size)) &&
            ftruncate(fd_, GetDcacheFileSize(level))) {
            return ENOENT;
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
        LOGI("WriteFile failed, size %{public}d != %{public}d", size, DENTRYGROUP_SIZE);
        return EINVAL;
    }

    ++dentryCount_;
    return E_OK;
}

int32_t MetaFile::DoLookup(MetaBase &base)
{
    if (fd_ < 0) {
        LOGE("bad metafile fd");
        return EINVAL;
    }

    return E_OK;
}

int32_t MetaFile::DoRemove(const MetaBase &base)
{
    if (fd_ < 0) {
        LOGE("bad metafile fd");
        return EINVAL;
    }

    return E_OK;
}

int32_t MetaFile::DoUpdate(const MetaBase &base)
{
    if (fd_ < 0) {
        LOGE("bad metafile fd");
        return EINVAL;
    }

    return E_OK;
}

int32_t MetaFile::DoRename(MetaBase &oldbase, MetaBase &newbase)
{
    if (fd_ < 0) {
        LOGE("bad metafile fd");
        return EINVAL;
    }

    return E_OK;
}
} // namespace FileManagement
} // namespace OHOS
