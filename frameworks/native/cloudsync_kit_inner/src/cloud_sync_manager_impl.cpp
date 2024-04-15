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
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
using namespace std;
constexpr int32_t MIN_USER_ID = 100;
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
    SubscribeListener();
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

int32_t CloudSyncManagerImpl::StopSync(const std::string &bundleName)
{
    auto CloudSyncServiceProxy = CloudSyncServiceProxy::GetInstance();
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    return CloudSyncServiceProxy->StopSyncInner(bundleName);
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

int32_t CloudSyncManagerImpl::StartFileCache(const std::string &uri)
{
    LOGI("StartDownloadCache start");
    auto CloudSyncServiceProxy = CloudSyncServiceProxy::GetInstance();
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    int32_t ret = CloudSyncServiceProxy->StartFileCache(uri);
    LOGI("StartDownloadCache ret %{public}d", ret);
    return ret;
}

int32_t CloudSyncManagerImpl::StopDownloadFile(const std::string &uri)
{
    LOGI("StopDownloadFile start");
    auto CloudSyncServiceProxy = CloudSyncServiceProxy::GetInstance();
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    int32_t ret = CloudSyncServiceProxy->StopDownloadFile(uri);
    LOGI("StopDownloadFile ret %{public}d", ret);
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
    int32_t ret = CloudSyncServiceProxy->RegisterDownloadFileCallback(
        sptr(new (std::nothrow) CloudDownloadCallbackClient(downloadCallback)));
    LOGI("RegisterDownloadFileCallback ret %{public}d", ret);
    downloadCallback_ = downloadCallback;
    SubscribeListener();
    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    return ret;
}

int32_t CloudSyncManagerImpl::UnregisterDownloadFileCallback()
{
    LOGI("UnregisterDownloadFileCallback start");
    auto CloudSyncServiceProxy = CloudSyncServiceProxy::GetInstance();
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    int32_t ret = CloudSyncServiceProxy->UnregisterDownloadFileCallback();
    LOGI("UnregisterDownloadFileCallback ret %{public}d", ret);
    if (ret == E_OK) {
        downloadCallback_ = nullptr;
        SubscribeListener();
    }
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
        remoteObject->AddDeathRecipient(deathRecipient_);
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

void CloudSyncManagerImpl::SubscribeListener()
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
        listener_ = new SystemAbilityStatusChange(callback_, downloadCallback_);
        auto ret = samgr->SubscribeSystemAbility(FILEMANAGEMENT_CLOUD_SYNC_SERVICE_SA_ID, listener_);
        LOGI("subscribed to systemAbility ret %{public}d", ret);
    } else {
        listener_ = nullptr;
    }
}

void CloudSyncManagerImpl::SystemAbilityStatusChange::OnAddSystemAbility(int32_t systemAbilityId,
    const std::string &deviceId)
{
    auto CloudSyncServiceProxy = CloudSyncServiceProxy::GetInstance();
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return;
    }
    if (downloadCallback_) {
        CloudSyncServiceProxy->RegisterDownloadFileCallback(
            sptr(new (std::nothrow) CloudDownloadCallbackClient(downloadCallback_)));
        CloudSyncManagerImpl::GetInstance().SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    }
    if (callback_) {
        CloudSyncServiceProxy->RegisterCallbackInner(sptr(new (std::nothrow) CloudSyncCallbackClient(callback_)));
        CloudSyncManagerImpl::GetInstance().SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    }
    return;
}

void CloudSyncManagerImpl::SystemAbilityStatusChange::OnRemoveSystemAbility(int32_t systemAbilityId,
    const std::string &deviceId)
{
    return;
}
} // namespace OHOS::FileManagement::CloudSync
