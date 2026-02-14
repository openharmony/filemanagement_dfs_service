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

#include "cloud_disk_service.h"

#include <cstdlib>
#include <exception>
#include <stdexcept>
#include <sys/utsname.h>
#include <sys/xattr.h>
#include <thread>

#include "cloud_disk_service_access_token.h"
#include "cloud_disk_service_callback_manager.h"
#include "cloud_disk_service_error.h"
#include "cloud_disk_service_syncfolder.h"
#ifdef SUPPORT_CLOUD_DISK_SERVICE
#include "cloud_disk_sync_folder_manager.h"
#endif
#include "cloud_disk_sync_folder.h"
#include "iremote_object.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudDiskService {
using namespace std;

const int32_t GET_FILE_SYNC_MAX = 100;
const int32_t GET_SYNC_FOLDER_CHANGE_MAX = 100;
constexpr const char *FILE_SYNC_STATE = "user.clouddisk.filesyncstate";

namespace {
}
REGISTER_SYSTEM_ABILITY_BY_ID(CloudDiskService, FILEMANAGEMENT_CLOUD_DISK_SERVICE_SA_ID, true);

CloudDiskService::CloudDiskService(int32_t saID, bool runOnCreate) : SystemAbility(saID, runOnCreate)
{
    accountStatusListener_ = make_shared<AccountStatusListener>();
}

CloudDiskService::CloudDiskService() {}

void CloudDiskService::PublishSA()
{
    LOGI("Begin to init");
    if (!registerToService_) {
        bool ret = SystemAbility::Publish(this);
        if (!ret) {
            throw runtime_error("Failed to publish the clouddiskservice");
        }
        registerToService_ = true;
    }
    LOGI("Init finished successfully");
}

void CloudDiskService::OnStart()
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    LOGI("Begin to start service");
    if (state_ == ServiceRunningState::STATE_RUNNING) {
        LOGI("CloudDiskService has already started");
        return;
    }

    std::vector<FileManagement::SyncFolderExt> syncFolders;
    OHOS::FileManagement::CloudDiskSyncFolderManager::GetInstance().GetAllSyncFoldersForSa(syncFolders);
    int32_t userId = CloudDiskServiceAccessToken::GetUserId();
    if (userId == 0) {
        CloudDiskServiceAccessToken::GetAccountId(userId);
    }
    for (const auto &item : syncFolders) {
        std::string path;
        if (CloudDiskSyncFolder::GetInstance().PathToPhysicalPath(item.path_, std::to_string(userId), path) != E_OK) {
            LOGE("Get path failed");
            continue;
        }
        SyncFolderValue syncFolderValue;
        syncFolderValue.bundleName = item.bundleName_;
        syncFolderValue.path = path;
        uint32_t syncFolderIndex = CloudDisk::CloudFileUtils::DentryHash(path);
        CloudDiskSyncFolder::GetInstance().AddSyncFolder(syncFolderIndex, syncFolderValue);
    }

    try {
        PublishSA();
        AddSystemAbilityListener(COMMON_EVENT_SERVICE_ID);
    } catch (const exception &e) {
        LOGE("%{public}s", e.what());
    }

    if (CloudDiskSyncFolder::GetInstance().GetSyncFolderSize() > 0) {
        DiskMonitor::GetInstance().StartMonitor(userId);
    }
    state_ = ServiceRunningState::STATE_RUNNING;
    UnloadSa();

    LOGI("Start service successfully");
#endif
}

void CloudDiskService::OnStop()
{
    LOGI("Begin to stop");
    state_ = ServiceRunningState::STATE_NOT_START;
    registerToService_ = false;
    LOGI("Stop finished successfully");
}

static int32_t CheckPermissions(const string &permission, bool isSystemApp)
{
    if (!permission.empty() && !CloudDiskServiceAccessToken::CheckCallerPermission(permission)) {
        LOGE("permission denied");
        return E_PERMISSION_DENIED;
    }
    return E_OK;
}

