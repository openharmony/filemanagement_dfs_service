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

#ifndef OHOS_FILEMGMT_CLOUD_DISK_COMMON_H
#define OHOS_FILEMGMT_CLOUD_DISK_COMMON_H

#include <bitset>
#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "parcel.h"

namespace OHOS::FileManagement::CloudDiskService {
constexpr uint64_t MAX_EXECUTE_DATA_SIZE = 128ULL * 1024;

constexpr size_t MAX_CALLBACK_REQUEST_KEY_SIZE = sizeof(uint64_t);

struct CloudDiskPathInfo {
    char *value;
    size_t length;
};

using CloudDiskFileIdInfo = CloudDiskPathInfo;
using CloudDiskSyncFolderPath = CloudDiskPathInfo;

struct CloudDiskDataBuf {
    uint8_t *data;
    uint64_t dataSize;
};

enum class CloudDiskCallbackType : int32_t {
    FETCH_DATA = 0,
    CANCEL_FETCH_DATA,
    FETCH_RANGE_DATA,
    DEHYDRATE,
};

enum CloudDiskHydratePriority : uint32_t {
    CLOUD_DISK_HYDRATE_PRIORITY_LOW = 0,
    CLOUD_DISK_HYDRATE_PRIORITY_NORMAL = 1,
    CLOUD_DISK_HYDRATE_PRIORITY_HIGH = 2,
};

struct CloudDiskCallbackReqHead {
    CloudDiskSyncFolderPath syncFolderPath;
    CloudDiskCallbackType callbackType;
    CloudDiskDataBuf reqKey;
};

struct CloudDiskRangeInfo {
    CloudDiskPathInfo filePath;
    uint64_t offset;
    uint64_t size;
    CloudDiskDataBuf data;
};

struct CloudDiskDehydrateInfo {
    CloudDiskPathInfo filePath;
    bool allow;
};

struct CloudDiskFetchDataRequest {
    CloudDiskPathInfo filePath;
    CloudDiskHydratePriority priority;
};

union CloudDiskCallbackContext {
    CloudDiskFetchDataRequest *fetchData;
    CloudDiskPathInfo *cancelFetchData;
    CloudDiskRangeInfo *fetchRangeData;
    CloudDiskDehydrateInfo *dehydrateData;
};

struct CloudDiskFetchData {
    uint64_t offset;
    uint64_t size;
    uint64_t totalSize;
    CloudDiskDataBuf data;
    bool isComplete;
};

union CloudDiskCallbackResponse {
    CloudDiskFetchData *fetchData;
};

struct CallbackExecuteRequest final : public Parcelable {
    std::vector<uint8_t> reqKey;
    std::string syncFolder;
    std::string filePath;
    int32_t callbackType = static_cast<int32_t>(CloudDiskCallbackType::FETCH_DATA);
    uint64_t offset = 0;
    uint64_t size = 0;
    uint64_t totalSize = 0;
    std::vector<uint8_t> data;
    bool isComplete = false;

    bool Marshalling(Parcel &parcel) const override;
    bool ReadFromParcel(Parcel &parcel);
    static CallbackExecuteRequest *Unmarshalling(Parcel &parcel);
};

enum class HydrateProgressState : int32_t {
    PENDING = 0,
    IN_PROGRESS,
    COMPLETED,
    CANCELLED,
};

struct HydrateProgress final : public Parcelable {
    std::string filePath;
    int32_t state = static_cast<int32_t>(HydrateProgressState::PENDING);
    uint64_t processedSize = 0;
    uint64_t totalSize = 0;

    bool Marshalling(Parcel &parcel) const override;
    bool ReadFromParcel(Parcel &parcel);
    static HydrateProgress *Unmarshalling(Parcel &parcel);
};

struct CallbackParcelStorage {
    std::string syncFolder;
    std::vector<uint8_t> reqKey;
    std::string filePath;
    std::vector<uint8_t> rangeData;
    CloudDiskPathInfo pathInfo{};
    CloudDiskFetchDataRequest fetchDataRequest{};
    CloudDiskRangeInfo rangeInfo{};
    CloudDiskDehydrateInfo dehydrateInfo{};
};

/*
When adding new enumeration values, pay attention to the maximum enumeration value judgment of the getxattr method in
clouddiskservice.cpp
*/
enum class SyncState {
    IDLE = 0,
    SYNCING,
    SYNC_SUCCESSED,
    SYNC_FAILED,
    SYNC_CANCELED,
    SYNC_CONFLICTED,
};

enum class OperationType : uint8_t {
    CREATE = 0,
    DELETE,
    MOVE_FROM,
    MOVE_TO,
    CLOSE_WRITE,
    SYNC_FOLDER_INVALID,
    OH_CLOUD_DISK_CLOSE_MODIFY,
    OPERATION_MAX,
};

struct FileSyncState : public Parcelable {
    std::string path;
    SyncState state;
    bool ReadFromParcel(Parcel &parcel);
    bool Marshalling(Parcel &parcel) const override;
    static FileSyncState *Unmarshalling(Parcel &parcel);
};

struct ChangeData : public Parcelable {
    uint64_t updateSequenceNumber;
    std::string fileId;
    std::string parentFileId;
    std::string relativePath;
    OperationType operationType;
    uint64_t size;
    uint64_t mtime;
    uint64_t timeStamp;
    bool ReadFromParcel(Parcel &parcel);
    bool Marshalling(Parcel &parcel) const override;
    static ChangeData *Unmarshalling(Parcel &parcel);
};

bool WriteCallbackParcel(Parcel &parcel,
                         const CloudDiskCallbackReqHead &reqHead,
                         const CloudDiskCallbackContext &context);
bool ReadCallbackParcel(Parcel &parcel,
                        CloudDiskCallbackReqHead &reqHead,
                        CloudDiskCallbackContext &context,
                        CallbackParcelStorage &storage);
bool WriteCallbackReply(Parcel &parcel,
                        CloudDiskCallbackType callbackType,
                        const CloudDiskCallbackContext &context,
                        uint64_t rangeDataCapacity);
bool ReadCallbackReply(Parcel &parcel, CloudDiskCallbackType callbackType, CloudDiskCallbackContext &context);

struct ChangesResult : public Parcelable {
    uint64_t nextUsn;
    bool isEof;
    std::vector<ChangeData> changesData;
    bool ReadFromParcel(Parcel &parcel);
    bool Marshalling(Parcel &parcel) const override;
    static ChangesResult *Unmarshalling(Parcel &parcel);
};

enum class ErrorReason {
    INVALID_ARGUMENT = 0,
    NO_SUCH_FILE,
    NO_SPACE_LEFT,
    OUT_OF_RANGE,
    NO_SYNC_STATE,
};

struct FailedList : public Parcelable {
    std::string path;
    ErrorReason error;
    virtual ~FailedList() = default;
    bool ReadFromParcel(Parcel &parcel);
    bool Marshalling(Parcel &parcel) const override;
    static FailedList *Unmarshalling(Parcel &parcel);
};

struct ResultList : public FailedList {
    bool isSuccess;
    SyncState state;
    bool ReadFromParcel(Parcel &parcel);
    bool Marshalling(Parcel &parcel) const override;
    static ResultList *Unmarshalling(Parcel &parcel);
};

struct PlaceholderInfo : public Parcelable {
    uint64_t logicalSize{0};
    uint64_t atimeMs{0};
    uint64_t mtimeMs{0};
    bool ReadFromParcel(Parcel &parcel);
    bool Marshalling(Parcel &parcel) const override;
    static PlaceholderInfo *Unmarshalling(Parcel &parcel);
};

struct PlaceholderCustomInfo : public Parcelable {
    std::vector<uint8_t> data;
    bool ReadFromParcel(Parcel &parcel);
    bool Marshalling(Parcel &parcel) const override;
    static PlaceholderCustomInfo *Unmarshalling(Parcel &parcel);
};

#define RETURN_ON_ERR(ret)   \
    do {                     \
        int32_t res = ret;   \
        if ((res) != E_OK) { \
            return res;      \
        }                    \
    } while (0)
} // namespace OHOS::FileManagement::CloudDiskService
#endif // OHOS_FILEMGMT_CLOUD_DISK_COMMON_H
