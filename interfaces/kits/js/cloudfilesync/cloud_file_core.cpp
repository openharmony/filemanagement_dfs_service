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

#include "cloud_file_core.h"

#include <sys/types.h>

#include "cloud_sync_manager.h"
#include "dfs_error.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
using namespace std;
const int32_t E_PARAMS = 401;

FsResult<CloudFileCore *> CloudFileCore::Constructor()
{
    CloudFileCore *cloudfile = new CloudFileCore();
    if (cloudfile == nullptr) {
        LOGE("Failed to create CloudFileCore object on heap.");
        return FsResult<CloudFileCore *>::Error(ENOMEM);
    }

    return FsResult<CloudFileCore *>::Success(move(cloudfile));
}

CloudFileCore::CloudFileCore() {}

FsResult<void> CloudFileCore::DoStart(const string &uri)
{
    LOGI("Start begin");
    int32_t ret = CloudSyncManager::GetInstance().StartDownloadFile(uri);
    if (ret != E_OK) {
        LOGE("Start Download failed! ret = %{public}d", ret);
        return FsResult<void>::Error(Convert2ErrNum(ret));
    }

    LOGI("Start Download Success!");
    return FsResult<void>::Success();
}

FsResult<void> CloudFileCore::DoOn(const string &event, const shared_ptr<CloudDownloadCallbackMiddle> callback)
{
    LOGI("On begin");
    if (event != "progress") {
        LOGE("On get progress failed!");
        return FsResult<void>::Error(E_PARAMS);
    }

    if (callback_ != nullptr) {
        LOGI("callback already exist");
        return FsResult<void>::Success();
    }

    callback_ = callback;
    int32_t ret = CloudSyncManager::GetInstance().RegisterDownloadFileCallback(callback_);
    if (ret != E_OK) {
        LOGE("RegisterDownloadFileCallback error, ret: %{public}d", ret);
        return FsResult<void>::Error(Convert2ErrNum(ret));
    }

    return FsResult<void>::Success();
}

FsResult<void> CloudFileCore::DoOff(
    const string &event, const optional<shared_ptr<CloudDownloadCallbackMiddle>> &callback)
{
    LOGI("Off begin");
    if (event != "progress") {
        LOGE("Off get progress failed!");
        return FsResult<void>::Error(E_PARAMS);
    }

    /* callback_ may be nullptr */
    int32_t ret = CloudSyncManager::GetInstance().UnregisterDownloadFileCallback();
    if (ret != E_OK) {
        LOGE("UnregisterDownloadFileCallback error, ret: %{public}d", ret);
        return FsResult<void>::Error(Convert2ErrNum(ret));
    }

    if (callback_ != nullptr) {
        /* delete callback */
        callback_->DeleteReference();
        callback_ = nullptr;
    }

    return FsResult<void>::Success();
}

FsResult<void> CloudFileCore::DoStop(const string &uri, bool needClean)
{
    LOGI("Stop begin");
    int32_t ret = CloudSyncManager::GetInstance().StopDownloadFile(uri, needClean);
    if (ret != E_OK) {
        LOGE("Stop Download failed! ret = %{public}d", ret);
        return FsResult<void>::Error(Convert2ErrNum(ret));
    }

    LOGI("Stop Download Success!");
    return FsResult<void>::Success();
}
} // namespace OHOS::FileManagement::CloudSync