int32_t CloudDiskService::RegisterSyncFolderChangesInner(const std::string &syncFolder,
                                                         const sptr<IRemoteObject> &remoteObject)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    LOGI("Begin RegisterSyncFolderChangesInner");

    if (remoteObject == nullptr) {
        LOGE("remoteObject is nullptr");
        return E_INVALID_ARG;
    }

    int32_t userId = CloudDiskServiceAccessToken::GetUserId();
    std::string path;

    int32_t ret = CloudDiskSyncFolder::GetInstance().PathToPhysicalPath(syncFolder, std::to_string(userId), path);
    if (ret != E_OK) {
        LOGE("Get path failed, ret = %{public}d", ret);
        return ret;
    }

    std::string bundleName = "";
    ret = CloudDiskServiceAccessToken::GetCallerBundleName(bundleName);
    if (ret != E_OK) {
        LOGE("Get bundleName failed, ret:%{public}d", ret);
        return E_TRY_AGAIN;
    }

    auto syncFolderIndex = CloudDisk::CloudFileUtils::DentryHash(path);
    SyncFolderValue syncFolderValue;
    if (!CloudDiskSyncFolder::GetInstance().GetSyncFolderValueByIndex(syncFolderIndex, syncFolderValue) ||
        syncFolderValue.bundleName != bundleName) {
        LOGE("SyncFolder is not exist");
        return E_SYNC_FOLDER_NOT_REGISTERED;
    }

    auto callback = iface_cast<ICloudDiskServiceCallback>(remoteObject);

    if (!CloudDiskServiceCallbackManager::GetInstance().RegisterSyncFolderMap(bundleName, syncFolderIndex, callback)) {
        return E_LISTENER_ALREADY_REGISTERED;
    }

    CloudDiskServiceSyncFolder::RegisterSyncFolderChanges(userId, syncFolderIndex);
    CloudDiskServiceCallbackManager::GetInstance().AddCallback(bundleName, callback);

    LOGI("End RegisterSyncFolderChangesInner");
    return E_OK;
#else
    return E_NOT_SUPPORTED;
#endif
}

int32_t CloudDiskService::UnregisterSyncFolderChangesInner(const std::string &syncFolder)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    LOGI("Begin UnregisterSyncFolderChangesInner");

    int32_t userId = CloudDiskServiceAccessToken::GetUserId();
    std::string path;

    int32_t ret = CloudDiskSyncFolder::GetInstance().PathToPhysicalPath(syncFolder, std::to_string(userId), path);
    if (ret != E_OK) {
        LOGE("Get path failed, ret = %{public}d", ret);
        return ret;
    }

    std::string bundleName = "";
    ret = CloudDiskServiceAccessToken::GetCallerBundleName(bundleName);
    if (ret != E_OK) {
        LOGE("Get bundleName failed, ret:%{public}d", ret);
        return E_TRY_AGAIN;
    }

    auto syncFolderIndex = CloudDisk::CloudFileUtils::DentryHash(path);
    SyncFolderValue syncFolderValue;
    if (!CloudDiskSyncFolder::GetInstance().GetSyncFolderValueByIndex(syncFolderIndex, syncFolderValue) ||
        syncFolderValue.bundleName != bundleName) {
        LOGE("SyncFolder is not exist");
        return E_SYNC_FOLDER_NOT_REGISTERED;
    }

    if (!CloudDiskServiceCallbackManager::GetInstance().UnregisterSyncFolderForChangesMap(
        bundleName, syncFolderIndex)) {
        return E_LISTENER_NOT_REGISTERED;
    }

    CloudDiskServiceSyncFolder::UnRegisterSyncFolderChanges(userId, syncFolderIndex);
    LOGI("End UnregisterSyncFolderChangesInner");
    return E_OK;
#else
    return E_NOT_SUPPORTED;
#endif
}

