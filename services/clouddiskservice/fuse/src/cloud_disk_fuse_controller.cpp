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

#include "cloud_disk_fuse_controller.h"

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <climits>
#include <cstring>
#include <fcntl.h>
#include <limits>
#include <poll.h>
#include <sys/eventfd.h>
#include <sys/uio.h>
#include <unistd.h>

#include "cloud_disk_fuse_mount_adapter.h"
#include "cloud_disk_fuse_operations.h"
#include "cloud_disk_service_logfile.h"
#include "cloud_disk_service_metafile.h"
#include "cloud_disk_sync_folder.h"
#include "ffrt_inner.h"
#include "placeholder_helper.h"
#include "utils_directory.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudDiskService {
namespace {
constexpr int32_t E_OK = 0;
constexpr size_t MAX_NODE_CONTEXTS = 8192;
constexpr uint32_t MAX_OPEN_CONTEXTS = 1024;
constexpr uint32_t MAX_LOOKUP_ATTEMPTS = 2;
constexpr uint32_t ROOT_RETIRE_GRACE_SECONDS = 1;
constexpr mode_t MOUNT_POINT_MODE = 0770;
constexpr nfds_t FUSE_POLL_FD_COUNT = 2;
constexpr nfds_t FUSE_ONLY_POLL_FD_COUNT = 1;

void DispatchIdleCallback(const std::function<void()> &callback)
{
    if (callback) {
        ffrt::submit([callback] { callback(); });
    }
}

void CloseFds(const std::vector<int> &fds)
{
    for (int fd : fds) {
        if (fd >= 0 && close(fd) != 0) {
            LOGW("Close backing fd failed, errno: %{public}d", errno);
        }
    }
}

bool IsPathInRoot(const std::string &path, const std::string &root)
{
    if (path.size() <= root.size() || path.compare(0, root.size(), root) != 0) {
        return false;
    }
    return root.back() == '/' || path[root.size()] == '/';
}
} // namespace

CloudDiskFuseController &CloudDiskFuseController::GetInstance()
{
    static CloudDiskFuseController instance;
    return instance;
}

CloudDiskFuseController::~CloudDiskFuseController()
{
    StopForServiceExit();
}

bool CloudDiskFuseController::BuildRootContext(int32_t userId,
                                               uint32_t syncFolderIndex,
                                               const std::string &physicalPath,
                                               RootContext &root) const
{
    std::string mountPath;
    if (!CloudDiskSyncFolder::GetInstance().PathToMntPathByPhysicalPath(physicalPath, std::to_string(userId),
                                                                        mountPath)) {
        LOGE("Convert FUSE root to HMDFS mount path failed, index: %{public}u", syncFolderIndex);
        return false;
    }
    root.syncFolderIndex = syncFolderIndex;
    root.physicalPath = physicalPath;
    root.mountPath = mountPath;
    return true;
}

void CloudDiskFuseController::StartUser(int32_t userId, const std::unordered_map<uint32_t, std::string> &roots)
{
    std::lock_guard<std::mutex> lifecycleLock(lifecycleMutex_);
    StartUserLocked(userId, roots);
}

void CloudDiskFuseController::StartUserLocked(int32_t userId, const std::unordered_map<uint32_t, std::string> &roots)
{
    StopLocked(StopMode::PRESERVE_PENDING_UNMOUNTS);

    std::unordered_map<uint32_t, RootContext> rootContexts;
    for (const auto &[syncFolderIndex, physicalPath] : roots) {
        RootContext root;
        if (BuildRootContext(userId, syncFolderIndex, physicalPath, root)) {
            rootContexts.emplace(syncFolderIndex, std::move(root));
        }
    }

    std::vector<RootContext> warmupRoots;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        activeUserId_ = userId;
        roots_ = std::move(rootContexts);
        warmupRoots.reserve(roots_.size());
        for (auto &[syncFolderIndex, root] : roots_) {
            (void)syncFolderIndex;
            root.warmupEpoch = ++rootWarmupEpoch_;
            root.state = RootState::WARMING;
            root.lastWarmupError = 0;
            root.warmupInFlight = true;
            warmupRoots.push_back(root);
        }
        shutdown_ = false;
        started_ = true;
        state_ = pendingUnmounts_.empty() ? State::IDLE : State::CLEANUP_RETRY;
    }
    workerThread_ = std::thread(&CloudDiskFuseController::WorkerMain, this);
    for (const auto &root : warmupRoots) {
        ScheduleRootWarmup(userId, root);
    }
    condition_.notify_all();
}

void CloudDiskFuseController::ScheduleRootWarmup(int32_t userId, const RootContext &root)
{
    if (!BeginRootWarmup(userId, root)) {
        return;
    }
    LOGI("CloudDiskService FUSE root warmup started, userId: %{private}d, root: %{private}u, epoch: %{public}llu",
         userId, root.syncFolderIndex, static_cast<unsigned long long>(root.warmupEpoch));
    ffrt::submit([this, userId, root] { RunRootWarmup(userId, root); });
}

bool CloudDiskFuseController::BeginRootWarmup(int32_t userId, const RootContext &root)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto current = roots_.find(root.syncFolderIndex);
    if (!IsWarmupCurrentLocked(userId, root.syncFolderIndex, root.warmupEpoch)) {
        return false;
    }
    ++current->second.warmupTaskCount;
    current->second.warmupInFlight = true;
    ++pendingWarmupCount_;
    return true;
}

void CloudDiskFuseController::RunRootWarmup(int32_t userId, const RootContext &root)
{
    auto shouldCancel = [this, userId, root] {
        std::lock_guard<std::mutex> lock(mutex_);
        return !IsWarmupCurrentLocked(userId, root.syncFolderIndex, root.warmupEpoch);
    };
    int32_t ret =
        LogFileMgr::GetInstance().WarmupSyncFolder(userId, root.syncFolderIndex, root.physicalPath, shouldCancel);
    FinishRootWarmup(userId, root, ret);
}

