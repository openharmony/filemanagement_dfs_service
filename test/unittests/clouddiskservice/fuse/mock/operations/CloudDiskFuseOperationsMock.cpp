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

#include "CloudDiskFuseOperationsMock.h"

#include <cerrno>
#include <dlfcn.h>
#include <sys/syscall.h>
#include <unistd.h>

namespace OHOS::FileManagement::CloudDiskService::Test {
namespace {
CloudDiskFuseOperationsMockState g_mockState;
}

CloudDiskFuseOperationsMockState &GetCloudDiskFuseOperationsMockState()
{
    return g_mockState;
}

void ResetCloudDiskFuseOperationsMock()
{
    g_mockState = {};
    g_mockState.identityUserId = -1;
    g_mockState.releaseIdentityUserId = -1;
    g_mockState.releaseFd = -1;
    g_mockState.closeFd = -1;
}

CloudDiskFuseController &GetCloudDiskFuseOperationsMockController()
{
    return CloudDiskFuseController::GetInstance();
}

void RecordCloudDiskFuseOperationsInfoLog()
{
    ++g_mockState.infoLogCallCount;
}

} // namespace OHOS::FileManagement::CloudDiskService::Test

namespace OHOS::FileManagement::CloudDiskService {
using Test::GetCloudDiskFuseOperationsMockState;

CloudDiskFuseController &CloudDiskFuseController::GetInstance()
{
    static CloudDiskFuseController instance;
    auto &state = GetCloudDiskFuseOperationsMockState();
    instance.lookupLogCount_.store(state.lookupLogCountSeed, std::memory_order_relaxed);
    instance.forgetLogCount_.store(state.forgetLogCountSeed, std::memory_order_relaxed);
    instance.getattrLogCount_.store(state.getattrLogCountSeed, std::memory_order_relaxed);
    return instance;
}

CloudDiskFuseController::~CloudDiskFuseController() = default;

int32_t CloudDiskFuseController::ResolveLookup(fuse_ino_t parent, const char *name, LookupResult &result)
{
    auto &state = GetCloudDiskFuseOperationsMockState();
    ++state.resolveLookupCallCount;
    state.lookupParent = parent;
    state.lookupNameIsNull = name == nullptr;
    state.lookupName = name == nullptr ? "" : name;
    result.syncFolderIndex = state.lookupSyncFolderIndex;
    result.rootEpoch = state.lookupRootEpoch;
    result.path = state.lookupPath;
    result.attr = state.lookupAttr;
    return state.resolveLookupResult;
}

int32_t CloudDiskFuseController::RegisterLookupNode(const LookupResult &result, fuse_ino_t &nodeId)
{
    (void)result;
    auto &state = GetCloudDiskFuseOperationsMockState();
    ++state.registerLookupCallCount;
    nodeId = state.registerLookupNodeId;
    return state.registerLookupResult;
}

void CloudDiskFuseController::RollbackLookup(fuse_ino_t nodeId)
{
    auto &state = GetCloudDiskFuseOperationsMockState();
    ++state.rollbackLookupCallCount;
    state.rollbackLookupNodeId = nodeId;
}

void CloudDiskFuseController::ForgetNode(fuse_ino_t ino, uint64_t nlookup)
{
    auto &state = GetCloudDiskFuseOperationsMockState();
    ++state.forgetNodeCallCount;
    state.forgetNodeId = ino;
    state.forgetNodeLookupCount = nlookup;
}

int32_t CloudDiskFuseController::GetAttrNode(fuse_ino_t ino, NodeContext &node)
{
    auto &state = GetCloudDiskFuseOperationsMockState();
    ++state.getAttrNodeCallCount;
    state.getAttrNodeId = ino;
    node.syncFolderIndex = state.nodeSyncFolderIndex;
    node.path = state.nodePath;
    node.device = state.nodeDevice;
    node.inode = state.nodeInode;
    return state.getAttrNodeResult;
}

CloudDiskFuseController::OperationIdentity CloudDiskFuseController::GetOperationIdentity()
{
    auto &state = GetCloudDiskFuseOperationsMockState();
    ++state.getIdentityCallCount;
    return {state.identityUserId, state.identityGeneration, state.identityRootIndex};
}

int32_t CloudDiskFuseController::ReserveOpen(fuse_ino_t ino, OpenOperation &operation)
{
    auto &state = GetCloudDiskFuseOperationsMockState();
    ++state.reserveOpenCallCount;
    state.reserveOpenNodeId = ino;
    operation.node.syncFolderIndex = state.reserveOpenSyncFolderIndex;
    operation.generation = state.reserveOpenGeneration;
    return state.reserveOpenResult;
}

void CloudDiskFuseController::ReleaseOpenReservation()
{
    ++GetCloudDiskFuseOperationsMockState().releaseOpenReservationCallCount;
}

int32_t CloudDiskFuseController::OpenBackingFile(OpenOperation &operation) const
{
    (void)operation;
    auto &state = GetCloudDiskFuseOperationsMockState();
    ++state.openBackingFileCallCount;
    return state.openBackingFileResult;
}

int32_t CloudDiskFuseController::CommitOpen(fuse_ino_t ino, OpenOperation &operation)
{
    auto &state = GetCloudDiskFuseOperationsMockState();
    ++state.commitOpenCallCount;
    state.commitOpenNodeId = ino;
    operation.handle = state.committedHandle;
    return state.commitOpenResult;
}

void CloudDiskFuseController::RollbackOpen(uint64_t handle)
{
    auto &state = GetCloudDiskFuseOperationsMockState();
    ++state.rollbackOpenCallCount;
    state.rollbackOpenHandle = handle;
}

int32_t CloudDiskFuseController::TakeOpenContext(fuse_ino_t ino, uint64_t handle, ReleaseOperation &operation)
{
    auto &state = GetCloudDiskFuseOperationsMockState();
    ++state.takeOpenContextCallCount;
    state.takeOpenNodeId = ino;
    state.takeOpenHandle = handle;
    operation.identity = {state.releaseIdentityUserId, state.releaseIdentityGeneration, state.releaseIdentityRootIndex};
    operation.handle = handle;
    operation.fd = state.releaseFd;
    return state.takeOpenContextResult;
}

} // namespace OHOS::FileManagement::CloudDiskService