int32_t CloudDiskService::GetSyncFolderChangesInner(const std::string &syncFolder,
                                                    uint64_t count,
                                                    uint64_t startUsn,
                                                    ChangesResult &changesResult)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    LOGI("Begin GetSyncFolderChangesInner");

    if (count > GET_SYNC_FOLDER_CHANGE_MAX) {
        LOGE("Invalid argument");
        return E_INVALID_ARG;
    }

    int32_t userId = CloudDiskServiceAccessToken::GetUserId();
    std::string path;

    int32_t ret = CloudDiskSyncFolder::GetInstance().PathToPhysicalPath(syncFolder, std::to_string(userId), path);
    if (ret != E_OK) {
        LOGE("Get path failed, ret = %{public}d", ret);
        return ret;
    }

    std::string bundleName = "";
    ret = CloudDiskServiceAccessToken::GetCallerBundleName(bundleName);
    if (ret != E_OK) {
        LOGE("Get bundleName failed, ret:%{public}d", ret);
        return E_TRY_AGAIN;
    }

    auto syncFolderIndex = CloudDisk::CloudFileUtils::DentryHash(path);
    SyncFolderValue syncFolderValue;
    if (!CloudDiskSyncFolder::GetInstance().GetSyncFolderValueByIndex(syncFolderIndex, syncFolderValue) ||
        syncFolderValue.bundleName != bundleName) {
        LOGE("SyncFolder is not exist");
        return E_SYNC_FOLDER_NOT_REGISTERED;
    }

    ret = CloudDiskServiceSyncFolder::GetSyncFolderChanges(userId, syncFolderIndex, startUsn, count, changesResult);
    if (ret != E_OK) {
        LOGE("GetSyncFolderChanges failed");
        return ret;
    }
    LOGI("End GetSyncFolderChangesInner");
    return E_OK;
#else
    return E_NOT_SUPPORTED;
#endif
}

static int32_t GetErrorNum(int32_t error)
{
    int32_t errNum;
    switch (error) {
        case EDQUOT:
            errNum = static_cast<int32_t>(ErrorReason::NO_SPACE_LEFT);
            break;
        case ERANGE:
            errNum = static_cast<int32_t>(ErrorReason::OUT_OF_RANGE);
            break;
        case ENOENT:
            errNum = static_cast<int32_t>(ErrorReason::NO_SUCH_FILE);
            break;
        default:
            errNum = static_cast<int32_t>(ErrorReason::NO_SYNC_STATE);
    }
    return errNum;
}

static bool SetFileSyncStates(const FileSyncState &fileSyncStates, int32_t userId, FailedList &failed,
    const string &syncFolder)
{
    std::string setXattrPath;
    if (CloudDiskSyncFolder::GetInstance().PathToMntPathBySandboxPath(fileSyncStates.path, std::to_string(userId),
        setXattrPath) != E_OK) {
        LOGE("Get path failed");
        failed.path = fileSyncStates.path;
        failed.error = ErrorReason::NO_SUCH_FILE;
        return false;
    }
    std::string actualSyncFolder = syncFolder;
    if (!syncFolder.empty() && syncFolder.back() != '/') {
        actualSyncFolder += "/";
    }
    if (fileSyncStates.path.compare(0, actualSyncFolder.size(), actualSyncFolder) != 0) {
        LOGE("path does not match the syncFolder");
        failed.path = fileSyncStates.path;
        failed.error = ErrorReason::INVALID_ARGUMENT;
        return false;
    }
    uint8_t state = static_cast<uint8_t>(fileSyncStates.state);
    if (state > static_cast<int>(SyncState::SYNC_CONFLICTED)) {
        LOGE("Invalid state");
        failed.path = fileSyncStates.path;
        failed.error = ErrorReason::OUT_OF_RANGE;
        return false;
    }
    if (setxattr(setXattrPath.c_str(), FILE_SYNC_STATE, &state, sizeof(state), 0) != 0) {
        int32_t error = errno;
        LOGE("Failed to set xattr, err: %{public}d", error);
        failed.path = fileSyncStates.path;
        failed.error = static_cast<ErrorReason>(GetErrorNum(error));
        return false;
    }
    return true;
}

int32_t CloudDiskService::SetFileSyncStatesInner(const std::string &syncFolder,
                                                 const std::vector<FileSyncState> &fileSyncStates,
                                                 std::vector<FailedList> &failedList)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    LOGI("Begin SetXattrInner");

    if (fileSyncStates.empty() || fileSyncStates.size() > GET_FILE_SYNC_MAX) {
        LOGE("Invalid parameter");
        return E_INVALID_ARG;
    }

    int32_t userId = CloudDiskServiceAccessToken::GetUserId();
    std::string path;

    int32_t ret = CloudDiskSyncFolder::GetInstance().PathToPhysicalPath(syncFolder, std::to_string(userId), path);
    if (ret != E_OK) {
        LOGE("Get path failed, ret = %{public}d", ret);
        return ret;
    }

    std::string bundleName = "";
    ret = CloudDiskServiceAccessToken::GetCallerBundleName(bundleName);
    if (ret != E_OK) {
        LOGE("Get bundleName failed, ret:%{public}d", ret);
        return E_TRY_AGAIN;
    }

    auto syncFolderIndex = CloudDisk::CloudFileUtils::DentryHash(path);
    SyncFolderValue syncFolderValue;
    if (!CloudDiskSyncFolder::GetInstance().GetSyncFolderValueByIndex(syncFolderIndex, syncFolderValue) ||
        syncFolderValue.bundleName != bundleName) {
        LOGE("SyncFolder is not exist");
        return E_SYNC_FOLDER_NOT_REGISTERED;
    }

    FailedList failed;
    for (auto &item : fileSyncStates) {
        if (!SetFileSyncStates(item, userId, failed, syncFolder)) {
            failedList.push_back(failed);
        }
    }

    LOGI("End SetXattrInner");
    return E_OK;
#else
    return E_NOT_SUPPORTED;
#endif
}

static ResultList MakeResult(const std::string &path)
{
    ResultList result;
    result.path = path;
    result.isSuccess = false;
    return result;
}

static ResultList GetFileSyncState(const std::string &path, int32_t &userId, const string &syncFolder)
{
    ResultList getResult = MakeResult(path);
    std::string getXattrPath;

    if (CloudDiskSyncFolder::GetInstance().PathToMntPathBySandboxPath(path, std::to_string(userId),
                                                                      getXattrPath) != E_OK) {
        LOGE("Get path failed");
        getResult.error = ErrorReason::NO_SUCH_FILE;
        return getResult;
    }

    string actualSyncFolder = syncFolder;
    if (!syncFolder.empty() && syncFolder.back() != '/') {
        actualSyncFolder += "/";
    }
    if (path.compare(0, actualSyncFolder.size(), actualSyncFolder) != 0) {
        LOGE("path does not match the syncFolder");
        getResult.error = ErrorReason::INVALID_ARGUMENT;
        return getResult;
    }

    auto xattrValueSize = getxattr(getXattrPath.c_str(), FILE_SYNC_STATE, nullptr, 0);
    if (xattrValueSize < 0) {
        int32_t error = errno;
        LOGE("getxattr failed, errno : %{public}d", error);
        getResult.error = static_cast<ErrorReason>(GetErrorNum(error));
        return getResult;
    }

    std::unique_ptr<char[]> xattrValue = std::make_unique<char[]>((long)xattrValueSize);
    if (xattrValue == nullptr) {
        LOGE("Failed to allocate memory for xattrValue, errno : %{public}d", errno);
        getResult.error = ErrorReason::NO_SPACE_LEFT;
        return getResult;
    }

    xattrValueSize = getxattr(getXattrPath.c_str(), FILE_SYNC_STATE, xattrValue.get(), xattrValueSize);
    if (xattrValueSize <= 0) {
        int32_t error = errno;
        LOGE("getxattr failed, errno : %{public}d", error);
        getResult.error = static_cast<ErrorReason>(GetErrorNum(error));
        return getResult;
    }

    uint8_t rawState = static_cast<uint8_t>(xattrValue[0]);
    if (rawState > static_cast<int>(SyncState::SYNC_CONFLICTED)) {
        LOGE("get invalid number");
        getResult.error = ErrorReason::INVALID_ARGUMENT;
        return getResult;
    }

    getResult.isSuccess = true;
    getResult.state = static_cast<SyncState>(rawState);
    return getResult;
}

