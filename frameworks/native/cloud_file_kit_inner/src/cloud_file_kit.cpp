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

#include "cloud_file_kit.h"
#include "dfs_error.h"

namespace OHOS::FileManagement::CloudFile {
using namespace std;

CloudFileKit *CloudFileKit::GetInstance()
{
    return instance_;
}

bool CloudFileKit::RegisterCloudInstance(CloudFileKit *instance)
{
    if (instance_ != nullptr) {
        return false;
    }
    instance_ = instance;
    return true;
}

int32_t CloudFileKit::GetCloudUserInfo(const int32_t userId, CloudUserInfo &userInfo)
{
    userInfo.enableCloud = true;
    return E_OK;
}

std::pair<uint64_t, uint64_t> CloudFileKit::GetSpaceInfo(const int32_t userId, const std::string &bundleName)
{
    return make_pair(0, 0);
}

int32_t CloudFileKit::GetAppSwitchStatus(const std::string &bundleName, const int32_t userId, bool &switchStatus)
{
    switchStatus = true;
    return E_OK;
}

int32_t CloudFileKit::ResolveNotificationEvent(const int32_t userId,
                                               const std::string &extraData,
                                               std::string &appBundleName,
                                               std::string &prepareTraceId)
{
    appBundleName = "com.ohos.photos";
    return E_OK;
}

int32_t CloudFileKit::GetAppConfigParams(const int32_t userId,
                                         const std::string &bundleName,
                                         std::map<std::string, std::string> &param)
{
    param["validDays"] = "30";
    param["dataAgingPolicy"] = "0";
    return E_OK;
}

int32_t CloudFileKit::CleanCloudUserInfo(const int32_t userId)
{
    return E_OK;
}

shared_ptr<DataSyncManager> CloudFileKit::GetDataSyncManager()
{
    return std::make_shared<DataSyncManager>();
}

int32_t CloudFileKit::OnUploadAsset(const int32_t userId, const std::string &request, std::string &result)
{
    return E_OK;
}

std::shared_ptr<CloudAssetsDownloader> CloudFileKit::GetCloudAssetsDownloader(const int32_t userId,
                                                                              const std::string &bundleName)
{
    return make_shared<CloudAssetsDownloader>(userId, bundleName);
}

std::shared_ptr<CloudDatabase> CloudFileKit::GetCloudDatabase(const int32_t userId, const std::string &bundleName)
{
    return make_shared<CloudDatabase>(userId, bundleName);
}

std::shared_ptr<CloudSyncHelper> CloudFileKit::GetCloudSyncHelper(const int32_t userId, const std::string &bundleName)
{
    return make_shared<CloudSyncHelper>(userId, bundleName);
}

std::string CloudFileKit::GetPrepareTraceId(const int32_t userId)
{
    return "";
}

void CloudFileKit::Release(int32_t userId) {}
} // namespace OHOS::FileManagement::CloudFile