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

#ifndef CLOUD_DISK_FUSE_OPERATIONS_MOCK_H
#define CLOUD_DISK_FUSE_OPERATIONS_MOCK_H

#define UTILS_LOG_H

#include <cstdint>
#include <string>
#include <sys/stat.h>

#include "cloud_disk_fuse_controller.h"

namespace OHOS::FileManagement::CloudDiskService::Test {

struct FuseReplyMockState {
    int32_t error{0};
    uint32_t errorReplyCount{0};
    uint32_t noneReplyCount{0};
    uint32_t entryReplyCount{0};
    uint32_t attrReplyCount{0};
    uint32_t openReplyCount{0};
    int32_t entryReplyResult{0};
    int32_t attrReplyResult{0};
    int32_t openReplyResult{0};
    struct fuse_entry_param entry {};
    struct stat attr {};
    double attrTimeout{0.0};
    struct fuse_file_info fileInfo {};
};

struct CloudDiskFuseOperationsMockState {
    int32_t resolveLookupResult{0};
    uint32_t resolveLookupCallCount{0};
    fuse_ino_t lookupParent{0};
    bool lookupNameIsNull{false};
    std::string lookupName;
    uint32_t lookupSyncFolderIndex{0};
    uint64_t lookupRootEpoch{0};
    std::string lookupPath;
    struct stat lookupAttr {};

    int32_t registerLookupResult{0};
    uint32_t registerLookupCallCount{0};
    fuse_ino_t registerLookupNodeId{FUSE_ROOT_ID + 1};
    uint32_t rollbackLookupCallCount{0};
    fuse_ino_t rollbackLookupNodeId{0};

    uint32_t forgetNodeCallCount{0};
    fuse_ino_t forgetNodeId{0};
    uint64_t forgetNodeLookupCount{0};

    int32_t getAttrNodeResult{0};
    uint32_t getAttrNodeCallCount{0};
    fuse_ino_t getAttrNodeId{0};
    uint32_t nodeSyncFolderIndex{0};
    std::string nodePath;
    dev_t nodeDevice{0};
    ino_t nodeInode{0};

    int32_t identityUserId{-1};
    uint64_t identityGeneration{0};
    uint32_t identityRootIndex{0};
    uint32_t getIdentityCallCount{0};

    int32_t reserveOpenResult{0};
    uint32_t reserveOpenCallCount{0};
    fuse_ino_t reserveOpenNodeId{0};
    uint32_t reserveOpenSyncFolderIndex{0};
    uint64_t reserveOpenGeneration{0};
    uint32_t releaseOpenReservationCallCount{0};

    int32_t openBackingFileResult{0};
    uint32_t openBackingFileCallCount{0};
    int32_t commitOpenResult{0};
    uint32_t commitOpenCallCount{0};
    fuse_ino_t commitOpenNodeId{0};
    uint64_t committedHandle{1};
    uint32_t rollbackOpenCallCount{0};
    uint64_t rollbackOpenHandle{0};

    int32_t takeOpenContextResult{0};
    uint32_t takeOpenContextCallCount{0};
    fuse_ino_t takeOpenNodeId{0};
    uint64_t takeOpenHandle{0};
    int32_t releaseIdentityUserId{-1};
    uint64_t releaseIdentityGeneration{0};
    uint32_t releaseIdentityRootIndex{0};
    int32_t releaseFd{-1};

    uint32_t lookupLogCountSeed{0};
    uint32_t forgetLogCountSeed{0};
    uint32_t getattrLogCountSeed{0};

    bool mockLstat{false};
    int32_t lstatResult{0};
    int32_t lstatError{0};
    uint32_t lstatCallCount{0};
    std::string lstatPath;
    struct stat lstatAttr {};

    bool mockClose{false};
    int32_t closeFd{-1};
    int32_t closeResult{0};
    int32_t closeError{0};
    uint32_t closeCallCount{0};

    uint32_t infoLogCallCount{0};
};

CloudDiskFuseOperationsMockState &GetCloudDiskFuseOperationsMockState();
void ResetCloudDiskFuseOperationsMock();
CloudDiskFuseController &GetCloudDiskFuseOperationsMockController();
void RecordCloudDiskFuseOperationsInfoLog();

template<typename... Args>
void RecordCloudDiskFuseOperationsInfoLog(const Args &...)
{
    RecordCloudDiskFuseOperationsInfoLog();
}

} // namespace OHOS::FileManagement::CloudDiskService::Test

struct fuse_req {
    void *userdata{nullptr};
    OHOS::FileManagement::CloudDiskService::Test::FuseReplyMockState reply;
};

#define LOGI(...) ::OHOS::FileManagement::CloudDiskService::Test::RecordCloudDiskFuseOperationsInfoLog(__VA_ARGS__)

#endif // CLOUD_DISK_FUSE_OPERATIONS_MOCK_H
