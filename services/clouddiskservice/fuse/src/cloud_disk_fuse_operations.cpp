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

#include "cloud_disk_fuse_operations.h"

#include <atomic>
#include <cerrno>
#include <chrono>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "cloud_disk_fuse_controller.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudDiskService {
namespace {
constexpr int32_t E_OK = 0;
constexpr mode_t FUSE_ROOT_MODE = 0550;
constexpr nlink_t FUSE_ROOT_LINK_COUNT = 2;
constexpr int MUTATING_OPEN_FLAGS = O_CREAT | O_EXCL | O_TRUNC | O_APPEND;
constexpr double LOOKUP_ENTRY_TIMEOUT_SECONDS = 1.0;
constexpr double LOOKUP_ATTR_TIMEOUT_SECONDS = 1.0;
constexpr double GETATTR_ATTR_TIMEOUT_SECONDS = 0.0;
constexpr uint32_t OPERATION_LOG_LIMIT = 16;
constexpr uint32_t TIMED_OPERATION_LOG_LIMIT = 32;
constexpr int64_t OPERATION_LOG_WINDOW_SECONDS = 10;

std::atomic<int64_t> g_openLogWindowStart{0};
std::atomic<uint32_t> g_openLogWindowCount{0};
std::atomic<int64_t> g_releaseLogWindowStart{0};
std::atomic<uint32_t> g_releaseLogWindowCount{0};

struct TimedOperationContext {
    const char *operation;
    int32_t userId;
    uint64_t generation;
    uint32_t rootIndex;
    fuse_ino_t nodeId;
    uint64_t handle;
    int32_t result;
};

bool ShouldLogOperation(std::atomic<uint32_t> &counter)
{
    return counter.fetch_add(1, std::memory_order_relaxed) < OPERATION_LOG_LIMIT;
}

bool ShouldLogTimedOperation(std::atomic<int64_t> &windowStart, std::atomic<uint32_t> &windowCount)
{
    auto now =
        std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    int64_t observedStart = windowStart.load(std::memory_order_relaxed);
    if (now - observedStart >= OPERATION_LOG_WINDOW_SECONDS &&
        windowStart.compare_exchange_strong(observedStart, now, std::memory_order_relaxed)) {
        windowCount.store(0, std::memory_order_relaxed);
    }
    return windowCount.fetch_add(1, std::memory_order_relaxed) < TIMED_OPERATION_LOG_LIMIT;
}

void LogTimedOperation(const TimedOperationContext &context,
                       const std::chrono::steady_clock::time_point &start,
                       std::atomic<int64_t> &windowStart,
                       std::atomic<uint32_t> &windowCount)
{
    if (!ShouldLogTimedOperation(windowStart, windowCount)) {
        return;
    }
    auto duration =
        std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start).count();
    LOGI(
        "CloudDiskService FUSE operation=%{public}s userId=%{private}d generation=%{public}llu "
        "root=%{private}u nodeId=%{public}llu handle=%{public}llu result=%{public}d durationUs=%{public}lld",
        context.operation, context.userId, static_cast<unsigned long long>(context.generation), context.rootIndex,
        static_cast<unsigned long long>(context.nodeId), static_cast<unsigned long long>(context.handle),
        context.result, static_cast<long long>(duration));
}

int32_t ValidateOpenRequest(const struct fuse_file_info *fi)
{
    if (fi == nullptr) {
        return EINVAL;
    }
    if ((fi->flags & O_DIRECTORY) != 0) {
        return ENOTDIR;
    }
    if ((fi->flags & O_ACCMODE) != O_RDONLY || (fi->flags & MUTATING_OPEN_FLAGS) != 0) {
        return EROFS;
    }
    return E_OK;
}
} // namespace

void CloudDiskFuseOperations::Lookup(fuse_req_t req, fuse_ino_t parent, const char *name)
{
    auto *controller = static_cast<CloudDiskFuseController *>(fuse_req_userdata(req));
    if (controller == nullptr) {
        (void)fuse_reply_err(req, EIO);
        return;
    }

    CloudDiskFuseController::LookupResult result;
    int32_t ret = controller->ResolveLookup(parent, name, result);
    if (ret != E_OK) {
        (void)fuse_reply_err(req, ret);
        return;
    }

    fuse_ino_t nodeId = 0;
    ret = controller->RegisterLookupNode(result, nodeId);
    if (ret != E_OK) {
        (void)fuse_reply_err(req, ret);
        return;
    }

    struct fuse_entry_param entry {};
    entry.ino = nodeId;
    entry.generation = 0;
    entry.attr = result.attr;
    entry.attr_timeout = LOOKUP_ATTR_TIMEOUT_SECONDS;
    entry.entry_timeout = LOOKUP_ENTRY_TIMEOUT_SECONDS;
    if (fuse_reply_entry(req, &entry) != 0) {
        controller->RollbackLookup(nodeId);
    } else if (ShouldLogOperation(controller->lookupLogCount_)) {
        LOGI("CloudDiskService FUSE operation=LOOKUP result=success nodeId=%{public}llu",
             static_cast<unsigned long long>(nodeId));
    }
}

void CloudDiskFuseOperations::Forget(fuse_req_t req, fuse_ino_t ino, uint64_t nlookup)
{
    auto *controller = static_cast<CloudDiskFuseController *>(fuse_req_userdata(req));
    if (controller != nullptr) {
        controller->ForgetNode(ino, nlookup);
    }
    fuse_reply_none(req);
    if (controller != nullptr && ShouldLogOperation(controller->forgetLogCount_)) {
        LOGI("CloudDiskService FUSE operation=FORGET result=success nodeId=%{public}llu",
             static_cast<unsigned long long>(ino));
    }
}

void CloudDiskFuseOperations::GetAttr(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi)
{
    (void)fi;
    auto *controller = static_cast<CloudDiskFuseController *>(fuse_req_userdata(req));
    if (controller == nullptr) {
        (void)fuse_reply_err(req, EIO);
        return;
    }

    if (ino == FUSE_ROOT_ID) {
        struct stat attr {};
        attr.st_ino = FUSE_ROOT_ID;
        attr.st_mode = S_IFDIR | FUSE_ROOT_MODE;
        attr.st_nlink = FUSE_ROOT_LINK_COUNT;
        (void)fuse_reply_attr(req, &attr, GETATTR_ATTR_TIMEOUT_SECONDS);
        return;
    }

    CloudDiskFuseController::NodeContext node;
    int32_t ret = controller->GetAttrNode(ino, node);
    if (ret != E_OK) {
        (void)fuse_reply_err(req, ret);
        return;
    }

    struct stat attr {};
    if (lstat(node.path.c_str(), &attr) != 0) {
        (void)fuse_reply_err(req, errno);
        return;
    }
    if (!S_ISREG(attr.st_mode) || attr.st_dev != node.device || attr.st_ino != node.inode) {
        (void)fuse_reply_err(req, ESTALE);
        return;
    }
    if (fuse_reply_attr(req, &attr, GETATTR_ATTR_TIMEOUT_SECONDS) == 0 &&
        ShouldLogOperation(controller->getattrLogCount_)) {
        LOGI("CloudDiskService FUSE operation=GETATTR result=success nodeId=%{public}llu",
             static_cast<unsigned long long>(ino));
    }
}

void CloudDiskFuseOperations::Open(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi)
{
    auto start = std::chrono::steady_clock::now();
    auto *controller = static_cast<CloudDiskFuseController *>(fuse_req_userdata(req));
    if (controller == nullptr) {
        (void)fuse_reply_err(req, EINVAL);
        return;
    }
    CloudDiskFuseController::OperationIdentity identity = controller->GetOperationIdentity();
    auto replyError = [&](int32_t error) {
        TimedOperationContext context = {"OPEN", identity.userId, identity.generation, identity.rootIndex, ino, 0,
                                         error};
        LogTimedOperation(context, start, g_openLogWindowStart, g_openLogWindowCount);
        (void)fuse_reply_err(req, error);
    };
    int32_t ret = ValidateOpenRequest(fi);
    if (ret != E_OK) {
        replyError(ret);
        return;
    }

    CloudDiskFuseController::OpenOperation operation;
    ret = controller->ReserveOpen(ino, operation);
    if (ret != E_OK) {
        replyError(ret);
        return;
    }
    identity.generation = operation.generation;
    identity.rootIndex = operation.node.syncFolderIndex;
    ret = controller->OpenBackingFile(operation);
    if (ret != E_OK) {
        controller->ReleaseOpenReservation();
        replyError(ret);
        return;
    }
    ret = controller->CommitOpen(ino, operation);
    if (ret != E_OK) {
        replyError(ret);
        return;
    }

    fi->fh = operation.handle;
    int32_t replyRet = fuse_reply_open(req, fi);
    if (replyRet != 0) {
        controller->RollbackOpen(operation.handle);
    }
    int32_t result = replyRet < 0 ? -replyRet : replyRet;
    TimedOperationContext context = {"OPEN",           identity.userId, identity.generation, identity.rootIndex, ino,
                                     operation.handle, result};
    LogTimedOperation(context, start, g_openLogWindowStart, g_openLogWindowCount);
}

void CloudDiskFuseOperations::Release(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi)
{
    auto start = std::chrono::steady_clock::now();
    auto *controller = static_cast<CloudDiskFuseController *>(fuse_req_userdata(req));
    if (controller == nullptr) {
        (void)fuse_reply_err(req, EBADF);
        return;
    }

    CloudDiskFuseController::ReleaseOperation operation;
    operation.identity = controller->GetOperationIdentity();
    operation.handle = fi == nullptr ? 0 : fi->fh;
    auto logOperation = [&](int32_t result) {
        TimedOperationContext context = {"RELEASE",
                                         operation.identity.userId,
                                         operation.identity.generation,
                                         operation.identity.rootIndex,
                                         ino,
                                         operation.handle,
                                         result};
        LogTimedOperation(context, start, g_releaseLogWindowStart, g_releaseLogWindowCount);
    };
    if (fi == nullptr || fi->fh == 0) {
        logOperation(EBADF);
        (void)fuse_reply_err(req, EBADF);
        return;
    }

    int32_t error = controller->TakeOpenContext(ino, fi->fh, operation);
    if (error == E_OK && operation.fd >= 0 && close(operation.fd) != 0) {
        error = errno;
    }
    logOperation(error);
    (void)fuse_reply_err(req, error);
}

} // namespace OHOS::FileManagement::CloudDiskService
