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
#ifndef OHOS_CLOUD_FILE_DATA_SYNC_MANAGER_H
#define OHOS_CLOUD_FILE_DATA_SYNC_MANAGER_H

#include <memory>

#include "data_sync_const.h"
#include "i_cloud_download_callback.h"
#include "i_cloud_sync_callback.h"

namespace OHOS::FileManagement::CloudFile {
class DataSyncManager {
public:
    using SyncTriggerType = CloudSync::SyncTriggerType;
    DataSyncManager() = default;
    virtual ~DataSyncManager() = default;

    virtual int32_t TriggerStartSync(const std::string &bundleName,
                                     const int32_t userId,
                                     bool forceFlag,
                                     SyncTriggerType triggerType);
    virtual int32_t TriggerStopSync(const std::string &bundleName, const int32_t userId, SyncTriggerType triggerType);
    virtual int32_t TriggerRecoverySync(SyncTriggerType triggerType);
    virtual void RegisterCloudSyncCallback(const std::string &bundleName,
                                           const int32_t userId,
                                           const sptr<CloudSync::ICloudSyncCallback> &callback);
    virtual void UnRegisterCloudSyncCallback(const std::string &bundleName);
    virtual int32_t IsSkipSync(const std::string &bundleName, const int32_t userId, bool forceFlag);
    virtual int32_t StartDownloadFile(const std::string &bundleName, const int32_t userId, const std::string path);
    virtual int32_t StopDownloadFile(const std::string &bundleName, const int32_t userId, const std::string path);
    virtual int32_t RegisterDownloadFileCallback(const std::string &bundleName,
                                                 const int32_t userId,
                                                 const sptr<CloudSync::ICloudDownloadCallback> &downloadCallback);
    virtual int32_t UnregisterDownloadFileCallback(const std::string &bundleName, const int32_t userId);
    virtual int32_t CleanCloudFile(const int32_t userId, const std::string &bundleName, const int action);
    virtual int32_t OptimizeStorage(const std::string &bundleName, const int32_t userId, const int32_t agingDays);
    virtual int32_t DownloadThumb();
    virtual int32_t CleanCache(const std::string &bundleName, const int32_t userId, const std::string &uri);
    virtual int32_t DisableCloud(const int32_t userId);
    virtual int32_t GetUserId(int32_t &userId);
    virtual int32_t SaveSubscription(const std::string &bundleName, const int32_t userId);
};
} // namespace OHOS::FileManagement::CloudSync

#endif // OHOS_CLOUD_FILE_DATA_SYNC_MANAGER_H