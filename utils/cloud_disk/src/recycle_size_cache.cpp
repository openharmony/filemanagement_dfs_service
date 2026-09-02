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
#include <cstring>
#include <filesystem>
#include <fcntl.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/xattr.h>
#include <unistd.h>

#include "dfs_error.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudDisk {
namespace {
    static const std::string POSITION_XATTR = "user.cloud.location";

    static const int32_t POSITION_CLOUD = 2;

    static const int32_t OID_FILE_MANAGER = 1006;
    static const int32_t OID_DFS = 1009;
    static const mode_t FILE_MODE = 0660;
    static const size_t SIZE_BUF_LEN = 32;
    static const size_t XATTR_BUF_LEN = 32;
}

std::string RecycleSizeCache::GetCacheFilePath(int32_t userId, const std::string &bundleName)
{
    return "/data/service/el2/" + std::to_string(userId) + "/hmdfs/cloudfile_manager/" + bundleName + "/RecycleSizeCache";
}

std::string RecycleSizeCache::GetTrashDir(int32_t userId, const std::string &bundleName)
{
    return "/mnt/hmdfs/" + std::to_string(userId) + "/cloud/data/" + bundleName + "/.trash";
}

int32_t RecycleSizeCache::ReadCachedSize(const std::string &path, int64_t &size)
{
    size = 0;
    int fd = open(path.c_str(), O_RDONLY);
    if (fd < 0) {
        if (errno == ENOENT) {
            return E_OK;
        }
        LOGE("open cache file for read failed, errno:%{public}d", errno);
        return E_PATH;
    }
    char buf[SIZE_BUF_LEN] = {0};
    ssize_t n = read(fd, buf, sizeof(buf) - 1);
    close(fd);
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

int32_t RecycleSizeCache::WriteCachedSize(const std::string &path, int64_t size)
{
    if (size < 0) {
        size = 0;
    }
    bool isNewFile = false;
    struct stat st = {};
    if (stat(path.c_str(), &st) != 0 && errno == ENOENT) {
        isNewFile = true;
    }
    int fd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, FILE_MODE);
    if (fd < 0) {
        LOGE("open cache file for write failed, errno:%{public}d", errno);
        return E_PATH;
    }
    std::string val = std::to_string(size);
    ssize_t w = write(fd, val.data(), val.size());
    close(fd);
    if (w != static_cast<ssize_t>(val.size())) {
        LOGE("write cache size failed, errno:%{public}d", errno);
        return E_PATH;
    }
    if (isNewFile) {
        chmod(path.c_str(), FILE_MODE);
        chown(path.c_str(), OID_FILE_MANAGER, OID_DFS);
    }
    return E_OK;
}

bool RecycleSizeCache::GetXattrByPosition(const std::string &path, int32_t &position)
{
    char buf[XATTR_BUF_LEN] = {0};
    ssize_t n = getxattr(path.c_str(), POSITION_XATTR.c_str(), buf, sizeof(buf) - 1);
    if (n <= 0) {
        return false;
    }
    const char *first = buf;
    const char *last = buf + n;
    while (first < last && std::isspace(static_cast<unsigned char>(*first))) {
        ++first;
    }
    if (first == last) {
        return false;
    }
    auto res = std::from_chars(first, last, position);
    return res.ec == std::errc{};
}

int32_t RecycleSizeCache::CheckActualRecycleBinSize(const std::string &trashDir, int64_t &actual)
{
    actual = 0;
    std::error_code err;
    for (const auto &entry : std::filesystem::recursive_directory_iterator(trashDir, err)) {
        if (std::filesystem::is_regular_file(entry, err)) {
            std::string filePath = entry.path().string();
            int32_t position = POSITION_CLOUD;
            if (!GetXattrByPosition(filePath, position)) {
                continue;
            }

            if (position != POSITION_CLOUD) {
                int64_t fileSize = std::filesystem::file_size(entry.path(), err);
                if (!err) {
                    actual += fileSize;
                } 
            }
        }
    }
    if (err && err.value() != ENOENT) {
        LOGE("scan %{public}s failed, ec:%{public}d", trashDir.c_str(), err.value());
    }
    return E_OK;
}

int32_t RecycleSizeCache::GetRecycleBinSize(int32_t userId, const std::string &bundleName, int64_t &size)
{
    if (bundleName.empty() || userId < 0) {
        LOGE("invalid arguments");
        return E_INVAL_ARG;
    }
    std::string path = GetCacheFilePath(userId, bundleName);
    struct stat st = {};
    if (stat(path.c_str(), &st) == 0) {
        return ReadCachedSize(path, size);
    }
    if (errno != ENOENT) {
        LOGE("stat cache file failed, errno:%{public}d", errno);
        return E_PATH;
    }
    size = 0;
    return WriteCachedSize(path, 0);
}

int32_t RecycleSizeCache::IncreaseRecycleBinSize(int32_t userId, const std::string &bundleName, int64_t delta)
{
    if (bundleName.empty() || userId < 0 || delta < 0) {
        LOGE("invalid arguments");
        return E_INVAL_ARG;
    }
    std::string path = GetCacheFilePath(userId, bundleName);
    int64_t cur = 0;
    int32_t ret = ReadCachedSize(path, cur);
    if (ret != E_OK) {
        return ret;
    }
    cur += delta;
    return WriteCachedSize(path, cur);
}

int32_t RecycleSizeCache::DecreaseRecycleBinSize(int32_t userId, const std::string &bundleName, int64_t delta)
{
    if (bundleName.empty() || userId < 0 || delta < 0) {
        LOGE("invalid arguments");
        return E_INVAL_ARG;
    }
    std::string path = GetCacheFilePath(userId, bundleName);
    int64_t cur = 0;
    int32_t ret = ReadCachedSize(path, cur);
    if (ret != E_OK) {
        return ret;
    }
    cur -= delta;
    if (cur < 0) {
        cur = 0;
    }
    return WriteCachedSize(path, cur);
}

int32_t RecycleSizeCache::VerifyRecycleBinSize(int32_t userId, const std::string &bundleName)
{
    if (bundleName.empty() || userId < 0) {
        LOGE("invalid arguments");
        return E_INVAL_ARG;
    }
    int64_t actual = 0;
    int32_t ret = CheckActualRecycleBinSize(GetTrashDir(userId, bundleName), actual);
    if (ret != E_OK) {
        return ret;
    }
    std::string path = GetCacheFilePath(userId, bundleName);
    return WriteCachedSize(path, actual);
}
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS
