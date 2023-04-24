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

MetaFile::~MetaFile() {}

int32_t MetaFile::DoCreate(const MetaBase &base)
{
    if (fd_ < 0) {
        LOGE("bad metafile fd");
        return EINVAL;
    }

    return E_OK;
}
} // namespace FileManagement
} // namespace OHOS
