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
#ifndef CLOUD_FILE_DAEMON_CLOUD_FILE_UTILS_H
#define CLOUD_FILE_DAEMON_CLOUD_FILE_UTILS_H

#include <string>

namespace OHOS {
namespace FileManagement {
namespace CloudDisk {
struct CloudDiskFileInfo {
    std::string fileName;
    std::string cloudId;
    std::string parentCloudId;
    uint32_t location{1};
    int64_t localId{0};
    unsigned long long size{0};
    unsigned long long atime{0};
    unsigned long long ctime{0};
    unsigned long long mtime{0};
    bool IsDirectory{false};
    off_t nextOff{0};
};

namespace {
    static const std::string CLOUD_CLOUD_ID_XATTR = "user.cloud.cloudid";
    static const std::string CLOUD_FILE_LOCATION = "user.cloud.location";
    static const std::string HMDFS_PERMISSION_XATTR = "user.hmdfs.perm";
    static const std::string CLOUD_CLOUD_RECYCLE_XATTR = "user.cloud.recycle";
    static const std::string IS_FAVORITE_XATTR = "user.cloud.favorite";
    static const std::string IS_FILE_STATUS_XATTR = "user.cloud.filestatus";
}

class CloudFileUtils final {
public:
    static bool CheckIsCloud(const std::string &key);
    static bool CheckIsCloudLocation(const std::string &key);
    static bool CheckIsHmdfsPermission(const std::string &key);
    static bool CheckIsCloudRecycle(const std::string &key);
    static bool CheckIsFavorite(const std::string &key);
    static bool CheckFileStatus(const std::string &key);
    static std::string GetLocalBucketPath(std::string cloudId, std::string bundleName,
                                          int32_t userId);
    static std::string GetLocalFilePath(std::string cloudId, std::string bundleName,
                                        int32_t userId);
    static std::string GetPathWithoutTmp(const std::string &path);
    static std::string GetCloudId(const std::string &path);
    static bool LocalWriteOpen(const std::string &dfsPath);
    static const std::string TMP_SUFFIX;
private:
    static bool EndsWith(const std::string &fullString, const std::string &ending);
};
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS
#endif // CLOUD_FILE_DAEMON_CLOUD_FILE_UTILS_H
