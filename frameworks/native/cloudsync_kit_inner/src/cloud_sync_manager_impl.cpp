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

#include <charconv>
#include <filesystem>
#include <fstream>

#include "account_utils.h"
#include "cloud_download_callback_client.h"
#include "cloud_sync_callback_client.h"
#include "cloud_sync_manager_impl.h"
#include "cloud_upload_callback_client.h"
#include "cloud_upload_callback_client_manager.h"
#include "downgrade_download_callback_client.h"
#include "dfs_error.h"
#include "iservice_registry.h"
#include "service_proxy.h"
#include "system_ability_definition.h"
#include "utils_directory.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
using namespace std;
constexpr int32_t MIN_USER_ID = 100;
constexpr int32_t MAX_FILE_CACHE_NUM = 400;
constexpr int32_t MAX_DENTRY_FILE_SIZE = 500;
constexpr int32_t MAX_PROGRESS_QUERY_NUM = 100;
constexpr int32_t BASE_USER_RANGE = 200000;
const string CLOUDSYNC_MEDIA_CALLBACK_ID = "cloudSyncMediaCallbackId";
static const std::string GLOBAL_CONFIG_FILE_PATH = "globalConfig.xml";
static const std::string SUM_OCCUPY_FLAG = "SumOccupyFlag";
constexpr int32_t MAX_OCCUPY_LINE_LIMIT = 10;
CloudSyncManagerImpl &CloudSyncManagerImpl::GetInstance()
{
    static CloudSyncManagerImpl instance;
    return instance;
}

int32_t CloudSyncManagerImpl::RegisterCallback(const CallbackInfo &callbackInfo)
{
    LOGI("Start RegisterCallback");
    if ((callbackInfo.callbackId.empty()) || (callbackInfo.callback == nullptr)) {
        LOGE("callbackId or callback is null");
        return E_INVAL_ARG;
    }

    auto CloudSyncServiceProxy = ServiceProxy::GetInstance("RegisterCallback");
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    auto ret = CloudSyncServiceProxy->RegisterCallbackInner(
        sptr(new (std::nothrow) CloudSyncCallbackClient(callbackInfo.callback)),
        callbackInfo.callbackId, callbackInfo.bundleName);
    if (ret == E_OK) {
        CloudSyncCallbackClientManager::GetInstance().AddCallback(callbackInfo);
        SubscribeListener();
    }

    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    LOGI("RegisterCallback ret %{public}d", ret);
    return ret;
}

int32_t CloudSyncManagerImpl::RegisterFileSyncCallback(const CallbackInfo &callbackInfo)
{
    LOGI("Start RegisterFileSyncCallback");
    if ((callbackInfo.callbackId.empty()) || (callbackInfo.callback == nullptr)) {
        LOGE("callbackId or callback is null");
        return E_INVAL_ARG;
    }
    auto CloudSyncServiceProxy = ServiceProxy::GetInstance(
        "RegisterFileSyncCallback");
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    auto ret = CloudSyncServiceProxy->RegisterFileSyncCallbackInner(
        sptr(new (std::nothrow) CloudSyncCallbackClient(callbackInfo.callback)),
        callbackInfo.callbackId, callbackInfo.bundleName);
    if (ret == E_OK) {
        CloudSyncCallbackClientManager::GetInstance().AddCallback(callbackInfo);
        SubscribeListener();
    }

    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    LOGI("RegisterFileSyncCallback ret %{public}d", ret);
    return ret;
}

int32_t CloudSyncManagerImpl::UnRegisterCallback(const CallbackInfo &callbackInfo)
{
    LOGI("Start UnRegisterCallback");
    if (callbackInfo.callbackId.empty()) {
        LOGE("callbackId is null");
        return E_INVAL_ARG;
    }

    auto CloudSyncServiceProxy = ServiceProxy::GetInstance("UnRegisterCallback");
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }

    auto ret = CloudSyncServiceProxy->UnRegisterCallbackInner(callbackInfo.callbackId, callbackInfo.bundleName);
    if (ret == E_OK) {
        CloudSyncCallbackClientManager::GetInstance().RemoveCallback(callbackInfo);
        SubscribeListener();
    }
    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    LOGI("UnRegisterCallback ret %{public}d", ret);
    return ret;
}

int32_t CloudSyncManagerImpl::UnRegisterFileSyncCallback(const CallbackInfo &callbackInfo)
{
    LOGI("Start UnRegisterFileSyncCallback");
    if (callbackInfo.callbackId.empty()) {
        LOGE("callbackId is null");
        return E_INVAL_ARG;
    }

    auto CloudSyncServiceProxy = ServiceProxy::GetInstance(
        "UnRegisterFileSyncCallback");
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }

    auto ret = CloudSyncServiceProxy->UnRegisterFileSyncCallbackInner(callbackInfo.callbackId, callbackInfo.bundleName);
    if (ret == E_OK) {
        CloudSyncCallbackClientManager::GetInstance().RemoveCallback(callbackInfo);
        SubscribeListener();
    }
    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    LOGI("UnRegisterFileSyncCallback ret %{public}d", ret);
    return ret;
}

int32_t CloudSyncManagerImpl::StartSync(const std::string &bundleName)
{
    if (!AccountUtils::IsAccountAvailable()) {
        return E_INVAL_ARG;
    }
    auto CloudSyncServiceProxy = ServiceProxy::GetInstance("StartSync001");
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    return CloudSyncServiceProxy->StartSyncInner(true, bundleName);
}

int32_t CloudSyncManagerImpl::StartFileSync(const std::string &bundleName)
{
    if (!AccountUtils::IsAccountAvailable()) {
        return E_INVAL_ARG;
    }
    auto CloudSyncServiceProxy = ServiceProxy::GetInstance("StartFileSync");
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    return CloudSyncServiceProxy->StartFileSyncInner(true, bundleName);
}

int32_t CloudSyncManagerImpl::GetSyncTime(int64_t &syncTime, const std::string &bundleName)
{
    LOGI("GetSyncTime Start");
    auto CloudSyncServiceProxy = ServiceProxy::GetInstance("GetSyncTime");
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    return CloudSyncServiceProxy->GetSyncTimeInner(syncTime, bundleName);
}

int32_t CloudSyncManagerImpl::OptimizeStorage(const OptimizeSpaceOptions &optimizeOptions,
    const std::shared_ptr<CloudOptimizeCallback> optimizeCallback)
{
    LOGI("OptimizeStorage Start");
    auto CloudSyncServiceProxy = ServiceProxy::GetInstance("OptimizeStorage");
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    SetDeathRecipient(CloudSyncServiceProxy->AsObject());

    bool isCallbackValid = false;
    sptr<CloudOptimizeCallbackClient> opCallback = sptr(new (std::nothrow) CloudOptimizeCallbackClient(nullptr));
    if (optimizeCallback != nullptr) {
        opCallback = sptr(new (std::nothrow) CloudOptimizeCallbackClient(optimizeCallback));
        isCallbackValid = true;
        if (opCallback == nullptr) {
            isCallbackValid = false;
            LOGE("OptimizeStorage callback failed");
        }
    }

    return CloudSyncServiceProxy->OptimizeStorage(optimizeOptions, isCallbackValid, opCallback);
}

int32_t CloudSyncManagerImpl::StopOptimizeStorage()
{
    LOGI("StopOptimizeStorage Start");
    auto CloudSyncServiceProxy = ServiceProxy::GetInstance("StopOptimizeStorage");
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    SetDeathRecipient(CloudSyncServiceProxy->AsObject());

    return CloudSyncServiceProxy->StopOptimizeStorage();
}

int32_t CloudSyncManagerImpl::StartSync(bool forceFlag, const std::shared_ptr<CloudSyncCallback> callback)
{
    if (!callback) {
        LOGE("callback is null");
        return E_INVAL_ARG;
    }

    if (!AccountUtils::IsAccountAvailable()) {
        return E_INVAL_ARG;
    }

    {
        unique_lock<mutex> lock(startSyncPendingMtx_);
        if (startSyncPending_) {
            return E_OK;
        }
        startSyncPending_ = true;
    }
    auto CloudSyncServiceProxy = ServiceProxy::GetInstance("StartSync002");
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        SetStartSyncPending();
        return E_SA_LOAD_FAILED;
    }

    if (!isFirstCall_.test()) {
        LOGI("Register callback");
        auto ret = CloudSyncServiceProxy->RegisterCallbackInner(
            sptr(new (std::nothrow) CloudSyncCallbackClient(callback)), CLOUDSYNC_MEDIA_CALLBACK_ID, "");
        if (ret) {
            LOGE("Register callback failed");
            isFirstCall_.clear();
            SetStartSyncPending();
            return ret;
        }
        CallbackInfo callbackInfo = {CLOUDSYNC_MEDIA_CALLBACK_ID, callback, ""};
        CloudSyncCallbackClientManager::GetInstance().AddCallback(callbackInfo);
        SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    }

    auto ret = CloudSyncServiceProxy->StartSyncInner(forceFlag, "");
    SetStartSyncPending();
    return ret;
}

int32_t CloudSyncManagerImpl::TriggerSync(const std::string &bundleName, const int32_t &userId)
{
    if (!AccountUtils::IsAccountAvailable()) {
        return E_INVAL_ARG;
    }

    if (bundleName.empty() || userId < MIN_USER_ID) {
        LOGE("Trigger Sync parameter is invalid");
        return E_INVAL_ARG;
    }
    auto CloudSyncServiceProxy = ServiceProxy::GetInstance("TriggerSync001_" + bundleName);
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    return CloudSyncServiceProxy->TriggerSyncInner(bundleName, userId);
}

int32_t CloudSyncManagerImpl::StopSync(const std::string &bundleName, bool forceFlag)
{
    auto CloudSyncServiceProxy = ServiceProxy::GetInstance("StopSync");
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    return CloudSyncServiceProxy->StopSyncInner(bundleName, forceFlag);
}

int32_t CloudSyncManagerImpl::StopFileSync(const std::string &bundleName, bool forceFlag)
{
    auto CloudSyncServiceProxy = ServiceProxy::GetInstance("StopFileSync");
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    return CloudSyncServiceProxy->StopFileSyncInner(bundleName, forceFlag);
}

int32_t CloudSyncManagerImpl::ResetCursor(const std::string &bundleName)
{
    auto CloudSyncServiceProxy = ServiceProxy::GetInstance("ResetCursor001");
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    return CloudSyncServiceProxy->ResetCursor(false, bundleName);
}

int32_t CloudSyncManagerImpl::ResetCursor(bool flag, const std::string &bundleName)
{
    auto CloudSyncServiceProxy = ServiceProxy::GetInstance("ResetCursor002");
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    return CloudSyncServiceProxy->ResetCursor(flag, bundleName);
}

int32_t CloudSyncManagerImpl::ChangeAppSwitch(const std::string &accoutId, const std::string &bundleName, bool status)
{
    auto CloudSyncServiceProxy = ServiceProxy::GetInstance("ChangeAppSwitch");
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }

    SetDeathRecipient(CloudSyncServiceProxy->AsObject());

    int32_t ret = CloudSyncServiceProxy->ChangeAppSwitch(accoutId, bundleName, status);
    LOGI("ChangeAppSwitch ret %{public}d", ret);
    return ret;
}

int32_t CloudSyncManagerImpl::NotifyDataChange(const std::string &accoutId, const std::string &bundleName)
{
    auto CloudSyncServiceProxy = ServiceProxy::GetInstance("NotifyDataChange");
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }

    SetDeathRecipient(CloudSyncServiceProxy->AsObject());

    int32_t ret = CloudSyncServiceProxy->NotifyDataChange(accoutId, bundleName);
    LOGI("NotifyDataChange ret %{public}d", ret);
    return ret;
}

int32_t CloudSyncManagerImpl::NotifyEventChange(
    int32_t userId, const std::string &eventId, const std::string &extraData)
{
    auto CloudSyncServiceProxy = ServiceProxy::GetInstance("NotifyEventChange");
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }

    SetDeathRecipient(CloudSyncServiceProxy->AsObject());

    int32_t ret = CloudSyncServiceProxy->NotifyEventChange(userId, eventId, extraData);
    LOGI("NotifyDataChange ret %{public}d", ret);
    return ret;
}

int32_t CloudSyncManagerImpl::StartDownloadFile(const std::string &uri,
                                                const std::shared_ptr<CloudDownloadCallback> downloadCallback,
                                                int64_t &downloadId)
{
    LOGI("StartDownloadFile start");
    auto CloudSyncServiceProxy = ServiceProxy::GetInstance("StartDownloadFile");
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    if (downloadCallback == nullptr) {
        LOGE("The callback is null");
        return E_INVAL_ARG;
    }
    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    auto dlCallback = sptr<CloudDownloadCallbackClient>::MakeSptr(downloadCallback);
    int32_t ret = CloudSyncServiceProxy->StartDownloadFile(uri, dlCallback, downloadId);
    LOGI("StartDownloadFile ret %{public}d", ret);
    return ret;
}

int32_t CloudSyncManagerImpl::BatchCleanFile(const std::vector<CleanFileInfo> &fileInfo,
    std::vector<std::string> &failCloudId)
{
    if (fileInfo.size() > CLEAN_FILE_MAX_SIZE) {
        LOGE("BatchCleanFile size over max limit");
        return E_INVAL_ARG;
    }
    auto CloudSyncServiceProxy = ServiceProxy::GetInstance("BatchCleanFile");
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }

    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    std::vector<CleanFileInfoObj> fileInfoObj;
    for (const auto &info : fileInfo) {
        CleanFileInfoObj obj(info);
        fileInfoObj.emplace_back(obj);
    }

    return CloudSyncServiceProxy->BatchCleanFile(fileInfoObj, failCloudId);
}

int32_t CloudSyncManagerImpl::StartFileCache(const std::vector<std::string> &uriVec,
                                             int64_t &downloadId,
                                             int32_t fieldkey,
                                             const std::shared_ptr<CloudDownloadCallback> downloadCallback,
                                             int32_t timeout)
{
    LOGI("StartFileCache start, uriVec size: %{public}zu, fieldKey: %{public}d", uriVec.size(), fieldkey);
    if (uriVec.empty()) {
        LOGE("The uri list is empty");
        return E_INVAL_ARG;
    }
    if (uriVec.size() > MAX_FILE_CACHE_NUM) {
        LOGE("The size of uri list exceeded the maximum limit, size: %{public}zu", uriVec.size());
        return E_EXCEED_MAX_SIZE;
    }
    auto CloudSyncServiceProxy = ServiceProxy::GetInstance("StartFileCache");
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    SetDeathRecipient(CloudSyncServiceProxy->AsObject());

    if (downloadCallback == nullptr) {
        LOGW("The callback is null, download progress may not be received");
    }
    auto dlCallback = sptr<CloudDownloadCallbackClient>::MakeSptr(downloadCallback);
    int32_t ret = CloudSyncServiceProxy->StartFileCache(uriVec, downloadId, fieldkey, dlCallback, timeout);
    LOGI("StartFileCache end, ret %{public}d", ret);
    return ret;
}

int32_t CloudSyncManagerImpl::GetDownloadList(const std::vector<std::string> &uriVec,
                                              std::vector<CloudSync::DownloadProgressObj> &downloadList)
{
    if (uriVec.empty()) {
        LOGE("The uri list is empty");
        return E_INVAL_ARG;
    }
    if (uriVec.size() > MAX_PROGRESS_QUERY_NUM) {
        LOGE("The size of uri list exceeded the maximum limit, size: %{public}zu", uriVec.size());
        return E_EXCEED_MAX_SIZE;
    }
    auto CloudSyncServiceProxy = ServiceProxy::GetInstance("GetDownloadList");
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    int32_t ret = CloudSyncServiceProxy->GetDownloadList(uriVec, downloadList);
    return ret;
}

