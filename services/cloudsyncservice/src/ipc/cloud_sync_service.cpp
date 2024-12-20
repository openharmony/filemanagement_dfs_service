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

#include "battery_status.h"
#include "cloud_file_kit.h"
#include "cloud_status.h"
#include "cycle_task/cycle_task_runner.h"
#include "data_sync_const.h"
#include "data_syncer_rdb_store.h"
#include "dfs_error.h"
#include "dfsu_access_token_helper.h"
#include "directory_ex.h"
#include "ipc/download_asset_callback_manager.h"
#include "meta_file.h"
#include "net_conn_callback_observer.h"
#include "parameters.h"
#include "periodic_check_task.h"
#include "plugin_loader.h"
#include "network_status.h"
#include "sandbox_helper.h"
#include "screen_status.h"
#include "session_manager.h"
#include "system_ability_definition.h"
#include "system_load.h"
#include "task_state_manager.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
using namespace std;
using namespace OHOS;
using namespace CloudFile;
constexpr int32_t MIN_USER_ID = 100;
constexpr int LOAD_SA_TIMEOUT_MS = 4000;

REGISTER_SYSTEM_ABILITY_BY_ID(CloudSyncService, FILEMANAGEMENT_CLOUD_SYNC_SERVICE_SA_ID, false);

CloudSyncService::CloudSyncService(int32_t saID, bool runOnCreate) : SystemAbility(saID, runOnCreate)
{
}

void CloudSyncService::PublishSA()
{
    LOGI("Begin to init");
    if (!SystemAbility::Publish(this)) {
        throw runtime_error("Failed to publish the daemon");
    }
    LOGI("Init finished successfully");
}

void CloudSyncService::PreInit()
{
    /* load cloud file ext plugin */
    CloudFile::PluginLoader::GetInstance().LoadCloudKitPlugin(true);
    auto instance = CloudFile::CloudFileKit::GetInstance();
    if (instance == nullptr) {
        LOGE("get cloud file helper instance failed");
        dataSyncManager_ = make_shared<DataSyncManager>();
    } else {
        dataSyncManager_ = instance->GetDataSyncManager();
    }

    batteryStatusListener_ = make_shared<BatteryStatusListener>(dataSyncManager_);
    screenStatusListener_ = make_shared<ScreenStatusListener>(dataSyncManager_);
    userStatusListener_ = make_shared<UserStatusListener>(dataSyncManager_);
}

void CloudSyncService::Init()
{
    NetworkStatus::InitNetwork(dataSyncManager_);
    /* Get Init Charging status */
    BatteryStatus::GetInitChargingStatus();
    ScreenStatus::InitScreenStatus();
}

constexpr int TEST_MAIN_USR_ID = 100;
int32_t CloudSyncService::GetBundleNameUserInfo(BundleNameUserInfo &bundleNameUserInfo)
{
    string bundleName;
    if (DfsuAccessTokenHelper::GetCallerBundleName(bundleName)) {
        return E_INVAL_ARG;
    }
    bundleNameUserInfo.bundleName = bundleName;

    auto callerUserId = DfsuAccessTokenHelper::GetUserId();
    if (callerUserId == 0) {
        callerUserId = TEST_MAIN_USR_ID; // for root user change id to main user for test
    }
    bundleNameUserInfo.userId = callerUserId;

    auto callerPid = DfsuAccessTokenHelper::GetPid();
    bundleNameUserInfo.pid = callerPid;

    return E_OK;
}

void CloudSyncService::GetBundleNameUserInfo(const std::vector<std::string> &uriVec,
                                             BundleNameUserInfo &bundleNameUserInfo)
{
    Uri uri(uriVec[0]);
    string bundleName = uri.GetAuthority();
    bundleNameUserInfo.bundleName = bundleName;

    auto callerUserId = DfsuAccessTokenHelper::GetUserId();
    if (callerUserId == 0) {
        callerUserId = TEST_MAIN_USR_ID; // for root user change id to main user for test
    }
    bundleNameUserInfo.userId = callerUserId;

    auto callerPid = DfsuAccessTokenHelper::GetPid();
    bundleNameUserInfo.pid = callerPid;
}

