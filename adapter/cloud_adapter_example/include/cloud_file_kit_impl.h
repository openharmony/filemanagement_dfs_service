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

#ifndef OHOS_CLOUD_FILE_CLOUD_FILE_KIT_IMPL_H
#define OHOS_CLOUD_FILE_CLOUD_FILE_KIT_IMPL_H

#include "cloud_file_kit.h"
#include <mutex>

namespace OHOS::FileManagement::CloudFile {
class CloudFileKitImpl final : public CloudFileKit {
public:
    int32_t GetCloudUserInfo(const int32_t userId, CloudUserInfo &userInfo) override;
    std::pair<uint64_t, uint64_t> GetSpaceInfo(const int32_t userId, const std::string& bundleName) override;
    int32_t GetAppSwitchStatus(const std::string &bundleName, const int32_t userId, bool &switchStatus) override;
    int32_t ResolveNotificationEvent(const int32_t userId,
                                     const std::string &extraData,
                                     std::string &appBundleName,
                                     std::string &prepareTraceId) override;

    std::shared_ptr<DataSyncManager> GetDataSyncManager() override;
    std::shared_ptr<CloudAssetsDownloader> GetCloudAssetsDownloader(const int32_t userId,
                                                                    const std::string &bundleName) override;
    std::shared_ptr<CloudDatabase> GetCloudDatabase(const int32_t userId, const std::string &bundleName) override;
    std::shared_ptr<CloudSyncHelper> GetCloudSyncHelper(const int32_t userId, const std::string &bundleName) override;
    std::string GetPrepareTraceId(const int32_t userId) override;

private:
    std::mutex dataSyncManagerMutex_;
    std::shared_ptr<DataSyncManager> dataSyncManager_;

    std::mutex cloudAssetsDownloaderMutex_;
    std::shared_ptr<CloudAssetsDownloader> cloudAssetsDownloader_;

    std::mutex cloudSyncHelperMutex_;
    std::shared_ptr<CloudSyncHelper> cloudSyncHelper_;

    std::mutex cloudDatabaseMutex_;
    std::shared_ptr<CloudDatabase> cloudDatabase_;
};
} // namespace OHOS::FileManagement::CloudFile

#endif // OHOS_CLOUD_FILE_CLOUD_FILE_KIT_IMPL_H