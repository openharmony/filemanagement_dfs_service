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

#include "cloud_sync_core.h"

#include <sys/types.h>
#include <sys/xattr.h>

#include "cloud_sync_manager.h"
#include "dfs_error.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
using namespace std;
const int32_t E_PARAMS = 401;

const string &CloudSyncCore::GetBundleName() const
{
    static const string emptyString = "";
    return (bundleEntity) ? bundleEntity->bundleName_ : emptyString;
}

FsResult<CloudSyncCore *> CloudSyncCore::Constructor(const optional<string> &bundleName)
{
    CloudSyncCore *cloudSyncPtr = nullptr;
    if (bundleName.has_value()) {
        const string &name = *bundleName;
        if (name == "") {
            LOGE("Failed to get bundle name");
            return FsResult<CloudSyncCore *>::Error(E_PARAMS);
        }
        cloudSyncPtr = new CloudSyncCore(name);
    } else {
        cloudSyncPtr = new CloudSyncCore();
    }

    if (cloudSyncPtr == nullptr) {
        LOGE("Failed to create CloudSyncCore object on heap.");
        return FsResult<CloudSyncCore *>::Error(ENOMEM);
    }

    return FsResult<CloudSyncCore *>::Success(move(cloudSyncPtr));
}

CloudSyncCore::CloudSyncCore(const string &bundleName)
{
    LOGI("init with bundle name");
    bundleEntity = make_unique<BundleEntity>(bundleName);
}

CloudSyncCore::CloudSyncCore()
{
    LOGI("init without bundle name");
    bundleEntity = nullptr;
}

FsResult<void> CloudSyncCore::DoOn(const string &event, const shared_ptr<CloudSyncCallbackMiddle> callback)
{
    if (event != "progress") {
        LOGE("event is not progress.");
        return FsResult<void>::Error(E_PARAMS);
    }

    if (callback_ != nullptr) {
        LOGI("callback already exist");
        return FsResult<void>::Success();
    }

    string bundleName = GetBundleName();
    callback_ = callback;
    int32_t ret = CloudSyncManager::GetInstance().RegisterCallback(callback_, bundleName);
    if (ret != E_OK) {
        LOGE("DoOn Register error, result: %{public}d", ret);
        return FsResult<void>::Error(Convert2ErrNum(ret));
    }

    return FsResult<void>::Success();
}

FsResult<void> CloudSyncCore::DoOff(const string &event, const optional<shared_ptr<CloudSyncCallbackMiddle>> &callback)
{
    if (event != "progress") {
        LOGE("event is not progress");
        return FsResult<void>::Error(E_PARAMS);
    }

    string bundleName = GetBundleName();
    int32_t ret = CloudSyncManager::GetInstance().UnRegisterCallback(bundleName);
    if (ret != E_OK) {
        LOGE("DoOff UnRegister error, result: %{public}d", ret);
        return FsResult<void>::Error(Convert2ErrNum(ret));
    }

    if (callback_ != nullptr) {
        /* delete callback */
        callback_->DeleteReference();
        callback_ = nullptr;
    }

    return FsResult<void>::Success();
}

FsResult<void> CloudSyncCore::DoStart()
{
    string bundleName = GetBundleName();
    int32_t ret = CloudSyncManager::GetInstance().StartSync(bundleName);
    if (ret != E_OK) {
        LOGE("Start Sync error, result: %{public}d", ret);
        return FsResult<void>::Error(Convert2ErrNum(ret));
    }

    return FsResult<void>::Success();
}

FsResult<void> CloudSyncCore::DoStop()
{
    string bundleName = GetBundleName();
    int32_t ret = CloudSyncManager::GetInstance().StopSync(bundleName);
    if (ret != E_OK) {
        LOGE("Stop Sync error, result: %{public}d", ret);
        return FsResult<void>::Error(Convert2ErrNum(ret));
    }

    return FsResult<void>::Success();
}

FsResult<int64_t> CloudSyncCore::CoreGetLastSyncTime()
{
    LOGI("Start begin");
    int64_t time = 0;
    string bundleName = GetBundleName();
    int32_t ret = CloudSyncManager::GetInstance().GetSyncTime(time, bundleName);
    if (ret != E_OK) {
        LOGE("GetLastSyncTime error, result: %{public}d", ret);
        return FsResult<int64_t>::Error(Convert2ErrNum(ret));
    }
    LOGI("Start GetLastSyncTime Success!");
    return FsResult<int64_t>::Success(time);
}
} // namespace OHOS::FileManagement::CloudSync
