/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "device_manager_impl.h"

#include "gtest/gtest.h"

namespace OHOS {
namespace DistributedHardware {
DeviceManagerImpl &DeviceManagerImpl::GetInstance()
{
    GTEST_LOG_(INFO) << "GetInstance start";
    static DeviceManagerImpl instance;
    return instance;
}

int32_t DeviceManagerImpl::InitDeviceManager(const std::string &pkgName, std::shared_ptr<DmInitCallback> dmInitCallback)
{
    return 0;
}

int32_t DeviceManagerImpl::UnInitDeviceManager(const std::string &pkgName)
{
    return 0;
}

int32_t DeviceManagerImpl::GetTrustedDeviceList(const std::string &pkgName,
                                                const std::string &extra,
                                                std::vector<DmDeviceInfo> &deviceList)
{
    GTEST_LOG_(INFO) << "GetTrustedDeviceList start";
    DmDeviceInfo deviceInfo{};
    deviceList.push_back(deviceInfo);
    return 0;
}

int32_t DeviceManagerImpl::GetTrustedDeviceList(const std::string &pkgName,
                                                const std::string &extra,
                                                bool isRefresh,
                                                std::vector<DmDeviceInfo> &deviceList)
{
    return 0;
}

int32_t DeviceManagerImpl::GetAvailableDeviceList(const std::string &pkgName,
                                                  std::vector<DmDeviceBasicInfo> &deviceList)
{
    return 0;
}

int32_t DeviceManagerImpl::GetDeviceInfo(const std::string &pkgName,
                                         const std::string networkId,
                                         DmDeviceInfo &deviceInfo)
{
    return 0;
}

int32_t DeviceManagerImpl::GetLocalDeviceInfo(const std::string &pkgName, DmDeviceInfo &info)
{
    return 0;
}

int32_t DeviceManagerImpl::RegisterDevStateCallback(const std::string &pkgName,
                                                    const std::string &extra,
                                                    std::shared_ptr<DeviceStateCallback> callback)
{
    return 0;
}

int32_t DeviceManagerImpl::RegisterDevStatusCallback(const std::string &pkgName,
                                                     const std::string &extra,
                                                     std::shared_ptr<DeviceStatusCallback> callback)
{
    return 0;
}

int32_t DeviceManagerImpl::UnRegisterDevStateCallback(const std::string &pkgName)
{
    return 0;
}

int32_t DeviceManagerImpl::UnRegisterDevStatusCallback(const std::string &pkgName)
{
    return 0;
}

int32_t DeviceManagerImpl::StartDeviceDiscovery(const std::string &pkgName,
                                                const DmSubscribeInfo &subscribeInfo,
                                                const std::string &extra,
                                                std::shared_ptr<DiscoveryCallback> callback)
{
    return 0;
}

int32_t DeviceManagerImpl::StartDeviceDiscovery(const std::string &pkgName,
                                                uint64_t tokenId,
                                                const std::string &filterOptions,
                                                std::shared_ptr<DiscoveryCallback> callback)
{
    return 0;
}

int32_t DeviceManagerImpl::StopDeviceDiscovery(const std::string &pkgName, uint16_t subscribeId)
{
    return 0;
}

int32_t DeviceManagerImpl::StopDeviceDiscovery(uint64_t tokenId, const std::string &pkgName)
{
    return 0;
}

int32_t DeviceManagerImpl::PublishDeviceDiscovery(const std::string &pkgName,
                                                  const DmPublishInfo &publishInfo,
                                                  std::shared_ptr<PublishCallback> callback)
{
    return 0;
}

int32_t DeviceManagerImpl::UnPublishDeviceDiscovery(const std::string &pkgName, int32_t publishId)
{
    return 0;
}

int32_t DeviceManagerImpl::AuthenticateDevice(const std::string &pkgName,
                                              int32_t authType,
                                              const DmDeviceInfo &deviceInfo,
                                              const std::string &extra,
                                              std::shared_ptr<AuthenticateCallback> callback)
{
    return 0;
}

int32_t DeviceManagerImpl::UnAuthenticateDevice(const std::string &pkgName, const DmDeviceInfo &deviceInfo)
{
    return 0;
}

int32_t DeviceManagerImpl::RegisterDeviceManagerFaCallback(const std::string &pkgName,
                                                           std::shared_ptr<DeviceManagerUiCallback> callback)
{
    return 0;
}

int32_t DeviceManagerImpl::UnRegisterDeviceManagerFaCallback(const std::string &pkgName)
{
    return 0;
}

int32_t DeviceManagerImpl::VerifyAuthentication(const std::string &pkgName,
                                                const std::string &authPara,
                                                std::shared_ptr<VerifyAuthCallback> callback)
{
    return 0;
}

int32_t DeviceManagerImpl::GetFaParam(const std::string &pkgName, DmAuthParam &dmFaParam)
{
    return 0;
}

int32_t DeviceManagerImpl::SetUserOperation(const std::string &pkgName, int32_t action, const std::string &params)
{
    return 0;
}

int32_t DeviceManagerImpl::GetUdidByNetworkId(const std::string &pkgName,
                                              const std::string &netWorkId,
                                              std::string &udid)
{
    return 0;
}

int32_t DeviceManagerImpl::GetUuidByNetworkId(const std::string &pkgName,
                                              const std::string &netWorkId,
                                              std::string &uuid)
{
    return 0;
}

int32_t DeviceManagerImpl::RegisterDevStateCallback(const std::string &pkgName, const std::string &extra)
{
    return 0;
}

int32_t DeviceManagerImpl::UnRegisterDevStateCallback(const std::string &pkgName, const std::string &extra)
{
    return 0;
}

int32_t DeviceManagerImpl::RequestCredential(const std::string &pkgName,
                                             const std::string &reqJsonStr,
                                             std::string &returnJsonStr)
{
    return 0;
}

int32_t DeviceManagerImpl::ImportCredential(const std::string &pkgName, const std::string &credentialInfo)
{
    return 0;
}

int32_t DeviceManagerImpl::DeleteCredential(const std::string &pkgName, const std::string &deleteInfo)
{
    return 0;
}

int32_t DeviceManagerImpl::RegisterCredentialCallback(const std::string &pkgName,
                                                      std::shared_ptr<CredentialCallback> callback)
{
    return 0;
}

int32_t DeviceManagerImpl::UnRegisterCredentialCallback(const std::string &pkgName)
{
    return 0;
}

int32_t DeviceManagerImpl::NotifyEvent(const std::string &pkgName, const int32_t eventId, const std::string &event)
{
    return 0;
}

int32_t DeviceManagerImpl::OnDmServiceDied()
{
    return 0;
}

int32_t DeviceManagerImpl::GetEncryptedUuidByNetworkId(const std::string &pkgName,
                                                       const std::string &networkId,
                                                       std::string &uuid)
{
    return 0;
}

int32_t DeviceManagerImpl::GenerateEncryptedUuid(const std::string &pkgName,
                                                 const std::string &uuid,
                                                 const std::string &appId,
                                                 std::string &encryptedUuid)
{
    return 0;
}

int32_t DeviceManagerImpl::CheckAPIAccessPermission()
{
    return 0;
}

int32_t DeviceManagerImpl::CheckNewAPIAccessPermission()
{
    return 0;
}

int32_t DeviceManagerImpl::GetLocalDeviceNetWorkId(const std::string &pkgName, std::string &networkId)
{
    return 0;
}

int32_t DeviceManagerImpl::GetLocalDeviceId(const std::string &pkgName, std::string &deviceId)
{
    return 0;
}

int32_t DeviceManagerImpl::GetLocalDeviceName(const std::string &pkgName, std::string &deviceName)
{
    return 0;
}

int32_t DeviceManagerImpl::GetLocalDeviceType(const std::string &pkgName, int32_t &deviceType)
{
    return 0;
}

int32_t DeviceManagerImpl::GetDeviceName(const std::string &pkgName,
                                         const std::string &networkId,
                                         std::string &deviceName)
{
    return 0;
}

int32_t DeviceManagerImpl::GetDeviceType(const std::string &pkgName, const std::string &networkId, int32_t &deviceType)
{
    return 0;
}

int32_t DeviceManagerImpl::BindDevice(const std::string &pkgName,
                                      int32_t bindType,
                                      const std::string &deviceId,
                                      const std::string &bindParam,
                                      std::shared_ptr<AuthenticateCallback> callback)
{
    return 0;
}

int32_t DeviceManagerImpl::UnBindDevice(const std::string &pkgName, const std::string &deviceId)
{
    return 0;
}

int32_t DeviceManagerImpl::GetNetworkTypeByNetworkId(const std::string &pkgName,
                                                     const std::string &netWorkId,
                                                     int32_t &netWorkType)
{
    GTEST_LOG_(INFO) << "GetTrustedDeviceList start";
    netWorkType = 0;
    return 0;
}
} // namespace DistributedHardware
} // namespace OHOS
