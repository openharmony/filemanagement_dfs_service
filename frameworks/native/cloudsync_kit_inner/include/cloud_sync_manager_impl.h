/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_FILEMGMT_CLOUD_SYNC_MANAGER_IMPL_H
#define OHOS_FILEMGMT_CLOUD_SYNC_MANAGER_IMPL_H

#include <atomic>

#include "nocopyable.h"

#include "cloud_sync_callback_client.h"
#include "cloud_sync_common.h"
#include "cloud_sync_manager.h"
#include "svc_death_recipient.h"
#include "system_ability_status_change_stub.h"

namespace OHOS::FileManagement::CloudSync {
class CloudSyncManagerImpl final : public CloudSyncManager, public NoCopyable {
public:
    static CloudSyncManagerImpl &GetInstance();

    int32_t RegisterCallback(const std::shared_ptr<CloudSyncCallback> callback,
                             const std::string &bundleName = "") override;
    int32_t UnRegisterCallback(const std::string &bundleName = "") override;
    int32_t StartSync(const std::string &bundleName = "") override;
    int32_t StartSync(bool forceFlag, const std::shared_ptr<CloudSyncCallback> callback) override;
    int32_t TriggerSync(const std::string &bundleName, const int32_t &userId) override;
    int32_t StopSync(const std::string &bundleName = "") override;
    int32_t ChangeAppSwitch(const std::string &accoutId, const std::string &bundleName, bool status) override;
    int32_t Clean(const std::string &accountId, const CleanOptions &cleanOptions) override;
    int32_t NotifyDataChange(const std::string &accoutId, const std::string &bundleName) override;
    int32_t NotifyEventChange(int32_t userId, const std::string &eventId, const std::string &extraData) override;
    int32_t EnableCloud(const std::string &accoutId, const SwitchDataObj &switchData) override;
    int32_t DisableCloud(const std::string &accoutId) override;
    int32_t StartDownloadFile(const std::string &uri) override;
    int32_t StartFileCache(const std::string &uri) override;
    int32_t StopDownloadFile(const std::string &uri) override;
    int32_t RegisterDownloadFileCallback(const std::shared_ptr<CloudDownloadCallback> downloadCallback) override;
    int32_t UnregisterDownloadFileCallback() override;
    int32_t GetSyncTime(int64_t &syncTime, const std::string &bundleName = "") override;
    int32_t CleanCache(const std::string &uri) override;

    class SystemAbilityStatusChange : public SystemAbilityStatusChangeStub {
    public:
        SystemAbilityStatusChange(std::shared_ptr<CloudSyncCallback> callback,
            std::shared_ptr<CloudDownloadCallback> downloadCallback)
            : callback_(callback), downloadCallback_(downloadCallback) {};
        void OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId);
        void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId);
    private:
        std::shared_ptr<CloudSyncCallback> callback_;
        std::shared_ptr<CloudDownloadCallback> downloadCallback_;
    };
private:
    CloudSyncManagerImpl() = default;
    void SetDeathRecipient(const sptr<IRemoteObject> &remoteObject);

    std::atomic_flag isFirstCall_{false};
    sptr<SvcDeathRecipient> deathRecipient_;
    std::shared_ptr<CloudSyncCallback> callback_;
    std::shared_ptr<CloudDownloadCallback> downloadCallback_;
    sptr<CloudSyncManagerImpl::SystemAbilityStatusChange> listener_;
    std::mutex subscribeMutex_;
    std::mutex callbackMutex_;
    void SubscribeListener();
};
} // namespace OHOS::FileManagement::CloudSync

#endif // OHOS_FILEMGMT_CLOUD_SYNC_MANAGER_IMPL_H
