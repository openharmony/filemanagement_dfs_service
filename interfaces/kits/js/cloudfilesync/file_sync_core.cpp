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

#include "file_sync_core.h"

#include <memory>
#include <sys/types.h>
#include <sys/xattr.h>

#include "cloud_file_utils.h"
#include "cloud_sync_callback_ani.h"
#include "cloud_sync_manager.h"
#include "dfs_error.h"
#include "uri.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
using namespace std;
const int32_t E_PARAMS = 401;

const string &FileSyncCore::GetBundleName() const
{
    return bundleEntity->callbackInfo.bundleName;
}

FsResult<FileSyncCore *> FileSyncCore::Constructor(const optional<string> &bundleName)
{
    FileSyncCore *fileSyncPtr = nullptr;
    if (bundleName.has_value()) {
        const string &name = *bundleName;
        if (name == "") {
            LOGE("Failed to get bundle name");
            return FsResult<FileSyncCore *>::Error(E_PARAMS);
        }
        fileSyncPtr = new FileSyncCore(name);
    } else {
        fileSyncPtr = new FileSyncCore();
    }

    if (fileSyncPtr == nullptr) {
        LOGE("Failed to create CloudSyncCore object on heap.");
        return FsResult<FileSyncCore *>::Error(ENOMEM);
    }

    return FsResult<FileSyncCore *>::Success(move(fileSyncPtr));
}

FileSyncCore::FileSyncCore(const string &bundleName)
{
    LOGI("init with bundle name");
    bundleEntity = make_unique<BundleEntity>();
    bundleEntity->callbackInfo.bundleName = bundleName;
    bundleEntity->uploadCallbackInfo.bundleName = bundleName;
}

FileSyncCore::FileSyncCore()
{
    LOGI("init without bundle name");
    bundleEntity = make_unique<BundleEntity>();
}

FsResult<void> FileSyncCore::DoOn(const string &event, const shared_ptr<CloudSyncCallbackMiddle> callback)
{
    if (event != "progress") {
        LOGE("event is not progress.");
        return FsResult<void>::Error(E_PARAMS);
    }

    if (bundleEntity->callbackInfo.callback != nullptr) {
        LOGI("callback already exist");
        return FsResult<void>::Success();
    }

    bundleEntity->callbackInfo.callbackId = CloudDisk::CloudFileUtils::GenerateUuid();
    bundleEntity->callbackInfo.callback = callback;
    int32_t ret = CloudSyncManager::GetInstance().RegisterFileSyncCallback(bundleEntity->callbackInfo);
    if (ret != E_OK) {
        LOGE("DoOn Register error, result: %{public}d", ret);
        return FsResult<void>::Error(Convert2ErrNum(ret));
    }

    return FsResult<void>::Success();
}

FsResult<void> FileSyncCore::DoOff(const string &event, const optional<shared_ptr<CloudSyncCallbackMiddle>> &callback)
{
    if (event != "progress") {
        LOGE("event is not progress");
        return FsResult<void>::Error(E_PARAMS);
    }

    int32_t ret = CloudSyncManager::GetInstance().UnRegisterFileSyncCallback(bundleEntity->callbackInfo);
    if (ret != E_OK) {
        LOGE("DoOff UnRegister error, result: %{public}d", ret);
        return FsResult<void>::Error(Convert2ErrNum(ret));
    }

    if (bundleEntity->callbackInfo.callback != nullptr) {
        /* delete callback */
        bundleEntity->callbackInfo.callback->DeleteReference();
        bundleEntity->callbackInfo.callback = nullptr;
    }

    return FsResult<void>::Success();
}

FsResult<void> FileSyncCore::DoStart()
{
    string bundleName = GetBundleName();
    int32_t ret = CloudSyncManager::GetInstance().StartFileSync(bundleName);
    if (ret != E_OK) {
        LOGE("Start Sync error, result: %{public}d", ret);
        return FsResult<void>::Error(Convert2ErrNum(ret));
    }

    return FsResult<void>::Success();
}

