/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_CLOUD_DISK_SERVICE_NOTIFY_UTILS_H
#define OHOS_CLOUD_DISK_SERVICE_NOTIFY_UTILS_H

#include <list>
#include <unordered_map>

#include "clouddisk_db_const.h"
#include "clouddisk_notify_const.h"

namespace OHOS::FileManagement::CloudDisk {
using namespace std;
using namespace DriveKit;

class CloudDiskNotifyUtils final {
public:
    using FindCloudDiskInodeFunc = std::function<std::shared_ptr<CloudDiskInode>(CloudDiskFuseData*, int64_t)>;
    /* cloud disk */
    static int32_t GetNotifyData(CloudDiskFuseData* data, FindCloudDiskInodeFunc func, const fuse_ino_t &ino,
        NotifyData &notifyData);
    static int32_t GetNotifyData(CloudDiskFuseData* data, FindCloudDiskInodeFunc func, const fuse_ino_t &parent,
        const string &name,
        NotifyData &notifyData);
    static int32_t GetNotifyData(CloudDiskFuseData* data, FindCloudDiskInodeFunc func,
        shared_ptr<CloudDiskInode> inoPtr, NotifyData &notifyData);
    static int32_t GetNotifyData(CloudDiskFuseData* data, FindCloudDiskInodeFunc func,
        shared_ptr<CloudDiskInode> inoPtr, const string &name, NotifyData &notifyData);
    /* cloud disk syncer */
    static int32_t GetCacheNode(const string &cloudId, CacheNode &cacheNode);
    static void PutCacheNode(const string &cloudId, const CacheNode &cacheNode);
    static int32_t
        GetUriFromCache(const string &bundleName, const string &rootId, const CacheNode &cacheNode, string &uri);
private:
    static const int32_t maxCacheCnt_ = 50;
    static list<pair<string, CacheNode>> cacheList_;
    static unordered_map<string, list<pair<string, CacheNode>>::iterator> cacheMap_;
    static mutex cacheMutex_;
    static string rootId_;
};
} // namespace OHOS::FileManagement::CloudDisk

#endif // OHOS_CLOUD_DISK_SERVICE_NOTIFY_UTILS_H