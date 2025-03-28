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

#include "iservice_registry.h"
#include "nocopyable.h"
#include "system_ability.h"
#include "system_ability_load_callback_stub.h"

#include "cloud_sync_service_stub.h"
#include "file_transfer_manager.h"
#include "i_cloud_download_callback.h"
#include "i_cloud_sync_callback.h"
#include "svc_death_recipient.h"
#include "sync_rule/battery_status_listener.h"
#include "sync_rule/package_status_listener.h"
#include "sync_rule/screen_status_listener.h"
#include "sync_rule/user_status_listener.h"

namespace OHOS::FileManagement::CloudSync {
class CloudSyncService final : public SystemAbility, public CloudSyncServiceStub, protected NoCopyable {
    DECLARE_SYSTEM_ABILITY(CloudSyncService);

public:
    explicit CloudSyncService(int32_t saID, bool runOnCreate = true);
    virtual ~CloudSyncService() = default;
    ErrCode UnRegisterCallbackInner(const std::string &bundleName = "") override;
    ErrCode UnRegisterFileSyncCallbackInner(const std::string &bundleName = "") override;
    ErrCode RegisterCallbackInner(const sptr<IRemoteObject> &remoteObject, const std::string &bundleName = "") override;
    ErrCode RegisterFileSyncCallbackInner(const sptr<IRemoteObject> &remoteObject,
                                          const std::string &bundleName = "") override;
    ErrCode StartSyncInner(bool forceFlag, const std::string &bundleName = "") override;
    ErrCode StartFileSyncInner(bool forceFlag, const std::string &bundleName = "") override;
    ErrCode TriggerSyncInner(const std::string &bundleName, int32_t userId) override;
    ErrCode StopSyncInner(const std::string &bundleName = "", bool forceFlag = false) override;
    ErrCode StopFileSyncInner(const std::string &bundleName = "", bool forceFlag = false) override;
    ErrCode ResetCursor(const std::string &bundleName = "") override;
    ErrCode OptimizeStorage(const OptimizeSpaceOptions &optimizeOptions,
                            bool isCallbackValid,
                            const sptr<IRemoteObject> &optimizeCallback) override;
    ErrCode StopOptimizeStorage() override;
    ErrCode ChangeAppSwitch(const std::string &accoutId, const std::string &bundleName, bool status) override;
    ErrCode Clean(const std::string &accountId, const CleanOptions &cleanOptions) override;
    ErrCode NotifyDataChange(const std::string &accoutId, const std::string &bundleName) override;
    ErrCode NotifyEventChange(int32_t userId, const std::string &eventId, const std::string &extraData) override;
    ErrCode EnableCloud(const std::string &accoutId, const SwitchDataObj &switchData) override;
    ErrCode DisableCloud(const std::string &accoutId) override;
    ErrCode StartDownloadFile(const std::string &path) override;
    ErrCode StartFileCache(const std::vector<std::string> &uriVec,
                           int64_t &downloadId,
                           int32_t fieldkey,
                           bool isCallbackValid,
                           const sptr<IRemoteObject> &downloadCallback,
                           int32_t timeout = -1) override;
    ErrCode StopDownloadFile(const std::string &path, bool needClean = false) override;
    ErrCode StopFileCache(int64_t downloadId, bool needClean = false, int32_t timeout = -1) override;
    ErrCode DownloadThumb() override;
    ErrCode RegisterDownloadFileCallback(const sptr<IRemoteObject> &downloadCallback) override;
    ErrCode RegisterFileCacheCallback(const sptr<IRemoteObject> &downloadCallback) override;
    ErrCode UnregisterDownloadFileCallback() override;
    ErrCode UnregisterFileCacheCallback() override;
    ErrCode UploadAsset(const int32_t userId, const std::string &request, std::string &result) override;
    ErrCode DownloadFile(const int32_t userId, const std::string &bundleName,
                         const AssetInfoObj &assetInfoObj) override;
    ErrCode DownloadFiles(const int32_t userId,
                          const std::string &bundleName,
                          const std::vector<AssetInfoObj> &assetInfoObj,
                          std::vector<bool> &assetResultMap) override;
    ErrCode DownloadAsset(const uint64_t taskId,
                          const int32_t userId,
                          const std::string &bundleName,
                          const std::string &networkId,
                          const AssetInfoObj &assetInfoObj) override;
    ErrCode RegisterDownloadAssetCallback(const sptr<IRemoteObject> &remoteObject) override;
    ErrCode DeleteAsset(const int32_t userId, const std::string &uri) override;
    ErrCode GetSyncTimeInner(int64_t &syncTime, const std::string &bundleName = "") override;
    ErrCode CleanCacheInner(const std::string &uri) override;
    void SetDeathRecipient(const sptr<IRemoteObject> &remoteObject);
    ErrCode BatchCleanFile(const std::vector<CleanFileInfoObj> &fileInfo,
                           std::vector<std::string> &failCloudId) override;
    ErrCode BatchDentryFileInsert(const std::vector<DentryFileInfoObj> &fileInfo,
                                  std::vector<std::string> &failCloudId) override;

private:
    std::string GetHmdfsPath(const std::string &uri, int32_t userId);
    void OnStart(const SystemAbilityOnDemandReason &startReason) override;
    void OnActive(const SystemAbilityOnDemandReason &startReason) override;
    void OnStop() override;
    void PublishSA();
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;
    void PreInit();
    void Init();
    void HandleStartReason(const SystemAbilityOnDemandReason &startReason);
    void HandlePackageRemoved(const SystemAbilityOnDemandReason &startReason);
    int32_t GetBundleNameUserInfo(BundleNameUserInfo &bundleNameUserInfo);
    void GetBundleNameUserInfo(const std::vector<std::string> &uriVec, BundleNameUserInfo &bundleNameUserInfo);

    class LoadRemoteSACallback : public SystemAbilityLoadCallbackStub {
    public:
        void OnLoadSACompleteForRemote(const std::string &deviceId,
                                       int32_t systemAbilityId,
                                       const sptr<IRemoteObject> &remoteObject);
        std::condition_variable proxyConVar_;
        std::atomic<bool> isLoadSuccess_{false};
    };

    int32_t LoadRemoteSA(const std::string &deviceId);

    static inline std::mutex loadRemoteSAMutex_;

    std::shared_ptr<CloudFile::DataSyncManager> dataSyncManager_;
    std::shared_ptr<UserStatusListener> userStatusListener_;
    std::shared_ptr<BatteryStatusListener> batteryStatusListener_;
    std::shared_ptr<PackageStatusListener> packageStatusListener_;
    std::shared_ptr<ScreenStatusListener> screenStatusListener_;
    std::shared_ptr<FileTransferManager> fileTransferManager_;
    sptr<SvcDeathRecipient> deathRecipient_;
    static inline std::map<std::string, sptr<OHOS::IRemoteObject>> remoteObjectMap_;
};
} // namespace OHOS::FileManagement::CloudSync

#endif // OHOS_FILEMGMT_CLOUD_SYNC_SERVICE_H