int32_t CloudSyncManagerImpl::GetUploadList(const std::vector<std::string> &uriVec,
                                            std::vector<CloudSync::UploadProgressObj> &uploadList)
{
    if (uriVec.empty()) {
        LOGE("The uri list is empty");
        return E_INVAL_ARG;
    }
    if (uriVec.size() > MAX_PROGRESS_QUERY_NUM) {
        LOGE("The size of uri list exceeded the maximum limit, size: %{public}zu", uriVec.size());
        return E_EXCEED_MAX_SIZE;
    }
    auto CloudSyncServiceProxy = ServiceProxy::GetInstance("GetUploadList");
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    int32_t ret = CloudSyncServiceProxy->GetUploadList(uriVec, uploadList);
    return ret;
}

int32_t CloudSyncManagerImpl::RegisterUploadCallback(const UploadCallbackInfo &uploadCallbackInfo)
{
    LOGI("Start RegisterUploadCallback");
    if ((uploadCallbackInfo.callbackId.empty()) || (uploadCallbackInfo.callback == nullptr)) {
        LOGE("callbackId or callback is null");
        return E_INVAL_ARG;
    }

    auto CloudSyncServiceProxy = ServiceProxy::GetInstance(
        "RegisterUploadCallback");
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }

    auto ret = CloudSyncServiceProxy->RegisterUploadCallbackInner(
        sptr(new (std::nothrow) CloudUploadCallbackClient(uploadCallbackInfo.callback)),
        uploadCallbackInfo.callbackId, uploadCallbackInfo.bundleName);
    if (ret != E_OK) {
        LOGE("RegisterUploadCallback failed: %{public}d", ret);
        return ret;
    }
    if (ret == E_OK) {
        CloudUploadCallbackClientManager::GetInstance().AddCallback(uploadCallbackInfo);
        SubscribeListener();
    }

    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    LOGI("RegisterUploadCallback ret %{public}d", ret);
    return ret;
}

int32_t CloudSyncManagerImpl::UnRegisterUploadCallback(const UploadCallbackInfo &uploadCallbackInfo)
{
    LOGI("Start UnRegisterUploadCallback");
    if (uploadCallbackInfo.callbackId.empty()) {
        LOGE("callbackId is null");
        return E_INVAL_ARG;
    }

    auto CloudSyncServiceProxy = ServiceProxy::GetInstance(
        "UnRegisterUploadCallback");
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }

    auto ret = CloudSyncServiceProxy->UnRegisterUploadCallbackInner(uploadCallbackInfo.callbackId,
        uploadCallbackInfo.bundleName);
    if (ret == E_OK) {
        CloudUploadCallbackClientManager::GetInstance().RemoveCallback(uploadCallbackInfo);
        SubscribeListener();
    }
    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    LOGI("UnRegisterUploadCallback ret %{public}d", ret);
    return ret;
}

int32_t CloudSyncManagerImpl::PauseUpload(const std::string &uri)
{
    auto CloudSyncServiceProxy = ServiceProxy::GetInstance("PauseUpload");
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    return  CloudSyncServiceProxy->PauseUpload(uri);
}

int32_t CloudSyncManagerImpl::ResumeUpload(const std::string &uri)
{
    auto CloudSyncServiceProxy = ServiceProxy::GetInstance("ResumeUpload");
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    return CloudSyncServiceProxy->ResumeUpload(uri);
}

int32_t CloudSyncManagerImpl::StopDownloadFile(int64_t downloadId, bool needClean)
{
    LOGI("StopDownloadFile start");
    auto CloudSyncServiceProxy = ServiceProxy::GetInstance("StopDownloadFile");
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    int32_t ret = CloudSyncServiceProxy->StopDownloadFile(downloadId, needClean);
    LOGI("StopDownloadFile end, ret %{public}d", ret);
    return ret;
}

int32_t CloudSyncManagerImpl::StopFileCache(int64_t downloadId, bool needClean, int32_t timeout)
{
    LOGI("StopFileCache start");
    auto CloudSyncServiceProxy = ServiceProxy::GetInstance("StopFileCache");
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    int32_t ret = CloudSyncServiceProxy->StopFileCache(downloadId, needClean, timeout);
    LOGI("StopFileCache end, ret %{public}d", ret);
    return ret;
}

int32_t CloudSyncManagerImpl::StartDowngrade(const std::string &bundleName,
                                             const std::shared_ptr<DowngradeDlCallback> downloadCallback)
{
    LOGI("StartDowngrade start");
    auto CloudSyncServiceProxy = ServiceProxy::GetInstance("StartDowngrade");
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    if (bundleName.empty() || downloadCallback == nullptr) {
        LOGE("Invalid argument");
        return E_INVAL_ARG;
    }
    sptr<DowngradeDownloadCallbackClient> dlCallback =
        sptr(new (std::nothrow) DowngradeDownloadCallbackClient(downloadCallback));
    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    int32_t ret = CloudSyncServiceProxy->StartDowngrade(bundleName, dlCallback);
    LOGI("StartDowngrade ret %{public}d", ret);
    return ret;
}

int32_t CloudSyncManagerImpl::StopDowngrade(const std::string &bundleName)
{
    LOGI("StopDowngrade start");
    auto CloudSyncServiceProxy = ServiceProxy::GetInstance("StopDowngrade");
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    if (bundleName.empty()) {
        LOGE("Invalid argument");
        return E_INVAL_ARG;
    }

    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    int32_t ret = CloudSyncServiceProxy->StopDowngrade(bundleName);
    LOGI("StopDowngrade ret %{public}d", ret);
    return ret;
}

