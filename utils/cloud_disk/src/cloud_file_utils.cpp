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

namespace OHOS {
namespace FileManagement {
namespace CloudDisk {
using namespace std;
namespace {
    static const string LOCAL_PATH_DATA_SERVICE_EL2 = "/data/service/el2/";
    static const string LOCAL_PATH_HMDFS_CLOUD_DATA = "/hmdfs/cloud/data/";
    static const uint32_t CLOUD_ID_MIN_SIZE = 3;
    static const uint32_t CLOUD_ID_BUCKET_MID_TIMES = 2;
    static const uint32_t CLOUD_ID_BUCKET_MAX_SIZE = 32;
}

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
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS
