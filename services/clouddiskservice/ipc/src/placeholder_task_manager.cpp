/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "placeholder_task_manager.h"

#include <algorithm>
#include <cerrno>
#include <climits>
#include <fcntl.h>
#include <limits>
#include <memory>
#include <sys/stat.h>
#include <unistd.h>

#include "cloud_disk_service_error.h"
#include "cloud_disk_service_utils.h"
#include "placeholder_callback_manager.h"
#include "placeholder_helper.h"
#include "placeholder_progress_manager.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudDiskService {
namespace {
constexpr uint64_t INVALID_REQUEST_VALUE = 0;
constexpr uint64_t INITIAL_REQUEST_VALUE = 1;
constexpr size_t REQUEST_KEY_SIZE = sizeof(uint64_t);
constexpr size_t BITS_PER_BYTE = CHAR_BIT;
constexpr size_t MAX_CANCELLATION_RECORDS_PER_APP = 16;
constexpr size_t MAX_CANCELLATION_RECORDS_GLOBAL = 32;
constexpr auto HYDRATION_IDLE_TIMEOUT = std::chrono::minutes(5);
constexpr auto CANCELLATION_RECORD_TTL = std::chrono::seconds(60);

static_assert(CLOUD_DISK_MAX_ACTIVE_TASKS_PER_APP > 0);
static_assert(CLOUD_DISK_MAX_ACTIVE_TASKS_GLOBAL > 0);
static_assert(CLOUD_DISK_MAX_PENDING_TASKS_PER_APP > 0);
static_assert(CLOUD_DISK_MAX_PENDING_TASKS_GLOBAL > 0);
static_assert(CLOUD_DISK_MAX_ACTIVE_TASKS_PER_APP <= CLOUD_DISK_MAX_ACTIVE_TASKS_GLOBAL);
static_assert(CLOUD_DISK_MAX_PENDING_TASKS_PER_APP <= CLOUD_DISK_MAX_PENDING_TASKS_GLOBAL);

bool IsValidPriority(CloudDiskHydratePriority priority)
{
    return priority >= CLOUD_DISK_HYDRATE_PRIORITY_LOW && priority <= CLOUD_DISK_HYDRATE_PRIORITY_HIGH;
}

int32_t ConvertHydrationTargetError(int32_t error)
{
    return error == ENOENT ? E_FILE_NOT_EXIST : ConvertErrnoToCloudDiskError(error);
}

int32_t ConvertHydrationStateError(int32_t error)
{
    if (error == EINVAL || error == ERANGE) {
        return E_INVALID_PLACEHOLDER_STATE;
    }
    return ConvertHydrationTargetError(error);
}

int32_t ConvertHydrationIoError(int32_t error)
{
    switch (error) {
        case ENOSPC:
        case EDQUOT:
            return E_NO_SPACE_LEFT;
        case EFBIG:
            return E_FILE_TOO_LARGE;
        case EACCES:
            return E_ACCES;
        case EPERM:
            return E_PERM;
        default:
            return E_TRY_AGAIN;
    }
}

int32_t WriteHydrationData(int32_t fd, const CallbackExecuteRequest &request)
{
    size_t writtenSize = 0;
    while (writtenSize < request.data.size()) {
        ssize_t written = pwrite(fd, request.data.data() + writtenSize, request.data.size() - writtenSize,
                                 static_cast<off_t>(request.offset + writtenSize));
        if (written < 0) {
            int32_t error = errno;
            if (error == EINTR) {
                continue;
            }
            LOGE("Write hydration data failed, errno:%{public}d", error);
            return ConvertHydrationIoError(error);
        }
        if (written == 0) {
            LOGE("Hydration write made no progress");
            return E_TRY_AGAIN;
        }
        writtenSize += static_cast<size_t>(written);
    }
    return E_OK;
}

bool ShouldKeepCancellationRecord(PlaceholderTaskCancelReason reason)
{
    return reason != PlaceholderTaskCancelReason::DISPATCH_FAILED &&
           reason != PlaceholderTaskCancelReason::USER_SWITCH && reason != PlaceholderTaskCancelReason::SERVICE_STOP;
}

bool ShouldNotifyCancellation(PlaceholderTaskCancelReason reason)
{
    return reason == PlaceholderTaskCancelReason::USER_REQUEST || reason == PlaceholderTaskCancelReason::TIMEOUT ||
           reason == PlaceholderTaskCancelReason::INTERNAL_ERROR;
}

bool IsInvalidFetchDataRequest(const std::shared_ptr<PlaceholderTaskRecord> &task,
                               const CallbackExecuteRequest &request)
{
    constexpr uint64_t MAX_OFFSET = static_cast<uint64_t>(std::numeric_limits<off_t>::max());
    const bool invalidDataBounds = request.size != request.data.size() || request.size > MAX_EXECUTE_DATA_SIZE ||
                                   request.offset > request.totalSize ||
                                   request.size > request.totalSize - request.offset || request.offset > MAX_OFFSET ||
                                   request.size > MAX_OFFSET - request.offset;
    const bool invalidCompletion =
        (!request.isComplete && request.size == 0) ||
        (request.isComplete && request.totalSize != 0 && request.size == 0) ||
        (request.totalSize == 0 && (request.offset != 0 || request.size != 0 || !request.isComplete));
    const bool invalidTaskSize =
        task->cachedSize > request.totalSize || (task->totalSizeInitialized && task->totalSize != request.totalSize);
    return invalidDataBounds || invalidCompletion || invalidTaskSize;
}

using CanonicalPath = std::string;

int32_t ResolveHydrationPath(const std::string &syncRoot,
                             const std::string &path,
                             CanonicalPath &rootPath,
                             CanonicalPath &filePath)
{
    if (syncRoot.empty() || path.empty()) {
        return E_INVALID_ARG;
    }
    char resolvedRoot[PATH_MAX] = {'\0'};
    if (realpath(syncRoot.c_str(), resolvedRoot) == nullptr) {
        int32_t error = errno;
        LOGE("Resolve hydration root failed, errno:%{public}d", error);
        return ConvertErrnoToCloudDiskError(error);
    }
    rootPath = resolvedRoot;
    char resolvedFile[PATH_MAX] = {'\0'};
    if (realpath(path.c_str(), resolvedFile) == nullptr) {
        int32_t error = errno;
        LOGE("Resolve hydration target failed, errno:%{public}d", error);
        return ConvertHydrationTargetError(error);
    }
    filePath = resolvedFile;
    if (!IsPathInSyncFolder(rootPath, filePath)) {
        LOGE("Hydration path escapes sync folder");
        return E_INVALID_ARG;
    }
    return E_OK;
}

int32_t CheckHydrationState(int32_t fd)
{
    uint8_t state = PLACEHOLDER_STATE_NONE;
    int32_t ret = GetFilePlaceholderState(fd, state);
    if (ret != E_OK) {
        LOGE("Read placeholder state before hydration failed, errno:%{public}d", ret);
        return ConvertHydrationStateError(ret);
    }
    if (!IsValidPlaceholderState(state)) {
        LOGE("Invalid placeholder state before hydration, state:%{public}u", state);
        return E_INVALID_PLACEHOLDER_STATE;
    }
    if (state == PLACEHOLDER_STATE_NONE) {
        return E_NOT_A_PLACEHOLDER;
    }
    return state == PLACEHOLDER_STATE_FULLY_HYDRATED ? E_ALREADY_HYDRATED : E_OK;
}

int32_t OpenCanonicalHydrationFile(const CanonicalPath &rootPath, const CanonicalPath &filePath, UniqueFd &fd)
{
    UniqueFd currentFd(open(rootPath.c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW));
    if (currentFd < 0) {
        int32_t error = errno;
        LOGE("Open hydration root failed, errno:%{public}d", error);
        return ConvertHydrationTargetError(error);
    }

    size_t start = rootPath == "/" ? 1 : rootPath.size() + 1;
    while (start < filePath.size()) {
        size_t end = filePath.find('/', start);
        bool isLast = end == std::string::npos;
        std::string component = filePath.substr(start, isLast ? end : end - start);
        int32_t flags = isLast ? O_RDWR | O_CLOEXEC | O_NOFOLLOW : O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW;
        UniqueFd nextFd(openat(currentFd, component.c_str(), flags));
        if (nextFd < 0) {
            int32_t error = errno;
            LOGE("Open hydration path component failed, errno:%{public}d", error);
            return ConvertHydrationTargetError(error);
        }
        if (isLast) {
            fd = std::move(nextFd);
            return E_OK;
        }
        currentFd = std::move(nextFd);
        start = end + 1;
    }
    return E_INVALID_ARG;
}

int32_t OpenHydrationFile(const CanonicalPath &rootPath,
                          const CanonicalPath &filePath,
                          UniqueFd &fd,
                          HydrationFileMetadata &metadata)
{
    UniqueFd openedFd;
    int32_t ret = OpenCanonicalHydrationFile(rootPath, filePath, openedFd);
    if (ret != E_OK) {
        return ret;
    }
    struct stat fileStat {};
    if (fstat(openedFd, &fileStat) != 0) {
        int32_t error = errno;
        LOGE("Read hydration metadata failed, errno:%{public}d", error);
        return ConvertErrnoToCloudDiskError(error);
    }
    if (S_ISDIR(fileStat.st_mode) || fileStat.st_size < 0) {
        LOGE("Hydration target is a directory or has an invalid size");
        return E_INVALID_ARG;
    }
    ret = CheckHydrationState(openedFd);
    if (ret != E_OK) {
        return ret;
    }
    metadata.deviceId = static_cast<uint64_t>(fileStat.st_dev);
    metadata.inodeId = static_cast<uint64_t>(fileStat.st_ino);
    metadata.logicalSize = static_cast<uint64_t>(fileStat.st_size);
    fd = std::move(openedFd);
    return E_OK;
}
} // namespace

int32_t OpenValidatedHydrationFile(const std::string &syncRoot,
                                   const std::string &path,
                                   UniqueFd &fd,
                                   HydrationFileMetadata &metadata)
{
    CanonicalPath rootPath{};
    CanonicalPath filePath{};
    int32_t ret = ResolveHydrationPath(syncRoot, path, rootPath, filePath);
    return ret == E_OK ? OpenHydrationFile(rootPath, filePath, fd, metadata) : ret;
}

PlaceholderTaskManager &PlaceholderTaskManager::GetInstance()
{
    static PlaceholderTaskManager instance;
    return instance;
}

void PlaceholderTaskManager::StartScheduler()
{
    {
        std::lock_guard<std::mutex> lifecycleLock(lifecycleMutex_);
        std::lock_guard<std::mutex> lock(mapMutex_);
        if (running_) {
            return;
        }
        running_ = true;
        stopping_ = false;
        monitorHandles_.emplace_back(ffrt::submit_h([this] { DeadlineLoop(); }));
    }
    ScheduleDispatch();
}

void PlaceholderTaskManager::StopScheduler()
{
    std::lock_guard<std::mutex> lifecycleLock(lifecycleMutex_);
    std::vector<ffrt::task_handle> monitorHandles;
    std::vector<std::shared_ptr<PlaceholderTaskRecord>> tasks;
    {
        std::lock_guard<std::mutex> lock(mapMutex_);
        running_ = false;
        stopping_ = true;
        for (const auto &[reqKey, task] : taskMap_) {
            (void)reqKey;
            tasks.push_back(task);
        }
        monitorHandles.swap(monitorHandles_);
    }
    deadlineCv_.notify_all();
    for (const auto &task : tasks) {
        std::lock_guard<std::mutex> lock(task->mutex);
        CancelTaskRecordLocked(task, PlaceholderTaskCancelReason::SERVICE_STOP);
    }
    auto dispatchBarrier = dispatchQueue_.submit_h([] {});
    dispatchQueue_.wait(dispatchBarrier);
    for (const auto &handle : monitorHandles) {
        ffrt::wait({handle});
    }
    std::lock_guard<std::mutex> lock(mapMutex_);
    deadlineMap_.clear();
    cancellationRecordMap_.clear();
    cancellationOrder_.clear();
    dispatchScheduled_ = false;
    stopping_ = false;
}

PlaceholderTaskManager::RequestKey PlaceholderTaskManager::GenerateRequestKeyLocked()
{
    PurgeExpiredCancellationRecordsLocked();
    do {
        if (nextReqKeyValue_ == INVALID_REQUEST_VALUE) {
            nextReqKeyValue_ = INITIAL_REQUEST_VALUE;
        }
        uint64_t requestValue = nextReqKeyValue_;
        nextReqKeyValue_ =
            requestValue == std::numeric_limits<uint64_t>::max() ? INITIAL_REQUEST_VALUE : requestValue + 1;
        RequestKey reqKey(REQUEST_KEY_SIZE);
        for (size_t index = 0; index < reqKey.size(); ++index) {
            reqKey[index] = static_cast<uint8_t>(requestValue >> (index * BITS_PER_BYTE));
        }
        if (taskMap_.find(reqKey) == taskMap_.end() &&
            cancellationRecordMap_.find(reqKey) == cancellationRecordMap_.end()) {
            return reqKey;
        }
    } while (nextReqKeyValue_ != INITIAL_REQUEST_VALUE);
    return {};
}

int32_t PlaceholderTaskManager::PrepareHydrateTaskLocked(const std::string &syncFolder,
                                                         const std::string &filePath,
                                                         const std::string &bundleName,
                                                         uint32_t syncFolderIndex,
                                                         RequestKey &reqKey)
{
    if (stopping_) {
        LOGW("Reject hydrate task while scheduler is stopping");
        return E_TRY_AGAIN;
    }
    for (const auto &[key, task] : taskMap_) {
        (void)key;
        if (task->syncFolder == syncFolder && task->filePath == filePath && task->syncFolderIndex == syncFolderIndex &&
            (task->state == PlaceholderTaskState::PENDING || task->state == PlaceholderTaskState::IN_PROGRESS)) {
            LOGW("Reject duplicate active hydrate task");
            return E_HYDRATE_IN_PROGRESS;
        }
    }

    size_t appPendingCount = 0;
    size_t globalPendingCount = 0;
    for (const auto &[key, task] : taskMap_) {
        (void)key;
        if (task->state != PlaceholderTaskState::PENDING) {
            continue;
        }
        ++globalPendingCount;
        if (task->bundleName == bundleName) {
            ++appPendingCount;
        }
    }
    if (appPendingCount >= CLOUD_DISK_MAX_PENDING_TASKS_PER_APP ||
        globalPendingCount >= CLOUD_DISK_MAX_PENDING_TASKS_GLOBAL) {
        LOGW(
            "Reject hydrate task because pending task limit was reached, "
            "app:%{public}zu, global:%{public}zu",
            appPendingCount, globalPendingCount);
        return E_HYDRATION_TASK_LIMIT_REACHED;
    }

    if (nextCreateSeq_ == std::numeric_limits<uint64_t>::max()) {
        if (!taskMap_.empty()) {
            LOGE("Hydration queue sequence exhausted");
            return E_TRY_AGAIN;
        }
        nextCreateSeq_ = INITIAL_REQUEST_VALUE;
    }
    reqKey = GenerateRequestKeyLocked();
    if (reqKey.empty()) {
        LOGE("Hydration request key space exhausted");
        return E_TRY_AGAIN;
    }
    return E_OK;
}

int32_t PlaceholderTaskManager::CreateHydrateTask(const std::string &syncFolder,
                                                  const std::string &filePath,
                                                  const std::string &bundleName,
                                                  uint32_t syncFolderIndex,
                                                  CloudDiskHydratePriority priority,
                                                  UniqueFd validationFd,
                                                  RequestKey &reqKey,
                                                  const PlaceholderProgressContext &progressContext)
{
    reqKey.clear();
    if (syncFolder.empty() || filePath.empty() || bundleName.empty() || validationFd < 0 ||
        !IsValidPriority(priority)) {
        LOGE("Invalid hydrate task context");
        return E_INVALID_ARG;
    }
    struct stat metadata {};
    if (fstat(validationFd, &metadata) != 0 || metadata.st_size < 0) {
        LOGE("Read hydration metadata failed, errno:%{public}d", errno);
        return E_TRY_AGAIN;
    }

    {
        std::lock_guard<std::mutex> lock(mapMutex_);
        int32_t ret = PrepareHydrateTaskLocked(syncFolder, filePath, bundleName, syncFolderIndex, reqKey);
        if (ret != E_OK) {
            LOGW("Prepare hydrate task failed, ret:%{public}d", ret);
            return ret;
        }
        auto task = std::make_shared<PlaceholderTaskRecord>();
        if (task == nullptr) {
            return E_TRY_AGAIN;
        }
        task->reqKey = reqKey;
        task->syncFolder = syncFolder;
        task->filePath = filePath;
        task->bundleName = bundleName;
        task->syncFolderIndex = syncFolderIndex;
        task->priority = priority;
        task->absolutePath =
            progressContext.absolutePath.empty() ? syncFolder + "/" + filePath : progressContext.absolutePath;
        task->hmdfsPath = progressContext.hmdfsPath;
        task->mountSyncFolder = progressContext.mountSyncFolder;
        task->userId = progressContext.userId;
        task->deviceId = static_cast<uint64_t>(metadata.st_dev);
        task->inodeId = static_cast<uint64_t>(metadata.st_ino);
        task->totalSize = static_cast<uint64_t>(metadata.st_size);
        task->createSeq = nextCreateSeq_++;
        taskMap_[reqKey] = task;
        NotifyProgressLocked(task);
    }
    ScheduleDispatch();
    return E_OK;
}

bool PlaceholderTaskManager::HasOutstandingTask(const std::string &syncFolder,
                                                const std::string &filePath,
                                                uint32_t syncFolderIndex)
{
    std::lock_guard<std::mutex> lock(mapMutex_);
    for (const auto &[reqKey, task] : taskMap_) {
        (void)reqKey;
        if (task->syncFolder == syncFolder && task->filePath == filePath && task->syncFolderIndex == syncFolderIndex &&
            (task->state == PlaceholderTaskState::PENDING || task->state == PlaceholderTaskState::IN_PROGRESS)) {
            return true;
        }
    }
    return false;
}

int32_t PlaceholderTaskManager::CancelTask(const std::string &syncFolder,
                                           const std::string &filePath,
                                           uint32_t syncFolderIndex)
{
    std::vector<std::shared_ptr<PlaceholderTaskRecord>> tasks;
    {
        std::lock_guard<std::mutex> lock(mapMutex_);
        for (const auto &[reqKey, task] : taskMap_) {
            (void)reqKey;
            if (task->syncFolder == syncFolder && task->filePath == filePath &&
                task->syncFolderIndex == syncFolderIndex) {
                tasks.push_back(task);
            }
        }
    }
    bool cancelled = false;
    for (const auto &task : tasks) {
        std::lock_guard<std::mutex> lock(task->mutex);
        if (task->state == PlaceholderTaskState::PENDING || task->state == PlaceholderTaskState::IN_PROGRESS) {
            CancelTaskRecordLocked(task, PlaceholderTaskCancelReason::USER_REQUEST);
            cancelled = true;
        }
    }
    ScheduleDispatch();
    if (!cancelled) {
        LOGW("Cancel hydrate task failed: no outstanding task");
    }
    return cancelled ? E_OK : E_NO_HYDRATION_IN_PROGRESS;
}

void PlaceholderTaskManager::CancelTasksBySyncFolder(const std::string &bundleName,
                                                     uint32_t syncFolderIndex,
                                                     PlaceholderTaskCancelReason reason)
{
    std::vector<std::shared_ptr<PlaceholderTaskRecord>> tasks;
    {
        std::lock_guard<std::mutex> lock(mapMutex_);
        for (const auto &[reqKey, task] : taskMap_) {
            (void)reqKey;
            if (task->bundleName == bundleName && task->syncFolderIndex == syncFolderIndex) {
                tasks.push_back(task);
            }
        }
    }
    for (const auto &task : tasks) {
        std::lock_guard<std::mutex> lock(task->mutex);
        if (task->state == PlaceholderTaskState::PENDING || task->state == PlaceholderTaskState::IN_PROGRESS) {
            CancelTaskRecordLocked(task, reason);
        }
    }
    ScheduleDispatch();
}

void PlaceholderTaskManager::CancelAllTasks(PlaceholderTaskCancelReason reason)
{
    std::vector<std::shared_ptr<PlaceholderTaskRecord>> tasks;
    {
        std::lock_guard<std::mutex> lock(mapMutex_);
        for (const auto &[reqKey, task] : taskMap_) {
            (void)reqKey;
            tasks.push_back(task);
        }
    }
    for (const auto &task : tasks) {
        std::lock_guard<std::mutex> lock(task->mutex);
        CancelTaskRecordLocked(task, reason);
    }
    ScheduleDispatch();
    deadlineCv_.notify_all();
}

void PlaceholderTaskManager::ClearCancellationRecords()
{
    std::lock_guard<std::mutex> lock(mapMutex_);
    cancellationRecordMap_.clear();
    cancellationOrder_.clear();
}

std::shared_ptr<PlaceholderTaskRecord> PlaceholderTaskManager::FindTask(const RequestKey &reqKey)
{
    std::lock_guard<std::mutex> lock(mapMutex_);
    auto task = taskMap_.find(reqKey);
    return task == taskMap_.end() ? nullptr : task->second;
}

bool PlaceholderTaskManager::GetTaskState(const RequestKey &reqKey, PlaceholderTaskState &state)
{
    std::lock_guard<std::mutex> lock(mapMutex_);
    auto task = taskMap_.find(reqKey);
    if (task == taskMap_.end()) {
        return false;
    }
    state = task->second->state.load();
    return true;
}

std::shared_ptr<PlaceholderTaskRecord> PlaceholderTaskManager::SelectNextTaskLocked()
{
    size_t globalActiveCount = 0;
    std::map<std::string, size_t> appActiveCounts;
    for (const auto &[reqKey, task] : taskMap_) {
        (void)reqKey;
        if (task->state == PlaceholderTaskState::IN_PROGRESS) {
            ++globalActiveCount;
            ++appActiveCounts[task->bundleName];
        }
    }
    if (globalActiveCount >= CLOUD_DISK_MAX_ACTIVE_TASKS_GLOBAL) {
        return nullptr;
    }

    std::shared_ptr<PlaceholderTaskRecord> selected;
    for (const auto &[reqKey, task] : taskMap_) {
        (void)reqKey;
        if (task->state != PlaceholderTaskState::PENDING ||
            appActiveCounts[task->bundleName] >= CLOUD_DISK_MAX_ACTIVE_TASKS_PER_APP) {
            continue;
        }
        if (selected == nullptr || task->priority > selected->priority ||
            (task->priority == selected->priority && task->createSeq < selected->createSeq)) {
            selected = task;
        }
    }
    return selected;
}

void PlaceholderTaskManager::ScheduleDispatch()
{
    std::lock_guard<std::mutex> lock(mapMutex_);
    if (!running_ || stopping_ || dispatchScheduled_) {
        return;
    }
    dispatchScheduled_ = true;
    dispatchQueue_.submit([this] { DispatchLoop(); });
}

void PlaceholderTaskManager::DispatchLoop()
{
    bool dispatching = true;
    while (dispatching) {
        std::shared_ptr<PlaceholderTaskRecord> task;
        {
            std::lock_guard<std::mutex> lock(mapMutex_);
            if (!running_ || stopping_) {
                dispatchScheduled_ = false;
                dispatching = false;
                continue;
            }
            task = SelectNextTaskLocked();
            if (task == nullptr) {
                dispatchScheduled_ = false;
                dispatching = false;
                continue;
            }
        }
        ActivateAndDispatch(task);
    }
}

void PlaceholderTaskManager::ActivateAndDispatch(const std::shared_ptr<PlaceholderTaskRecord> &task)
{
    std::lock_guard<std::mutex> fileLock(GetPlaceholderFileMutex(task->hmdfsPath));
    std::lock_guard<std::mutex> lock(task->mutex);
    if (task->state != PlaceholderTaskState::PENDING) {
        return;
    }

    UniqueFd outputFd;
    HydrationFileMetadata metadata;
    int32_t ret = OpenValidatedHydrationFile(task->mountSyncFolder, task->hmdfsPath, outputFd, metadata);
    if (ret != E_OK) {
        LOGW(
            "Cancel pending hydration because activation validation failed, "
            "ret:%{public}d",
            ret);
        CancelTaskRecordLocked(task, PlaceholderTaskCancelReason::INTERNAL_ERROR);
        return;
    }
    if (metadata.deviceId != task->deviceId || metadata.inodeId != task->inodeId) {
        LOGW("Cancel pending hydration because file identity changed");
        CancelTaskRecordLocked(task, PlaceholderTaskCancelReason::INTERNAL_ERROR);
        return;
    }

    CloudDiskCallbackReqHead reqHead{};
    reqHead.syncFolderPath = {task->syncFolder.data(), task->syncFolder.length()};
    reqHead.reqKey = {task->reqKey.data(), task->reqKey.size()};
    CloudDiskPathInfo pathInfo{task->filePath.data(), task->filePath.length()};
    ret = PlaceholderCallbackManager::GetInstance().DispatchFetchData(task->bundleName, task->syncFolderIndex, reqHead,
                                                                      pathInfo, task->priority);
    if (ret != E_OK) {
        LOGE("Dispatch fetch data failed, ret:%{public}d", ret);
        CancelTaskRecordLocked(task, PlaceholderTaskCancelReason::DISPATCH_FAILED);
        return;
    }
    task->outputFd = std::move(outputFd);
    task->totalSize = metadata.logicalSize;
    task->fetchDispatched = true;
    task->state = PlaceholderTaskState::IN_PROGRESS;
    RefreshDeadlineLocked(task);
    NotifyProgressLocked(task);
}

void PlaceholderTaskManager::DeadlineLoop()
{
    bool monitoring = true;
    while (monitoring) {
        std::vector<RequestKey> expiredKeys;
        {
            std::unique_lock<std::mutex> lock(mapMutex_);
            deadlineCv_.wait(lock, [this] { return !running_ || !deadlineMap_.empty(); });
            if (!running_) {
                monitoring = false;
                continue;
            }
            auto earliest =
                std::min_element(deadlineMap_.begin(), deadlineMap_.end(),
                                 [](const auto &left, const auto &right) { return left.second < right.second; });
            auto now = std::chrono::steady_clock::now();
            if (earliest != deadlineMap_.end() && earliest->second > now) {
                deadlineCv_.wait_until(lock, earliest->second);
                continue;
            }
            for (const auto &[reqKey, deadline] : deadlineMap_) {
                if (deadline <= now) {
                    expiredKeys.push_back(reqKey);
                }
            }
        }
        for (const auto &reqKey : expiredKeys) {
            auto task = FindTask(reqKey);
            if (task == nullptr) {
                continue;
            }
            std::lock_guard<std::mutex> taskLock(task->mutex);
            bool expired = false;
            {
                std::lock_guard<std::mutex> lock(mapMutex_);
                auto deadline = deadlineMap_.find(reqKey);
                expired = deadline != deadlineMap_.end() && deadline->second <= std::chrono::steady_clock::now();
            }
            if (expired) {
                LOGW("Hydration task idle timeout reached");
                CancelTaskRecordLocked(task, PlaceholderTaskCancelReason::TIMEOUT);
            }
        }
        ScheduleDispatch();
    }
}

void PlaceholderTaskManager::RefreshDeadlineLocked(const std::shared_ptr<PlaceholderTaskRecord> &task)
{
    std::lock_guard<std::mutex> lock(mapMutex_);
    auto item = taskMap_.find(task->reqKey);
    if (item != taskMap_.end() && item->second == task) {
        deadlineMap_[task->reqKey] = std::chrono::steady_clock::now() + HYDRATION_IDLE_TIMEOUT;
        deadlineCv_.notify_one();
    }
}

int32_t PlaceholderTaskManager::EnsurePartialStateLocked(const std::shared_ptr<PlaceholderTaskRecord> &task)
{
    if (task->hasPartialState) {
        return E_OK;
    }
    uint8_t placeholderState = PLACEHOLDER_STATE_NONE;
    int32_t ret = GetFilePlaceholderState(task->outputFd, placeholderState);
    if (ret != E_OK) {
        LOGE("Read hydration state failed, ret:%{public}d, state:%{public}u", ret, placeholderState);
        return ConvertHydrationStateError(ret);
    }
    if (!IsValidPlaceholderState(placeholderState)) {
        LOGE("Invalid hydration state, state:%{public}u", placeholderState);
        return E_INVALID_PLACEHOLDER_STATE;
    }
    if (placeholderState != PLACEHOLDER_STATE_UNHYDRATED && placeholderState != PLACEHOLDER_STATE_PARTIALLY_HYDRATED) {
        LOGE("Unexpected hydration state, state:%{public}u", placeholderState);
        return E_TRY_AGAIN;
    }
    if (placeholderState == PLACEHOLDER_STATE_UNHYDRATED) {
        uint8_t oldState = PLACEHOLDER_STATE_NONE;
        ret = SetFilePlaceholderState(task->outputFd, PLACEHOLDER_STATE_PARTIALLY_HYDRATED, oldState);
        if (ret != E_OK) {
            LOGE("Set partial hydration state failed, ret:%{public}d", ret);
            return ConvertHydrationIoError(ret);
        }
        if (!IsValidPlaceholderState(oldState)) {
            LOGE("Invalid old hydration state, state:%{public}u", oldState);
            return E_INVALID_PLACEHOLDER_STATE;
        }
        if (oldState != PLACEHOLDER_STATE_UNHYDRATED && oldState != PLACEHOLDER_STATE_PARTIALLY_HYDRATED) {
            LOGE("Unexpected old hydration state, state:%{public}u", oldState);
            return E_TRY_AGAIN;
        }
    }
    task->hasPartialState = true;
    return E_OK;
}

int32_t PlaceholderTaskManager::SetCompleteStateLocked(const std::shared_ptr<PlaceholderTaskRecord> &task)
{
    uint8_t placeholderState = PLACEHOLDER_STATE_NONE;
    int32_t ret = GetFilePlaceholderState(task->outputFd, placeholderState);
    if (ret != E_OK) {
        LOGE("Read hydration state before completion failed, ret:%{public}d", ret);
        return ConvertHydrationStateError(ret);
    }
    if (!IsValidPlaceholderState(placeholderState)) {
        LOGE("Invalid hydration state before completion, state:%{public}u", placeholderState);
        return E_INVALID_PLACEHOLDER_STATE;
    }
    if (placeholderState != PLACEHOLDER_STATE_UNHYDRATED && placeholderState != PLACEHOLDER_STATE_PARTIALLY_HYDRATED) {
        LOGE("Unexpected hydration state before completion, state:%{public}u", placeholderState);
        return E_TRY_AGAIN;
    }
    uint8_t oldState = PLACEHOLDER_STATE_NONE;
    ret = SetFilePlaceholderState(task->outputFd, PLACEHOLDER_STATE_FULLY_HYDRATED, oldState);
    if (ret != E_OK) {
        LOGE("Set complete hydration state failed, ret:%{public}d", ret);
        return ConvertHydrationIoError(ret);
    }
    if (!IsValidPlaceholderState(oldState)) {
        LOGE("Invalid old hydration state before completion, state:%{public}u", oldState);
        return E_INVALID_PLACEHOLDER_STATE;
    }
    if (oldState != PLACEHOLDER_STATE_UNHYDRATED && oldState != PLACEHOLDER_STATE_PARTIALLY_HYDRATED) {
        LOGE("Unexpected old hydration state before completion, state:%{public}u", oldState);
        return E_TRY_AGAIN;
    }
    return E_OK;
}

int32_t PlaceholderTaskManager::Execute(const std::string &callerBundleName,
                                        uint32_t syncFolderIndex,
                                        const CallbackExecuteRequest &request)
{
    if (request.reqKey.empty() || request.reqKey.size() > MAX_CALLBACK_REQUEST_KEY_SIZE ||
        (request.callbackType != static_cast<int32_t>(CloudDiskCallbackType::FETCH_DATA) &&
         request.callbackType != static_cast<int32_t>(CloudDiskCallbackType::CANCEL_FETCH_DATA))) {
        LOGE("Invalid Execute request");
        return E_INVALID_ARG;
    }
    auto task = FindTask(request.reqKey);
    if (task == nullptr) {
        std::lock_guard<std::mutex> lock(mapMutex_);
        return FindCancellationResultLocked(callerBundleName, syncFolderIndex, request);
    }
    if (task->bundleName != callerBundleName || task->syncFolderIndex != syncFolderIndex) {
        LOGE("Execute provider does not match task");
        return E_CALLBACK_NOT_REGISTERED;
    }
    if (task->syncFolder != request.syncFolder || task->filePath != request.filePath) {
        LOGE("Execute path does not match task");
        return E_INVALID_ARG;
    }
    std::lock_guard<std::mutex> lock(task->mutex);
    switch (task->state.load()) {
        case PlaceholderTaskState::CANCELLED:
            LOGW("Execute hydrate request rejected: task cancelled");
            return E_CANCELLED;
        case PlaceholderTaskState::COMPLETED:
            return E_NO_HYDRATION_IN_PROGRESS;
        case PlaceholderTaskState::PENDING:
            LOGW("Execute hydrate request rejected: task still pending");
            return E_TRY_AGAIN;
        default:
            break;
    }
    if (request.callbackType == static_cast<int32_t>(CloudDiskCallbackType::CANCEL_FETCH_DATA)) {
        CancelTaskRecordLocked(task, PlaceholderTaskCancelReason::USER_REQUEST);
        ScheduleDispatch();
        return E_OK;
    }
    return ExecuteFetchDataLocked(task, request);
}

int32_t PlaceholderTaskManager::ExecuteFetchDataLocked(const std::shared_ptr<PlaceholderTaskRecord> &task,
                                                       const CallbackExecuteRequest &request)
{
    if (IsInvalidFetchDataRequest(task, request)) {
        LOGE("Invalid Execute data bounds");
        return E_INVALID_ARG;
    }
    if (task->outputFd < 0) {
        LOGE("Invalid Execute task fd");
        CancelTaskRecordLocked(task, PlaceholderTaskCancelReason::INTERNAL_ERROR);
        ScheduleDispatch();
        return E_TRY_AGAIN;
    }
    task->totalSize = request.totalSize;
    task->totalSizeInitialized = true;
    int32_t ret = WriteHydrationData(task->outputFd, request);
    if (ret != E_OK) {
        return ret;
    }
    ret = CommitFetchDataLocked(task, request);
    if (ret != E_OK) {
        return ret;
    }

    uint64_t remainingSize = task->cachedSize < task->totalSize ? task->totalSize - task->cachedSize : 0;
    task->cachedSize += std::min(request.size, remainingSize);
    if (request.isComplete) {
        task->state = PlaceholderTaskState::COMPLETED;
        NotifyProgressLocked(task);
        EraseTaskLocked(task);
        ScheduleDispatch();
    } else {
        RefreshDeadlineLocked(task);
        NotifyProgressLocked(task);
    }
    return E_OK;
}

int32_t PlaceholderTaskManager::CommitFetchDataLocked(const std::shared_ptr<PlaceholderTaskRecord> &task,
                                                      const CallbackExecuteRequest &request)
{
    if (!request.isComplete) {
        return EnsurePartialStateLocked(task);
    }
    if (fsync(task->outputFd) < 0) {
        int32_t error = errno;
        LOGE("Flush completed hydration failed, errno:%{public}d", error);
        return ConvertHydrationIoError(error);
    }
    if (request.size != 0) {
        int32_t ret = EnsurePartialStateLocked(task);
        if (ret != E_OK) {
            return ret;
        }
    }
    return SetCompleteStateLocked(task);
}

void PlaceholderTaskManager::CancelTaskRecordLocked(const std::shared_ptr<PlaceholderTaskRecord> &task,
                                                    PlaceholderTaskCancelReason reason)
{
    if (task->state != PlaceholderTaskState::PENDING && task->state != PlaceholderTaskState::IN_PROGRESS) {
        return;
    }
    bool notifyCallback = !task->cancelCallbackAttempted && ShouldNotifyCancellation(reason);
    task->state = PlaceholderTaskState::CANCELLED;
    NotifyProgressLocked(task);
    if (notifyCallback) {
        task->cancelCallbackAttempted = true;
    }
    if (ShouldKeepCancellationRecord(reason) && (task->fetchDispatched || task->cancelCallbackAttempted)) {
        AddCancellationRecordLocked(task);
    }
    if (notifyCallback) {
        CloudDiskCallbackReqHead reqHead{};
        reqHead.syncFolderPath = {task->syncFolder.data(), task->syncFolder.length()};
        reqHead.reqKey = {task->reqKey.data(), task->reqKey.size()};
        CloudDiskPathInfo pathInfo{task->filePath.data(), task->filePath.length()};
        int32_t ret = PlaceholderCallbackManager::GetInstance().DispatchCancelFetchData(
            task->bundleName, task->syncFolderIndex, reqHead, pathInfo);
        if (ret != E_OK) {
            LOGW("Dispatch hydration cancellation failed, ret:%{public}d", ret);
        }
    }
    EraseTaskLocked(task);
}

void PlaceholderTaskManager::PurgeExpiredCancellationRecordsLocked()
{
    auto now = std::chrono::steady_clock::now();
    for (auto item = cancellationRecordMap_.begin(); item != cancellationRecordMap_.end();) {
        if (item->second.expiresAt <= now) {
            item = cancellationRecordMap_.erase(item);
        } else {
            ++item;
        }
    }
    cancellationOrder_.erase(std::remove_if(cancellationOrder_.begin(), cancellationOrder_.end(),
                                            [this](const auto &item) {
                                                auto cancellationRecord = cancellationRecordMap_.find(item.first);
                                                return cancellationRecord == cancellationRecordMap_.end() ||
                                                       cancellationRecord->second.createSeq != item.second;
                                            }),
                             cancellationOrder_.end());
}

void PlaceholderTaskManager::AddCancellationRecordLocked(const std::shared_ptr<PlaceholderTaskRecord> &task)
{
    std::lock_guard<std::mutex> lock(mapMutex_);
    PurgeExpiredCancellationRecordsLocked();
    if (nextCancellationSeq_ == std::numeric_limits<uint64_t>::max()) {
        nextCancellationSeq_ = INITIAL_REQUEST_VALUE;
    }
    CancelledTaskRecord cancellationRecord;
    cancellationRecord.reqKey = task->reqKey;
    cancellationRecord.syncFolder = task->syncFolder;
    cancellationRecord.filePath = task->filePath;
    cancellationRecord.bundleName = task->bundleName;
    cancellationRecord.syncFolderIndex = task->syncFolderIndex;
    cancellationRecord.expiresAt = std::chrono::steady_clock::now() + CANCELLATION_RECORD_TTL;
    cancellationRecord.createSeq = nextCancellationSeq_++;
    cancellationRecordMap_[task->reqKey] = cancellationRecord;
    cancellationOrder_.emplace_back(task->reqKey, cancellationRecord.createSeq);

    auto countForApp = [this, &task] {
        return static_cast<size_t>(
            std::count_if(cancellationRecordMap_.begin(), cancellationRecordMap_.end(),
                          [&task](const auto &item) { return item.second.bundleName == task->bundleName; }));
    };
    while (countForApp() > MAX_CANCELLATION_RECORDS_PER_APP) {
        auto oldest =
            std::find_if(cancellationOrder_.begin(), cancellationOrder_.end(), [this, &task](const auto &item) {
                auto cancellationRecord = cancellationRecordMap_.find(item.first);
                return cancellationRecord != cancellationRecordMap_.end() &&
                       cancellationRecord->second.createSeq == item.second &&
                       cancellationRecord->second.bundleName == task->bundleName;
            });
        if (oldest == cancellationOrder_.end()) {
            break;
        }
        cancellationRecordMap_.erase(oldest->first);
        cancellationOrder_.erase(oldest);
    }
    while (cancellationRecordMap_.size() > MAX_CANCELLATION_RECORDS_GLOBAL && !cancellationOrder_.empty()) {
        const auto oldest = cancellationOrder_.front();
        cancellationOrder_.pop_front();
        auto cancellationRecord = cancellationRecordMap_.find(oldest.first);
        if (cancellationRecord != cancellationRecordMap_.end() &&
            cancellationRecord->second.createSeq == oldest.second) {
            cancellationRecordMap_.erase(cancellationRecord);
        }
    }
}

int32_t PlaceholderTaskManager::FindCancellationResultLocked(const std::string &callerBundleName,
                                                             uint32_t syncFolderIndex,
                                                             const CallbackExecuteRequest &request)
{
    PurgeExpiredCancellationRecordsLocked();
    auto cancellationRecord = cancellationRecordMap_.find(request.reqKey);
    if (cancellationRecord == cancellationRecordMap_.end()) {
        LOGW("Execute hydrate request failed: task not found");
        return E_NO_HYDRATION_IN_PROGRESS;
    }
    if (cancellationRecord->second.bundleName != callerBundleName ||
        cancellationRecord->second.syncFolderIndex != syncFolderIndex) {
        return E_CALLBACK_NOT_REGISTERED;
    }
    if (cancellationRecord->second.syncFolder != request.syncFolder ||
        cancellationRecord->second.filePath != request.filePath) {
        return E_INVALID_ARG;
    }
    return E_CANCELLED;
}

void PlaceholderTaskManager::NotifyProgressLocked(const std::shared_ptr<PlaceholderTaskRecord> &task)
{
    if (task->terminalProgressSent) {
        return;
    }
    HydrateProgress progress;
    progress.filePath = task->absolutePath;
    progress.state = static_cast<int32_t>(task->state.load());
    progress.processedSize = task->cachedSize;
    progress.totalSize = task->totalSize;
    task->terminalProgressSent =
        task->state == PlaceholderTaskState::COMPLETED || task->state == PlaceholderTaskState::CANCELLED;
    PlaceholderProgressManager::GetInstance().OnTaskProgress(task->reqKey, task->userId, progress);
}

void PlaceholderTaskManager::EraseTaskLocked(const std::shared_ptr<PlaceholderTaskRecord> &task)
{
    int32_t fd = task->outputFd.Release();
    if (fd >= 0 && close(fd) < 0) {
        LOGE("Close hydration fd failed, errno:%{public}d", errno);
    }
    std::lock_guard<std::mutex> lock(mapMutex_);
    auto item = taskMap_.find(task->reqKey);
    if (item != taskMap_.end() && item->second == task) {
        taskMap_.erase(item);
    }
    deadlineMap_.erase(task->reqKey);
    deadlineCv_.notify_one();
}

} // namespace OHOS::FileManagement::CloudDiskService