static int32_t CheckTransferTargetUri(const std::string &targetUri)
{
    if (targetUri.empty()) {
        LOGE("Transfer target uri is empty.");
        return E_INVAL_PARAM;
    }

    const std::string uriPrefix = "file://docs/storage/Users/currentUser/";
    if (targetUri.find(uriPrefix) != 0) {
        LOGE("Transfer target uri %{public}s is invalid", targetUri.c_str());
        return E_INVAL_PARAM;
    }

    const std::string pathPrefix = "file:/docs/storage/Users/currentUser/";
    std::filesystem::path targetPath(targetUri);
    std::filesystem::path normalizedPath = targetPath.lexically_normal();
    if (normalizedPath.string().find(pathPrefix) != 0) {
        LOGE("Transfer target path %{public}s is invalid", targetUri.c_str());
        return E_INVAL_PARAM;
    }

    return E_OK;
}

int32_t CloudSyncManagerImpl::StartTransfer(const std::string &bundleName, const std::string &targetUri,
                                            const std::shared_ptr<DowngradeDlCallback> downloadCallback)
{
    LOGI("StartTransfer start");
    auto CloudSyncServiceProxy = ServiceProxy::GetInstance("StartTransfer");
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }

    if (bundleName.empty() || downloadCallback == nullptr) {
        LOGE("Invalid argument");
        return E_INVAL_PARAM;
    }

    int32_t ret = CheckTransferTargetUri(targetUri);
    if (ret != E_OK) {
        return ret;
    }

    sptr<DowngradeDownloadCallbackClient> dlCallback =
        sptr(new (std::nothrow) DowngradeDownloadCallbackClient(downloadCallback));
    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    ret = CloudSyncServiceProxy->StartTransfer(bundleName, targetUri, dlCallback);
    LOGI("StartTransfer ret %{public}d", ret);
    return ret;
}

int32_t CloudSyncManagerImpl::GetCloudFileInfo(const std::string &bundleName, CloudFileInfo &cloudFileInfo)
{
    LOGI("GetCloudFileInfo start");
    auto CloudSyncServiceProxy = ServiceProxy::GetInstance("GetCloudFileInfo");
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    if (bundleName.empty()) {
        LOGE("Invalid argument");
        return E_INVAL_ARG;
    }

    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    int32_t ret = CloudSyncServiceProxy->GetCloudFileInfo(bundleName, cloudFileInfo);
    LOGI("GetCloudFileInfo ret %{public}d", ret);
    return ret;
}

int32_t CloudSyncManagerImpl::GetHistoryVersionList(const std::string &uri, const int32_t versionNumLimit,
    std::vector<CloudSync::HistoryVersion> &historyVersionList)
{
    LOGI("GetHistoryVersionList start, versionNumLimit is %{public}d", versionNumLimit);
    auto CloudSyncServiceProxy = ServiceProxy::GetInstance("GetHistoryVersionList");
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    if (uri.empty()) {
        LOGE("Invalid argument");
        return E_ILLEGAL_URI;
    }

    if (versionNumLimit <= 0) {
        LOGE("Invalid argument");
        return E_INVAL_ARG;
    }

    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    int32_t ret = CloudSyncServiceProxy->GetHistoryVersionList(uri, versionNumLimit, historyVersionList);
    LOGI("GetHistoryVersionList ret %{public}d", ret);
    return ret;
}

int32_t CloudSyncManagerImpl::DownloadHistoryVersion(const std::string &uri, int64_t &downloadId,
    const uint64_t versionId, const std::shared_ptr<CloudDownloadCallback> downloadCallback, std::string &versionUri)
{
    LOGI("DownloadHistoryVersion start, versionId is %{public}lld, Callback is null: %{public}d",
         static_cast<long long>(versionId), (downloadCallback == nullptr));
    auto CloudSyncServiceProxy = ServiceProxy::GetInstance("DownloadHistoryVersion");
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    if (uri.empty()) {
        LOGE("Invalid argument");
        return E_ILLEGAL_URI;
    }

    if (downloadCallback == nullptr) {
        LOGE("Invalid argument");
        return E_INVAL_ARG;
    }

    sptr<CloudDownloadCallbackClient> dlCallback =
        sptr(new (std::nothrow) CloudDownloadCallbackClient(downloadCallback));
    if (dlCallback == nullptr) {
        LOGE("DownloadHistoryVersion register download callback failed");
        return E_SERVICE_INNER_ERROR;
    }

    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    int32_t ret = CloudSyncServiceProxy->DownloadHistoryVersion(uri, downloadId, versionId, dlCallback, versionUri);
    LOGI("DownloadHistoryVersion ret %{public}d, downloadId %{public}lld",
         ret, static_cast<long long>(downloadId));
    return ret;
}

int32_t CloudSyncManagerImpl::ReplaceFileWithHistoryVersion(const std::string &uri, const std::string &versionUri)
{
    LOGI("ReplaceFileWithHistoryVersion start");
    auto CloudSyncServiceProxy = ServiceProxy::GetInstance("ReplaceFileWithHistoryVersion");
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    if (uri.empty() || versionUri.empty()) {
        LOGE("Invalid argument");
        return E_ILLEGAL_URI;
    }

    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    int32_t ret = CloudSyncServiceProxy->ReplaceFileWithHistoryVersion(uri, versionUri);
    return ret;
}

int32_t CloudSyncManagerImpl::IsFileConflict(const std::string &uri, bool &isConflict)
{
    LOGI("IsFileConflict start");
    auto CloudSyncServiceProxy = ServiceProxy::GetInstance("IsFileConflict");
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    if (uri.empty()) {
        LOGE("Invalid argument");
        return E_ILLEGAL_URI;
    }

    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    int32_t ret = CloudSyncServiceProxy->IsFileConflict(uri, isConflict);
    return ret;
}

