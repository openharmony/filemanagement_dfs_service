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

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <string>

namespace OHOS {
namespace Storage {
namespace DistributedFile {
class DfsuFDGuard;
}
}

namespace FileManagement {
struct MetaBase;

namespace CloudDisk {

class RecycleSizeCache {
public:
    static int32_t GetRecycleBinSize(int32_t userId, const std::string &bundleName, int64_t &size);
    static int32_t IncreaseRecycleBinSize(int32_t userId, const std::string &bundleName, const MetaBase &metaBase);
    static int32_t DecreaseRecycleBinSize(int32_t userId, const std::string &bundleName, const MetaBase &metaBase);
    static int32_t ResetRecycleBinSize(int32_t userId, const std::string &bundleName);

private:
    static std::string GetCacheFilePath(int32_t userId, const std::string &bundleName);
    static int32_t ReadCachedSize(const std::string &path, int64_t &size);
    static int32_t WriteCachedSize(const std::string &path, int64_t size);
    static int32_t OpenAndCheckCacheFile(const std::string &path, bool &isNewFile,
        OHOS::Storage::DistributedFile::DfsuFDGuard &fdGuard);
    static size_t GetVersionSlot(const std::string &key);
    static int64_t GetCacheVersion(const std::string &key);
    static void AddCacheVersion(const std::string &key);

    static constexpr size_t kVersionSlotCount = 1024;
    static std::mutex gMutex_;
    static std::atomic<int64_t> gVersionSlots_[kVersionSlotCount];
};
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS

#endif // CLOUD_FILE_DAEMON_RECYCLE_SIZE_CACHE_H
