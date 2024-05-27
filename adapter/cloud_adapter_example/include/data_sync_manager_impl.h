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

#ifndef OHOS_CLOUD_FILE_DATA_SYNC_MANAGER_IMPL_H
#define OHOS_CLOUD_FILE_DATA_SYNC_MANAGER_IMPL_H

#include "data_sync_manager.h"

namespace OHOS::FileManagement::CloudFile {
class DataSyncManagerImpl : public DataSyncManager {
public:
    virtual int32_t TriggerStartSync(const std::string &bundleName,
                                     const int32_t userId,
                                     bool forceFlag,
                                     SyncTriggerType triggerType);
};
} // namespace OHOS::FileManagement::CloudFile

#endif // OHOS_CLOUD_FILE_DATA_SYNC_MANAGER_IMPL_H