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
#include <limits>
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
constexpr size_t MAX_ACTIVE_TASKS_PER_APP = 5;
constexpr size_t MAX_ACTIVE_TASKS_GLOBAL = 10;
constexpr size_t MAX_TOMBSTONES_PER_APP = 16;
constexpr size_t MAX_TOMBSTONES_GLOBAL = 32;
constexpr auto HYDRATION_IDLE_TIMEOUT = std::chrono::minutes(5);
constexpr auto TOMBSTONE_TTL = std::chrono::seconds(60);

bool IsValidPriority(CloudDiskHydratePriority priority)
{
    return priority >= CLOUD_DISK_HYDRATE_PRIORITY_LOW && priority <= CLOUD_DISK_HYDRATE_PRIORITY_HIGH;
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
            return error == ENOSPC ? E_NO_SPACE_LEFT : E_TRY_AGAIN;
        }
        if (written == 0) {
            LOGE("Hydration write made no progress");
            return E_TRY_AGAIN;
        }
        writtenSize += static_cast<size_t>(written);
    }
    return E_OK;
}

bool ShouldKeepTombstone(PlaceholderTaskCancelReason reason)
{
    return reason != PlaceholderTaskCancelReason::DISPATCH_FAILED &&
           reason != PlaceholderTaskCancelReason::USER_SWITCH && reason != PlaceholderTaskCancelReason::SERVICE_STOP;
}

bool ShouldNotifyCancellation(PlaceholderTaskCancelReason reason)
{
    return reason == PlaceholderTaskCancelReason::USER_REQUEST || reason == PlaceholderTaskCancelReason::TIMEOUT ||
           reason == PlaceholderTaskCancelReason::INTERNAL_ERROR;
}
} // namespace

PlaceholderTaskManager &PlaceholderTaskManager::GetInstance()
{
    static PlaceholderTaskManager instance;
    return instance;
}

void PlaceholderTaskManager::StartWorkerPool()
{
    std::lock_guard<std::mutex> workerLock(workerMutex_);
    std::lock_guard<std::mutex> lock(mapMutex_);
    if (running_) {
        return;
    }
    running_ = true;
    workerHandles_.reserve(CLOUD_DISK_HYDRATE_WORKER_COUNT);
    for (uint32_t index = 0; index < CLOUD_DISK_HYDRATE_WORKER_COUNT; ++index) {
        workerHandles_.emplace_back(ffrt::submit_h([this] { WorkerLoop(); }));
    }
    workerHandles_.emplace_back(ffrt::submit_h([this] { DeadlineLoop(); }));
}

void PlaceholderTaskManager::StopWorkerPool()
{
    std::lock_guard<std::mutex> workerLock(workerMutex_);
    std::vector<ffrt::task_handle> workerHandles;
    std::vector<std::shared_ptr<PlaceholderTaskRecord>> tasks;
    {
        std::lock_guard<std::mutex> lock(mapMutex_);
        running_ = false;
        stopping_ = true;
        for (const auto &[reqKey, task] : taskMap_) {
            (void)reqKey;
            tasks.push_back(task);
        }
        workerHandles.swap(workerHandles_);
    }
    taskCv_.notify_all();
    deadlineCv_.notify_all();
    for (const auto &task : tasks) {
        std::lock_guard<std::mutex> lock(task->mutex);
        CancelTaskRecordLocked(task, PlaceholderTaskCancelReason::SERVICE_STOP);
    }
    for (const auto &handle : workerHandles) {
        ffrt::wait({handle});
    }
    std::lock_guard<std::mutex> lock(mapMutex_);
    pendingQueue_ = {};
    deadlineMap_.clear();
    tombstoneMap_.clear();
    tombstoneOrder_.clear();
    stopping_ = false;
}

PlaceholderTaskManager::RequestKey PlaceholderTaskManager::GenerateRequestKeyLocked()
{
    PurgeExpiredTombstonesLocked();
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
        if (taskMap_.find(reqKey) == taskMap_.end() && tombstoneMap_.find(reqKey) == tombstoneMap_.end()) {
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
        LOGW("Reject hydrate task while worker pool is stopping");
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

    size_t appTaskCount =
        static_cast<size_t>(std::count_if(taskMap_.begin(), taskMap_.end(), [&bundleName](const auto &item) {
            return item.second->bundleName == bundleName;
        }));
    if (appTaskCount >= MAX_ACTIVE_TASKS_PER_APP || taskMap_.size() >= MAX_ACTIVE_TASKS_GLOBAL) {
        LOGW("Reject hydrate task because active task limit was reached");
        return E_HYDRATION_TASK_LIMIT_REACHED;
    }

    if (nextCreateSeq_ == std::numeric_limits<uint64_t>::max()) {
        if (!pendingQueue_.empty()) {
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
                                                  UniqueFd outputFd,
                                                  RequestKey &reqKey,
                                                  const PlaceholderProgressContext &progressContext)
{
    reqKey.clear();
    if (syncFolder.empty() || filePath.empty() || bundleName.empty() || outputFd < 0 || !IsValidPriority(priority)) {
        LOGE("Invalid hydrate task context");
        return E_INVALID_ARG;
    }
    struct stat metadata {};
    if (fstat(outputFd, &metadata) != 0 || metadata.st_size < 0) {
        LOGE("Read hydration metadata failed, errno:%{public}d", errno);
        return E_TRY_AGAIN;
    }

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
    task->userId = progressContext.userId;
    task->totalSize = static_cast<uint64_t>(metadata.st_size);
    task->outputFd = std::move(outputFd);
    task->createSeq = nextCreateSeq_++;
    taskMap_[reqKey] = task;
    pendingQueue_.push(QueueEntry{reqKey, priority, task->createSeq});
    NotifyProgressLocked(task);
    taskCv_.notify_one();
    return E_OK;
}

bool PlaceholderTaskManager::HasActiveTask(const std::string &syncFolder,
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
    taskCv_.notify_all();
    if (!cancelled) {
        LOGW("Cancel hydrate task failed: no active task");
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
    taskCv_.notify_all();
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
    taskCv_.notify_all();
    deadlineCv_.notify_all();
}

void PlaceholderTaskManager::ClearTombstones()
{
    std::lock_guard<std::mutex> lock(mapMutex_);
    tombstoneMap_.clear();
    tombstoneOrder_.clear();
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

std::shared_ptr<PlaceholderTaskRecord> PlaceholderTaskManager::GetNextTask()
{
    std::unique_lock<std::mutex> lock(mapMutex_);
    while (true) {
        taskCv_.wait(lock, [this] { return !running_ || !pendingQueue_.empty(); });
        if (!running_) {
            return nullptr;
        }
        QueueEntry entry = pendingQueue_.top();
        pendingQueue_.pop();
        auto item = taskMap_.find(entry.reqKey);
        if (item == taskMap_.end()) {
            continue;
        }
        auto task = item->second;
        lock.unlock();
        {
            std::lock_guard<std::mutex> taskLock(task->mutex);
            if (task->state == PlaceholderTaskState::PENDING) {
                return task;
            }
        }
        lock.lock();
    }
}

void PlaceholderTaskManager::WorkerLoop()
{
    while (true) {
        auto task = GetNextTask();
        if (task == nullptr) {
            return;
        }
        std::lock_guard<std::mutex> lock(task->mutex);
        if (task->state != PlaceholderTaskState::PENDING) {
            continue;
        }
        CloudDiskCallbackReqHead reqHead{};
        reqHead.syncFolderPath = {task->syncFolder.data(), task->syncFolder.length()};
        reqHead.reqKey = {task->reqKey.data(), task->reqKey.size()};
        CloudDiskPathInfo pathInfo{task->filePath.data(), task->filePath.length()};
        int32_t ret = PlaceholderCallbackManager::GetInstance().DispatchFetchData(
            task->bundleName, task->syncFolderIndex, reqHead, pathInfo, task->priority);
        if (ret != E_OK) {
            LOGE("Dispatch fetch data failed, ret:%{public}d", ret);
            CancelTaskRecordLocked(task, PlaceholderTaskCancelReason::DISPATCH_FAILED);
            continue;
        }
        task->fetchDispatched = true;
        task->state = PlaceholderTaskState::IN_PROGRESS;
        RefreshDeadlineLocked(task);
        NotifyProgressLocked(task);
    }
}

void PlaceholderTaskManager::DeadlineLoop()
{
    while (true) {
        std::vector<RequestKey> expiredKeys;
        {
            std::unique_lock<std::mutex> lock(mapMutex_);
            deadlineCv_.wait(lock, [this] { return !running_ || !deadlineMap_.empty(); });
            if (!running_) {
                return;
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
    if (ret != E_OK || (placeholderState != PLACEHOLDER_STATE_UNHYDRATED &&
                        placeholderState != PLACEHOLDER_STATE_PARTIALLY_HYDRATED)) {
        LOGE("Read hydration state failed, ret:%{public}d, state:%{public}u", ret, placeholderState);
        return E_TRY_AGAIN;
    }
    if (placeholderState == PLACEHOLDER_STATE_UNHYDRATED) {
        uint8_t oldState = PLACEHOLDER_STATE_NONE;
        ret = SetFilePlaceholderState(task->outputFd, PLACEHOLDER_STATE_PARTIALLY_HYDRATED, oldState);
        if (ret != E_OK) {
            LOGE("Set partial hydration state failed, ret:%{public}d", ret);
            return E_TRY_AGAIN;
        }
    }
    task->hasPartialState = true;
    return E_OK;
}

int32_t PlaceholderTaskManager::SetCompleteStateLocked(const std::shared_ptr<PlaceholderTaskRecord> &task)
{
    uint8_t oldState = PLACEHOLDER_STATE_NONE;
    int32_t ret = SetFilePlaceholderState(task->outputFd, PLACEHOLDER_STATE_FULLY_HYDRATED, oldState);
    if (ret != E_OK) {
        LOGE("Set complete hydration state failed, ret:%{public}d", ret);
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
        return FindTombstoneResultLocked(callerBundleName, syncFolderIndex, request);
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
        return E_OK;
    }
    return ExecuteFetchDataLocked(task, request);
}

int32_t PlaceholderTaskManager::ExecuteFetchDataLocked(const std::shared_ptr<PlaceholderTaskRecord> &task,
                                                       const CallbackExecuteRequest &request)
{
    constexpr uint64_t MAX_OFFSET = static_cast<uint64_t>(std::numeric_limits<off_t>::max());
    if (request.size != request.data.size() || request.size > MAX_EXECUTE_DATA_SIZE ||
        request.offset > request.totalSize || request.size > request.totalSize - request.offset ||
        request.offset > MAX_OFFSET || request.size > MAX_OFFSET - request.offset ||
        (!request.isComplete && request.size == 0) ||
        (request.isComplete && request.totalSize != 0 && request.size == 0) ||
        (request.totalSize == 0 && (request.offset != 0 || request.size != 0 || !request.isComplete)) ||
        task->cachedSize > request.totalSize || (task->totalSizeInitialized && task->totalSize != request.totalSize)) {
        LOGE("Invalid Execute data bounds");
        return E_INVALID_ARG;
    }
    if (task->outputFd < 0) {
        LOGE("Invalid Execute task fd");
        CancelTaskRecordLocked(task, PlaceholderTaskCancelReason::INTERNAL_ERROR);
        return E_TRY_AGAIN;
    }
    task->totalSize = request.totalSize;
    task->totalSizeInitialized = true;
    int32_t ret = WriteHydrationData(task->outputFd, request);
    if (ret != E_OK) {
        return ret;
    }
    if (request.isComplete) {
        if (fsync(task->outputFd) < 0) {
            LOGE("Flush completed hydration failed, errno:%{public}d", errno);
            return errno == ENOSPC ? E_NO_SPACE_LEFT : E_TRY_AGAIN;
        }
        if (request.size != 0 && EnsurePartialStateLocked(task) != E_OK) {
            return E_TRY_AGAIN;
        }
        if (SetCompleteStateLocked(task) != E_OK) {
            return E_TRY_AGAIN;
        }
    } else if (EnsurePartialStateLocked(task) != E_OK) {
        return E_TRY_AGAIN;
    }

    uint64_t remainingSize = task->cachedSize < task->totalSize ? task->totalSize - task->cachedSize : 0;
    task->cachedSize += std::min(request.size, remainingSize);
    if (request.isComplete) {
        task->state = PlaceholderTaskState::COMPLETED;
        NotifyProgressLocked(task);
        EraseTaskLocked(task);
    } else {
        RefreshDeadlineLocked(task);
        NotifyProgressLocked(task);
    }
    return E_OK;
}

void PlaceholderTaskManager::CancelTaskRecordLocked(const std::shared_ptr<PlaceholderTaskRecord> &task,
                                                    PlaceholderTaskCancelReason reason)
{
    if (task->state != PlaceholderTaskState::PENDING && task->state != PlaceholderTaskState::IN_PROGRESS) {
        return;
    }
    bool notifyCallback = task->fetchDispatched && !task->cancelCallbackSent && ShouldNotifyCancellation(reason);
    task->cancelCallbackSent = notifyCallback;
    task->state = PlaceholderTaskState::CANCELLED;
    NotifyProgressLocked(task);
    if (ShouldKeepTombstone(reason)) {
        AddTombstoneLocked(task);
    }
    EraseTaskLocked(task);
    if (!notifyCallback) {
        return;
    }
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

void PlaceholderTaskManager::PurgeExpiredTombstonesLocked()
{
    auto now = std::chrono::steady_clock::now();
    for (auto item = tombstoneMap_.begin(); item != tombstoneMap_.end();) {
        if (item->second.expiresAt <= now) {
            item = tombstoneMap_.erase(item);
        } else {
            ++item;
        }
    }
    tombstoneOrder_.erase(std::remove_if(tombstoneOrder_.begin(), tombstoneOrder_.end(),
                                         [this](const auto &item) {
                                             auto tombstone = tombstoneMap_.find(item.first);
                                             return tombstone == tombstoneMap_.end() ||
                                                    tombstone->second.createSeq != item.second;
                                         }),
                          tombstoneOrder_.end());
}

void PlaceholderTaskManager::AddTombstoneLocked(const std::shared_ptr<PlaceholderTaskRecord> &task)
{
    std::lock_guard<std::mutex> lock(mapMutex_);
    PurgeExpiredTombstonesLocked();
    if (nextTombstoneSeq_ == std::numeric_limits<uint64_t>::max()) {
        nextTombstoneSeq_ = INITIAL_REQUEST_VALUE;
    }
    CancelledTaskTombstone tombstone;
    tombstone.reqKey = task->reqKey;
    tombstone.syncFolder = task->syncFolder;
    tombstone.filePath = task->filePath;
    tombstone.bundleName = task->bundleName;
    tombstone.syncFolderIndex = task->syncFolderIndex;
    tombstone.expiresAt = std::chrono::steady_clock::now() + TOMBSTONE_TTL;
    tombstone.createSeq = nextTombstoneSeq_++;
    tombstoneMap_[task->reqKey] = tombstone;
    tombstoneOrder_.emplace_back(task->reqKey, tombstone.createSeq);

    auto countForApp = [this, &task] {
        return static_cast<size_t>(std::count_if(tombstoneMap_.begin(), tombstoneMap_.end(), [&task](const auto &item) {
            return item.second.bundleName == task->bundleName;
        }));
    };
    while (countForApp() > MAX_TOMBSTONES_PER_APP) {
        auto oldest = std::find_if(tombstoneOrder_.begin(), tombstoneOrder_.end(), [this, &task](const auto &item) {
            auto tombstone = tombstoneMap_.find(item.first);
            return tombstone != tombstoneMap_.end() && tombstone->second.createSeq == item.second &&
                   tombstone->second.bundleName == task->bundleName;
        });
        if (oldest == tombstoneOrder_.end()) {
            break;
        }
        tombstoneMap_.erase(oldest->first);
        tombstoneOrder_.erase(oldest);
    }
    while (tombstoneMap_.size() > MAX_TOMBSTONES_GLOBAL && !tombstoneOrder_.empty()) {
        const auto oldest = tombstoneOrder_.front();
        tombstoneOrder_.pop_front();
        auto tombstone = tombstoneMap_.find(oldest.first);
        if (tombstone != tombstoneMap_.end() && tombstone->second.createSeq == oldest.second) {
            tombstoneMap_.erase(tombstone);
        }
    }
}

int32_t PlaceholderTaskManager::FindTombstoneResultLocked(const std::string &callerBundleName,
                                                          uint32_t syncFolderIndex,
                                                          const CallbackExecuteRequest &request)
{
    PurgeExpiredTombstonesLocked();
    auto tombstone = tombstoneMap_.find(request.reqKey);
    if (tombstone == tombstoneMap_.end()) {
        LOGW("Execute hydrate request failed: task not found");
        return E_NO_HYDRATION_IN_PROGRESS;
    }
    if (tombstone->second.bundleName != callerBundleName || tombstone->second.syncFolderIndex != syncFolderIndex) {
        return E_CALLBACK_NOT_REGISTERED;
    }
    if (tombstone->second.syncFolder != request.syncFolder || tombstone->second.filePath != request.filePath) {
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
