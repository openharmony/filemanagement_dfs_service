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

#include <algorithm>
#include <limits>
#include <memory>
#include <sstream>

#include "utils_log.h"

namespace OHOS::FileManagement::CloudDiskService {
bool HydrateProgress::Marshalling(Parcel &parcel) const
{
    if (state < static_cast<int32_t>(HydrateProgressState::PENDING) ||
        state > static_cast<int32_t>(HydrateProgressState::CANCELLED) ||
        !parcel.WriteString(filePath) || !parcel.WriteInt32(state) ||
        !parcel.WriteUint64(processedSize) || !parcel.WriteUint64(totalSize)) {
        LOGE("Write hydration progress failed");
        return false;
    }
    return true;
}

bool HydrateProgress::ReadFromParcel(Parcel &parcel)
{
    if (!parcel.ReadString(filePath) || !parcel.ReadInt32(state) ||
        !parcel.ReadUint64(processedSize) || !parcel.ReadUint64(totalSize) ||
        state < static_cast<int32_t>(HydrateProgressState::PENDING) ||
        state > static_cast<int32_t>(HydrateProgressState::CANCELLED)) {
        LOGE("Read hydration progress failed");
        return false;
    }
    return true;
}

HydrateProgress *HydrateProgress::Unmarshalling(Parcel &parcel)
{
    auto progress = std::make_unique<HydrateProgress>();
    return progress->ReadFromParcel(parcel) ? progress.release() : nullptr;
}

namespace {
bool IsValidCallbackType(int32_t callbackType)
{
    return callbackType == static_cast<int32_t>(CloudDiskCallbackType::FETCH_DATA) ||
           callbackType == static_cast<int32_t>(CloudDiskCallbackType::CANCEL_FETCH_DATA) ||
           callbackType == static_cast<int32_t>(CloudDiskCallbackType::DEHYDRATE);
}

bool IsValidHydratePriority(int32_t priority)
{
    return priority >= static_cast<int32_t>(CLOUD_DISK_HYDRATE_PRIORITY_LOW) &&
           priority <= static_cast<int32_t>(CLOUD_DISK_HYDRATE_PRIORITY_HIGH);
}

bool ReadExecuteVector(Parcel &parcel, std::vector<uint8_t> &value, uint64_t maxSize)
{
    size_t position = parcel.GetReadPosition();
    int32_t size = 0;
    if (!parcel.ReadInt32(size) || size < 0 || static_cast<uint64_t>(size) > maxSize ||
        !parcel.RewindRead(position) || !parcel.ReadUInt8Vector(&value)) {
        LOGE("Failed to read bounded Execute vector");
        return false;
    }
    return true;
}

bool WritePathInfo(Parcel &parcel, const CloudDiskPathInfo &pathInfo, const char *fieldName)
{
    if (pathInfo.value == nullptr && pathInfo.length != 0) {
        LOGE("Failed to write %{public}s: invalid path info", fieldName);
        return false;
    }
    std::string path(pathInfo.value == nullptr ? "" : std::string(pathInfo.value, pathInfo.length));
    if (!parcel.WriteString(path)) {
        LOGE("Failed to write %{public}s", fieldName);
        return false;
    }
    return true;
}

bool WriteDataBuf(Parcel &parcel, const CloudDiskDataBuf &dataBuf, const char *fieldName)
{
    if (dataBuf.data == nullptr && dataBuf.dataSize != 0) {
        LOGE("Failed to write %{public}s: invalid data buffer", fieldName);
        return false;
    }
    if (dataBuf.dataSize > static_cast<uint64_t>(std::numeric_limits<size_t>::max())) {
        LOGE("Failed to write %{public}s: invalid data size", fieldName);
        return false;
    }
    size_t dataSize = static_cast<size_t>(dataBuf.dataSize);
    std::vector<uint8_t> data(dataSize);
    if (dataBuf.data != nullptr && dataSize != 0) {
        std::copy(dataBuf.data, dataBuf.data + dataSize, data.begin());
    }
    if (!parcel.WriteUInt8Vector(data)) {
        LOGE("Failed to write %{public}s", fieldName);
        return false;
    }
    return true;
}

void AssignPathInfo(CloudDiskPathInfo &pathInfo, std::string &path)
{
    pathInfo.value = path.empty() ? nullptr : path.data();
    pathInfo.length = path.length();
}

void AssignDataBuf(CloudDiskDataBuf &dataBuf, std::vector<uint8_t> &data)
{
    dataBuf.data = data.empty() ? nullptr : data.data();
    dataBuf.dataSize = data.size();
}
} // namespace

bool CallbackExecuteRequest::Marshalling(Parcel &parcel) const
{
    if (reqKey.empty() || reqKey.size() > MAX_CALLBACK_REQUEST_KEY_SIZE || data.size() > MAX_EXECUTE_DATA_SIZE) {
        LOGE("Invalid Execute parcel buffer size");
        return false;
    }
    if (!parcel.WriteUInt8Vector(reqKey) || !parcel.WriteString(syncFolder) || !parcel.WriteString(filePath) ||
        !parcel.WriteInt32(callbackType) || !parcel.WriteUint64(offset) || !parcel.WriteUint64(size) ||
        !parcel.WriteUint64(totalSize) || !parcel.WriteUInt8Vector(data) || !parcel.WriteBool(isComplete)) {
        LOGE("Failed to write Execute request");
        return false;
    }
    return true;
}

CallbackExecuteRequest *CallbackExecuteRequest::Unmarshalling(Parcel &parcel)
{
    auto *request = new (std::nothrow) CallbackExecuteRequest();
    if (request != nullptr && !request->ReadFromParcel(parcel)) {
        delete request;
        request = nullptr;
    }
    return request;
}

bool CallbackExecuteRequest::ReadFromParcel(Parcel &parcel)
{
    if (!ReadExecuteVector(parcel, reqKey, MAX_CALLBACK_REQUEST_KEY_SIZE) || reqKey.empty() ||
        !parcel.ReadString(syncFolder) || !parcel.ReadString(filePath) || !parcel.ReadInt32(callbackType) ||
        !parcel.ReadUint64(offset) || !parcel.ReadUint64(size) || !parcel.ReadUint64(totalSize) ||
        !ReadExecuteVector(parcel, data, MAX_EXECUTE_DATA_SIZE) || !parcel.ReadBool(isComplete)) {
        LOGE("Failed to read Execute request");
        return false;
    }
    return true;
}

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

bool WriteCallbackParcel(Parcel &parcel,
                         const CloudDiskCallbackReqHead &reqHead,
                         const CloudDiskCallbackContext &context)
{
    if (!WritePathInfo(parcel, reqHead.syncFolderPath, "syncFolderPath") ||
        !parcel.WriteInt32(static_cast<int32_t>(reqHead.callbackType)) ||
        !WriteDataBuf(parcel, reqHead.reqKey, "reqKey")) {
        LOGE("Failed to write callback request header");
        return false;
    }

    switch (reqHead.callbackType) {
        case CloudDiskCallbackType::FETCH_DATA:
            return context.fetchData != nullptr && WritePathInfo(parcel, context.fetchData->filePath, "fetchData") &&
                   parcel.WriteInt32(static_cast<int32_t>(context.fetchData->priority));
        case CloudDiskCallbackType::CANCEL_FETCH_DATA:
            return context.cancelFetchData != nullptr &&
                   WritePathInfo(parcel, *context.cancelFetchData, "cancelFetchData");
        case CloudDiskCallbackType::DEHYDRATE:
            return context.dehydrateData != nullptr &&
                   WritePathInfo(parcel, context.dehydrateData->filePath, "dehydrateFilePath");
        default:
            LOGE("Failed to write callback context: invalid callback type");
            return false;
    }
}

static bool ReadCallbackRequestHead(Parcel &parcel, CloudDiskCallbackReqHead &reqHead, CallbackParcelStorage &storage)
{
    if (!parcel.ReadString(storage.syncFolder)) {
        LOGE("Failed to read syncFolderPath");
        return false;
    }
    AssignPathInfo(reqHead.syncFolderPath, storage.syncFolder);

    int32_t callbackType = 0;
    if (!parcel.ReadInt32(callbackType) || !IsValidCallbackType(callbackType)) {
        LOGE("Failed to read callbackType");
        return false;
    }
    reqHead.callbackType = static_cast<CloudDiskCallbackType>(callbackType);
    if (!parcel.ReadUInt8Vector(&storage.reqKey)) {
        LOGE("Failed to read reqKey");
        return false;
    }
    AssignDataBuf(reqHead.reqKey, storage.reqKey);
    if (!parcel.ReadString(storage.filePath)) {
        LOGE("Failed to read filePath");
        return false;
    }
    AssignPathInfo(storage.pathInfo, storage.filePath);
    return true;
}

bool ReadCallbackParcel(Parcel &parcel,
                        CloudDiskCallbackReqHead &reqHead,
                        CloudDiskCallbackContext &context,
                        CallbackParcelStorage &storage)
{
    if (!ReadCallbackRequestHead(parcel, reqHead, storage)) {
        return false;
    }
    if (reqHead.callbackType == CloudDiskCallbackType::FETCH_DATA) {
        int32_t priority = 0;
        if (!parcel.ReadInt32(priority) || !IsValidHydratePriority(priority)) {
            LOGE("Failed to read hydrate priority");
            return false;
        }
        storage.fetchDataRequest.filePath = storage.pathInfo;
        storage.fetchDataRequest.priority = static_cast<CloudDiskHydratePriority>(priority);
        context.fetchData = &storage.fetchDataRequest;
        return true;
    }
    if (reqHead.callbackType == CloudDiskCallbackType::CANCEL_FETCH_DATA) {
        context.cancelFetchData = &storage.pathInfo;
        return true;
    }
    if (reqHead.callbackType == CloudDiskCallbackType::DEHYDRATE) {
        storage.dehydrateInfo.filePath = storage.pathInfo;
        storage.dehydrateInfo.allow = false;
        context.dehydrateData = &storage.dehydrateInfo;
        return true;
    }

    LOGE("Failed to read callback context: invalid callback type");
    return false;
}

bool WriteCallbackReply(Parcel &parcel, CloudDiskCallbackType callbackType, const CloudDiskCallbackContext &context)
{
    if (callbackType == CloudDiskCallbackType::DEHYDRATE) {
        if (context.dehydrateData == nullptr) {
            LOGE("Failed to write dehydrate callback reply: context is nullptr");
            return false;
        }
        return parcel.WriteBool(context.dehydrateData->allow);
    }
    if (callbackType == CloudDiskCallbackType::FETCH_DATA || callbackType == CloudDiskCallbackType::CANCEL_FETCH_DATA) {
        return true;
    }
    LOGE("Failed to write callback reply: invalid callback type");
    return false;
}

bool ReadCallbackReply(Parcel &parcel, CloudDiskCallbackType callbackType, CloudDiskCallbackContext &context)
{
    if (callbackType == CloudDiskCallbackType::DEHYDRATE) {
        if (context.dehydrateData == nullptr) {
            LOGE("Failed to read dehydrate callback reply: context is nullptr");
            return false;
        }
        return parcel.ReadBool(context.dehydrateData->allow);
    }
    if (callbackType == CloudDiskCallbackType::FETCH_DATA || callbackType == CloudDiskCallbackType::CANCEL_FETCH_DATA) {
        return true;
    }
    LOGE("Failed to read callback reply: invalid callback type");
    return false;
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
        if (!changeData.ReadFromParcel(parcel)) {
            LOGE("failed to read changeData");
            continue;
        }
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

bool PlaceholderInfo::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteUint64(logicalSize)) {
        LOGE("failed to write logicalSize");
        return false;
    }
    if (!parcel.WriteUint64(atimeMs)) {
        LOGE("failed to write atimeMs");
        return false;
    }
    if (!parcel.WriteUint64(mtimeMs)) {
        LOGE("failed to write mtimeMs");
        return false;
    }
    return true;
}

PlaceholderInfo *PlaceholderInfo::Unmarshalling(Parcel &parcel)
{
    PlaceholderInfo *info = new (std::nothrow) PlaceholderInfo();
    if ((info != nullptr) && (!info->ReadFromParcel(parcel))) {
        LOGW("read from parcel failed");
        delete info;
        info = nullptr;
    }
    return info;
}

bool PlaceholderInfo::ReadFromParcel(Parcel &parcel)
{
    if (!parcel.ReadUint64(logicalSize)) {
        LOGE("failed to read logicalSize");
        return false;
    }
    if (!parcel.ReadUint64(atimeMs)) {
        LOGE("failed to read atimeMs");
        return false;
    }
    if (!parcel.ReadUint64(mtimeMs)) {
        LOGE("failed to read mtimeMs");
        return false;
    }
    return true;
}

bool PlaceholderCustomInfo::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteUInt8Vector(data)) {
        LOGE("failed to write placeholder custom info");
        return false;
    }
    return true;
}

PlaceholderCustomInfo *PlaceholderCustomInfo::Unmarshalling(Parcel &parcel)
{
    PlaceholderCustomInfo *info = new (std::nothrow) PlaceholderCustomInfo();
    if ((info != nullptr) && (!info->ReadFromParcel(parcel))) {
        LOGW("read placeholder custom info from parcel failed");
        delete info;
        info = nullptr;
    }
    return info;
}

bool PlaceholderCustomInfo::ReadFromParcel(Parcel &parcel)
{
    if (!parcel.ReadUInt8Vector(&data)) {
        LOGE("failed to read placeholder custom info");
        return false;
    }
    return true;
}

} // namespace OHOS::FileManagement::CloudDiskService