void CloudDiskFuseController::FinishRootWarmup(int32_t userId, const RootContext &root, int32_t ret)
{
    std::function<void()> idleCallback;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto current = roots_.find(root.syncFolderIndex);
        if (current != roots_.end() && current->second.warmupTaskCount > 0) {
            --current->second.warmupTaskCount;
            current->second.warmupInFlight = current->second.warmupTaskCount != 0;
            if (activeUserId_ == userId && current->second.warmupEpoch == root.warmupEpoch &&
                current->second.state == RootState::WARMING) {
                current->second.lastWarmupError = ret;
                if (ret == E_OK) {
                    current->second.state = RootState::READY;
                }
            }
        }
        if (pendingWarmupCount_ > 0) {
            --pendingWarmupCount_;
        }
        if (!shutdown_ && IsIdleLocked()) {
            idleCallback = idleCallback_;
        }
    }
    condition_.notify_all();
    DispatchIdleCallback(idleCallback);
    LOGI(
        "CloudDiskService FUSE root warmup finished, userId: %{private}d, root: %{private}u, "
        "epoch: %{public}llu, ret: %{public}d",
        userId, root.syncFolderIndex, static_cast<unsigned long long>(root.warmupEpoch), ret);
}

void CloudDiskFuseController::AddRoot(int32_t userId, uint32_t syncFolderIndex, const std::string &physicalPath)
{
    RootContext root;
    if (!BuildRootContext(userId, syncFolderIndex, physicalPath, root)) {
        LogFileMgr::GetInstance().ScheduleFillChildForDir(userId, syncFolderIndex, physicalPath);
        return;
    }

    std::unique_lock<std::mutex> lifecycleLock(lifecycleMutex_);
    std::unique_lock<std::mutex> lock(mutex_);
    if (!started_ || activeUserId_ != userId || shutdown_) {
        LOGW("Ignore root for inactive FUSE user, userId: %{public}d", userId);
        lock.unlock();
        lifecycleLock.unlock();
        LogFileMgr::GetInstance().ScheduleFillChildForDir(userId, syncFolderIndex, physicalPath);
        return;
    }
    auto existingRoot = roots_.find(syncFolderIndex);
    if (existingRoot != roots_.end()) {
        root.warmupTaskCount = existingRoot->second.warmupTaskCount;
    }
    root.warmupEpoch = ++rootWarmupEpoch_;
    root.state = RootState::WARMING;
    root.lastWarmupError = 0;
    root.warmupInFlight = true;
    roots_[syncFolderIndex] = root;
    condition_.notify_all();
    lock.unlock();
    ScheduleRootWarmup(userId, root);
}

void CloudDiskFuseController::RemoveRoot(int32_t userId, uint32_t syncFolderIndex)
{
    std::lock_guard<std::mutex> lifecycleLock(lifecycleMutex_);
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (userId != activeUserId_) {
            return;
        }
        auto rootIt = roots_.find(syncFolderIndex);
        if (rootIt == roots_.end() || rootIt->second.state == RootState::RETIRING) {
            return;
        }
        rootIt->second.state = RootState::RETIRING;
        rootIt->second.warmupEpoch = ++rootWarmupEpoch_;
        rootIt->second.retireDeadline =
            std::chrono::steady_clock::now() + std::chrono::seconds(ROOT_RETIRE_GRACE_SECONDS);
        LOGI("CloudDiskService FUSE root enters retiring, userId: %{private}d, root: %{private}u", userId,
             syncFolderIndex);
        condition_.notify_all();
        condition_.wait(lock, [this, syncFolderIndex] {
            auto current = roots_.find(syncFolderIndex);
            return current == roots_.end() || current->second.warmupTaskCount == 0;
        });
    }
    condition_.notify_all();
}

void CloudDiskFuseController::SetIdleCallback(std::function<void()> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    idleCallback_ = std::move(callback);
}

void CloudDiskFuseController::Stop()
{
    std::lock_guard<std::mutex> lifecycleLock(lifecycleMutex_);
    StopLocked(StopMode::PRESERVE_PENDING_UNMOUNTS);
}

void CloudDiskFuseController::StopForServiceExit()
{
    std::lock_guard<std::mutex> lifecycleLock(lifecycleMutex_);
    StopLocked(StopMode::DISCARD_PENDING_UNMOUNTS);
}

void CloudDiskFuseController::StopLocked(StopMode mode)
{
    bool discardPendingUnmounts = mode == StopMode::DISCARD_PENDING_UNMOUNTS;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!started_ && !workerThread_.joinable()) {
            if (discardPendingUnmounts && !pendingUnmounts_.empty()) {
                LOGW("Discard %{public}zu pending FUSE unmounts while service exits", pendingUnmounts_.size());
                pendingUnmounts_.clear();
                state_ = State::IDLE;
            }
            return;
        }
        shutdown_ = true;
        roots_.clear();
        state_ = State::STOPPING;
    }
    condition_.notify_all();
    if (workerThread_.joinable()) {
        workerThread_.join();
    }

    std::vector<int> fds;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        condition_.wait(lock, [this] { return pendingWarmupCount_ == 0; });
        PurgeAllLocked(fds);
        if (discardPendingUnmounts && !pendingUnmounts_.empty()) {
            LOGW("Discard %{public}zu pending FUSE unmounts while service exits", pendingUnmounts_.size());
            pendingUnmounts_.clear();
        }
        state_ = pendingUnmounts_.empty() ? State::IDLE : State::CLEANUP_RETRY;
        activeUserId_ = -1;
        started_ = false;
        shutdown_ = false;
        session_ = nullptr;
        wakeFd_ = -1;
    }
    CloseFds(fds);
    condition_.notify_all();
}

bool CloudDiskFuseController::CanUnload() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return IsIdleLocked();
}

std::string CloudDiskFuseController::GetMountPoint(int32_t userId) const
{
    return "/mnt/data/" + std::to_string(userId) + "/cloud_disk_fuse/clouddiskservice";
}

void CloudDiskFuseController::WorkerMain()
{
    uint32_t mountRetryCount = 0;
    while (!ShouldStopWorker()) {
        FinalizeRetiredRoots();
        UnmountTarget pendingUnmount;
        if (SelectPendingUnmount(pendingUnmount)) {
            if (!HandlePendingUnmount(pendingUnmount)) {
                break;
            }
            continue;
        }

        if (HasActiveSession()) {
            if (!HandleActiveSession()) {
                break;
            }
            mountRetryCount = 0;
            continue;
        }

        MountAttempt mountAttempt;
        if (!PrepareMountAttempt(mountAttempt)) {
            break;
        }
        int32_t ret = 0;
        if (!MountSession(mountAttempt, ret)) {
            break;
        }
        if (ret == E_OK) {
            mountRetryCount = 0;
            continue;
        }
        if (!WaitForMountRetry(ret, mountRetryCount)) {
            break;
        }
    }

    std::lock_guard<std::mutex> lock(mutex_);
    state_ = pendingUnmounts_.empty() ? State::IDLE : State::CLEANUP_RETRY;
    condition_.notify_all();
}

