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
constexpr uint32_t DENTRY_RESERVED_LENGTH = 4;
constexpr uint32_t DENTRY_PER_GROUP = 52;
constexpr uint32_t DENTRY_BITMAP_LENGTH = 7;
constexpr uint32_t DENTRY_GROUP_RESERVED = 32;
constexpr uint32_t CLOUD_RECORD_ID_LEN = 33;
constexpr uint32_t CLOUD_ID_BUCKET_MAX_SIZE = 32;
constexpr uint32_t CLOUD_ID_BUCKET_MID_TIMES = 2;
constexpr uint32_t CLOUD_ID_MIN_SIZE = 3;
constexpr uint32_t FILE_TYPE_OFFSET = 2;

#pragma pack(push, 1)
struct HmdfsDentry {
    uint32_t hash{0};
    uint16_t mode{0};
    uint16_t namelen{0};
    uint64_t size{0};
    uint64_t atime{0};
    uint64_t mtime{0};
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
    de->flags &= 0x3;
    de->flags |= (fileType << FILE_TYPE_OFFSET);
}

void MetaHelper::SetPosition(struct HmdfsDentry *de, uint8_t position)
{
    de->flags &= 0xFC;
    de->flags |= position;
}

uint8_t MetaHelper::GetFileType(const struct HmdfsDentry *de)
{
    return (de->flags & 0xFC) >> FILE_TYPE_OFFSET;
}

uint8_t MetaHelper::GetPosition(const struct HmdfsDentry *de)
{
    return de->flags & 0x3;
}

static uint32_t GetBucketId(std::string cloudId)
{
    size_t size = cloudId.size();
    if (size < CLOUD_ID_MIN_SIZE) {
        return 0;
    }

    char first = cloudId[0];
    char last = cloudId[size - 1];
    char middle = cloudId[size / CLOUD_ID_BUCKET_MID_TIMES];
    return (first + last + middle) % CLOUD_ID_BUCKET_MAX_SIZE;
}

static std::string GetCloudDiskDentryFileByPath(uint32_t userId, const std::string &bundleName,
    const std::string &cloudId)
{
    std::string cacheDir =
        "/data/service/el2/" + std::to_string(userId) +
        "/hmdfs/cloud/data/" + bundleName + "/" + std::to_string(GetBucketId(cloudId)) + "/";
    std::string dentryFileName = MetaFileMgr::GetInstance().CloudIdToRecordId(cloudId);
    ForceCreateDirectory(cacheDir);
    return cacheDir + dentryFileName;
}

CloudDiskMetaFile::CloudDiskMetaFile(uint32_t userId, const std::string &bundleName, const std::string &cloudId)
{
    userId_ = userId;
    bundleName_ = bundleName;
    cloudId_ = cloudId;
    cacheFile_ = GetCloudDiskDentryFileByPath(userId_, bundleName_, cloudId_);
    fd_ = UniqueFd{open(cacheFile_.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)};
    LOGD("CloudDiskMetaFile cloudId=%{public}s, path=%{public}s", cloudId_.c_str(), cacheFile_.c_str());
    LOGD("CloudDiskMetaFile fd=%{public}d, errno :%{public}d", fd_.Get(), errno);

    HmdfsDcacheHeader header{};
    (void)FileUtils::ReadFile(fd_, 0, sizeof(header), &header);
    dentryCount_ = header.dentryCount;
}

int32_t CloudDiskMetaFile::DoLookupAndUpdate(const std::string &name, CloudDiskMetaFileCallBack callback)
{
    return E_OK;
}

int32_t CloudDiskMetaFile::DoLookupAndRemove(MetaBase &metaBase)
{
    return 0;
}

CloudDiskMetaFile::~CloudDiskMetaFile()
{
    HmdfsDcacheHeader header{.dentryCount = dentryCount_};
    (void)FileUtils::WriteFile(fd_, &header, 0, sizeof(header));
}

int32_t CloudDiskMetaFile::HandleFileByFd(unsigned long &endBlock, uint32_t &level)
{
    return E_OK;
}

int32_t CloudDiskMetaFile::DoCreate(const MetaBase &base)
{
    return E_OK;
}

struct DcacheLookupCtx {
    int fd{-1};
    std::string name{};
    uint32_t hash{0};
    uint32_t bidx{0};
    std::unique_ptr<HmdfsDentryGroup> page{nullptr};
};

int32_t CloudDiskMetaFile::DoRemove(const MetaBase &base)
{
    return E_OK;
}

int32_t CloudDiskMetaFile::DoLookup(MetaBase &base)
{
    return E_OK;
}

int32_t CloudDiskMetaFile::DoUpdate(const MetaBase &base)
{
    return E_OK;
}

int32_t CloudDiskMetaFile::DoRename(MetaBase &metaBase, const std::string &newName,
    std::shared_ptr<CloudDiskMetaFile> newMetaFile)
{
    return E_OK;
}

std::shared_ptr<CloudDiskMetaFile> MetaFileMgr::GetCloudDiskMetaFile(uint32_t userId, const std::string &bundleName,
    const std::string &cloudId)
{
    return nullptr;
}

int32_t MetaFileMgr::MoveIntoRecycleDentryfile(uint32_t userId, const std::string &bundleName, const std::string &name,
    const std::string &parentCloudId, int64_t rowId)
{
    return E_OK;
}

int32_t MetaFileMgr::RemoveFromRecycleDentryfile(uint32_t userId, const std::string &bundleName,
    const std::string &name, const std::string &parentCloudId, int64_t rowId)
{
    return E_OK;
}
} // namespace FileManagement
} // namespace OHOS
