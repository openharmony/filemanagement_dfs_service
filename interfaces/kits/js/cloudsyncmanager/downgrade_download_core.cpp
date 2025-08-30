/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "downgrade_download_core.h"

#include "cloud_sync_manager.h"
#include "dfs_error.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
using namespace std;
 

const string &DowngradeDownloadCore::GetBundleName() const
{
    static const string emptyString = "";
    return (bundleEntity) ? bundleEntity->bundleName : emptyString;
}

DowngradeDownloadCore::DowngradeDownloadCore(const string &bundleName)
{
    LOGI("init with bundle name");
    bundleEntity = make_unique<DowngradeEntity>(bundleName);
}

FsResult<DowngradeDownloadCore *> DowngradeDownloadCore::Constructor(const string &bundleName)
{
    if (bundleName.empty()) {
        LOGE("Failed to get bundle name");
        return FsResult<DowngradeDownloadCore *>::Error(EINVAL);
    }
    DowngradeDownloadCore *downgradeDlPtr = new DowngradeDownloadCore(bundleName);
    if (downgradeDlPtr == nullptr) {
        LOGE("Failed to create DowngradeDownloadCore object on heap.");
        return FsResult<DowngradeDownloadCore *>::Error(ENOMEM);
    }

    return FsResult<DowngradeDownloadCore *>::Success(move(downgradeDlPtr));
}

FsResult<CloudFileInfo> DowngradeDownloadCore::DoDowngradeDlGetCloudFileInfo()
{
    LOGI("DoDowngradeDlGetCloudFileInfo begin.");
    auto fileInfo = make_shared<CloudFileInfo>();
    string bundleName = GetBundleName();
    int32_t ret = CloudSyncManager::GetInstance().GetCloudFileInfo(bundleName, *fileInfo);
    if (ret != E_OK) {
        LOGE("DoDowngradeDlGetCloudFileInfo failed, ret = %{public}d", ret);
        return FsResult<CloudFileInfo>::Error(Convert2ErrNum(ret));
    }
    CloudFileInfo &info = *fileInfo;
    return FsResult<CloudFileInfo>::Success(info);
}

FsResult<void> DowngradeDownloadCore::DoDowngradeDlStartDownload(
    const shared_ptr<DowngradeCallbackAniImpl> callback)
{
    LOGI("DoDowngradeDlStartDownload begin.");
    string bundleName = GetBundleName();
    int32_t ret = CloudSyncManager::GetInstance().StartDowngrade(bundleName, callback);
    if (ret != E_OK) {
        LOGE("DoDowngradeDlStartDownload failed, ret = %{public}d", ret);
        return FsResult<void>::Error(Convert2ErrNum(ret));
    }
    return FsResult<void>::Success();
}

FsResult<void> DowngradeDownloadCore::DoDowngradeDlStopDownload()
{
    LOGI("DoDowngradeDlStopDownload begin.");
    string bundleName = GetBundleName();
    int32_t ret = CloudSyncManager::GetInstance().StopDowngrade(bundleName);
    if (ret != E_OK) {
        LOGE("DoDowngradeDlStopDownload failed, ret = %{public}d", ret);
        return FsResult<void>::Error(Convert2ErrNum(ret));
    }
    return FsResult<void>::Success();
}
} // namespace OHOS::FileManagement::CloudSync