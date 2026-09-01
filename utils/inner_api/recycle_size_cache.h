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

#ifndef CLOUD_FILE_DAEMON_RECYCLE_SIZE_CACHE_H
#define CLOUD_FILE_DAEMON_RECYCLE_SIZE_CACHE_H

#include <cstdint>
#include <string>

namespace OHOS {
namespace FileManagement {
namespace CloudDisk {

class RecycleSizeCache final {
public:
    static int32_t GetRecycleBinSize(int32_t userId, const std::string &bundleName, int64_t &size);
    static int32_t IncreaseRecycleBinSize(int32_t userId, const std::string &bundleName, int64_t delta);
    static int32_t DecreaseRecycleBinSize(int32_t userId, const std::string &bundleName, int64_t delta);
    static int32_t VerifyRecycleBinSize(int32_t userId, const std::string &bundleName);

private:
    RecycleSizeCache() = default;
    ~RecycleSizeCache() = default;
    RecycleSizeCache(const RecycleSizeCache &) = delete;
    RecycleSizeCache &operator=(const RecycleSizeCache &) = delete;

    static std::string GetCacheFilePath(int32_t userId, const std::string &bundleName);
    static std::string GetTrashDir(int32_t userId, const std::string &bundleName);
    static int32_t ReadCachedSize(const std::string &path, int64_t &size);
    static int32_t WriteCachedSize(const std::string &path, int64_t size);
    static bool GetXattrByPosition(const std::string &path, int32_t &position);
    static int32_t CheckActualRecycleBinSize(const std::string &trashDir, int64_t &actual);
};
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS

#endif // CLOUD_FILE_DAEMON_RECYCLE_SIZE_CACHE_H
