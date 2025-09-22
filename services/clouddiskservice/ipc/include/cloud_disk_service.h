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

#include <memory>
#include <mutex>
#include <string>

#include "account_status_listener.h"
#include "cloud_disk_service_stub.h"
#include "i_cloud_disk_service_callback.h"
#include "icloud_disk_service.h"
#include "iremote_stub.h"
#include "nocopyable.h"
#include "refbase.h"
#include "system_ability.h"

namespace OHOS {
namespace FileManagement {
namespace CloudDiskService {
enum class ServiceRunningState { STATE_NOT_START, STATE_RUNNING };

class CloudDiskService final : public SystemAbility, public CloudDiskServiceStub, protected NoCopyable {
    DECLARE_SYSTEM_ABILITY(CloudDiskService);

public:
    explicit CloudDiskService(int32_t saID, bool runOnCreate = true);
    virtual ~CloudDiskService() = default;

    void OnStart() override;
    void OnStop() override;
    ErrCode RegisterSyncFolderChangesInner(const std::string &syncFolder,
                                           const sptr<IRemoteObject> &remoteObject) override;
    ErrCode UnregisterSyncFolderChangesInner(const std::string &syncFolder) override;
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
    ErrCode RegisterSyncFolderInner(int32_t userId, const std::string &bundleName, const std::string &path) override;
    ErrCode UnregisterSyncFolderInner(int32_t userId, const std::string &bundleName, const std::string &path) override;

    int32_t UnregisterForSaInner(const std::string &path) override;
    void UnloadSa();

private:
    CloudDiskService();
    ServiceRunningState state_{ServiceRunningState::STATE_NOT_START};
    static sptr<CloudDiskService> instance_;
    bool registerToService_{false};
    void PublishSA();
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;
    std::shared_ptr<AccountStatusListener> accountStatusListener_ = nullptr;
};
} // namespace CloudDiskService
} // namespace FileManagement
} // namespace OHOS
#endif // CLOUD_DISK_SERVICE_H
