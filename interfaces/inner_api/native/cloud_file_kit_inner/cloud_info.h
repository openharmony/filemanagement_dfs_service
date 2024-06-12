/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_CLOUD_FILE_CLOUD_INFO_H
#define OHOS_CLOUD_FILE_CLOUD_INFO_H

#include <string>

namespace OHOS::FileManagement::CloudFile {
struct CloudUserInfo {
    std::string accountId;
    bool enableCloud{false};
    uint64_t totalSpace{0};
    uint64_t remainSpace{0};
};

struct Asset {
    std::string uri;
    std::string assetName;
};

struct DownloadAssetInfo {
    std::string recordType;
    std::string recordId;
    std::string fieldKey;
    Asset asset;
    std::string downLoadPath;
};

enum class CloudError : int32_t {
    CK_NO_ERROR,
    CK_NETWORK_ERROR,
    CK_SERVER_ERROR,
    CK_LOCAL_ERROR,
    CK_UNKNOWN_ERROR,
};
} // namespace OHOS::FileManagement::CloudFile

#endif // OHOS_CLOUD_FILE_CLOUD_INFO_H