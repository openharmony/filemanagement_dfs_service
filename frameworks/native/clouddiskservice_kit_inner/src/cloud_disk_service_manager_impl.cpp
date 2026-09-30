/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include <new>

#include "cloud_disk_service_callback_client.h"
#include "cloud_disk_service_callback_table_client.h"
#include "cloud_disk_service_error.h"
#ifdef SUPPORT_CLOUD_DISK_SERVICE
#include "cloud_disk_sync_folder_manager.h"
#endif
#include "cloud_file_utils.h"
#include "iservice_registry.h"
#include "service_proxy.h"
#include "system_ability_definition.h"
#include "utils_directory.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudDiskService {
using namespace std;
#ifdef SUPPORT_CLOUD_DISK_SERVICE
static int32_t CheckSyncFolderRegistered()
{
    std::vector<FileManagement::SyncFolderExt> syncFolders;
    int32_t ret = FileManagement::CloudDiskSyncFolderManager::GetInstance().GetAllSyncFoldersForSa(syncFolders);
    if (ret != E_OK) {
        LOGE("Check sync folder failed: query returned %{public}d", ret);
        return E_SYNC_FOLDER_NOT_REGISTERED;
    }
    if (syncFolders.empty()) {
        LOGW("Check sync folder failed: no sync folder registered");
        return E_SYNC_FOLDER_NOT_REGISTERED;
    }
    return E_OK;
}
#endif

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

int32_t CloudDiskServiceManagerImpl::RegisterCallbackTable(
    const std::string &syncFolder,
    const std::shared_ptr<CloudDiskServiceCallbackTable> &callbackTable)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    if (callbackTable == nullptr) {
        LOGE("Callback table is nullptr");
        return E_INVALID_ARG;
    }
    auto serviceProxy = ServiceProxy::GetInstance();
    if (serviceProxy == nullptr) {
        LOGE("Proxy is nullptr");
        return E_IPC_FAILED;
    }
    auto callbackClient = sptr(new (std::nothrow) CloudDiskServiceCallbackTableClient(callbackTable));
    if (callbackClient == nullptr) {
        LOGE("Failed to allocate callback table client");
        return E_TRY_AGAIN;
    }
    int32_t ret = serviceProxy->RegisterCallbackTableInner(syncFolder, callbackClient);
    if (ret == E_OK) {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        callbackTables_[syncFolder] = callbackTable;
        callbackTableClients_[syncFolder] = callbackClient;
    }
    SetDeathRecipient(serviceProxy->AsObject());
    LOGI("RegisterCallbackTable ret %{public}d", ret);
    return ret;
#else
    return E_NOT_SUPPORTED;
#endif
}

int32_t CloudDiskServiceManagerImpl::UnregisterCallbackTable(const std::string &syncFolder)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    auto serviceProxy = ServiceProxy::GetInstance();
    if (serviceProxy == nullptr) {
        LOGE("Proxy is nullptr");
        return E_IPC_FAILED;
    }
    sptr<CloudDiskServiceCallbackTableClient> callbackClient;
    {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        auto item = callbackTableClients_.find(syncFolder);
        if (item != callbackTableClients_.end()) {
            callbackClient = item->second;
            callbackClient->SetActive(false);
        }
    }
    int32_t ret = serviceProxy->UnregisterCallbackTableInner(syncFolder);
    if (ret == E_OK) {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        callbackTables_.erase(syncFolder);
        callbackTableClients_.erase(syncFolder);
    } else if (callbackClient != nullptr) {
        callbackClient->SetActive(true);
    }
    SetDeathRecipient(serviceProxy->AsObject());
    LOGI("UnregisterCallbackTable ret %{public}d", ret);
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

int32_t CloudDiskServiceManagerImpl::CreatePlaceholderFile(const std::string &syncFolder,
                                                           const std::string &relativePath,
                                                           const PlaceholderInfo &info,
                                                           const PlaceholderCustomInfo &customInfo)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    LOGI("CreatePlaceholderFile route=manager_to_proxy");
    auto serviceProxy = ServiceProxy::GetInstance();
    if (!serviceProxy) {
        LOGE("proxy is null");
        return E_IPC_FAILED;
    }

    auto ret = serviceProxy->CreatePlaceholderFileInner(syncFolder, relativePath, info, customInfo);
    SetDeathRecipient(serviceProxy->AsObject());
    if (ret != E_OK) {
        LOGE("CreatePlaceholderFile branch=proxy_failed ret=%{public}d", ret);
    } else {
        LOGI("CreatePlaceholderFile branch=proxy_success");
    }
    return ret;
#else
    return E_NOT_SUPPORTED;
#endif
}

