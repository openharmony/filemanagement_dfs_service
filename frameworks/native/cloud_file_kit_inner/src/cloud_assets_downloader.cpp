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
#include "cloud_assets_downloader.h"
#include "cloud_info.h"
#include "dfs_error.h"

namespace OHOS::FileManagement::CloudFile {
using namespace std;

CloudAssetsDownloader::CloudAssetsDownloader(const int32_t userId, const string &bundleName)
    : userId_(userId), bundleName_(bundleName)
{
}

int32_t CloudAssetsDownloader::DownloadAssets(DownloadAssetInfo &assetsToDownload)
{
    (void)userId_;
    (void)bundleName_;
    return E_OK;
}

int32_t CloudAssetsDownloader::DownloadAssets(const std::vector<DownloadAssetInfo> &assetsToDownload,
                                              std::vector<bool> &assetResultMap)
{
    return E_OK;
}

} // namespace OHOS::FileManagement::CloudFile