bool CloudDiskFuseController::ShouldStopWorker()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return shutdown_ && session_ == nullptr;
}

bool CloudDiskFuseController::HasActiveRootLocked() const
{
    return std::any_of(roots_.begin(), roots_.end(), [](const auto &entry) {
        return entry.second.state != RootState::RETIRING;
    });
}

bool CloudDiskFuseController::IsWarmupCurrentLocked(int32_t userId,
                                                    uint32_t syncFolderIndex,
                                                    uint64_t warmupEpoch) const
{
    auto current = roots_.find(syncFolderIndex);
    return !shutdown_ && activeUserId_ == userId && current != roots_.end() &&
           current->second.warmupEpoch == warmupEpoch && current->second.state == RootState::WARMING;
}

bool CloudDiskFuseController::IsIdleLocked() const
{
    return roots_.empty() && pendingUnmounts_.empty() && session_ == nullptr && pendingWarmupCount_ == 0 &&
           state_ == State::IDLE;
}

std::chrono::steady_clock::time_point CloudDiskFuseController::GetRetireDeadlineLocked() const
{
    auto deadline = std::chrono::steady_clock::time_point::max();
    for (const auto &[syncFolderIndex, root] : roots_) {
        (void)syncFolderIndex;
        if (root.state == RootState::RETIRING && root.warmupTaskCount == 0) {
            deadline = std::min(deadline, root.retireDeadline);
        }
    }
    return deadline;
}

void CloudDiskFuseController::FinalizeRetiredRoots()
{
    std::vector<int> fds;
    std::function<void()> idleCallback;
    bool rootsChanged = false;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto now = std::chrono::steady_clock::now();
        for (auto rootIt = roots_.begin(); rootIt != roots_.end();) {
            if (rootIt->second.state != RootState::RETIRING || rootIt->second.warmupTaskCount != 0 ||
                rootIt->second.retireDeadline > now) {
                ++rootIt;
                continue;
            }
            uint32_t syncFolderIndex = rootIt->first;
            PurgeRootLocked(syncFolderIndex, fds);
            rootIt = roots_.erase(rootIt);
            rootsChanged = true;
            LOGI("CloudDiskService FUSE root retired, root: %{private}u", syncFolderIndex);
        }
        if (rootsChanged && roots_.empty()) {
            if (session_ != nullptr) {
                state_ = State::DRAINING;
                LOGI("CloudDiskService FUSE enters draining, userId: %{private}d", activeUserId_);
            } else if (!shutdown_) {
                state_ = pendingUnmounts_.empty() ? State::IDLE : State::CLEANUP_RETRY;
                if (IsIdleLocked()) {
                    idleCallback = idleCallback_;
                }
            }
        }
    }
    CloseFds(fds);
    condition_.notify_all();
    DispatchIdleCallback(idleCallback);
}

bool CloudDiskFuseController::SelectPendingUnmount(UnmountTarget &pendingUnmount)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (shutdown_ || pendingUnmounts_.empty()) {
        return false;
    }
    auto pendingIt = pendingUnmounts_.begin();
    bool hasCurrentEndpointOrphan = false;
    if (session_ == nullptr && !roots_.empty()) {
        std::string currentMountPoint = GetMountPoint(activeUserId_);
        auto currentEndpoint = std::find_if(pendingUnmounts_.begin(), pendingUnmounts_.end(),
            [&currentMountPoint](auto &it) { return it.mountPoint == currentMountPoint; });
        if (currentEndpoint != pendingUnmounts_.end()) {
            pendingIt = currentEndpoint;
            hasCurrentEndpointOrphan = true;
        }
    }
    bool currentSessionNeedsStop =
        session_ != nullptr && (roots_.empty() || loopExited_ || state_ == State::DRAINING);
    bool newEndpointNeedsMount = session_ == nullptr && !roots_.empty() && !hasCurrentEndpointOrphan;
    bool canCleanAlongsideSession = session_ == nullptr || state_ == State::READY;
    if (currentSessionNeedsStop || newEndpointNeedsMount || !canCleanAlongsideSession) {
        return false;
    }
    pendingUnmount = *pendingIt;
    if (session_ == nullptr) {
        state_ = State::CLEANUP_RETRY;
    }
    return true;
}

bool CloudDiskFuseController::HandlePendingUnmount(const UnmountTarget &pendingUnmount)
{
    int32_t ret = CloudDiskFuseMountAdapter::Unmount(pendingUnmount.userId, pendingUnmount.mountPoint);
    return RetryUnmount(pendingUnmount.userId, pendingUnmount.mountPoint, ret);
}

bool CloudDiskFuseController::HasActiveSession()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return session_ != nullptr;
}

void CloudDiskFuseController::WaitForSessionStop(std::unique_lock<std::mutex> &lock)
{
    auto shouldStopSession = [this] {
        return shutdown_ || roots_.empty() || loopExited_ || state_ == State::DRAINING;
    };
    while (!shouldStopSession()) {
        auto retireDeadline = GetRetireDeadlineLocked();
        if (retireDeadline == std::chrono::steady_clock::time_point::max()) {
            condition_.wait(lock);
            continue;
        }
        (void)condition_.wait_until(lock, retireDeadline);
        if (std::chrono::steady_clock::now() >= GetRetireDeadlineLocked()) {
            return;
        }
    }
}

bool CloudDiskFuseController::HandleActiveSession()
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (session_ == nullptr) {
        return true;
    }
    WaitForSessionStop(lock);
    lock.unlock();
    FinalizeRetiredRoots();
    lock.lock();
    if (session_ == nullptr) {
        return true;
    }
    if (!shutdown_ && !roots_.empty() && !loopExited_ && state_ != State::DRAINING) {
        return true;
    }
    int32_t userId = activeUserId_;
    std::string mountPoint = GetMountPoint(userId);
    state_ = shutdown_ ? State::STOPPING : State::DRAINING;
    lock.unlock();
    int32_t unmountRet = StopSession(userId, mountPoint);
    if (!RetryUnmount(userId, mountPoint, unmountRet)) {
        return false;
    }
    lock.lock();
    bool shouldStop = shutdown_;
    lock.unlock();
    return !shouldStop;
}

bool CloudDiskFuseController::PrepareMountAttempt(MountAttempt &mountAttempt)
{
    std::unique_lock<std::mutex> lock(mutex_);
    while (!shutdown_ && !HasActiveRootLocked()) {
        auto retireDeadline = GetRetireDeadlineLocked();
        if (retireDeadline == std::chrono::steady_clock::time_point::max()) {
            condition_.wait(lock, [this] { return shutdown_ || HasActiveRootLocked(); });
            continue;
        }
        if (!condition_.wait_until(lock, retireDeadline, [this] { return shutdown_ || HasActiveRootLocked(); })) {
            lock.unlock();
            FinalizeRetiredRoots();
            lock.lock();
        }
    }
    if (shutdown_) {
        return false;
    }
    mountAttempt.userId = activeUserId_;
    mountAttempt.mountPoint = GetMountPoint(mountAttempt.userId);
    state_ = State::MOUNTING;
    loopExited_ = false;
    mountAttempt.generation = ++generation_;
    return true;
}

bool CloudDiskFuseController::MountSession(const MountAttempt &mountAttempt, int32_t &ret)
{
    LOGI("CloudDiskService FUSE mount attempt, userId: %{private}d, generation: %{public}llu", mountAttempt.userId,
         static_cast<unsigned long long>(mountAttempt.generation));
    Storage::DistributedFile::Utils::ForceCreateDirectory(mountAttempt.mountPoint, MOUNT_POINT_MODE);
    int fuseFd = -1;
    ret = CloudDiskFuseMountAdapter::Mount(mountAttempt.userId, mountAttempt.mountPoint, fuseFd);
    if (ret == E_OK && fuseFd >= 0) {
        ret = CreateSession(fuseFd);
        if (ret != E_OK) {
            int32_t unmountRet = CloudDiskFuseMountAdapter::Unmount(mountAttempt.userId, mountAttempt.mountPoint);
            return RetryUnmount(mountAttempt.userId, mountAttempt.mountPoint, unmountRet);
        }
        return true;
    }
    if (ret == E_OK) {
        ret = -EBADF;
    }
    if (ret == EBUSY || ret == -EBUSY || ret == -EBADF) {
        int32_t unmountRet = CloudDiskFuseMountAdapter::Unmount(mountAttempt.userId, mountAttempt.mountPoint);
        return RetryUnmount(mountAttempt.userId, mountAttempt.mountPoint, unmountRet);
    }
    return true;
}

bool CloudDiskFuseController::WaitForMountRetry(int32_t ret, uint32_t &retryCount)
{
    std::function<void()> idleCallback;
    std::unique_lock<std::mutex> lock(mutex_);
    if (!shutdown_) {
        state_ = State::IDLE;
        if (IsIdleLocked()) {
            idleCallback = idleCallback_;
        }
    }
    condition_.notify_all();
    if (shutdown_) {
        return false;
    }
    if (!HasActiveRootLocked()) {
        lock.unlock();
        DispatchIdleCallback(idleCallback);
        retryCount = 0;
        return true;
    }

    if (retryCount >= CLOUD_DISK_FUSE_MOUNT_RETRY_COUNT) {
        uint64_t failedRootEpoch = rootWarmupEpoch_;
        LOGE(
            "Create CloudDiskService FUSE session failed after %{public}u retries, ret: %{public}d; "
            "wait for sync root change",
            CLOUD_DISK_FUSE_MOUNT_RETRY_COUNT, ret);
        lock.unlock();
        DispatchIdleCallback(idleCallback);
        bool shouldContinue = WaitForRetryReset(failedRootEpoch, true);
        retryCount = 0;
        return shouldContinue;
    }

    ++retryCount;
    LOGW(
        "Create CloudDiskService FUSE session failed, ret: %{public}d; retry %{public}u/%{public}u in "
        "%{public}u seconds",
        ret, retryCount, CLOUD_DISK_FUSE_MOUNT_RETRY_COUNT, CLOUD_DISK_FUSE_MOUNT_RETRY_INTERVAL_SECONDS);
    (void)condition_.wait_for(lock, std::chrono::seconds(CLOUD_DISK_FUSE_MOUNT_RETRY_INTERVAL_SECONDS),
                              [this] { return shutdown_ || !HasActiveRootLocked(); });
    if (!HasActiveRootLocked()) {
        retryCount = 0;
    }
    return true;
}

bool CloudDiskFuseController::WaitForRetryReset(uint64_t failedRootEpoch, bool resetWhenNoActiveRoot)
{
    auto shouldReset = [this, failedRootEpoch, resetWhenNoActiveRoot] {
        return shutdown_ || rootWarmupEpoch_ != failedRootEpoch || (resetWhenNoActiveRoot && !HasActiveRootLocked());
    };
    while (true) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (shouldReset()) {
            return !shutdown_;
        }

        auto retireDeadline = GetRetireDeadlineLocked();
        if (retireDeadline == std::chrono::steady_clock::time_point::max()) {
            condition_.wait(lock, shouldReset);
            continue;
        }
        if (!condition_.wait_until(lock, retireDeadline, shouldReset)) {
            lock.unlock();
            FinalizeRetiredRoots();
        }
    }
}

bool CloudDiskFuseController::RetryUnmount(int32_t userId, const std::string &mountPoint, int32_t unmountRet)
{
    uint32_t retryCount = 0;
    while (unmountRet != E_OK && retryCount < CLOUD_DISK_FUSE_UNMOUNT_RETRY_COUNT) {
        std::unique_lock<std::mutex> lock(mutex_);
        state_ = shutdown_ ? State::STOPPING : State::CLEANUP_RETRY;
        condition_.notify_all();
        if (shutdown_) {
            break;
        }
        ++retryCount;
        LOGW(
            "Unmount CloudDiskService FUSE failed, ret: %{public}d; retry %{public}u/%{public}u in "
            "%{public}u seconds",
            unmountRet, retryCount, CLOUD_DISK_FUSE_UNMOUNT_RETRY_COUNT,
            CLOUD_DISK_FUSE_UNMOUNT_RETRY_INTERVAL_SECONDS);
        (void)condition_.wait_for(lock, std::chrono::seconds(CLOUD_DISK_FUSE_UNMOUNT_RETRY_INTERVAL_SECONDS),
                                  [this] { return shutdown_; });
        if (shutdown_) {
            break;
        }
        lock.unlock();

        unmountRet = CloudDiskFuseMountAdapter::Unmount(userId, mountPoint);
    }

    std::function<void()> idleCallback;
    uint64_t failedRootEpoch = 0;
    bool shouldStop = false;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto target = std::find_if(pendingUnmounts_.begin(), pendingUnmounts_.end(), [userId, &mountPoint](auto &it) {
            return it.userId == userId && it.mountPoint == mountPoint;
        });
        if (unmountRet == E_OK) {
            if (target != pendingUnmounts_.end()) {
                pendingUnmounts_.erase(target);
            }
            if (!shutdown_) {
                state_ = pendingUnmounts_.empty() ? State::IDLE : State::CLEANUP_RETRY;
            }
            if (IsIdleLocked()) {
                idleCallback = idleCallback_;
            }
        } else {
            if (target == pendingUnmounts_.end()) {
                pendingUnmounts_.push_back({userId, mountPoint});
            }
            state_ = shutdown_ ? State::STOPPING : State::CLEANUP_RETRY;
            failedRootEpoch = rootWarmupEpoch_;
        }
        shouldStop = shutdown_;
    }
    condition_.notify_all();
    DispatchIdleCallback(idleCallback);
    if (unmountRet == E_OK) {
        return true;
    }
    if (shouldStop) {
        return false;
    }
    LOGE(
        "Unmount CloudDiskService FUSE failed after %{public}u retries, ret: %{public}d; "
        "wait for sync root change",
        retryCount, unmountRet);
    return WaitForRetryReset(failedRootEpoch, false);
}

int32_t CloudDiskFuseController::CreateSession(int fuseFd)
{
    UniqueFd fuseFdGuard(fuseFd);
    UniqueFd wakeFdGuard(eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK));
    if (wakeFdGuard < 0) {
        return -errno;
    }

    struct fuse_args args = FUSE_ARGS_INIT(0, nullptr);
    if (fuse_opt_add_arg(&args, "clouddiskservice_fuse") != 0) {
        fuse_opt_free_args(&args);
        return -ENOMEM;
    }
    struct fuse_lowlevel_ops operations {};
    operations.init = Init;
    operations.destroy = Destroy;
    operations.lookup = CloudDiskFuseOperations::Lookup;
    operations.forget = CloudDiskFuseOperations::Forget;
    operations.getattr = CloudDiskFuseOperations::GetAttr;
    operations.open = CloudDiskFuseOperations::Open;
    operations.release = CloudDiskFuseOperations::Release;

    struct fuse_session *session = fuse_session_new(&args, &operations, sizeof(operations), this);
    fuse_opt_free_args(&args);
    if (session == nullptr) {
        return -ENOMEM;
    }

    struct fuse_custom_io customIo {};
    customIo.read = ReadFuseFd;
    customIo.writev = WriteFuseFd;
    int32_t ret = fuse_session_custom_io(session, &customIo, sizeof(customIo), fuseFdGuard.Get());
    if (ret != 0) {
        fuse_session_destroy(session);
        return ret;
    }
    (void)fuseFdGuard.Release();

    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (shutdown_ || !HasActiveRootLocked()) {
            fuse_session_destroy(session);
            return -ECANCELED;
        }
        session_ = session;
        wakeFd_ = wakeFdGuard.Release();
        loopExited_ = false;
        lookupLogCount_.store(0, std::memory_order_relaxed);
        forgetLogCount_.store(0, std::memory_order_relaxed);
        getattrLogCount_.store(0, std::memory_order_relaxed);
    }
    loopThread_ = std::thread(&CloudDiskFuseController::LoopMain, this, session);
    return 0;
}

void CloudDiskFuseController::LoopMain(struct fuse_session *session)
{
    struct fuse_loop_config *config = fuse_loop_cfg_create();
    int32_t ret = -ENOMEM;
    if (config != nullptr) {
        fuse_loop_cfg_set_idle_threads(config, 1);
        fuse_loop_cfg_set_clone_fd(config, 0);
        ret = fuse_session_loop_mt(session, config);
        fuse_loop_cfg_destroy(config);
    }
    LOGI("CloudDiskService FUSE loop exited, ret: %{public}d", ret);
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (session_ == session) {
            loopExited_ = true;
        }
    }
    condition_.notify_all();
}

void CloudDiskFuseController::WakeSession()
{
    int wakeFd = -1;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        wakeFd = wakeFd_;
    }
    if (wakeFd >= 0) {
        uint64_t value = 1;
        (void)write(wakeFd, &value, sizeof(value));
    }
}

int32_t CloudDiskFuseController::StopSession(int32_t userId, const std::string &mountPoint)
{
    struct fuse_session *session = nullptr;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        session = session_;
    }
    if (session == nullptr) {
        return 0;
    }

    fuse_session_exit(session);
    WakeSession();
    int32_t unmountRet = CloudDiskFuseMountAdapter::Unmount(userId, mountPoint);
    if (unmountRet != 0) {
        LOGW("Unmount CloudDiskService FUSE failed, ret: %{public}d", unmountRet);
    } else {
        LOGI("Unmount CloudDiskService FUSE succeeded, userId: %{private}d", userId);
    }
    if (loopThread_.joinable()) {
        loopThread_.join();
    }
    fuse_session_destroy(session);

    std::vector<int> fds;
    std::function<void()> idleCallback;
    int wakeFd = -1;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (session_ == session) {
            session_ = nullptr;
        }
        wakeFd = wakeFd_;
        wakeFd_ = -1;
        loopExited_ = false;
        PurgeAllLocked(fds);
        state_ = unmountRet == 0 ? State::IDLE : State::CLEANUP_RETRY;
        if (!shutdown_ && IsIdleLocked()) {
            idleCallback = idleCallback_;
        }
    }
    if (wakeFd >= 0) {
        (void)close(wakeFd);
    }
    CloseFds(fds);
    condition_.notify_all();
    DispatchIdleCallback(idleCallback);
    return unmountRet;
}

