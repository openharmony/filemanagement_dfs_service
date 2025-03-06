/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef DM_DEVICE_INFO_EXT_H
#define DM_DEVICE_INFO_EXT_H
#include <cstdint>
#include <map>
#include <string>

#include "dm_device_info.h"
#include "parcel.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {

using namespace OHOS::DistributedHardware;

struct DmDeviceInfoExt : public Parcelable, public DmDeviceInfo {
    using DmDeviceInfo::DmDeviceInfo;

    DmDeviceInfoExt() {}

    DmDeviceInfoExt(const DmDeviceInfo &devInfo)
    {
        strncpy_s(deviceId, DM_MAX_DEVICE_ID_LEN - 1, devInfo.deviceId, DM_MAX_DEVICE_ID_LEN - 1);
        deviceId[DM_MAX_DEVICE_ID_LEN - 1] = '\0';

        strncpy_s(deviceName, DM_MAX_DEVICE_NAME_LEN - 1, devInfo.deviceName, DM_MAX_DEVICE_NAME_LEN - 1);
        deviceName[DM_MAX_DEVICE_NAME_LEN - 1] = '\0';

        strncpy_s(networkId, DM_MAX_DEVICE_ID_LEN - 1, devInfo.networkId, DM_MAX_DEVICE_ID_LEN - 1);
        networkId[DM_MAX_DEVICE_ID_LEN - 1] = '\0';

        deviceTypeId = devInfo.deviceTypeId;
        range = devInfo.range;
        networkType = devInfo.networkType;
        authForm = static_cast<DmAuthForm>(devInfo.authForm);
        extraData = devInfo.extraData;
    }

    DmDeviceInfo ConvertToDmDeviceInfo()
    {
        DmDeviceInfo dmDeviceInfo;

        strncpy_s(dmDeviceInfo.deviceId, DM_MAX_DEVICE_ID_LEN - 1, deviceId, DM_MAX_DEVICE_ID_LEN - 1);
        dmDeviceInfo.deviceId[DM_MAX_DEVICE_ID_LEN - 1] = '\0';

        strncpy_s(dmDeviceInfo.deviceName, DM_MAX_DEVICE_NAME_LEN - 1, deviceName, DM_MAX_DEVICE_NAME_LEN - 1);
        dmDeviceInfo.deviceName[DM_MAX_DEVICE_NAME_LEN - 1] = '\0';

        strncpy_s(dmDeviceInfo.networkId, DM_MAX_DEVICE_ID_LEN - 1, networkId, DM_MAX_DEVICE_ID_LEN - 1);
        dmDeviceInfo.networkId[DM_MAX_DEVICE_ID_LEN - 1] = '\0';

        dmDeviceInfo.deviceTypeId = deviceTypeId;
        dmDeviceInfo.range = range;
        dmDeviceInfo.networkType = networkType;
        dmDeviceInfo.authForm = static_cast<DmAuthForm>(authForm);
        dmDeviceInfo.extraData = extraData;

        return dmDeviceInfo;
    }

    virtual bool Marshalling(Parcel &parcel) const override;

    static DmDeviceInfoExt *Unmarshalling(Parcel &parcel);
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // DM_DEVICE_INFO_EXT_H