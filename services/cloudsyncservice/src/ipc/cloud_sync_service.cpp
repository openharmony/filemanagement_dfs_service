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
#include "ipc/cloud_sync_service.h"

#include <cstdint>
#include <memory>

#include "cycle_task/cycle_task_runner.h"
#include "dfs_error.h"
#include "data_sync_const.h"
#include "dfsu_access_token_helper.h"
#include "directory_ex.h"
#include "file_transfer_manager.h"
#include "ipc/cloud_sync_callback_manager.h"
#include "ipc/download_asset_callback_manager.h"
#include "meta_file.h"
#include "sandbox_helper.h"
#include "session_manager.h"
#include "sdk_helper.h"
#include "sync_rule/battery_status.h"
#include "sync_rule/cloud_status.h"
#include "sync_rule/net_conn_callback_observer.h"
#include "sync_rule/network_status.h"
#include "system_ability_definition.h"
#include "sync_rule/screen_status.h"
#include "task_state_manager.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
using namespace std;
using namespace OHOS;
constexpr int32_t MIN_USER_ID = 100;
constexpr int LOAD_SA_TIMEOUT_MS = 4000;

REGISTER_SYSTEM_ABILITY_BY_ID(CloudSyncService, FILEMANAGEMENT_CLOUD_SYNC_SERVICE_SA_ID, false);

CloudSyncService::CloudSyncService(int32_t saID, bool runOnCreate) : SystemAbility(saID, runOnCreate)
{
    dataSyncManager_ = make_shared<DataSyncManager>();
    batteryStatusListener_ = make_shared<BatteryStatusListener>(dataSyncManager_);
    screenStatusListener_ = make_shared<ScreenStatusListener>();
}

void CloudSyncService::PublishSA()
{
    LOGI("Begin to init");
    if (!SystemAbility::Publish(this)) {
        throw runtime_error("Failed to publish the daemon");
    }
    LOGI("Init finished successfully");
}

void CloudSyncService::Init()
{
    NetworkStatus::InitNetwork(dataSyncManager_);
    /* Get Init Charging status */
    BatteryStatus::GetInitChargingStatus();
    ScreenStatus::InitScreenStatus();
    auto sessionManager = make_shared<SessionManager>();
    sessionManager->Init();
    fileTransferManager_ = make_shared<FileTransferManager>(sessionManager);
    fileTransferManager_->Init();
}

std::string CloudSyncService::GetHmdfsPath(const std::string &uri, int32_t userId)
{
    const std::string HMDFS_DIR = "/mnt/hmdfs/";
    const std::string DATA_DIR = "/account/device_view/local/data/";
    const std::string FILE_DIR = "data/storage/el2/distributedfiles/";
    const std::string URI_PREFIX = "://";
    if (uri.empty()) {
        return "";
    }

    std::string bundleName;
    size_t uriPrefixPos = uri.find(URI_PREFIX);
    if (uriPrefixPos == std::string::npos) {
        return "";
    }
    uriPrefixPos += URI_PREFIX.length();
    size_t bundleNameEndPos = uri.find('/', uriPrefixPos);
    if (bundleNameEndPos == std::string::npos) {
        return "";
    }
    bundleName = uri.substr(uriPrefixPos, bundleNameEndPos - uriPrefixPos);

    std::string relativePath;
    size_t fileDirPos = uri.find(FILE_DIR);
    if (fileDirPos == std::string::npos) {
        return "";
    }
    fileDirPos += FILE_DIR.length();
    relativePath = uri.substr(fileDirPos);

    std::string outputPath = HMDFS_DIR + std::to_string(userId) + DATA_DIR + bundleName + "/" + relativePath;
    std::string dir = outputPath.substr(0, outputPath.find_last_of('/'));

    ForceCreateDirectory(dir);
    return outputPath;
}

void CloudSyncService::OnStart(const SystemAbilityOnDemandReason& startReason)
{
    Init();
    try {
        PublishSA();
        AddSystemAbilityListener(COMMON_EVENT_SERVICE_ID);
    } catch (const exception &e) {
        LOGE("%{public}s", e.what());
    }
    LOGI("Start service successfully");
    TaskStateManager::GetInstance().DelayUnloadTask();
    HandleStartReason(startReason);
}

void CloudSyncService::OnStop()
{
    LOGI("Stop finished successfully");
}

void CloudSyncService::HandleStartReason(const SystemAbilityOnDemandReason& startReason)
{
    string reason = startReason.GetName();
    LOGI("Begin to start service reason: %{public}s", reason.c_str());
    if (reason == "usual.event.wifi.SCAN_FINISHED") {
        dataSyncManager_->TriggerRecoverySync(SyncTriggerType::NETWORK_AVAIL_TRIGGER);
    } else if (reason == "usual.event.BATTERY_OKAY") {
        dataSyncManager_->TriggerRecoverySync(SyncTriggerType::BATTERY_OK_TRIGGER);
    } else if (reason == "usual.event.SCREEN_OFF") {
        dataSyncManager_->DownloadThumb();
    }
    if (reason != "load") {
        shared_ptr<CycleTaskRunner> taskRunner = make_shared<CycleTaskRunner>(dataSyncManager_);
        taskRunner->StartTask();
    }
}

void CloudSyncService::OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    LOGI("OnAddSystemAbility systemAbilityId:%{public}d added!", systemAbilityId);
    batteryStatusListener_->Start();
    screenStatusListener_->Start();
}

void CloudSyncService::LoadRemoteSACallback::OnLoadSACompleteForRemote(const std::string &deviceId,
                                                                       int32_t systemAbilityId,
                                                                       const sptr<IRemoteObject> &remoteObject)
{
    LOGI("Load CloudSync SA success,systemAbilityId:%{public}d, remoteObj result:%{public}s", systemAbilityId,
         (remoteObject == nullptr ? "false" : "true"));
    unique_lock<mutex> lock(loadRemoteSAMutex_);
    isLoadSuccess_.store(true);
    proxyConVar_.notify_one();
}

int32_t CloudSyncService::LoadRemoteSA(const std::string &deviceId)
{
    unique_lock<mutex> lock(loadRemoteSAMutex_);
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        LOGE("Samgr is nullptr");
        return E_SA_LOAD_FAILED;
    }
    sptr<LoadRemoteSACallback> cloudSyncLoadCallback = new LoadRemoteSACallback();
    if (cloudSyncLoadCallback == nullptr) {
        LOGE("cloudSyncLoadCallback is nullptr");
        return E_SA_LOAD_FAILED;
    }
    int32_t ret = samgr->LoadSystemAbility(FILEMANAGEMENT_CLOUD_SYNC_SERVICE_SA_ID, deviceId, cloudSyncLoadCallback);
    if (ret != E_OK) {
        LOGE("Failed to Load systemAbility, systemAbilityId:%{pulbic}d, ret code:%{pulbic}d",
             FILEMANAGEMENT_CLOUD_SYNC_SERVICE_SA_ID, ret);
        return E_SA_LOAD_FAILED;
    }

    auto waitStatus = cloudSyncLoadCallback->proxyConVar_.wait_for(
        lock, std::chrono::milliseconds(LOAD_SA_TIMEOUT_MS),
        [cloudSyncLoadCallback]() { return cloudSyncLoadCallback->isLoadSuccess_.load(); });
    if (!waitStatus) {
        LOGE("Load CloudSynd SA timeout");
        return E_SA_LOAD_FAILED;
    }
    return E_OK;
}

int32_t CloudSyncService::UnRegisterCallbackInner()
{
    string bundleName;
    if (DfsuAccessTokenHelper::GetCallerBundleName(bundleName)) {
        return E_INVAL_ARG;
    }

    CloudSyncCallbackManager::GetInstance().RemoveCallback(bundleName);
    return E_OK;
}

