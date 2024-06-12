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

#ifndef OHOS_FILEMGMT_CLOUD_SYNC_ASSET_MANAGER_IMPL_H
#define OHOS_FILEMGMT_CLOUD_SYNC_ASSET_MANAGER_IMPL_H

#include <atomic>
#include <mutex>

#include "cloud_sync_asset_manager.h"
#include "download_asset_callback_client.h"
#include "nocopyable.h"
#include "svc_death_recipient.h"

namespace OHOS::FileManagement::CloudSync {
class CloudSyncAssetManagerImpl final : public CloudSyncAssetManager, public NoCopyable {
public:
    static CloudSyncAssetManagerImpl &GetInstance();

    int32_t UploadAsset(const int32_t userId, const std::string &request, std::string &result) override;
    int32_t DownloadFile(const int32_t userId, const std::string &bundleName, AssetInfo &assetInfo) override;
    int32_t DeleteAsset(const int32_t userId, const std::string &uri) override;
    int32_t DownloadFile(const int32_t userId,
                         const std::string &bundleName,
                         const std::string &networkId,
                         AssetInfo &assetInfo,
                         ResultCallback resultCallback) override;
    int32_t DownloadFiles(const int32_t userId,
                          const std::string &bundleName,
                          const std::vector<AssetInfo> &assetInfo,
                          std::vector<bool> &assetResultMap) override;

private:
    CloudSyncAssetManagerImpl() = default;
    void SetDeathRecipient(const sptr<IRemoteObject> &remoteObject);

    sptr<DownloadAssetCallbackClient> downloadAssetCallback_;
    std::atomic_flag isCallbackRegistered_{false};
    std::atomic_flag isFirstCall_{false};
    sptr<SvcDeathRecipient> deathRecipient_;
    std::mutex callbackInitMutex_;
};
} // namespace OHOS::FileManagement::CloudSync

#endif // OHOS_FILEMGMT_CLOUD_SYNC_ASSET_MANAGER_IMPL_H
