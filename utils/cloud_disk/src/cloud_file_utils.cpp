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
#include <sys/xattr.h>
namespace OHOS {
namespace FileManagement {
namespace CloudDisk {
using namespace std;
namespace {
    static const string LOCAL_PATH_DATA_SERVICE_EL2 = "/data/service/el2/";
    static const string LOCAL_PATH_HMDFS_CLOUD_DATA = "/hmdfs/cloud/data/";
    static const string CLOUD_FILE_CLOUD_ID_XATTR = "user.cloud.cloudid";
    static const uint32_t CLOUD_ID_MIN_SIZE = 3;
    static const uint32_t CLOUD_ID_BUCKET_MID_TIMES = 2;
    static const uint32_t CLOUD_ID_BUCKET_MAX_SIZE = 32;
}
const string CloudFileUtils::TMP_SUFFIX = ".temp.download";

static uint32_t GetBucketId(string cloudId)
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

string CloudFileUtils::GetLocalBucketPath(string cloudId, string bundleName, int32_t userId)
{
    string baseDir = LOCAL_PATH_DATA_SERVICE_EL2 + to_string(userId) +
                     LOCAL_PATH_HMDFS_CLOUD_DATA + bundleName + "/";
    uint32_t bucketId = GetBucketId(cloudId);
    string bucketPath = baseDir + to_string(bucketId);
    return bucketPath;
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
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS
