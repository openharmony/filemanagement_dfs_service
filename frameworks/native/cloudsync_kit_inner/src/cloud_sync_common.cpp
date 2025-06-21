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

bool DownloadProgressObj::MarshallingBatch(Parcel &parcel) const
{
    if (!parcel.WriteInt64(batchDownloadSize)) {
        LOGE("failed to write batchDownloadSize");
        return false;
    }
    if (!parcel.WriteInt64(batchTotalSize)) {
        LOGE("failed to write batchTotalSize");
        return false;
    }
    if (!parcel.WriteInt64(batchSuccNum)) {
        LOGE("failed to write batchSuccNum");
        return false;
    }
    if (!parcel.WriteInt64(batchFailNum)) {
        LOGE("failed to write batchFailNum");
        return false;
    }
    if (!parcel.WriteInt64(batchTotalNum)) {
        LOGE("failed to write batchTotalNum");
        return false;
    }
    if (!parcel.WriteInt32(batchState)) {
        LOGE("failed to write batchState");
        return false;
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
    if (!parcel.WriteInt64(downloadId)) {
        LOGE("failed to write downloadId");
        return false;
    }

    if (!MarshallingBatch(parcel)) {
        return false;
    }

    return true;
}

bool DowngradeProgress::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteInt32(state)) {
        LOGE("failed to write download state");
        return false;
    }
    if (!parcel.WriteInt32(stopReason)) {
        LOGE("failed to write stopReason");
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
    if (!parcel.WriteInt32(successfulCount)) {
        LOGE("failed to write successfulCount");
        return false;
    }
    if (!parcel.WriteInt32(failedCount)) {
        LOGE("failed to write failedCount");
        return false;
    }
    if (!parcel.WriteInt32(totalCount)) {
        LOGE("failed to write totalCount");
        return false;
    }

    return true;
}

bool CloudFileInfo::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteInt32(cloudfileCount)) {
        LOGE("failed to write cloudfileCount");
        return false;
    }
    if (!parcel.WriteInt64(cloudFileTotalSize)) {
        LOGE("failed to write cloudFileTotalSize");
        return false;
    }
    if (!parcel.WriteInt32(localFileCount)) {
        LOGE("failed to write localFileCount");
        return false;
    }
    if (!parcel.WriteInt64(localFileTotalSize)) {
        LOGE("failed to write localFileTotalSize");
        return false;
    }
    if (!parcel.WriteInt32(bothFileCount)) {
        LOGE("failed to write bothFileCount");
        return false;
    }
    if (!parcel.WriteInt64(bothFileTotalSize)) {
        LOGE("failed to write bothFileTotalSize");
        return false;
    }

    return true;
}

