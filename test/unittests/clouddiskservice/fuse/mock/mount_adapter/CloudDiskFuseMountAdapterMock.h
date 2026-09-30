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

#ifndef CLOUD_DISK_FUSE_MOUNT_ADAPTER_MOCK_H
#define CLOUD_DISK_FUSE_MOUNT_ADAPTER_MOCK_H

#define UTILS_LOG_H

#include <cstdint>
#include <memory>
#include <string>

// cloud_disk_fuse_mount_adapter.cpp is compiled with this header force-included. These guards keep the target
// independent from the real SA and generated StorageManager IPC implementations.
#define SERVICE_REGISTRY_INCLUDE_H
#define OHOS_STORAGEMANAGER_STORAGEMANAGERPROXY_H
#define UTILS_SYSTEM_ABILITY_DEFINITION_H

namespace OHOS {

template<typename T>
using sptr = std::shared_ptr<T>;

class IRemoteObject {
public:
    virtual ~IRemoteObject() = default;

    virtual bool IsStorageManager() const
    {
        return false;
    }
};

class ISystemAbilityManager {
public:
    virtual ~ISystemAbilityManager() = default;
    virtual sptr<IRemoteObject> GetSystemAbility(int32_t systemAbilityId) = 0;
};

class SystemAbilityManagerClient {
public:
    static SystemAbilityManagerClient &GetInstance();
    sptr<ISystemAbilityManager> GetSystemAbilityManager();
};

template<typename T>
sptr<T> iface_cast(const sptr<IRemoteObject> &remoteObject)
{
    if (remoteObject == nullptr || !remoteObject->IsStorageManager()) {
        return nullptr;
    }
    return std::static_pointer_cast<T>(remoteObject);
}

constexpr int32_t STORAGE_MANAGER_MANAGER_ID = 5003;

namespace StorageManager {

class IStorageManager : public IRemoteObject {
public:
    ~IStorageManager() override = default;

    bool IsStorageManager() const override
    {
        return true;
    }

    virtual int32_t MountCloudDiskFuse(int32_t userId, const std::string &mountPoint, int &fuseFd) = 0;
    virtual int32_t UMountCloudDiskFuse(int32_t userId, const std::string &mountPoint) = 0;
};

} // namespace StorageManager

namespace FileManagement::CloudDiskService::Test {

enum class StorageManagerRemoteKind {
    NONE,
    WRONG_INTERFACE,
    STORAGE_MANAGER,
};

struct CloudDiskFuseMountAdapterMockState {
    bool systemAbilityManagerAvailable{true};
    StorageManagerRemoteKind remoteKind{StorageManagerRemoteKind::STORAGE_MANAGER};
    uint32_t getSystemAbilityManagerCallCount{0};
    uint32_t getSystemAbilityCallCount{0};
    int32_t requestedSystemAbilityId{0};

    int32_t mountResult{0};
    int32_t mountOutputFd{-1};
    uint32_t mountCallCount{0};
    int32_t mountUserId{-1};
    std::string mountPoint;

    int32_t unmountResult{0};
    uint32_t unmountCallCount{0};
    int32_t unmountUserId{-1};
    std::string unmountPoint;

    bool mockClose{false};
    int32_t closeFd{-1};
    int32_t closeResult{0};
    int32_t closeError{0};
    uint32_t closeCallCount{0};
    uint32_t errorLogCallCount{0};
};

CloudDiskFuseMountAdapterMockState &GetCloudDiskFuseMountAdapterMockState();
void ResetCloudDiskFuseMountAdapterMock();
void RecordCloudDiskFuseMountAdapterErrorLog();

} // namespace FileManagement::CloudDiskService::Test
} // namespace OHOS

#define LOGE(...) ::OHOS::FileManagement::CloudDiskService::Test::RecordCloudDiskFuseMountAdapterErrorLog()

#endif // CLOUD_DISK_FUSE_MOUNT_ADAPTER_MOCK_H
