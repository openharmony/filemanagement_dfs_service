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

#ifndef OHOS_FILEMGMT_CLOUD_SYNC_SERVICE_H
#define OHOS_FILEMGMT_CLOUD_SYNC_SERVICE_H

#include <map>

#include "nocopyable.h"
#include "system_ability.h"

#include "cloud_sync_service_stub.h"
#include "i_cloud_download_callback.h"
#include "i_cloud_sync_callback.h"
#include "sync_rule/battery_status_listener.h"
#include "sync_rule/screen_status_listener.h"

namespace OHOS::FileManagement::CloudSync {
class CloudSyncService final : public SystemAbility, public CloudSyncServiceStub, protected NoCopyable {
    DECLARE_SYSTEM_ABILITY(CloudSyncService);

public:
    explicit CloudSyncService(int32_t saID, bool runOnCreate = true);
    virtual ~CloudSyncService() = default;
    int32_t UnRegisterCallbackInner() override;
    int32_t RegisterCallbackInner(const sptr<IRemoteObject> &remoteObject) override;
    int32_t StartSyncInner(bool forceFlag) override;
    int32_t StopSyncInner() override;
    int32_t ChangeAppSwitch(const std::string &accoutId, const std::string &bundleName, bool status) override;
    int32_t Clean(const std::string &accountId, const CleanOptions &cleanOptions) override;
    int32_t NotifyDataChange(const std::string &accoutId, const std::string &bundleName) override;
    int32_t EnableCloud(const std::string &accoutId, const SwitchDataObj &switchData) override;
    int32_t DisableCloud(const std::string &accoutId) override;
    int32_t StartDownloadFile(const std::string &path) override;
    int32_t StopDownloadFile(const std::string &path) override;
    int32_t RegisterDownloadFileCallback(const sptr<IRemoteObject> &downloadCallback) override;
    int32_t UnregisterDownloadFileCallback() override;
    int32_t UploadAsset(const int32_t userId, const std::string &request, std::string &result) override;
    int32_t DownloadFile(const int32_t userId, const std::string &bundleName, AssetInfoObj &assetInfoObj) override;
    int32_t DeleteAsset(const int32_t userId, const std::string &uri) override;

private:
    std::string GetHmdfsPath(const std::string &uri, int32_t userId);
    void OnStart(const SystemAbilityOnDemandReason& startReason) override;
    void OnStop() override;
    void PublishSA();
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;
    void Init();
    void HandleStartReason(const SystemAbilityOnDemandReason& startReason);

    std::shared_ptr<DataSyncManager> dataSyncManager_;
    std::shared_ptr<BatteryStatusListener> batteryStatusListener_;
    std::shared_ptr<ScreenStatusListener> screenStatusListener_;
};
} // namespace OHOS::FileManagement::CloudSync

#endif // OHOS_FILEMGMT_CLOUD_SYNC_SERVICE_H