int32_t CloudDiskFuseController::ResolveLookup(fuse_ino_t parent, const char *name, LookupResult &result)
{
    if (name == nullptr || name[0] == '\0' || strcmp(name, ".") == 0 || strcmp(name, "..") == 0 ||
        strchr(name, '/') != nullptr || strlen(name) > NAME_MAX) {
        return EINVAL;
    }

    for (uint32_t lookupAttempt = 0; lookupAttempt < MAX_LOOKUP_ATTEMPTS; ++lookupAttempt) {
        LookupSnapshot snapshot;
        int32_t ret = CaptureLookupSnapshot(snapshot);
        if (ret != E_OK) {
            return ret;
        }
        RootLookupOutcome outcome = ScanLookupRoots(snapshot, parent, name, result);
        if (outcome.state == RootLookupState::MATCH) {
            return 0;
        }
        if (outcome.state == RootLookupState::TERMINAL_ERROR || outcome.state == RootLookupState::CONTINUE) {
            return outcome.error;
        }
        if (snapshot.rootWarming && lookupAttempt == 0) {
            ret = WaitForLookupWarmup(snapshot);
            if (ret != E_OK) {
                return ret;
            }
            continue;
        }
        if (snapshot.warmupError != E_OK) {
            return EIO;
        }
        return EAGAIN;
    }
    return EAGAIN;
}

int32_t CloudDiskFuseController::CaptureLookupSnapshot(LookupSnapshot &snapshot)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (state_ != State::READY) {
        return ENOTCONN;
    }
    snapshot.userId = activeUserId_;
    snapshot.generation = generation_;
    snapshot.roots.reserve(roots_.size());
    for (const auto &[index, root] : roots_) {
        (void)index;
        if (root.state == RootState::RETIRING) {
            continue;
        }
        if (root.state == RootState::READY) {
            snapshot.roots.push_back(root);
        } else if (root.state == RootState::WARMING && root.warmupInFlight) {
            snapshot.rootWarming = true;
        } else if (root.lastWarmupError != E_OK && snapshot.warmupError == E_OK) {
            snapshot.warmupError = root.lastWarmupError;
        }
    }
    return E_OK;
}

int32_t CloudDiskFuseController::BuildLookupCandidate(const RootContext &root,
                                                      const std::string &relativePath,
                                                      const char *name,
                                                      std::string &candidate) const
{
    bool needsSeparator = relativePath.empty() || relativePath.back() != '/';
    size_t separatorLength = needsSeparator ? 1 : 0;
    size_t nameLength = strlen(name);
    if (root.mountPath.size() > PATH_MAX || relativePath.size() > PATH_MAX ||
        root.mountPath.size() + relativePath.size() > PATH_MAX - separatorLength ||
        root.mountPath.size() + relativePath.size() + separatorLength > PATH_MAX - nameLength) {
        return ENAMETOOLONG;
    }
    candidate = root.mountPath + relativePath;
    if (needsSeparator) {
        candidate.push_back('/');
    }
    candidate += name;
    return E_OK;
}

int32_t CloudDiskFuseController::ValidateLookupCandidate(const RootContext &root,
                                                         const std::string &candidate,
                                                         std::string &canonicalPath,
                                                         struct stat &candidateAttr) const
{
    if (lstat(candidate.c_str(), &candidateAttr) != 0) {
        return errno;
    }
    if (!S_ISREG(candidateAttr.st_mode)) {
        return ESTALE;
    }
    char canonicalRootBuffer[PATH_MAX] = {'\0'};
    char canonicalPathBuffer[PATH_MAX] = {'\0'};
    if (realpath(root.mountPath.c_str(), canonicalRootBuffer) == nullptr) {
        return errno;
    }
    if (realpath(candidate.c_str(), canonicalPathBuffer) == nullptr) {
        return errno;
    }
    if (!IsPathInRoot(canonicalPathBuffer, canonicalRootBuffer)) {
        return ESTALE;
    }
    if (lstat(canonicalPathBuffer, &candidateAttr) != 0) {
        return errno;
    }
    if (!S_ISREG(candidateAttr.st_mode)) {
        return ESTALE;
    }
    canonicalPath = canonicalPathBuffer;
    return E_OK;
}

CloudDiskFuseController::RootLookupOutcome CloudDiskFuseController::ResolveLookupInRoot(
    int32_t userId, fuse_ino_t parent, const char *name, const RootContext &root, LookupResult &result)
{
    auto parentMetaFile =
        MetaFileMgr::GetInstance().GetCloudDiskServiceMetaFileIfExists(userId, root.syncFolderIndex, parent);
    if (parentMetaFile == nullptr) {
        return {RootLookupState::NO_PARENT, E_OK};
    }
    MetaBase meta(name);
    int32_t ret = parentMetaFile->DoLookupByName(meta);
    if (ret == ENOENT) {
        return {RootLookupState::TERMINAL_ERROR, ENOENT};
    }
    if (ret != E_OK) {
        return {RootLookupState::CONTINUE, EIO};
    }
    if (meta.placeholder == PLACEHOLDER_STATE_NONE || !S_ISREG(meta.mode)) {
        return {RootLookupState::TERMINAL_ERROR, ESTALE};
    }
    std::string relativePath;
    if (MetaFileMgr::GetInstance().GetRelativePathIfExists(parentMetaFile, relativePath) != E_OK) {
        return {RootLookupState::CONTINUE, EIO};
    }
    std::string candidate;
    ret = BuildLookupCandidate(root, relativePath, name, candidate);
    if (ret != E_OK) {
        return {RootLookupState::TERMINAL_ERROR, ret};
    }
    std::string canonicalPath;
    struct stat candidateAttr {};
    ret = ValidateLookupCandidate(root, candidate, canonicalPath, candidateAttr);
    if (ret != E_OK) {
        return {RootLookupState::CONTINUE, ret};
    }
    result.syncFolderIndex = root.syncFolderIndex;
    result.rootEpoch = root.warmupEpoch;
    result.path = canonicalPath;
    result.attr = candidateAttr;
    return {RootLookupState::MATCH, E_OK};
}

CloudDiskFuseController::RootLookupOutcome CloudDiskFuseController::ScanLookupRoots(
    const LookupSnapshot &snapshot, fuse_ino_t parent, const char *name, LookupResult &result)
{
    bool parentMetadataFound = false;
    int32_t lookupError = E_OK;
    for (const auto &root : snapshot.roots) {
        RootLookupOutcome outcome = ResolveLookupInRoot(snapshot.userId, parent, name, root, result);
        if (outcome.state == RootLookupState::NO_PARENT) {
            continue;
        }
        parentMetadataFound = true;
        if (outcome.state == RootLookupState::MATCH || outcome.state == RootLookupState::TERMINAL_ERROR) {
            return outcome;
        }
        lookupError = outcome.error;
    }
    if (!parentMetadataFound) {
        return {RootLookupState::NO_PARENT, E_OK};
    }
    return {RootLookupState::CONTINUE, lookupError == E_OK ? ENOENT : lookupError};
}

