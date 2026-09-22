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

#include "CloudDiskFuseMountAdapterMock.h"

#include <cerrno>
#include <sys/syscall.h>
#include <unistd.h>

namespace OHOS::FileManagement::CloudDiskService::Test {
namespace {
CloudDiskFuseMountAdapterMockState g_mockState;

class StorageManagerMock final : public StorageManager::IStorageManager {
public:
    int32_t MountCloudDiskFuse(int32_t userId, const std::string &mountPoint, int &fuseFd) override
    {
        auto &state = GetCloudDiskFuseMountAdapterMockState();
        ++state.mountCallCount;
        state.mountUserId = userId;
        state.mountPoint = mountPoint;
        fuseFd = state.mountOutputFd;
        return state.mountResult;
    }

    int32_t UMountCloudDiskFuse(int32_t userId, const std::string &mountPoint) override
    {
        auto &state = GetCloudDiskFuseMountAdapterMockState();
        ++state.unmountCallCount;
        state.unmountUserId = userId;
        state.unmountPoint = mountPoint;
        return state.unmountResult;
    }
};

class SystemAbilityManagerMock final : public ISystemAbilityManager {
public:
    sptr<IRemoteObject> GetSystemAbility(int32_t systemAbilityId) override
    {
        auto &state = GetCloudDiskFuseMountAdapterMockState();
        ++state.getSystemAbilityCallCount;
        state.requestedSystemAbilityId = systemAbilityId;
        if (state.remoteKind == StorageManagerRemoteKind::NONE) {
            return nullptr;
        }
        if (state.remoteKind == StorageManagerRemoteKind::WRONG_INTERFACE) {
            return std::make_shared<IRemoteObject>();
        }
        static sptr<StorageManagerMock> storageManager = std::make_shared<StorageManagerMock>();
        return storageManager;
    }
};
} // namespace

sptr<ISystemAbilityManager> GetSystemAbilityManagerMock()
{
    static sptr<ISystemAbilityManager> manager = std::make_shared<SystemAbilityManagerMock>();
    return manager;
}

CloudDiskFuseMountAdapterMockState &GetCloudDiskFuseMountAdapterMockState()
{
    return g_mockState;
}

void ResetCloudDiskFuseMountAdapterMock()
{
    g_mockState = {};
    g_mockState.systemAbilityManagerAvailable = true;
    g_mockState.remoteKind = StorageManagerRemoteKind::STORAGE_MANAGER;
    g_mockState.mountOutputFd = -1;
    g_mockState.mountUserId = -1;
    g_mockState.unmountUserId = -1;
    g_mockState.closeFd = -1;
}

void RecordCloudDiskFuseMountAdapterErrorLog()
{
    ++g_mockState.errorLogCallCount;
}

} // namespace OHOS::FileManagement::CloudDiskService::Test

namespace OHOS {
using FileManagement::CloudDiskService::Test::GetCloudDiskFuseMountAdapterMockState;

SystemAbilityManagerClient &SystemAbilityManagerClient::GetInstance()
{
    static SystemAbilityManagerClient instance;
    return instance;
}

sptr<ISystemAbilityManager> SystemAbilityManagerClient::GetSystemAbilityManager()
{
    auto &state = GetCloudDiskFuseMountAdapterMockState();
    ++state.getSystemAbilityManagerCallCount;
    if (!state.systemAbilityManagerAvailable) {
        return nullptr;
    }
    return FileManagement::CloudDiskService::Test::GetSystemAbilityManagerMock();
}

} // namespace OHOS

extern "C" int close(int fd)
{
    auto &state = OHOS::FileManagement::CloudDiskService::Test::GetCloudDiskFuseMountAdapterMockState();
    if (state.mockClose && fd == state.closeFd) {
        ++state.closeCallCount;
        errno = state.closeError;
        return state.closeResult;
    }
    return static_cast<int>(syscall(SYS_close, fd));
}
