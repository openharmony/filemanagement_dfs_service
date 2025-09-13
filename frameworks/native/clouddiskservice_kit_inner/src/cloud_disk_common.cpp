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

#include "cloud_disk_common.h"

#include <sstream>

#include "utils_log.h"

namespace OHOS::FileManagement::CloudDiskService {

bool FileSyncState::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(path)) {
        LOGE("failed to write path");
        return false;
    }

    if (!parcel.WriteInt32(static_cast<int32_t>(state))) {
        LOGE("failed to write state");
        return false;
    }

    return true;
}

FileSyncState *FileSyncState::Unmarshalling(Parcel &parcel)
{
    FileSyncState *info = new (std::nothrow) FileSyncState();
    if ((info != nullptr) && (!info->ReadFromParcel(parcel))) {
        LOGW("read from parcel failed");
        delete info;
        info = nullptr;
    }
    return info;
}

bool FileSyncState::ReadFromParcel(Parcel &parcel)
{
    if (!parcel.ReadString(path)) {
        LOGE("failed to read path");
        return false;
    }

    int32_t readState = 0;
    if (!parcel.ReadInt32(readState)) {
        LOGE("failed to read state");
        return false;
    }
    state = static_cast<SyncState>(readState);

    return true;
}

bool ChangeData::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteUint64(updateSequenceNumber)) {
        LOGE("failed to write updateSequenceNumber");
        return false;
    }

    if (!parcel.WriteString(fileId)) {
        LOGE("failed to write fileId");
        return false;
    }

    if (!parcel.WriteString(parentFileId)) {
        LOGE("failed to write parentFileId");
        return false;
    }

    if (!parcel.WriteString(relativePath)) {
        LOGE("failed to write relativePath");
        return false;
    }

    if (!parcel.WriteUint8(static_cast<uint8_t>(operationType))) {
        LOGE("failed to write operationType");
        return false;
    }

    if (!parcel.WriteUint64(size)) {
        LOGE("failed to write size");
        return false;
    }

    if (!parcel.WriteUint64(mtime)) {
        LOGE("failed to write mtime");
        return false;
    }

    if (!parcel.WriteUint64(timeStamp)) {
        LOGE("failed to write timeStamp");
        return false;
    }

    return true;
}

ChangeData *ChangeData::Unmarshalling(Parcel &parcel)
{
    ChangeData *info = new (std::nothrow) ChangeData();
    if ((info != nullptr) && (!info->ReadFromParcel(parcel))) {
        LOGW("read from parcel failed");
        delete info;
        info = nullptr;
    }
    return info;
}

bool ChangeData::ReadFromParcel(Parcel &parcel)
{
    if (!parcel.ReadUint64(updateSequenceNumber)) {
        LOGE("failed to read updateSequenceNumber");
        return false;
    }

    if (!parcel.ReadString(fileId)) {
        LOGE("failed to read fileId");
        return false;
    }

    if (!parcel.ReadString(parentFileId)) {
        LOGE("failed to read parentFileId");
        return false;
    }

    if (!parcel.ReadString(relativePath)) {
        LOGE("failed to read relativePath");
        return false;
    }

    uint8_t readType = 0;
    if (!parcel.ReadUint8(readType)) {
        LOGE("failed to read operationType");
        return false;
    }
    operationType = static_cast<OperationType>(readType);

    if (!parcel.ReadUint64(size)) {
        LOGE("failed to read size");
        return false;
    }

    if (!parcel.ReadUint64(mtime)) {
        LOGE("failed to read mtime");
        return false;
    }

    if (!parcel.ReadUint64(timeStamp)) {
        LOGE("failed to read timeStamp");
        return false;
    }

    return true;
}

