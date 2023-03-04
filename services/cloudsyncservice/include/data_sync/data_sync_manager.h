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

#ifndef OHOS_FILEMGMT_DATA_SYNC_MANAGER_H
#define OHOS_FILEMGMT_DATA_SYNC_MANAGER_H

#include <map>
#include <memory>
#include <mutex>
#include <atomic>

#include "data_sync/data_syncer.h"

namespace OHOS::FileManagement::CloudSync {
class DataSyncManager {
public:
    DataSyncManager() = default;
    ~DataSyncManager() = default;

    int32_t Init(const int32_t userId);
    std::shared_ptr<DataSyncer> GetDataSyncer(const std::string appPackageName);
    int32_t IsBlockCloudSync(const std::string appPackageName, const int32_t userId) const;

private:
    std::map<std::string, std::shared_ptr<DataSyncer>> dataSyncers_;
    std::mutex dataSyncMutex_;
};
} // namespace OHOS::FileManagement::CloudSync

#endif // OHOS_FILEMGMT_DATA_SYNC_MANAGER_H