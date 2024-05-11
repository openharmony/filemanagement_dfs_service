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

#ifndef OHOS_FILEMGMT_CLOUD_SYNC_SERVICE_PROXY_H
#define OHOS_FILEMGMT_CLOUD_SYNC_SERVICE_PROXY_H

#include "iremote_proxy.h"
#include "system_ability_load_callback_stub.h"

#include "i_cloud_sync_service.h"

namespace OHOS::FileManagement::CloudSync {
class CloudSyncServiceProxy : public IRemoteProxy<ICloudSyncService> {
public:
    explicit CloudSyncServiceProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<ICloudSyncService>(impl) {}
    ~CloudSyncServiceProxy() override {}

    static sptr<ICloudSyncService> GetInstance();
    static void InvaildInstance();
    int32_t UnRegisterCallbackInner(const std::string &bundleName = "") override;
    int32_t RegisterCallbackInner(const sptr<IRemoteObject> &remoteObject, const std::string &bundleName = "") override;
    int32_t StartSyncInner(bool forceFlag, const std::string &bundleName = "") override;
    int32_t TriggerSyncInner(const std::string &bundleName, const int32_t &userId) override;
    int32_t StopSyncInner(const std::string &bundleName = "") override;
    int32_t ChangeAppSwitch(const std::string &accoutId, const std::string &bundleName, bool status) override;
    int32_t Clean(const std::string &accountId, const CleanOptions &cleanOptions) override;
    int32_t NotifyDataChange(const std::string &accoutId, const std::string &bundleName) override;
    int32_t NotifyEventChange(int32_t userId, const std::string &eventId, const std::string &extraData) override;
    int32_t EnableCloud(const std::string &accoutId, const SwitchDataObj &switchData) override;
    int32_t DisableCloud(const std::string &accoutId) override;
    int32_t StartDownloadFile(const std::string &uri) override;
    int32_t StartFileCache(const std::string &uri) override;
    int32_t StopDownloadFile(const std::string &uri) override;
    int32_t RegisterDownloadFileCallback(const sptr<IRemoteObject> &downloadCallback) override;
    int32_t UnregisterDownloadFileCallback() override;
    int32_t UploadAsset(const int32_t userId, const std::string &request, std::string &result) override;
    int32_t DownloadFile(const int32_t userId, const std::string &bundleName, AssetInfoObj &assetInfoObj) override;
    int32_t DownloadFiles(const int32_t userId,
                          const std::string &bundleName,
                          const std::vector<AssetInfoObj> &assetInfoObj,
                          std::vector<bool> &assetResultMap) override;
    int32_t DownloadAsset(const uint64_t taskId, const int32_t userId,
                          const std::string &bundleName,
                          const std::string &networkId,
                          AssetInfoObj &assetInfoObj) override;
    int32_t RegisterDownloadAssetCallback(const sptr<IRemoteObject> &remoteObject) override;
    int32_t DeleteAsset(const int32_t userId, const std::string &uri) override;
    int32_t GetSyncTimeInner(int64_t &syncTime, const std::string &bundleName = "") override;
    int32_t CleanCacheInner(const std::string &uri) override;

    class ServiceProxyLoadCallback : public SystemAbilityLoadCallbackStub {
    public:
        void OnLoadSystemAbilitySuccess(int32_t systemAbilityId, const sptr<IRemoteObject> &remoteObject) override;
        void OnLoadSystemAbilityFail(int32_t systemAbilityId) override;

        std::condition_variable proxyConVar_;
        std::atomic<bool> isLoadSuccess_{false};
    };

private:
    static inline std::mutex proxyMutex_;
    static inline std::mutex instanceMutex_;
    static inline std::mutex downloadMutex_;
    static inline sptr<ICloudSyncService> serviceProxy_;
    static inline BrokerDelegator<CloudSyncServiceProxy> delegator_;
};
} // namespace OHOS::FileManagement::CloudSync

#endif // OHOS_FILEMGMT_CLOUD_SYNC_SERVICE_PROXY_H
