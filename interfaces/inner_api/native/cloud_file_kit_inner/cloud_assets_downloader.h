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

#ifndef OHOS_CLOUD_FILE_CLOUD_ASSETS_DOWNLOADER_H
#define OHOS_CLOUD_FILE_CLOUD_ASSETS_DOWNLOADER_H

#include <string>
#include <vector>

#include "cloud_info.h"

namespace OHOS::FileManagement::CloudFile {
class CloudAssetsDownloader {
public:
    CloudAssetsDownloader(const int32_t userId, const std::string &bundleName);
    virtual ~CloudAssetsDownloader() = default;
    virtual int32_t DownloadAssets(DownloadAssetInfo &assetsToDownload);
    virtual int32_t DownloadAssets(const std::vector<DownloadAssetInfo> &assetsToDownload,
                                   std::vector<bool> &assetResultMap);

protected:
    int32_t userId_;
    std::string bundleName_;
};
} // namespace OHOS::FileManagement::CloudFile

#endif // OHOS_CLOUD_FILE_CLOUD_ASSETS_DOWNLOADER_H