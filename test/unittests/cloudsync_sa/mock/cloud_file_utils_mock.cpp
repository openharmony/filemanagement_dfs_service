/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
    static const string CLOUD_FILE_CLOUD_ID_XATTR = "user.cloud.cloudid";
    static const int64_t SECOND_TO_MILLISECOND = 1e3;
    static const int64_t MILLISECOND_TO_NANOSECOND = 1e6;
}

#define HMDFS_IOC_GET_WRITEOPEN_CNT _IOR(HMDFS_IOC, WRITEOPEN_CMD, uint32_t)
const string CloudFileUtils::TMP_SUFFIX = ".temp.download";

bool CloudFileUtils::IsDotDotdot(const std::string &name)
{
    return name == "." || name == "..";
}

void CloudFileUtils::Str2HashBuf(const char *msg, size_t len, uint32_t *buf, int num)
{
}

void CloudFileUtils::TeaTransform(uint32_t buf[4], uint32_t const in[])
{
}

uint32_t CloudFileUtils::DentryHash(const std::string &inputStr)
{
    return 0;
}

uint32_t CloudFileUtils::GetBucketId(string cloudId)
{
    return 0;
}

int64_t CloudFileUtils::Timespec2Milliseconds(const struct timespec &time)
{
    return time.tv_sec * SECOND_TO_MILLISECOND + time.tv_nsec / MILLISECOND_TO_NANOSECOND;
}

string CloudFileUtils::GetLocalBucketPath(string cloudId, string bundleName, int32_t userId)
{
    return "";
}

string CloudFileUtils::GetLocalFilePath(string cloudId, string bundleName, int32_t userId)
{
    if (userId == 0) {
        return "mock";
    }
    return "/data";
}

string CloudFileUtils::GetPathWithoutTmp(const string &path)
{
    return "";
}

bool CloudFileUtils::EndsWith(const string &fullString, const string &ending)
{
    return false;
}

string CloudFileUtils::GetCloudId(const string &path)
{
    return "";
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

bool CloudFileUtils::CheckFileStatus(const string &key)
{
    return key == IS_FILE_STATUS_XATTR;
}

bool CloudFileUtils::LocalWriteOpen(const string &dfsPath)
{
    return false;
}
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS