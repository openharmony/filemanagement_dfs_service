/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include <cstdint>
#include <map>
#include <string>

#include "dm_device_info_ext.h"
#include "utils_log.h"
#include "dfs_error.h"

#define DM_MAX_DEVICE_ID_LEN (97)
#define DM_MAX_DEVICE_NAME_LEN (129)

namespace OHOS {
namespace Storage {
namespace DistributedFile {

using namespace OHOS::DistributedHardware;

bool DmDeviceInfoExt::Marshalling(Parcel &parcel) const
{
    bool bRet = true;
    std::string deviceIdStr(deviceId);
    bRet = (bRet && parcel.WriteString(deviceIdStr));
    std::string deviceNameStr(deviceName);
    bRet = (bRet && parcel.WriteString(deviceNameStr));
    bRet = (bRet && parcel.WriteUint16(deviceTypeId));
    std::string networkIdStr(networkId);
    bRet = (bRet && parcel.WriteString(networkIdStr));
    bRet = (bRet && parcel.WriteInt32(range));
    bRet = (bRet && parcel.WriteInt32(networkType));
    bRet = (bRet && parcel.WriteInt32(authForm));
    bRet = (bRet && parcel.WriteString(extraData));
    return bRet;
}

DmDeviceInfoExt *DmDeviceInfoExt::Unmarshalling(Parcel &parcel)
{
    DmDeviceInfoExt *deviceInfoExt = new (std::nothrow) DmDeviceInfoExt();
    if (deviceInfoExt == nullptr) {
        LOGE("Create DmDeviceInfoExt failed");
        return nullptr;
    }
    std::string deviceIdStr = parcel.ReadString();
    if (strcpy_s(deviceInfoExt->deviceId, deviceIdStr.size() + 1, deviceIdStr.c_str()) != OHOS::FileManagement::E_OK) {
        LOGE("strcpy_s deviceId failed!");
        delete deviceInfoExt;
        return nullptr;
    }
    std::string deviceNameStr = parcel.ReadString();
    if (strcpy_s(deviceInfoExt->deviceName, deviceNameStr.size() + 1, deviceNameStr.c_str()) !=
        OHOS::FileManagement::E_OK) {
        LOGE("strcpy_s deviceName failed!");
        delete deviceInfoExt;
        return nullptr;
    }
    deviceInfoExt->deviceTypeId = parcel.ReadUint16();
    std::string networkIdStr = parcel.ReadString();
    if (strcpy_s(deviceInfoExt->networkId, networkIdStr.size() + 1, networkIdStr.c_str()) !=
        OHOS::FileManagement::E_OK) {
        LOGE("strcpy_s networkId failed!");
        delete deviceInfoExt;
        return nullptr;
    }
    deviceInfoExt->range = parcel.ReadInt32();
    deviceInfoExt->networkType = parcel.ReadInt32();
    deviceInfoExt->authForm = static_cast<DmAuthForm>(parcel.ReadInt32());
    deviceInfoExt->extraData = parcel.ReadString();
    return deviceInfoExt;
}

} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS