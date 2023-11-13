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
    unsigned long long size{0};
    unsigned long long atime{0};
    unsigned long long ctime{0};
    unsigned long long mtime{0};
    bool IsDirectory{false};
};

class CloudFileUtils final {
public:
    static std::string GetLocalBucketPath(std::string cloudId, std::string bundleName,
                                          int32_t userId);
    static std::string GetLocalFilePath(std::string cloudId, std::string bundleName,
                                        int32_t userId);
};
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS
#endif // CLOUD_FILE_DAEMON_CLOUD_FILE_UTILS_H
