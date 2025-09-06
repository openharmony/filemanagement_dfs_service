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
#include <map>

#include "parcel.h"

namespace OHOS::FileManagement::CloudDiskService {

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

#define RETURN_ON_ERR(ret)   \
    do {                     \
        int32_t res = ret;   \
        if ((res) != E_OK) { \
            return res;      \
        }                    \
    } while (0)
} // namespace OHOS::FileManagement::CloudDiskService
#endif // OHOS_FILEMGMT_CLOUD_DISK_COMMON_H