int32_t CloudSyncService::RegisterCallbackInner(const sptr<IRemoteObject> &remoteObject)
{
    if (remoteObject == nullptr) {
        LOGE("remoteObject is nullptr");
        return E_INVAL_ARG;
    }

    string bundleName;
    if (DfsuAccessTokenHelper::GetCallerBundleName(bundleName)) {
        return E_INVAL_ARG;
    }

    auto callback = iface_cast<ICloudSyncCallback>(remoteObject);
    auto callerUserId = DfsuAccessTokenHelper::GetUserId();
    CloudSyncCallbackManager::GetInstance().AddCallback(bundleName, callerUserId, callback);
    dataSyncManager_->RegisterCloudSyncCallback(bundleName, callerUserId);
    return E_OK;
}

int32_t CloudSyncService::StartSyncInner(bool forceFlag)
{
    string bundleName;
    if (DfsuAccessTokenHelper::GetCallerBundleName(bundleName)) {
        return E_INVAL_ARG;
    }
    auto callerUserId = DfsuAccessTokenHelper::GetUserId();
    return dataSyncManager_->TriggerStartSync(bundleName, callerUserId, forceFlag,
        SyncTriggerType::APP_TRIGGER);
}

int32_t CloudSyncService::TriggerSyncInner(const std::string &bundleName, const int32_t &userId)
{
    if (bundleName.empty() || userId < MIN_USER_ID) {
        LOGE("Trigger sync parameter is invalid");
        return E_INVAL_ARG;
    }
    return dataSyncManager_->TriggerStartSync(bundleName, userId, false,
        SyncTriggerType::APP_TRIGGER);
}

int32_t CloudSyncService::StopSyncInner()
{
    string bundleName;
    if (DfsuAccessTokenHelper::GetCallerBundleName(bundleName)) {
        return E_INVAL_ARG;
    }
    auto callerUserId = DfsuAccessTokenHelper::GetUserId();
    return dataSyncManager_->TriggerStopSync(bundleName, callerUserId, SyncTriggerType::APP_TRIGGER);
}

int32_t CloudSyncService::GetSyncTimeInner(int64_t &syncTime)
{
    syncTime = 0;
    LOGI("GetLasySyncTime(service) is ok");
    return E_OK;
}

int32_t CloudSyncService::CleanCacheInner(const std::string &uri)
{
    string bundleName;
    if (DfsuAccessTokenHelper::GetCallerBundleName(bundleName)) {
        return E_INVAL_ARG;
    }
    auto callerUserId = DfsuAccessTokenHelper::GetUserId();
    return dataSyncManager_->CleanCache(bundleName, callerUserId, uri);
}

int32_t CloudSyncService::ChangeAppSwitch(const std::string &accoutId, const std::string &bundleName, bool status)
{
    auto callerUserId = DfsuAccessTokenHelper::GetUserId();

    /* update app switch status */
    auto ret = CloudStatus::ChangeAppSwitch(bundleName, callerUserId, status);
    if (ret != E_OK) {
        return ret;
    }
    if (status) {
        dataSyncManager_->RestoreClean(bundleName, callerUserId);
        return dataSyncManager_->TriggerStartSync(bundleName, callerUserId, false, SyncTriggerType::CLOUD_TRIGGER);
    } else {
        return dataSyncManager_->TriggerStopSync(bundleName, callerUserId, SyncTriggerType::CLOUD_TRIGGER);
    }
}

int32_t CloudSyncService::NotifyDataChange(const std::string &accoutId, const std::string &bundleName)
{
    auto callerUserId = DfsuAccessTokenHelper::GetUserId();
    return dataSyncManager_->TriggerStartSync(bundleName, callerUserId, false, SyncTriggerType::CLOUD_TRIGGER);
}