int32_t CloudDiskFuseController::WaitForLookupWarmup(const LookupSnapshot &snapshot)
{
    std::unique_lock<std::mutex> lock(mutex_);
    bool completed =
        condition_.wait_for(lock, std::chrono::milliseconds(CLOUD_DISK_FUSE_LOOKUP_WARMUP_WAIT_MS), [this, &snapshot] {
            if (state_ != State::READY || activeUserId_ != snapshot.userId || generation_ != snapshot.generation) {
                return true;
            }
            return std::none_of(roots_.begin(), roots_.end(), [](const auto &entry) {
                return entry.second.state == RootState::WARMING && entry.second.warmupInFlight;
            });
        });
    if (state_ != State::READY || activeUserId_ != snapshot.userId || generation_ != snapshot.generation) {
        return ENOTCONN;
    }
    return completed ? E_OK : EAGAIN;
}

void CloudDiskFuseController::PurgeRootLocked(uint32_t syncFolderIndex, std::vector<int> &fds)
{
    for (auto it = openContexts_.begin(); it != openContexts_.end();) {
        if (it->second->syncFolderIndex == syncFolderIndex) {
            fds.push_back(it->second->fd.Release());
            it = openContexts_.erase(it);
        } else {
            ++it;
        }
    }
    for (auto it = nodes_.begin(); it != nodes_.end();) {
        if (it->second.syncFolderIndex == syncFolderIndex) {
            identityToNode_.erase({static_cast<uint64_t>(it->second.device), static_cast<uint64_t>(it->second.inode)});
            it = nodes_.erase(it);
        } else {
            ++it;
        }
    }
}

void CloudDiskFuseController::PurgeAllLocked(std::vector<int> &fds)
{
    for (auto &[handle, context] : openContexts_) {
        (void)handle;
        fds.push_back(context->fd.Release());
    }
    openContexts_.clear();
    nodes_.clear();
    identityToNode_.clear();
    pendingOpenCount_ = 0;
    nextNodeId_ = FUSE_ROOT_ID + 1;
    nextHandle_ = 1;
}

void CloudDiskFuseController::RollbackLookup(fuse_ino_t nodeId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = nodes_.find(nodeId);
    if (it == nodes_.end()) {
        return;
    }
    if (it->second.nlookup > 0) {
        --it->second.nlookup;
    }
    if (it->second.nlookup == 0 && it->second.openCount == 0) {
        identityToNode_.erase({static_cast<uint64_t>(it->second.device), static_cast<uint64_t>(it->second.inode)});
        nodes_.erase(it);
    }
}

void CloudDiskFuseController::RollbackOpen(uint64_t handle)
{
    int fd = -1;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto contextIt = openContexts_.find(handle);
        if (contextIt == openContexts_.end()) {
            return;
        }
        fuse_ino_t nodeId = contextIt->second->nodeId;
        fd = contextIt->second->fd.Release();
        openContexts_.erase(contextIt);
        auto nodeIt = nodes_.find(nodeId);
        if (nodeIt != nodes_.end() && nodeIt->second.openCount > 0) {
            --nodeIt->second.openCount;
            if (nodeIt->second.nlookup == 0 && nodeIt->second.openCount == 0) {
                identityToNode_.erase(
                    {static_cast<uint64_t>(nodeIt->second.device), static_cast<uint64_t>(nodeIt->second.inode)});
                nodes_.erase(nodeIt);
            }
        }
    }
    if (fd >= 0) {
        (void)close(fd);
    }
}

void CloudDiskFuseController::Init(void *userdata, struct fuse_conn_info *conn)
{
    (void)conn;
    auto *controller = static_cast<CloudDiskFuseController *>(userdata);
    if (controller == nullptr) {
        return;
    }
    int32_t userId = -1;
    uint64_t generation = 0;
    bool ready = false;
    {
        std::lock_guard<std::mutex> lock(controller->mutex_);
        userId = controller->activeUserId_;
        generation = controller->generation_;
        if (controller->state_ == State::MOUNTING && controller->HasActiveRootLocked() && !controller->shutdown_) {
            controller->state_ = State::READY;
            ready = true;
        }
    }
    controller->condition_.notify_all();
    LOGI(
        "CloudDiskService FUSE connection initialized, userId: %{private}d, generation: %{public}llu, "
        "ready: %{public}d",
        userId, static_cast<unsigned long long>(generation), ready);
}

void CloudDiskFuseController::Destroy(void *userdata)
{
    (void)userdata;
    LOGI("CloudDiskService FUSE connection destroyed");
}

int32_t CloudDiskFuseController::RegisterLookupNode(const LookupResult &result, fuse_ino_t &nodeId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto rootIt = roots_.find(result.syncFolderIndex);
    if (state_ != State::READY || rootIt == roots_.end() || rootIt->second.warmupEpoch != result.rootEpoch ||
        rootIt->second.state != RootState::READY) {
        return ESTALE;
    }
    auto identity =
        std::make_pair(static_cast<uint64_t>(result.attr.st_dev), static_cast<uint64_t>(result.attr.st_ino));
    auto identityIt = identityToNode_.find(identity);
    if (identityIt != identityToNode_.end()) {
        nodeId = identityIt->second;
        auto nodeIt = nodes_.find(nodeId);
        if (nodeIt == nodes_.end() || nodeIt->second.nlookup == std::numeric_limits<uint64_t>::max()) {
            return EOVERFLOW;
        }
        nodeIt->second.path = result.path;
        nodeIt->second.syncFolderIndex = result.syncFolderIndex;
        nodeIt->second.rootEpoch = result.rootEpoch;
        ++nodeIt->second.nlookup;
        return E_OK;
    }
    if (nodes_.size() >= MAX_NODE_CONTEXTS || nextNodeId_ == 0) {
        return ENOSPC;
    }
    nodeId = nextNodeId_++;
    NodeContext node;
    node.nodeId = nodeId;
    node.syncFolderIndex = result.syncFolderIndex;
    node.path = result.path;
    node.device = result.attr.st_dev;
    node.inode = result.attr.st_ino;
    node.rootEpoch = result.rootEpoch;
    node.nlookup = 1;
    nodes_.emplace(nodeId, std::move(node));
    identityToNode_.emplace(identity, nodeId);
    return E_OK;
}