std::string CloudSyncService::GetHmdfsPath(const std::string &uri, int32_t userId)
{
    const std::string HMDFS_DIR = "/mnt/hmdfs/";
    const std::string DATA_DIR = "/account/device_view/local/data/";
    const std::string FILE_DIR = "data/storage/el2/distributedfiles/";
    const std::string URI_PREFIX = "://";
    if (uri.empty() || uri.find("..") != std::string::npos) {
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
    PreInit();
    try {
        PublishSA();
        AddSystemAbilityListener(COMMON_EVENT_SERVICE_ID);
        AddSystemAbilityListener(SOFTBUS_SERVER_SA_ID);
        AddSystemAbilityListener(RES_SCHED_SYS_ABILITY_ID);
    } catch (const exception &e) {
        LOGE("%{public}s", e.what());
    }
    LOGI("Start service successfully");
    Init();
    LOGI("init service successfully");
    system::SetParameter(CLOUD_FILE_SERVICE_SA_STATUS_FLAG, CLOUD_FILE_SERVICE_SA_START);
    TaskStateManager::GetInstance().StartTask();
    // 跟随进程生命周期
    ffrt::submit([startReason, this]() {
        this->HandleStartReason(startReason);
    });
}

void CloudSyncService::OnActive(const SystemAbilityOnDemandReason& startReason)
{
    LOGI("active service successfully");
    system::SetParameter(CLOUD_FILE_SERVICE_SA_STATUS_FLAG, CLOUD_FILE_SERVICE_SA_START);
    TaskStateManager::GetInstance().StartTask();
    ffrt::submit([startReason, this]() {
        this->HandleStartReason(startReason);
    });
}

void CloudSyncService::OnStop()
{
    LOGI("Stop finished successfully");
}

void CloudSyncService::HandleStartReason(const SystemAbilityOnDemandReason& startReason)
{
    string reason = startReason.GetName();
    int32_t userId = 0;

    LOGI("Begin to start service reason: %{public}s", reason.c_str());

    if (reason == "usual.event.USER_UNLOCKED") {
        return;
    }

    if (dataSyncManager_->GetUserId(userId) != E_OK) {
        return;
    }

    if (reason == "usual.event.wifi.CONN_STATE") {
        dataSyncManager_->TriggerRecoverySync(SyncTriggerType::NETWORK_AVAIL_TRIGGER);
        dataSyncManager_->DownloadThumb();
        dataSyncManager_->CacheVideo();
    } else if (reason == "usual.event.BATTERY_OKAY") {
        dataSyncManager_->TriggerRecoverySync(SyncTriggerType::BATTERY_OK_TRIGGER);
    } else if (reason == "usual.event.SCREEN_OFF" || reason == "usual.event.POWER_CONNECTED") {
        dataSyncManager_->DownloadThumb();
        dataSyncManager_->CacheVideo();
    }

    if (reason != "load") {
        shared_ptr<CycleTaskRunner> taskRunner = make_shared<CycleTaskRunner>(dataSyncManager_);
        taskRunner->StartTask();
    }
}

void CloudSyncService::OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    LOGI("OnAddSystemAbility systemAbilityId:%{public}d added!", systemAbilityId);
    if (systemAbilityId == COMMON_EVENT_SERVICE_ID) {
        userStatusListener_->Start();
        batteryStatusListener_->Start();
        screenStatusListener_->Start();
    } else if (systemAbilityId == SOFTBUS_SERVER_SA_ID) {
        auto sessionManager = make_shared<SessionManager>();
        sessionManager->Init();
        userStatusListener_->AddObserver(sessionManager);
        fileTransferManager_ = make_shared<FileTransferManager>(sessionManager);
        fileTransferManager_->Init();
    } else if (systemAbilityId == RES_SCHED_SYS_ABILITY_ID) {
        SystemLoadStatus::InitSystemload(dataSyncManager_);
    } else {
        LOGE("unexpected");
    }
}