int32_t CloudSyncService::NotifyEventChange(int32_t userId, const std::string &eventId, const std::string &extraData)
{
    auto driveKit = DriveKit::DriveKitNative::GetInstance(userId);
    if (driveKit == nullptr) {
        LOGE("sdk helper get drive kit instance fail");
        return E_CLOUD_SDK;
    }

    DriveKit::DKUserInfo userInfo;
    auto err = driveKit->GetCloudUserInfo(userInfo);
    if (err.HasError()) {
        LOGE("GetCloudUserInfo failed, server err:%{public}d and dk err:%{public}d", err.serverErrorCode,
             err.dkErrorCode);
        return E_CLOUD_SDK;
    }

    DriveKit::DKRecordChangeEvent event;
    auto eventErr = driveKit->ResolveNotificationEvent(extraData, event);
    if (eventErr.HasError()) {
        LOGE("ResolveNotificationEvent failed, server err:%{public}d and dk err:%{public}d", eventErr.serverErrorCode,
             eventErr.dkErrorCode);
        return E_CLOUD_SDK;
    }
    return dataSyncManager_->TriggerStartSync(event.appBundleName, userId, false, SyncTriggerType::CLOUD_TRIGGER);
}

int32_t CloudSyncService::DisableCloud(const std::string &accoutId)
{
    auto callerUserId = DfsuAccessTokenHelper::GetUserId();
    return dataSyncManager_->DisableCloud(callerUserId);
}

int32_t CloudSyncService::EnableCloud(const std::string &accoutId, const SwitchDataObj &switchData)
{
    return E_OK;
}

int32_t CloudSyncService::Clean(const std::string &accountId, const CleanOptions &cleanOptions)
{
    LOGD("Clean accountId is: %{public}s", accountId.c_str());
    for (auto &iter : cleanOptions.appActionsData) {
        LOGD("Clean key is: %s, value is: %d", iter.first.c_str(), iter.second);
    }

    MetaFileMgr::GetInstance().ClearAll();
    auto callerUserId = DfsuAccessTokenHelper::GetUserId();
    LOGD("Clean callerUserId is: %{public}d", callerUserId);
    for (auto iter = cleanOptions.appActionsData.begin(); iter != cleanOptions.appActionsData.end(); ++iter) {
        dataSyncManager_->CleanCloudFile(callerUserId, iter->first, iter->second);
    }

    return E_OK;
}
int32_t CloudSyncService::StartFileCache(const std::string &uriStr)
{
    Uri uri(uriStr);
    string bundleName = uri.GetAuthority();
    auto callerUserId = DfsuAccessTokenHelper::GetUserId();
    return dataSyncManager_->StartDownloadFile(bundleName, callerUserId, uriStr);
}

constexpr int TEST_MAIN_USR_ID = 100;
int32_t CloudSyncService::StartDownloadFile(const std::string &path)
{
    string bundleName;
    if (DfsuAccessTokenHelper::GetCallerBundleName(bundleName)) {
        return E_INVAL_ARG;
    }
    auto callerUserId = DfsuAccessTokenHelper::GetUserId();
    LOGI("start StartDownloadFile");
    if (callerUserId == 0) {
        callerUserId = TEST_MAIN_USR_ID; // for root user change id to main user for test
    }
    return dataSyncManager_->StartDownloadFile(bundleName, callerUserId, path);
}

int32_t CloudSyncService::StopDownloadFile(const std::string &path)
{
    string bundleName;
    if (DfsuAccessTokenHelper::GetCallerBundleName(bundleName)) {
        return E_INVAL_ARG;
    }
    auto callerUserId = DfsuAccessTokenHelper::GetUserId();
    LOGI("start StopDownloadFile");
    if (callerUserId == 0) {
        callerUserId = TEST_MAIN_USR_ID; // for root user change id to main user for test
    }
    return dataSyncManager_->StopDownloadFile(bundleName, callerUserId, path);
}

int32_t CloudSyncService::RegisterDownloadFileCallback(const sptr<IRemoteObject> &downloadCallback)
{
    string bundleName;
    if (DfsuAccessTokenHelper::GetCallerBundleName(bundleName)) {
        return E_INVAL_ARG;
    }
    auto callerUserId = DfsuAccessTokenHelper::GetUserId();
    auto downloadCb = iface_cast<ICloudDownloadCallback>(downloadCallback);
    LOGI("start RegisterDownloadFileCallback");
    if (callerUserId == 0) {
        callerUserId = TEST_MAIN_USR_ID; // for root user change id to main user for test
    }
    return dataSyncManager_->RegisterDownloadFileCallback(bundleName, callerUserId, downloadCb);
}

