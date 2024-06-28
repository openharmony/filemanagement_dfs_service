/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#ifndef OHOS_DFS_SERVICE_DEVICE_MANAGER_MOCK_H
#define OHOS_DFS_SERVICE_DEVICE_MANAGER_MOCK_H

#include <memory>
#include <string>
#include <gmock/gmock.h>

#include "device_manager.h"
#include "softbus_bus_center.h"

namespace OHOS::Storage::DistributedFile {
using namespace OHOS::DistributedHardware;
class DfsDeviceManagerImpl {
public:
    virtual ~DfsDeviceManagerImpl() = default;
public:
    virtual int32_t InitDeviceManager(const std::string &pkgName,
        std::shared_ptr<DmInitCallback> dmInitCallback) = 0;
    virtual int32_t UnInitDeviceManager(const std::string &pkgName) = 0;
    virtual int32_t GetTrustedDeviceList(const std::string &pkgName,
        const std::string &extra, std::vector<DmDeviceInfo> &deviceList) = 0;
    virtual int32_t RegisterDevStateCallback(const std::string &pkgName,
        const std::string &extra, std::shared_ptr<DeviceStateCallback> callback) = 0;
    virtual int32_t UnRegisterDevStateCallback(const std::string &pkgName) = 0;
    virtual int32_t GetLocalDeviceInfo(const std::string &pkgName, DmDeviceInfo &info) = 0;
public:
    virtual int32_t GetLocalNodeDeviceInfo(const char *pkgName, NodeBasicInfo *info) = 0;
public:
    static inline std::shared_ptr<DfsDeviceManagerImpl> dfsDeviceManagerImpl = nullptr;
};

class DeviceManagerImplMock : public DfsDeviceManagerImpl {
public:
    MOCK_METHOD2(InitDeviceManager, int32_t(const std::string &pkgName,
        std::shared_ptr<DmInitCallback> dmInitCallback));
    MOCK_METHOD1(UnInitDeviceManager, int32_t(const std::string &pkgName));
    MOCK_METHOD3(GetTrustedDeviceList, int32_t(const std::string &pkgName,
        const std::string &extra, std::vector<DmDeviceInfo> &deviceList));
    MOCK_METHOD3(RegisterDevStateCallback, int32_t(const std::string &pkgName,
        const std::string &extra, std::shared_ptr<DeviceStateCallback> callback));
    MOCK_METHOD1(UnRegisterDevStateCallback, int32_t(const std::string &pkgName));
    MOCK_METHOD2(GetLocalNodeDeviceInfo, int32_t(const char *pkgName, NodeBasicInfo *info));
    MOCK_METHOD2(GetLocalDeviceInfo, int32_t(const std::string &pkgName, DmDeviceInfo &info));
};
}
#endif