int32_t CloudDiskService::GetFileSyncStatesInner(const std::string &syncFolder,
                                                 const std::vector<std::string> &pathArray,
                                                 std::vector<ResultList> &resultList)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    LOGI("Begin GetXattrInner");

    if (pathArray.empty() || pathArray.size() > GET_FILE_SYNC_MAX) {
        LOGE("Invalid parameter");
        return E_INVALID_ARG;
    }

    int32_t userId = CloudDiskServiceAccessToken::GetUserId();
    std::string path;

    int32_t ret = CloudDiskSyncFolder::GetInstance().PathToPhysicalPath(syncFolder, std::to_string(userId), path);
    if (ret != E_OK) {
        LOGE("Get path failed, ret = %{public}d", ret);
        return ret;
    }

    std::string bundleName = "";
    ret = CloudDiskServiceAccessToken::GetCallerBundleName(bundleName);
    if (ret != E_OK) {
        LOGE("Get bundleName failed, ret:%{public}d", ret);
        return E_TRY_AGAIN;
    }

    auto syncFolderIndex = CloudDisk::CloudFileUtils::DentryHash(path);
    SyncFolderValue syncFolderValue;
    if (!CloudDiskSyncFolder::GetInstance().GetSyncFolderValueByIndex(syncFolderIndex, syncFolderValue) ||
        syncFolderValue.bundleName != bundleName) {
        LOGE("SyncFolder is not exist");
        return E_SYNC_FOLDER_NOT_REGISTERED;
    }

    ResultList getResult;
    for (auto &item : pathArray) {
        getResult = GetFileSyncState(item, userId, syncFolder);
        resultList.push_back(getResult);
    }

    LOGI("End GetXattrInner");
    return E_OK;
#else
    return E_NOT_SUPPORTED;
#endif
}

int32_t CloudDiskService::RegisterSyncFolderInner(int32_t userId, const std::string &bundleName,
    const std::string &path)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    LOGI("Begin RegisterSyncFolderInner");
    RETURN_ON_ERR(CheckPermissions(PERM_CLOUD_DISK_SERVICE, true));

    std::string registerSyncFolder;
    int32_t ret = CloudDiskSyncFolder::GetInstance().PathToPhysicalPath(path,
        std::to_string(userId), registerSyncFolder);
    if (ret != E_OK) {
        LOGE("Get path failed, ret = %{public}d", ret);
        return ret;
    }

    auto syncFolderIndex = CloudDisk::CloudFileUtils::DentryHash(registerSyncFolder);
    ret = CloudDiskServiceSyncFolder::RegisterSyncFolder(userId, syncFolderIndex, registerSyncFolder);
    if (ret != E_OK) {
        LOGE("RegisterSyncFolder failed");
        return ret;
    }

    if (CloudDiskSyncFolder::GetInstance().GetSyncFolderSize() == 0) {
        DiskMonitor::GetInstance().StartMonitor(userId);
    }

    SyncFolderValue syncFolderValue;
    syncFolderValue.bundleName = bundleName;
    syncFolderValue.path = registerSyncFolder;

    CloudDiskSyncFolder::GetInstance().AddSyncFolder(syncFolderIndex, syncFolderValue);
    LOGI("End RegisterSyncFolderInner");
    return E_OK;
#else
    return E_NOT_SUPPORTED;
#endif
}

