/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef CLOUD_DISK_SYNC_FOLDER_H
#define CLOUD_DISK_SYNC_FOLDER_H

#include <memory>
#include <string>

namespace OHOS {
namespace FileManagement {
namespace CloudDiskService {

struct SyncFolderValue {
    std::string bundleName;
    std::string path;
};

/*
 * for tdd only, do not depend other modules
 */
class CloudDiskSyncFolder {
public:
    static CloudDiskSyncFolder &GetInstance()
    {
        static CloudDiskSyncFolder instance;
        return instance;
    }

    std::unordered_map<uint32_t, SyncFolderValue> GetSyncFolderMap()
    {
        return syncFolderMap;
    }

private:
    std::unordered_map<uint32_t, SyncFolderValue> syncFolderMap;
};
} // namespace CloudDiskService
} // namespace FileManagement
} // namespace OHOS
#endif // CLOUD_DISK_SYNC_FOLDER_H
