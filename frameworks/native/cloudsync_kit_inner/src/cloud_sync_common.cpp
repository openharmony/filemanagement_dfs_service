/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "cloud_sync_common.h"

#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
namespace {
constexpr uint32_t MAX_MAP_SIZE = 1024;
}
bool SwitchDataObj::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteUint32(switchData.size())) {
        LOGE("failed to write switch data size");
        return false;
    }
    for (const auto& it : switchData) {
        if (!parcel.WriteString(it.first)) {
            LOGE("failed to write key");
            return false;
        }
        if (!parcel.WriteBool(it.second)) {
            LOGE("failed to write value");
            return false;
        }
    }
    return true;
}

SwitchDataObj *SwitchDataObj::Unmarshalling(Parcel &parcel)
{
    SwitchDataObj *info = new (std::nothrow) SwitchDataObj();
    if ((info != nullptr) && (!info->ReadFromParcel(parcel))) {
        LOGW("read from parcel failed");
        delete info;
        info = nullptr;
    }
    return info;
}

bool SwitchDataObj::ReadFromParcel(Parcel &parcel)
{
    switchData.clear();
    uint32_t size = 0;
    if (!parcel.ReadUint32(size)) {
        LOGE("fail to read switch data size");
        return false;
    }
    if (size > MAX_MAP_SIZE) {
        LOGE("switch data is oversize, the limit is %{public}d", MAX_MAP_SIZE);
        return false;
    }
    for (uint32_t i = 0; i < size; ++i) {
        std::string key;
        if (!parcel.ReadString(key)) {
            LOGE("fail to read switch data key");
            return false;
        }
        bool value = false;
        if (!parcel.ReadBool(value)) {
            LOGE("fail to read switch data value");
            return false;
        }
        switchData.emplace(key, value);
    }
    return true;
}
}