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

#ifndef OHOS_FILEMGMT_CLOUD_DISK_SERVICE_MANAGER_H
#define OHOS_FILEMGMT_CLOUD_DISK_SERVICE_MANAGER_H

#include <memory>

#include "cloud_disk_service_callback.h"

namespace OHOS::FileManagement::CloudDiskService {
class CloudDiskServiceManager {
public:
    static CloudDiskServiceManager &GetInstance();

    virtual int32_t RegisterSyncFolderChanges(const std::string &syncFolder,
                                              const std::shared_ptr<CloudDiskServiceCallback> callback) = 0;
    virtual int32_t UnregisterSyncFolderChanges(const std::string &syncFolder) = 0;
    virtual int32_t GetSyncFolderChanges(const std::string &syncFolder,
                                         uint64_t count,
                                         uint64_t startUsn,
                                         ChangesResult &changesResult) = 0;
    virtual int32_t SetFileSyncStates(const std::string &syncFolder,
                                      const std::vector<FileSyncState> &fileSyncStates,
                                      std::vector<FailedList> &failedList) = 0;
    virtual int32_t GetFileSyncStates(const std::string &syncFolder,
                                      const std::vector<std::string> &pathArray,
                                      std::vector<ResultList> &resultList) = 0;
    virtual int32_t RegisterSyncFolder(int32_t userId, const std::string &bundleName, const std::string &path) = 0;
    virtual int32_t UnregisterSyncFolder(int32_t userId, const std::string &bundleName, const std::string &path) = 0;

    virtual int32_t UnregisterForSa(const std::string &path) = 0;
};
} // namespace OHOS::FileManagement::CloudDiskService

#endif // OHOS_FILEMGMT_CLOUD_DISK_SERVICE_MANAGER_H
