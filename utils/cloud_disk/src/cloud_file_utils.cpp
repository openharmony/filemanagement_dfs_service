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
#include "cloud_file_utils.h"
#include <ctime>
#include <fcntl.h>
#include <filesystem>
#include <sys/xattr.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "utils_log.h"
namespace OHOS {
namespace FileManagement {
namespace CloudDisk {
using namespace std;
namespace {
    static const string LOCAL_PATH_DATA_SERVICE_EL2 = "/data/service/el2/";
    static const string LOCAL_PATH_HMDFS_CLOUD_DATA = "/hmdfs/cloud/data/";
    static const string LOCAL_PATH_HMDFS_CLOUD_CACHE = "/hmdfs/cache/cloud_cache";
    static const string CLOUDDISK_CACHE_DIR = "/disk_drivekit_cache/";
    static const string CLOUD_FILE_CLOUD_ID_XATTR = "user.cloud.cloudid";
    static const uint32_t CLOUD_ID_MIN_SIZE = 3;
    static const uint32_t CLOUD_ID_BUCKET_MID_TIMES = 2;
    static const uint32_t CLOUD_ID_BUCKET_MAX_SIZE = 32;
    static const int64_t SECOND_TO_MILLISECOND = 1e3;
    static const int64_t MILLISECOND_TO_NANOSECOND = 1e6;
    static const uint64_t DELTA_DISK = 0x9E3779B9;
    static const uint64_t HMDFS_HASH_COL_BIT_DISK = (0x1ULL) << 63;
}

constexpr unsigned HMDFS_IOC = 0xf2;
constexpr unsigned WRITEOPEN_CMD = 0x02;
constexpr unsigned CLOUD_ENABLE_CMD = 0x0b;
#define HMDFS_IOC_GET_WRITEOPEN_CNT _IOR(HMDFS_IOC, WRITEOPEN_CMD, uint32_t)
#define HMDFS_IOC_SET_CLOUD_GENERATION _IOR(HMDFS_IOC, CLOUD_ENABLE_CMD, uint32_t)
const string CloudFileUtils::TMP_SUFFIX = ".temp.download";

bool CloudFileUtils::IsDotDotdot(const std::string &name)
{
    return name == "." || name == "..";
}

void CloudFileUtils::Str2HashBuf(const char *msg, size_t len, uint32_t *buf, int num)
{
    const int32_t shift8 = 8;
    const int32_t shift16 = 16;
    const int32_t three = 3;
    const int32_t mod = 4;
    uint32_t pad = static_cast<uint32_t>(len) | (static_cast<uint32_t>(len) << shift8);
    pad |= pad << shift16;

    uint32_t val = pad;
    len = std::min(len, static_cast<size_t>(num * sizeof(int)));
    for (uint32_t i = 0; i < len; i++) {
        if ((i % sizeof(int)) == 0) {
            val = pad;
        }
        uint8_t c = static_cast<uint8_t>(tolower(msg[i]));
        val = c + (val << shift8);
        if ((i % mod) == three) {
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

void CloudFileUtils::TeaTransform(uint32_t buf[4], uint32_t const in[]) __attribute__((no_sanitize(
    "unsigned-integer-overflow")))
{
    int n = 16;
    uint32_t a = in[0];
    uint32_t b = in[1];
    uint32_t c = in[2];
    uint32_t d = in[3];
    uint32_t b0 = buf[0];
    uint32_t b1 = buf[1];
    uint32_t sum = 0;
    const int32_t LEFT_SHIFT = 4;
    const int32_t RIGHT_SHIFT = 5;
    do {
        sum += DELTA_DISK;
        b0 += ((b1 << LEFT_SHIFT) + a) ^ (b1 + sum) ^ ((b1 >> RIGHT_SHIFT) + b);
        b1 += ((b0 << LEFT_SHIFT) + c) ^ (b0 + sum) ^ ((b0 >> RIGHT_SHIFT) + d);
    } while (--n);

    buf[0] += b0;
    buf[1] += b1;
}

uint32_t CloudFileUtils::DentryHash(const std::string &inputStr)
{
    if (IsDotDotdot(inputStr)) {
        return 0;
    }
    constexpr int inLen = 8;
    constexpr int bufLen = 4;
    uint32_t in[inLen];
    uint32_t buf[bufLen] = {0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476};
    auto len = inputStr.length();
    constexpr decltype(len) hashWidth = 16;
    const char *p = inputStr.c_str();

    bool loopFlag = true;
    while (loopFlag) {
        Str2HashBuf(p, len, in, bufLen);
        TeaTransform(buf, in);

        if (len <= hashWidth) {
            break;
        }
        p += hashWidth;
        len -= hashWidth;
    };
    uint32_t hash = buf[0];
    uint32_t hmdfsHash = hash & ~HMDFS_HASH_COL_BIT_DISK;
    return hmdfsHash;
}

uint32_t CloudFileUtils::GetBucketId(string cloudId)
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

int64_t CloudFileUtils::Timespec2Milliseconds(const struct timespec &time)
{
    return time.tv_sec * SECOND_TO_MILLISECOND + time.tv_nsec / MILLISECOND_TO_NANOSECOND;
}

string CloudFileUtils::GetLocalBucketPath(string cloudId, string bundleName, int32_t userId)
{
    string baseDir = LOCAL_PATH_DATA_SERVICE_EL2 + to_string(userId) +
                     LOCAL_PATH_HMDFS_CLOUD_DATA + bundleName + "/";
    uint32_t bucketId = GetBucketId(cloudId);
    string bucketPath = baseDir + to_string(bucketId);
    return bucketPath;
}

string CloudFileUtils::GetLocalDKCachePath(string cloudId, string bundleName, int32_t userId)
{
    string baseDir = LOCAL_PATH_DATA_SERVICE_EL2 + to_string(userId) + LOCAL_PATH_HMDFS_CLOUD_CACHE +
                     CLOUDDISK_CACHE_DIR + bundleName + "/";
    uint32_t bucketId = GetBucketId(cloudId);
    string cachePath = baseDir + to_string(bucketId) + "/" + cloudId;
    return cachePath;
}

string CloudFileUtils::GetLocalFilePath(string cloudId, string bundleName, int32_t userId)
{
    return GetLocalBucketPath(cloudId, bundleName, userId) + "/" + cloudId;
}

string CloudFileUtils::GetPathWithoutTmp(const string &path)
{
    string ret = path;
    if (EndsWith(path, TMP_SUFFIX)) {
        ret = path.substr(0, path.length() - TMP_SUFFIX.length());
    }
    return ret;
}

bool CloudFileUtils::EndsWith(const string &fullString, const string &ending)
{
    if (fullString.length() >= ending.length()) {
        return (!fullString.compare(fullString.length() - ending.length(),
                                    ending.length(),
                                    ending));
    }
    return false;
}

string CloudFileUtils::GetCloudId(const string &path)
{
    auto idSize = getxattr(path.c_str(), CLOUD_FILE_CLOUD_ID_XATTR.c_str(), nullptr, 0);
    if (idSize <= 0) {
        return "";
    }
    char cloudId[idSize + 1];
    idSize = getxattr(path.c_str(), CLOUD_FILE_CLOUD_ID_XATTR.c_str(), cloudId, idSize);
    if (idSize <= 0) {
        return "";
    }
    return string(cloudId);
}

bool CloudFileUtils::CheckIsCloud(const string &key)
{
    return key == CLOUD_CLOUD_ID_XATTR;
}

bool CloudFileUtils::CheckIsCloudLocation(const string &key)
{
    return key == CLOUD_FILE_LOCATION;
}

bool CloudFileUtils::CheckIsHmdfsPermission(const string &key)
{
    return key == HMDFS_PERMISSION_XATTR;
}

bool CloudFileUtils::CheckIsCloudRecycle(const string &key)
{
    return key == CLOUD_CLOUD_RECYCLE_XATTR;
}

bool CloudFileUtils::CheckIsFavorite(const string &key)
{
    return key == IS_FAVORITE_XATTR;
}

bool CloudFileUtils::CheckIsTimeRecycled(const string &key)
{
    return key == CLOUD_TIME_RECYCLED;
}

bool CloudFileUtils::CheckIsHasLCD(const string &key)
{
    return key == CLOUD_HAS_LCD;
}

bool CloudFileUtils::CheckIsHasTHM(const string &key)
{
    return key == CLOUD_HAS_THM;
}

bool CloudFileUtils::CheckFileStatus(const string &key)
{
    return key == IS_FILE_STATUS_XATTR;
}

bool CloudFileUtils::CheckIsRecyclePath(const string &key)
{
    return key == CLOUD_RECYCLE_PATH;
}

bool CloudFileUtils::LocalWriteOpen(const string &dfsPath)
{
    unique_ptr<char[]> absPath = make_unique<char[]>(PATH_MAX + 1);
    if (realpath(dfsPath.c_str(), absPath.get()) == nullptr) {
        return false;
    }
    string realPath = absPath.get();
    char resolvedPath[PATH_MAX] = {'\0'};
    char *realPaths = realpath(realPath.c_str(), resolvedPath);
    if (realPaths == NULL) {
        LOGE("realpath failed");
        return false;
    }
    std::FILE *file = fopen(realPaths, "r");
    if (file == nullptr) {
        LOGE("fopen failed, errno:%{public}d", errno);
        return false;
    }
    int fd = fileno(file);
    if (fd < 0) {
        LOGE("get fd failed, errno:%{public}d", errno);
        return false;
    }
    uint32_t writeOpenCnt = 0;
    int ret = ioctl(fd, HMDFS_IOC_GET_WRITEOPEN_CNT, &writeOpenCnt);
    if (ret < 0) {
        LOGE("ioctl failed, errno:%{public}d", errno);
        if (fclose(file)) {
            LOGE("fclose failed, errno:%{public}d", errno);
        }
        return false;
    }

    if (fclose(file)) {
        LOGE("fclose failed, errno:%{public}d", errno);
        return false;
    }
    return writeOpenCnt != 0;
}

bool CloudFileUtils::ClearCache(const string &dfsPath)
{
    auto resolvedPath = realpath(dfsPath.c_str(), NULL);
    if (resolvedPath == NULL) {
        LOGE("realpath failed");
        return false;
    }
    std::FILE *file = fopen(resolvedPath, "r");
    free(resolvedPath);
    if (file == nullptr) {
        LOGE("fopen failed, errno:%{public}d", errno);
        return false;
    }
    int fd = fileno(file);
    if (fd < 0) {
        LOGE("get fd failed, errno:%{public}d", errno);
        return false;
    }
    int ret = ioctl(fd, HMDFS_IOC_SET_CLOUD_GENERATION);
    if (ret < 0) {
        LOGE("ioctl failed, errno:%{public}d", errno);
        if (fclose(file)) {
            LOGE("fclose failed, errno:%{public}d", errno);
        }
        return false;
    }
    if (fclose(file)) {
        LOGE("fclose failed, errno:%{public}d", errno);
        return false;
    }
    return true;
}

string CloudFileUtils::GetRealPath(const string &path)
{
    filesystem::path tempPath(path);
    filesystem::path realPath{};
    for (const auto& component : tempPath) {
        if (component == ".") {
            continue;
        } else if (component == "..") {
            realPath = realPath.parent_path();
        } else {
            realPath /= component;
        }
    }
    return realPath.string();
}
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS
