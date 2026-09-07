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

#include <array>
#include <cerrno>
#include <charconv>
#include <climits>
#include <cstdlib>
#include <fcntl.h>
#include <functional>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <sys/xattr.h>
#include <unistd.h>

#include "cloud_disk_service_access_token.h"
#include "cloud_disk_service_callback_manager.h"
#include "cloud_disk_service_callback_proxy.h"
#include "cloud_disk_service_error.h"
#include "cloud_disk_service_syncfolder.h"
#include "cloud_disk_service_utils.h"
#ifdef SUPPORT_CLOUD_DISK_SERVICE
#include "cloud_disk_sync_folder_manager.h"
#endif
#include "cloud_disk_sync_folder.h"
#include "iremote_object.h"
#include "iservice_registry.h"
#include "placeholder_callback_manager.h"
#include "placeholder_helper.h"
#include "placeholder_task_manager.h"
#include "cloud_disk_progress_callback_proxy.h"
#include "placeholder_progress_manager.h"
#include "ipc_skeleton.h"
#include "system_ability_definition.h"
#include "unique_fd.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudDiskService {
using namespace std;

const int32_t GET_FILE_SYNC_MAX = 100;
const int32_t GET_SYNC_FOLDER_CHANGE_MAX = 100;
constexpr const char *USER_UNLOCKED_REASON = "usual.event.USER_UNLOCKED";

namespace {
constexpr mode_t PLACEHOLDER_FILE_MODE = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
constexpr uint64_t MILLISECONDS_PER_SECOND = 1000;
constexpr uint64_t NANOSECONDS_PER_MILLISECOND = 1000000;
constexpr size_t PLACEHOLDER_CUSTOM_INFO_MAX_SIZE = 4096;
constexpr const char *CLOUD_DISK_CUSTOM_INFO_XATTR = "user.clouddisk.custominfo";
constexpr size_t DEHYDRATE_LOCK_BUCKET_COUNT = 64;
constexpr int32_t DEHYDRATE_EXTEND_MAX_ATTEMPTS = 3;

struct CreatePlaceholderPath {
    std::string parentMntPath;
    std::string fileName;
    std::string syncRootMntPath;
};

static int32_t BuildCreatePlaceholderPath(const std::string &syncFolder,
                                          const std::string &relativePath,
                                          int32_t userId,
                                          CreatePlaceholderPath &path)
{
    if (relativePath.empty() || HasInvalidRelativePathSegment(relativePath) || relativePath.front() == '/' ||
        relativePath.back() == '/') {
        LOGE("CreatePlaceholderFile branch=invalid_relative_path");
        return E_INVALID_ARG;
    }
    std::string parentRelativePath;
    auto pos = relativePath.find_last_of('/');
    if (pos == std::string::npos) {
        path.fileName = relativePath;
    } else {
        parentRelativePath = relativePath.substr(0, pos);
        path.fileName = relativePath.substr(pos + 1);
    }

    int32_t ret = CloudDiskSyncFolder::GetInstance().PathToMntPathBySandboxPath(syncFolder, std::to_string(userId),
                                                                                path.syncRootMntPath);
    if (ret != E_OK) {
        LOGE("CreatePlaceholderFile branch=sync_folder_mnt_path_failed ret=%{public}d", ret);
        return ret;
    }
    path.parentMntPath = path.syncRootMntPath;
    if (!parentRelativePath.empty()) {
        path.parentMntPath = JoinSyncFolderAndRelativePath(path.parentMntPath, parentRelativePath);
    }
    LOGI("CreatePlaceholderFile branch=build_path_success");
    return E_OK;
}

#ifdef SUPPORT_CLOUD_DISK_SERVICE
bool ParseUserIdFromReasonValue(const string &reasonValue, int32_t &userId)
{
    if (reasonValue.empty()) {
        LOGE("Parse userId from start reason failed, reasonValue: %{public}s", reasonValue.c_str());
        return false;
    }

    auto result = std::from_chars(reasonValue.data(), reasonValue.data() + reasonValue.size(), userId);
    if (result.ec != std::errc{} || result.ptr != reasonValue.data() + reasonValue.size() || userId < 0) {
        LOGE("Parse userId from start reason failed, reasonValue: %{public}s", reasonValue.c_str());
        return false;
    }
    return true;
}

bool ParseAndVerifyUserIdFromReasonValue(const SystemAbilityOnDemandReason &startReason, int32_t &userId)
{
    string reasonValue = startReason.GetValue();
    if (!ParseUserIdFromReasonValue(reasonValue, userId)) {
        return false;
    }
    LOGI("Parse userId from start reason success, userId: %{public}d", userId);

    if (CloudDiskServiceAccessToken::IsUserVerifyed(userId)) {
        LOGI("UserId is verified, userId: %{public}d", userId);
        return true;
    }
    LOGE("userId is invalid, userId: %{public}d, reasonValue: %{public}s", userId, reasonValue.c_str());
    return false;
}

bool GetUserIdByStartReason(const SystemAbilityOnDemandReason &startReason, int32_t &userId)
{
    string reason = startReason.GetName();
    LOGI("Get userId by start reason, reason: %{public}s", reason.c_str());
    if (reason == USER_UNLOCKED_REASON) {
        return ParseAndVerifyUserIdFromReasonValue(startReason, userId);
    }

    userId = CloudDiskServiceAccessToken::GetUserId();
    if (userId == 0) {
        CloudDiskServiceAccessToken::GetAccountId(userId);
    }
    return true;
}

constexpr int32_t GET_SYNC_FOLDERS_RETRY_ERR_CODE = 34400001;
constexpr int32_t GET_SYNC_FOLDERS_MAX_RETRY_TIMES = 2;
constexpr int32_t GET_SYNC_FOLDERS_RETRY_INTERVAL_US = 200 * 1000;

int32_t GetAllSyncFoldersForSaWithRetry(std::vector<FileManagement::SyncFolderExt> &syncFolders)
{
    int32_t ret = OHOS::FileManagement::CloudDiskSyncFolderManager::GetInstance().GetAllSyncFoldersForSa(syncFolders);
    int32_t retryCount = 0;
    while (ret == GET_SYNC_FOLDERS_RETRY_ERR_CODE && retryCount < GET_SYNC_FOLDERS_MAX_RETRY_TIMES) {
        ++retryCount;
        LOGE("Get all sync folders for sa failed, retry: %{public}d/%{public}d, ret: %{public}d", retryCount,
             GET_SYNC_FOLDERS_MAX_RETRY_TIMES, ret);
        usleep(GET_SYNC_FOLDERS_RETRY_INTERVAL_US);
        syncFolders.clear();
        ret = OHOS::FileManagement::CloudDiskSyncFolderManager::GetInstance().GetAllSyncFoldersForSa(syncFolders);
    }
    return ret;
}
#endif

static struct timespec MillisecondsToTimespec(uint64_t timeMs)
{
    struct timespec time = {
        .tv_sec = static_cast<time_t>(timeMs / MILLISECONDS_PER_SECOND),
        .tv_nsec = static_cast<long>((timeMs % MILLISECONDS_PER_SECOND) * NANOSECONDS_PER_MILLISECOND),
    };
    return time;
}

static int32_t SetPlaceholderFileAttributes(int32_t fileFd,
                                            const PlaceholderInfo &info,
                                            uint8_t *oldPlaceholderState = nullptr,
                                            const PlaceholderCustomInfo &customInfo = PlaceholderCustomInfo())
{
    if (ftruncate(fileFd, static_cast<off_t>(info.logicalSize)) < 0) {
        int32_t err = errno;
        LOGE("CreatePlaceholderFile branch=set_size_failed errno=%{public}d", err);
        return err;
    }
    int32_t err = E_OK;
    if (oldPlaceholderState == nullptr) {
        err = SetNewFilePlaceholderState(fileFd, PLACEHOLDER_STATE_UNHYDRATED);
    } else {
        err = SetFilePlaceholderState(fileFd, PLACEHOLDER_STATE_UNHYDRATED, *oldPlaceholderState);
    }
    if (err != E_OK) {
        LOGE("CreatePlaceholderFile branch=set_xattr_failed errno=%{public}d", err);
        return err;
    }
    if (!customInfo.data.empty() &&
        fsetxattr(fileFd, CLOUD_DISK_CUSTOM_INFO_XATTR, customInfo.data.data(), customInfo.data.size(), 0) < 0) {
        err = errno;
        LOGE("Set placeholder custom info failed, errno=%{public}d", err);
        return err;
    }

    struct timespec times[2] = {
        MillisecondsToTimespec(info.atimeMs),
        MillisecondsToTimespec(info.mtimeMs),
    };
    if (futimens(fileFd, times) < 0) {
        err = errno;
        LOGE("CreatePlaceholderFile branch=set_times_failed errno=%{public}d", err);
        return err;
    }
    return E_OK;
}

static int32_t CreatePlaceholderFileAt(const CreatePlaceholderPath &path,
                                       const PlaceholderInfo &info,
                                       const PlaceholderCustomInfo &customInfo = PlaceholderCustomInfo())
{
    UniqueFd parentFd(open(path.parentMntPath.c_str(), O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC));
    if (parentFd < 0) {
        LOGE("CreatePlaceholderFile branch=open_parent_failed errno=%{public}d", errno);
        return ConvertErrnoToCloudDiskError(errno);
    }

    UniqueFd fileFd(openat(parentFd, path.fileName.c_str(), O_CREAT | O_EXCL | O_RDWR | O_NOFOLLOW | O_CLOEXEC,
                           PLACEHOLDER_FILE_MODE));
    if (fileFd < 0) {
        LOGE("CreatePlaceholderFile branch=openat_file_failed errno=%{public}d", errno);
        return ConvertErrnoToCloudDiskError(errno);
    }

    int32_t err = SetPlaceholderFileAttributes(fileFd, info, nullptr, customInfo);
    if (err != E_OK) {
        if (unlinkat(parentFd, path.fileName.c_str(), 0) != 0) {
            LOGE("CreatePlaceholderFile branch=rollback_unlink_failed errno=%{public}d", errno);
        }
        return ConvertErrnoToCloudDiskError(err);
    }
    std::string filePath = JoinSyncFolderAndRelativePath(path.parentMntPath, path.fileName);
    err = RefreshAncestorPlaceholderCount(path.syncRootMntPath, filePath, 1);
    if (err != E_OK) {
        LOGE("CreatePlaceholderFile branch=refresh_placeholder_count_failed errno=%{public}d", err);
    }
    LOGI("CreatePlaceholderFile branch=create_file_success");
    return E_OK;
}

static int32_t NormalizeCreatePlaceholderError(int32_t ret)
{
    switch (ret) {
        case E_OK:
        case E_PERM:
        case E_ACCES:
        case E_INVALID_ARG:
        case E_PERMISSION_DENIED:
        case E_SYNC_FOLDER_NOT_REGISTERED:
        case E_SYNC_FOLDER_PATH_NOT_EXIST:
        case E_NOT_SUPPORTED:
        case E_TRY_AGAIN:
        case E_IPC_FAILED:
        case E_FILE_ALREADY_EXISTS:
        case E_NO_SPACE_LEFT:
        case E_NOT_A_DIRECTORY:
        case E_NAME_TOO_LONG:
        case E_FILE_TOO_LARGE:
            return ret;
        default:
            return ConvertErrnoToCloudDiskError(ret);
    }
}
} // namespace
REGISTER_SYSTEM_ABILITY_BY_ID(CloudDiskService, FILEMANAGEMENT_CLOUD_DISK_SERVICE_SA_ID, true);

