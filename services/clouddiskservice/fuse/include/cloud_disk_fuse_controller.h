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

#ifndef CLOUD_DISK_FUSE_CONTROLLER_H
#define CLOUD_DISK_FUSE_CONTROLLER_H

#ifndef FUSE_USE_VERSION
#define FUSE_USE_VERSION 317
#endif

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include <fuse_lowlevel.h>
#include <sys/stat.h>

#include "nocopyable.h"
#include "unique_fd.h"

#define CLOUD_DISK_FUSE_LOOKUP_WARMUP_WAIT_MS 1000

#ifndef CLOUD_DISK_FUSE_MOUNT_RETRY_INTERVAL_SECONDS
#define CLOUD_DISK_FUSE_MOUNT_RETRY_INTERVAL_SECONDS 1U
#endif

#ifndef CLOUD_DISK_FUSE_MOUNT_RETRY_COUNT
#define CLOUD_DISK_FUSE_MOUNT_RETRY_COUNT 3U
#endif

#ifndef CLOUD_DISK_FUSE_UNMOUNT_RETRY_INTERVAL_SECONDS
#define CLOUD_DISK_FUSE_UNMOUNT_RETRY_INTERVAL_SECONDS 1U
#endif

#ifndef CLOUD_DISK_FUSE_UNMOUNT_RETRY_COUNT
#define CLOUD_DISK_FUSE_UNMOUNT_RETRY_COUNT 3U
#endif

namespace OHOS::FileManagement::CloudDiskService {

class CloudDiskFuseOperations;

class CloudDiskFuseController final : protected NoCopyable {
public:
    static CloudDiskFuseController &GetInstance();

    void StartUser(int32_t userId, const std::unordered_map<uint32_t, std::string> &roots);
    void AddRoot(int32_t userId, uint32_t syncFolderIndex, const std::string &physicalPath);
    void RemoveRoot(int32_t userId, uint32_t syncFolderIndex);
    void SetIdleCallback(std::function<void()> callback);
    void Stop();
    void StopForServiceExit();
    bool CanUnload() const;

private:
    friend class CloudDiskFuseOperations;

    enum class State {
        IDLE,
        MOUNTING,
        READY,
        DRAINING,
        CLEANUP_RETRY,
        STOPPING,
    };

    enum class RootState {
        WARMING,
        READY,
        RETIRING,
    };

    enum class StopMode {
        PRESERVE_PENDING_UNMOUNTS,
        DISCARD_PENDING_UNMOUNTS,
    };

    enum class RootLookupState {
        NO_PARENT,
        MATCH,
        CONTINUE,
        TERMINAL_ERROR,
    };

    struct RootContext {
        uint32_t syncFolderIndex{0};
        std::string physicalPath;
        std::string mountPath;
        uint64_t warmupEpoch{0};
        RootState state{RootState::WARMING};
        int32_t lastWarmupError{0};
        bool warmupInFlight{false};
        uint32_t warmupTaskCount{0};
        std::chrono::steady_clock::time_point retireDeadline{};
    };

    struct NodeContext {
        fuse_ino_t nodeId{0};
        uint32_t syncFolderIndex{0};
        std::string path;
        dev_t device{0};
        ino_t inode{0};
        uint64_t rootEpoch{0};
        uint64_t nlookup{0};
        uint32_t openCount{0};
    };

    struct OpenContext {
        uint64_t handle{0};
        fuse_ino_t nodeId{0};
        uint32_t syncFolderIndex{0};
        uint64_t generation{0};
        dev_t device{0};
        ino_t inode{0};
        UniqueFd fd;
    };

    struct LookupResult {
        uint32_t syncFolderIndex{0};
        uint64_t rootEpoch{0};
        std::string path;
        struct stat attr {};
    };

    struct UnmountTarget {
        int32_t userId{-1};
        std::string mountPoint;
    };

    struct MountAttempt {
        int32_t userId{-1};
        std::string mountPoint;
        uint64_t generation{0};
    };

    struct LookupSnapshot {
        int32_t userId{-1};
        uint64_t generation{0};
        std::vector<RootContext> roots;
        bool rootWarming{false};
        int32_t warmupError{0};
    };

    struct RootLookupOutcome {
        RootLookupState state{RootLookupState::NO_PARENT};
        int32_t error{0};
    };

    struct OperationIdentity {
        int32_t userId{-1};
        uint64_t generation{0};
        uint32_t rootIndex{0};
    };

    struct OpenOperation {
        NodeContext node;
        uint64_t generation{0};
        uint64_t handle{0};
        UniqueFd fd;
        struct stat attr {};
    };

    struct ReleaseOperation {
        OperationIdentity identity;
        uint64_t handle{0};
        int fd{-1};
    };

    CloudDiskFuseController() = default;
    ~CloudDiskFuseController();

