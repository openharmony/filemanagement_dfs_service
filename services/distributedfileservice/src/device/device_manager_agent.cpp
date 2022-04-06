/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "device_manager_agent.h"
#include "dfsu_exception.h"
#include "distributedfile_service.h"
#include "softbus_agent.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
DeviceManagerAgent::DeviceManagerAgent() : DfsuActor<DeviceManagerAgent>(this, std::numeric_limits<uint32_t>::max()) {}

DeviceManagerAgent::~DeviceManagerAgent()
{
    StopInstance();
}

void DeviceManagerAgent::StartInstance()
{
    StartActor();
}

void DeviceManagerAgent::StopInstance()
{
    StopActor();
}

void DeviceManagerAgent::Start()
{
    // the time sequence can ensure there is no resource competition
    alreadyOnlineDev_.clear();
    RegisterToExternalDm();
}

void DeviceManagerAgent::Stop()
{
    UnregisterFromExternalDm();
}

std::vector<DistributedHardware::DmDeviceInfo> DeviceManagerAgent::GetRemoteDevicesInfo()
{
    std::string extra = "";
    std::vector<DistributedHardware::DmDeviceInfo> deviceList;

    auto &deviceManager = DistributedHardware::DeviceManager::GetInstance();
    int errCode = deviceManager.GetTrustedDeviceList(DistributedFileService::pkgName_, extra, deviceList);
    if (errCode) {
        ThrowException(errCode, "Failed to get info of remote devices");
    }
    return deviceList;
}

void DeviceManagerAgent::OnDeviceOnline(const DistributedHardware::DmDeviceInfo &deviceInfo)
{
    std::unique_lock<std::mutex> lock(devsRecordMutex_);
    std::string cid = std::string(deviceInfo.deviceId);
    alreadyOnlineDev_.insert(cid);
    SoftbusAgent::GetInstance()->OnDeviceOnline(cid);
}

void DeviceManagerAgent::OnDeviceOffline(const DistributedHardware::DmDeviceInfo &deviceInfo)
{
    std::unique_lock<std::mutex> lock(devsRecordMutex_);
    std::string cid = std::string(deviceInfo.deviceId);
    auto softBusAgent = SoftbusAgent::GetInstance();
    softBusAgent->OnDeviceOffline(cid);

    alreadyOnlineDev_.erase(cid);
    LOGI("cid %{public}s offline, left online devices num %{public}zu", cid.c_str(), alreadyOnlineDev_.size());
}

void DeviceManagerAgent::OnRemoteDied()
{
    LOGI("device manager service died");
    UnregisterFromExternalDm();
    RegisterToExternalDm();
}

void DeviceManagerAgent::RegisterToExternalDm()
{
    auto &deviceManager = DistributedHardware::DeviceManager::GetInstance();
    int errCode = deviceManager.InitDeviceManager(DistributedFileService::pkgName_, shared_from_this());
    if (errCode != 0) {
        ThrowException(errCode, "Failed to InitDeviceManager");
    }
    std::string extra = "";
    errCode = deviceManager.RegisterDevStateCallback(DistributedFileService::pkgName_, extra, shared_from_this());
    if (errCode != 0) {
        ThrowException(errCode, "Failed to RegisterDevStateCallback");
    }
    LOGI("RegisterToExternalDm Succeed");
}

void DeviceManagerAgent::UnregisterFromExternalDm()
{
    auto &deviceManager = DistributedHardware::DeviceManager::GetInstance();
    int errCode = deviceManager.UnRegisterDevStateCallback(DistributedFileService::pkgName_);
    if (errCode != 0) {
        ThrowException(errCode, "Failed to UnRegisterDevStateCallback");
    }
    errCode = deviceManager.UnInitDeviceManager(DistributedFileService::pkgName_);
    if (errCode != 0) {
        ThrowException(errCode, "Failed to UnInitDeviceManager");
    }
    LOGI("UnregisterFromExternalDm Succeed");
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS