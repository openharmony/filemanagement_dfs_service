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

#include "copy/ipc_wrapper.h"

#include <sys/stat.h>
#include <filesystem>

#include "dfs_error.h"
#include "securec.h"
#include "utils_log.h"

#undef LOG_DOMAIN
#undef LOG_TAG
#define LOG_DOMAIN 0xD00430B
#define LOG_TAG "distributedfile_daemon"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
constexpr size_t MAX_IPC_RAW_DATA_SIZE = 128 * 1024 * 1024;

bool IpcWrapper::WriteUriByRawData(MessageParcel &data, const std::vector<std::string> &uriVec)
{
    MessageParcel tempParcel;
    tempParcel.SetMaxCapacity(MAX_IPC_RAW_DATA_SIZE);
    if (!tempParcel.WriteStringVector(uriVec)) {
        LOGE("Write uris failed");
        return false;
    }
    size_t dataSize = tempParcel.GetDataSize();
    if (!data.WriteInt32(static_cast<int32_t>(dataSize))) {
        LOGE("Write data size failed");
        return false;
    }
    if (!data.WriteRawData(reinterpret_cast<uint8_t *>(tempParcel.GetData()), dataSize)) {
        LOGE("Write raw data failed");
        return false;
    }
    return true;
}

bool IpcWrapper::WriteBatchUris(MessageParcel &data, const std::vector<std::string> &uriVec)
{
    if (!data.WriteUint32(uriVec.size())) {
        LOGE("Write uri size failed");
        return false;
    }
    if (!WriteUriByRawData(data, uriVec)) {
        LOGE("Write uri by raw data failed");
        return false;
    }
    return true;
}

bool IpcWrapper::GetData(void *&buffer, size_t size, const void *data)
{
    if (data == nullptr) {
        LOGE("null data");
        return false;
    }
    if (size == 0 || size > MAX_IPC_RAW_DATA_SIZE) {
        LOGE("size invalid: %{public}zu", size);
        return false;
    }
    buffer = malloc(size);
    if (buffer == nullptr) {
        LOGE("malloc buffer failed");
        return false;
    }
    if (memcpy_s(buffer, size, data, size) != FileManagement::E_OK) {
        free(buffer);
        LOGE("memcpy failed");
        return false;
    }
    return true;
}

bool IpcWrapper::ReadBatchUriByRawData(MessageParcel &data, std::vector<std::string> &uriVec)
{
    size_t dataSize = static_cast<size_t>(data.ReadInt32());
    if (dataSize == 0) {
        LOGE("parcel no data");
        return false;
    }

    void *buffer = nullptr;
    if (!GetData(buffer, dataSize, data.ReadRawData(dataSize))) {
        LOGE("read raw data failed: %{public}zu", dataSize);
        return false;
    }

    MessageParcel tempParcel;
    if (!tempParcel.ParseFrom(reinterpret_cast<uintptr_t>(buffer), dataSize)) {
        LOGE("failed to parseFrom");
        free(buffer);
        return false;
    }
    tempParcel.ReadStringVector(&uriVec);
    return true;
}

int32_t IpcWrapper::ReadBatchUris(MessageParcel &data, std::vector<std::string> &uriVec)
{
    uint32_t size = data.ReadUint32();
    if (size == 0) {
        LOGE("out of range: %{public}u", size);
        return OHOS::FileManagement::E_INVAL_ARG;
    }
    if (!ReadBatchUriByRawData(data, uriVec)) {
        LOGE("read uris failed");
        return OHOS::FileManagement::E_IPC_READ_FAILED;
    }
    if (size != uriVec.size()) {
        LOGE("size read err, except: %{public}u, actual: %{public}zu", size, uriVec.size());
        return OHOS::FileManagement::E_IPC_READ_FAILED;
    }
    return FileManagement::E_OK;
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
