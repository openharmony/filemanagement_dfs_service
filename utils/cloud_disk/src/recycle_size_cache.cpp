/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include "recycle_size_cache.h"

#include <cctype>
#include <cerrno>
#include <charconv>
#include <cstdint>
#include <fcntl.h>
#include <functional>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "dfs_error.h"
#include "dfsu_fd_guard.h"
#include "ffrt_inner.h"
#include "meta_file.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudDisk {
using OHOS::Storage::DistributedFile::DfsuFDGuard;
namespace {
    static const int32_t OID_FILE_MANAGER = 1006;
    static const int32_t OID_DFS = 1009;
    static const mode_t FILE_MODE = 0660;
    static const size_t SIZE_BUF_LEN = 32;

    bool IsValidParams(int32_t userId, const std::string &bundleName)
    {
        if (userId < 0 || bundleName.empty()) {
            return false;
        }
        if (bundleName.find('/') != std::string::npos || bundleName.find("..") != std::string::npos) {
            return false;
        }
        return true;
    }

    bool IsFilteredMetaBase(const MetaBase &metaBase)
    {
        return (metaBase.position & POSITION_LOCAL) == 0 || S_ISDIR(metaBase.mode);
    }
}

std::mutex RecycleSizeCache::gMutex_;
constexpr size_t RecycleSizeCache::kVersionSlotCount;
std::atomic<int64_t> RecycleSizeCache::gVersionSlots_[RecycleSizeCache::kVersionSlotCount] = {};

int32_t RecycleSizeCache::GetRecycleBinSize(int32_t userId, const std::string &bundleName, int64_t &size)
{
    if (!IsValidParams(userId, bundleName)) {
        LOGE("invalid recycle size params");
        return E_INVAL_ARG;
    }
    std::lock_guard<std::mutex> lock(gMutex_);
    return ReadCachedSize(GetCacheFilePath(userId, bundleName), size);
}

int32_t RecycleSizeCache::IncreaseRecycleBinSize(int32_t userId, const std::string &bundleName,
    const MetaBase &metaBase)
{
    if (!IsValidParams(userId, bundleName)) {
        LOGE("invalid increase params");
        return E_INVAL_ARG;
    }
    if (IsFilteredMetaBase(metaBase)) {
        return E_OK;
    }
    int64_t delta = static_cast<int64_t>(metaBase.size);
    if (delta <= 0) {
        return E_OK;
    }
    std::string path = GetCacheFilePath(userId, bundleName);
    int64_t myVersion = GetCacheVersion(path);
    ffrt::thread([path, delta, myVersion] {
        std::lock_guard<std::mutex> lock(gMutex_);
        if (GetCacheVersion(path) != myVersion) {
            LOGD("increase discarded, cache version changed, key:%{private}s, myVersion:%{public}lld",
                path.c_str(), static_cast<long long>(myVersion));
            return;
        }
        int64_t cur = 0;
        if (ReadCachedSize(path, cur) != E_OK) {
            LOGE("increase read cache size failed, key:%{private}s", path.c_str());
            return;
        }
        WriteCachedSize(path, cur + delta);
    }).detach();
    return E_OK;
}

int32_t RecycleSizeCache::DecreaseRecycleBinSize(int32_t userId, const std::string &bundleName,
    const MetaBase &metaBase)
{
    if (!IsValidParams(userId, bundleName)) {
        LOGE("invalid decrease params");
        return E_INVAL_ARG;
    }
    if (IsFilteredMetaBase(metaBase)) {
        return E_OK;
    }
    int64_t delta = static_cast<int64_t>(metaBase.size);
    if (delta <= 0) {
        return E_OK;
    }
    std::string path = GetCacheFilePath(userId, bundleName);
    int64_t myVersion = GetCacheVersion(path);
    ffrt::thread([path, delta, myVersion] {
        std::lock_guard<std::mutex> lock(gMutex_);
        if (GetCacheVersion(path) != myVersion) {
            LOGD("decrease discarded, cache version changed, key:%{private}s, myVersion:%{public}lld",
                path.c_str(), static_cast<long long>(myVersion));
            return;
        }
        int64_t cur = 0;
        if (ReadCachedSize(path, cur) != E_OK) {
            LOGE("decrease read cache size failed, key:%{private}s", path.c_str());
            return;
        }
        cur -= delta;
        if (cur < 0) {
            cur = 0;
        }
        WriteCachedSize(path, cur);
    }).detach();
    return E_OK;
}