bool HistoryVersion::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteInt64(editedTime)) {
        LOGE("failed to write editedTime");
        return false;
    }
    if (!parcel.WriteUint64(fileSize)) {
        LOGE("failed to write fileSize");
        return false;
    }
    if (!parcel.WriteUint64(versionId)) {
        LOGE("failed to write versionId");
        return false;
    }
    if (!parcel.WriteString(originalFileName)) {
        LOGE("failed to write originalFileName");
        return false;
    }
    if (!parcel.WriteString(sha256)) {
        LOGE("failed to write sha256");
        return false;
    }
    if (!parcel.WriteBool(resolved)) {
        LOGE("failed to write resolved");
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

DowngradeProgress *DowngradeProgress::Unmarshalling(Parcel &parcel)
{
    DowngradeProgress *info = new (std::nothrow) DowngradeProgress();
    if ((info != nullptr) && (!info->ReadFromParcel(parcel))) {
        LOGW("read from parcel failed");
        delete info;
        info = nullptr;
    }
    return info;
}

CloudFileInfo *CloudFileInfo::Unmarshalling(Parcel &parcel)
{
    CloudFileInfo *info = new (std::nothrow) CloudFileInfo();
    if ((info != nullptr) && (!info->ReadFromParcel(parcel))) {
        LOGW("read from parcel failed");
        delete info;
        info = nullptr;
    }
    return info;
}

HistoryVersion *HistoryVersion::Unmarshalling(Parcel &parcel)
{
    HistoryVersion *info = new (std::nothrow) HistoryVersion();
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

OptimizeSpaceOptions *OptimizeSpaceOptions::Unmarshalling(Parcel &parcel)
{
    OptimizeSpaceOptions *info = new (std::nothrow) OptimizeSpaceOptions();
    if ((info != nullptr) && (!info->ReadFromParcel(parcel))) {
        LOGW("read from parcel failed");
        delete info;
        info = nullptr;
    }
    return info;
}

bool OptimizeSpaceOptions::ReadFromParcel(Parcel &parcel)
{
    if (!parcel.ReadInt64(totalSize)) {
        LOGE("failed to read totalSize");
        return false;
    }
    if (!parcel.ReadInt32(agingDays)) {
        LOGE("failed to read agingDays");
        return false;
    }
    return true;
}

bool OptimizeSpaceOptions::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteInt64(totalSize)) {
        LOGE("failed to write totalSize");
        return false;
    }
    if (!parcel.WriteInt32(agingDays)) {
        LOGE("failed to write agingDays");
        return false;
    }

    return true;
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

bool DownloadProgressObj::ReadBatchFromParcel(Parcel &parcel)
{
    if (!parcel.ReadInt64(batchDownloadSize)) {
        LOGE("failed to read batchDownloadSize");
        return false;
    }
    if (!parcel.ReadInt64(batchTotalSize)) {
        LOGE("failed to read batchTotalSize");
        return false;
    }
    if (!parcel.ReadInt64(batchSuccNum)) {
        LOGE("failed to read batchSuccNum");
        return false;
    }
    if (!parcel.ReadInt64(batchFailNum)) {
        LOGE("failed to read batchFailNum");
        return false;
    }
    if (!parcel.ReadInt64(batchTotalNum)) {
        LOGE("failed to read batchTotalNum");
        return false;
    }
    int32_t tempBatchState = 0;
    if (!parcel.ReadInt32(tempBatchState)) {
        LOGE("failed to read download batchState");
        return false;
    }
    batchState = static_cast<Status>(tempBatchState);

    return true;
}

bool DownloadProgressObj::ReadFromParcel(Parcel &parcel)
{
    if (!parcel.ReadString(path)) {
        LOGE("failed to read download path");
        return false;
    }
    int32_t tempState = 0;
    if (!parcel.ReadInt32(tempState)) {
        LOGE("failed to read download state");
        return false;
    }
    state = static_cast<Status>(tempState);
    if (!parcel.ReadInt64(downloadedSize)) {
        LOGE("failed to read downloadedSize");
        return false;
    }
    if (!parcel.ReadInt64(totalSize)) {
        LOGE("failed to read totalSize");
        return false;
    }
    if (!parcel.ReadInt32(downloadErrorType)) {
        LOGE("failed to read downloadErrorType");
        return false;
    }
    if (!parcel.ReadInt64(downloadId)) {
        LOGE("failed to read downloadId");
        return false;
    }

    if (!ReadBatchFromParcel(parcel)) {
        return false;
    }

    return true;
}

bool DowngradeProgress::ReadFromParcel(Parcel &parcel)
{
    int32_t temp = 0;
    if (!parcel.ReadInt32(temp)) {
        LOGE("failed to read download state");
        return false;
    }
    state = static_cast<State>(temp);
    if (!parcel.ReadInt32(temp)) {
        LOGE("failed to read stopReason");
        return false;
    }
    stopReason = static_cast<StopReason>(temp);
    if (!parcel.ReadInt64(downloadedSize)) {
        LOGE("failed to read downloadedSize");
        return false;
    }
    if (!parcel.ReadInt64(totalSize)) {
        LOGE("failed to read totalSize");
        return false;
    }
    if (!parcel.ReadInt32(successfulCount)) {
        LOGE("failed to read successfulCount");
        return false;
    }
    if (!parcel.ReadInt32(failedCount)) {
        LOGE("failed to read failedCount");
        return false;
    }
    if (!parcel.ReadInt32(totalCount)) {
        LOGE("failed to read totalCount");
        return false;
    }

    return true;
}

bool CloudFileInfo::ReadFromParcel(Parcel &parcel)
{
    if (!parcel.ReadInt32(cloudfileCount)) {
        LOGE("failed to read cloudfileCount");
        return false;
    }
    if (!parcel.ReadInt64(cloudFileTotalSize)) {
        LOGE("failed to read cloudFileTotalSize");
        return false;
    }
    if (!parcel.ReadInt32(localFileCount)) {
        LOGE("failed to read localFileCount");
        return false;
    }
    if (!parcel.ReadInt64(localFileTotalSize)) {
        LOGE("failed to read localFileTotalSize");
        return false;
    }
    if (!parcel.ReadInt32(bothFileCount)) {
        LOGE("failed to read bothFileCount");
        return false;
    }
    if (!parcel.ReadInt64(bothFileTotalSize)) {
        LOGE("failed to read bothFileTotalSize");
        return false;
    }

    return true;
}

bool HistoryVersion::ReadFromParcel(Parcel &parcel)
{
    if (!parcel.ReadInt64(editedTime)) {
        LOGE("failed to read editedTime");
        return false;
    }
    if (!parcel.ReadUint64(fileSize)) {
        LOGE("failed to read fileSize");
        return false;
    }
    if (!parcel.ReadUint64(versionId)) {
        LOGE("failed to read versionId");
        return false;
    }
    if (!parcel.ReadString(originalFileName)) {
        LOGE("failed to read originalFileName");
        return false;
    }
    if (!parcel.ReadString(sha256)) {
        LOGE("failed to read sha256");
        return false;
    }
    if (!parcel.ReadBool(resolved)) {
        LOGE("failed to read resolved");
        return false;
    }

    return true;
}

std::string DownloadProgressObj::to_string()
{
    std::stringstream ss;
    std::string pathAnony = GetAnonyString(path);
    ss << "DownloadProgressObj [path: " << pathAnony;
    ss << " state: " << state;
    ss << " downloaded: " << downloadedSize;
    ss << " total: " << totalSize;
    ss << " downloadErrorType: " << downloadErrorType;

    ss << " downloadId: " << downloadId;
    ss << " batchState: " << batchState;
    ss << " batchDownloadSize: " << batchDownloadSize;
    ss << " batchTotalSize: " << batchTotalSize;
    ss << " batchSuccNum: " << batchSuccNum;
    ss << " batchFailNum: " << batchFailNum;
    ss << " batchTotalNum: " << batchTotalNum << "]";
    return ss.str();
}

std::string DowngradeProgress::to_string() const
{
    std::stringstream ss;
    ss << "DowngradeProgress [DownloadState: " << state;
    ss << " downloadStopReason: " << stopReason;
    ss << " downloadedSize: " << downloadedSize;
    ss << " totalSize: " << totalSize;
    ss << " successfulCount: " << successfulCount;
    ss << " failedCount: " << failedCount;
    ss << " totalCount: " << totalCount << "]";
    return ss.str();
}

bool DentryFileInfoObj::ReadFromParcel(Parcel &parcel)
{
    parcel.ReadString(cloudId);
    parcel.ReadInt64(size);
    parcel.ReadInt64(modifiedTime);
    parcel.ReadString(path);
    parcel.ReadString(fileName);
    parcel.ReadString(fileType);
    return true;
}

bool DentryFileInfoObj::Marshalling(Parcel &parcel) const
{
    parcel.WriteString(cloudId);
    parcel.WriteInt64(size);
    parcel.WriteInt64(modifiedTime);
    parcel.WriteString(path);
    parcel.WriteString(fileName);
    parcel.WriteString(fileType);
    return true;
}

DentryFileInfoObj *DentryFileInfoObj::Unmarshalling(Parcel &parcel)
{
    DentryFileInfoObj *info = new (std::nothrow) DentryFileInfoObj();
    if ((info != nullptr) && (!info->ReadFromParcel(parcel))) {
        LOGW("read from parcel failed");
        delete info;
        info = nullptr;
    }
    return info;
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

bool CleanFileInfoObj::ReadFromParcel(Parcel &parcel)
{
    parcel.ReadString(cloudId);
    parcel.ReadInt64(size);
    parcel.ReadInt64(modifiedTime);
    parcel.ReadString(path);
    parcel.ReadString(fileName);
    parcel.ReadStringVector(&attachment);
    return true;
}

bool CleanFileInfoObj::Marshalling(Parcel &parcel) const
{
    parcel.WriteString(cloudId);
    parcel.WriteInt64(size);
    parcel.WriteInt64(modifiedTime);
    parcel.WriteString(path);
    parcel.WriteString(fileName);
    parcel.WriteStringVector(attachment);
    return true;
}

CleanFileInfoObj *CleanFileInfoObj::Unmarshalling(Parcel &parcel)
{
    CleanFileInfoObj *info = new (std::nothrow) CleanFileInfoObj();
    if ((info != nullptr) && (!info->ReadFromParcel(parcel))) {
        LOGW("read from parcel failed");
        delete info;
        info = nullptr;
    }
    return info;
}

} // namespace OHOS::FileManagement::CloudSync
