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

#ifndef DRIVE_KIT_ASSET_H
#define DRIVE_KIT_ASSET_H

#include <string>

namespace DriveKit {
enum class DKAssetOperType {
    DK_ASSET_NONE = 0,
    DK_ASSET_ADD,
    DK_ASSET_UPDATE,
    DK_ASSET_DELETE,
    DK_ASSET_MAX,
};
struct DKAsset {
    std::string uri;
    std::string assetName;
    DKAssetOperType operationType;
    std::string hash;
    uint64_t version{0};
    std::string assetId;
    std::string subPath;
    std::string exCheckInfo;
    uint64_t size{0};
};
} // namespace DriveKit

#endif
