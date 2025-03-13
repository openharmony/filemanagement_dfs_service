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

#include "cloud_file_kit_impl.h"

#include <memory>

#include "cloud_database_impl.h"
#include "cloud_assets_downloader_impl.h"
#include "cloud_sync_helper_impl.h"
#include "data_sync_manager_impl.h"
#include "dfs_error.h"


namespace OHOS::FileManagement::CloudFile {

__attribute__((used)) static bool g_isInit =
    CloudFileKit::RegisterCloudInstance(new (std::nothrow) CloudFileKitImpl());

using namespace std;

int32_t CloudFileKitImpl::GetCloudUserInfo(const int32_t userId, CloudUserInfo &userInfo)
{
    userInfo.enableCloud = true;
    return E_OK;
}

std::pair<uint64_t, uint64_t> CloudFileKitImpl::GetSpaceInfo(const int32_t userId, const std::string &bundleName)
{
    return std::make_pair(0, 0);
}

int32_t CloudFileKitImpl::GetAppSwitchStatus(const std::string &bundleName,
                                             const int32_t userId,
                                             bool &switchStatus)
{
    switchStatus = true;
    return E_OK;
}

int32_t CloudFileKitImpl::ResolveNotificationEvent(const int32_t userId,
                                                   const std::string &extraData,
                                                   std::string &appBundleName,
                                                   std::string &prepareTraceId)
{
    appBundleName = "com.ohos.photos";
    return E_OK;
}

std::shared_ptr<DataSyncManager> CloudFileKitImpl::GetDataSyncManager()
{
    std::lock_guard<std::mutex> lck(dataSyncManagerMutex_);
    if (dataSyncManager_ == nullptr) {
        dataSyncManager_ = std::make_shared<DataSyncManagerImpl>();
    }
    return dataSyncManager_;
}

std::shared_ptr<CloudAssetsDownloader> CloudFileKitImpl::GetCloudAssetsDownloader(const int32_t userId,
                                                                                  const std::string &bundleName)
{
    std::lock_guard<std::mutex> lck(cloudAssetsDownloaderMutex_);
    if (cloudAssetsDownloader_ == nullptr) {
        cloudAssetsDownloader_ = std::make_shared<CloudAssetsDownloaderImpl>(userId, bundleName);
    }
    return cloudAssetsDownloader_;
}

std::shared_ptr<CloudSyncHelper> CloudFileKitImpl::GetCloudSyncHelper(const int32_t userId,
                                                                      const std::string &bundleName)
{
    std::lock_guard<std::mutex> lck(cloudSyncHelperMutex_);
    if (cloudSyncHelper_ == nullptr) {
        std::shared_ptr<CloudSyncHelper> cloudSyncHelper = std::make_shared<CloudSyncHelperImpl>(userId, bundleName);
        if (cloudSyncHelper->Init() == E_OK) {
            cloudSyncHelper_ = cloudSyncHelper;
        }
    }
    return cloudSyncHelper_;
}

std::shared_ptr<CloudDatabase> CloudFileKitImpl::GetCloudDatabase(const int32_t userId, const std::string &bundleName)
{
    std::lock_guard<std::mutex> lck(cloudDatabaseMutex_);
    if (cloudDatabase_ == nullptr) {
        auto cloudDatabase = std::make_shared<CloudDatabaseImpl>(userId, bundleName);
        if (cloudDatabase->Init() == E_OK) {
            cloudDatabase_ = cloudDatabase;
        }
    }
    return cloudDatabase_;
}

std::string CloudFileKitImpl::GetPrepareTraceId(const int32_t userId)
{
    return "";
}
} // namespace OHOS::FileManagement::CloudFile