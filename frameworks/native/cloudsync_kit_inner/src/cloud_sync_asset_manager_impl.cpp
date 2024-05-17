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

#include "cloud_sync_asset_manager_impl.h"

#include <cinttypes>

#include "cloud_sync_service_proxy.h"
#include "dfs_error.h"
#include "download_asset_callback_client.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
CloudSyncAssetManagerImpl &CloudSyncAssetManagerImpl::GetInstance()
{
    static CloudSyncAssetManagerImpl instance;
    return instance;
}

int32_t CloudSyncAssetManagerImpl::UploadAsset(const int32_t userId,
                                               const std::string &request,
                                               std::string &result)
{
    auto CloudSyncServiceProxy = CloudSyncServiceProxy::GetInstance();
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }

    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    int32_t ret = CloudSyncServiceProxy->UploadAsset(userId, request, result);
    LOGD("UploadAsset ret %{public}d", ret);
    return ret;
}

int32_t CloudSyncAssetManagerImpl::DownloadFile(const int32_t userId,
                                                const std::string &bundleName,
                                                AssetInfo &assetInfo)
{
    auto CloudSyncServiceProxy = CloudSyncServiceProxy::GetInstance();
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }

    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    AssetInfoObj assetInfoObj(assetInfo);
    int32_t ret = CloudSyncServiceProxy->DownloadFile(userId, bundleName, assetInfoObj);
    LOGI("DownloadFile ret %{public}d", ret);
    return ret;
}

int32_t CloudSyncAssetManagerImpl::DownloadFiles(const int32_t userId,
                                                 const std::string &bundleName,
                                                 const std::vector<AssetInfo> &assetInfo,
                                                 std::vector<bool> &assetResultMap)
{
    auto CloudSyncServiceProxy = CloudSyncServiceProxy::GetInstance();
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }

    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    std::vector<AssetInfoObj> assetInfoObj;
    for (const auto &info : assetInfo) {
        AssetInfoObj obj(info);
        assetInfoObj.emplace_back(obj);
    }
    int32_t ret = CloudSyncServiceProxy->DownloadFiles(userId, bundleName, assetInfoObj, assetResultMap);
    LOGI("DownloadFile ret %{public}d", ret);
    return ret;
}

int32_t CloudSyncAssetManagerImpl::DownloadFile(const int32_t userId,
                                                const std::string &bundleName,
                                                const std::string &networkId,
                                                AssetInfo &assetInfo,
                                                ResultCallback resultCallback)
{
    auto CloudSyncServiceProxy = CloudSyncServiceProxy::GetInstance();
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }

    {
        std::lock_guard<std::mutex> lock(callbackInitMutex_);
        if (downloadAssetCallback_ == nullptr) {
            downloadAssetCallback_ = sptr(new (std::nothrow) DownloadAssetCallbackClient());
        }
        if (downloadAssetCallback_ == nullptr) {
            LOGE("have no enough memory");
            return E_MEMORY;
        }
    }

    if (!isCallbackRegistered_.test_and_set()) {
        LOGI("register callback");
        CloudSyncServiceProxy->RegisterDownloadAssetCallback(downloadAssetCallback_);
    }
    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    auto taskId = downloadAssetCallback_->GetTaskId();
    downloadAssetCallback_->AddDownloadTaskCallback(taskId, resultCallback);
    AssetInfoObj assetInfoObj(assetInfo);
    int32_t ret = CloudSyncServiceProxy->DownloadAsset(taskId, userId, bundleName, networkId, assetInfoObj);
    LOGI("DownloadFile ret %{public}d, taskId:%{public}" PRIu64 "", ret, taskId);
    return ret;
}

int32_t CloudSyncAssetManagerImpl::DeleteAsset(const int32_t userId, const std::string &uri)
{
    auto CloudSyncServiceProxy = CloudSyncServiceProxy::GetInstance();
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }

    SetDeathRecipient(CloudSyncServiceProxy->AsObject());
    int32_t ret = CloudSyncServiceProxy->DeleteAsset(userId, uri);
    LOGI("DeleteAsset ret %{public}d", ret);
    return ret;
}

void CloudSyncAssetManagerImpl::SetDeathRecipient(const sptr<IRemoteObject> &remoteObject)
{
    if (!isFirstCall_.test_and_set()) {
        auto deathCallback = [this](const wptr<IRemoteObject> &obj) {
            LOGE("service died. Died remote obj");
            CloudSyncServiceProxy::InvaildInstance();
            isCallbackRegistered_.clear();
            isFirstCall_.clear();
        };
        deathRecipient_ = sptr(new SvcDeathRecipient(deathCallback));
        remoteObject->AddDeathRecipient(deathRecipient_);
    }
}
} // namespace OHOS::FileManagement::CloudSync