int32_t CloudSyncService::UnregisterDownloadFileCallback()
{
    string bundleName;
    if (DfsuAccessTokenHelper::GetCallerBundleName(bundleName)) {
        return E_INVAL_ARG;
    }
    auto callerUserId = DfsuAccessTokenHelper::GetUserId();
    LOGI("start UnregisterDownloadFileCallback");
    if (callerUserId == 0) {
        callerUserId = TEST_MAIN_USR_ID; // for root user change id to main user for test
    }
    return dataSyncManager_->UnregisterDownloadFileCallback(bundleName, callerUserId);
}

int32_t CloudSyncService::UploadAsset(const int32_t userId, const std::string &request, std::string &result)
{
    auto driveKit = DriveKit::DriveKitNative::GetInstance(userId);
    if (driveKit == nullptr) {
        LOGE("uploadAsset get drive kit instance err");
        return E_CLOUD_SDK;
    }
    return driveKit->OnUploadAsset(request, result);
}

int32_t CloudSyncService::DownloadFile(const int32_t userId, const std::string &bundleName, AssetInfoObj &assetInfoObj)
{
    auto sdkHelper = std::make_shared<SdkHelper>();
    auto ret = sdkHelper->Init(userId, bundleName);
    if (ret != E_OK) {
        LOGE("get sdk helper err %{public}d", ret);
        return ret;
    }

    DriveKit::DKAsset asset;
    asset.assetName = assetInfoObj.assetName;

    asset.uri = GetHmdfsPath(assetInfoObj.uri, userId);
    if (asset.uri.empty()) {
        LOGE("fail to get download path from %{public}s", assetInfoObj.uri.c_str());
        return E_INVAL_ARG;
    }

    // Not to pass the assetinfo.fieldkey
    DriveKit::DKDownloadAsset assetsToDownload{assetInfoObj.recordType, assetInfoObj.recordId, {}, asset, {}};
    return sdkHelper->DownloadAssets(assetsToDownload);
}

int32_t CloudSyncService::DownloadAsset(const uint64_t taskId,
                                        const int32_t userId,
                                        const std::string &bundleName,
                                        const std::string &networkId,
                                        AssetInfoObj &assetInfoObj)
{
    if (networkId == "edge2cloud") {
        LOGE("now not support");
        return E_INVAL_ARG;
    }
    // Load sa for remote device
    if (LoadRemoteSA(networkId) != E_OK) { // maybe need to convert deviceId
        return E_SA_LOAD_FAILED;
    }

    string uri = assetInfoObj.uri;
    fileTransferManager_->DownloadFileFromRemoteDevice(networkId, userId, taskId, uri);
    return E_OK;
}

int32_t CloudSyncService::RegisterDownloadAssetCallback(const sptr<IRemoteObject> &remoteObject)
{
    if (remoteObject == nullptr) {
        LOGE("remoteObject is nullptr");
        return E_INVAL_ARG;
    }
    auto callback = iface_cast<IDownloadAssetCallback>(remoteObject);
    DownloadAssetCallbackManager::GetInstance().AddCallback(callback);
    return E_OK;
}

int32_t CloudSyncService::DeleteAsset(const int32_t userId, const std::string &uri)
{
    std::string physicalPath = "";
    int ret = AppFileService::SandboxHelper::GetPhysicalPath(uri, std::to_string(userId), physicalPath);
    if (ret != 0) {
        LOGE("Get physical path failed with %{public}d", ret);
        return E_GET_PHYSICAL_PATH_FAILED;
    }

    LOGD("delete assert, path %{public}s", physicalPath.c_str());

    ret = unlink(physicalPath.c_str());
    if (ret != 0) {
        LOGE("fail to delete asset, errno %{public}d", errno);
        return E_DELETE_FAILED;
    }
    return E_OK;
}
} // namespace OHOS::FileManagement::CloudSync