void CloudDiskFuseController::ForgetNode(fuse_ino_t ino, uint64_t nlookup)
{
    if (ino == FUSE_ROOT_ID) {
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = nodes_.find(ino);
    if (it == nodes_.end()) {
        return;
    }
    it->second.nlookup = nlookup >= it->second.nlookup ? 0 : it->second.nlookup - nlookup;
    if (it->second.nlookup == 0 && it->second.openCount == 0) {
        identityToNode_.erase({static_cast<uint64_t>(it->second.device), static_cast<uint64_t>(it->second.inode)});
        nodes_.erase(it);
    }
}

int32_t CloudDiskFuseController::GetAttrNode(fuse_ino_t ino, NodeContext &node)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (state_ != State::READY) {
        return ENOTCONN;
    }
    auto nodeIt = nodes_.find(ino);
    if (nodeIt == nodes_.end()) {
        return ESTALE;
    }
    auto rootIt = roots_.find(nodeIt->second.syncFolderIndex);
    if (rootIt == roots_.end() || rootIt->second.warmupEpoch != nodeIt->second.rootEpoch ||
        rootIt->second.state != RootState::READY) {
        return ESTALE;
    }
    node = nodeIt->second;
    return E_OK;
}

CloudDiskFuseController::OperationIdentity CloudDiskFuseController::GetOperationIdentity()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return {activeUserId_, generation_, 0};
}

int32_t CloudDiskFuseController::ReserveOpen(fuse_ino_t ino, OpenOperation &operation)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (state_ != State::READY) {
        return ENOTCONN;
    }
    auto nodeIt = nodes_.find(ino);
    if (nodeIt == nodes_.end()) {
        return ESTALE;
    }
    auto rootIt = roots_.find(nodeIt->second.syncFolderIndex);
    if (rootIt == roots_.end() || rootIt->second.warmupEpoch != nodeIt->second.rootEpoch ||
        rootIt->second.state != RootState::READY) {
        return ESTALE;
    }
    if (openContexts_.size() + pendingOpenCount_ >= MAX_OPEN_CONTEXTS) {
        return EMFILE;
    }
    ++pendingOpenCount_;
    operation.node = nodeIt->second;
    operation.generation = generation_;
    return E_OK;
}

void CloudDiskFuseController::ReleaseOpenReservation()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (pendingOpenCount_ > 0) {
        --pendingOpenCount_;
    }
}

int32_t CloudDiskFuseController::OpenBackingFile(OpenOperation &operation) const
{
    UniqueFd fd(open(operation.node.path.c_str(), O_RDONLY | O_CLOEXEC | O_NOFOLLOW));
    if (fd < 0) {
        return errno;
    }
    if (fstat(fd.Get(), &operation.attr) != 0) {
        return errno;
    }
    if (!S_ISREG(operation.attr.st_mode) || operation.attr.st_dev != operation.node.device ||
        operation.attr.st_ino != operation.node.inode) {
        return ESTALE;
    }
    operation.fd = std::move(fd);
    return E_OK;
}

int32_t CloudDiskFuseController::CommitOpen(fuse_ino_t ino, OpenOperation &operation)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (pendingOpenCount_ > 0) {
        --pendingOpenCount_;
    }
    if (state_ != State::READY || generation_ != operation.generation) {
        return ENOTCONN;
    }
    auto nodeIt = nodes_.find(ino);
    if (nodeIt == nodes_.end()) {
        return ESTALE;
    }
    auto rootIt = roots_.find(operation.node.syncFolderIndex);
    if (rootIt == roots_.end() || rootIt->second.warmupEpoch != operation.node.rootEpoch ||
        rootIt->second.state != RootState::READY) {
        return ESTALE;
    }
    if (nextHandle_ == 0) {
        return EMFILE;
    }
    operation.handle = nextHandle_++;
    auto context = std::make_unique<OpenContext>();
    context->handle = operation.handle;
    context->nodeId = ino;
    context->syncFolderIndex = operation.node.syncFolderIndex;
    context->generation = operation.generation;
    context->device = operation.attr.st_dev;
    context->inode = operation.attr.st_ino;
    context->fd = std::move(operation.fd);
    openContexts_.emplace(operation.handle, std::move(context));
    ++nodeIt->second.openCount;
    return E_OK;
}

int32_t CloudDiskFuseController::TakeOpenContext(fuse_ino_t ino, uint64_t handle, ReleaseOperation &operation)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto contextIt = openContexts_.find(handle);
    if (contextIt == openContexts_.end() || contextIt->second->nodeId != ino ||
        contextIt->second->generation != generation_) {
        return EBADF;
    }

    operation.identity.generation = contextIt->second->generation;
    operation.identity.rootIndex = contextIt->second->syncFolderIndex;
    operation.fd = contextIt->second->fd.Release();
    openContexts_.erase(contextIt);
    auto nodeIt = nodes_.find(ino);
    if (nodeIt == nodes_.end() || nodeIt->second.openCount == 0) {
        return E_OK;
    }
    --nodeIt->second.openCount;
    if (nodeIt->second.nlookup == 0 && nodeIt->second.openCount == 0) {
        identityToNode_.erase(
            {static_cast<uint64_t>(nodeIt->second.device), static_cast<uint64_t>(nodeIt->second.inode)});
        nodes_.erase(nodeIt);
    }
    return E_OK;
}

ssize_t CloudDiskFuseController::ReadFuseFd(int fd, void *buf, size_t len, void *userdata)
{
    auto *controller = static_cast<CloudDiskFuseController *>(userdata);
    if (controller == nullptr) {
        errno = EINVAL;
        return -1;
    }
    int wakeFd = -1;
    {
        std::lock_guard<std::mutex> lock(controller->mutex_);
        wakeFd = controller->wakeFd_;
    }

    struct pollfd pollFds[FUSE_POLL_FD_COUNT] = {
        {fd, POLLIN, 0},
        {wakeFd, POLLIN, 0},
    };
    int ret;
    do {
        ret = poll(pollFds, wakeFd >= 0 ? FUSE_POLL_FD_COUNT : FUSE_ONLY_POLL_FD_COUNT, -1);
    } while (ret < 0 && errno == EINTR);
    if (ret < 0) {
        return -1;
    }
    if (wakeFd >= 0 && (pollFds[1].revents & POLLIN) != 0) {
        errno = EINTR;
        return -1;
    }
    return read(fd, buf, len);
}

ssize_t CloudDiskFuseController::WriteFuseFd(int fd, struct iovec *iov, int count, void *userdata)
{
    (void)userdata;
    return writev(fd, iov, count);
}

} // namespace OHOS::FileManagement::CloudDiskService
