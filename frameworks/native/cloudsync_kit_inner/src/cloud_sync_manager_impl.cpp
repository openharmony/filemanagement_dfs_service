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

#include "cloud_download_callback_client.h"
#include "cloud_sync_manager_impl.h"
#include "cloud_sync_callback_client.h"
#include "cloud_sync_service_proxy.h"
#include "dfs_error.h"
#include "system_ability_definition.h"
#include "iservice_registry.h"
#include "utils_directory.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
using namespace std;
constexpr int32_t MIN_USER_ID = 100;
constexpr int32_t MAX_FILE_CACHE_NUM = 400;
constexpr int32_t MAX_DENTRY_FILE_SIZE = 500;
CloudSyncManagerImpl &CloudSyncManagerImpl::GetInstance()
{
    static CloudSyncManagerImpl instance;
    return instance;
}

int32_t CloudSyncManagerImpl::RegisterCallback(const std::shared_ptr<CloudSyncCallback> callback,
                                               const std::string &bundleName)
{
    if (!callback) {
        LOGE("callback is null");
        return E_INVAL_ARG;
    }
    auto CloudSyncServiceProxy = CloudSyncServiceProxy::GetInstance();
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    auto ret = CloudSyncServiceProxy->RegisterCallbackInner(sptr(new (std::nothrow) CloudSyncCallbackClient(callback)),
                                                            bundleName);
    {
        unique_lock<mutex> lock(callbackMutex_);
        callback_ = callback;
    }
    SubscribeListener(bundleName);
    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    LOGI("RegisterCallback ret %{public}d", ret);
    return ret;
}

int32_t CloudSyncManagerImpl::UnRegisterCallback(const std::string &bundleName)
{
    auto CloudSyncServiceProxy = CloudSyncServiceProxy::GetInstance();
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }

    auto ret = CloudSyncServiceProxy->UnRegisterCallbackInner(bundleName);
    if (!ret) {
        {
            unique_lock<mutex> lock(callbackMutex_);
            callback_ = nullptr;
        }
        SubscribeListener();
    }
    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    LOGI("UnRegisterCallback ret %{public}d", ret);
    return ret;
}

int32_t CloudSyncManagerImpl::StartSync(const std::string &bundleName)
{
    auto CloudSyncServiceProxy = CloudSyncServiceProxy::GetInstance();
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    return CloudSyncServiceProxy->StartSyncInner(true, bundleName);
}

int32_t CloudSyncManagerImpl::GetSyncTime(int64_t &syncTime, const std::string &bundleName)
{
    LOGI("GetSyncTime Start");
    auto CloudSyncServiceProxy = CloudSyncServiceProxy::GetInstance();
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
    auto CloudSyncServiceProxy = CloudSyncServiceProxy::GetInstance();
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    SetDeathRecipient(CloudSyncServiceProxy->AsObject());

    bool isCallbackValid = false;
    sptr<CloudOptimizeCallbackClient> opCallback = nullptr;
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
    auto CloudSyncServiceProxy = CloudSyncServiceProxy::GetInstance();
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
    auto CloudSyncServiceProxy = CloudSyncServiceProxy::GetInstance();
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }

    if (!isFirstCall_.test()) {
        LOGI("Register callback");
        auto ret =
            CloudSyncServiceProxy->RegisterCallbackInner(sptr(new (std::nothrow) CloudSyncCallbackClient(callback)));
        if (ret) {
            LOGE("Register callback failed");
            isFirstCall_.clear();
            return ret;
        }
        callback_ = callback;
        SubscribeListener();
        SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    }

    return CloudSyncServiceProxy->StartSyncInner(forceFlag);
}

int32_t CloudSyncManagerImpl::TriggerSync(const std::string &bundleName, const int32_t &userId)
{
    if (bundleName.empty() || userId < MIN_USER_ID) {
        LOGE("Trigger Sync parameter is invalid");
        return E_INVAL_ARG;
    }
    auto CloudSyncServiceProxy = CloudSyncServiceProxy::GetInstance();
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    return CloudSyncServiceProxy->TriggerSyncInner(bundleName, userId);
}

int32_t CloudSyncManagerImpl::StopSync(const std::string &bundleName, bool forceFlag)
{
    auto CloudSyncServiceProxy = CloudSyncServiceProxy::GetInstance();
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    return CloudSyncServiceProxy->StopSyncInner(bundleName, forceFlag);
}

int32_t CloudSyncManagerImpl::ResetCursor(const std::string &bundleName)
{
    auto CloudSyncServiceProxy = CloudSyncServiceProxy::GetInstance();
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    return CloudSyncServiceProxy->ResetCursor(bundleName);
}

int32_t CloudSyncManagerImpl::ChangeAppSwitch(const std::string &accoutId, const std::string &bundleName, bool status)
{
    auto CloudSyncServiceProxy = CloudSyncServiceProxy::GetInstance();
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
    auto CloudSyncServiceProxy = CloudSyncServiceProxy::GetInstance();
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
    auto CloudSyncServiceProxy = CloudSyncServiceProxy::GetInstance();
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }

    SetDeathRecipient(CloudSyncServiceProxy->AsObject());

    int32_t ret = CloudSyncServiceProxy->NotifyEventChange(userId, eventId, extraData);
    LOGI("NotifyDataChange ret %{public}d", ret);
    return ret;
}

