/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "dk_assets_downloader.h"
#include "dk_database.h"

namespace DriveKit {
DKLocalErrorCode DKAssetsDownloader::DownLoadAssets(
    std::shared_ptr<DKContext> context,
    std::vector<DKDownloadAsset> &assetsToDownload,
    DKAssetPath downLoadPath,
    DKDownloadId &id,
    std::function<void(std::shared_ptr<DKContext>,
                       std::shared_ptr<const DKDatabase>,
                       const std::map<DKDownloadAsset, DKDownloadResult> &,
                       const DKError &)> resultCallback,
    std::function<void(std::shared_ptr<DKContext>, DKDownloadAsset, TotalSize, DownloadSize)> progressCallback,
    bool isPriority)
{
    return DKLocalErrorCode::NO_ERROR;
}

DKLocalErrorCode DKAssetsDownloader::DownLoadAssets(DKDownloadAsset &assetsToDownload)
{
    return DKLocalErrorCode::NO_ERROR;
}

DKLocalErrorCode DKAssetsDownloader::DownLoadAssets(const std::vector<DriveKit::DKDownloadAsset> &assetsToDownload,
    std::vector<DriveKit::DKDownloadResult> &result)
{
    return DKLocalErrorCode::NO_ERROR;
}

DKLocalErrorCode DKAssetsDownloader::CancelDownloadAssets(DKDownloadId id, bool isClearCache)
{
    return DKLocalErrorCode::NO_ERROR;
}
} // namespace DriveKit
