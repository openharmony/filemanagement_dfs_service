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

#ifndef CLOUD_DISK_SERVICE_H
#define CLOUD_DISK_SERVICE_H

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "account_status_listener.h"
#include "cloud_disk_service_error.h"
#include "cloud_disk_service_stub.h"
#include "ffrt.h"
#include "i_cloud_disk_service_callback.h"
#include "icloud_disk_service.h"
#include "iremote_stub.h"
#include "nocopyable.h"
#include "refbase.h"
#include "system_ability.h"

namespace OHOS {
namespace FileManagement {
namespace CloudDiskService {
class CloudDiskService final : public SystemAbility, public CloudDiskServiceStub, protected NoCopyable {
    DECLARE_SYSTEM_ABILITY(CloudDiskService);

public:
    explicit CloudDiskService(int32_t saID, bool runOnCreate = true);
    virtual ~CloudDiskService() = default;

    void OnStart(const SystemAbilityOnDemandReason &startReason) override;
    void OnStop() override;
    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
    ErrCode RegisterSyncFolderChangesInner(const std::string &syncFolder,
                                           const sptr<IRemoteObject> &remoteObject) override;
    ErrCode UnregisterSyncFolderChangesInner(const std::string &syncFolder) override;
    ErrCode RegisterCallbackTableInner(const std::string &syncFolder, const sptr<IRemoteObject> &remoteObject) override;
    ErrCode UnregisterCallbackTableInner(const std::string &syncFolder) override;
    ErrCode GetSyncFolderChangesInner(const std::string &syncFolder,
                                      uint64_t count,
                                      uint64_t startUsn,
                                      ChangesResult &changesResult) override;
    ErrCode SetFileSyncStatesInner(const std::string &syncFolder,
                                   const std::vector<FileSyncState> &fileSyncStates,
                                   std::vector<FailedList> &failedList) override;
    ErrCode GetFileSyncStatesInner(const std::string &syncFolder,
                                   const std::vector<std::string> &pathArray,
                                   std::vector<ResultList> &resultList) override;
    ErrCode CreatePlaceholderFileInner(const std::string &syncFolder,
                                       const std::string &relativePath,
                                       const PlaceholderInfo &info,
                                       const PlaceholderCustomInfo &customInfo = PlaceholderCustomInfo()) override;
    ErrCode IsPlaceholderFileInner(const std::string &syncFolder, const std::string &path,
                                   bool &isPlaceholder) override;
    ErrCode RegisterSyncFolderInner(int32_t userId, const std::string &bundleName, const std::string &path) override;
    ErrCode UnregisterSyncFolderInner(int32_t userId, const std::string &bundleName, const std::string &path) override;

    int32_t UnregisterForSaInner(const std::string &path) override;
    ErrCode ConvertPlaceholderToFileInner(const std::string &syncFolder, const std::string &relativePath) override;
    ErrCode MarkFileAsPlaceholderInner(const std::string &syncFolder, const std::string &relativePath) override;
    ErrCode UnmarkPlaceholderFileInner(const std::string &syncFolder, const std::string &relativePath) override;
    ErrCode StartHydrationInner(const std::string &syncFolder,
                                const std::string &relativePath,
                                int32_t priority) override;
    ErrCode CancelHydrationInner(const std::string &syncFolder, const std::string &relativePath) override;
    ErrCode ExecuteInner(const CallbackExecuteRequest &request) override;
    ErrCode StartHydrationByPathInner(const std::string &path, int32_t callbackType, int32_t priority) override;
    ErrCode DehydrateFileByPathInner(const std::string &path) override;
    ErrCode RegisterProgressCallbackInner(const sptr<IRemoteObject> &callback) override;
    ErrCode UnregisterProgressCallbackInner() override;
    ErrCode DehydrateInner(const std::string &syncFolder, const std::string &relativePath) override;
    ErrCode UpdatePlaceholderInner(const std::string &syncFolder, const std::string &relativePath,
        const PlaceholderInfo &metaData,
        const PlaceholderCustomInfo &customInfo = PlaceholderCustomInfo()) override;
    ErrCode GetPlaceholderCustomInfoInner(const std::string &syncFolder, const std::string &relativePath,
        PlaceholderCustomInfo &customInfo) override;
    ErrCode GetPlaceholderStateInner(const std::string &syncFolder,
                                     const std::string &relativePath,
                                     int32_t &state) override;
    void UnloadSa();

private:
    CloudDiskService();
    struct InitState {
        int32_t result = E_TRY_AGAIN;
        std::atomic<bool> stopped{false};
    };
    std::mutex startMutex_;
    ffrt::task_handle initTask_;
    std::shared_ptr<InitState> initState_;
    int32_t currentUserId_ = -1;
    static sptr<CloudDiskService> instance_;
    bool PublishSA();
    int32_t Init(const SystemAbilityOnDemandReason &startReason);
    int32_t WaitForInit();
    int32_t ResolveOwnedSyncFolder(const std::string &syncFolder,
                                   std::string &bundleName,
                                   uint32_t &syncFolderIndex,
                                   std::string &physicalPath);
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;
    std::shared_ptr<AccountStatusListener> accountStatusListener_ = nullptr;
};
} // namespace CloudDiskService
} // namespace FileManagement
} // namespace OHOS
#endif // CLOUD_DISK_SERVICE_H