int32_t CloudSyncManagerImpl::ClearFileConflict(const std::string &uri)
{
    auto CloudSyncServiceProxy = ServiceProxy::GetInstance("ClearFileConflict");
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    if (uri.empty()) {
        LOGE("Invalid argument");
        return E_ILLEGAL_URI;
    }

    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    int32_t ret = CloudSyncServiceProxy->ClearFileConflict(uri);
    return ret;
}

int32_t CloudSyncManagerImpl::DownloadThumb()
{
    LOGI("DownloadThumb start");
    auto CloudSyncServiceProxy = ServiceProxy::GetInstance("DownloadThumb");
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    int32_t ret = CloudSyncServiceProxy->DownloadThumb();
    LOGI("DownloadThumb ret %{public}d", ret);
    return ret;
}

void CloudSyncManagerImpl::SetDeathRecipient(const sptr<IRemoteObject> &remoteObject)
{
    if (remoteObject == nullptr) {
        LOGW("remoteObject is nullptr");
        return;
    }
    if (!isFirstCall_.test_and_set()) {
        auto deathCallback = [this](const wptr<IRemoteObject> &obj) {
            std::vector<CallbackInfo> callbackInfos;
            CloudSyncCallbackClientManager::GetInstance().GetAllCallback(callbackInfos);
            LOGE("service died, callback num:%{public}zu.", callbackInfos.size());
            for (auto &callbackInfo : callbackInfos) {
                callbackInfo.callback->OnDeathRecipient();
            }
            isFirstCall_.clear();
        };
        deathRecipient_ = sptr(new SvcDeathRecipient(deathCallback));
        if (!remoteObject->AddDeathRecipient(deathRecipient_)) {
            LOGE("add death recipient failed");
            isFirstCall_.clear();
        }
    }
}

int32_t CloudSyncManagerImpl::EnableCloud(const std::string &accoutId,
                                          const SwitchDataObj &switchData)
{
    auto CloudSyncServiceProxy = ServiceProxy::GetInstance("EnableCloud");
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }

    SetDeathRecipient(CloudSyncServiceProxy->AsObject());

    return CloudSyncServiceProxy->EnableCloud(accoutId, switchData);
}

int32_t CloudSyncManagerImpl::DisableCloud(const std::string &accoutId)
{
    auto CloudSyncServiceProxy = ServiceProxy::GetInstance("DisableCloud");
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }

    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    return CloudSyncServiceProxy->DisableCloud(accoutId);
}

int32_t CloudSyncManagerImpl::Clean(const std::string &accountId, const CleanOptions &cleanOptions)
{
    auto CloudSyncServiceProxy = ServiceProxy::GetInstance("Clean");
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }

    SetDeathRecipient(CloudSyncServiceProxy->AsObject());

    return CloudSyncServiceProxy->Clean(accountId, cleanOptions);
}

int32_t CloudSyncManagerImpl::CleanCache(const std::string &uri)
{
    LOGI("CleanCache Start");
    auto CloudSyncServiceProxy = ServiceProxy::GetInstance("CleanCache");
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    return CloudSyncServiceProxy->CleanCacheInner(uri);
}

int32_t CloudSyncManagerImpl::CleanAllFileCache()
{
    LOGI("CleanAllFileCache Start");
    auto CloudSyncServiceProxy = ServiceProxy::GetInstance("CleanAllFileCache");
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    return CloudSyncServiceProxy->CleanAllFileCacheInner();
}

int32_t CloudSyncManagerImpl::CleanAllFileCache(const std::string &bundleName)
{
    LOGI("CleanAllFileCache Start for bundle: %{public}s", bundleName.c_str());
    auto CloudSyncServiceProxy = ServiceProxy::GetInstance("CleanAllFileCache");
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    return CloudSyncServiceProxy->CleanAllFileCacheInner(bundleName);
}

int32_t CloudSyncManagerImpl::GetCachedTotalSize(int64_t &totalSize)
{
    LOGI("GetCachedTotalSize Start");
    auto CloudSyncServiceProxy = ServiceProxy::GetInstance("GetCachedTotalSize");
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    return CloudSyncServiceProxy->GetCachedTotalSizeInner(totalSize);
}

int32_t CloudSyncManagerImpl::GetCachedTotalSize(const std::string &bundleName, int64_t &totalSize)
{
    LOGI("GetCachedTotalSize Start for bundle: %{public}s", bundleName.c_str());
    auto CloudSyncServiceProxy = ServiceProxy::GetInstance("GetCachedTotalSize");
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    return CloudSyncServiceProxy->GetCachedTotalSizeInner(bundleName, totalSize);
}

int32_t CloudSyncManagerImpl::CleanFileCache(const std::string &uri)
{
    LOGI("CleanFileCache Start");
    auto CloudSyncServiceProxy = ServiceProxy::GetInstance("CleanFileCache");
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    int32_t ret = CloudSyncServiceProxy->CleanFileCacheInner(uri);
    LOGI("CleanFileCache end, ret: %{public}d", ret);
    return ret;
}

int32_t CloudSyncManagerImpl::BatchDentryFileInsert(const std::vector<DentryFileInfo> &fileInfo,
    std::vector<std::string> &failCloudId)
{
    if (fileInfo.size() > MAX_DENTRY_FILE_SIZE) {
        LOGE("BatchDentryFileInsert parameter is invalid");
        return E_INVAL_ARG;
    }
    auto CloudSyncServiceProxy = ServiceProxy::GetInstance("BatchDentryFileInsert");
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }

    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    std::vector<DentryFileInfoObj> fileInfoObj;
    for (const auto &info : fileInfo) {
        DentryFileInfoObj obj(info);
        fileInfoObj.emplace_back(obj);
    }

    return CloudSyncServiceProxy->BatchDentryFileInsert(fileInfoObj, failCloudId);
}

