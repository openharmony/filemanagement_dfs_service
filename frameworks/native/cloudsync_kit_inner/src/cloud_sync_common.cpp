/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include <sstream>
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

bool DownloadProgressObj::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(path)) {
        LOGE("failed to write download path");
        return false;
    }
    if (!parcel.WriteInt32(state)) {
        LOGE("failed to write download state");
        return false;
    }
    if (!parcel.WriteInt64(downloadedSize)) {
        LOGE("failed to write downloadedSize");
        return false;
    }
    if (!parcel.WriteInt64(totalSize)) {
        LOGE("failed to write totalSize");
        return false;
    }
    if (!parcel.WriteInt32(downloadErrorType)) {
        LOGE("failed to write downloadErrorType");
        return false;
    }
    return true;
}

bool CleanOptions::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteUint32(appActionsData.size())) {
        LOGE("failed to write appActions data size");
        return false;
    }
    for (const auto& it : appActionsData) {
        if (!parcel.WriteString(it.first)) {
            LOGE("failed to write key");
            return false;
        }
        if (!parcel.WriteInt32(it.second)) {
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

DownloadProgressObj *DownloadProgressObj::Unmarshalling(Parcel &parcel)
{
    DownloadProgressObj *info = new (std::nothrow) DownloadProgressObj();
    if ((info != nullptr) && (!info->ReadFromParcel(parcel))) {
        LOGW("read from parcel failed");
        delete info;
        info = nullptr;
    }
    return info;
}

CleanOptions *CleanOptions::Unmarshalling(Parcel &parcel)
{
    CleanOptions *info = new (std::nothrow) CleanOptions();
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

bool CleanOptions::ReadFromParcel(Parcel &parcel)
{
    appActionsData.clear();
    uint32_t size = 0;
    if (!parcel.ReadUint32(size)) {
        LOGE("fail to read appActions data size");
        return false;
    }
    if (size > MAX_MAP_SIZE) {
        LOGE("appActions data is oversize, the limit is %{public}d", MAX_MAP_SIZE);
        return false;
    }
    for (uint32_t i = 0; i < size; ++i) {
        std::string key;
        if (!parcel.ReadString(key)) {
            LOGE("fail to read appActions data key");
            return false;
        }
        int value = 0;
        if (!parcel.ReadInt32(value)) {
            LOGE("fail to read appActions data value");
            return false;
        }
        appActionsData.emplace(key, value);
    }
    return true;
}

bool DownloadProgressObj::ReadFromParcel(Parcel &parcel)
{
    if (!parcel.ReadString(path)) {
        LOGE("failed to write download path");
        return false;
    }
    int32_t tempState = 0;
    if (!parcel.ReadInt32(tempState)) {
        LOGE("failed to write download state");
        return false;
    }
    state = static_cast<Status>(tempState);
    if (!parcel.ReadInt64(downloadedSize)) {
        LOGE("failed to write downloadedSize");
        return false;
    }
    if (!parcel.ReadInt64(totalSize)) {
        LOGE("failed to write totalSize");
        return false;
    }
    if (!parcel.ReadInt32(downloadErrorType)) {
        LOGE("failed to write downloadErrorType");
        return false;
    }
    return true;
}

std::string DownloadProgressObj::to_string()
{
    std::stringstream ss;
    ss << "DownloadProgressObj [path: " << path;
    ss << " state: " << state;
    ss << " downloaded: " << downloadedSize;
    ss << " total: " << totalSize;
    ss << " downloadErrorType: " << downloadErrorType << "]";

    return ss.str();
}

bool AssetInfoObj::ReadFromParcel(Parcel &parcel)
{
    parcel.ReadString(uri);
    parcel.ReadString(recordType);
    parcel.ReadString(recordId);
    parcel.ReadString(fieldKey);
    parcel.ReadString(assetName);
    return true;
}

bool AssetInfoObj::Marshalling(Parcel &parcel) const
{
    parcel.WriteString(uri);
    parcel.WriteString(recordType);
    parcel.WriteString(recordId);
    parcel.WriteString(fieldKey);
    parcel.WriteString(assetName);
    return true;
}

AssetInfoObj *AssetInfoObj::Unmarshalling(Parcel &parcel)
{
    AssetInfoObj *info = new (std::nothrow) AssetInfoObj();
    if ((info != nullptr) && (!info->ReadFromParcel(parcel))) {
        LOGW("read from parcel failed");
        delete info;
        info = nullptr;
    }
    return info;
}
} // namespace OHOS::FileManagement::CloudSync