int32_t CloudDiskServiceManagerImpl::IsPlaceholderFile(const std::string &syncFolder,
                                                       const std::string &path,
                                                       bool &isPlaceholder)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    LOGI("IsPlaceholderFile route=manager_to_proxy");

    auto serviceProxy = ServiceProxy::GetInstance();
    if (!serviceProxy) {
        LOGE("IsPlaceholderFile branch=proxy_null");
        return E_IPC_FAILED;
    }

    auto ret = serviceProxy->IsPlaceholderFileInner(syncFolder, path, isPlaceholder);
    SetDeathRecipient(serviceProxy->AsObject());
    if (ret != E_OK) {
        LOGE("IsPlaceholderFile branch=proxy_failed ret=%{public}d", ret);
    } else {
        LOGI("IsPlaceholderFile branch=proxy_success isPlaceholder=%{public}d", isPlaceholder);
    }
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

int32_t CloudDiskServiceManagerImpl::ConvertPlaceholderToFile(const std::string &syncFolder,
                                                              const std::string &relativePath)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    LOGI("start ConvertPlaceholderToFile in impl");
    auto serviceProxy = ServiceProxy::GetInstance();
    if (!serviceProxy) {
        LOGE("proxy is null");
        return E_IPC_FAILED;
    }

    SetDeathRecipient(serviceProxy->AsObject());
    auto ret = serviceProxy->ConvertPlaceholderToFileInner(syncFolder, relativePath);
    LOGI("ConvertPlaceholderToFile, ret %{public}d", ret);
    return ret;
#else
    return E_NOT_SUPPORTED;
#endif
}

int32_t CloudDiskServiceManagerImpl::MarkFileAsPlaceholder(const std::string &syncFolder,
                                                           const std::string &relativePath)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    LOGI("start MarkFileAsPlaceholder in impl");
    auto serviceProxy = ServiceProxy::GetInstance();
    if (!serviceProxy) {
        LOGE("proxy is null");
        return E_IPC_FAILED;
    }

    SetDeathRecipient(serviceProxy->AsObject());
    int32_t ret = serviceProxy->MarkFileAsPlaceholderInner(syncFolder, relativePath);
    LOGI("MarkFileAsPlaceholder, ret %{public}d", ret);
    return ret;
#else
    return E_NOT_SUPPORTED;
#endif
}

int32_t CloudDiskServiceManagerImpl::UnmarkPlaceholderFile(const std::string &syncFolder,
                                                           const std::string &relativePath)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    LOGI("start UnmarkPlaceholderFile in impl");
    auto serviceProxy = ServiceProxy::GetInstance();
    if (!serviceProxy) {
        LOGE("proxy is null");
        return E_IPC_FAILED;
    }

    SetDeathRecipient(serviceProxy->AsObject());
    int32_t ret = serviceProxy->UnmarkPlaceholderFileInner(syncFolder, relativePath);
    LOGI("UnmarkPlaceholderFile, ret %{public}d", ret);
    return ret;
#else
    return E_NOT_SUPPORTED;
#endif
}

int32_t CloudDiskServiceManagerImpl::StartHydration(const std::string &syncFolder,
                                                    const std::string &relativePath,
                                                    CloudDiskHydratePriority priority)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    auto serviceProxy = ServiceProxy::GetInstance();
    if (!serviceProxy) {
        LOGE("proxy is null");
        return E_IPC_FAILED;
    }
    SetDeathRecipient(serviceProxy->AsObject());
    int32_t ret = serviceProxy->StartHydrationInner(syncFolder, relativePath, static_cast<int32_t>(priority));
    if (ret != E_OK) {
        LOGW("StartHydration failed, ret:%{public}d", ret);
    }
    return ret;
#else
    return E_NOT_SUPPORTED;
#endif
}

int32_t CloudDiskServiceManagerImpl::CancelHydration(const std::string &syncFolder, const std::string &relativePath)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    auto serviceProxy = ServiceProxy::GetInstance();
    if (!serviceProxy) {
        LOGE("proxy is null");
        return E_IPC_FAILED;
    }
    SetDeathRecipient(serviceProxy->AsObject());
    int32_t ret = serviceProxy->CancelHydrationInner(syncFolder, relativePath);
    if (ret != E_OK) {
        LOGW("CancelHydration failed, ret:%{public}d", ret);
    }
    return ret;
#else
    return E_NOT_SUPPORTED;
#endif
}

int32_t CloudDiskServiceManagerImpl::Execute(const CallbackExecuteRequest &request)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    auto serviceProxy = ServiceProxy::GetInstance();
    if (!serviceProxy) {
        LOGE("proxy is null");
        return E_IPC_FAILED;
    }
    SetDeathRecipient(serviceProxy->AsObject());
    int32_t ret = serviceProxy->ExecuteInner(request);
    if (ret != E_OK) {
        LOGW("Execute failed, ret:%{public}d", ret);
    }
    return ret;
