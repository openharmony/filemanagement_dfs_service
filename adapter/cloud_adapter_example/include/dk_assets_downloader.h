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

#ifndef DRIVE_KIT_ASSETS_DOWNLOADER_H
#define DRIVE_KIT_ASSETS_DOWNLOADER_H

#include <functional>
#include <map>

#include "dk_context.h"
#include "dk_error.h"
#include "dk_record.h"
#include "dk_result.h"

namespace DriveKit {
using DKDownloadId = int64_t;
using DKAssetPath = std::string;
using TotalSize = int64_t;
using DownloadSize = int64_t;
class DKDatabase;
class DKDownloadResult : public DKResult {
public:
    DKAsset GetAsset()
    {
        return asset_;
    }
    void SetAsset(DKAsset asset)
    {
        asset_ = asset;
    }

private:
    DKAsset asset_;
};
struct DKDownloadAsset {
    DKRecordType recordType;
    DKRecordId recordId;
    DKFieldKey fieldKey;
    DKAsset asset;
    DKAssetPath downLoadPath;
    bool operator<(const DKDownloadAsset &downloadAsset) const
    {
        if (recordType != downloadAsset.recordType) {
            return recordType < downloadAsset.recordType;
        }
        if (recordId != downloadAsset.recordId) {
            return recordId < downloadAsset.recordId;
        }
        return fieldKey < downloadAsset.fieldKey;
    }
};

class DKAssetsDownloader {
public:
    DKLocalErrorCode DownLoadAssets(
        std::shared_ptr<DKContext> contex,
        std::vector<DKDownloadAsset> &assetsToDownload,
        DKAssetPath downLoadPath,
        DKDownloadId &id,
        std::function<void(std::shared_ptr<DKContext>,
                           std::shared_ptr<const DKDatabase>,
                           const std::map<DKDownloadAsset, DKDownloadResult> &,
                           const DKError &)> resultCallback,
        std::function<void(std::shared_ptr<DKContext>, DKDownloadAsset, TotalSize, DownloadSize)> progressCallback,
        bool isPriority = false);
    DKLocalErrorCode DownLoadAssets(DKDownloadAsset &assetsToDownload);
    DKLocalErrorCode DownLoadAssets(const std::vector<DKDownloadAsset> &assetsToDownload,
                                    std::vector<DKDownloadResult> &result);
    DKLocalErrorCode CancelDownloadAssets(DKDownloadId id, bool isClearCache = false);
    DKAssetsDownloader(std::shared_ptr<DKDatabase> database)
    {
        database_ = database;
    };
    virtual ~DKAssetsDownloader(){};

private:
    std::shared_ptr<DKDatabase> database_;
};
} // namespace DriveKit
#endif