extern "C" {
void *fuse_req_userdata(fuse_req_t req)
{
    return req == nullptr ? nullptr : req->userdata;
}

int fuse_reply_err(fuse_req_t req, int err)
{
    if (req != nullptr) {
        req->reply.error = err;
        ++req->reply.errorReplyCount;
    }
    return 0;
}

void fuse_reply_none(fuse_req_t req)
{
    if (req != nullptr) {
        ++req->reply.noneReplyCount;
    }
}

int fuse_reply_entry(fuse_req_t req, const struct fuse_entry_param *entry)
{
    if (req == nullptr) {
        return -EINVAL;
    }
    ++req->reply.entryReplyCount;
    if (entry != nullptr) {
        req->reply.entry = *entry;
    }
    return req->reply.entryReplyResult;
}

int fuse_reply_attr(fuse_req_t req, const struct stat *attr, double attrTimeout)
{
    if (req == nullptr) {
        return -EINVAL;
    }
    ++req->reply.attrReplyCount;
    if (attr != nullptr) {
        req->reply.attr = *attr;
    }
    req->reply.attrTimeout = attrTimeout;
    return req->reply.attrReplyResult;
}

int fuse_reply_open(fuse_req_t req, const struct fuse_file_info *fileInfo)
{
    if (req == nullptr) {
        return -EINVAL;
    }
    ++req->reply.openReplyCount;
    if (fileInfo != nullptr) {
        req->reply.fileInfo = *fileInfo;
    }
    return req->reply.openReplyResult;
}

int lstat(const char *path, struct stat *buf)
{
    auto &state = OHOS::FileManagement::CloudDiskService::Test::GetCloudDiskFuseOperationsMockState();
    if (state.mockLstat) {
        ++state.lstatCallCount;
        state.lstatPath = path == nullptr ? "" : path;
        if (state.lstatResult == 0 && buf != nullptr) {
            *buf = state.lstatAttr;
        }
        errno = state.lstatError;
        return state.lstatResult;
    }
    static auto realLstat = reinterpret_cast<int (*)(const char *, struct stat *)>(dlsym(RTLD_NEXT, "lstat"));
    if (realLstat == nullptr) {
        errno = EIO;
        return -1;
    }
    return realLstat(path, buf);
}

int close(int fd)
{
    auto &state = OHOS::FileManagement::CloudDiskService::Test::GetCloudDiskFuseOperationsMockState();
    if (state.mockClose && fd == state.closeFd) {
        ++state.closeCallCount;
        errno = state.closeError;
        return state.closeResult;
    }
    return static_cast<int>(syscall(SYS_close, fd));
}
}
