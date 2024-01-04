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

#ifndef OHOS_FILEMGMT_CLOUD_DOWNLOAD_CALLBACK_MANAGER_H
#define OHOS_FILEMGMT_CLOUD_DOWNLOAD_CALLBACK_MANAGER_H

#include <memory>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include "dk_assets_downloader.h"

#include "i_cloud_download_callback.h"
#include "data_handler.h"
#include "sdk_helper.h"

namespace OHOS::FileManagement::CloudSync {

class CloudDownloadCallbackManager {
public:
    CloudDownloadCallbackManager();
    ~CloudDownloadCallbackManager() = default;

    bool FindDownload(const std::string path);
    void StartDonwload(const std::string path, const int32_t userId, const int64_t downloadId);
    bool StopDonwload(const std::string path, const int32_t userId, DownloadProgressObj &download);
    std::vector<int64_t> StopAllDownloads(const int32_t userId);
    void NotifyProcessStop(DownloadProgressObj &download);
    void RegisterCallback(const int32_t userId, const sptr<ICloudDownloadCallback> downloadCallback);
    void UnregisterCallback(const int32_t userId);
    void OnDownloadedResult(const std::string path,
                            std::vector<DriveKit::DKDownloadAsset> assetsToDownload,
                            std::shared_ptr<DataHandler> handler,
                            std::shared_ptr<DriveKit::DKContext> context,
                            std::shared_ptr<const DriveKit::DKDatabase> database,
                            const std::map<DriveKit::DKDownloadAsset, DriveKit::DKDownloadResult> &results,
                            const DriveKit::DKError &err);
    void OnDownloadProcess(const std::string path,
                           std::shared_ptr<DriveKit::DKContext> context,
                           DriveKit::DKDownloadAsset asset,
                           DriveKit::TotalSize totalSize,
                           DriveKit::DownloadSize downloadSize);
    void SetBundleName(const std::string &bundleName);

private:
    std::mutex downloadsMtx_;
    std::mutex callbackMutex_;
    std::unordered_map<std::string, DownloadProgressObj> downloads_;
    sptr<ICloudDownloadCallback> callback_;
    std::string bundleName_;
    void CheckTaskState();
};
} // namespace OHOS::FileManagement::CloudSync

#endif // OHOS_FILEMGMT_CLOUD_DOWNLOAD_CALLBACK_MANAGER_H
