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
#include <mutex>
#include <string>

namespace OHOS {
namespace FileManagement {
namespace CloudDisk {

/**
 * 回收站大小缓存。
 *
 * 设计要点：
 * - 纯文件操作，无内存镜像，磁盘是唯一事实来源。
 * - Increase/Decrease：完整"读-改-写 + 落盘"整体丢进 ffrt::thread(...).detach()，
 *   调用方不阻塞；用一把锁串行整段读-改-写，保证时序严格正确、不丢更新。
 * - Verify：校验重扫按需同步触发，先扫描（不持锁）再于最后一步加锁写回，
 *   避免慢扫描长时间阻断其它写操作。
 * - Get：同步读缓存文件。
 * - userId/bundleName 固定内部过滤，对外接口签名不变。
 */
class RecycleSizeCache final {
public:
    static int32_t GetRecycleBinSize(int32_t userId, const std::string &bundleName, int64_t &size);
    static int32_t IncreaseRecycleBinSize(int32_t userId, const std::string &bundleName, int64_t delta);
    static int32_t DecreaseRecycleBinSize(int32_t userId, const std::string &bundleName, int64_t delta);
    static int32_t VerifyRecycleBinSize(int32_t userId, const std::string &bundleName);

private:
    static std::string GetCacheFilePath();
    static std::string GetTrashDir();
    static int32_t ReadCachedSize(const std::string &path, int64_t &size);
    static int32_t WriteCachedSize(const std::string &path, int64_t size);
    static bool GetXattrByPosition(const std::string &path, int32_t &position);
    static int32_t CheckCachedSize(const std::string &trashDir, int64_t &actual);

    // 串行化"读-改-写"整体，保证时序正确与线程安全。
    static std::mutex gMutex_;
};
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS

#endif // CLOUD_FILE_DAEMON_RECYCLE_SIZE_CACHE_H