#else
    return E_NOT_SUPPORTED;
#endif
}

int32_t CloudDiskServiceManagerImpl::DehydrateFile(const std::string &syncFolder, const std::string &relativePath)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    LOGI("start DehydrateFile in impl");
    auto serviceProxy = ServiceProxy::GetInstance();
    if (!serviceProxy) {
        LOGE("proxy is null");
        return E_IPC_FAILED;
    }

    SetDeathRecipient(serviceProxy->AsObject());
    int32_t ret = serviceProxy->DehydrateInner(syncFolder, relativePath);
    LOGI("DehydrateFile, ret %{public}d", ret);
    return ret;
#else
    return E_NOT_SUPPORTED;
#endif
}

int32_t CloudDiskServiceManagerImpl::UpdatePlaceholder(const std::string &syncFolder,
                                                       const std::string &relativePath,
                                                       const PlaceholderInfo &metaData,
                                                       const PlaceholderCustomInfo &customInfo)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    LOGI("start UpdatePlaceholder in impl");
    auto serviceProxy = ServiceProxy::GetInstance();
    if (!serviceProxy) {
        LOGE("proxy is null");
        return E_IPC_FAILED;
    }

    SetDeathRecipient(serviceProxy->AsObject());
    auto ret = serviceProxy->UpdatePlaceholderInner(syncFolder, relativePath, metaData, customInfo);
    LOGI("UpdatePlaceholder, ret %{public}d", ret);
    return ret;
#else
    return E_NOT_SUPPORTED;
#endif
}

int32_t CloudDiskServiceManagerImpl::GetPlaceholderCustomInfo(const std::string &syncFolder,
                                                              const std::string &relativePath,
                                                              PlaceholderCustomInfo &customInfo)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    LOGI("start GetPlaceholderCustomInfo in impl");
    auto serviceProxy = ServiceProxy::GetInstance();
    if (!serviceProxy) {
        LOGE("proxy is null");
        return E_IPC_FAILED;
    }

    SetDeathRecipient(serviceProxy->AsObject());
    auto ret = serviceProxy->GetPlaceholderCustomInfoInner(syncFolder, relativePath, customInfo);
    LOGI("GetPlaceholderCustomInfo, ret %{public}d", ret);
    return ret;
#else
    return E_NOT_SUPPORTED;
#endif
}

int32_t CloudDiskServiceManagerImpl::StartHydrationByPath(
    const std::string &path, int32_t callbackType, int32_t priority, uint64_t accessorId)
{
    LOGI("StartHydrationByPath begin, accessorId:%{private}llu",
         static_cast<unsigned long long>(accessorId));
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    int32_t ret = CheckSyncFolderRegistered();
    if (ret != E_OK) {
        return ret;
    }
    auto proxy = ServiceProxy::GetInstance();
    if (proxy == nullptr) {
        LOGE("Start hydration for accessor failed: proxy is null");
        return E_IPC_FAILED;
    }
    SetDeathRecipient(proxy->AsObject());
    ret = proxy->StartHydrationByPathInner(path, callbackType, priority, accessorId);
    LOGI("StartHydrationByPath completed, ret:%{public}d", ret);
    return ret;
#else
    return E_NOT_SUPPORTED;
#endif
}

int32_t CloudDiskServiceManagerImpl::DehydrateFileByPath(const std::string &path)
{
    LOGI("DehydrateFileByPath begin, path:%{private}s", path.c_str());
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    int32_t ret = CheckSyncFolderRegistered();
    if (ret != E_OK) {
        return ret;
    }
    auto proxy = ServiceProxy::GetInstance();
    if (proxy == nullptr) {
        LOGE("DehydrateFileByPath failed: proxy is null, ret:%{public}d", static_cast<int32_t>(E_IPC_FAILED));
        return E_IPC_FAILED;
    }
    SetDeathRecipient(proxy->AsObject());
    ret = proxy->DehydrateFileByPathInner(path);
    LOGI("DehydrateFileByPath completed, ret:%{public}d", ret);
    return ret;
#else
    return E_NOT_SUPPORTED;
#endif
}

