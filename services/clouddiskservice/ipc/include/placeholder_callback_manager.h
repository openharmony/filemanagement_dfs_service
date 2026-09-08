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

#ifndef OHOS_FILEMGMT_PLACEHOLDER_CALLBACK_MANAGER_H
#define OHOS_FILEMGMT_PLACEHOLDER_CALLBACK_MANAGER_H

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <tuple>
#include <vector>

#include "i_cloud_disk_service_callback_table.h"
#include "nocopyable.h"
#include "svc_death_recipient.h"

namespace OHOS::FileManagement::CloudDiskService {
class PlaceholderCallbackManager final : public NoCopyable {
public:
    static PlaceholderCallbackManager &GetInstance();

    int32_t RegisterCallbackTable(const std::string &bundleName,
                                  uint32_t syncFolderIndex,
                                  const sptr<ICloudDiskServiceCallbackTable> &callback);
    int32_t UnregisterCallbackTable(const std::string &bundleName, uint32_t syncFolderIndex);
    sptr<ICloudDiskServiceCallbackTable> GetCallback(const std::string &bundleName, uint32_t syncFolderIndex);
    bool IsCallbackRegistered(const std::string &bundleName, uint32_t syncFolderIndex);
    int32_t RunIfRegistered(const std::string &bundleName,
                            uint32_t syncFolderIndex,
                            const std::function<int32_t()> &operation);
    int32_t DispatchFetchData(const std::string &bundleName,
                              uint32_t syncFolderIndex,
                              CloudDiskCallbackReqHead &reqHead,
                              CloudDiskPathInfo &filePath,
                              CloudDiskHydratePriority priority = CLOUD_DISK_HYDRATE_PRIORITY_NORMAL);
    int32_t DispatchCancelFetchData(const std::string &bundleName,
                                    uint32_t syncFolderIndex,
                                    CloudDiskCallbackReqHead &reqHead,
                                    CloudDiskPathInfo &filePath);
    int32_t DispatchFetchRangeData(const std::string &bundleName,
                                   uint32_t syncFolderIndex,
                                   CloudDiskCallbackReqHead &reqHead,
                                   CloudDiskRangeInfo &rangeInfo);
    int32_t DispatchDehydrate(const std::string &bundleName,
                              uint32_t syncFolderIndex,
                              CloudDiskCallbackReqHead &reqHead,
                              CloudDiskPathInfo &filePath);
    void ClearBySyncFolder(const std::string &bundleName, uint32_t syncFolderIndex);
    void ClearAll();

private:
    struct CallbackKey {
        std::string bundleName;
        uint32_t syncFolderIndex;

        bool operator<(const CallbackKey &other) const
        {
            return std::tie(bundleName, syncFolderIndex) < std::tie(other.bundleName, other.syncFolderIndex);
        }
    };

    struct CallbackEntry {
        sptr<ICloudDiskServiceCallbackTable> callback;
        std::mutex dispatchMutex;
    };

    int32_t DispatchCallback(const std::string &bundleName,
                             uint32_t syncFolderIndex,
                             const CloudDiskCallbackReqHead &reqHead,
                             CloudDiskCallbackContext &reqContext);
    void AddDeathRecipientLocked(const sptr<ICloudDiskServiceCallbackTable> &callback);
    void RemoveRemoteKeyLocked(const sptr<ICloudDiskServiceCallbackTable> &callback, const CallbackKey &key);
    void OnRemoteDied(const void *remoteKey);

    std::map<CallbackKey, std::shared_ptr<CallbackEntry>> callbackMap_;
    std::map<const void *, std::vector<CallbackKey>> remoteCallbackMap_;
    std::map<const void *, sptr<SvcDeathRecipient>> deathRecipientMap_;
    std::mutex callbackMutex_;
};
} // namespace OHOS::FileManagement::CloudDiskService

#endif // OHOS_FILEMGMT_PLACEHOLDER_CALLBACK_MANAGER_H