int32_t RecycleSizeCache::ResetRecycleBinSize(int32_t userId, const std::string &bundleName)
{
    if (!IsValidParams(userId, bundleName)) {
        LOGE("invalid reset params");
        return E_INVAL_ARG;
    }
    std::string path = GetCacheFilePath(userId, bundleName);
    std::lock_guard<std::mutex> lock(gMutex_);
    AddCacheVersion(path);
    return WriteCachedSize(path, 0);
}

std::string RecycleSizeCache::GetCacheFilePath(int32_t userId, const std::string &bundleName)
{
    return "/data/service/el2/" + std::to_string(userId) +
        "/hmdfs/cloudfile_manager/" + bundleName + "/RecycleSizeCache";
}

int32_t RecycleSizeCache::ReadCachedSize(const std::string &path, int64_t &size)
{
    size = 0;
    DfsuFDGuard fdGuard(open(path.c_str(), O_RDONLY | O_NOFOLLOW | O_CLOEXEC));
    if (!fdGuard) {
        int err = errno;
        if (err == ENOENT) {
            return E_OK;
        }
        LOGE("open cache file for read failed, errno:%{public}d", err);
        return E_PATH;
    }
    char buf[SIZE_BUF_LEN] = {0};
    ssize_t n = read(fdGuard.GetFD(), buf, sizeof(buf) - 1);
    if (n < 0) {
        LOGE("read cache file failed, errno:%{public}d", errno);
        return E_PATH;
    }
    const char *first = buf;
    const char *last = buf + n;
    while (first < last && std::isspace(static_cast<unsigned char>(*first))) {
        ++first;
    }
    if (first == last) {
        return E_OK;
    }
    auto res = std::from_chars(first, last, size);
    if (res.ec != std::errc{}) {
        LOGE("parse cache size failed, value:%{public}s", buf);
        size = 0;
    }
    if (size < 0) {
        size = 0;
    }
    return E_OK;
}

int32_t RecycleSizeCache::OpenAndCheckCacheFile(const std::string &path, bool &isNewFile,
    DfsuFDGuard &fdGuard)
{
    isNewFile = false;
    fdGuard.SetFD(open(path.c_str(),
        O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, FILE_MODE));
    if (!fdGuard) {
        if (errno != EEXIST) {
            LOGE("open cache file for write failed, errno:%{public}d", errno);
            return E_PATH;
        }
        fdGuard.SetFD(open(path.c_str(), O_WRONLY | O_TRUNC | O_NOFOLLOW | O_CLOEXEC));
        if (!fdGuard) {
            LOGE("open cache file for write failed, errno:%{public}d", errno);
            return E_PATH;
        }
    } else {
        isNewFile = true;
    }
    struct stat st = {};
    if (fstat(fdGuard.GetFD(), &st) != 0) {
        LOGE("fstat cache file failed, errno:%{public}d", errno);
        return E_PATH;
    }
    if (!S_ISREG(st.st_mode) || st.st_nlink != 1) {
        LOGE("cache file check failed, mode:%{public}o, nlink:%{public}lu", st.st_mode,
            static_cast<unsigned long>(st.st_nlink));
        return E_PATH;
    }
    return E_OK;
}

int32_t RecycleSizeCache::WriteCachedSize(const std::string &path, int64_t size)
{
    if (size < 0) {
        size = 0;
    }
    bool isNewFile = false;
    DfsuFDGuard fdGuard;
    int32_t ret = OpenAndCheckCacheFile(path, isNewFile, fdGuard);
    if (ret != E_OK) {
        return ret;
    }
    std::string val = std::to_string(size);
    ssize_t w = write(fdGuard.GetFD(), val.data(), val.size());
    if (w != static_cast<ssize_t>(val.size())) {
        LOGE("write cache size failed, errno:%{public}d", errno);
        return E_PATH;
    }
    if (isNewFile) {
        fchmod(fdGuard.GetFD(), FILE_MODE);
        fchown(fdGuard.GetFD(), OID_FILE_MANAGER, OID_DFS);
    }
    return E_OK;
}

size_t RecycleSizeCache::GetVersionSlot(const std::string &key)
{
    return std::hash<std::string>{}(key) & (kVersionSlotCount - 1);
}

int64_t RecycleSizeCache::GetCacheVersion(const std::string &key)
{
    int64_t version = gVersionSlots_[GetVersionSlot(key)].load();
    LOGD("get cache version, key:%{private}s, version:%{public}lld", key.c_str(),
        static_cast<long long>(version));
    return version;
}

void RecycleSizeCache::AddCacheVersion(const std::string &key)
{
    gVersionSlots_[GetVersionSlot(key)].fetch_add(1);
}
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS
