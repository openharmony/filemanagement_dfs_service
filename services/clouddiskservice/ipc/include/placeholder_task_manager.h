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
#include <string>
#include <vector>

#include "cloud_disk_common.h"
#include "ffrt.h"
#include "nocopyable.h"
#include "unique_fd.h"

#ifndef CLOUD_DISK_MAX_ACTIVE_TASKS_PER_APP
#define CLOUD_DISK_MAX_ACTIVE_TASKS_PER_APP 5U
#endif

#ifndef CLOUD_DISK_MAX_ACTIVE_TASKS_GLOBAL
#define CLOUD_DISK_MAX_ACTIVE_TASKS_GLOBAL 10U
#endif

#ifndef CLOUD_DISK_MAX_PENDING_TASKS_PER_APP
#define CLOUD_DISK_MAX_PENDING_TASKS_PER_APP 20U
#endif

#ifndef CLOUD_DISK_MAX_PENDING_TASKS_GLOBAL
#define CLOUD_DISK_MAX_PENDING_TASKS_GLOBAL 40U
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
    std::string hmdfsPath;
    std::string mountSyncFolder;
};

struct HydrationFileMetadata {
    uint64_t deviceId = 0;
    uint64_t inodeId = 0;
    uint64_t logicalSize = 0;
};

int32_t OpenValidatedHydrationFile(const std::string &syncRoot,
                                   const std::string &path,
                                   UniqueFd &fd,
                                   HydrationFileMetadata &metadata);

struct PlaceholderTaskRecord {
    std::vector<uint8_t> reqKey;
    std::string syncFolder;
    std::string filePath;
    std::string bundleName;
    uint32_t syncFolderIndex = 0;
    CloudDiskCallbackType callbackType = CloudDiskCallbackType::FETCH_DATA;
    CloudDiskHydratePriority priority = CLOUD_DISK_HYDRATE_PRIORITY_NORMAL;
    std::string hmdfsPath;
    std::string mountSyncFolder;
    uint64_t deviceId = 0;
    uint64_t inodeId = 0;
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
    bool cancelCallbackAttempted = false;
    uint64_t createSeq = 0;
    std::mutex mutex;
};

class PlaceholderTaskManager final : public NoCopyable {
public:
    using RequestKey = std::vector<uint8_t>;

    static PlaceholderTaskManager &GetInstance();

    void StartScheduler();
    void StopScheduler();
    int32_t CreateHydrateTask(const std::string &syncFolder,
                              const std::string &filePath,
                              const std::string &bundleName,
                              uint32_t syncFolderIndex,
                              CloudDiskHydratePriority priority,
                              UniqueFd validationFd,
                              RequestKey &reqKey,
                              const PlaceholderProgressContext &progressContext = {});
    bool HasOutstandingTask(const std::string &syncFolder, const std::string &filePath, uint32_t syncFolderIndex);
    int32_t CancelTask(const std::string &syncFolder, const std::string &filePath, uint32_t syncFolderIndex);
    void CancelTasksBySyncFolder(const std::string &bundleName,
                                 uint32_t syncFolderIndex,
                                 PlaceholderTaskCancelReason reason = PlaceholderTaskCancelReason::UNREGISTER);
    void CancelAllTasks(PlaceholderTaskCancelReason reason);
    void ClearCancellationRecords();
    int32_t
        Execute(const std::string &callerBundleName, uint32_t syncFolderIndex, const CallbackExecuteRequest &request);
    bool GetTaskState(const RequestKey &reqKey, PlaceholderTaskState &state);

private:
    struct CancelledTaskRecord {
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
    std::shared_ptr<PlaceholderTaskRecord> SelectNextTaskLocked();
    void ScheduleDispatch();
    void DispatchLoop();
    void ActivateAndDispatch(const std::shared_ptr<PlaceholderTaskRecord> &task);
    void DeadlineLoop();
    void RefreshDeadlineLocked(const std::shared_ptr<PlaceholderTaskRecord> &task);
    int32_t EnsurePartialStateLocked(const std::shared_ptr<PlaceholderTaskRecord> &task);
    int32_t SetCompleteStateLocked(const std::shared_ptr<PlaceholderTaskRecord> &task);
    int32_t CommitFetchDataLocked(const std::shared_ptr<PlaceholderTaskRecord> &task,
                                  const CallbackExecuteRequest &request);
    int32_t ExecuteFetchDataLocked(const std::shared_ptr<PlaceholderTaskRecord> &task,
                                   const CallbackExecuteRequest &request);
    void CancelTaskRecordLocked(const std::shared_ptr<PlaceholderTaskRecord> &task, PlaceholderTaskCancelReason reason);
    void AddCancellationRecordLocked(const std::shared_ptr<PlaceholderTaskRecord> &task);
    void PurgeExpiredCancellationRecordsLocked();
    int32_t FindCancellationResultLocked(const std::string &callerBundleName,
                                         uint32_t syncFolderIndex,
                                         const CallbackExecuteRequest &request);
    void EraseTaskLocked(const std::shared_ptr<PlaceholderTaskRecord> &task);

    std::map<RequestKey, std::shared_ptr<PlaceholderTaskRecord>> taskMap_;
    std::map<RequestKey, CancelledTaskRecord> cancellationRecordMap_;
    std::deque<std::pair<RequestKey, uint64_t>> cancellationOrder_;
    std::map<RequestKey, std::chrono::steady_clock::time_point> deadlineMap_;
    std::vector<ffrt::task_handle> monitorHandles_;
    uint64_t nextReqKeyValue_ = 1;
    uint64_t nextCreateSeq_ = 1;
    uint64_t nextCancellationSeq_ = 1;
    bool running_ = false;
    bool stopping_ = false;
    bool dispatchScheduled_ = false;
    // Never wait on a record mutex while holding mapMutex_. Record ownership keeps its mutex alive after erase.
    std::mutex mapMutex_;
    std::mutex lifecycleMutex_;
    std::condition_variable deadlineCv_;
    ffrt::queue dispatchQueue_{"clouddisk_hydration_dispatch"};
};
} // namespace OHOS::FileManagement::CloudDiskService

#endif // OHOS_FILEMGMT_PLACEHOLDER_TASK_MANAGER_H