FsResult<void> FileSyncCore::DoStop()
{
    string bundleName = GetBundleName();
    int32_t ret = CloudSyncManager::GetInstance().StopFileSync(bundleName);
    if (ret != E_OK) {
        LOGE("Stop Sync error, result: %{public}d", ret);
        return FsResult<void>::Error(Convert2ErrNum(ret));
    }

    return FsResult<void>::Success();
}

FsResult<int64_t> FileSyncCore::DoGetLastSyncTime()
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

FsResult<void> FileSyncCore::DoRegisterUploadProgress(const shared_ptr<CloudUploadCallback> callback)
{
    if (bundleEntity == nullptr) {
        bundleEntity = make_unique<BundleEntity>();
    }
    
    if (bundleEntity->uploadCallbackInfo.callback != nullptr) {
        LOGI("upload callback already exist");
        return FsResult<void>::Success();
    }

    bundleEntity->uploadCallbackInfo.callbackId = CloudDisk::CloudFileUtils::GenerateUuid();
    bundleEntity->uploadCallbackInfo.callback = callback;
    int32_t ret = CloudSyncManager::GetInstance().RegisterUploadCallback(bundleEntity->uploadCallbackInfo);
    if (ret != E_OK) {
        LOGE("DoRegisterUploadProgress Register error, result: %{public}d", ret);
        return FsResult<void>::Error(Convert2ErrNum(ret));
    }

    return FsResult<void>::Success();
}

FsResult<void> FileSyncCore::DoUnRegisterUploadProgress()
{
    if (bundleEntity == nullptr) {
        LOGI("upload callback not exist");
        return FsResult<void>::Success();
    }
    
    int32_t ret = CloudSyncManager::GetInstance().UnRegisterUploadCallback(bundleEntity->uploadCallbackInfo);
    if (ret != E_OK) {
        LOGE("DoUnRegisterUploadProgress UnRegister error, result: %{public}d", ret);
        return FsResult<void>::Error(Convert2ErrNum(ret));
    }
    if (bundleEntity->uploadCallbackInfo.callback != nullptr) {
        bundleEntity->uploadCallbackInfo.callback->DeleteReference();
        bundleEntity->uploadCallbackInfo.callback = nullptr;
    }
    return FsResult<void>::Success();
}

FsResult<void> FileSyncCore::DoGetUploadList(const vector<string> &uriVec,
                                             vector<CloudSync::UploadProgressObj> &uploadList)
{
    LOGI("GetUploadList begin, uriVec size: %{public}zu", uriVec.size());
    int32_t ret = CloudSyncManager::GetInstance().GetUploadList(uriVec, uploadList);
    if (ret != E_OK) {
        LOGE("GetUploadList failed! ret = %{public}d", ret);
        return FsResult<void>::Error(Convert2ErrNum(ret));
    }
    LOGI("GetUploadList success, result size: %{public}zu", uploadList.size());
    return FsResult<void>::Success();
}

FsResult<void> FileSyncCore::DoPauseUpload(const string &uri)
{
    if (uri.empty()) {
        LOGE("unavailable uri.");
        return FsResult<void>::Error(Convert2ErrNum(E_ILLEGAL_URI));
    }
    int32_t ret = CloudSyncManager::GetInstance().PauseUpload(uri);
    if (ret != E_OK) {
        LOGE("PauseUpload failed! ret = %{public}d", ret);
        return FsResult<void>::Error(Convert2ErrNum(ret));
    }
    return FsResult<void>::Success();
}

FsResult<void> FileSyncCore::DoResumeUpload(const string &uri)
{
    if (uri.empty()) {
        LOGE("unavailable uri.");
        return FsResult<void>::Error(Convert2ErrNum(E_ILLEGAL_URI));
    }
    int32_t ret = CloudSyncManager::GetInstance().ResumeUpload(uri);
    if (ret != E_OK) {
        LOGE("ResumeUpload failed! ret = %{public}d", ret);
        return FsResult<void>::Error(Convert2ErrNum(ret));
    }
    return FsResult<void>::Success();
}
} // namespace OHOS::FileManagement::CloudSync