int32_t CloudSyncManagerImpl::StartDownloadFile(const std::string &uri)
{
    LOGI("StartDownloadFile start");
    auto CloudSyncServiceProxy = CloudSyncServiceProxy::GetInstance();
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    int32_t ret = CloudSyncServiceProxy->StartDownloadFile(uri);
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
    auto CloudSyncServiceProxy = CloudSyncServiceProxy::GetInstance();
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

int32_t CloudSyncManagerImpl::StartFileCache(const std::string &uri)
{
    LOGI("StartFileCache start");
    int64_t downloadId = 0;
    auto CloudSyncServiceProxy = CloudSyncServiceProxy::GetInstance();
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    std::vector<std::string> uriVec;
    uriVec.push_back(uri);
    int32_t ret = CloudSyncServiceProxy->StartFileCache(uriVec, downloadId);
    LOGI("StartFileCache ret %{public}d", ret);
    return ret;
}

int32_t CloudSyncManagerImpl::StartFileCache(const std::vector<std::string> &uriVec,
                                             int64_t &downloadId, std::bitset<FIELD_KEY_MAX_SIZE> fieldkey,
                                             const std::shared_ptr<CloudDownloadCallback> downloadCallback,
                                             int32_t timeout)
{
    LOGI("StartFileCache batch start, uriVec size: %{public}zu, fieldKey: %{public}llu, Callback is null: %{public}d",
         uriVec.size(), static_cast<unsigned long long>(fieldkey.to_ulong()), (downloadCallback == nullptr));
    if (uriVec.empty()) {
        LOGE("StartFileCache, uri list is empty");
        return E_INVAL_ARG;
    }
    if (uriVec.size() > MAX_FILE_CACHE_NUM) {
        LOGE("StartFileCache, the size of uri list exceeded the maximum limit, size: %{public}zu", uriVec.size());
        return E_EXCEED_MAX_SIZE;
    }
    auto CloudSyncServiceProxy = CloudSyncServiceProxy::GetInstance();
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    SetDeathRecipient(CloudSyncServiceProxy->AsObject());

    bool isCallbackValid = false;
    sptr<CloudDownloadCallbackClient> dlCallback = nullptr;
    if (downloadCallback != nullptr) {
        dlCallback = sptr(new (std::nothrow) CloudDownloadCallbackClient(downloadCallback));
        isCallbackValid = true;
        if (dlCallback == nullptr) {
            LOGE("register download callback failed");
            isCallbackValid = false;
        }
    }

    int32_t ret = CloudSyncServiceProxy->StartFileCache(uriVec, downloadId, fieldkey,
        isCallbackValid, dlCallback, timeout);
    LOGI("StartFileCache batch ret %{public}d", ret);
    return ret;
}

int32_t CloudSyncManagerImpl::StopDownloadFile(const std::string &uri, bool needClean)
{
    LOGI("StopDownloadFile start");
    auto CloudSyncServiceProxy = CloudSyncServiceProxy::GetInstance();
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    int32_t ret = CloudSyncServiceProxy->StopDownloadFile(uri, needClean);
    LOGI("StopDownloadFile ret %{public}d", ret);
    return ret;
}

int32_t CloudSyncManagerImpl::StopFileCache(int64_t downloadId, bool needClean, int32_t timeout)
{
    LOGI("StopFileCache start");
    auto CloudSyncServiceProxy = CloudSyncServiceProxy::GetInstance();
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    int32_t ret = CloudSyncServiceProxy->StopFileCache(downloadId, needClean, timeout);
    LOGI("StopFileCache ret %{public}d", ret);
    return ret;
}

int32_t CloudSyncManagerImpl::DownloadThumb()
{
    LOGI("DownloadThumb start");
    auto CloudSyncServiceProxy = CloudSyncServiceProxy::GetInstance();
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    int32_t ret = CloudSyncServiceProxy->DownloadThumb();
    LOGI("DownloadThumb ret %{public}d", ret);
    return ret;
}

int32_t CloudSyncManagerImpl::RegisterDownloadFileCallback(
    const std::shared_ptr<CloudDownloadCallback> downloadCallback)
{
    LOGI("RegisterDownloadFileCallback start");
    auto CloudSyncServiceProxy = CloudSyncServiceProxy::GetInstance();
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    {
        unique_lock<mutex> lock(downloadMutex_);
        auto dlCallback = sptr(new (std::nothrow) CloudDownloadCallbackClient(downloadCallback));
        if (dlCallback == nullptr ||
            CloudSyncServiceProxy->RegisterDownloadFileCallback(dlCallback) != E_OK) {
            LOGE("register download callback failed");
        } else {
            downloadCallback_ = downloadCallback;
        }
    }
    SubscribeListener();
    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    return E_OK;
}

int32_t CloudSyncManagerImpl::UnregisterDownloadFileCallback()
{
    LOGI("UnregisterDownloadFileCallback start");
    auto CloudSyncServiceProxy = CloudSyncServiceProxy::GetInstance();
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    int32_t ret = E_OK;
    {
        unique_lock<mutex> lock(downloadMutex_);
        ret = CloudSyncServiceProxy->UnregisterDownloadFileCallback();
        LOGI("UnregisterDownloadFileCallback ret %{public}d", ret);
        if (ret == E_OK) {
            downloadCallback_ = nullptr;
        }
    }
    SubscribeListener();
    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    return ret;
}
void CloudSyncManagerImpl::SetDeathRecipient(const sptr<IRemoteObject> &remoteObject)
{
    if (!isFirstCall_.test_and_set()) {
        auto deathCallback = [this](const wptr<IRemoteObject> &obj) {
            LOGE("service died.");
            CloudSyncServiceProxy::InvaildInstance();
            if (callback_) {
                callback_->OnSyncStateChanged(CloudSyncState::COMPLETED, ErrorType::NO_ERROR);
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
    auto CloudSyncServiceProxy = CloudSyncServiceProxy::GetInstance();
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }

    SetDeathRecipient(CloudSyncServiceProxy->AsObject());

    return CloudSyncServiceProxy->EnableCloud(accoutId, switchData);
}

int32_t CloudSyncManagerImpl::DisableCloud(const std::string &accoutId)
{
    auto CloudSyncServiceProxy = CloudSyncServiceProxy::GetInstance();
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }

    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    return CloudSyncServiceProxy->DisableCloud(accoutId);
}

int32_t CloudSyncManagerImpl::Clean(const std::string &accountId, const CleanOptions &cleanOptions)
{
    auto CloudSyncServiceProxy = CloudSyncServiceProxy::GetInstance();
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
    auto CloudSyncServiceProxy = CloudSyncServiceProxy::GetInstance();
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    return CloudSyncServiceProxy->CleanCacheInner(uri);
}

int32_t CloudSyncManagerImpl::BatchDentryFileInsert(const std::vector<DentryFileInfo> &fileInfo,
    std::vector<std::string> &failCloudId)
{
    if (fileInfo.size() > MAX_DENTRY_FILE_SIZE) {
        LOGE("BatchDentryFileInsert parameter is invalid");
        return E_INVAL_ARG;
    }
    auto CloudSyncServiceProxy = CloudSyncServiceProxy::GetInstance();
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

void CloudSyncManagerImpl::SubscribeListener(std::string bundleName)
{
    unique_lock<mutex> lock(subscribeMutex_);
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        LOGE("Samgr is nullptr");
        return;
    }
    if (listener_ != nullptr) {
        auto ret = samgr->UnSubscribeSystemAbility(FILEMANAGEMENT_CLOUD_SYNC_SERVICE_SA_ID, listener_);
        LOGI("unsubscribed to systemAbility ret %{public}d", ret);
    }
    if (callback_ != nullptr || downloadCallback_ != nullptr) {
        listener_ = new SystemAbilityStatusChange(bundleName);
        auto ret = samgr->SubscribeSystemAbility(FILEMANAGEMENT_CLOUD_SYNC_SERVICE_SA_ID, listener_);
        LOGI("subscribed to systemAbility ret %{public}d", ret);
    } else {
        listener_ = nullptr;
    }
}

bool CloudSyncManagerImpl::ResetProxyCallback(uint32_t retryCount, const string &bundleName)
{
    auto cloudSyncServiceProxy = CloudSyncServiceProxy::GetInstance();
    if (cloudSyncServiceProxy == nullptr) {
        LOGE("proxy is null");
        return false;
    }
    bool hasCallback = false;
    {
        unique_lock<mutex> downloadLock(downloadMutex_);
        if (downloadCallback_ != nullptr) {
            auto dlCallback = sptr(new (std::nothrow) CloudDownloadCallbackClient(downloadCallback_));
            if (dlCallback == nullptr ||
                cloudSyncServiceProxy->RegisterDownloadFileCallback(dlCallback) != E_OK) {
                LOGW("register download callback failed, try time is %{public}d", retryCount);
            } else {
                hasCallback = true;
            }
        }
    }
    if (callback_ != nullptr) {
        auto callback = sptr(new (std::nothrow) CloudSyncCallbackClient(callback_));
        if (callback == nullptr ||
            cloudSyncServiceProxy->RegisterCallbackInner(callback, bundleName) != E_OK) {
            LOGW("register callback failed, try time is %{public}d", retryCount);
        } else {
            hasCallback = true;
        }
    }
    if (hasCallback) {
        CloudSyncManagerImpl::GetInstance().SetDeathRecipient(cloudSyncServiceProxy->AsObject());
    }
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
        if (!CloudSyncManagerImpl::GetInstance().ResetProxyCallback(retryCount, bundleName_)) {
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
} // namespace OHOS::FileManagement::CloudSync
