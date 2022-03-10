/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#ifndef DFS_DEVICE_MANAGER_AGENT_H
#define DFS_DEVICE_MANAGER_AGENT_H

#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <unordered_map>
#include <vector>

#include "device_manager.h"
#include "dfsu_singleton.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
class DeviceManagerAgent : public DistributedHardware::DmInitCallback,
                           public DistributedHardware::DeviceStateCallback,
                           public std::enable_shared_from_this<DeviceManagerAgent>,
                           public Utils::DfsuSingleton<DeviceManagerAgent> {
    DECLARE_SINGLETON(DeviceManagerAgent);

public:
    void OnDeviceOnline(const DistributedHardware::DmDeviceInfo &deviceInfo) override;
    void OnDeviceOffline(const DistributedHardware::DmDeviceInfo &deviceInfo) override;
    void OnDeviceChanged(const DistributedHardware::DmDeviceInfo &deviceInfo) override {}
    void OnDeviceReady(const DistributedHardware::DmDeviceInfo &deviceInfo) override {}
    void OfflineAllDevice();
    void OnRemoteDied() override;
    std::set<std::string> getOnlineDevs() const
    {
        return alreadyOnlineDev_;
    }
    std::vector<std::string> GetRemoteDevicesInfo();

private:
    void StartInstance() override;
    void StopInstance() override;

    void RegisterToExternalDm();
    void UnregisterFromExternalDm();

    std::mutex devsRecordMutex_;
    std::set<std::string> alreadyOnlineDev_;
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // DFS_DEVICE_MANAGER_AGENT_H