int32_t CloudDiskServiceManagerImpl::RegisterProgressCallback(
    uint64_t accessorId, const sptr<ICloudDiskProgressCallback> &callback)
{
    LOGI("RegisterProgressCallback begin, accessorId:%{private}llu",
         static_cast<unsigned long long>(accessorId));
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    if (callback == nullptr) {
        LOGE("Register accessor progress failed: callback is null");
        return E_INVALID_ARG;
    }
    int32_t ret = CheckSyncFolderRegistered();
    if (ret != E_OK) {
        return ret;
    }
    std::lock_guard<std::mutex> lock(progressMutex_);
    auto proxy = ServiceProxy::GetInstance();
    if (proxy == nullptr) {
        LOGE("Register accessor progress failed: proxy is null");
        return E_IPC_FAILED;
    }
    if (progressClient_ == nullptr) {
        progressClient_ = sptr(new (std::nothrow) CloudDiskProgressCallbackClient());
    }
    if (progressClient_ == nullptr) {
        LOGE("Register accessor progress failed: allocate broker failed");
        return E_TRY_AGAIN;
    }
    bool added = false;
    ret = progressClient_->Add(accessorId, callback, added);
    if (ret != E_OK) {
        LOGE("Register accessor progress failed: local registration returned %{public}d", ret);
        return ret;
    }
    SetDeathRecipient(proxy->AsObject());
    ret = proxy->RegisterProgressCallbackInner(accessorId, progressClient_->AsObject());
    if (ret != E_OK && added) {
        progressClient_->Remove(accessorId, callback);
        LOGW("Roll back accessor progress registration, ret:%{public}d", ret);
    }
    LOGI("RegisterProgressCallback completed, ret:%{public}d", ret);
    return ret;
#else
    return E_NOT_SUPPORTED;
#endif
}

int32_t CloudDiskServiceManagerImpl::UnregisterProgressCallback(
    uint64_t accessorId, const sptr<ICloudDiskProgressCallback> &callback)
{
    LOGI("UnregisterProgressCallback begin, accessorId:%{private}llu",
         static_cast<unsigned long long>(accessorId));
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    if (callback == nullptr) {
        LOGE("Unregister accessor progress failed: callback is null");
        return E_INVALID_ARG;
    }
    std::lock_guard<std::mutex> lock(progressMutex_);
    if (progressClient_ == nullptr || !progressClient_->Remove(accessorId, callback)) {
        LOGW("Unregister accessor progress failed: matching callback is not registered");
        return E_CALLBACK_NOT_REGISTERED;
    }
    int32_t ret = CheckSyncFolderRegistered();
    if (ret != E_OK) {
        LOGE("Unregister accessor progress failed after local removal, ret:%{public}d", ret);
        return ret;
    }
    auto proxy = ServiceProxy::GetInstance();
    if (proxy == nullptr) {
        LOGE("Unregister accessor progress failed after local removal: proxy is null");
        return E_IPC_FAILED;
    }
    ret = proxy->UnregisterProgressCallbackInner(accessorId);
    LOGI("UnregisterProgressCallback completed, ret:%{public}d", ret);
    return ret;
#else
    LOGW("Unregister accessor progress is not supported");
    return E_NOT_SUPPORTED;
#endif
}

int32_t CloudDiskServiceManagerImpl::GetPlaceholderState(const std::string &syncFolder,
                                                         const std::string &relativePath,
                                                         int32_t &state)
{
    state = 0;
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    auto serviceProxy = ServiceProxy::GetInstance();
    if (serviceProxy == nullptr) {
        LOGE("GetPlaceholderState branch=proxy_null");
        return E_IPC_FAILED;
    }
    SetDeathRecipient(serviceProxy->AsObject());
    int32_t ret = serviceProxy->GetPlaceholderStateInner(syncFolder, relativePath, state);
    if (ret != E_OK) {
        state = 0;
        LOGE("GetPlaceholderState branch=proxy_failed ret=%{public}d", ret);
    }
    return ret;
#else
    return E_NOT_SUPPORTED;
#endif
}

void CloudDiskServiceManagerImpl::SetDeathRecipient(const sptr<IRemoteObject> &remoteObject)
{
    if (isFirstCall_.test_and_set()) {
        return;
    }
    auto deathCallback = [this](const wptr<IRemoteObject> &obj) {
        LOGE("service died.");
        ServiceProxy::InvalidInstance();
        std::shared_ptr<CloudDiskServiceCallback> callback;
        std::vector<std::shared_ptr<CloudDiskServiceCallbackTable>> callbackTables;
        {
            std::lock_guard<std::mutex> lock(callbackMutex_);
            callback = callback_;
            for (const auto &item : callbackTableClients_) {
                if (item.second != nullptr) {
                    item.second->SetActive(false);
                }
            }
            for (const auto &item : callbackTables_) {
                callbackTables.push_back(item.second);
            }
        }
        if (callback != nullptr) {
            callback->OnDeathRecipient();
        }
        for (const auto &callbackTable : callbackTables) {
            if (callbackTable != nullptr) {
                callbackTable->OnDeathRecipient();
            }
        }
        isFirstCall_.clear();
    };
    deathRecipient_ = sptr(new SvcDeathRecipient(deathCallback));
    if (!remoteObject->AddDeathRecipient(deathRecipient_)) {
        LOGE("add death recipient failed");
        isFirstCall_.clear();
    }
}

} // namespace OHOS::FileManagement::CloudDiskService
