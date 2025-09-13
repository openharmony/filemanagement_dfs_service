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

#ifndef OHOS_FILEMGMT_CLOUD_DISK_SERVICE_CALLBACK_MANAGER_H
#define OHOS_FILEMGMT_CLOUD_DISK_SERVICE_CALLBACK_MANAGER_H

#include "nocopyable.h"

#include "i_cloud_disk_service_callback.h"
#include "svc_death_recipient.h"

namespace OHOS::FileManagement::CloudDiskService {
class CloudDiskServiceCallbackManager final : public NoCopyable {
public:
    using TaskId = uint64_t;
    static CloudDiskServiceCallbackManager &GetInstance();
    bool RigisterSyncFolderMap(std::string &bundleName,
                            uint32_t syncFolderIndex,
                            const sptr<ICloudDiskServiceCallback> &callback);
    void UnregisterSyncFolderMap(const std::string &bundleName, uint32_t syncFolderIndex);
    bool UnregisterSyncFolderForChangesMap(std::string &bundleName, uint32_t syncFolderIndex);
    void AddCallback(const std::string &bundleName, const sptr<ICloudDiskServiceCallback> &callback);
    void OnChangeData(const uint32_t syncFolderIndex, const std::vector<ChangeData> &changeData);

    struct CallbackValue {
        sptr<ICloudDiskServiceCallback> callback;
        std::vector<uint32_t> syncFolderIndexs;
    };

private:
    std::map<std::string, CallbackValue> callbackAppMap_;
    std::map<uint32_t, sptr<ICloudDiskServiceCallback>> callbackIndexMap_;
    std::mutex callbackMutex_;
};
} // namespace OHOS::FileManagement::CloudDiskService

#endif // OHOS_FILEMGMT_CLOUD_DISK_SERVICE_CALLBACK_MANAGER_H