void CloudSyncManagerImpl::SubscribeListener(void)
{
    unique_lock<mutex> lock(subscribeMutex_);
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        LOGE("Samgr is nullptr");
        return;
    }
    if (listener_ != nullptr) {
        LOGI("The callback has already been subscribed to, no need to subscribe to it again.");
        return;
    }
    if (!CloudSyncCallbackClientManager::GetInstance().IsEmpty()) {
        listener_ = new SystemAbilityStatusChange();
        auto ret = samgr->SubscribeSystemAbility(FILEMANAGEMENT_CLOUD_SYNC_SERVICE_SA_ID, listener_);
        LOGI("subscribed to systemAbility ret %{public}d", ret);
    }
}

bool CloudSyncManagerImpl::ResetProxyCallback(uint32_t retryCount)
{
    auto cloudSyncServiceProxy = ServiceProxy::GetInstance("ResetProxyCallback");
    if (cloudSyncServiceProxy == nullptr) {
        LOGE("proxy is null");
        return false;
    }
    std::vector<CallbackInfo> callbackInfos;
    CloudSyncCallbackClientManager::GetInstance().GetAllCallback(callbackInfos);
    bool hasCallback = false;
    for (auto &callbackInfo : callbackInfos) {
        auto callback = sptr(new (std::nothrow) CloudSyncCallbackClient(callbackInfo.callback));
        if ((callback == nullptr) || (cloudSyncServiceProxy->RegisterFileSyncCallbackInner(
            callback, callbackInfo.callbackId, callbackInfo.bundleName) != E_OK)) {
            LOGW("register callback failed, try time is %{public}d", retryCount);
        } else {
            hasCallback = true;
        }
    }
    std::vector<UploadCallbackInfo> uploadCallbackInfos;
    CloudUploadCallbackClientManager::GetInstance().GetAllCallback(uploadCallbackInfos);
    for (auto &callbackInfo : uploadCallbackInfos) {
        auto callback = sptr(new (std::nothrow) CloudUploadCallbackClient(callbackInfo.callback));
        if ((callback == nullptr) || (cloudSyncServiceProxy->RegisterUploadCallbackInner(
            callback, callbackInfo.callbackId, callbackInfo.bundleName) != E_OK)) {
            LOGW("register upload callback failed, try time is %{public}d", retryCount);
        } else {
            hasCallback = true;
        }
    }
    if (hasCallback) {
        CloudSyncManagerImpl::GetInstance().SetDeathRecipient(cloudSyncServiceProxy->AsObject());
    }
    LOGD("Reset proxy, callback num:%{public}zu.", callbackInfos.size());

    return true;
}

void CloudSyncManagerImpl::SystemAbilityStatusChange::OnAddSystemAbility(int32_t systemAbilityId,
    const std::string &deviceId)
{
    const uint32_t RETRY_TIMES = 3;
    const uint32_t SLEEP_TIME = 20 * 1000;
    uint32_t retryCount = 0;
    LOGI("saId %{public}d loaded", systemAbilityId);
    do {
        usleep(SLEEP_TIME);
        if (!CloudSyncManagerImpl::GetInstance().ResetProxyCallback(retryCount)) {
            continue;
        }
        return;
    } while (++retryCount < RETRY_TIMES);
    LOGE("register callback failed, try too many times");
}

void CloudSyncManagerImpl::SystemAbilityStatusChange::OnRemoveSystemAbility(int32_t systemAbilityId,
    const std::string &deviceId)
{
    return;
}

void CloudSyncManagerImpl::CleanGalleryDentryFile()
{
    LOGI("CleanGalleryDentryFile start");
    const std::string photoDir = "/storage/media/cloud/files/Photo";
    const std::string thumbsDir = "/storage/media/cloud/files/.thumbs/Photo";
    if (!OHOS::Storage::DistributedFile::Utils::ForceRemoveDirectoryDeepFirst(photoDir)) {
        LOGW("remove photo dentry dir failed, errno: %{public}d", errno);
    }
    if (!OHOS::Storage::DistributedFile::Utils::ForceRemoveDirectoryDeepFirst(thumbsDir)) {
        LOGW("remove thumbs dentry dir failed, errno: %{public}d", errno);
    }
}

template <typename RemoveFn>
void LogAndDelete(const std::string& path, RemoveFn fn, const std::string& message)
{
    int ret = fn(path.c_str());
    if (ret != 0 && errno != ENOENT) {
        LOGE("%{public}s, errno %{public}d", message.c_str(), errno);
    }
}

static bool IsPhotoPath(const std::string& path)
{
    static const std::string prefix = "/storage/cloud/files/Photo/";
    return path.length() >= prefix.length() && path.compare(0, prefix.length(), prefix) == 0;
}

static std::string GetThumbsPath(const std::string& path)
{
    const string str = "files/";
    size_t len = str.size() - 1;
    auto found = path.find(str);
    if (found == string::npos) {
        LOGE("\"files/\" not found in path");
        return "";
    }
    std::string newPath = "/storage/media/cloud/files/.thumbs" + path.substr(found + len); // 待删除的文件路径
    return newPath;
}

