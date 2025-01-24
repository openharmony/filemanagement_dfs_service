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

#ifndef OHOS_FILEMGMT_CLOUD_SYNC_COMMON_H
#define OHOS_FILEMGMT_CLOUD_SYNC_COMMON_H

#include <bitset>
#include <map>

#include "parcel.h"
#include "cloud_sync_asset_manager.h"

namespace OHOS::FileManagement::CloudSync {
#define FIELD_KEY_MAX_SIZE 4
constexpr int32_t CLEAN_FILE_MAX_SIZE = 200;
enum FieldKey {
    FIELDKEY_CONTENT = 1 << 0,
    FIELDKEY_THUMB = 1 << 1,
    FIELDKEY_LCD = 1 << 2,
};

struct DownloadProgressObj : public Parcelable {
    enum Status : int32_t {
        RUNNING = 0,
        COMPLETED = 1,
        FAILED = 2,
        STOPPED = 3,
    };
    enum DownloadErrorType : int32_t {
        NO_ERROR = 0,
        UNKNOWN_ERROR = 1,
        NETWORK_UNAVAILABLE = 2,
        LOCAL_STORAGE_FULL = 3,
        CONTENT_NOT_FOUND = 4,
        FREQUENT_USER_REQUESTS = 5,
    };
    uint32_t refCount;
    std::string path;
    int64_t downloadId;
    Status state;
    int32_t downloadErrorType;
    int64_t downloadedSize;
    int64_t totalSize;

    // member for batch download(same downloadId)
    int64_t batchDownloadSize;
    int64_t batchTotalSize;
    int64_t batchSuccNum;
    int64_t batchFailNum;
    int64_t batchTotalNum;
    Status batchState;

    bool ReadBatchFromParcel(Parcel &parcel);
    bool ReadFromParcel(Parcel &parcel);
    bool MarshallingBatch(Parcel &parcel) const;
    bool Marshalling(Parcel &parcel) const override;
    static DownloadProgressObj *Unmarshalling(Parcel &parcel);
    std::string to_string();
};

struct SwitchDataObj : public Parcelable {
    std::map<std::string, bool> switchData;
    bool ReadFromParcel(Parcel &parcel);
    bool Marshalling(Parcel &parcel) const override;
    static SwitchDataObj *Unmarshalling(Parcel &parcel);
};

struct CleanOptions : public Parcelable {
    std::map<std::string, int32_t> appActionsData;
    bool ReadFromParcel(Parcel &parcel);
    bool Marshalling(Parcel &parcel) const override;
    static CleanOptions *Unmarshalling(Parcel &parcel);
};

struct DentryFileInfo {
    std::string cloudId;
    int64_t size;
    int64_t modifiedTime;
    std::string path;
    std::string fileName;
    std::string fileType;
};

struct  DentryFileInfoObj : public Parcelable {
    std::string cloudId;
    int64_t size;
    int64_t modifiedTime;
    std::string path;
    std::string fileName;
    std::string fileType;
    bool ReadFromParcel(Parcel &parcel);
    bool Marshalling(Parcel &parcel) const override;
    static DentryFileInfoObj *Unmarshalling(Parcel &parcel);

    DentryFileInfoObj() = default;
    DentryFileInfoObj(const DentryFileInfo &FileInfo)
        : cloudId(FileInfo.cloudId),
          size(FileInfo.size),
          modifiedTime(FileInfo.modifiedTime),
          path(FileInfo.path),
          fileName(FileInfo.fileName),
          fileType(FileInfo.fileType)
    {
    }
};

struct AssetInfoObj : public Parcelable {
    std::string uri;
    std::string recordType;
    std::string recordId;
    std::string fieldKey;
    std::string assetName;
    bool ReadFromParcel(Parcel &parcel);
    bool Marshalling(Parcel &parcel) const override;
    static AssetInfoObj *Unmarshalling(Parcel &parcel);

    AssetInfoObj() = default;
    AssetInfoObj(const AssetInfo &assetInfo)
        : uri(assetInfo.uri),
          recordType(assetInfo.recordType),
          recordId(assetInfo.recordId),
          fieldKey(assetInfo.fieldKey),
          assetName(assetInfo.assetName)
    {
    }
};

struct CleanFileInfo {
    std::string cloudId;
    int64_t size;
    int64_t modifiedTime;
    std::string path;
    std::string fileName;
    std::vector<std::string> attachment;
};

struct  CleanFileInfoObj : public Parcelable {
    std::string cloudId;
    int64_t size;
    int64_t modifiedTime;
    std::string path;
    std::string fileName;
    std::vector<std::string> attachment;
    bool ReadFromParcel(Parcel &parcel);
    bool Marshalling(Parcel &parcel) const override;
    static CleanFileInfoObj *Unmarshalling(Parcel &parcel);

    CleanFileInfoObj() = default;
    CleanFileInfoObj(const CleanFileInfo &FileInfo)
        : cloudId(FileInfo.cloudId),
          size(FileInfo.size),
          modifiedTime(FileInfo.modifiedTime),
          path(FileInfo.path),
          fileName(FileInfo.fileName),
          attachment(FileInfo.attachment)
    {
    }
};

} // namespace OHOS::FileManagement::CloudSync
#endif