bool ChangesResult::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteUint64(nextUsn)) {
        LOGE("failed to write nextUsn");
        return false;
    }

    if (!parcel.WriteInt32(static_cast<int32_t>(isEof))) {
        LOGE("failed to write isEof");
        return false;
    }

    if (!parcel.WriteInt32(static_cast<int32_t>(changesData.size()))) {
        LOGE("failed to write changeData");
        return false;
    }

    for (auto &item : changesData) {
        if (!item.Marshalling(parcel)) {
            return false;
        }
    }

    return true;
}

ChangesResult *ChangesResult::Unmarshalling(Parcel &parcel)
{
    ChangesResult *info = new (std::nothrow) ChangesResult();
    if ((info != nullptr) && (!info->ReadFromParcel(parcel))) {
        LOGW("read from parcel failed");
        delete info;
        info = nullptr;
    }
    return info;
}

bool ChangesResult::ReadFromParcel(Parcel &parcel)
{
    if (!parcel.ReadUint64(nextUsn)) {
        LOGE("failed to read nextUSN");
        return false;
    }

    int32_t readIsEof = 0;
    if (!parcel.ReadInt32(readIsEof)) {
        LOGE("failed to read isEof");
        return false;
    }
    isEof = readIsEof;

    int32_t size = 0;
    if (!parcel.ReadInt32(size)) {
        LOGE("failed to read changeData size");
        return false;
    }
    for (int32_t i = 0; i < size; ++i) {
        ChangeData changeData;
        changeData.ReadFromParcel(parcel);
        changesData.push_back(changeData);
    }

    return true;
}

bool FailedList::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(path)) {
        LOGE("failed to write path");
        return false;
    }

    if (!parcel.WriteInt32(static_cast<int32_t>(error))) {
        LOGE("failed to write error");
        return false;
    }

    return true;
}

FailedList *FailedList::Unmarshalling(Parcel &parcel)
{
    FailedList *info = new (std::nothrow) FailedList();
    if ((info != nullptr) && (!info->ReadFromParcel(parcel))) {
        LOGW("read from parcel failed");
        delete info;
        info = nullptr;
    }
    return info;
}

bool FailedList::ReadFromParcel(Parcel &parcel)
{
    if (!parcel.ReadString(path)) {
        LOGE("failed to read path");
        return false;
    }

    int32_t readError = 0;
    if (!parcel.ReadInt32(readError)) {
        LOGE("failed to read errno");
        return false;
    }
    error = static_cast<ErrorReason>(readError);

    return true;
}

bool ResultList::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteInt32(static_cast<int32_t>(isSuccess))) {
        LOGE("failed to write isSuccess");
        return false;
    }

    if (!parcel.WriteInt32(static_cast<int32_t>(state))) {
        LOGE("failed to write state");
        return false;
    }

    if (!parcel.WriteString(path)) {
        LOGE("failed to write path");
        return false;
    }

    if (!parcel.WriteInt32(static_cast<int32_t>(error))) {
        LOGE("failed to write error");
        return false;
    }

    return true;
}

ResultList *ResultList::Unmarshalling(Parcel &parcel)
{
    ResultList *info = new (std::nothrow) ResultList();
    if ((info != nullptr) && (!info->ReadFromParcel(parcel))) {
        LOGW("read from parcel failed");
        delete info;
        info = nullptr;
    }
    return info;
}

bool ResultList::ReadFromParcel(Parcel &parcel)
{
    int32_t readIsSuccess = 0;
    if (!parcel.ReadInt32(readIsSuccess)) {
        LOGE("failed to read isSuccess");
        return false;
    }
    isSuccess = readIsSuccess;

    int32_t readState = 0;
    if (!parcel.ReadInt32(readState)) {
        LOGE("failed to read state");
        return false;
    }
    state = static_cast<SyncState>(readState);

    if (!parcel.ReadString(path)) {
        LOGE("failed to read path");
        return false;
    }

    int32_t readError = 0;
    if (!parcel.ReadInt32(readError)) {
        LOGE("failed to read state");
        return false;
    }
    error = static_cast<ErrorReason>(readError);

    return true;
}

} // namespace OHOS::FileManagement::CloudDiskService