int32_t CloudDiskService::UnregisterSyncFolderInner(int32_t userId,
    const std::string &bundleName, const std::string &path)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    LOGI("Begin UnregisterSyncFolderInner");
    RETURN_ON_ERR(CheckPermissions(PERM_CLOUD_DISK_SERVICE, true));

    std::string unregisterSyncFolder;
    int32_t ret = CloudDiskSyncFolder::GetInstance().PathToPhysicalPath(path,
        std::to_string(userId), unregisterSyncFolder);
    if (ret != E_OK) {
        LOGE("Get unregisterSyncFolder failed, ret = %{public}d", ret);
        return ret;
    }

    auto syncFolderIndex = CloudDisk::CloudFileUtils::DentryHash(unregisterSyncFolder);
    ret = CloudDiskServiceSyncFolder::UnRegisterSyncFolder(userId, syncFolderIndex);
    if (ret != E_OK) {
        LOGE("UnRegisterSyncFolder failed");
        return ret;
    }

    CloudDiskSyncFolder::GetInstance().DeleteSyncFolder(syncFolderIndex);
    CloudDiskServiceCallbackManager::GetInstance().UnregisterSyncFolderMap(bundleName, syncFolderIndex);

    ret = CloudDiskSyncFolder::GetInstance().PathToMntPathBySandboxPath(path, std::to_string(userId),
        unregisterSyncFolder);
    if (ret != E_OK) {
        LOGE("Get unregisterSyncFolder failed, ret = %{public}d", ret);
        return ret;
    }

    CloudDiskSyncFolder::GetInstance().RemoveXattr(unregisterSyncFolder, FILE_SYNC_STATE);
    UnloadSa();
    LOGI("End UnregisterSyncFolderInner");
    return E_OK;
#else
    return E_NOT_SUPPORTED;
#endif
}

int32_t CloudDiskService::UnregisterForSaInner(const std::string &path)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    std::string pathRemove;
    int32_t userId = CloudDiskServiceAccessToken::GetUserId();
    if (userId == 0) {
        CloudDiskServiceAccessToken::GetAccountId(userId);
    }

    if (!CloudDiskSyncFolder::GetInstance().PathToSandboxPathByPhysicalPath(path,
        std::to_string(userId), pathRemove)) {
        LOGE("Get path failed");
        return E_INVALID_ARG;
    }

    int32_t ret = OHOS::FileManagement::CloudDiskSyncFolderManager::GetInstance().UnregisterForSa(pathRemove);
    if (ret != E_OK) {
        LOGE("UnregisterForSa failed, ret:%{public}d", ret);
        return ret;
    }

    if (!CloudDiskSyncFolder::GetInstance().PathToMntPathByPhysicalPath(path, std::to_string(userId), pathRemove)) {
        LOGE("Get path failed");
        return E_INVALID_ARG;
    }
    CloudDiskSyncFolder::GetInstance().RemoveXattr(pathRemove, FILE_SYNC_STATE);

    auto syncFolderIndex = CloudDisk::CloudFileUtils::DentryHash(path);
    SyncFolderValue syncFolderValue;
    if (!CloudDiskSyncFolder::GetInstance().GetSyncFolderValueByIndex(syncFolderIndex, syncFolderValue)) {
        LOGE("No such index");
        return E_SYNC_FOLDER_NOT_REGISTERED;
    }

    CloudDiskSyncFolder::GetInstance().DeleteSyncFolder(syncFolderIndex);
    CloudDiskServiceCallbackManager::GetInstance().UnregisterSyncFolderMap(syncFolderValue.bundleName, syncFolderIndex);
    UnloadSa();
    return ret;
#else
    return E_NOT_SUPPORTED;
#endif
}

void CloudDiskService::OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    LOGI("OnAddSystemAbility systemAbilityId:%{public}d added!", systemAbilityId);
    accountStatusListener_->Start();
}

void CloudDiskService::UnloadSa()
{
    if (CloudDiskSyncFolder::GetInstance().GetSyncFolderSize() == 0) {
        DiskMonitor::GetInstance().StopMonitor();
        auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (samgrProxy == nullptr) {
            LOGE("get samgr failed");
            return;
        }
        int32_t ret = samgrProxy->UnloadSystemAbility(FILEMANAGEMENT_CLOUD_DISK_SERVICE_SA_ID);
        if (ret != ERR_OK) {
            LOGE("remove system ability failed");
            return;
        }
    }
}
} // namespace CloudDiskService
} // namespace FileManagement
} // namespace OHOS
