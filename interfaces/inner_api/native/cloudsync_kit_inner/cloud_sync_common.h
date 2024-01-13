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

#include <map>

#include "parcel.h"
#include "cloud_sync_asset_manager.h"

namespace OHOS::FileManagement::CloudSync {

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
    bool ReadFromParcel(Parcel &parcel);
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
} // namespace OHOS::FileManagement::CloudSync
#endif
