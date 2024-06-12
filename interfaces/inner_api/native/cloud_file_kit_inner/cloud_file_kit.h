/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_CLOUD_FILE_CLOUD_FILE_KIT_H
#define OHOS_CLOUD_FILE_CLOUD_FILE_KIT_H

#include <memory>

#include "cloud_assets_downloader.h"
#include "cloud_database.h"
#include "cloud_info.h"
#include "cloud_sync_helper.h"
#include "data_sync_manager.h"
#include "visibility.h"


namespace OHOS::FileManagement::CloudFile {
class API_EXPORT CloudFileKit {
public:
    API_EXPORT static CloudFileKit *GetInstance();
    API_EXPORT static bool RegisterCloudInstance(CloudFileKit *instance);

    virtual ~CloudFileKit() = default;
    virtual int32_t GetCloudUserInfo(const int32_t userId, CloudUserInfo &userInfo);
    virtual int32_t GetAppSwitchStatus(const std::string &bundleName, const int32_t userId, bool &switchStatus);
    virtual int32_t ResolveNotificationEvent(const int32_t userId,
                                             const std::string &extraData,
                                             std::string &appBundleName);
    virtual int32_t GetAppConfigParams(const int32_t userId,
                                       const std::string &bundleName,
                                       std::map<std::string, std::string> &param);
    virtual int32_t CleanCloudUserInfo();
    virtual int32_t OnUploadAsset(const int32_t userId, const std::string &request, std::string &result);

    virtual std::shared_ptr<DataSyncManager> GetDataSyncManager();
    virtual std::shared_ptr<CloudDatabase> GetCloudDatabase(const int32_t userId, const std::string &bundleName);
    virtual std::shared_ptr<CloudAssetsDownloader> GetCloudAssetsDownloader(const int32_t userId,
                                                                            const std::string &bundleName);
    virtual std::shared_ptr<CloudSyncHelper> GetCloudSyncHelper(const int32_t userId, const std::string &bundleName);

private:
    static inline CloudFileKit *instance_;
};
} // namespace OHOS::FileManagement::CloudFile

#endif // OHOS_CLOUD_FILE_CLOUD_FILE_KIT_H