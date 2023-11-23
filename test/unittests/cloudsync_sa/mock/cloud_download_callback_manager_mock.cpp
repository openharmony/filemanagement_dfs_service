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

#include "ipc/cloud_download_callback_manager.h"

#include <gtest/gtest.h>

namespace OHOS::FileManagement::CloudSync {
using namespace std;

CloudDownloadCallbackManager::CloudDownloadCallbackManager()
{
    callback_ = nullptr;
}

bool CloudDownloadCallbackManager::FindDownload(const std::string path)
{
    return true;
}

void CloudDownloadCallbackManager::StartDonwload(const std::string path,
                                                 const int32_t userId,
                                                 const int64_t downloadId)
{
}

bool CloudDownloadCallbackManager::StopDonwload(const std::string path, const int32_t userId,
    DownloadProgressObj &download)
{
    return true;
}

std::vector<int64_t> CloudDownloadCallbackManager::StopAllDownloads(const int32_t userId)
{
    std::vector<int64_t> ret;
    return ret;
}

void CloudDownloadCallbackManager::RegisterCallback(const int32_t userId,
                                                    const sptr<ICloudDownloadCallback> downloadCallback)
{
}

void CloudDownloadCallbackManager::UnregisterCallback(const int32_t userId) {}

void CloudDownloadCallbackManager::OnDownloadedResult(
    const std::string path,
    std::vector<DriveKit::DKDownloadAsset> assetsToDownload,
    std::shared_ptr<DataHandler> handler,
    std::shared_ptr<DriveKit::DKContext> context,
    std::shared_ptr<const DriveKit::DKDatabase> database,
    const std::map<DriveKit::DKDownloadAsset, DriveKit::DKDownloadResult> &results,
    const DriveKit::DKError &err)
{
}

void CloudDownloadCallbackManager::OnDownloadProcess(const std::string path,
                                                     std::shared_ptr<DriveKit::DKContext> context,
                                                     DriveKit::DKDownloadAsset asset,
                                                     DriveKit::TotalSize totalSize,
                                                     DriveKit::DownloadSize downloadSize)
{
}

void CloudDownloadCallbackManager::SetBundleName(const std::string &bundleName)
{
}

void CloudDownloadCallbackManager::CheckTaskState()
{
}

void CloudDownloadCallbackManager::NotifyProcessStop(DownloadProgressObj &download)
{
}
} // namespace OHOS::FileManagement::CloudSync