static std::string GetMediaPath(const std::string& path)
{
    const string str = "storage/";
    size_t len = str.size() - 1;
    auto found = path.find(str);
    if (found == string::npos) {
        LOGE("\"storage/\" not found in path");
        return "";
    }
    std::string newPath = "/storage/media" + path.substr(found + len);
    return newPath;
}

void CloudSyncManagerImpl::CleanGalleryDentryFile(const std::string path)
{
    if (!IsPhotoPath(path)) {
        LOGE("CleanGalleryDentryFile path is not photo");
        return;
    }

    const std::string mediaDir = GetMediaPath(path);
    LogAndDelete(mediaDir, unlink, "fail to delete path");
    const std::string thumbsDir = GetThumbsPath(mediaDir);
    LogAndDelete(thumbsDir, ::remove, "fail to remove thumbsDir");
}

int32_t CloudSyncManagerImpl::GetBundlesLocalFilePresentStatus(const std::vector<std::string> &bundleNames,
    std::vector<LocalFilePresentStatus> &localFilePresentStatusList)
{
    auto CloudSyncServiceProxy = ServiceProxy::GetInstance("GetBundlesLocalFilePresentStatus");
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }

    if (bundleNames.empty()) {
        LOGE("Invalid argument");
        return EINVAL;
    }

    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    int32_t ret = CloudSyncServiceProxy->GetBundlesLocalFilePresentStatus(bundleNames, localFilePresentStatusList);
    return ret;
}

int32_t CloudSyncManagerImpl::IsFinishPull(bool &finishFlag)
{
    auto CloudSyncServiceProxy = ServiceProxy::GetInstance("IsFinishPull");
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    int32_t ret = CloudSyncServiceProxy->IsFinishPull(finishFlag);
    if (ret != E_OK) {
        LOGE("ret is %{public}d", ret);
    }
    return ret;
}

static bool ConvertToInt64(const std::string &str, int64_t &val)
{
    auto [ptr, ec] = std::from_chars(str.c_str(), str.c_str() + str.size(), val);
    return ec == std::errc() && ptr == str.c_str() + str.size();
}

static int64_t ReadConfigFile(const std::string &configFilePath)
{
    std::error_code ec;
    if (!std::filesystem::exists(configFilePath, ec)) {
        LOGI("ConfigFile doesn't exist, ec: %{public}d", ec.value());
        return 0;
    }

    std::ifstream file(configFilePath);
    if (!file.is_open()) {
        LOGE("open config file failed");
        return 0;
    }

    int32_t idx = 0;
    int64_t value = 0;
    std::string line;
    while (std::getline(file, line) && idx < MAX_OCCUPY_LINE_LIMIT) {
        size_t pos = line.find('=');
        if (pos != std::string::npos && line.substr(0, pos) == SUM_OCCUPY_FLAG) {
            std::string valueStr = line.substr(pos + 1);
            if (!ConvertToInt64(valueStr, value)) {
                LOGE("ConvertToInt64 failed, value: %{public}s", valueStr.c_str());
            }
            break;
        }
        idx += 1;
    }

    return value;
}

int32_t CloudSyncManagerImpl::GetDentryFileOccupy(int64_t &occupyNum)
{
    int32_t userId = getuid() / BASE_USER_RANGE;
    std::string dentryFileConfigPath = "/data/service/el2/" + std::to_string(userId) +
        "/hmdfs/cache/account_cache/" + GLOBAL_CONFIG_FILE_PATH;
    occupyNum = ReadConfigFile(dentryFileConfigPath);
    LOGI("client GetDentryFileOccupy: %{public}s", std::to_string(occupyNum).c_str());
    return E_OK;
}

int32_t CloudSyncManagerImpl::GetAclXattrBatch(const bool isAccess,
                                               const std::vector<std::string> &filePaths,
                                               std::vector<XattrResult> &aclXattrResults)
{
    auto CloudSyncServiceProxy = ServiceProxy::GetInstance("GetAclXattrBatch");
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    int32_t ret = CloudSyncServiceProxy->GetAclXattrBatch(isAccess, filePaths, aclXattrResults);
    if (ret != E_OK) {
        LOGE("ret is %{public}d", ret);
    }
    return ret;
}

void CloudSyncManagerImpl::SetStartSyncPending()
{
    unique_lock<mutex> lock(startSyncPendingMtx_);
    startSyncPending_ = false;
}

int32_t CloudSyncManagerImpl::GetDowngradeDownloadTaskState(const std::vector<std::string> &bundleNames,
    std::vector<DowngradeProgress> &downgradeProgressList)
{
    auto CloudSyncServiceProxy = ServiceProxy::GetInstance("GetDowngradeDownloadTaskState");
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }

    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    int32_t ret = CloudSyncServiceProxy->GetDowngradeDownloadTaskState(bundleNames, downgradeProgressList);
    return ret;
}

int32_t CloudSyncManagerImpl::SetMediaPreShared(const std::string &albumId, const std::string &albumName,
    const std::string &localPath)
{
    auto CloudSyncServiceProxy = ServiceProxy::GetInstance("SetMediaPreShared");
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    int32_t ret = CloudSyncServiceProxy->SetMediaPreShared(albumId, albumName, localPath);
    if (ret != E_OK) {
        LOGE("ret is %{public}d id: %{public}s name: %{public}s path: %{public}s",
            ret, albumId.c_str(), albumName.c_str(), localPath.c_str());
    }
    return ret;
}

} // namespace OHOS::FileManagement::CloudSync