void CloudSyncService::LoadRemoteSACallback::OnLoadSACompleteForRemote(const std::string &deviceId,
                                                                       int32_t systemAbilityId,
                                                                       const sptr<IRemoteObject> &remoteObject)
{
    LOGI("Load CloudSync SA success,systemAbilityId:%{public}d, remoteObj result:%{public}s", systemAbilityId,
         (remoteObject == nullptr ? "false" : "true"));
    unique_lock<mutex> lock(loadRemoteSAMutex_);
    if (remoteObject == nullptr) {
        isLoadSuccess_.store(false);
    } else {
        isLoadSuccess_.store(true);
        remoteObjectMap_[deviceId] = remoteObject;
    }
    proxyConVar_.notify_one();
}

void CloudSyncService::SetDeathRecipient(const sptr<IRemoteObject> &remoteObject)
{
    LOGD("set death recipient");
    auto deathCallback = [this](const wptr<IRemoteObject> &obj) {
        unique_lock<mutex> lock(loadRemoteSAMutex_);
        for (auto it = remoteObjectMap_.begin(); it != remoteObjectMap_.end();) {
            if (it->second.GetRefPtr() == obj.GetRefPtr()) {
                it = remoteObjectMap_.erase(it);
                LOGD("remote sa died");
            } else {
                ++it;
            }
        }
    };
    deathRecipient_ = sptr(new SvcDeathRecipient(deathCallback));
    remoteObject->AddDeathRecipient(deathRecipient_);
}

int32_t CloudSyncService::LoadRemoteSA(const std::string &deviceId)
{
    unique_lock<mutex> lock(loadRemoteSAMutex_);
    auto iter = remoteObjectMap_.find(deviceId);
    if (iter != remoteObjectMap_.end()) {
        return E_OK;
    }
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
    SetDeathRecipient(remoteObjectMap_[deviceId]);
    return E_OK;
}

static int32_t GetTargetBundleName(string &targetBundleName, string &callerBundleName)
{
    if (DfsuAccessTokenHelper::GetCallerBundleName(callerBundleName)) {
        return E_INVAL_ARG;
    }
    if (targetBundleName == "") {
        targetBundleName = callerBundleName;
    }
    if (targetBundleName != callerBundleName &&
        !DfsuAccessTokenHelper::CheckCallerPermission(PERM_CLOUD_SYNC_MANAGER)) {
        LOGE("permission denied: cloudfile_sync_manager");
        return E_PERMISSION_DENIED;
    }
    return E_OK;
}

int32_t CloudSyncService::UnRegisterCallbackInner(const string &bundleName)
{
    string targetBundleName = bundleName;
    string callerBundleName = "";
    int32_t ret = GetTargetBundleName(targetBundleName, callerBundleName);
    if (ret != E_OK) {
        LOGE("get bundle name failed: %{public}d", ret);
        return ret;
    }
    dataSyncManager_->UnRegisterCloudSyncCallback(targetBundleName, callerBundleName);
    return E_OK;
}

int32_t CloudSyncService::RegisterCallbackInner(const sptr<IRemoteObject> &remoteObject, const string &bundleName)
{
    if (remoteObject == nullptr) {
        LOGE("remoteObject is nullptr");
        return E_INVAL_ARG;
    }

    string targetBundleName = bundleName;
    string callerBundleName = "";
    int32_t ret = GetTargetBundleName(targetBundleName, callerBundleName);
    if (ret != E_OK) {
        LOGE("get bundle name failed: %{public}d", ret);
        return ret;
    }

    auto callback = iface_cast<ICloudSyncCallback>(remoteObject);
    auto callerUserId = DfsuAccessTokenHelper::GetUserId();
    dataSyncManager_->RegisterCloudSyncCallback(targetBundleName, callerBundleName, callerUserId, callback);
    return E_OK;
}

