/*
* Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef FILEMANAGEMENT_DFS_SERVICE_DEVICE_MANAGER_AGENT_MOCK_H
#define FILEMANAGEMENT_DFS_SERVICE_DEVICE_MANAGER_AGENT_MOCK_H

#include <gmock/gmock.h>
#include "device/device_manager_agent.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
class IDeviceManagerAgentMock {
public:
    virtual ~IDeviceManagerAgentMock() = default;
    virtual int32_t IsSupportedDevice(const DistributedHardware::DmDeviceInfo &deviceInfo) = 0;
    virtual int32_t OnDeviceP2POnline(const DistributedHardware::DmDeviceInfo &deviceInfo) = 0;
    virtual int32_t OnDeviceP2POffline(const DistributedHardware::DmDeviceInfo &deviceInfo) = 0;
    virtual int32_t AddRemoteReverseObj(uint32_t callingTokenId, sptr<IFileDfsListener> remoteReverseObj) = 0;
    virtual int32_t RemoveRemoteReverseObj(bool clear, uint32_t callingTokenId) = 0;
    virtual int32_t FindListenerByObject(const wptr<IRemoteObject> &remote, uint32_t &tokenId,
                                         sptr<IFileDfsListener>& listener) = 0;
    virtual std::string GetDeviceIdByNetworkId(const std::string &networkId) = 0;
    virtual int32_t MountDfsDocs(const std::string &networkId, const std::string &deviceId) = 0;
    virtual int32_t UMountDfsDocs(const std::string &networkId, const std::string &deviceId, bool needClear) = 0;
    virtual std::unordered_set<std::string> GetNetworkIds(uint32_t tokenId) = 0;
    virtual DeviceInfo &GetLocalDeviceInfo() = 0;
    virtual std::vector<DeviceInfo> GetRemoteDevicesInfo() = 0;
    virtual sptr<IFileDfsListener> GetDfsListener() = 0;

public:
    static inline std::shared_ptr<IDeviceManagerAgentMock> iDeviceManagerAgentMock_ = nullptr;
};

class DeviceManagerAgentMock : public IDeviceManagerAgentMock {
public:
    MOCK_METHOD1(IsSupportedDevice, int32_t(const DistributedHardware::DmDeviceInfo &deviceInfo));
    MOCK_METHOD1(OnDeviceP2POnline, int32_t(const DistributedHardware::DmDeviceInfo &deviceInfo));
    MOCK_METHOD1(OnDeviceP2POffline, int32_t(const DistributedHardware::DmDeviceInfo &deviceInfo));
    MOCK_METHOD2(AddRemoteReverseObj, int32_t(uint32_t callingTokenId, sptr<IFileDfsListener> remoteReverseObj));
    MOCK_METHOD2(RemoveRemoteReverseObj, int32_t(bool clear, uint32_t callingTokenId));
    MOCK_METHOD3(FindListenerByObject, int32_t(const wptr<IRemoteObject> &remote, uint32_t &tokenId,
                                               sptr<IFileDfsListener>& listener));
    MOCK_METHOD1(GetDeviceIdByNetworkId, std::string(const std::string &networkId));
    MOCK_METHOD2(MountDfsDocs, int32_t(const std::string &networkId, const std::string &deviceId));
    MOCK_METHOD3(UMountDfsDocs, int32_t(const std::string &networkId, const std::string &deviceId, bool needClear));
    MOCK_METHOD1(GetNetworkIds, std::unordered_set<std::string>(uint32_t tokenId));
    MOCK_METHOD0(GetLocalDeviceInfo, DeviceInfo &());
    MOCK_METHOD0(GetRemoteDevicesInfo, std::vector<DeviceInfo>());
    MOCK_METHOD0(GetDfsListener, sptr<IFileDfsListener>());
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // FILEMANAGEMENT_DFS_SERVICE_DEVICE_MANAGER_AGENT_MOCK_H
