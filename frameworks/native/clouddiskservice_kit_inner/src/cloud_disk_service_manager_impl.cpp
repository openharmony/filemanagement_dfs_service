/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "cloud_disk_service_manager_impl.h"

#include "cloud_disk_service_callback_client.h"
#include "cloud_disk_service_error.h"
#include "cloud_file_utils.h"
#include "iservice_registry.h"
#include "service_proxy.h"
#include "system_ability_definition.h"
#include "utils_directory.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudDiskService {
using namespace std;
CloudDiskServiceManagerImpl &CloudDiskServiceManagerImpl::GetInstance()
{
    static CloudDiskServiceManagerImpl instance;
    return instance;
}

int32_t CloudDiskServiceManagerImpl::RegisterSyncFolderChanges(const std::string &syncFolder,
                                                               const std::shared_ptr<CloudDiskServiceCallback> callback)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    if (!callback) {
        LOGE("callback is null");
        return E_INVALID_ARG;
    }
    auto serviceProxy = ServiceProxy::GetInstance();
    if (!serviceProxy) {
        LOGE("proxy is null");
        return E_IPC_FAILED;
    }
    auto ret = serviceProxy->RegisterSyncFolderChangesInner(
        syncFolder, sptr(new (std::nothrow) CloudDiskServiceCallbackClient(callback)));
    {
        unique_lock<mutex> lock(callbackMutex_);
        callback_ = callback;
    }
    SetDeathRecipient(serviceProxy->AsObject());
    LOGI("RegisterSyncFolderChanges ret %{public}d", ret);
    return ret;
#else
    return E_NOT_SUPPORTED;
#endif
}

int32_t CloudDiskServiceManagerImpl::UnregisterSyncFolderChanges(const std::string &syncFolder)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    auto serviceProxy = ServiceProxy::GetInstance();
    if (!serviceProxy) {
        LOGE("proxy is null");
        return E_IPC_FAILED;
    }

    auto ret = serviceProxy->UnregisterSyncFolderChangesInner(syncFolder);
    {
        unique_lock<mutex> lock(callbackMutex_);
        callback_ = nullptr;
    }
    SetDeathRecipient(serviceProxy->AsObject());
    LOGI("UnregisterSyncFolderChanges ret %{public}d", ret);
    return ret;
#else
    return E_NOT_SUPPORTED;
#endif
}

int32_t CloudDiskServiceManagerImpl::GetSyncFolderChanges(const std::string &syncFolder,
                                                          uint64_t count,
                                                          uint64_t startUsn,
                                                          ChangesResult &changesResult)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    auto serviceProxy = ServiceProxy::GetInstance();
    if (!serviceProxy) {
        LOGE("proxy is null");
        return E_IPC_FAILED;
    }

    auto ret = serviceProxy->GetSyncFolderChangesInner(syncFolder, count, startUsn, changesResult);
    SetDeathRecipient(serviceProxy->AsObject());
    LOGI("GetSyncFolderChangesInner ret %{public}d", ret);
    return ret;
#else
    return E_NOT_SUPPORTED;
#endif
}

int32_t CloudDiskServiceManagerImpl::SetFileSyncStates(const std::string &syncFolder,
                                                       const std::vector<FileSyncState> &fileSyncStates,
                                                       std::vector<FailedList> &failedList)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    LOGI("start SetXattr in impl");

    auto serviceProxy = ServiceProxy::GetInstance();
    if (!serviceProxy) {
        LOGE("proxy is null");
        return E_IPC_FAILED;
    }

    auto ret = serviceProxy->SetFileSyncStatesInner(syncFolder, fileSyncStates, failedList);
    SetDeathRecipient(serviceProxy->AsObject());
    LOGI("SetFileSyncState, ret %{public}d", ret);
    return ret;
#else
    return E_NOT_SUPPORTED;
#endif
}

int32_t CloudDiskServiceManagerImpl::GetFileSyncStates(const std::string &syncFolder,
                                                       const std::vector<std::string> &pathArray,
                                                       std::vector<ResultList> &resultList)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    LOGI("start GetXattr in impl");

    auto serviceProxy = ServiceProxy::GetInstance();
    if (!serviceProxy) {
        LOGE("proxy is null");
        return E_IPC_FAILED;
    }

    auto ret = serviceProxy->GetFileSyncStatesInner(syncFolder, pathArray, resultList);
    SetDeathRecipient(serviceProxy->AsObject());
    LOGI("GetFileSyncState, ret %{public}d", ret);
    return ret;
#else
    return E_NOT_SUPPORTED;
#endif
}

int32_t CloudDiskServiceManagerImpl::RegisterSyncFolder(int32_t userId,
                                                        const std::string &bundleName,
                                                        const std::string &path)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    LOGI("start RegisterSyncFolder in impl");
    auto serviceProxy = ServiceProxy::GetInstance();
    if (!serviceProxy) {
        LOGE("proxy is null");
        return E_IPC_FAILED;
    }

    auto ret = serviceProxy->RegisterSyncFolderInner(userId, bundleName, path);
    SetDeathRecipient(serviceProxy->AsObject());
    LOGI("RegisterSyncFolder, ret %{public}d", ret);
    return ret;
#else
    return E_NOT_SUPPORTED;
#endif
}

int32_t CloudDiskServiceManagerImpl::UnregisterSyncFolder(int32_t userId,
                                                          const std::string &bundleName,
                                                          const std::string &path)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    LOGI("start UnregisterSyncFolder in impl");
    auto serviceProxy = ServiceProxy::GetInstance();
    if (!serviceProxy) {
        LOGE("proxy is null");
        return E_IPC_FAILED;
    }

    auto ret = serviceProxy->UnregisterSyncFolderInner(userId, bundleName, path);
    SetDeathRecipient(serviceProxy->AsObject());
    LOGI("UnregisterSyncFolder, ret %{public}d", ret);
    return ret;
#else
    return E_NOT_SUPPORTED;
#endif
}

int32_t CloudDiskServiceManagerImpl::UnregisterForSa(const std::string &path)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    LOGI("start UnregisterForSa in impl");
    auto serviceProxy = ServiceProxy::GetInstance();
    if (!serviceProxy) {
        LOGE("proxy is null");
        return E_IPC_FAILED;
    }

    auto ret = serviceProxy->UnregisterForSaInner(path);
    SetDeathRecipient(serviceProxy->AsObject());
    LOGI("UnregisterForSa, ret %{public}d", ret);
    return ret;
#else
    return E_NOT_SUPPORTED;
#endif
}

void CloudDiskServiceManagerImpl::SetDeathRecipient(const sptr<IRemoteObject> &remoteObject)
{
    if (!isFirstCall_.test_and_set()) {
        auto deathCallback = [this](const wptr<IRemoteObject> &obj) {
            LOGE("service died.");
            ServiceProxy::InvalidInstance();
            if (callback_) {
                callback_->OnDeathRecipient();
            }
            isFirstCall_.clear();
        };
        deathRecipient_ = sptr(new SvcDeathRecipient(deathCallback));
        if (!remoteObject->AddDeathRecipient(deathRecipient_)) {
            LOGE("add death recipient failed");
            isFirstCall_.clear();
        }
    }
}

} // namespace OHOS::FileManagement::CloudDiskService