CloudDiskService::CloudDiskService(int32_t saID, bool runOnCreate) : SystemAbility(saID, runOnCreate)
{
    accountStatusListener_ = make_shared<AccountStatusListener>();
}

CloudDiskService::CloudDiskService() {}

bool CloudDiskService::PublishSA()
{
    LOGI("Begin to init");
    if (!registerToService_) {
        bool ret = SystemAbility::Publish(this);
        if (!ret) {
            LOGE("Failed to publish the clouddiskservice");
            return false;
        }
        registerToService_ = true;
    }
    LOGI("Init finished successfully");
    return true;
}

void CloudDiskService::OnStart(const SystemAbilityOnDemandReason &startReason)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    LOGI("Begin to start service");
    if (state_ == ServiceRunningState::STATE_RUNNING) {
        LOGI("CloudDiskService has already started");
        return;
    }

    int32_t userId = 0;
    if (!GetUserIdByStartReason(startReason, userId)) {
        return;
    }
    currentUserId_ = userId;

    std::vector<FileManagement::SyncFolderExt> syncFolders;
    int32_t ret = GetAllSyncFoldersForSaWithRetry(syncFolders);
    if (ret != E_OK) {
        LOGE("Get all sync folders for sa failed, ret: %{public}d, syncFolderSize: %{public}zu", ret,
             syncFolders.size());
        return;
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

    if (!PublishSA()) {
        return;
    }
    PlaceholderTaskManager::GetInstance().StartWorkerPool();
    AddSystemAbilityListener(COMMON_EVENT_SERVICE_ID);

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
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    PlaceholderTaskManager::GetInstance().StopWorkerPool();
    PlaceholderProgressManager::GetInstance().Drain();
    PlaceholderProgressManager::GetInstance().Clear();
#endif
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

int32_t CloudDiskService::ResolveOwnedSyncFolder(const std::string &syncFolder,
                                                 std::string &bundleName,
                                                 uint32_t &syncFolderIndex,
                                                 std::string &physicalPath)
{
    int32_t userId = CloudDiskServiceAccessToken::GetUserId();
    int32_t ret =
        CloudDiskSyncFolder::GetInstance().PathToPhysicalPath(syncFolder, std::to_string(userId), physicalPath);
    if (ret != E_OK) {
        LOGE("Get physical sync folder failed, ret: %{public}d", ret);
        return ret;
    }
    ret = CloudDiskServiceAccessToken::GetCallerBundleName(bundleName);
    if (ret != E_OK) {
        LOGE("Get caller bundle name failed, ret: %{public}d", ret);
        return E_TRY_AGAIN;
    }

    syncFolderIndex = CloudDisk::CloudFileUtils::DentryHash(physicalPath);
    SyncFolderValue syncFolderValue;
    if (!CloudDiskSyncFolder::GetInstance().GetSyncFolderValueByIndex(syncFolderIndex, syncFolderValue)) {
        LOGE("Sync folder is not registered");
        return E_SYNC_FOLDER_NOT_REGISTERED;
    }
    if (syncFolderValue.bundleName != bundleName) {
        LOGE("Sync folder is not owned by caller");
        return E_SYNC_FOLDER_PATH_UNAUTHORIZED;
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

    if (!CloudDiskServiceCallbackManager::GetInstance().UnregisterSyncFolderForChangesMap(bundleName,
                                                                                          syncFolderIndex)) {
        return E_LISTENER_NOT_REGISTERED;
    }

    CloudDiskServiceSyncFolder::UnRegisterSyncFolderChanges(userId, syncFolderIndex);
    LOGI("End UnregisterSyncFolderChangesInner");
    return E_OK;
#else
    return E_NOT_SUPPORTED;
#endif
}

int32_t CloudDiskService::RegisterCallbackTableInner(const std::string &syncFolder,
                                                     const sptr<IRemoteObject> &remoteObject)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    LOGI("Begin RegisterCallbackTableInner");
    if (remoteObject == nullptr) {
        LOGE("Callback table remote object is nullptr");
        return E_INVALID_ARG;
    }

    std::string bundleName;
    uint32_t syncFolderIndex = 0;
    std::string physicalPath;
    int32_t ret = ResolveOwnedSyncFolder(syncFolder, bundleName, syncFolderIndex, physicalPath);
    if (ret != E_OK) {
        return ret;
    }
    auto callback = iface_cast<ICloudDiskServiceCallbackTable>(remoteObject);
    if (callback == nullptr) {
        LOGE("Callback table interface is nullptr");
        return E_INVALID_ARG;
    }
    return PlaceholderCallbackManager::GetInstance().RegisterCallbackTable(bundleName, syncFolderIndex, callback);
#else
    return E_NOT_SUPPORTED;
#endif
}

int32_t CloudDiskService::UnregisterCallbackTableInner(const std::string &syncFolder)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    LOGI("Begin UnregisterCallbackTableInner");
    std::string bundleName;
    uint32_t syncFolderIndex = 0;
    std::string physicalPath;
    int32_t ret = ResolveOwnedSyncFolder(syncFolder, bundleName, syncFolderIndex, physicalPath);
    if (ret != E_OK) {
        return ret;
    }
    return PlaceholderCallbackManager::GetInstance().UnregisterCallbackTable(bundleName, syncFolderIndex);
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

static bool SetFileSyncStates(
    const FileSyncState &fileSyncStates, int32_t userId, FailedList &failed, const string &syncFolder)
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
    int32_t ret = SetFileSyncState(setXattrPath, state);
    if (ret != E_OK) {
        int32_t error = ret;
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

    if (CloudDiskSyncFolder::GetInstance().PathToMntPathBySandboxPath(path, std::to_string(userId), getXattrPath) !=
        E_OK) {
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

    struct stat statInfo {};
    if (stat(getXattrPath.c_str(), &statInfo) != 0) {
        int32_t error = errno;
        LOGE("stat failed, errno : %{public}d", error);
        getResult.error = static_cast<ErrorReason>(GetErrorNum(error));
        return getResult;
    }
    if (S_ISDIR(statInfo.st_mode)) {
        getResult.error = ErrorReason::NO_SYNC_STATE;
        return getResult;
    }

    uint8_t rawStateByte = 0;
    int32_t ret = GetFileSyncStateByte(getXattrPath, rawStateByte, false);
    if (ret != E_OK) {
        int32_t error = ret;
        LOGE("getxattr failed, errno : %{public}d", error);
        getResult.error = static_cast<ErrorReason>(GetErrorNum(error));
        return getResult;
    }

    uint8_t rawState = GetSyncStateFromFileSyncState(rawStateByte);
    if (rawState > static_cast<int>(SyncState::SYNC_CONFLICTED)) {
        LOGE("get invalid number");
        getResult.error = ErrorReason::INVALID_ARGUMENT;
        return getResult;
    }

    getResult.isSuccess = true;
    getResult.state = static_cast<SyncState>(rawState);
    return getResult;
}

static int32_t ConvertPlaceholderXattrErrno(int32_t error)
{
    if (error == ENODATA) {
        return E_OK;
    }
    if (error == ERANGE) {
        return E_INVALID_ARG;
    }
    if (error == ENOENT) {
        return E_FILE_NOT_EXIST;
    }
    return ConvertErrnoToCloudDiskError(error);
}

static bool IsDirectoryPath(const std::string &path)
{
    struct stat statInfo;
    if (stat(path.c_str(), &statInfo) != 0) {
        return false;
    }
    return S_ISDIR(statInfo.st_mode);
}

static int32_t QueryPlaceholderStateByXattr(const std::string &getXattrPath, uint8_t &placeholderState)
{
    placeholderState = PLACEHOLDER_STATE_NONE;
    int32_t ret = GetFilePlaceholderState(getXattrPath, placeholderState);
    if (ret != E_OK) {
        LOGE("QueryPlaceholderStateByXattr branch=get_placeholder_state_failed errno=%{public}d", ret);
        return ConvertPlaceholderXattrErrno(ret);
    }
    return E_OK;
}

static int32_t QueryPlaceholderByXattr(const std::string &getXattrPath, bool &isPlaceholder)
{
    uint8_t placeholderState = PLACEHOLDER_STATE_NONE;
    int32_t ret = QueryPlaceholderStateByXattr(getXattrPath, placeholderState);
    if (ret != E_OK) {
        return ret;
    }
    isPlaceholder = IsPlaceholderState(placeholderState);
    return E_OK;
}

static int32_t GetRegisteredMntSyncFolder(const std::string &syncFolder, int32_t userId, std::string &mntSyncFolder)
{
    std::string physicalSyncFolder;
    int32_t ret =
        CloudDiskSyncFolder::GetInstance().PathToPhysicalPath(syncFolder, std::to_string(userId), physicalSyncFolder);
    if (ret != E_OK) {
        LOGE("ResolvePlaceholderQueryPath branch=sync_folder_physical_path_failed ret=%{public}d", ret);
        return ret;
    }

    std::string bundleName = "";
    ret = CloudDiskServiceAccessToken::GetCallerBundleName(bundleName);
    if (ret != E_OK) {
        LOGE("ResolvePlaceholderQueryPath branch=get_bundle_failed ret=%{public}d", ret);
        return E_TRY_AGAIN;
    }

    auto syncFolderIndex = CloudDisk::CloudFileUtils::DentryHash(physicalSyncFolder);
    SyncFolderValue syncFolderValue;
    if (!CloudDiskSyncFolder::GetInstance().GetSyncFolderValueByIndex(syncFolderIndex, syncFolderValue) ||
        syncFolderValue.bundleName != bundleName) {
        LOGE("ResolvePlaceholderQueryPath branch=sync_folder_not_registered_or_bundle_mismatch");
        return E_SYNC_FOLDER_NOT_REGISTERED;
    }

    if (!CloudDiskSyncFolder::GetInstance().PathToMntPathByPhysicalPath(physicalSyncFolder, std::to_string(userId),
                                                                        mntSyncFolder)) {
        LOGE("ResolvePlaceholderQueryPath branch=mnt_sync_folder_path_failed");
        return E_INVALID_ARG;
    }
    return E_OK;
}

static int32_t
    ResolvePlaceholderQueryPath(const std::string &syncFolder, const std::string &relativePath, std::string &queryPath)
{
    if (relativePath.empty() || HasInvalidRelativePathSegment(relativePath) || relativePath.front() == '/' ||
        relativePath.back() == '/') {
        LOGE("ResolvePlaceholderQueryPath branch=invalid_relative_path path_size=%{public}zu", relativePath.size());
        return E_INVALID_ARG;
    }

    int32_t userId = CloudDiskServiceAccessToken::GetUserId();
    if (userId == 0) {
        CloudDiskServiceAccessToken::GetAccountId(userId);
    }
    std::string mntSyncFolder;
    int32_t ret = GetRegisteredMntSyncFolder(syncFolder, userId, mntSyncFolder);
    if (ret != E_OK) {
        return ret;
    }

    queryPath = JoinSyncFolderAndRelativePath(mntSyncFolder, relativePath);
    if (!IsPathInSyncFolder(mntSyncFolder, queryPath)) {
        LOGE("ResolvePlaceholderQueryPath branch=path_out_of_sync_folder");
        return E_INVALID_ARG;
    }
    if (IsDirectoryPath(queryPath)) {
        LOGE("ResolvePlaceholderQueryPath branch=directory_path");
        return E_INVALID_ARG;
    }
    return E_OK;
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

int32_t CloudDiskService::CreatePlaceholderFileInner(const std::string &syncFolder,
                                                     const std::string &relativePath,
                                                     const PlaceholderInfo &info,
                                                     const PlaceholderCustomInfo &customInfo)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    LOGI("CreatePlaceholderFileInner route=service_entry");
    if (customInfo.data.size() > PLACEHOLDER_CUSTOM_INFO_MAX_SIZE) {
        LOGE("CreatePlaceholderFileInner branch=custom_info_too_large size=%{public}zu", customInfo.data.size());
        return E_INVALID_ARG;
    }

    int32_t userId = CloudDiskServiceAccessToken::GetUserId();
    std::string syncFolderPhysicalPath;
    int32_t ret = CloudDiskSyncFolder::GetInstance().PathToPhysicalPath(syncFolder, std::to_string(userId),
                                                                        syncFolderPhysicalPath);
    if (ret != E_OK) {
        LOGE("CreatePlaceholderFileInner branch=sync_folder_physical_path_failed ret=%{public}d", ret);
        return NormalizeCreatePlaceholderError(ret);
    }

    std::string bundleName;
    ret = CloudDiskServiceAccessToken::GetCallerBundleName(bundleName);
    if (ret != E_OK) {
        LOGE("CreatePlaceholderFileInner branch=get_bundle_failed ret=%{public}d", ret);
        return E_TRY_AGAIN;
    }

    auto syncFolderIndex = CloudDisk::CloudFileUtils::DentryHash(syncFolderPhysicalPath);
    SyncFolderValue syncFolderValue;
    if (!CloudDiskSyncFolder::GetInstance().GetSyncFolderValueByIndex(syncFolderIndex, syncFolderValue) ||
        syncFolderValue.bundleName != bundleName) {
        LOGE("CreatePlaceholderFileInner branch=sync_folder_not_registered_or_bundle_mismatch");
        return E_SYNC_FOLDER_NOT_REGISTERED;
    }

    CreatePlaceholderPath createPath;
    ret = BuildCreatePlaceholderPath(syncFolder, relativePath, userId, createPath);
    if (ret != E_OK) {
        LOGE("CreatePlaceholderFileInner branch=build_create_path_failed ret=%{public}d", ret);
        return NormalizeCreatePlaceholderError(ret);
    }

    ret = CreatePlaceholderFileAt(createPath, info, customInfo);
    if (ret != E_OK) {
        LOGE("CreatePlaceholderFileInner branch=create_file_at_failed ret=%{public}d", ret);
        return NormalizeCreatePlaceholderError(ret);
    }

    LOGI("CreatePlaceholderFileInner branch=success");
    return E_OK;
#else
    return E_NOT_SUPPORTED;
#endif
}

int32_t CloudDiskService::IsPlaceholderFileInner(const std::string &syncFolder,
                                                 const std::string &path,
                                                 bool &isPlaceholder)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    LOGI("IsPlaceholderFileInner route=service_entry");
    isPlaceholder = false;
    std::string queryPath;
    int32_t ret = ResolvePlaceholderQueryPath(syncFolder, path, queryPath);
    if (ret != E_OK) {
        return ret;
    }
    ret = QueryPlaceholderByXattr(queryPath, isPlaceholder);
    if (ret != E_OK) {
        LOGE("IsPlaceholderFileInner branch=query_xattr_failed ret=%{public}d", ret);
    } else {
        LOGI("IsPlaceholderFileInner branch=success isPlaceholder=%{public}d", isPlaceholder);
    }
    return ret;
#else
    return E_NOT_SUPPORTED;
#endif
}

int32_t CloudDiskService::GetPlaceholderStateInner(const std::string &syncFolder,
                                                   const std::string &relativePath,
                                                   int32_t &state)
{
    state = PLACEHOLDER_STATE_NONE;
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    std::string queryPath;
    int32_t ret = ResolvePlaceholderQueryPath(syncFolder, relativePath, queryPath);
    if (ret != E_OK) {
        LOGE("GetPlaceholderStateInner branch=resolve_path_failed ret=%{public}d", ret);
        return ret;
    }

    uint8_t rawState = PLACEHOLDER_STATE_NONE;
    ret = QueryPlaceholderStateByXattr(queryPath, rawState);
    if (ret != E_OK) {
        LOGE("GetPlaceholderStateInner branch=query_xattr_failed ret=%{public}d", ret);
        return ret;
    }
    if (!IsValidPlaceholderState(rawState)) {
        LOGE("GetPlaceholderStateInner branch=invalid_stored_state state=%{public}u", rawState);
        return E_INVALID_PLACEHOLDER_STATE;
    }
    state = static_cast<int32_t>(rawState);
    return E_OK;
#else
    return E_NOT_SUPPORTED;
#endif
}

int32_t CloudDiskService::RegisterSyncFolderInner(
    int32_t userId, const std::string &bundleName, const std::string &path)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    LOGI("Begin RegisterSyncFolderInner");
    RETURN_ON_ERR(CheckPermissions(PERM_CLOUD_DISK_SERVICE, true));

    std::string registerSyncFolder;
    int32_t ret =
        CloudDiskSyncFolder::GetInstance().PathToPhysicalPath(path, std::to_string(userId), registerSyncFolder);
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

int32_t CloudDiskService::UnregisterSyncFolderInner(
    int32_t userId, const std::string &bundleName, const std::string &path)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    LOGI("Begin UnregisterSyncFolderInner");
    RETURN_ON_ERR(CheckPermissions(PERM_CLOUD_DISK_SERVICE, true));

    std::string unregisterSyncFolder;
    int32_t ret =
        CloudDiskSyncFolder::GetInstance().PathToPhysicalPath(path, std::to_string(userId), unregisterSyncFolder);
    if (ret != E_OK) {
        LOGE("Get unregister physical path failed, ret = %{public}d", ret);
        return ret;
    }

    auto syncFolderIndex = CloudDisk::CloudFileUtils::DentryHash(unregisterSyncFolder);
    std::string unregisterSyncFolderMnt;
    ret = CloudDiskSyncFolder::GetInstance().PathToMntPathBySandboxPath(path, std::to_string(userId),
                                                                        unregisterSyncFolderMnt);
    if (ret != E_OK) {
        LOGE("Get unregister mnt path failed, ret = %{public}d", ret);
        return ret;
    }

    ret = CloudDiskServiceSyncFolder::UnRegisterSyncFolder(userId, syncFolderIndex);
    if (ret != E_OK) {
        LOGE("UnRegisterSyncFolder failed");
        return ret;
    }

    CloudDiskSyncFolder::GetInstance().DeleteSyncFolder(syncFolderIndex);
    CloudDiskServiceCallbackManager::GetInstance().UnregisterSyncFolderMap(bundleName, syncFolderIndex);
    PlaceholderCallbackManager::GetInstance().ClearBySyncFolder(bundleName, syncFolderIndex);

    CloudDiskSyncFolder::GetInstance().RemovePlaceholderFilesBatch(unregisterSyncFolderMnt);
    CloudDiskSyncFolder::GetInstance().RemoveXattr(unregisterSyncFolderMnt, CLOUD_DISK_FILE_SYNC_STATE_XATTR);
    CloudDiskSyncFolder::GetInstance().RemoveXattr(unregisterSyncFolderMnt, CLOUD_DISK_PLACEHOLDER_COUNT_XATTR);
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

    if (!CloudDiskSyncFolder::GetInstance().PathToSandboxPathByPhysicalPath(path, std::to_string(userId), pathRemove)) {
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
    CloudDiskSyncFolder::GetInstance().RemovePlaceholderFilesBatch(pathRemove);
    CloudDiskSyncFolder::GetInstance().RemoveXattr(pathRemove, CLOUD_DISK_FILE_SYNC_STATE_XATTR);
    CloudDiskSyncFolder::GetInstance().RemoveXattr(pathRemove, CLOUD_DISK_PLACEHOLDER_COUNT_XATTR);

    auto syncFolderIndex = CloudDisk::CloudFileUtils::DentryHash(path);
    SyncFolderValue syncFolderValue;
    if (!CloudDiskSyncFolder::GetInstance().GetSyncFolderValueByIndex(syncFolderIndex, syncFolderValue)) {
        LOGE("No such index");
        return E_SYNC_FOLDER_NOT_REGISTERED;
    }

    CloudDiskSyncFolder::GetInstance().DeleteSyncFolder(syncFolderIndex);
    CloudDiskServiceCallbackManager::GetInstance().UnregisterSyncFolderMap(syncFolderValue.bundleName, syncFolderIndex);
    PlaceholderCallbackManager::GetInstance().ClearBySyncFolder(syncFolderValue.bundleName, syncFolderIndex);
    UnloadSa();
    return ret;
#else
    return E_NOT_SUPPORTED;
#endif
}

void CloudDiskService::OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    LOGI("OnAddSystemAbility systemAbilityId:%{public}d added!", systemAbilityId);
    accountStatusListener_->Start(currentUserId_);
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

static int32_t CheckSyncFolderBundleName(const std::string &syncFolder, int32_t userId, const std::string &bundleName)
{
    std::string physicalSyncFolder;
    uint32_t syncFolderIndex;

    // 步骤1：转换同步根sandbox路径到物理路径
    int32_t ret =
        CloudDiskSyncFolder::GetInstance().PathToPhysicalPath(syncFolder, std::to_string(userId), physicalSyncFolder);
    if (ret != E_OK) {
        LOGE("Get physical path failed, ret:%{public}d", ret);
        return ret;
    }

    // 步骤2：计算同步根索引并查询注册信息
    syncFolderIndex = CloudDisk::CloudFileUtils::DentryHash(physicalSyncFolder);
    SyncFolderValue syncFolderValue;
    if (!CloudDiskSyncFolder::GetInstance().GetSyncFolderValueByIndex(syncFolderIndex, syncFolderValue)) {
        LOGE("SyncFolder not registered, path:%{public}s", GetAnonyStringStrictly(physicalSyncFolder).c_str());
        return E_SYNC_FOLDER_NOT_REGISTERED;
    }

    // 步骤3：bundleName一致性校验
    if (syncFolderValue.bundleName != bundleName) {
        LOGE("Permission denied, caller:%{public}s, registered:%{public}s", bundleName.c_str(),
             syncFolderValue.bundleName.c_str());
        return E_SYNC_FOLDER_PATH_UNAUTHORIZED;
    }

    return E_OK;
}

static int32_t GetHmdfsPath(
    const std::string &syncFolder, const std::string &relativePath, int32_t userId, std::string &hmdfsPath)
{
    // 校验相对路径合法性
    if (relativePath.empty() || HasInvalidRelativePathSegment(relativePath) || relativePath.front() == '/' ||
        relativePath.back() == '/') {
        LOGE("GetHmdfsPath branch=invalid_relative_path");
        return E_INVALID_ARG;
    }

    // 拼接实际路径
    std::string path = syncFolder + "/" + relativePath;

    // hmdfs路径：hmdfs文件系统的挂载路径，用于实际文件操作
    int32_t ret =
        CloudDiskSyncFolder::GetInstance().PathToMntPathBySandboxPath(path, std::to_string(userId), hmdfsPath);
    if (ret != E_OK) {
        LOGE("Get hmdfs path failed, ret:%{public}d", ret);
        return ret;
    }

    // 在hmdfs路径上检查文件实际存在性
    if (access(hmdfsPath.c_str(), F_OK) != 0) {
        LOGE("File not exist, path:%{public}s", GetAnonyStringStrictly(hmdfsPath).c_str());
        return E_SYNC_FOLDER_PATH_NOT_EXIST;
    }

    return E_OK;
}

static int32_t GetSyncRootContext(const std::string &syncFolder,
                                  int32_t userId,
                                  std::string &mntSyncFolder,
                                  uint32_t &syncFolderIndex)
{
    std::string physicalSyncFolder;
    int32_t ret =
        CloudDiskSyncFolder::GetInstance().PathToPhysicalPath(syncFolder, std::to_string(userId), physicalSyncFolder);
    if (ret != E_OK) {
        LOGE("Get physical path failed, ret:%{public}d", ret);
        return ret;
    }
    syncFolderIndex = CloudDisk::CloudFileUtils::DentryHash(physicalSyncFolder);
    ret = CloudDiskSyncFolder::GetInstance().PathToMntPathBySandboxPath(syncFolder, std::to_string(userId),
                                                                        mntSyncFolder);
    if (ret != E_OK) {
        LOGE("Get mnt path failed, ret:%{public}d", ret);
        return ret;
    }
    return E_OK;
}

struct PlaceholderStatePathContext {
    std::string hmdfsPath;
    std::string mntSyncFolder;
    int32_t userId = -1;
    uint32_t syncFolderIndex = 0;
    std::string bundleName;
    std::string syncFolder;
    std::string absolutePath;
};

static int32_t ResolvePlaceholderStatePath(const std::string &syncFolder,
                                           const std::string &relativePath,
                                           PlaceholderStatePathContext &context)
{
    context.userId = CloudDiskServiceAccessToken::GetUserId();
    if (context.userId == 0) {
        CloudDiskServiceAccessToken::GetAccountId(context.userId);
    }
    int32_t ret = CloudDiskServiceAccessToken::GetCallerBundleName(context.bundleName);
    if (ret != E_OK) {
        LOGE("Get bundleName failed, ret:%{public}d", ret);
        return E_TRY_AGAIN;
    }
    ret = CheckSyncFolderBundleName(syncFolder, context.userId, context.bundleName);
    if (ret != E_OK) {
        LOGE("CheckSyncFolderAccess failed, ret:%{public}d", ret);
        return ret;
    }
    ret = GetHmdfsPath(syncFolder, relativePath, context.userId, context.hmdfsPath);
    if (ret != E_OK) {
        return ret;
    }
    ret = GetSyncRootContext(syncFolder, context.userId, context.mntSyncFolder, context.syncFolderIndex);
    if (ret == E_OK) {
        context.syncFolder = syncFolder;
    }
    return ret;
}

static int32_t CheckPathNotDir(const std::string &hmdfsPath)
{
    struct stat st;
    int32_t ret = E_OK;
    if (stat(hmdfsPath.c_str(), &st) < 0) {
        ret = errno;
        LOGE("stat failed, errno:%{public}d", ret);
        return ConvertErrnoToCloudDiskError(ret);
    }

    if (S_ISDIR(st.st_mode)) {
        ret = E_INVALID_ARG;
        LOGE("Not support dir, errno:%{public}d", ret);
    }
    return ret;
}

static std::mutex &GetDehydrateFileMutex(const std::string &hmdfsPath)
{
    static std::array<std::mutex, DEHYDRATE_LOCK_BUCKET_COUNT> mutexes;
    size_t index = std::hash<std::string>{}(hmdfsPath) % mutexes.size();
    return mutexes[index];
}

static int32_t CheckDehydrateState(uint8_t state)
{
    if (state == PLACEHOLDER_STATE_NONE) {
        return E_NOT_A_PLACEHOLDER;
    }
    if (state == PLACEHOLDER_STATE_UNHYDRATED) {
        return E_OK;
    }
    if (state != PLACEHOLDER_STATE_FULLY_HYDRATED) {
        return E_PLACEHOLDER_NOT_FULLY_HYDRATED;
    }
    return E_OK;
}

static int32_t DispatchDehydrateAuthorization(const PlaceholderStatePathContext &context,
                                              const std::string &relativePath)
{
    std::string syncFolder = context.syncFolder;
    std::string filePath = relativePath;
    CloudDiskCallbackReqHead reqHead{};
    reqHead.syncFolderPath = {syncFolder.empty() ? nullptr : syncFolder.data(), syncFolder.length()};
    reqHead.reqKey = {nullptr, 0};
    CloudDiskPathInfo pathInfo{filePath.empty() ? nullptr : filePath.data(), filePath.length()};
    return PlaceholderCallbackManager::GetInstance().DispatchDehydrate(
        context.bundleName, context.syncFolderIndex, reqHead, pathInfo);
}

static int32_t DehydrateOpenFile(int fd)
{
    struct stat fileStat = {};
    if (fstat(fd, &fileStat) < 0) {
        int32_t error = errno;
        LOGE("Get file size before dehydration failed, errno:%{public}d", error);
        return ConvertErrnoToCloudDiskError(error);
    }
    if (ftruncate(fd, 0) < 0) {
        int32_t error = errno;
        LOGE("Truncate file before dehydration failed, errno:%{public}d", error);
        return ConvertErrnoToCloudDiskError(error);
    }

    bool extendSucceeded = false;
    for (int32_t attempt = 0; attempt < DEHYDRATE_EXTEND_MAX_ATTEMPTS; ++attempt) {
        if (ftruncate(fd, fileStat.st_size) == 0) {
            extendSucceeded = true;
            break;
        }
        LOGE("Extend placeholder after dehydration failed, attempt:%{public}d, errno:%{public}d",
             attempt + 1, errno);
    }
    if (!extendSucceeded) {
        return E_TRY_AGAIN;
    }

    uint8_t stateBeforeWrite = PLACEHOLDER_STATE_NONE;
    int32_t ret = SetFilePlaceholderState(fd, PLACEHOLDER_STATE_UNHYDRATED, stateBeforeWrite);
    if (ret != E_OK) {
        LOGE("Set unhydrated state failed, errno:%{public}d", ret);
        return ConvertErrnoToCloudDiskError(ret);
    }
    if (fsync(fd) < 0) {
        LOGW("Fsync dehydrated placeholder failed, errno:%{public}d", errno);
    }
    return E_OK;
}

static int32_t DehydratePlaceholderFile(const PlaceholderStatePathContext &context,
                                        const std::string &relativePath)
{
    std::lock_guard<std::mutex> lock(GetDehydrateFileMutex(context.hmdfsPath));
    if (PlaceholderTaskManager::GetInstance().HasActiveTask(
        context.syncFolder, relativePath, context.syncFolderIndex)) {
        return E_HYDRATE_IN_PROGRESS;
    }
    int32_t ret = CheckPathNotDir(context.hmdfsPath);
    if (ret != E_OK) {
        return ret;
    }

    UniqueFd fd(open(context.hmdfsPath.c_str(), O_RDWR | O_CLOEXEC | O_NOFOLLOW));
    if (fd < 0) {
        int32_t error = errno;
        LOGE("Open file for dehydration failed, errno:%{public}d", error);
        return ConvertErrnoToCloudDiskError(error);
    }

    uint8_t state = PLACEHOLDER_STATE_NONE;
    ret = GetFilePlaceholderState(fd, state);
    if (ret != E_OK) {
        LOGE("Get placeholder state before dehydration failed, errno:%{public}d", ret);
        return ConvertErrnoToCloudDiskError(ret);
    }
    ret = CheckDehydrateState(state);
    if (ret != E_OK || state == PLACEHOLDER_STATE_UNHYDRATED) {
        return ret;
    }

    ret = DispatchDehydrateAuthorization(context, relativePath);
    if (ret != E_OK) {
        return ret;
    }

    return DehydrateOpenFile(fd);
}

enum class PlaceholderStateTransition {
    MARK,
    UNMARK,
};

static int32_t ChangePlaceholderStateOnly(const PlaceholderStatePathContext &context,
                                          PlaceholderStateTransition transition)
{
    int32_t ret = CheckPathNotDir(context.hmdfsPath);
    if (ret != E_OK) {
        return ret;
    }

    UniqueFd fd(open(context.hmdfsPath.c_str(), O_RDWR | O_CLOEXEC | O_NOFOLLOW));
    if (fd < 0) {
        int32_t error = errno;
        LOGE("Open file for placeholder state transition failed, errno:%{public}d", error);
        return ConvertErrnoToCloudDiskError(error);
    }

    uint8_t oldState = PLACEHOLDER_STATE_NONE;
    ret = GetFilePlaceholderState(fd, oldState);
    if (ret != E_OK) {
        LOGE("Get placeholder state failed, errno:%{public}d", ret);
        return ConvertErrnoToCloudDiskError(ret);
    }

    uint8_t newState = PLACEHOLDER_STATE_NONE;
    int32_t delta = -1;
    if (transition == PlaceholderStateTransition::MARK) {
        if (IsPlaceholderState(oldState)) {
            return E_IS_A_PLACEHOLDER;
        }
        newState = PLACEHOLDER_STATE_FULLY_HYDRATED;
        delta = 1;
    } else {
        if (!IsPlaceholderState(oldState)) {
            return E_NOT_A_PLACEHOLDER;
        }
        if (oldState != PLACEHOLDER_STATE_FULLY_HYDRATED) {
            return E_PLACEHOLDER_NOT_FULLY_HYDRATED;
        }
    }

    uint8_t stateBeforeWrite = PLACEHOLDER_STATE_NONE;
    ret = SetFilePlaceholderState(fd, newState, stateBeforeWrite);
    if (ret != E_OK) {
        LOGE("Set placeholder state failed, errno:%{public}d", ret);
        return ConvertErrnoToCloudDiskError(ret);
    }
    (void)RefreshAncestorPlaceholderCount(context.mntSyncFolder, context.hmdfsPath, delta);
    (void)UpdateDentryPlaceholderState(context.userId, context.syncFolderIndex, context.hmdfsPath, newState);
    return E_OK;
}

static int32_t CheckPlaceHolderXattr(int32_t fd, const std::string &hmdfsPath)
{
    uint8_t placeholderState = PLACEHOLDER_STATE_NONE;
    int32_t ret = GetFilePlaceholderState(fd, placeholderState);
    if (ret != E_OK) {
        LOGE("get placeholder state failed, path:%{public}s, errno:%{public}d",
             GetAnonyStringStrictly(hmdfsPath).c_str(), ret);
        return ConvertErrnoToCloudDiskError(ret);
    }
    if (!IsPlaceholderState(placeholderState)) {
        LOGE("not a placeholder, path:%{public}s", GetAnonyStringStrictly(hmdfsPath).c_str());
        return E_NOT_A_PLACEHOLDER;
    }
    return E_OK;
}

static int32_t ConvertPlaceholderToEmptyFile(const std::string &hmdfsPath,
                                             const std::string &mntSyncFolder = "",
                                             int32_t userId = -1,
                                             uint32_t syncFolderIndex = 0)
{
    int32_t ret = 0;
    int fd = -1;

    if ((ret = CheckPathNotDir(hmdfsPath)) != E_OK) {
        LOGE("check dir failed, path:%{public}s, errno:%{public}d", GetAnonyStringStrictly(hmdfsPath).c_str(), ret);
        return ret;
    }

    do {
        fd = open(hmdfsPath.c_str(), O_RDWR | O_CLOEXEC | O_NOFOLLOW);
        if (fd < 0) {
            ret = errno;
            LOGE("open failed, path:%{public}s, errno:%{public}d", GetAnonyStringStrictly(hmdfsPath).c_str(), ret);
            ret = ConvertErrnoToCloudDiskError(ret);
            break;
        }

        if ((ret = CheckPlaceHolderXattr(fd, hmdfsPath)) != E_OK) {
            break;
        }

        if (ftruncate(fd, 0) < 0) {
            ret = errno;
            LOGE("ftruncate failed, path:%{public}s, errno:%{public}d", GetAnonyStringStrictly(hmdfsPath).c_str(), ret);
            ret = ConvertErrnoToCloudDiskError(ret);
            break;
        }

        uint8_t oldPlaceholderState = PLACEHOLDER_STATE_NONE;
        ret = SetFilePlaceholderState(fd, PLACEHOLDER_STATE_NONE, oldPlaceholderState);
        if (ret != E_OK) {
            LOGE("set placeholder state failed, path:%{public}s, errno:%{public}d",
                 GetAnonyStringStrictly(hmdfsPath).c_str(), ret);
            ret = ConvertErrnoToCloudDiskError(ret);
            break;
        }
        if (IsPlaceholderState(oldPlaceholderState) && !mntSyncFolder.empty()) {
            (void)RefreshAncestorPlaceholderCount(mntSyncFolder, hmdfsPath, -1);
            if (userId >= 0) {
                (void)UpdateDentryPlaceholderState(userId, syncFolderIndex, hmdfsPath, PLACEHOLDER_STATE_NONE);
            }
        }
    } while (0);

    if (fd >= 0) {
        close(fd);
    }

    return ret;
}

int32_t CloudDiskService::ConvertPlaceholderToFileInner(const std::string &syncFolder, const std::string &relativePath)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    LOGI("Begin ConvertPlaceholderToFileInner");

    if (syncFolder.empty() || relativePath.empty()) {
        LOGE("Invalid parameter");
        return E_INVALID_ARG;
    }

    int32_t userId = CloudDiskServiceAccessToken::GetUserId();
    if (userId == 0) {
        CloudDiskServiceAccessToken::GetAccountId(userId);
    }
    std::string bundleName;
    int32_t ret = CloudDiskServiceAccessToken::GetCallerBundleName(bundleName);
    if (ret != E_OK) {
        LOGE("Get bundleName failed, ret:%{public}d", ret);
        return E_TRY_AGAIN;
    }

    // 步骤1：权限校验
    ret = CheckSyncFolderBundleName(syncFolder, userId, bundleName);
    if (ret != E_OK) {
        LOGE("CheckSyncFolderAccess failed, ret:%{public}d", ret);
        return ret;
    }

    // 步骤2：获取hmdfs路径
    std::string hmdfsPath;
    ret = GetHmdfsPath(syncFolder, relativePath, userId, hmdfsPath);
    if (ret != E_OK) {
        return ret;
    }

    std::string mntSyncFolder;
    uint32_t syncFolderIndex = 0;
    ret = GetSyncRootContext(syncFolder, userId, mntSyncFolder, syncFolderIndex);
    if (ret != E_OK) {
        return ret;
    }

    ret = ConvertPlaceholderToEmptyFile(hmdfsPath, mntSyncFolder, userId, syncFolderIndex);
    if (ret != 0) {
        LOGE("Convert failed, ret:%{public}d", ret);
        return ret;
    }

    LOGI("Convert success");
    return E_OK;
#else
    return E_NOT_SUPPORTED;
#endif
}

int32_t CloudDiskService::MarkFileAsPlaceholderInner(const std::string &syncFolder, const std::string &relativePath)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    if (syncFolder.empty() || relativePath.empty()) {
        LOGE("MarkFileAsPlaceholderInner invalid parameter");
        return E_INVALID_ARG;
    }
    PlaceholderStatePathContext context;
    int32_t ret = ResolvePlaceholderStatePath(syncFolder, relativePath, context);
    if (ret != E_OK) {
        return ret;
    }
    return ChangePlaceholderStateOnly(context, PlaceholderStateTransition::MARK);
#else
    return E_NOT_SUPPORTED;
#endif
}

int32_t CloudDiskService::UnmarkPlaceholderFileInner(const std::string &syncFolder, const std::string &relativePath)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    if (syncFolder.empty() || relativePath.empty()) {
        LOGE("UnmarkPlaceholderFileInner invalid parameter");
        return E_INVALID_ARG;
    }
    PlaceholderStatePathContext context;
    int32_t ret = ResolvePlaceholderStatePath(syncFolder, relativePath, context);
    if (ret != E_OK) {
        return ret;
    }
    return ChangePlaceholderStateOnly(context, PlaceholderStateTransition::UNMARK);
#else
    return E_NOT_SUPPORTED;
#endif
}

static int32_t ValidateHydrationPath(const PlaceholderStatePathContext &context)
{
    std::array<char, PATH_MAX + 1> rootPath{};
    std::array<char, PATH_MAX + 1> filePath{};
    if (realpath(context.mntSyncFolder.c_str(), rootPath.data()) == nullptr ||
        realpath(context.hmdfsPath.c_str(), filePath.data()) == nullptr) {
        int32_t error = errno;
        LOGE("Resolve hydration path failed, errno:%{public}d", error);
        return ConvertErrnoToCloudDiskError(error);
    }
    if (!IsPathInSyncFolder(rootPath.data(), filePath.data())) {
        LOGE("Hydration path escapes sync folder");
        return E_INVALID_ARG;
    }
    return E_OK;
}

static int32_t CreateHydrationTask(const PlaceholderStatePathContext &context,
                                   const std::string &relativePath,
                                   CloudDiskHydratePriority priority)
{
    std::lock_guard<std::mutex> lock(GetDehydrateFileMutex(context.hmdfsPath));
    auto &taskManager = PlaceholderTaskManager::GetInstance();
    if (taskManager.HasActiveTask(context.syncFolder, relativePath, context.syncFolderIndex)) {
        return E_HYDRATE_IN_PROGRESS;
    }
    int32_t ret = ValidateHydrationPath(context);
    if (ret != E_OK) {
        return ret;
    }
    ret = CheckPathNotDir(context.hmdfsPath);
    if (ret != E_OK) {
        return ret;
    }

    UniqueFd outputFd(open(context.hmdfsPath.c_str(), O_RDWR | O_CLOEXEC | O_NOFOLLOW));
    if (outputFd < 0) {
        int32_t error = errno;
        LOGE("Open file for hydration failed, errno:%{public}d", error);
        return ConvertErrnoToCloudDiskError(error);
    }
    uint8_t placeholderState = PLACEHOLDER_STATE_NONE;
    ret = GetFilePlaceholderState(outputFd, placeholderState);
    if (ret != E_OK) {
        return ConvertErrnoToCloudDiskError(ret);
    }
    if (placeholderState == PLACEHOLDER_STATE_NONE) {
        return E_NOT_A_PLACEHOLDER;
    }
    if (placeholderState == PLACEHOLDER_STATE_FULLY_HYDRATED) {
        return E_ALREADY_HYDRATED;
    }

    PlaceholderTaskManager::RequestKey reqKey;
    return taskManager.CreateHydrateTask(context.syncFolder,
                                         relativePath,
                                         context.bundleName,
                                         context.syncFolderIndex,
                                         priority,
                                         std::move(outputFd),
                                         reqKey,
                                         {context.userId, context.absolutePath});
}

int32_t CloudDiskService::StartHydrationInner(const std::string &syncFolder,
                                              const std::string &relativePath,
                                              int32_t priority)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    if (syncFolder.empty() || relativePath.empty() ||
        priority < static_cast<int32_t>(CLOUD_DISK_HYDRATE_PRIORITY_LOW) ||
        priority > static_cast<int32_t>(CLOUD_DISK_HYDRATE_PRIORITY_HIGH)) {
        return E_INVALID_ARG;
    }
    PlaceholderStatePathContext context;
    int32_t ret = ResolvePlaceholderStatePath(syncFolder, relativePath, context);
    if (ret != E_OK) {
        return ret;
    }
    return CreateHydrationTask(context, relativePath, static_cast<CloudDiskHydratePriority>(priority));
#else
    return E_NOT_SUPPORTED;
#endif
}

int32_t CloudDiskService::CancelHydrationInner(const std::string &syncFolder, const std::string &relativePath)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    if (syncFolder.empty() || relativePath.empty()) {
        return E_INVALID_ARG;
    }
    PlaceholderStatePathContext context;
    int32_t ret = ResolvePlaceholderStatePath(syncFolder, relativePath, context);
    if (ret != E_OK) {
        return ret;
    }
    return PlaceholderTaskManager::GetInstance().CancelTask(
        context.syncFolder, relativePath, context.syncFolderIndex);
#else
    return E_NOT_SUPPORTED;
#endif
}

int32_t CloudDiskService::ExecuteInner(const CallbackExecuteRequest &request)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    if (request.reqKey.empty()) {
        LOGE("Empty Execute request key");
        return E_INVALID_ARG;
    }
    std::string bundleName;
    std::string physicalPath;
    uint32_t syncFolderIndex = 0;
    int32_t ret = ResolveOwnedSyncFolder(request.syncFolder, bundleName, syncFolderIndex, physicalPath);
    if (ret != E_OK) {
        LOGE("Resolve Execute sync folder failed, ret:%{private}d", ret);
        return E_CALLBACK_NOT_REGISTERED;
    }
    if (!PlaceholderCallbackManager::GetInstance().IsCallbackRegistered(bundleName, syncFolderIndex)) {
        LOGE("Execute callback table is not registered");
        return E_CALLBACK_NOT_REGISTERED;
    }
    return PlaceholderTaskManager::GetInstance().Execute(bundleName, syncFolderIndex, request);
#else
    return E_NOT_SUPPORTED;
#endif
}

int32_t CloudDiskService::DehydrateInner(const std::string &syncFolder, const std::string &relativePath)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    if (syncFolder.empty() || relativePath.empty()) {
        LOGE("DehydrateInner invalid parameter");
        return E_INVALID_ARG;
    }
    PlaceholderStatePathContext context;
    int32_t ret = ResolvePlaceholderStatePath(syncFolder, relativePath, context);
    if (ret != E_OK) {
        return ret;
    }
    return DehydratePlaceholderFile(context, relativePath);
#else
    return E_NOT_SUPPORTED;
#endif
}

#ifdef SUPPORT_CLOUD_DISK_SERVICE
static int32_t CheckSystemAccessorPermission()
{
    int32_t ret = CheckPermissions(PERM_CLOUD_DISK_SERVICE, true);
    if (ret != E_OK) {
        return ret;
    }
    if (!CloudDiskServiceAccessToken::IsSystemApp()) {
        LOGE("CloudDiskSystemAccessor requires a system application");
        return E_PERMISSION_SYSTEM;
    }
    return E_OK;
}

static int32_t ResolveSystemAccessorUser(int32_t &userId)
{
    userId = CloudDiskServiceAccessToken::GetUserId();
    if (userId == 0 && CloudDiskServiceAccessToken::GetAccountId(userId) != E_OK) {
        return E_CALLBACK_NOT_REGISTERED;
    }
    return userId >= 0 ? E_OK : E_CALLBACK_NOT_REGISTERED;
}

static bool IsValidSystemAccessorPath(const std::string &path)
{
    constexpr const char *SANDBOX_PREFIX = "/storage/Users/currentUser/";
    if (path.empty() || path.size() > PATH_MAX || path.find('\0') != std::string::npos ||
        path.compare(0, std::char_traits<char>::length(SANDBOX_PREFIX), SANDBOX_PREFIX) != 0 ||
        path.find("/../") != std::string::npos || path.find("/./") != std::string::npos ||
        path.back() == '/' || path.substr(path.find_last_of('/') + 1) == ".." ||
        path.substr(path.find_last_of('/') + 1) == ".") {
        return false;
    }
    return true;
}

static int32_t ResolveSystemAccessorPath(const std::string &path, PlaceholderStatePathContext &context,
    std::string &relativePath)
{
    if (!IsValidSystemAccessorPath(path)) {
        return E_INVALID_ARG;
    }
    int32_t ret = ResolveSystemAccessorUser(context.userId);
    if (ret != E_OK) {
        return ret;
    }
    auto &folders = CloudDiskSyncFolder::GetInstance();
    std::string physicalPath;
    std::string user = std::to_string(context.userId);
    ret = folders.PathToPhysicalPath(path, user, physicalPath);
    const std::string userRoot = "/data/service/el2/" + user + "/hmdfs/account/files/Docs";
    if (ret != E_OK || !IsPathInSyncFolder(userRoot, physicalPath)) {
        LOGE("Resolve system accessor path failed, ret:%{private}d", ret);
        return E_CALLBACK_NOT_REGISTERED;
    }
    SyncFolderValue selected;
    for (const auto &[index, folder] : folders.GetSyncFolderMap()) {
        if (!folder.bundleName.empty() && folder.path.size() > selected.path.size() &&
            IsPathInSyncFolder(userRoot, folder.path) && IsPathInSyncFolder(folder.path, physicalPath)) {
            selected = folder;
            context.syncFolderIndex = index;
        }
    }
    if (selected.path.empty() ||
        !folders.PathToSandboxPathByPhysicalPath(selected.path, user, context.syncFolder)) {
        LOGE("No registered sync folder for system accessor");
        return E_CALLBACK_NOT_REGISTERED;
    }
    context.absolutePath = path;
    context.bundleName = selected.bundleName;
    relativePath = physicalPath.substr(selected.path.size() + 1);
    ret = GetHmdfsPath(context.syncFolder, relativePath, context.userId, context.hmdfsPath);
    if (ret == E_OK) {
        ret = folders.PathToMntPathBySandboxPath(context.syncFolder, user, context.mntSyncFolder);
    }
    if (ret != E_OK) {
        LOGE("Resolve system accessor mount failed, ret:%{private}d", ret);
        return E_CALLBACK_NOT_REGISTERED;
    }
    return E_OK;
}
#endif

int32_t CloudDiskService::StartHydrationByPathInner(const std::string &path, int32_t callbackType, int32_t priority)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    int32_t ret = CheckSystemAccessorPermission();
    if (ret != E_OK) {
        return ret;
    }
    if (callbackType < static_cast<int32_t>(CloudDiskCallbackType::FETCH_DATA) ||
        callbackType > static_cast<int32_t>(CloudDiskCallbackType::CANCEL_FETCH_DATA) ||
        priority < static_cast<int32_t>(CLOUD_DISK_HYDRATE_PRIORITY_LOW) ||
        priority > static_cast<int32_t>(CLOUD_DISK_HYDRATE_PRIORITY_HIGH)) {
        return E_INVALID_ARG;
    }
    PlaceholderStatePathContext context;
    std::string relativePath;
    ret = ResolveSystemAccessorPath(path, context, relativePath);
    if (ret != E_OK) {
        return ret;
    }
    if (!PlaceholderCallbackManager::GetInstance().IsCallbackRegistered(context.bundleName, context.syncFolderIndex)) {
        return E_CALLBACK_NOT_REGISTERED;
    }
    if (callbackType == static_cast<int32_t>(CloudDiskCallbackType::CANCEL_FETCH_DATA)) {
        return PlaceholderTaskManager::GetInstance().CancelTask(
            context.syncFolder, relativePath, context.syncFolderIndex);
    }
    return CreateHydrationTask(context, relativePath, static_cast<CloudDiskHydratePriority>(priority));
#else
    return E_NOT_SUPPORTED;
#endif
}

int32_t CloudDiskService::DehydrateFileByPathInner(const std::string &path)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    int32_t ret = CheckSystemAccessorPermission();
    if (ret != E_OK) {
        return ret;
    }
    PlaceholderStatePathContext context;
    std::string relativePath;
    ret = ResolveSystemAccessorPath(path, context, relativePath);
    return ret == E_OK ? DehydratePlaceholderFile(context, relativePath) : ret;
#else
    return E_NOT_SUPPORTED;
#endif
}

int32_t CloudDiskService::RegisterProgressCallbackInner(const sptr<IRemoteObject> &callback)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    int32_t ret = CheckSystemAccessorPermission();
    if (ret != E_OK) {
        return ret;
    }
    int32_t userId = -1;
    ret = ResolveSystemAccessorUser(userId);
    if (ret != E_OK) {
        return ret;
    }
    auto proxy = iface_cast<ICloudDiskProgressCallback>(callback);
    return PlaceholderProgressManager::GetInstance().Register(
        {IPCSkeleton::GetCallingFullTokenID(), IPCSkeleton::GetCallingPid()}, userId, proxy);
#else
    return E_NOT_SUPPORTED;
#endif
}

int32_t CloudDiskService::UnregisterProgressCallbackInner()
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    int32_t ret = CheckSystemAccessorPermission();
    if (ret != E_OK) {
        return ret;
    }
    return PlaceholderProgressManager::GetInstance().Unregister(
        {IPCSkeleton::GetCallingFullTokenID(), IPCSkeleton::GetCallingPid()});
#else
    return E_NOT_SUPPORTED;
#endif
}

struct PlaceholderUpdateContext {
    std::string mntSyncFolder;
    int32_t userId = -1;
    uint32_t syncFolderIndex = 0;
};

static int32_t UpdatePlaceholderAttr(const std::string &hmdfsPath,
                                     const PlaceholderInfo &metaData,
                                     const PlaceholderUpdateContext &context = {},
                                     const PlaceholderCustomInfo &customInfo = {})
{
    int32_t ret = E_OK;
    int fd = -1;

    if ((ret = CheckPathNotDir(hmdfsPath)) != E_OK) {
        LOGE("check dir failed, path:%{public}s, errno:%{public}d", GetAnonyStringStrictly(hmdfsPath).c_str(), ret);
        return ret;
    }

    do {
        fd = open(hmdfsPath.c_str(), O_RDWR | O_CLOEXEC | O_NOFOLLOW);
        if (fd < 0) {
            ret = errno;
            LOGE("open failed, path:%{public}s, errno:%{public}d", GetAnonyStringStrictly(hmdfsPath).c_str(), ret);
            ret = ConvertErrnoToCloudDiskError(ret);
            break;
        }

        if (ftruncate(fd, 0) < 0) {
            ret = errno;
            LOGE("ftruncate to zero failed, path:%{public}s, errno:%{public}d",
                 GetAnonyStringStrictly(hmdfsPath).c_str(), ret);
            ret = ConvertErrnoToCloudDiskError(ret);
            break;
        }

        uint8_t oldPlaceholderState = PLACEHOLDER_STATE_NONE;
        if ((ret = SetPlaceholderFileAttributes(fd, metaData, &oldPlaceholderState, customInfo)) != E_OK) {
            LOGE("set placeholder file attr failed, path:%{public}s, errno:%{public}d",
                 GetAnonyStringStrictly(hmdfsPath).c_str(), ret);
            ret = ConvertErrnoToCloudDiskError(ret);
            break;
        }
        if (!IsPlaceholderState(oldPlaceholderState) && !context.mntSyncFolder.empty()) {
            (void)RefreshAncestorPlaceholderCount(context.mntSyncFolder, hmdfsPath, 1);
        }
        if (context.userId >= 0) {
            (void)UpdateDentryPlaceholderState(
                context.userId, context.syncFolderIndex, hmdfsPath, PLACEHOLDER_STATE_UNHYDRATED);
        }
    } while (0);

    if (fd >= 0) {
        close(fd);
    }
    return ret;
}

int32_t CloudDiskService::UpdatePlaceholderInner(const std::string &syncFolder,
                                                 const std::string &relativePath,
                                                 const PlaceholderInfo &metaData,
                                                 const PlaceholderCustomInfo &customInfo)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    LOGI("Begin UpdatePlaceholderInner");

    if (syncFolder.empty() || relativePath.empty() || customInfo.data.size() > PLACEHOLDER_CUSTOM_INFO_MAX_SIZE) {
        LOGE("Invalid parameter");
        return E_INVALID_ARG;
    }

    int32_t userId = CloudDiskServiceAccessToken::GetUserId();
    if (userId == 0) {
        CloudDiskServiceAccessToken::GetAccountId(userId);
    }
    std::string bundleName;
    int32_t ret = CloudDiskServiceAccessToken::GetCallerBundleName(bundleName);
    if (ret != E_OK) {
        LOGE("Get bundleName failed, ret:%{public}d", ret);
        return E_TRY_AGAIN;
    }

    // 步骤1：权限校验
    ret = CheckSyncFolderBundleName(syncFolder, userId, bundleName);
    if (ret != E_OK) {
        LOGE("CheckSyncFolderAccess failed, ret:%{public}d", ret);
        return ret;
    }

    // 步骤2：获取hmdfs路径
    std::string hmdfsPath;
    ret = GetHmdfsPath(syncFolder, relativePath, userId, hmdfsPath);
    if (ret != E_OK) {
        return ret;
    }

    std::string mntSyncFolder;
    uint32_t syncFolderIndex = 0;
    ret = GetSyncRootContext(syncFolder, userId, mntSyncFolder, syncFolderIndex);
    if (ret != E_OK) {
        return ret;
    }

    ret = UpdatePlaceholderAttr(hmdfsPath, metaData, {mntSyncFolder, userId, syncFolderIndex}, customInfo);
    if (ret != 0) {
        LOGE("Update failed, ret:%{public}d", ret);
        return ret;
    }

    LOGI("Update success");
    return E_OK;
#else
    return E_NOT_SUPPORTED;
#endif
}

#ifdef SUPPORT_CLOUD_DISK_SERVICE
static int32_t ReadPlaceholderCustomInfo(int fd, PlaceholderCustomInfo &customInfo)
{
    ssize_t customInfoSize = fgetxattr(fd, CLOUD_DISK_CUSTOM_INFO_XATTR, nullptr, 0);
    if (customInfoSize < 0) {
        int32_t error = errno;
        if (error == ENODATA) {
            return E_PLACEHOLDER_CUSTOM_INFO_NOT_FOUND;
        }
        return error == ENOENT ? E_FILE_NOT_EXIST : ConvertErrnoToCloudDiskError(error);
    }
    if (customInfoSize > static_cast<ssize_t>(PLACEHOLDER_CUSTOM_INFO_MAX_SIZE)) {
        LOGE("Stored placeholder custom info is too large, size:%{public}zd", customInfoSize);
        return E_INVALID_ARG;
    }
    customInfo.data.resize(static_cast<size_t>(customInfoSize));
    if (customInfoSize == 0) {
        return E_OK;
    }
    ssize_t readSize = fgetxattr(fd, CLOUD_DISK_CUSTOM_INFO_XATTR, customInfo.data.data(), customInfo.data.size());
    if (readSize < 0) {
        int32_t error = errno;
        customInfo.data.clear();
        if (error == ENODATA) {
            return E_PLACEHOLDER_CUSTOM_INFO_NOT_FOUND;
        }
        return error == ENOENT ? E_FILE_NOT_EXIST : ConvertErrnoToCloudDiskError(error);
    }
    if (readSize != customInfoSize) {
        LOGE("Placeholder custom info changed while reading");
        customInfo.data.clear();
        return E_TRY_AGAIN;
    }
    return E_OK;
}
#endif

int32_t CloudDiskService::GetPlaceholderCustomInfoInner(const std::string &syncFolder,
                                                        const std::string &relativePath,
                                                        PlaceholderCustomInfo &customInfo)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    customInfo.data.clear();
    if (syncFolder.empty() || relativePath.empty()) {
        LOGE("GetPlaceholderCustomInfoInner invalid parameter");
        return E_INVALID_ARG;
    }

    int32_t userId = CloudDiskServiceAccessToken::GetUserId();
    if (userId == 0) {
        CloudDiskServiceAccessToken::GetAccountId(userId);
    }
    std::string bundleName;
    int32_t ret = CloudDiskServiceAccessToken::GetCallerBundleName(bundleName);
    if (ret != E_OK) {
        LOGE("Get bundleName failed, ret:%{public}d", ret);
        return E_TRY_AGAIN;
    }
    ret = CheckSyncFolderBundleName(syncFolder, userId, bundleName);
    if (ret != E_OK) {
        LOGE("CheckSyncFolderAccess failed, ret:%{public}d", ret);
        return ret;
    }

    std::string hmdfsPath;
    ret = GetHmdfsPath(syncFolder, relativePath, userId, hmdfsPath);
    if (ret != E_OK) {
        return ret == E_SYNC_FOLDER_PATH_NOT_EXIST ? E_FILE_NOT_EXIST : ret;
    }
    ret = CheckPathNotDir(hmdfsPath);
    if (ret != E_OK) {
        return ret == E_SYNC_FOLDER_PATH_NOT_EXIST ? E_FILE_NOT_EXIST : ret;
    }

    UniqueFd fd(open(hmdfsPath.c_str(), O_RDONLY | O_CLOEXEC | O_NOFOLLOW));
    if (fd < 0) {
        int32_t error = errno;
        LOGE("Open placeholder failed, errno:%{public}d", error);
        return error == ENOENT ? E_FILE_NOT_EXIST : ConvertErrnoToCloudDiskError(error);
    }
    ret = CheckPlaceHolderXattr(fd, hmdfsPath);
    if (ret != E_OK) {
        return ret;
    }

    return ReadPlaceholderCustomInfo(fd, customInfo);
#else
    return E_NOT_SUPPORTED;
#endif
}

} // namespace CloudDiskService
} // namespace FileManagement
} // namespace OHOS
