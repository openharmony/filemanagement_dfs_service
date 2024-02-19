/*
 * Copyright (C) 2023 Huawei Device Co., Ltd. All rights reserved.
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
#include "cloud_file_utils_mock.h"
#include <fcntl.h>
#include <sys/xattr.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudDisk {
using namespace std;

const std::string TMP_SUFFIX = ".temp.download";
int g_getCloudIdFlag = 0;

string CloudFileUtils::GetLocalBucketPath(string cloudId, string bundleName, int32_t userId)
{
    if (userId == 0) {
        return "";
    }
    return "/data/test";
}

string CloudFileUtils::GetLocalFilePath(string cloudId, string bundleName, int32_t userId)
{
    if (userId == 0) {
        return "false/";
    }
    return "true";
}

string CloudFileUtils::GetPathWithoutTmp(const string &path)
{
    return TMP_SUFFIX;
}

bool CloudFileUtils::EndsWith(const string &fullString, const string &ending)
{
    return true;
}

string CloudFileUtils::GetCloudId(const string &path)
{
    string cloudId = "";
    switch (g_getCloudIdFlag) {
        case 0:
            cloudId = "cloud_disk_data_handler_test.txt";
            break;
        case 1:
            cloudId = "cloud_disk_data_handler_test1.txt";
            break;
        default:
            break;
    }
    return cloudId;
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

bool CloudFileUtils::LocalWriteOpen(const string &dfsPath)
{
    if (dfsPath.compare("false")) {
        return false;
    }
    if (dfsPath.compare("true")) {
        return true;
    }
    return true;
}

} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS
