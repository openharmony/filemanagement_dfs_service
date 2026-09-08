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

#ifndef OHOS_FILEMGMT_PLACEHOLDER_TASK_MANAGER_H
#define OHOS_FILEMGMT_PLACEHOLDER_TASK_MANAGER_H

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <vector>

#include "cloud_disk_common.h"
#include "ffrt.h"
#include "nocopyable.h"
#include "unique_fd.h"

#ifndef CLOUD_DISK_HYDRATE_WORKER_COUNT
#define CLOUD_DISK_HYDRATE_WORKER_COUNT 4
#endif

namespace OHOS::FileManagement::CloudDiskService {
enum class PlaceholderTaskState {
    PENDING = 0,
    IN_PROGRESS,
    COMPLETED,
    CANCELLED,
};

enum class PlaceholderTaskCancelReason {
    USER_REQUEST = 0,
    TIMEOUT,
    INTERNAL_ERROR,
    UNREGISTER,
    CALLBACK_DIED,
    USER_SWITCH,
    SERVICE_STOP,
    DISPATCH_FAILED,
};

struct PlaceholderProgressContext {
    int32_t userId = -1;
    std::string absolutePath;
};

struct PlaceholderTaskRecord {
    std::vector<uint8_t> reqKey;
    std::string syncFolder;
    std::string filePath;
    std::string bundleName;
    uint32_t syncFolderIndex = 0;
    CloudDiskCallbackType callbackType = CloudDiskCallbackType::FETCH_DATA;
    CloudDiskHydratePriority priority = CLOUD_DISK_HYDRATE_PRIORITY_NORMAL;
    UniqueFd outputFd;
    std::atomic<PlaceholderTaskState> state{PlaceholderTaskState::PENDING};
    std::string absolutePath;
    int32_t userId = -1;
    uint64_t cachedSize = 0;
    bool terminalProgressSent = false;
    uint64_t totalSize = 0;
    bool totalSizeInitialized = false;
    bool hasPartialState = false;
    bool fetchDispatched = false;
    bool cancelCallbackSent = false;
    uint64_t createSeq = 0;
    std::mutex mutex;
};

class PlaceholderTaskManager final : public NoCopyable {
public:
    using RequestKey = std::vector<uint8_t>;

    static PlaceholderTaskManager &GetInstance();

    void StartWorkerPool();
    void StopWorkerPool();
    int32_t CreateHydrateTask(const std::string &syncFolder,
                              const std::string &filePath,
                              const std::string &bundleName,
                              uint32_t syncFolderIndex,
                              CloudDiskHydratePriority priority,
                              UniqueFd outputFd,
                              RequestKey &reqKey,
                              const PlaceholderProgressContext &progressContext = {});
    bool HasActiveTask(const std::string &syncFolder, const std::string &filePath, uint32_t syncFolderIndex);
    int32_t CancelTask(const std::string &syncFolder, const std::string &filePath, uint32_t syncFolderIndex);
    void CancelTasksBySyncFolder(const std::string &bundleName,
                                 uint32_t syncFolderIndex,
                                 PlaceholderTaskCancelReason reason = PlaceholderTaskCancelReason::UNREGISTER);
    void CancelAllTasks(PlaceholderTaskCancelReason reason);
    void ClearTombstones();
    int32_t
        Execute(const std::string &callerBundleName, uint32_t syncFolderIndex, const CallbackExecuteRequest &request);
    bool GetTaskState(const RequestKey &reqKey, PlaceholderTaskState &state);

private:
    struct QueueEntry {
        RequestKey reqKey;
        CloudDiskHydratePriority priority;
        uint64_t createSeq;
    };

    struct PriorityComparator {
        bool operator()(const QueueEntry &left, const QueueEntry &right) const
        {
            if (left.priority != right.priority) {
                return left.priority < right.priority;
            }
            return left.createSeq > right.createSeq;
        }
    };

    struct CancelledTaskTombstone {
        RequestKey reqKey;
        std::string syncFolder;
        std::string filePath;
        std::string bundleName;
        uint32_t syncFolderIndex = 0;
        std::chrono::steady_clock::time_point expiresAt;
        uint64_t createSeq = 0;
    };

    void NotifyProgressLocked(const std::shared_ptr<PlaceholderTaskRecord> &task);
    RequestKey GenerateRequestKeyLocked();
    int32_t PrepareHydrateTaskLocked(const std::string &syncFolder,
                                     const std::string &filePath,
                                     const std::string &bundleName,
                                     uint32_t syncFolderIndex,
                                     RequestKey &reqKey);
    std::shared_ptr<PlaceholderTaskRecord> FindTask(const RequestKey &reqKey);
    std::shared_ptr<PlaceholderTaskRecord> GetNextTask();
    void WorkerLoop();
    void DeadlineLoop();
    void RefreshDeadlineLocked(const std::shared_ptr<PlaceholderTaskRecord> &task);
    int32_t EnsurePartialStateLocked(const std::shared_ptr<PlaceholderTaskRecord> &task);
    int32_t SetCompleteStateLocked(const std::shared_ptr<PlaceholderTaskRecord> &task);
    int32_t ExecuteFetchDataLocked(const std::shared_ptr<PlaceholderTaskRecord> &task,
                                   const CallbackExecuteRequest &request);
    void CancelTaskRecordLocked(const std::shared_ptr<PlaceholderTaskRecord> &task, PlaceholderTaskCancelReason reason);
    void AddTombstoneLocked(const std::shared_ptr<PlaceholderTaskRecord> &task);
    void PurgeExpiredTombstonesLocked();
    int32_t FindTombstoneResultLocked(const std::string &callerBundleName,
                                      uint32_t syncFolderIndex,
                                      const CallbackExecuteRequest &request);
    void EraseTaskLocked(const std::shared_ptr<PlaceholderTaskRecord> &task);

    std::map<RequestKey, std::shared_ptr<PlaceholderTaskRecord>> taskMap_;
    std::priority_queue<QueueEntry, std::vector<QueueEntry>, PriorityComparator> pendingQueue_;
    std::map<RequestKey, CancelledTaskTombstone> tombstoneMap_;
    std::deque<std::pair<RequestKey, uint64_t>> tombstoneOrder_;
    std::map<RequestKey, std::chrono::steady_clock::time_point> deadlineMap_;
    std::vector<ffrt::task_handle> workerHandles_;
    uint64_t nextReqKeyValue_ = 1;
    uint64_t nextCreateSeq_ = 1;
    uint64_t nextTombstoneSeq_ = 1;
    bool running_ = false;
    bool stopping_ = false;
    // Never wait on a record mutex while holding mapMutex_. Record ownership keeps its mutex alive after erase.
    std::mutex mapMutex_;
    std::mutex workerMutex_;
    std::condition_variable taskCv_;
    std::condition_variable deadlineCv_;
};
} // namespace OHOS::FileManagement::CloudDiskService

#endif // OHOS_FILEMGMT_PLACEHOLDER_TASK_MANAGER_H