    bool BuildRootContext(int32_t userId,
                          uint32_t syncFolderIndex,
                          const std::string &physicalPath,
                          RootContext &root) const;
    void StartUserLocked(int32_t userId, const std::unordered_map<uint32_t, std::string> &roots);
    void StopLocked(StopMode mode);
    void ScheduleRootWarmup(int32_t userId, const RootContext &root);
    bool BeginRootWarmup(int32_t userId, const RootContext &root);
    void RunRootWarmup(int32_t userId, const RootContext &root);
    void FinishRootWarmup(int32_t userId, const RootContext &root, int32_t ret);
    void WorkerMain();
    bool ShouldStopWorker();
    bool HasActiveRootLocked() const;
    bool IsWarmupCurrentLocked(int32_t userId, uint32_t syncFolderIndex, uint64_t warmupEpoch) const;
    bool IsIdleLocked() const;
    std::chrono::steady_clock::time_point GetRetireDeadlineLocked() const;
    void FinalizeRetiredRoots();
    bool SelectPendingUnmount(UnmountTarget &pendingUnmount);
    bool HandlePendingUnmount(const UnmountTarget &pendingUnmount);
    bool HasActiveSession();
    void WaitForSessionStop(std::unique_lock<std::mutex> &lock);
    bool HandleActiveSession();
    bool PrepareMountAttempt(MountAttempt &mountAttempt);
    bool MountSession(const MountAttempt &mountAttempt, int32_t &ret);
    bool WaitForMountRetry(int32_t ret, uint32_t &retryCount);
    bool WaitForRetryReset(uint64_t failedRootEpoch, bool resetWhenNoActiveRoot);
    int32_t CreateSession(int fuseFd);
    int32_t StopSession(int32_t userId, const std::string &mountPoint);
    int32_t
        RetryUnmountWithLimit(int32_t userId, const std::string &mountPoint, int32_t unmountRet, uint32_t &retryCount);
    bool RetryUnmount(int32_t userId, const std::string &mountPoint, int32_t unmountRet);
    void LoopMain(struct fuse_session *session);
    void WakeSession();
    std::string GetMountPoint(int32_t userId) const;

    int32_t ResolveLookup(fuse_ino_t parent, const char *name, LookupResult &result);
    int32_t CaptureLookupSnapshot(LookupSnapshot &snapshot);
    int32_t BuildLookupCandidate(const RootContext &root,
                                 const std::string &relativePath,
                                 const char *name,
                                 std::string &candidate) const;
    int32_t ValidateLookupCandidate(const RootContext &root,
                                    const std::string &candidate,
                                    std::string &canonicalPath,
                                    struct stat &candidateAttr) const;
    RootLookupOutcome ResolveLookupInRoot(int32_t userId,
                                          fuse_ino_t parent,
                                          const char *name,
                                          const RootContext &root,
                                          LookupResult &result);
    RootLookupOutcome ScanLookupRoots(const LookupSnapshot &snapshot,
                                      fuse_ino_t parent,
                                      const char *name,
                                      LookupResult &result);
    int32_t WaitForLookupWarmup(const LookupSnapshot &snapshot);
    int32_t RegisterLookupNode(const LookupResult &result, fuse_ino_t &nodeId);
    int32_t GetAttrNode(fuse_ino_t ino, NodeContext &node);
    OperationIdentity GetOperationIdentity();
    int32_t ReserveOpen(fuse_ino_t ino, OpenOperation &operation);
    void ReleaseOpenReservation();
    int32_t OpenBackingFile(OpenOperation &operation) const;
    int32_t CommitOpen(fuse_ino_t ino, OpenOperation &operation);
    int32_t TakeOpenContext(fuse_ino_t ino, uint64_t handle, ReleaseOperation &operation);
    void ForgetNode(fuse_ino_t ino, uint64_t nlookup);
    void PurgeRootLocked(uint32_t syncFolderIndex, std::vector<int> &fds);
    void PurgeAllLocked(std::vector<int> &fds);
    void RollbackLookup(fuse_ino_t nodeId);
    void RollbackOpen(uint64_t handle);

    static void Init(void *userdata, struct fuse_conn_info *conn);
    static void Destroy(void *userdata);
    static ssize_t ReadFuseFd(int fd, void *buf, size_t len, void *userdata);
    static ssize_t WriteFuseFd(int fd, struct iovec *iov, int count, void *userdata);

    std::mutex lifecycleMutex_;
    mutable std::mutex mutex_;
    std::condition_variable condition_;
    State state_{State::IDLE};
    int32_t activeUserId_{-1};
    uint64_t generation_{0};
    uint64_t rootWarmupEpoch_{0};
    bool started_{false};
    bool shutdown_{false};
    bool loopExited_{false};
    uint32_t pendingOpenCount_{0};
    uint32_t pendingWarmupCount_{0};
    std::unordered_map<uint32_t, RootContext> roots_;
    std::vector<UnmountTarget> pendingUnmounts_;

    struct fuse_session *session_{nullptr};
    int wakeFd_{-1};
    std::thread workerThread_;
    std::thread loopThread_;

    fuse_ino_t nextNodeId_{FUSE_ROOT_ID + 1};
    uint64_t nextHandle_{1};
    std::unordered_map<fuse_ino_t, NodeContext> nodes_;
    std::map<std::pair<uint64_t, uint64_t>, fuse_ino_t> identityToNode_;
    std::unordered_map<uint64_t, std::unique_ptr<OpenContext>> openContexts_;
    std::function<void()> idleCallback_;

    std::atomic<uint32_t> lookupLogCount_{0};
    std::atomic<uint32_t> forgetLogCount_{0};
    std::atomic<uint32_t> getattrLogCount_{0};
};

} // namespace OHOS::FileManagement::CloudDiskService

#endif // CLOUD_DISK_FUSE_CONTROLLER_H
