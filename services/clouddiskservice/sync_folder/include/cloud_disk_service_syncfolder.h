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

#ifndef CLOUD_DISK_SERVICE_SYNCFOLDER_H
#define CLOUD_DISK_SERVICE_SYNCFOLDER_H

#include <string>
#include <vector>

#include "disk_types.h"

namespace OHOS::FileManagement::CloudDiskService {

class CloudDiskServiceSyncFolder {
public:
    static int32_t RegisterSyncFolder(const int32_t userId, const uint32_t syncFolderIndex, const std::string &path);
    static int32_t UnRegisterSyncFolder(const int32_t userId, const uint32_t syncFolderIndex);
    static void RegisterSyncFolderChanges(const int32_t userId, const uint32_t syncFolderIndex);
    static void UnRegisterSyncFolderChanges(const int32_t userId, const uint32_t syncFolderIndex);
    static int32_t GetSyncFolderChanges(const int32_t userId, const uint32_t syncFolderIndex, const uint64_t start,
                                        const uint64_t count, struct ChangesResult &changesResult);
    static int32_t SetSyncFolderChanges(const struct EventInfo &eventInfos);
    static void CloudDiskServiceClearAll();
};

} // namespace OHOS::FileManagement::CloudDiskService

#endif // CLOUD_DISK_SERVICE_SYNCFOLDER_H