int32_t CloudSyncService::StartSyncInner(bool forceFlag, const string &bundleName)
{
    string targetBundleName = bundleName;
    string callerBundleName = "";
    int32_t ret = GetTargetBundleName(targetBundleName, callerBundleName);
    if (ret != E_OK) {
        LOGE("get bundle name failed: %{public}d", ret);
        return ret;
    }
    auto callerUserId = DfsuAccessTokenHelper::GetUserId();
    return dataSyncManager_->TriggerStartSync(targetBundleName, callerUserId, forceFlag,
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

int32_t CloudSyncService::StopSyncInner(const string &bundleName, bool forceFlag)
{
    string targetBundleName = bundleName;
    string callerBundleName = "";
    int32_t ret = GetTargetBundleName(targetBundleName, callerBundleName);
    if (ret != E_OK) {
        LOGE("get bundle name failed: %{public}d", ret);
        return ret;
    }
    auto callerUserId = DfsuAccessTokenHelper::GetUserId();
    return dataSyncManager_->TriggerStopSync(targetBundleName, callerUserId, forceFlag, SyncTriggerType::APP_TRIGGER);
}

int32_t CloudSyncService::ResetCursor(const string &bundleName)
{
    string targetBundleName = bundleName;
    string callerBundleName = "";
    int32_t ret = GetTargetBundleName(targetBundleName, callerBundleName);
    if (ret != E_OK) {
        LOGE("get bundle name failed: %{public}d", ret);
        return ret;
    }
    auto callerUserId = DfsuAccessTokenHelper::GetUserId();
    return dataSyncManager_->ResetCursor(targetBundleName, callerUserId);
}

int32_t CloudSyncService::GetSyncTimeInner(int64_t &syncTime, const string &bundleName)
{
    string targetBundleName = bundleName;
    string callerBundleName = "";
    int32_t ret = GetTargetBundleName(targetBundleName, callerBundleName);
    if (ret != E_OK) {
        LOGE("get bundle name failed: %{public}d", ret);
        return ret;
    }
    auto callerUserId = DfsuAccessTokenHelper::GetUserId();
    return DataSyncerRdbStore::GetInstance().GetLastSyncTime(callerUserId, targetBundleName, syncTime);
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
    LOGI("ChangeAppSwitch, bundleName: %{private}s, status: %{public}d, callerUserId: %{public}d",
         bundleName.c_str(), status, callerUserId);

    /* update app switch status */
    int32_t ret = CloudStatus::ChangeAppSwitch(bundleName, callerUserId, status);
    if (ret != E_OK) {
        LOGE("CloudStatus::ChangeAppSwitch failed, ret: %{public}d", ret);
        return ret;
    }
    if (status) {
        ret = dataSyncManager_->TriggerStartSync(bundleName, callerUserId, false, SyncTriggerType::CLOUD_TRIGGER);
    } else {
        system::SetParameter(CLOUDSYNC_STATUS_KEY, CLOUDSYNC_STATUS_SWITCHOFF);
        ret = dataSyncManager_->TriggerStopSync(bundleName, callerUserId, false, SyncTriggerType::CLOUD_TRIGGER);
    }
    if (ret != E_OK) {
        LOGE("dataSyncManager Trigger failed, status: %{public}d", status);
        return ret;
    }

    return dataSyncManager_->ChangeAppSwitch(bundleName, callerUserId, status);
}

int32_t CloudSyncService::NotifyDataChange(const std::string &accoutId, const std::string &bundleName)
{
    auto callerUserId = DfsuAccessTokenHelper::GetUserId();
    return dataSyncManager_->TriggerStartSync(bundleName, callerUserId, false, SyncTriggerType::CLOUD_TRIGGER);
}

int32_t CloudSyncService::NotifyEventChange(int32_t userId, const std::string &eventId, const std::string &extraData)
{
    auto instance = CloudFile::CloudFileKit::GetInstance();
    if (instance == nullptr) {
        LOGE("get cloud file helper instance failed");
        return E_NULLPTR;
    }

    string appBundleName;
    string prepareTraceId;
    auto ret = instance->ResolveNotificationEvent(userId, extraData, appBundleName, prepareTraceId);
    if (ret != E_OK) {
        LOGE("ResolveNotificationEvent failed, ret:%{public}d", ret);
        return E_CLOUD_SDK;
    }

    return dataSyncManager_->TriggerStartSync(appBundleName, userId, false,
        SyncTriggerType::CLOUD_TRIGGER, prepareTraceId);
}

int32_t CloudSyncService::DisableCloud(const std::string &accoutId)
{
    LOGI("DisableCloud start");
    auto callerUserId = DfsuAccessTokenHelper::GetUserId();
    system::SetParameter(CLOUDSYNC_STATUS_KEY, CLOUDSYNC_STATUS_LOGOUT);
    return dataSyncManager_->DisableCloud(callerUserId);
}

int32_t CloudSyncService::EnableCloud(const std::string &accoutId, const SwitchDataObj &switchData)
{
    return E_OK;
}

int32_t CloudSyncService::Clean(const std::string &accountId, const CleanOptions &cleanOptions)
{
    for (auto &iter : cleanOptions.appActionsData) {
        LOGD("Clean key is: %s, value is: %d", iter.first.c_str(), iter.second);
    }

    MetaFileMgr::GetInstance().ClearAll();
    MetaFileMgr::GetInstance().CloudDiskClearAll();
    auto callerUserId = DfsuAccessTokenHelper::GetUserId();
    LOGI("Clean callerUserId is: %{public}d", callerUserId);
    for (auto iter = cleanOptions.appActionsData.begin(); iter != cleanOptions.appActionsData.end(); ++iter) {
        dataSyncManager_->CleanCloudFile(callerUserId, iter->first, iter->second);
    }

    return E_OK;
}
int32_t CloudSyncService::StartFileCache(const std::vector<std::string> &uriVec,
                                         int64_t &downloadId, std::bitset<FIELD_KEY_MAX_SIZE> fieldkey)
{
    BundleNameUserInfo bundleNameUserInfo;
    int ret = GetBundleNameUserInfo(bundleNameUserInfo);
    if (ret != E_OK) {
        LOGE("GetBundleNameUserInfo failed.");
        return ret;
    }
    LOGI("start StartFileCache");
    return dataSyncManager_->StartDownloadFile(bundleNameUserInfo, uriVec, downloadId, fieldkey);
}

int32_t CloudSyncService::StartDownloadFile(const std::string &path)
{
    BundleNameUserInfo bundleNameUserInfo;
    int ret = GetBundleNameUserInfo(bundleNameUserInfo);
    if (ret != E_OK) {
        return ret;
    }
    std::vector<std::string> pathVec;
    pathVec.push_back(path);
    int64_t downloadId = 0;
    LOGI("start StartDownloadFile");
    return dataSyncManager_->StartDownloadFile(bundleNameUserInfo, pathVec, downloadId);
}

int32_t CloudSyncService::StopDownloadFile(const std::string &path, bool needClean)
{
    BundleNameUserInfo bundleNameUserInfo;
    int ret = GetBundleNameUserInfo(bundleNameUserInfo);
    if (ret != E_OK) {
        return ret;
    }
    LOGI("start StopDownloadFile");
    return dataSyncManager_->StopDownloadFile(bundleNameUserInfo, path, needClean);
}

int32_t CloudSyncService::StopFileCache(const int64_t &downloadId,  bool needClean)
{
    BundleNameUserInfo bundleNameUserInfo;
    int ret = GetBundleNameUserInfo(bundleNameUserInfo);
    if (ret != E_OK) {
        return ret;
    }
    LOGI("start StopFileCache");
    return dataSyncManager_->StopFileCache(bundleNameUserInfo, downloadId, needClean);
}

int32_t CloudSyncService::RegisterDownloadFileCallback(const sptr<IRemoteObject> &downloadCallback)
{
    BundleNameUserInfo bundleNameUserInfo;
    int ret = GetBundleNameUserInfo(bundleNameUserInfo);
    if (ret != E_OK) {
        return ret;
    }
    auto downloadCb = iface_cast<ICloudDownloadCallback>(downloadCallback);
    LOGI("start RegisterDownloadFileCallback");
    return dataSyncManager_->RegisterDownloadFileCallback(bundleNameUserInfo, downloadCb);
}

int32_t CloudSyncService::UnregisterDownloadFileCallback()
{
    BundleNameUserInfo bundleNameUserInfo;
    int ret = GetBundleNameUserInfo(bundleNameUserInfo);
    if (ret != E_OK) {
        return ret;
    }
    LOGI("start UnregisterDownloadFileCallback");
    return dataSyncManager_->UnregisterDownloadFileCallback(bundleNameUserInfo);
}

int32_t CloudSyncService::UploadAsset(const int32_t userId, const std::string &request, std::string &result)
{
    auto instance = CloudFile::CloudFileKit::GetInstance();
    if (instance == nullptr) {
        LOGE("get cloud file helper instance failed");
        return E_NULLPTR;
    }

    string bundleName("distributeddata");
    TaskStateManager::GetInstance().StartTask(bundleName, TaskType::UPLOAD_ASSET_TASK);
    auto ret = instance->OnUploadAsset(userId, request, result);
    TaskStateManager::GetInstance().CompleteTask(bundleName, TaskType::UPLOAD_ASSET_TASK);
    return ret;
}

int32_t CloudSyncService::DownloadFile(const int32_t userId, const std::string &bundleName, AssetInfoObj &assetInfoObj)
{
    auto instance = CloudFile::CloudFileKit::GetInstance();
    if (instance == nullptr) {
        LOGE("get cloud file helper instance failed");
        return E_NULLPTR;
    }

    auto assetsDownloader = instance->GetCloudAssetsDownloader(userId, bundleName);
    if (assetsDownloader == nullptr) {
        LOGE("get asset downloader failed");
        return E_NULLPTR;
    }

    Asset asset;
    asset.assetName = assetInfoObj.assetName;

    asset.uri = GetHmdfsPath(assetInfoObj.uri, userId);
    if (asset.uri.empty()) {
        LOGE("fail to get download path from %{public}s", GetAnonyString(assetInfoObj.uri).c_str());
        return E_INVAL_ARG;
    }

    // Not to pass the assetinfo.fieldkey
    DownloadAssetInfo assetsToDownload{assetInfoObj.recordType, assetInfoObj.recordId, {}, asset, {}};
    TaskStateManager::GetInstance().StartTask(bundleName, TaskType::DOWNLOAD_ASSET_TASK);
    auto ret = assetsDownloader->DownloadAssets(assetsToDownload);
    TaskStateManager::GetInstance().CompleteTask(bundleName, TaskType::DOWNLOAD_ASSET_TASK);
    return ret;
}

int32_t CloudSyncService::DownloadFiles(const int32_t userId, const std::string &bundleName,
    const std::vector<AssetInfoObj> &assetInfoObj, std::vector<bool> &assetResultMap)
{
    auto instance = CloudFile::CloudFileKit::GetInstance();
    if (instance == nullptr) {
        LOGE("get cloud file helper instance failed");
        return E_NULLPTR;
    }

    auto assetsDownloader = instance->GetCloudAssetsDownloader(userId, bundleName);
    if (assetsDownloader == nullptr) {
        LOGE("get asset downloader failed");
        return E_NULLPTR;
    }

    std::vector<DownloadAssetInfo> assetsToDownload;
    for (const auto &obj: assetInfoObj) {
        Asset asset;
        asset.assetName = obj.assetName;
        asset.uri = GetHmdfsPath(obj.uri, userId);
        if (asset.uri.empty()) {
            LOGE("fail to get download path from %{private}s", GetAnonyString(obj.uri).c_str());
            return E_INVAL_ARG;
        }
        DownloadAssetInfo assetToDownload{obj.recordType, obj.recordId, {}, asset, {}};
        assetsToDownload.emplace_back(assetToDownload);
    }

    TaskStateManager::GetInstance().StartTask(bundleName, TaskType::DOWNLOAD_ASSET_TASK);
    auto ret = assetsDownloader->DownloadAssets(assetsToDownload, assetResultMap);
    TaskStateManager::GetInstance().CompleteTask(bundleName, TaskType::DOWNLOAD_ASSET_TASK);
    return ret;
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

    LOGD("delete assert, path %{public}s", GetAnonyString(physicalPath).c_str());

    ret = unlink(physicalPath.c_str());
    if (ret != 0) {
        LOGE("fail to delete asset, errno %{public}d", errno);
        return E_DELETE_FAILED;
    }
    return E_OK;
}
} // namespace OHOS::FileManagement::CloudSync
