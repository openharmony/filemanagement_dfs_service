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
#include "cloud_sync_service_proxy.h"
#include "dfs_error.h"
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
    int32_t ret = CloudSyncServiceProxy->UploadAsset(userId, request, result);
    LOGI("UploadAsset ret %{public}d", ret);
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
    AssetInfoObj assetInfoObj(assetInfo);
    int32_t ret = CloudSyncServiceProxy->DownloadFile(userId, bundleName, assetInfoObj);
    LOGI("DownloadFile ret %{public}d", ret);
    return ret;
}
} // namespace OHOS::FileManagement::CloudSync
